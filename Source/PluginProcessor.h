#pragma once
#include <JuceHeader.h>
#include "Parameters.h"
#include "DSP/SynthVoice.h"
#include "DSP/SynthSound.h"
#include "DSP/EffectsChain.h"
#include "DSP/Arpeggiator.h"

//==============================================================================
class ArabesqueAudioProcessor final : public juce::AudioProcessor
{
public:
    ArabesqueAudioProcessor();
    ~ArabesqueAudioProcessor() override = default;

    //-------------------------------------------------- AudioProcessor ------
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override                                     { return true; }

    const juce::String getName() const override                         { return JucePlugin_Name; }
    bool acceptsMidi() const override                                   { return true; }
    bool producesMidi() const override                                  { return false; }
    bool isMidiEffect() const override                                  { return false; }
    double getTailLengthSeconds() const override                        { return 4.0; }

    int getNumPrograms() override                                       { return (int) numPresets(); }
    int getCurrentProgram() override                                    { return currentPresetIndex.load(); }
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override          {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //-------------------------------------------------- ARABESQUE API -------
    juce::AudioProcessorValueTreeState parameters;
    juce::MidiKeyboardState keyboardState;   // drives the on-screen keyboard
    VoiceSharedState sharedState;            // wheels + tempo shared with voices

    int  getCurrentArpStepForUI() const noexcept { return arpeggiator.getCurrentStepForUI(); }
    double getCurrentBpm() const noexcept         { return sharedState.bpm.load(); }

    void ensureVoiceCount();

    // presets
    static size_t numPresets();
    void applyFactoryPreset (int index);
    void saveCurrentAsUserPreset (const juce::String& presetName);
    std::vector<juce::File> getUserPresets() const;
    void loadUserPreset (const juce::File& file);
    const juce::String& getCurrentPresetName() const noexcept { return currentPresetName; }

    // UI wheels (writing from message thread, read on audio thread)
    std::atomic<float> uiPitchBend { 0.0f }; // -1..1
    std::atomic<float> uiModWheel  { 0.0f }; //  0..1

    juce::File getUserPresetFolder() const;

private:
    juce::Synthesiser synth;
    ArabesqueEffectsChain effects;
    ArabesqueArpeggiator arpeggiator;

    std::atomic<int> currentPresetIndex { 0 };
    juce::String currentPresetName;

    int  lastAppliedVoices = -1;
    float lastUiBend = -99.0f, lastUiMod = -99.0f;
    std::atomic<bool> audioReady { false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ArabesqueAudioProcessor)
};
