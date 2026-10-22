#pragma once
#include <JuceHeader.h>
#include "../Parameters.h"

//==============================================================================
// Master effects chain: Drive -> Chorus -> Phaser -> Delay -> Reverb.
// Everything lives after the synth voices so it is processed once, not per
// voice (much cheaper than the per-voice drive in the starter).
//==============================================================================

class ArabesqueEffectsChain
{
public:
    explicit ArabesqueEffectsChain (juce::AudioProcessorValueTreeState& state);

    void prepare (double sampleRate, int maxBlockSize, int numChannels);
    void reset();
    void setTempo (double bpm);

    // processes the synth output in place
    void process (juce::AudioBuffer<float>& buffer, int numSamples);

private:
    juce::AudioProcessorValueTreeState& apvts;
    double sr = 44100.0;
    double currentBpm = 120.0;
    int    channels = 2;

    // master drive & gain smoothing (zipper-free)
    juce::SmoothedValue<float> driveGain, driveMix, masterGain;
    juce::LinearSmoothedValue<float> panL, panR;

    // chorus / phaser from juce_dsp
    juce::dsp::Chorus<float> chorus;
    juce::dsp::Phaser<float> phaser;

    // hand-rolled tempo-synced stereo delay with damping filter
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayL { 192000 },
                                                                              delayR { 192000 };
    juce::dsp::IIR::Filter<float> delayDampL, delayDampR;
    juce::SmoothedValue<float> delayWet, delayDry, delayFb, delayTimeMs;

    juce::dsp::Reverb reverb;

    juce::SmoothedValue<float> fxWet; // smooth FX on/off crossfade to avoid clicks
    juce::AudioBuffer<float> fxBuffer;

    JUCE_DECLARE_NON_COPYABLE (ArabesqueEffectsChain)
};
