#include "EffectsChain.h"

ArabesqueEffectsChain::ArabesqueEffectsChain (juce::AudioProcessorValueTreeState& state)
    : apvts (state)
{
}

void ArabesqueEffectsChain::prepare (double sampleRate, int maxBlockSize, int numChannels)
{
    prepared.store (false);

    sr = sampleRate > 0.0 ? sampleRate : 44100.0;
    channels = juce::jmax (1, numChannels);
    maxBlock = juce::jmax (1, maxBlockSize);

    juce::dsp::ProcessSpec stereoSpec;
    stereoSpec.sampleRate = sr;
    stereoSpec.maximumBlockSize = (juce::uint32) maxBlock;
    stereoSpec.numChannels = (juce::uint32) juce::jmax (2, channels);

    juce::dsp::ProcessSpec monoSpec;
    monoSpec.sampleRate = sr;
    monoSpec.maximumBlockSize = (juce::uint32) maxBlock;
    monoSpec.numChannels = 1;

    chorus.prepare (stereoSpec);
    phaser.prepare (stereoSpec);
    reverb.prepare (stereoSpec);

    delayL.prepare (monoSpec);
    delayR.prepare (monoSpec);
    const int maxDelay = juce::jmax (256, (int) (2.0 * sr) + 16);
    delayL.setMaximumDelayInSamples (maxDelay);
    delayR.setMaximumDelayInSamples (maxDelay);

    delayDampL.prepare (monoSpec);
    delayDampR.prepare (monoSpec);
    auto dampCoeffs = juce::dsp::IIR::Coefficients<float>::makeFirstOrderLowPass (sr, 4500.0f);
    delayDampL.coefficients = dampCoeffs;
    delayDampR.coefficients = juce::dsp::IIR::Coefficients<float>::makeFirstOrderLowPass (sr, 4500.0f);

    driveGain.reset (sr, 0.02);
    driveMix.reset (sr, 0.02);
    masterGain.reset (sr, 0.02);
    panL.reset (sr, 0.02);
    panR.reset (sr, 0.02);
    delayWet.reset (sr, 0.02);
    delayDry.reset (sr, 0.02);
    delayFb.reset (sr, 0.02);
    delayTimeMs.reset (sr, 0.05);
    fxWet.reset (sr, 0.02);

    fxBuffer.setSize (2, maxBlock, false, true, false);
    fxBuffer.clear();

    reset();
    prepared.store (true);
}

void ArabesqueEffectsChain::release()
{
    prepared.store (false);
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
    if (! prepared.load() || numSamples <= 0)
        return;

    const int numCh = buffer.getNumChannels();
    if (numCh <= 0)
        return;

    if (fxBuffer.getNumChannels() < 2 || fxBuffer.getNumSamples() < numSamples)
        return; // never allocate on the audio thread

    auto val = [this] (const char* id) -> float
    {
        if (auto* p = apvts.getRawParameterValue (id))
            return p->load();
        return 0.0f;
    };

    //------------------------------------------------ master drive (tanh soft clip + make-up)
    const float driveAmt = val (ParamIDs::drive);
    driveMix.setTargetValue (driveAmt);
    driveGain.setTargetValue (1.0f / (1.0f + driveAmt * 1.5f));
    const float pregain = 1.0f + driveAmt * 24.0f;

    for (int i = 0; i < numSamples; ++i)
    {
        const float mix = driveMix.getNextValue();
        const float makeup = driveGain.getNextValue();
        const float g = pregain * makeup;
        for (int ch = 0; ch < numCh; ++ch)
        {
            const float s = buffer.getSample (ch, i);
            buffer.setSample (ch, i, s * (1.0f - mix) + std::tanh (s * g) * mix * 0.9f);
        }
    }

    const bool fxOn = val (ParamIDs::fxOn) > 0.5f;
    fxWet.setTargetValue (fxOn ? 1.0f : 0.0f);
    const bool fxActive = fxOn || fxWet.getCurrentValue() > 0.001f;

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

    const double beats = Choices::divBeats ((int) val (ParamIDs::delayTime));
    delayTimeMs.setTargetValue ((float) (beats / (currentBpm / 60.0) * 1000.0));
    delayFb.setTargetValue (juce::jlimit (0.0f, 0.95f, val (ParamIDs::delayFeedback)));
    delayWet.setTargetValue (val (ParamIDs::delayMix));
    delayDry.setTargetValue (1.0f - val (ParamIDs::delayMix));

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

    if (fxActive)
    {
        fxBuffer.clear();
        fxBuffer.copyFrom (0, 0, buffer, 0, 0, numSamples);
        if (numCh > 1)
            fxBuffer.copyFrom (1, 0, buffer, 1, 0, numSamples);
        else
            fxBuffer.copyFrom (1, 0, buffer, 0, 0, numSamples);

        juce::dsp::AudioBlock<float> fxBlock (fxBuffer.getArrayOfWritePointers(), 2, (size_t) numSamples);
        juce::dsp::ProcessContextReplacing<float> context (fxBlock);

        chorus.process (context);
        phaser.process (context);

        const float maxDelay = (float) juce::jmax (1, delayL.getMaximumDelayInSamples() - 2);

        for (int i = 0; i < numSamples; ++i)
        {
            const float delaySamples = juce::jlimit (1.0f, maxDelay, delayTimeMs.getNextValue() * (float) sr / 1000.0f);
            const float fb = delayFb.getNextValue();
            const float dWet = delayWet.getNextValue();
            const float dDry = delayDry.getNextValue();

            const float inL = fxBuffer.getSample (0, i);
            const float inR = fxBuffer.getSample (1, i);

            const float delayedL = delayL.popSample (0, delaySamples, true);
            const float delayedR = delayR.popSample (0, delaySamples, true);

            const float fbL = delayDampL.coefficients != nullptr ? delayDampL.processSample (delayedL) : delayedL;
            const float fbR = delayDampR.coefficients != nullptr ? delayDampR.processSample (delayedR) : delayedR;

            delayL.pushSample (0, inL + fbL * fb);
            delayR.pushSample (0, inR + fbR * fb);

            fxBuffer.setSample (0, i, inL * dDry + delayedL * dWet);
            fxBuffer.setSample (1, i, inR * dDry + delayedR * dWet);
        }

        if (reverbWet > 0.001f)
            reverb.process (context);

        for (int i = 0; i < numSamples; ++i)
        {
            const float wet = fxWet.getNextValue();
            for (int ch = 0; ch < numCh; ++ch)
            {
                const float dry = buffer.getSample (ch, i);
                const float wetS = fxBuffer.getSample (juce::jmin (ch, 1), i);
                buffer.setSample (ch, i, dry + (wetS - dry) * wet);
            }
        }
    }
    else
    {
        fxWet.skip (numSamples);
        delayTimeMs.skip (numSamples); delayFb.skip (numSamples);
        delayWet.skip (numSamples);    delayDry.skip (numSamples);
    }

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
        if (numCh > 1)
            buffer.setSample (1, i, buffer.getSample (1, i) * pr * mg);
    }
}
