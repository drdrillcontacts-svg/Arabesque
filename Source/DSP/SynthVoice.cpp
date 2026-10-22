#include "SynthVoice.h"

ArabesqueVoice::ArabesqueVoice (juce::AudioProcessorValueTreeState& state, VoiceSharedState& shared)
    : apvts (state), sharedState (shared)
{
    auto gp = [this] (const char* id) { return apvts.getRawParameterValue (id); };

    osc1Wave = gp (ParamIDs::osc1Wave);  osc1Oct = gp (ParamIDs::osc1Octave);
    osc1Semi = gp (ParamIDs::osc1Semi);  osc1Fine = gp (ParamIDs::osc1Fine);
    osc2Wave = gp (ParamIDs::osc2Wave);  osc2Oct = gp (ParamIDs::osc2Octave);
    osc2Semi = gp (ParamIDs::osc2Semi);  osc2Fine = gp (ParamIDs::osc2Fine);

    mixOsc1 = gp (ParamIDs::mixOsc1);    mixOsc2 = gp (ParamIDs::mixOsc2);
    mixNoise = gp (ParamIDs::mixNoise);  mixSub  = gp (ParamIDs::mixSub);

    filterType = gp (ParamIDs::filterType);
    cutoff = gp (ParamIDs::cutoff); resonance = gp (ParamIDs::resonance);
    keyFollow = gp (ParamIDs::keyFollow);

    ampA = gp (ParamIDs::ampAttack);  ampD = gp (ParamIDs::ampDecay);
    ampS = gp (ParamIDs::ampSustain); ampR = gp (ParamIDs::ampRelease);
    fltA = gp (ParamIDs::fltAttack);  fltD = gp (ParamIDs::fltDecay);
    fltS = gp (ParamIDs::fltSustain); fltR = gp (ParamIDs::fltRelease);
    pchA = gp (ParamIDs::pchAttack);  pchD = gp (ParamIDs::pchDecay);
    pchS = gp (ParamIDs::pchSustain); pchR = gp (ParamIDs::pchRelease);

    lfoShape = gp (ParamIDs::lfoShape); lfoRate = gp (ParamIDs::lfoRate);
    lfoSync = gp (ParamIDs::lfoSync);   lfoSyncRate = gp (ParamIDs::lfoSyncRate);
    lfoDepth = gp (ParamIDs::lfoDepth);

    vibrato = gp (ParamIDs::vibrato);
    glide   = gp (ParamIDs::glide);
}

void ArabesqueVoice::prepare (double sampleRate)
{
    sr = sampleRate;
    osc1.prepare (sr); osc2.prepare (sr); sub.prepare (sr);
    noise.prepare (sr); lfo.prepare (sr);
    filter.prepare (sr);

    ampEnv.setSampleRate (sr);
    filterEnv.setSampleRate (sr);
    pitchEnv.setSampleRate (sr);

    smoothedCutoff.reset (sr, 0.02);
    for (auto& s : smoothedMix) s.reset (sr, 0.02);

    isPrepared = true;
}

bool ArabesqueVoice::canPlaySound (juce::SynthesiserSound* sound)
{
    // starter bug: dynamic_cast<juce::SynthesiserSound*> always succeeds.
    // Must cast to the actual sound class.
    return dynamic_cast<ArabesqueSound*> (sound) != nullptr;
}

float ArabesqueVoice::pitchRatioFromSemitones (float semis) noexcept
{
    return std::pow (2.0f, semis / 12.0f);
}

void ArabesqueVoice::startNote (int midiNote, float velocity, juce::SynthesiserSound*, int)
{
    currentNoteNumber = midiNote;

    // glide in the pitch domain: constant "semitones per second" speed,
    // independent of interval size (the starter glided in Hz which sounds wrong)
    const double newPitch = (double) midiNote;
    if (glide->load() > 0.001f && currentPitchSemis > 0.0)
        targetPitchSemis = newPitch;
    else
        currentPitchSemis = targetPitchSemis = newPitch;

    noteVelocity = velocity;

    osc1.reset(); osc2.reset(); sub.reset();
    lfo.startNote();
    filter.reset();

    ampEnv.setParameters ({ ampA->load(), ampD->load(), ampS->load(), ampR->load() });
    filterEnv.setParameters ({ fltA->load(), fltD->load(), fltS->load(), fltR->load() });
    pitchEnv.setParameters ({ pchA->load(), pchD->load(), pchS->load(), pchR->load() });

    ampEnv.noteOn(); filterEnv.noteOn(); pitchEnv.noteOn();
}

void ArabesqueVoice::stopNote (float, bool allowTailOff)
{
    if (allowTailOff)
    {
        ampEnv.noteOff(); filterEnv.noteOff(); pitchEnv.noteOff();
    }
    else
    {
        ampEnv.reset(); filterEnv.reset(); pitchEnv.reset();
        clearCurrentNote();
    }
}

void ArabesqueVoice::pitchWheelMoved (int newPitchWheelValue)
{
    sharedState.bend.store ((newPitchWheelValue - 8192) / 8192.0f * 2.0f); // +-2 semitones
}

void ArabesqueVoice::controllerMoved (int controllerNumber, int newControllerValue)
{
    if (controllerNumber == 1)
        sharedState.mod.store (newControllerValue / 127.0f);
}

void ArabesqueVoice::renderNextBlock (juce::AudioBuffer<float>& output, int start, int numSamples)
{
    if (! isVoiceActive() || ! isPrepared || output.getNumChannels() <= 0 || numSamples <= 0)
        return;

    //------------------------------------------------------- per-block reads
    const float glideSeconds  = glide->load();
    const double glideCoef    = glideSeconds > 0.001f
        ? 1.0 - std::exp (-1.0 / (sr * (double) glideSeconds)) : 1.0;

    const float vibAmount     = juce::jlimit (0.0f, 1.0f, vibrato->load());
    const float lfoDepthAmt   = lfoDepth->load();
    const float modWheel      = juce::jlimit (0.0f, 1.0f, sharedState.mod.load());
    const float pitchBendSemi = juce::jlimit (-2.0f, 2.0f, sharedState.bend.load());

    lfo.setShape ((int) lfoShape->load());
    if (lfoSync->load() > 0.5f)
    {
        const double bpm   = juce::jmax (20.0, sharedState.bpm.load());
        const double beats = Choices::divBeats ((int) lfoSyncRate->load());
        lfo.setFrequency ((float) (bpm / 60.0 / beats));
    }
    else
        lfo.setFrequency (lfoRate->load());

    // total vibrato depth in semitones (knob + mod wheel), max ~1.5 st
    const float vibDepthSemis = (vibAmount * 1.0f + modWheel * 0.5f) * lfoDepthAmt * 1.5f;

    const int   fType     = (int) filterType->load();
    const float keyTrack  = keyFollow->load();
    const float reso      = resonance->load();
    const float baseCut   = cutoff->load();

    const float velGain   = 0.25f + 0.75f * noteVelocity;

    // mixer levels: dB -> gain, smoothed to avoid crackle when moving knobs
    smoothedMix[0].setTargetValue (juce::Decibels::decibelsToGain (juce::jmax (-59.5f, mixOsc1->load())));
    smoothedMix[1].setTargetValue (juce::Decibels::decibelsToGain (juce::jmax (-59.5f, mixOsc2->load())));
    smoothedMix[2].setTargetValue (mixNoise->load() <= -59.5f ? 0.0f : juce::Decibels::decibelsToGain (mixNoise->load()));
    smoothedMix[3].setTargetValue (mixSub->load()   <= -59.5f ? 0.0f : juce::Decibels::decibelsToGain (mixSub->load()));

    osc1.setWave ((int) osc1Wave->load());
    osc2.setWave ((int) osc2Wave->load());
    filter.setMode (fType);

    smoothedCutoff.setTargetValue (baseCut);

    // how many semitones of filter-env modulation relative to base cutoff
    constexpr float filterEnvOctaves = 3.5f;

    for (int i = 0; i < numSamples; ++i)
    {
        //--------------------------------- pitch: glide + bend + pitch-env
        currentPitchSemis += (targetPitchSemis - currentPitchSemis) * glideCoef;
        const float pitchEnvSemis = pitchEnv.getNextSample() * 12.0f;
        const double noteSemis = currentPitchSemis + pitchBendSemi + pitchEnvSemis;

        const float vibSemis = lfo.process() * vibDepthSemis;

        //--------------------------------- oscillator tunings
        const float o1Semis = (float) noteSemis + vibSemis
                            + osc1Oct->load() * 12.0f + osc1Semi->load() + osc1Fine->load() / 100.0f;
        const float o2Semis = (float) noteSemis + vibSemis
                            + osc2Oct->load() * 12.0f + osc2Semi->load() + osc2Fine->load() / 100.0f;

        const float f1 = (float) (440.0 * pitchRatioFromSemitones (o1Semis - 69.0f));
        const float f2 = (float) (440.0 * pitchRatioFromSemitones (o2Semis - 69.0f));

        osc1.setFrequency (f1);
        osc2.setFrequency (f2);
        sub.setFrequency ((float) (440.0 * pitchRatioFromSemitones ((float) noteSemis + vibSemis - 69.0f)));

        //--------------------------------- mix
        const float s1 = osc1.process() * smoothedMix[0].getNextValue();
        const float s2 = osc2.process() * smoothedMix[1].getNextValue();
        const float sn = noise.process() * smoothedMix[2].getNextValue();
        const float ss = sub.process()   * smoothedMix[3].getNextValue();
        const float mixed = s1 + s2 + sn + ss;

        //--------------------------------- filter: key-follow + envelope
        const float envMod = filterEnv.getNextSample();
        const float keyRatio = pitchRatioFromSemitones ((float) (currentPitchSemis - 60.0) * keyTrack);
        const float fEnvRatio = std::pow (2.0f, envMod * filterEnvOctaves);
        const float fCut = smoothedCutoff.getNextValue() * keyRatio * fEnvRatio;

        filter.setCutoffAndResonance (fCut, reso);

        float outL = filter.process (0, mixed);
        float outR = filter.process (1, mixed);

        //--------------------------------- amp envelope + velocity
        const float env = ampEnv.getNextSample() * velGain;
        outL *= env;
        outR *= env;

        if (output.getNumChannels() == 1)
        {
            output.addSample (0, start + i, (outL + outR) * 0.5f);
        }
        else
        {
            output.addSample (0, start + i, outL);
            output.addSample (1, start + i, outR);
        }
    }

    if (! ampEnv.isActive())
        clearCurrentNote();
}
