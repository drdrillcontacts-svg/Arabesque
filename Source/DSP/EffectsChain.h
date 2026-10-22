#pragma once
#include <JuceHeader.h>
#include "../Parameters.h"

//==============================================================================
// Master effects chain: Drive -> Chorus -> Phaser -> Delay -> Reverb.
//==============================================================================
class ArabesqueEffectsChain
{
public:
    explicit ArabesqueEffectsChain (juce::AudioProcessorValueTreeState& state);

    void prepare (double sampleRate, int maxBlockSize, int numChannels);
    void reset();
    void release();
    void setTempo (double bpm);

    void process (juce::AudioBuffer<float>& buffer, int numSamples);

private:
    juce::AudioProcessorValueTreeState& apvts;
    double sr = 44100.0;
    double currentBpm = 120.0;
    int    channels = 2;
    int    maxBlock = 0;
    std::atomic<bool> prepared { false };

    juce::SmoothedValue<float> driveGain, driveMix, masterGain;
    juce::LinearSmoothedValue<float> panL, panR;

    juce::dsp::Chorus<float> chorus;
    juce::dsp::Phaser<float> phaser;

    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayL { 384000 },
                                                                              delayR { 384000 };
    juce::dsp::IIR::Filter<float> delayDampL, delayDampR;
    juce::SmoothedValue<float> delayWet, delayDry, delayFb, delayTimeMs;

    juce::dsp::Reverb reverb;

    juce::SmoothedValue<float> fxWet;
    juce::AudioBuffer<float> fxBuffer;

    JUCE_DECLARE_NON_COPYABLE (ArabesqueEffectsChain)
};
