#include "EffectsChain.h"

ArabesqueEffectsChain::ArabesqueEffectsChain (juce::AudioProcessorValueTreeState& state)
    : apvts (state)
{
}

void ArabesqueEffectsChain::prepare (double sampleRate, int maxBlockSize, int numChannels)
{
    sr = sampleRate;
    channels = juce::jmax (1, numChannels);

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = (juce::uint32) maxBlockSize;
    spec.numChannels = (juce::uint32) channels;

    chorus.prepare (spec);
    phaser.prepare (spec);
    reverb.prepare (spec);

    delayL.prepare (spec); delayR.prepare (spec);
    delayL.setMaximumDelayInSamples ((int) (2.0 * sampleRate));
    delayR.setMaximumDelayInSamples ((int) (2.0 * sampleRate));

    delayDampL.prepare (spec); delayDampR.prepare (spec);
    auto dampCoeffs = juce::dsp::IIR::Coefficients<float>::makeFirstOrderLowPass (sampleRate, 4500.0f);
    *delayDampL.coefficients = *dampCoeffs;
    *delayDampR.coefficients = *dampCoeffs;

    driveGain.reset (sampleRate, 0.02);
    driveMix.reset (sampleRate, 0.02);
    masterGain.reset (sampleRate, 0.02);
    panL.reset (sampleRate, 0.02);
    panR.reset (sampleRate, 0.02);
    delayWet.reset (sampleRate, 0.02);
    delayDry.reset (sampleRate, 0.02);
    delayFb.reset (sampleRate, 0.02);
    delayTimeMs.reset (sampleRate, 0.05);
    fxWet.reset (sampleRate, 0.02);

    reset();
}

void ArabesqueEffectsChain::reset()
{
    chorus.reset(); phaser.reset(); reverb.reset();
    delayL.reset(); delayR.reset();
    delayDampL.reset(); delayDampR.reset();
}

void ArabesqueEffectsChain::setTempo (double bpm)
{
    currentBpm = juce::jlimit (20.0, 999.0, bpm);
}

void ArabesqueEffectsChain::process (juce::AudioBuffer<float>& buffer, int numSamples)
{
    auto val = [this] (const char* id) { return apvts.getRawParameterValue (id)->load(); };

    const int numCh = buffer.getNumChannels();

    //------------------------------------------------ master drive (tanh soft clip + make-up)
    const float driveAmt = val (ParamIDs::drive);
    driveMix.setTargetValue (driveAmt);
    driveGain.setTargetValue (1.0f / (1.0f + driveAmt * 1.5f)); // loudness compensation
    const float pregain = 1.0f + driveAmt * 24.0f;

    for (int i = 0; i < numSamples; ++i)
    {
        const float mix = driveMix.getNextValue();
        const float makeup = driveGain.getNextValue();
        const float g = pregain * makeup;
        for (int ch = 0; ch < numCh; ++ch)
        {
            auto* d = buffer.getWritePointer (ch, i);
            *d = *d * (1.0f - mix) + std::tanh (*d * g) * mix * 0.9f;
        }
    }

    //------------------------------------------------ FX master on/off (smooth crossfade)
    const bool fxOn = val (ParamIDs::fxOn) > 0.5f;
    fxWet.setTargetValue (fxOn ? 1.0f : 0.0f);
    const bool fxActive = fxOn || fxWet.getCurrentValue() > 0.001f;

    //------------------------------------------------ chorus / phaser settings
    chorus.setRate (val (ParamIDs::chorusRate));
    chorus.setDepth (val (ParamIDs::chorusDepth));
    chorus.setCentreDelay (7.0f);
    chorus.setFeedback (0.15f);
    chorus.setMix (0.5f);

    phaser.setRate (val (ParamIDs::phaserRate));
    phaser.setDepth (val (ParamIDs::phaserDepth));
    phaser.setCentreFrequency (850.0f);
    phaser.setFeedback (0.35f);
    phaser.setMix (0.5f);

    //------------------------------------------------ tempo-synced delay
    const double beats = Choices::divBeats ((int) val (ParamIDs::delayTime));
    delayTimeMs.setTargetValue ((float) (beats / (currentBpm / 60.0) * 1000.0));
    delayFb.setTargetValue (juce::jlimit (0.0f, 0.95f, val (ParamIDs::delayFeedback)));
    delayWet.setTargetValue (val (ParamIDs::delayMix));
    delayDry.setTargetValue (1.0f - val (ParamIDs::delayMix));

    //------------------------------------------------ reverb settings
    const int rType = (int) val (ParamIDs::reverbType);
    const float decaySecs = val (ParamIDs::reverbDecay);
    const float dampingBase = rType == 1 ? 0.65f : rType == 2 ? 0.55f : rType == 3 ? 0.35f : 0.45f;

    juce::dsp::Reverb::Parameters rp;
    rp.roomSize   = juce::jlimit (0.0f, 1.0f, val (ParamIDs::reverbSize) * (rType == 3 ? 1.0f : 0.85f));
    rp.damping    = juce::jlimit (0.0f, 1.0f, dampingBase * (1.0f - decaySecs / 22.0f));
    rp.wetLevel   = val (ParamIDs::reverbMix);
    rp.dryLevel   = 1.0f;
    rp.width      = 1.0f;
    rp.freezeMode = 0.0f;
    const float reverbWet = rp.wetLevel;
    if (fxActive && reverbWet > 0.001f)
        reverb.setParameters (rp);

    //------------------------------------------------ FX chain into scratch buffer
    if (fxActive)
    {
        const int chs = juce::jmin (numCh, fxBuffer.getNumChannels());
        for (int ch = 0; ch < chs; ++ch)
            fxBuffer.copyFrom (ch, 0, buffer, ch, 0, numSamples);

        juce::dsp::AudioBlock<float> fxBlock (fxBuffer.getArrayOfWritePointers(), (size_t) fxBuffer.getNumChannels(), (size_t) numSamples);
        juce::dsp::ProcessContextReplacing<float> context (fxBlock);

        chorus.process (context);
        phaser.process (context);

        // tempo delay (per sample, keeps its own smooth mix)
        for (int i = 0; i < numSamples; ++i)
        {
            const float delaySamples = delayTimeMs.getNextValue() * (float) sr / 1000.0f;
            const float fb = delayFb.getNextValue();
            const float dWet = delayWet.getNextValue();
            const float dDry = delayDry.getNextValue();

            float inL = fxBuffer.getSample (0, i);
            float inR = chs > 1 ? fxBuffer.getSample (1, i) : inL;

            const float delayedL = delayL.popSample (0, delaySamples, true);
            const float delayedR = chs > 1 ? delayR.popSample (1, delaySamples, true) : delayedL;

            delayL.pushSample (0, inL + delayDampL.processSample (delayedL) * fb);
            if (chs > 1)
                delayR.pushSample (1, inR + delayDampR.processSample (delayedR) * fb);

            fxBuffer.setSample (0, i, inL * dDry + delayedL * dWet);
            if (chs > 1) fxBuffer.setSample (1, i, inR * dDry + delayedR * dWet);
        }

        // reverb tail on the wet buffer
        if (reverbWet > 0.001f)
            reverb.process (context);

        // crossfade dry <-> wet (clickless FX on/off)
        for (int i = 0; i < numSamples; ++i)
        {
            const float wet = fxWet.getNextValue();
            for (int ch = 0; ch < chs; ++ch)
            {
                const float dry = buffer.getSample (ch, i);
                buffer.setSample (ch, i, dry + (fxBuffer.getSample (ch, i) - dry) * wet);
            }
        }
    }
    else
    {
        // keep smoothing states ticking so the next block starts on-target
        fxWet.skip (numSamples);
        delayTimeMs.skip (numSamples); delayFb.skip (numSamples);
        delayWet.skip (numSamples);    delayDry.skip (numSamples);
    }

    //------------------------------------------------ pan + master gain (smoothed, clickless)
    const float panVal = val (ParamIDs::pan);
    const float angle = (panVal * 0.5f + 0.5f) * juce::MathConstants<float>::halfPi;
    panL.setTargetValue (std::cos (angle));
    panR.setTargetValue (std::sin (angle));
    masterGain.setTargetValue (juce::Decibels::decibelsToGain (juce::jmax (-59.5f, val (ParamIDs::mainGain))));

    for (int i = 0; i < numSamples; ++i)
    {
        const float pl = panL.getNextValue();
        const float pr = panR.getNextValue();
        const float mg = masterGain.getNextValue();
        buffer.setSample (0, i, buffer.getSample (0, i) * pl * mg);
        if (numCh > 1) buffer.setSample (1, i, buffer.getSample (1, i) * pr * mg);
    }
}
