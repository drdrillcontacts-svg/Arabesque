#pragma once
#include <JuceHeader.h>
#include "Parameters.h"

struct ArabesquePreset
{
    const char* name;
    std::vector<std::pair<juce::String, float>> values;
};

inline const std::vector<ArabesquePreset>& getFactoryPresets()
{
    static const std::vector<ArabesquePreset> presets = []
    {
        std::vector<ArabesquePreset> list;
        auto add = [&] (const char* name, std::initializer_list<std::pair<const char*, float>> v)
        {
            ArabesquePreset ns { name, {} };
            for (auto& s : v) ns.values.emplace_back (juce::String (s.first), s.second);
            list.push_back (std::move (ns));
        };

        add ("Istanbul Nights", {
            { ParamIDs::osc1Wave, 0 }, { ParamIDs::osc2Wave, 3 }, { ParamIDs::osc2Fine, 7 },
            { ParamIDs::mixOsc1, -3.0f }, { ParamIDs::mixOsc2, -8.0f }, { ParamIDs::mixSub, -10.0f },
            { ParamIDs::unison, 2 }, { ParamIDs::unisonDet, 8.0f }, { ParamIDs::unisonSpr, 0.45f },
            { ParamIDs::cutoff, 2600.0f }, { ParamIDs::resonance, 0.32f }, { ParamIDs::filterEnv, 0.35f },
            { ParamIDs::vibrato, 0.38f }, { ParamIDs::glide, 0.14f }, { ParamIDs::drive, 0.10f },
            { ParamIDs::ampAttack, 0.008f }, { ParamIDs::ampDecay, 0.45f }, { ParamIDs::ampSustain, 0.7f }, { ParamIDs::ampRelease, 0.85f },
            { ParamIDs::reverbMix, 0.38f }, { ParamIDs::delayMix, 0.24f }, { ParamIDs::chorusDepth, 0.4f },
        });

        add ("Bosphorus Lead", {
            { ParamIDs::osc1Wave, 0 }, { ParamIDs::osc2Wave, 0 }, { ParamIDs::osc2Fine, 10.0f },
            { ParamIDs::mixOsc2, -4.0f }, { ParamIDs::unison, 3 }, { ParamIDs::unisonDet, 14.0f }, { ParamIDs::unisonSpr, 0.7f },
            { ParamIDs::cutoff, 5400.0f }, { ParamIDs::resonance, 0.28f }, { ParamIDs::filterEnv, 0.22f }, { ParamIDs::filterDrive, 0.18f },
            { ParamIDs::ampAttack, 0.003f }, { ParamIDs::ampRelease, 0.32f }, { ParamIDs::glide, 0.08f },
            { ParamIDs::drive, 0.22f }, { ParamIDs::delayTime, 6.0f }, { ParamIDs::delayMix, 0.28f }, { ParamIDs::reverbMix, 0.22f },
        });

        add ("Arabesk Cry", {
            { ParamIDs::osc1Wave, 0 }, { ParamIDs::osc2Wave, 2 }, { ParamIDs::osc2Semi, 7.0f }, { ParamIDs::mixOsc2, -9.0f },
            { ParamIDs::unison, 1 }, { ParamIDs::cutoff, 3200.0f }, { ParamIDs::resonance, 0.46f }, { ParamIDs::filterEnv, 0.55f },
            { ParamIDs::vibrato, 0.62f }, { ParamIDs::lfoRate, 5.6f }, { ParamIDs::lfoDepth, 0.55f }, { ParamIDs::glide, 0.28f },
            { ParamIDs::ampAttack, 0.04f }, { ParamIDs::ampRelease, 1.1f },
            { ParamIDs::delayTime, 4.0f }, { ParamIDs::delayFeedback, 0.42f }, { ParamIDs::delayMix, 0.36f },
            { ParamIDs::reverbType, 3.0f }, { ParamIDs::reverbMix, 0.48f }, { ParamIDs::reverbDecay, 6.5f },
        });

        add ("Golden Unison", {
            { ParamIDs::osc1Wave, 0 }, { ParamIDs::osc2Wave, 0 }, { ParamIDs::osc2Fine, -6.0f },
            { ParamIDs::mixOsc1, -4.0f }, { ParamIDs::mixOsc2, -6.0f },
            { ParamIDs::unison, 4 }, { ParamIDs::unisonDet, 18.0f }, { ParamIDs::unisonSpr, 0.85f },
            { ParamIDs::cutoff, 4800.0f }, { ParamIDs::resonance, 0.18f }, { ParamIDs::filterDrive, 0.2f },
            { ParamIDs::chorusDepth, 0.55f }, { ParamIDs::chorusRate, 0.35f },
            { ParamIDs::ampAttack, 0.02f }, { ParamIDs::ampSustain, 0.85f }, { ParamIDs::ampRelease, 0.6f },
            { ParamIDs::reverbMix, 0.28f }, { ParamIDs::drive, 0.16f },
        });

        add ("Ney Breath", {
            { ParamIDs::osc1Wave, 3 }, { ParamIDs::osc2Wave, 2 }, { ParamIDs::mixOsc2, -14.0f },
            { ParamIDs::mixNoise, -28.0f }, { ParamIDs::mixSub, -60.0f },
            { ParamIDs::cutoff, 2100.0f }, { ParamIDs::resonance, 0.22f }, { ParamIDs::filterEnv, 0.15f },
            { ParamIDs::vibrato, 0.55f }, { ParamIDs::lfoShape, 0.0f }, { ParamIDs::lfoRate, 5.2f }, { ParamIDs::lfoDepth, 0.48f },
            { ParamIDs::glide, 0.18f }, { ParamIDs::ampAttack, 0.08f }, { ParamIDs::ampRelease, 0.7f },
            { ParamIDs::reverbType, 3.0f }, { ParamIDs::reverbSize, 0.75f }, { ParamIDs::reverbMix, 0.5f }, { ParamIDs::delayMix, 0.12f },
        });

        add ("Electric Saz", {
            { ParamIDs::osc1Wave, 1 }, { ParamIDs::pwm, 0.38f }, { ParamIDs::osc2Wave, 1 }, { ParamIDs::osc2Fine, 5.0f },
            { ParamIDs::mixOsc2, -10.0f }, { ParamIDs::cutoff, 3600.0f }, { ParamIDs::resonance, 0.4f },
            { ParamIDs::filterEnv, 0.62f }, { ParamIDs::fltDecay, 0.18f }, { ParamIDs::fltSustain, 0.08f },
            { ParamIDs::ampDecay, 0.28f }, { ParamIDs::ampSustain, 0.12f }, { ParamIDs::ampRelease, 0.22f },
            { ParamIDs::pchDecay, 0.04f }, { ParamIDs::delayTime, 6.0f }, { ParamIDs::delayMix, 0.3f }, { ParamIDs::reverbMix, 0.2f },
        });

        add ("Sultan Solo", {
            { ParamIDs::osc1Wave, 0 }, { ParamIDs::osc2Wave, 3 }, { ParamIDs::osc2Octave, 1.0f }, { ParamIDs::mixOsc2, -12.0f },
            { ParamIDs::unison, 2 }, { ParamIDs::unisonDet, 6.0f },
            { ParamIDs::cutoff, 1800.0f }, { ParamIDs::resonance, 0.5f }, { ParamIDs::filterEnv, 0.7f },
            { ParamIDs::fltAttack, 0.01f }, { ParamIDs::fltDecay, 0.35f }, { ParamIDs::fltSustain, 0.25f },
            { ParamIDs::vibrato, 0.4f }, { ParamIDs::glide, 0.16f }, { ParamIDs::drive, 0.24f },
            { ParamIDs::delayMix, 0.22f }, { ParamIDs::reverbMix, 0.32f },
        });

        add ("Harem Pad", {
            { ParamIDs::osc1Wave, 3 }, { ParamIDs::osc2Wave, 3 }, { ParamIDs::osc2Semi, 12.0f }, { ParamIDs::mixOsc2, -8.0f },
            { ParamIDs::unison, 3 }, { ParamIDs::unisonDet, 10.0f }, { ParamIDs::unisonSpr, 0.8f },
            { ParamIDs::cutoff, 1400.0f }, { ParamIDs::resonance, 0.12f }, { ParamIDs::filterEnv, 0.2f },
            { ParamIDs::ampAttack, 1.1f }, { ParamIDs::ampDecay, 1.2f }, { ParamIDs::ampSustain, 0.8f }, { ParamIDs::ampRelease, 2.8f },
            { ParamIDs::chorusRate, 0.28f }, { ParamIDs::chorusDepth, 0.65f },
            { ParamIDs::reverbSize, 0.88f }, { ParamIDs::reverbDecay, 9.0f }, { ParamIDs::reverbMix, 0.55f }, { ParamIDs::delayMix, 0.1f },
        });

        add ("Cappadocia Clouds", {
            { ParamIDs::osc1Wave, 0 }, { ParamIDs::osc2Wave, 2 }, { ParamIDs::osc2Fine, -8.0f },
            { ParamIDs::unison, 4 }, { ParamIDs::unisonDet, 22.0f }, { ParamIDs::unisonSpr, 0.9f },
            { ParamIDs::cutoff, 900.0f }, { ParamIDs::filterEnv, 0.45f }, { ParamIDs::fltAttack, 0.8f }, { ParamIDs::fltDecay, 1.4f },
            { ParamIDs::ampAttack, 0.9f }, { ParamIDs::ampRelease, 3.2f }, { ParamIDs::ampSustain, 0.7f },
            { ParamIDs::chorusDepth, 0.5f }, { ParamIDs::phaserDepth, 0.35f }, { ParamIDs::reverbMix, 0.6f }, { ParamIDs::reverbDecay, 11.0f },
        });

        add ("Night Mosque", {
            { ParamIDs::osc1Wave, 0 }, { ParamIDs::osc1Octave, -1.0f }, { ParamIDs::osc2Wave, 3 }, { ParamIDs::mixOsc2, -6.0f },
            { ParamIDs::mixSub, -8.0f }, { ParamIDs::cutoff, 700.0f }, { ParamIDs::resonance, 0.38f }, { ParamIDs::filterDrive, 0.25f },
            { ParamIDs::ampAttack, 0.6f }, { ParamIDs::ampRelease, 2.4f },
            { ParamIDs::chorusDepth, 0.45f }, { ParamIDs::reverbType, 3.0f }, { ParamIDs::reverbMix, 0.5f }, { ParamIDs::delayMix, 0.18f },
        });

        add ("Anatolian Choir", {
            { ParamIDs::osc1Wave, 2 }, { ParamIDs::osc2Wave, 3 }, { ParamIDs::osc2Semi, 7.0f }, { ParamIDs::mixOsc2, -7.0f },
            { ParamIDs::unison, 3 }, { ParamIDs::unisonDet, 9.0f }, { ParamIDs::unisonSpr, 0.65f },
            { ParamIDs::cutoff, 2400.0f }, { ParamIDs::ampAttack, 0.35f }, { ParamIDs::ampRelease, 1.6f },
            { ParamIDs::chorusRate, 0.45f }, { ParamIDs::chorusDepth, 0.7f }, { ParamIDs::reverbMix, 0.42f },
        });

        add ("Ember Pad", {
            { ParamIDs::osc1Wave, 0 }, { ParamIDs::osc2Wave, 0 }, { ParamIDs::osc2Octave, 1.0f }, { ParamIDs::mixOsc2, -11.0f },
            { ParamIDs::unison, 2 }, { ParamIDs::cutoff, 1100.0f }, { ParamIDs::filterEnv, 0.5f }, { ParamIDs::fltAttack, 1.2f },
            { ParamIDs::ampAttack, 0.7f }, { ParamIDs::ampRelease, 2.2f }, { ParamIDs::drive, 0.2f },
            { ParamIDs::phaserRate, 0.15f }, { ParamIDs::phaserDepth, 0.4f }, { ParamIDs::reverbMix, 0.45f },
        });

        add ("Sultan Bass", {
            { ParamIDs::osc1Wave, 1 }, { ParamIDs::osc1Octave, -1.0f }, { ParamIDs::pwm, 0.42f },
            { ParamIDs::osc2Wave, 1 }, { ParamIDs::osc2Octave, -1.0f }, { ParamIDs::mixOsc2, -8.0f },
            { ParamIDs::mixSub, -4.0f }, { ParamIDs::unison, 1 },
            { ParamIDs::cutoff, 520.0f }, { ParamIDs::resonance, 0.3f }, { ParamIDs::filterEnv, 0.55f }, { ParamIDs::filterDrive, 0.35f },
            { ParamIDs::fltDecay, 0.22f }, { ParamIDs::fltSustain, 0.2f },
            { ParamIDs::ampDecay, 0.28f }, { ParamIDs::ampSustain, 0.6f }, { ParamIDs::ampRelease, 0.12f },
            { ParamIDs::drive, 0.32f }, { ParamIDs::reverbMix, 0.06f }, { ParamIDs::delayMix, 0.0f }, { ParamIDs::chorusDepth, 0.1f },
        });

        add ("Desert Sub", {
            { ParamIDs::osc1Wave, 3 }, { ParamIDs::osc1Octave, -1.0f }, { ParamIDs::mixOsc2, -60.0f },
            { ParamIDs::mixSub, -3.0f }, { ParamIDs::cutoff, 280.0f }, { ParamIDs::resonance, 0.15f }, { ParamIDs::filterEnv, 0.3f },
            { ParamIDs::ampAttack, 0.004f }, { ParamIDs::ampDecay, 0.4f }, { ParamIDs::ampSustain, 0.85f }, { ParamIDs::ampRelease, 0.18f },
            { ParamIDs::drive, 0.12f }, { ParamIDs::reverbMix, 0.04f }, { ParamIDs::delayMix, 0.0f }, { ParamIDs::fxOn, 0.0f },
        });

        add ("Growl Bass", {
            { ParamIDs::osc1Wave, 0 }, { ParamIDs::osc1Octave, -1.0f }, { ParamIDs::osc2Wave, 1 }, { ParamIDs::osc2Octave, -2.0f },
            { ParamIDs::oscFM, 0.35f }, { ParamIDs::mixOsc2, -6.0f }, { ParamIDs::mixSub, -9.0f },
            { ParamIDs::cutoff, 400.0f }, { ParamIDs::resonance, 0.55f }, { ParamIDs::filterEnv, 0.8f }, { ParamIDs::filterDrive, 0.45f },
            { ParamIDs::fltDecay, 0.16f }, { ParamIDs::fltSustain, 0.1f },
            { ParamIDs::ampSustain, 0.7f }, { ParamIDs::ampRelease, 0.15f }, { ParamIDs::drive, 0.4f }, { ParamIDs::reverbMix, 0.08f },
        });

        add ("Ottoman Pulse", {
            { ParamIDs::osc1Wave, 1 }, { ParamIDs::pwm, 0.28f }, { ParamIDs::osc1Octave, -1.0f },
            { ParamIDs::mixSub, -6.0f }, { ParamIDs::cutoff, 800.0f }, { ParamIDs::filterEnv, 0.65f },
            { ParamIDs::ampDecay, 0.2f }, { ParamIDs::ampSustain, 0.35f }, { ParamIDs::ampRelease, 0.1f },
            { ParamIDs::delayTime, 7.0f }, { ParamIDs::delayMix, 0.15f }, { ParamIDs::drive, 0.28f },
        });

        add ("Anatolian Pluck", {
            { ParamIDs::osc1Wave, 1 }, { ParamIDs::mixOsc1, -4.0f }, { ParamIDs::mixOsc2, -9.0f }, { ParamIDs::osc2Wave, 2 },
            { ParamIDs::cutoff, 3800.0f }, { ParamIDs::resonance, 0.48f }, { ParamIDs::filterEnv, 0.7f },
            { ParamIDs::ampDecay, 0.2f }, { ParamIDs::ampSustain, 0.05f }, { ParamIDs::ampRelease, 0.2f },
            { ParamIDs::fltDecay, 0.12f }, { ParamIDs::fltSustain, 0.02f },
            { ParamIDs::reverbSize, 0.45f }, { ParamIDs::reverbMix, 0.24f }, { ParamIDs::delayMix, 0.18f },
        });

        add ("Qanun Spark", {
            { ParamIDs::osc1Wave, 2 }, { ParamIDs::osc2Wave, 3 }, { ParamIDs::osc2Semi, 12.0f }, { ParamIDs::mixOsc2, -10.0f },
            { ParamIDs::cutoff, 6200.0f }, { ParamIDs::filterEnv, 0.4f },
            { ParamIDs::ampDecay, 1.4f }, { ParamIDs::ampSustain, 0.0f }, { ParamIDs::ampRelease, 1.2f },
            { ParamIDs::pchDecay, 0.025f }, { ParamIDs::reverbSize, 0.7f }, { ParamIDs::reverbDecay, 5.5f }, { ParamIDs::reverbMix, 0.4f },
        });

        add ("Silk Road Bell", {
            { ParamIDs::osc1Wave, 2 }, { ParamIDs::osc2Wave, 3 }, { ParamIDs::osc2Semi, 12.0f }, { ParamIDs::mixOsc2, -9.0f },
            { ParamIDs::cutoff, 7000.0f }, { ParamIDs::ampDecay, 1.8f }, { ParamIDs::ampSustain, 0.0f }, { ParamIDs::ampRelease, 1.8f },
            { ParamIDs::pchDecay, 0.02f }, { ParamIDs::reverbType, 1.0f }, { ParamIDs::reverbDecay, 6.0f }, { ParamIDs::reverbMix, 0.42f },
        });

        add ("Mystic Oud", {
            { ParamIDs::osc1Wave, 1 }, { ParamIDs::pwm, 0.45f }, { ParamIDs::mixOsc2, -14.0f },
            { ParamIDs::cutoff, 2400.0f }, { ParamIDs::resonance, 0.36f }, { ParamIDs::filterEnv, 0.5f },
            { ParamIDs::ampDecay, 0.5f }, { ParamIDs::ampSustain, 0.0f }, { ParamIDs::ampRelease, 0.4f },
            { ParamIDs::pchDecay, 0.03f }, { ParamIDs::reverbType, 2.0f }, { ParamIDs::reverbMix, 0.3f }, { ParamIDs::delayMix, 0.16f },
        });

        add ("Marmara Keys", {
            { ParamIDs::osc1Wave, 1 }, { ParamIDs::osc2Wave, 3 }, { ParamIDs::mixOsc2, -7.0f }, { ParamIDs::pwm, 0.5f },
            { ParamIDs::cutoff, 3400.0f }, { ParamIDs::ampDecay, 0.9f }, { ParamIDs::ampSustain, 0.28f }, { ParamIDs::ampRelease, 0.45f },
            { ParamIDs::chorusDepth, 0.4f }, { ParamIDs::delayFeedback, 0.3f }, { ParamIDs::delayMix, 0.16f }, { ParamIDs::reverbMix, 0.22f },
        });

        add ("Palace Organ", {
            { ParamIDs::osc1Wave, 1 }, { ParamIDs::osc2Wave, 1 }, { ParamIDs::osc2Octave, 1.0f }, { ParamIDs::mixOsc2, -8.0f },
            { ParamIDs::pwm, 0.5f }, { ParamIDs::cutoff, 2800.0f }, { ParamIDs::ampAttack, 0.04f }, { ParamIDs::ampSustain, 0.9f }, { ParamIDs::ampRelease, 0.35f },
            { ParamIDs::chorusDepth, 0.35f }, { ParamIDs::reverbMix, 0.38f }, { ParamIDs::reverbSize, 0.7f },
        });

        add ("Galata Brass", {
            { ParamIDs::osc1Wave, 0 }, { ParamIDs::osc2Wave, 1 }, { ParamIDs::mixOsc2, -5.0f }, { ParamIDs::unison, 2 }, { ParamIDs::unisonDet, 8.0f },
            { ParamIDs::cutoff, 1800.0f }, { ParamIDs::fltAttack, 0.16f }, { ParamIDs::fltSustain, 0.55f }, { ParamIDs::filterEnv, 0.5f },
            { ParamIDs::ampAttack, 0.07f }, { ParamIDs::ampSustain, 0.88f }, { ParamIDs::drive, 0.3f }, { ParamIDs::reverbMix, 0.28f },
        });

        add ("Dervish Trance", {
            { ParamIDs::osc1Wave, 0 }, { ParamIDs::osc2Wave, 0 }, { ParamIDs::osc2Semi, 7.0f }, { ParamIDs::unison, 2 },
            { ParamIDs::cutoff, 4200.0f }, { ParamIDs::resonance, 0.5f }, { ParamIDs::ampSustain, 0.8f }, { ParamIDs::drive, 0.3f },
            { ParamIDs::delayTime, 3.0f }, { ParamIDs::delayFeedback, 0.48f }, { ParamIDs::delayMix, 0.34f },
            { ParamIDs::phaserDepth, 0.42f }, { ParamIDs::arpOn, 1.0f }, { ParamIDs::arpRate, 7.0f }, { ParamIDs::arpOctave, 2.0f },
        });

        add ("Caravan Steps", {
            { ParamIDs::osc1Wave, 1 }, { ParamIDs::osc2Wave, 0 }, { ParamIDs::osc2Semi, 5.0f }, { ParamIDs::mixOsc2, -11.0f },
            { ParamIDs::cutoff, 4000.0f }, { ParamIDs::filterEnv, 0.6f },
            { ParamIDs::ampDecay, 0.22f }, { ParamIDs::ampSustain, 0.08f }, { ParamIDs::drive, 0.18f },
            { ParamIDs::delayMix, 0.22f }, { ParamIDs::arpOn, 1.0f }, { ParamIDs::arpRate, 4.0f }, { ParamIDs::arpOctave, 2.0f },
        });

        add ("Night Bazaar", {
            { ParamIDs::osc1Wave, 0 }, { ParamIDs::osc2Wave, 2 }, { ParamIDs::unison, 3 }, { ParamIDs::unisonDet, 12.0f },
            { ParamIDs::cutoff, 3000.0f }, { ParamIDs::ampSustain, 0.5f }, { ParamIDs::delayTime, 5.0f }, { ParamIDs::delayMix, 0.3f },
            { ParamIDs::reverbMix, 0.28f }, { ParamIDs::arpOn, 1.0f }, { ParamIDs::arpRate, 8.0f }, { ParamIDs::arpOctave, 3.0f },
        });

        add ("Aegean Tears", {
            { ParamIDs::osc1Wave, 3 }, { ParamIDs::glide, 0.4f }, { ParamIDs::vibrato, 0.58f }, { ParamIDs::lfoDepth, 0.5f },
            { ParamIDs::cutoff, 1700.0f }, { ParamIDs::ampAttack, 0.22f }, { ParamIDs::ampRelease, 1.4f },
            { ParamIDs::delayTime, 4.0f }, { ParamIDs::delayFeedback, 0.5f }, { ParamIDs::delayMix, 0.34f },
            { ParamIDs::reverbType, 3.0f }, { ParamIDs::reverbMix, 0.55f }, { ParamIDs::reverbDecay, 8.0f },
        });

        add ("Desert Wind", {
            { ParamIDs::osc1Wave, 3 }, { ParamIDs::mixOsc1, -8.0f }, { ParamIDs::mixNoise, -18.0f }, { ParamIDs::mixOsc2, -60.0f },
            { ParamIDs::cutoff, 1600.0f }, { ParamIDs::filterEnv, 0.25f }, { ParamIDs::ampAttack, 0.5f }, { ParamIDs::ampRelease, 2.0f },
            { ParamIDs::phaserDepth, 0.5f }, { ParamIDs::reverbMix, 0.6f }, { ParamIDs::reverbDecay, 10.0f },
        });

        add ("Black Sea Fog", {
            { ParamIDs::osc1Wave, 2 }, { ParamIDs::osc2Wave, 3 }, { ParamIDs::unison, 3 }, { ParamIDs::unisonSpr, 1.0f },
            { ParamIDs::cutoff, 600.0f }, { ParamIDs::ampAttack, 1.4f }, { ParamIDs::ampRelease, 3.5f },
            { ParamIDs::chorusDepth, 0.7f }, { ParamIDs::phaserRate, 0.08f }, { ParamIDs::phaserDepth, 0.55f },
            { ParamIDs::reverbMix, 0.62f }, { ParamIDs::reverbDecay, 12.0f },
        });

        add ("Ottoman Drone", {
            { ParamIDs::osc1Wave, 0 }, { ParamIDs::osc1Octave, -1.0f }, { ParamIDs::osc2Octave, 1.0f }, { ParamIDs::osc2Wave, 0 },
            { ParamIDs::mixSub, -7.0f }, { ParamIDs::unison, 2 }, { ParamIDs::unisonDet, 5.0f },
            { ParamIDs::cutoff, 850.0f }, { ParamIDs::resonance, 0.2f },
            { ParamIDs::ampAttack, 1.5f }, { ParamIDs::ampSustain, 1.0f }, { ParamIDs::ampRelease, 3.5f }, { ParamIDs::ampDecay, 1.0f },
            { ParamIDs::reverbDecay, 9.0f }, { ParamIDs::reverbMix, 0.52f },
        });

        add ("Spice Market", {
            { ParamIDs::osc1Wave, 0 }, { ParamIDs::osc2Wave, 1 }, { ParamIDs::mixNoise, -22.0f }, { ParamIDs::oscFM, 0.18f },
            { ParamIDs::cutoff, 2200.0f }, { ParamIDs::resonance, 0.45f }, { ParamIDs::filterEnv, 0.4f },
            { ParamIDs::ampDecay, 0.5f }, { ParamIDs::ampSustain, 0.4f }, { ParamIDs::drive, 0.22f },
            { ParamIDs::delayMix, 0.2f }, { ParamIDs::reverbMix, 0.3f },
        });

        add ("Topkapi Fanfare", {
            { ParamIDs::osc1Wave, 0 }, { ParamIDs::osc2Wave, 1 }, { ParamIDs::osc2Semi, 7.0f }, { ParamIDs::unison, 2 },
            { ParamIDs::cutoff, 2500.0f }, { ParamIDs::fltAttack, 0.12f }, { ParamIDs::filterEnv, 0.45f },
            { ParamIDs::ampAttack, 0.05f }, { ParamIDs::ampSustain, 0.85f }, { ParamIDs::drive, 0.26f },
            { ParamIDs::reverbMix, 0.3f }, { ParamIDs::chorusDepth, 0.25f },
        });

        add ("Whirling", {
            { ParamIDs::osc1Wave, 3 }, { ParamIDs::osc2Wave, 0 }, { ParamIDs::mixOsc2, -10.0f }, { ParamIDs::unison, 2 },
            { ParamIDs::cutoff, 2000.0f }, { ParamIDs::ampAttack, 0.4f }, { ParamIDs::ampRelease, 1.8f },
            { ParamIDs::phaserRate, 0.22f }, { ParamIDs::phaserDepth, 0.7f }, { ParamIDs::chorusDepth, 0.5f },
            { ParamIDs::reverbMix, 0.48f }, { ParamIDs::delayMix, 0.2f },
        });

        add ("Call to Evening", {
            { ParamIDs::osc1Wave, 0 }, { ParamIDs::osc1Octave, 1.0f }, { ParamIDs::osc2Fine, -11.0f }, { ParamIDs::unison, 2 },
            { ParamIDs::cutoff, 5000.0f }, { ParamIDs::resonance, 0.4f }, { ParamIDs::filterEnv, 0.3f },
            { ParamIDs::ampAttack, 0.003f }, { ParamIDs::ampRelease, 0.55f }, { ParamIDs::vibrato, 0.45f }, { ParamIDs::glide, 0.12f },
            { ParamIDs::drive, 0.28f }, { ParamIDs::delayTime, 6.0f }, { ParamIDs::delayMix, 0.32f }, { ParamIDs::reverbMix, 0.4f },
        });

        add ("Saz Strings", {
            { ParamIDs::osc1Wave, 0 }, { ParamIDs::osc2Wave, 2 }, { ParamIDs::osc2Fine, -7.0f }, { ParamIDs::unison, 2 }, { ParamIDs::unisonDet, 7.0f },
            { ParamIDs::cutoff, 3000.0f }, { ParamIDs::ampAttack, 0.015f }, { ParamIDs::ampRelease, 0.7f },
            { ParamIDs::chorusDepth, 0.5f }, { ParamIDs::vibrato, 0.2f }, { ParamIDs::reverbType, 3.0f }, { ParamIDs::reverbMix, 0.4f },
        });

        add ("Incense", {
            { ParamIDs::osc1Wave, 3 }, { ParamIDs::osc2Wave, 2 }, { ParamIDs::mixNoise, -26.0f }, { ParamIDs::unison, 2 },
            { ParamIDs::cutoff, 1200.0f }, { ParamIDs::ampAttack, 0.9f }, { ParamIDs::ampRelease, 2.6f },
            { ParamIDs::chorusDepth, 0.6f }, { ParamIDs::reverbMix, 0.58f }, { ParamIDs::reverbDecay, 8.5f }, { ParamIDs::delayMix, 0.14f },
        });

        add ("Kanun Cascade", {
            { ParamIDs::osc1Wave, 2 }, { ParamIDs::osc2Wave, 3 }, { ParamIDs::osc2Semi, 12.0f }, { ParamIDs::mixOsc2, -11.0f },
            { ParamIDs::cutoff, 5500.0f }, { ParamIDs::ampDecay, 0.35f }, { ParamIDs::ampSustain, 0.0f }, { ParamIDs::ampRelease, 0.4f },
            { ParamIDs::pchDecay, 0.03f }, { ParamIDs::delayTime, 7.0f }, { ParamIDs::delayFeedback, 0.4f }, { ParamIDs::delayMix, 0.28f },
            { ParamIDs::reverbMix, 0.3f }, { ParamIDs::arpOn, 1.0f }, { ParamIDs::arpRate, 7.0f }, { ParamIDs::arpOctave, 2.0f },
        });

        add ("Bosphorus Dawn", {
            { ParamIDs::osc1Wave, 3 }, { ParamIDs::osc2Wave, 0 }, { ParamIDs::osc2Octave, 1.0f }, { ParamIDs::mixOsc2, -12.0f },
            { ParamIDs::unison, 3 }, { ParamIDs::unisonSpr, 0.75f }, { ParamIDs::cutoff, 3500.0f },
            { ParamIDs::ampAttack, 0.8f }, { ParamIDs::ampRelease, 2.5f }, { ParamIDs::chorusDepth, 0.45f },
            { ParamIDs::reverbMix, 0.5f }, { ParamIDs::reverbDecay, 7.0f }, { ParamIDs::delayMix, 0.12f },
        });

        add ("Silk Lead", {
            { ParamIDs::osc1Wave, 3 }, { ParamIDs::osc2Wave, 2 }, { ParamIDs::mixOsc2, -9.0f }, { ParamIDs::osc2Fine, 4.0f },
            { ParamIDs::cutoff, 4000.0f }, { ParamIDs::vibrato, 0.28f }, { ParamIDs::glide, 0.1f },
            { ParamIDs::ampAttack, 0.02f }, { ParamIDs::ampRelease, 0.5f }, { ParamIDs::delayMix, 0.2f }, { ParamIDs::reverbMix, 0.3f },
        });

        return list;
    }();
    return presets;
}
