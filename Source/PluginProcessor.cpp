#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Presets.h"

//==============================================================================
ArabesqueAudioProcessor::ArabesqueAudioProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      parameters (*this, nullptr, "PARAMETERS", createParameterLayout()),
      effects (parameters),
      arpeggiator (parameters)
{
    for (int i = 0; i < 32; ++i)
        synth.addVoice (new ArabesqueVoice (parameters, sharedState));
    synth.addSound (new ArabesqueSound());
    synth.setNoteStealingEnabled (true);

    currentPresetName = getFactoryPresets().front().name;
}

//==============================================================================
void ArabesqueAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    audioReady.store (false);

    const double sr = sampleRate > 0.0 ? sampleRate : 44100.0;
    const int block = juce::jmax (1, samplesPerBlock);

    synth.setCurrentPlaybackSampleRate (sr);
    for (int i = 0; i < synth.getNumVoices(); ++i)
        if (auto* v = dynamic_cast<ArabesqueVoice*> (synth.getVoice (i)))
            v->prepare (sr);

    effects.prepare (sr, block, juce::jmax (2, getTotalNumOutputChannels()));
    arpeggiator.prepare (sr);
    lastAppliedVoices = -1;
    ensureVoiceCount();
    keyboardState.reset();
    audioReady.store (true);
}

void ArabesqueAudioProcessor::releaseResources()
{
    audioReady.store (false);
    effects.release();
    keyboardState.reset();
}

bool ArabesqueAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& mainOut = layouts.getMainOutputChannelSet();
    if (mainOut != juce::AudioChannelSet::mono() && mainOut != juce::AudioChannelSet::stereo())
        return false;
    // synth has no audio inputs; input bus must be disabled (or match output)
    const auto& mainIn = layouts.getMainInputChannelSet();
    return mainIn.isDisabled() || mainIn == mainOut;
}

void ArabesqueAudioProcessor::ensureVoiceCount()
{
    static constexpr int counts[] = { 2, 4, 6, 8, 12, 16, 24, 32 };
    const int idx = (int) parameters.getRawParameterValue (ParamIDs::voices)->load();
    const int wanted = counts[juce::jlimit (0, 7, idx)];

    if (wanted == lastAppliedVoices)
        return;
    lastAppliedVoices = wanted;

    while (synth.getNumVoices() > wanted)
        synth.removeVoice (synth.getNumVoices() - 1);
    while (synth.getNumVoices() < wanted)
    {
        auto* v = new ArabesqueVoice (parameters, sharedState);
        if (getSampleRate() > 0.0) v->prepare (getSampleRate());
        synth.addVoice (v);
    }
}

void ArabesqueAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;

    const int numSamples = buffer.getNumSamples();
    if (! audioReady.load() || getTotalNumOutputChannels() == 0 || numSamples == 0)
    {
        buffer.clear();
        return;
    }

    //------------------------------------------------------- tempo from host
    double bpm = sharedState.bpm.load();
    double ppq = -1.0;
    bool isPlaying = false;
    if (auto* ph = getPlayHead())
    {
        if (auto pos = ph->getPosition())
        {
            if (auto b = pos->getBpm())         bpm = juce::jlimit (20.0, 999.0, *b);
            if (auto q = pos->getPpqPosition()) ppq = *q;
            isPlaying = pos->getIsPlaying();
        }
    }
    sharedState.bpm.store (bpm);
    effects.setTempo (bpm);

    ensureVoiceCount();

    //------------------------------------------------------- merge UI extras
    // on-screen keyboard notes
    keyboardState.processNextMidiBuffer (midi, 0, numSamples, true);

    // pitch/mod wheels drawn in the UI -> standard MIDI controller messages
    {
        const float bend = uiPitchBend.load();
        const float mod  = uiModWheel.load();
        if (! juce::exactlyEqual (bend, lastUiBend))
        {
            midi.addEvent (juce::MidiMessage::pitchWheel (1, juce::jlimit (0, 16383, (int) std::round ((bend + 1.0f) * 8191.5f))), 0);
            lastUiBend = bend;
        }
        if (! juce::exactlyEqual (mod, lastUiMod))
        {
            midi.addEvent (juce::MidiMessage::controllerEvent (1, 1, juce::jlimit (0, 127, (int) std::round (mod * 127.0f))), 0);
            lastUiMod = mod;
        }
    }

    //------------------------------------------------------- arp + synth + fx
    arpeggiator.process (midi, numSamples, bpm, ppq, isPlaying);

    buffer.clear();
    synth.renderNextBlock (buffer, midi, 0, numSamples);

    effects.process (buffer, numSamples);
}

//==============================================================================
juce::AudioProcessorEditor* ArabesqueAudioProcessor::createEditor()
{
    return new ArabesqueAudioProcessorEditor (*this);
}

//==============================================================================
// presets
size_t ArabesqueAudioProcessor::numPresets()
{
    return getFactoryPresets().size();
}

void ArabesqueAudioProcessor::applyFactoryPreset (int index)
{
    auto& presets = getFactoryPresets();
    if (! juce::isPositiveAndBelow (index, (int) presets.size()))
        return;

    // start from defaults so older presets gain sensible values for new params
    for (auto* param : getParameters())
        if (auto* ranged = dynamic_cast<juce::RangedAudioParameter*> (param))
            ranged->setValueNotifyingHost (ranged->getDefaultValue());

    for (auto& kv : presets[(size_t) index].values)
        if (auto* p = parameters.getParameter (kv.first))
            if (auto* rp = dynamic_cast<juce::RangedAudioParameter*> (p))
                rp->setValueNotifyingHost (rp->convertTo0to1 (kv.second));

    currentPresetIndex.store (index);
    currentPresetName = presets[(size_t) index].name;
}

void ArabesqueAudioProcessor::setCurrentProgram (int index)
{
    applyFactoryPreset (index);
}

const juce::String ArabesqueAudioProcessor::getProgramName (int index)
{
    auto& presets = getFactoryPresets();
    return juce::isPositiveAndBelow (index, (int) presets.size())
         ? juce::String (index + 1).paddedLeft ('0', 2) + " - " + presets[(size_t) index].name
         : juce::String();
}

juce::File ArabesqueAudioProcessor::getUserPresetFolder() const
{
    auto dir = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
                    .getChildFile ("ARABESQUE Audio").getChildFile ("Presets");
    dir.createDirectory();
    return dir;
}

void ArabesqueAudioProcessor::saveCurrentAsUserPreset (const juce::String& presetName)
{
    auto safeName = juce::File::createLegalFileName (presetName.isNotEmpty() ? presetName : "Custom Preset");
    auto file = getUserPresetFolder().getChildFile (safeName + ".arabesque");
    if (auto state = parameters.copyState().createXml())
        state->writeTo (file);
    currentPresetName = presetName.isNotEmpty() ? presetName : "Custom Preset";
}

std::vector<juce::File> ArabesqueAudioProcessor::getUserPresets() const
{
    std::vector<juce::File> files;
    for (const auto& f : getUserPresetFolder().findChildFiles (juce::File::findFiles, false, "*.arabesque"))
        files.push_back (f);
    std::sort (files.begin(), files.end(),
               [] (const juce::File& a, const juce::File& b) { return a.getFileName() < b.getFileName(); });
    return files;
}

void ArabesqueAudioProcessor::loadUserPreset (const juce::File& file)
{
    if (auto xml = juce::XmlDocument::parse (file))
    {
        if (xml->hasTagName (parameters.state.getType()))
        {
            parameters.replaceState (juce::ValueTree::fromXml (*xml));
            currentPresetName = file.getFileNameWithoutExtension();
        }
    }
}

//==============================================================================
// state
void ArabesqueAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    state.setProperty ("presetIndex", currentPresetIndex.load(), nullptr);
    state.setProperty ("presetName", currentPresetName, nullptr);
    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void ArabesqueAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
    {
        if (xml->hasTagName (parameters.state.getType()))
        {
            auto tree = juce::ValueTree::fromXml (*xml);
            currentPresetIndex.store ((int) tree.getProperty ("presetIndex", 0));
            currentPresetName = tree.getProperty ("presetName", getFactoryPresets().front().name).toString();
            parameters.replaceState (tree);
        }
    }
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ArabesqueAudioProcessor();
}
