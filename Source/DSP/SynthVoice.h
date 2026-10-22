#pragma once
#include <JuceHeader.h>
#include "Oscillators.h"
#include "StateVariableFilter.h"
#include "SynthSound.h"
#include "../Parameters.h"

struct VoiceSharedState
{
    std::atomic<double> bpm   { 120.0 };
    std::atomic<float>  bend  { 0.0f };
    std::atomic<float>  mod   { 0.0f };
};

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
    std::atomic<float>* gp (const char* id) noexcept;

    juce::AudioProcessorValueTreeState& apvts;
    VoiceSharedState& sharedState;
    std::atomic<float> dummy { 0.0f };

    std::atomic<float> *osc1Wave, *osc1Oct, *osc1Semi, *osc1Fine, *mixOsc1;
    std::atomic<float> *osc2Wave, *osc2Oct, *osc2Semi, *osc2Fine, *mixOsc2;
    std::atomic<float> *mixNoise, *mixSub, *pwm, *oscFM;
    std::atomic<float> *unisonN, *unisonDet, *unisonSpr;
    std::atomic<float> *filterType, *cutoff, *resonance, *keyFollow, *filterEnvAmt, *filterDrive;
    std::atomic<float> *ampA, *ampD, *ampS, *ampR;
    std::atomic<float> *fltA, *fltD, *fltS, *fltR;
    std::atomic<float> *pchA, *pchD, *pchS, *pchR;
    std::atomic<float> *lfoShape, *lfoRate, *lfoSync, *lfoSyncRate, *lfoDepth;
    std::atomic<float> *vibrato, *glide;

    static constexpr int kMaxUni = 7;
    PolyBLEPOscillator osc1u[kMaxUni], osc2u[kMaxUni];
    SubOscillator      sub;
    NoiseSource        noise;
    VoiceLFO           lfo;
    TPTStateVariableFilter filter;
    juce::ADSR ampEnv, filterEnv, pitchEnv;

    juce::SmoothedValue<float> smoothedCutoff;
    juce::LinearSmoothedValue<float> smoothedMix[4];

    juce::Random rng { 7 };
    double sr = 44100.0;
    double currentPitchSemis = 0.0, targetPitchSemis = 0.0;
    float  noteVelocity = 0.0f;
    float  driftCents = 0.0f;
    float  dcX1 = 0.0f, dcY1 = 0.0f;
    int    currentNoteNumber = 60;
    bool   isPrepared = false;

    JUCE_DECLARE_NON_COPYABLE (ArabesqueVoice)
};
