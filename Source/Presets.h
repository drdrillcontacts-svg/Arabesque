#pragma once
#include <JuceHeader.h>
#include "Parameters.h"

//==============================================================================
// Factory presets. Each preset is a list of parameter overrides applied on top
// of init defaults, so adding a new parameter later stays backward compatible.
//==============================================================================

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

        add ("01 - Istanbul Nights", {
            { ParamIDs::osc1Wave, 0 }, { ParamIDs::osc2Wave, 3 }, { ParamIDs::mixNoise, -60.0f },
            { ParamIDs::mixSub, -12.0f }, { ParamIDs::cutoff, 2800.0f }, { ParamIDs::resonance, 0.34f },
            { ParamIDs::vibrato, 0.32f }, { ParamIDs::glide, 0.12f }, { ParamIDs::drive, 0.08f },
            { ParamIDs::reverbMix, 0.35f }, { ParamIDs::delayMix, 0.22f },
        });

        add ("02 - Bosphorus Lead", {
            { ParamIDs::osc1Wave, 0 }, { ParamIDs::osc2Wave, 0 }, { ParamIDs::osc2Fine, 9.0f },
            { ParamIDs::mixOsc2, -3.0f }, { ParamIDs::cutoff, 5200.0f }, { ParamIDs::resonance, 0.42f },
            { ParamIDs::ampAttack, 0.003f }, { ParamIDs::ampRelease, 0.35f }, { ParamIDs::glide, 0.09f },
            { ParamIDs::drive, 0.22f }, { ParamIDs::delayTime, 6.0f }, { ParamIDs::delayMix, 0.30f },
        });

        add ("03 - Anatolian Pluck", {
            { ParamIDs::osc1Wave, 1 }, { ParamIDs::mixOsc1, -4.0f }, { ParamIDs::mixOsc2, -8.0f },
            { ParamIDs::cutoff, 3800.0f }, { ParamIDs::resonance, 0.50f },
            { ParamIDs::ampDecay, 0.22f }, { ParamIDs::ampSustain, 0.05f }, { ParamIDs::ampRelease, 0.22f },
            { ParamIDs::fltDecay, 0.12f }, { ParamIDs::fltSustain, 0.02f },
            { ParamIDs::reverbSize, 0.45f }, { ParamIDs::reverbMix, 0.22f },
        });

        add ("04 - Saz Strings", {
            { ParamIDs::osc1Wave, 0 }, { ParamIDs::osc2Wave, 2 }, { ParamIDs::osc2Fine, -7.0f },
            { ParamIDs::cutoff, 3200.0f }, { ParamIDs::ampAttack, 0.012f },
            { ParamIDs::chorusDepth, 0.55f }, { ParamIDs::vibrato, 0.18f },
            { ParamIDs::reverbType, 3.0f }, { ParamIDs::reverbMix, 0.42f },
        });

        add ("05 - Harem Pad", {
            { ParamIDs::osc1Wave, 3 }, { ParamIDs::osc2Wave, 3 }, { ParamIDs::osc2Semi, 12.0f },
            { ParamIDs::mixOsc2, -9.0f }, { ParamIDs::cutoff, 1600.0f }, { ParamIDs::resonance, 0.15f },
            { ParamIDs::ampAttack, 0.85f }, { ParamIDs::ampRelease, 2.4f },
            { ParamIDs::chorusRate, 0.4f }, { ParamIDs::chorusDepth, 0.6f },
            { ParamIDs::reverbSize, 0.85f }, { ParamIDs::reverbDecay, 8.0f }, { ParamIDs::reverbMix, 0.5f },
        });

        add ("06 - Sultan Bass", {
            { ParamIDs::osc1Wave, 1 }, { ParamIDs::osc1Octave, -1.0f }, { ParamIDs::osc2Wave, 1 },
            { ParamIDs::osc2Octave, -1.0f }, { ParamIDs::mixSub, -6.0f },
            { ParamIDs::cutoff, 700.0f }, { ParamIDs::resonance, 0.25f },
            { ParamIDs::ampDecay, 0.30f }, { ParamIDs::ampSustain, 0.55f }, { ParamIDs::ampRelease, 0.12f },
            { ParamIDs::drive, 0.30f }, { ParamIDs::reverbMix, 0.05f }, { ParamIDs::delayMix, 0.0f },
        });

        add ("07 - Mystic Oud", {
            { ParamIDs::osc1Wave, 1 }, { ParamIDs::mixOsc2, -12.0f },
            { ParamIDs::cutoff, 2600.0f }, { ParamIDs::resonance, 0.38f },
            { ParamIDs::ampDecay, 0.55f }, { ParamIDs::ampSustain, 0.0f }, { ParamIDs::ampRelease, 0.4f },
            { ParamIDs::pchDecay, 0.03f }, { ParamIDs::reverbType, 2.0f }, { ParamIDs::reverbMix, 0.28f },
        });

        add ("08 - Dervish Trance", {
            { ParamIDs::osc1Wave, 0 }, { ParamIDs::osc2Wave, 0 }, { ParamIDs::osc2Semi, 7.0f },
            { ParamIDs::cutoff, 4200.0f }, { ParamIDs::resonance, 0.55f },
            { ParamIDs::ampSustain, 0.85f }, { ParamIDs::drive, 0.35f },
            { ParamIDs::delayTime, 3.0f }, { ParamIDs::delayFeedback, 0.5f }, { ParamIDs::delayMix, 0.35f },
            { ParamIDs::phaserDepth, 0.45f },
        });

        add ("09 - Cappadocia Flute", {
            { ParamIDs::osc1Wave, 3 }, { ParamIDs::osc2Wave, 2 }, { ParamIDs::mixOsc2, -15.0f },
            { ParamIDs::cutoff, 2400.0f }, { ParamIDs::vibrato, 0.5f }, { ParamIDs::glide, 0.16f },
            { ParamIDs::ampAttack, 0.06f }, { ParamIDs::lfoShape, 0.0f }, { ParamIDs::lfoRate, 5.4f },
            { ParamIDs::reverbType, 3.0f }, { ParamIDs::reverbMix, 0.45f },
        });

        add ("10 - Silk Road Bell", {
            { ParamIDs::osc1Wave, 2 }, { ParamIDs::osc2Wave, 3 }, { ParamIDs::osc2Semi, 12.0f },
            { ParamIDs::mixOsc2, -10.0f }, { ParamIDs::cutoff, 6000.0f },
            { ParamIDs::ampDecay, 1.8f }, { ParamIDs::ampSustain, 0.0f }, { ParamIDs::ampRelease, 1.8f },
            { ParamIDs::pchDecay, 0.02f }, { ParamIDs::reverbSize, 0.8f },
            { ParamIDs::reverbDecay, 6.0f }, { ParamIDs::reverbMix, 0.4f },
        });

        add ("11 - Marmara Keys", {
            { ParamIDs::osc1Wave, 1 }, { ParamIDs::osc2Wave, 3 }, { ParamIDs::mixOsc2, -7.0f },
            { ParamIDs::cutoff, 3400.0f }, { ParamIDs::ampDecay, 0.9f }, { ParamIDs::ampSustain, 0.3f },
            { ParamIDs::chorusDepth, 0.45f }, { ParamIDs::delayFeedback, 0.35f }, { ParamIDs::delayMix, 0.18f },
        });

        add ("12 - Galata Brass", {
            { ParamIDs::osc1Wave, 0 }, { ParamIDs::osc2Wave, 1 }, { ParamIDs::mixOsc2, -5.0f },
            { ParamIDs::cutoff, 2100.0f }, { ParamIDs::fltAttack, 0.18f }, { ParamIDs::fltSustain, 0.6f },
            { ParamIDs::ampAttack, 0.06f }, { ParamIDs::ampSustain, 0.9f },
            { ParamIDs::drive, 0.28f }, { ParamIDs::reverbMix, 0.3f },
        });

        add ("13 - Ottoman Drone", {
            { ParamIDs::osc1Wave, 0 }, { ParamIDs::osc1Octave, -1.0f }, { ParamIDs::osc2Octave, 1.0f },
            { ParamIDs::osc2Wave, 0 }, { ParamIDs::mixSub, -9.0f },
            { ParamIDs::cutoff, 900.0f }, { ParamIDs::resonance, 0.2f },
            { ParamIDs::ampAttack, 1.6f }, { ParamIDs::ampSustain, 1.0f }, { ParamIDs::ampRelease, 3.5f },
            { ParamIDs::ampDecay, 1.0f }, { ParamIDs::reverbDecay, 9.0f }, { ParamIDs::reverbMix, 0.55f },
        });

        add ("14 - Aegean Tears", {
            { ParamIDs::osc1Wave, 3 }, { ParamIDs::glide, 0.35f }, { ParamIDs::vibrato, 0.55f },
            { ParamIDs::cutoff, 1900.0f }, { ParamIDs::ampAttack, 0.2f }, { ParamIDs::ampRelease, 1.2f },
            { ParamIDs::delayTime, 4.0f }, { ParamIDs::delayFeedback, 0.45f }, { ParamIDs::delayMix, 0.3f },
            { ParamIDs::reverbType, 3.0f }, { ParamIDs::reverbMix, 0.5f },
        });

        add ("15 - Desert Caravan", {
            { ParamIDs::osc1Wave, 1 }, { ParamIDs::osc2Wave, 0 }, { ParamIDs::osc2Semi, 5.0f },
            { ParamIDs::mixOsc2, -11.0f }, { ParamIDs::cutoff, 4400.0f }, { ParamIDs::resonance, 0.3f },
            { ParamIDs::ampDecay, 0.4f }, { ParamIDs::ampSustain, 0.1f }, { ParamIDs::drive, 0.2f },
            { ParamIDs::delayFeedback, 0.4f }, { ParamIDs::delayMix, 0.25f }, { ParamIDs::pan, 0.0f },
        });

        add ("16 - Sultan's Call", {
            { ParamIDs::osc1Wave, 0 }, { ParamIDs::osc1Octave, 1.0f }, { ParamIDs::osc2Fine, -12.0f },
            { ParamIDs::cutoff, 6800.0f }, { ParamIDs::resonance, 0.45f },
            { ParamIDs::ampAttack, 0.002f }, { ParamIDs::ampRelease, 0.25f }, { ParamIDs::drive, 0.4f },
            { ParamIDs::delayTime, 6.0f }, { ParamIDs::delayMix, 0.28f }, { ParamIDs::reverbMix, 0.4f },
        });

        return list;
    }();
    return presets;
}
