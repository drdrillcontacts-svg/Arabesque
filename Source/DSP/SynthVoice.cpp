#include "SynthVoice.h"

ArabesqueVoice::ArabesqueVoice (juce::AudioProcessorValueTreeState& state, VoiceSharedState& shared)
    : apvts (state), sharedState (shared)
{
    osc1Wave = gp (ParamIDs::osc1Wave);  osc1Oct = gp (ParamIDs::osc1Octave);
    osc1Semi = gp (ParamIDs::osc1Semi);  osc1Fine = gp (ParamIDs::osc1Fine);
    osc2Wave = gp (ParamIDs::osc2Wave);  osc2Oct = gp (ParamIDs::osc2Octave);
    osc2Semi = gp (ParamIDs::osc2Semi);  osc2Fine = gp (ParamIDs::osc2Fine);

    mixOsc1 = gp (ParamIDs::mixOsc1);    mixOsc2 = gp (ParamIDs::mixOsc2);
    mixNoise = gp (ParamIDs::mixNoise);  mixSub  = gp (ParamIDs::mixSub);
    pwm = gp (ParamIDs::pwm);            oscFM = gp (ParamIDs::oscFM);
    unisonN = gp (ParamIDs::unison);     unisonDet = gp (ParamIDs::unisonDet);
    unisonSpr = gp (ParamIDs::unisonSpr);

    filterType = gp (ParamIDs::filterType);
    cutoff = gp (ParamIDs::cutoff); resonance = gp (ParamIDs::resonance);
    keyFollow = gp (ParamIDs::keyFollow);
    filterEnvAmt = gp (ParamIDs::filterEnv);
    filterDrive  = gp (ParamIDs::filterDrive);

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

std::atomic<float>* ArabesqueVoice::gp (const char* id) noexcept
{
    if (auto* p = apvts.getRawParameterValue (id))
        return p;
    return &dummy;
}

void ArabesqueVoice::prepare (double sampleRate)
{
    sr = sampleRate > 0.0 ? sampleRate : 44100.0;
    for (int i = 0; i < kMaxUni; ++i)
    {
        osc1u[i].prepare (sr);
        osc2u[i].prepare (sr);
    }
    sub.prepare (sr);
    noise.prepare (sr);
    lfo.prepare (sr);
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
    return dynamic_cast<ArabesqueSound*> (sound) != nullptr;
}

float ArabesqueVoice::pitchRatioFromSemitones (float semis) noexcept
{
    return std::pow (2.0f, semis / 12.0f);
}

void ArabesqueVoice::startNote (int midiNote, float velocity, juce::SynthesiserSound*, int)
{
    currentNoteNumber = midiNote;

    const double newPitch = (double) midiNote;
    if (glide->load() > 0.001f && currentPitchSemis > 1.0)
        targetPitchSemis = newPitch;
    else
        currentPitchSemis = targetPitchSemis = newPitch;

    noteVelocity = juce::jlimit (0.0f, 1.0f, velocity);
    driftCents = (rng.nextFloat() - 0.5f) * 4.0f;

    for (int i = 0; i < kMaxUni; ++i)
    {
        osc1u[i].reset();
        osc2u[i].reset();
        osc1u[i].randomisePhase (rng);
        osc2u[i].randomisePhase (rng);
    }
    sub.reset();
    sub.randomisePhase (rng);
    lfo.startNote();
    filter.reset();
    dcX1 = dcY1 = 0.0f;

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
    sharedState.bend.store ((newPitchWheelValue - 8192) / 8192.0f * 2.0f);
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
        lfo.setFrequency ((float) (bpm / 60.0 / juce::jmax (0.05, beats)));
    }
    else
        lfo.setFrequency (lfoRate->load());

    const float vibDepthSemis = (vibAmount * 1.15f + modWheel * 0.7f) * lfoDepthAmt * 1.6f;

    const int   fType     = (int) filterType->load();
    const float keyTrack  = keyFollow->load();
    const float reso      = resonance->load();
    const float baseCut   = cutoff->load();
    const float envAmt    = filterEnvAmt->load();
    filter.setDrive (filterDrive->load());
    filter.setMode (fType);

    const float velGain   = 0.22f + 0.78f * noteVelocity * noteVelocity; // slightly exponential

    auto dbToG = [] (float db) {
        return db <= -59.5f ? 0.0f : juce::Decibels::decibelsToGain (db);
    };
    smoothedMix[0].setTargetValue (dbToG (mixOsc1->load()));
    smoothedMix[1].setTargetValue (dbToG (mixOsc2->load()));
    smoothedMix[2].setTargetValue (dbToG (mixNoise->load()));
    smoothedMix[3].setTargetValue (dbToG (mixSub->load()));

    const int uniCount = Choices::unisonFromIndex ((int) unisonN->load());
    const float detCents = unisonDet->load();
    const float spread = unisonSpr->load();
    const float pw = pwm->load();
    const float fmAmt = oscFM->load();
    const int w1 = (int) osc1Wave->load();
    const int w2 = (int) osc2Wave->load();

    for (int u = 0; u < uniCount; ++u)
    {
        osc1u[u].setWave (w1);
        osc2u[u].setWave (w2);
        osc1u[u].setPulseWidth (pw);
        osc2u[u].setPulseWidth (pw);
    }

    smoothedCutoff.setTargetValue (baseCut);

    const float uniNorm = 1.0f / std::sqrt ((float) uniCount);
    const float voiceScale = 0.38f * uniNorm;

    for (int i = 0; i < numSamples; ++i)
    {
        currentPitchSemis += (targetPitchSemis - currentPitchSemis) * glideCoef;
        driftCents += ((rng.nextFloat() - 0.5f) * 0.04f);
        driftCents = juce::jlimit (-6.0f, 6.0f, driftCents);

        const float pitchEnvSemis = pitchEnv.getNextSample() * 12.0f;
        const float vibSemis = lfo.process() * vibDepthSemis;
        const double noteSemis = currentPitchSemis + pitchBendSemi + pitchEnvSemis
                               + vibSemis + driftCents * 0.01;

        const float o1base = (float) noteSemis
                           + osc1Oct->load() * 12.0f + osc1Semi->load() + osc1Fine->load() / 100.0f;
        const float o2base = (float) noteSemis
                           + osc2Oct->load() * 12.0f + osc2Semi->load() + osc2Fine->load() / 100.0f;

        const float g1 = smoothedMix[0].getNextValue();
        const float g2 = smoothedMix[1].getNextValue();
        const float gn = smoothedMix[2].getNextValue();
        const float gs = smoothedMix[3].getNextValue();

        float mixL = 0.0f, mixR = 0.0f;

        // Osc 2 first so we can FM osc 1
        float fmSample = 0.0f;
        for (int u = 0; u < uniCount; ++u)
        {
            const float t = uniCount == 1 ? 0.0f : ((float) u / (float) (uniCount - 1)) * 2.0f - 1.0f;
            const float det = t * detCents * 0.01f; // cents -> semitones / 100 already? wait
            // detCents is 0..50 cents, t is -1..1 → ±detCents cents = ±detCents/100 semitones
            const float detSemi = t * detCents / 100.0f;
            const float hz = (float) (440.0 * pitchRatioFromSemitones (o2base + detSemi - 69.0f));
            osc2u[u].setFrequency (hz);
            const float s = osc2u[u].process();
            if (u == 0) fmSample = s;
            const float pan = t * spread;
            const float pl = std::sqrt (0.5f * (1.0f - pan));
            const float pr = std::sqrt (0.5f * (1.0f + pan));
            mixL += s * g2 * pl;
            mixR += s * g2 * pr;
        }

        const float phaseMod = fmSample * fmAmt * 0.35f;

        for (int u = 0; u < uniCount; ++u)
        {
            const float t = uniCount == 1 ? 0.0f : ((float) u / (float) (uniCount - 1)) * 2.0f - 1.0f;
            const float detSemi = t * detCents / 100.0f;
            const float hz = (float) (440.0 * pitchRatioFromSemitones (o1base + detSemi - 69.0f));
            osc1u[u].setFrequency (hz);
            const float s = osc1u[u].process (phaseMod);
            const float pan = t * spread;
            const float pl = std::sqrt (0.5f * (1.0f - pan));
            const float pr = std::sqrt (0.5f * (1.0f + pan));
            mixL += s * g1 * pl;
            mixR += s * g1 * pr;
        }

        sub.setFrequency ((float) (440.0 * pitchRatioFromSemitones ((float) noteSemis - 69.0f)));
        const float subS = sub.process() * gs;
        mixL += subS;
        mixR += subS;

        const float nz = noise.process() * gn;
        mixL += nz;
        mixR += nz;

        mixL *= voiceScale;
        mixR *= voiceScale;

        const float envMod = filterEnv.getNextSample();
        const float keyRatio = pitchRatioFromSemitones ((float) (currentPitchSemis - 60.0) * keyTrack);
        const float fEnvRatio = std::pow (2.0f, envMod * envAmt * 4.0f);
        const float fCut = smoothedCutoff.getNextValue() * keyRatio * fEnvRatio;
        filter.setCutoffAndResonance (fCut, reso);

        float outL = filter.process (0, mixL);
        float outR = filter.process (1, mixR);

        const float env = ampEnv.getNextSample() * velGain;
        outL *= env;
        outR *= env;

        // DC blocker
        const float nx = (outL + outR) * 0.5f;
        const float ny = nx - dcX1 + 0.995f * dcY1;
        dcX1 = nx; dcY1 = ny;
        juce::ignoreUnused (ny);

        if (output.getNumChannels() == 1)
            output.addSample (0, start + i, (outL + outR) * 0.5f);
        else
        {
            output.addSample (0, start + i, outL);
            if (output.getNumChannels() > 1)
                output.addSample (1, start + i, outR);
        }
    }

    if (! ampEnv.isActive())
        clearCurrentNote();
}
