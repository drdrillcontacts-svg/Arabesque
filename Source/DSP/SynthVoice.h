#pragma once
#include <JuceHeader.h>
#include "Oscillators.h"
#include "StateVariableFilter.h"
#include "SynthSound.h"
#include "../Parameters.h"

//==============================================================================
// Shared data the processor passes down to every voice (host tempo, wheels).
struct VoiceSharedState
{
    std::atomic<double> bpm   { 120.0 };
    std::atomic<float>  bend  { 0.0f }; // semitones, set from pitch wheel / UI wheel
    std::atomic<float>  mod   { 0.0f }; // 0..1, mod wheel (adds to vibrato depth)
};

//==============================================================================
class ArabesqueVoice final : public juce::SynthesiserVoice
{
public:
    ArabesqueVoice (juce::AudioProcessorValueTreeState& state, VoiceSharedState& shared);

    bool canPlaySound (juce::SynthesiserSound*) override;
    void startNote (int midiNote, float velocity, juce::SynthesiserSound*, int currentPitchWheelPosition) override;
    void stopNote (float velocity, bool allowTailOff) override;
    void pitchWheelMoved (int newPitchWheelValue) override;
    void controllerMoved (int controllerNumber, int newControllerValue) override;
    void renderNextBlock (juce::AudioBuffer<float>&, int startSample, int numSamples) override;
    void prepare (double sampleRate);

private:
    static float pitchRatioFromSemitones (float semis) noexcept;

    juce::AudioProcessorValueTreeState& apvts;
    VoiceSharedState& sharedState;

    // raw parameter pointers (read once per block - safe, lock-free)
    struct ParamPtr
    {
        std::atomic<float>* get (juce::AudioProcessorValueTreeState& s, const char* id) { return s.getRawParameterValue (id); }
    };

    std::atomic<float> *osc1Wave, *osc1Oct, *osc1Semi, *osc1Fine, *mixOsc1;
    std::atomic<float> *osc2Wave, *osc2Oct, *osc2Semi, *osc2Fine, *mixOsc2;
    std::atomic<float> *mixNoise, *mixSub;
    std::atomic<float> *filterType, *cutoff, *resonance, *keyFollow;
    std::atomic<float> *ampA, *ampD, *ampS, *ampR;
    std::atomic<float> *fltA, *fltD, *fltS, *fltR;
    std::atomic<float> *pchA, *pchD, *pchS, *pchR;
    std::atomic<float> *lfoShape, *lfoRate, *lfoSync, *lfoSyncRate, *lfoDepth;
    std::atomic<float> *vibrato, *glide;

    PolyBLEPOscillator osc1, osc2;
    SubOscillator      sub;
    NoiseSource        noise;
    VoiceLFO           lfo;
    TPTStateVariableFilter filter;
    juce::ADSR ampEnv, filterEnv, pitchEnv;

    juce::SmoothedValue<float> smoothedCutoff; // stops zipper noise on the cutoff knob
    juce::LinearSmoothedValue<float> smoothedMix[4];

    double sr = 44100.0;
    double currentPitchSemis = 0.0, targetPitchSemis = 0.0; // glide in pitch domain (log), not Hz
    float  noteVelocity = 0.0f;
    int    currentNoteNumber = 60;
    bool   isPrepared = false;

    JUCE_DECLARE_NON_COPYABLE (ArabesqueVoice)
};
