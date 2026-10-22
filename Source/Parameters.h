#pragma once
#include <JuceHeader.h>

//==============================================================================
// ARABESQUE parameter system
// All parameter IDs, value formatting and the APVTS layout live here so that
// the processor, the editor and the preset system all share one definition.
//==============================================================================

namespace ParamIDs
{
    // Master (centre section)
    static constexpr const char* mainGain   = "MAIN_GAIN";
    static constexpr const char* drive      = "DRIVE";
    static constexpr const char* pan        = "PAN";
    static constexpr const char* vibrato    = "VIBRATO";
    static constexpr const char* glide      = "GLIDE";

    // Oscillators
    static constexpr const char* osc1Wave   = "OSC1_WAVE";
    static constexpr const char* osc1Octave = "OSC1_OCTAVE";
    static constexpr const char* osc1Semi   = "OSC1_SEMI";
    static constexpr const char* osc1Fine   = "OSC1_FINE";
    static constexpr const char* osc2Wave   = "OSC2_WAVE";
    static constexpr const char* osc2Octave = "OSC2_OCTAVE";
    static constexpr const char* osc2Semi   = "OSC2_SEMI";
    static constexpr const char* osc2Fine   = "OSC2_FINE";
    static constexpr const char* pwm        = "PWM";
    static constexpr const char* oscFM      = "OSC_FM";

    // Mixer (dB levels; the two OSC knobs mirror these)
    static constexpr const char* mixOsc1    = "MIX_OSC1";
    static constexpr const char* mixOsc2    = "MIX_OSC2";
    static constexpr const char* mixNoise   = "MIX_NOISE";
    static constexpr const char* mixSub     = "MIX_SUB";
    static constexpr const char* unison     = "UNISON";
    static constexpr const char* unisonDet  = "UNISON_DETUNE";
    static constexpr const char* unisonSpr  = "UNISON_SPREAD";

    // Filter
    static constexpr const char* filterType = "FILTER_TYPE";
    static constexpr const char* cutoff     = "CUTOFF";
    static constexpr const char* resonance  = "RESONANCE";
    static constexpr const char* keyFollow  = "KEY_FOLLOW";
    static constexpr const char* filterEnv  = "FILTER_ENV";
    static constexpr const char* filterDrive= "FILTER_DRIVE";

    // Envelopes: amp, filter, pitch
    static constexpr const char* ampAttack  = "ATTACK";
    static constexpr const char* ampDecay   = "DECAY";
    static constexpr const char* ampSustain = "SUSTAIN";
    static constexpr const char* ampRelease = "RELEASE";
    static constexpr const char* fltAttack  = "FLT_ATTACK";
    static constexpr const char* fltDecay   = "FLT_DECAY";
    static constexpr const char* fltSustain = "FLT_SUSTAIN";
    static constexpr const char* fltRelease = "FLT_RELEASE";
    static constexpr const char* pchAttack  = "PCH_ATTACK";
    static constexpr const char* pchDecay   = "PCH_DECAY";
    static constexpr const char* pchSustain = "PCH_SUSTAIN";
    static constexpr const char* pchRelease = "PCH_RELEASE";

    // LFO
    static constexpr const char* lfoShape    = "LFO_SHAPE";
    static constexpr const char* lfoRate     = "LFO_RATE";
    static constexpr const char* lfoSync     = "LFO_SYNC";
    static constexpr const char* lfoSyncRate = "LFO_SYNC_RATE";
    static constexpr const char* lfoDepth    = "LFO_DEPTH";

    // FX
    static constexpr const char* fxOn         = "FX_ON";
    static constexpr const char* chorusRate   = "CHORUS_RATE";
    static constexpr const char* chorusDepth  = "CHORUS_DEPTH";
    static constexpr const char* phaserRate   = "PHASER_RATE";
    static constexpr const char* phaserDepth  = "PHASER_DEPTH";
    static constexpr const char* delayTime    = "DELAY_TIME";
    static constexpr const char* delayFeedback= "DELAY_FEEDBACK";
    static constexpr const char* delayMix     = "DELAY_MIX";
    static constexpr const char* reverbType   = "REVERB_TYPE";
    static constexpr const char* reverbSize   = "REVERB_SIZE";
    static constexpr const char* reverbDecay  = "REVERB_DECAY";
    static constexpr const char* reverbMix    = "REVERB_MIX";

    // Arpeggiator
    static constexpr const char* arpOn        = "ARP_ON";
    static constexpr const char* arpRate      = "ARP_RATE";
    static constexpr const char* arpOctave    = "ARP_OCTAVE";
    static constexpr const char* arpStep1     = "ARP_STEP1";  // ... through ARP_STEP8

    // Polyphony
    static constexpr const char* voices       = "VOICES";
}

//==============================================================================
namespace Choices
{
    inline juce::StringArray waveforms()   { return { "Saw", "Square", "Triangle", "Sine" }; }
    inline juce::StringArray filterTypes() { return { "Low Pass 6 dB", "Low Pass 12 dB", "Low Pass 24 dB",
                                                      "High Pass 12 dB", "Band Pass 12 dB" }; }
    inline juce::StringArray lfoShapes()   { return { "Sine", "Triangle", "Saw", "Square", "S&H" }; }
    inline juce::StringArray reverbTypes() { return { "Hall", "Plate", "Room", "Cathedral" }; }
    inline juce::StringArray noteDivs()    { return { "1/2", "1/4", "1/4 T", "1/4 D", "1/8", "1/8 T",
                                                      "1/8 D", "1/16", "1/16 T", "1/16 D", "1/32", "1/32 T" }; }
    inline juce::StringArray voiceCounts() { return { "2", "4", "6", "8", "12", "16", "24", "32" }; }
    inline juce::StringArray unisonCounts() { return { "1", "2", "3", "5", "7" }; }
    inline int unisonFromIndex (int idx)
    {
        static constexpr int n[] = { 1, 2, 3, 5, 7 };
        return n[juce::jlimit (0, 4, idx)];
    }

    // Beat length of every note division above (quarter note = 1.0)
    inline double divBeats (int index)
    {
        static constexpr double beats[] = { 2.0, 1.0, 2.0 / 3.0, 1.5, 0.5, 1.0 / 3.0,
                                            0.75, 0.25, 1.0 / 6.0, 0.375, 0.125, 1.0 / 12.0 };
        return beats[juce::jlimit (0, (int) std::size (beats) - 1, index)];
    }
}

//==============================================================================
namespace ParamFormat
{
    inline juce::String hz (float v, int = 0)
    {
        if (v >= 1000.0f)  return juce::String (v / 1000.0f, 1) + " kHz";
        if (v >= 100.0f)   return juce::String ((int) std::round (v)) + " Hz";
        return juce::String (v, 1) + " Hz";
    }

    inline juce::String ms (float v, int = 0)
    {
        if (v < 1.0f)  return juce::String (v * 1000.0f, v < 0.1f ? 1 : 0) + " ms";
        return juce::String (v, 2) + " s";
    }

    inline juce::String percent (float v, int = 0) { return juce::String (v * 100.0f, 1) + " %"; }

    inline juce::String db (float v, int = 0)
    {
        if (v <= -59.5f) return "-inf dB";
        return juce::String (v, 1) + " dB";
    }

    inline juce::String cents (float v, int = 0)
    {
        const int c = (int) std::round (v);
        return (c > 0 ? "+" : "") + juce::String (c) + " ct";
    }

    inline juce::String signedSemitones (float v, int = 0)
    {
        const int s = (int) std::round (v);
        return (s > 0 ? "+" : "") + juce::String (s);
    }

    inline juce::String pan (float v, int = 0)
    {
        if (std::abs (v) < 0.01f) return "C";
        const int p = (int) std::round (std::abs (v) * 100.0f);
        return (v < 0.0f ? "L" : "R") + juce::String (p) + " %";
    }

    inline juce::String seconds (float v, int = 0) { return juce::String (v, 1) + " s"; }
    inline juce::String plain (float v, int = 0)   { return juce::String ((int) std::round (v)); }
}

//==============================================================================
inline juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    using namespace juce;
    using APF  = AudioParameterFloat;
    using APFAt= AudioParameterFloatAttributes;
    using APC  = AudioParameterChoice;
    using API  = AudioParameterInt;
    using APB  = AudioParameterBool;

    std::vector<std::unique_ptr<RangedAudioParameter>> p;

    auto pctRange  = [] { return NormalisableRange<float> (0.0f, 1.0f, 0.001f); };
    auto timeRange = [] (float lo, float hi) { return NormalisableRange<float> (lo, hi, 0.0001f, 0.35f); };
    auto gainRange = [] { return NormalisableRange<float> (-60.0f, 6.0f, 0.1f); };

    auto floatP = [&] (const juce::String& id, const juce::String& name, NormalisableRange<float> range, float def,
                       std::function<juce::String (float, int)> fmt = {})
    {
        auto atts = APFAt().withAutomatable (true);
        if (fmt) atts = atts.withStringFromValueFunction (fmt);
        p.push_back (std::make_unique<APF> (ParameterID { id, 1 }, name, range, def, atts));
    };

    auto choiceP = [&] (const juce::String& id, const juce::String& name, StringArray choices, int def)
    {
        p.push_back (std::make_unique<APC> (ParameterID { id, 1 }, name, choices, def));
    };

    auto intP = [&] (const juce::String& id, const juce::String& name, int lo, int hi, int def,
                     std::function<juce::String (float, int)> fmt = {})
    {
        auto atts = AudioParameterIntAttributes();
        if (fmt) atts = atts.withStringFromValueFunction ([fmt] (int v, int) { return fmt ((float) v, 0); });
        p.push_back (std::make_unique<API> (ParameterID { id, 1 }, name, lo, hi, def, atts));
    };

    auto boolP = [&] (const juce::String& id, const juce::String& name, bool def)
    {
        p.push_back (std::make_unique<APB> (ParameterID { id, 1 }, name, def));
    };

    //------------------------------------------------------------- Master ----
    floatP (ParamIDs::mainGain, "Main", gainRange(), -6.0f, ParamFormat::db);
    floatP (ParamIDs::drive,    "Drive", pctRange(), 0.08f, ParamFormat::percent);
    floatP (ParamIDs::pan,      "Pan", NormalisableRange<float> (-1.0f, 1.0f, 0.01f), 0.0f, ParamFormat::pan);
    floatP (ParamIDs::vibrato,  "Vibrato", pctRange(), 0.32f, ParamFormat::percent);
    floatP (ParamIDs::glide,    "Glide", NormalisableRange<float> (0.0f, 2.0f, 0.001f, 0.4f), 0.12f,
            [] (float v, int) { return v < 0.002f ? juce::String ("Off") : ParamFormat::ms (v, 0); });

    //----------------------------------------------------------- Oscillator --
    choiceP (ParamIDs::osc1Wave, "Osc 1 Waveform", Choices::waveforms(), 0);
    intP    (ParamIDs::osc1Octave, "Osc 1 Octave", -3, 3, 0, ParamFormat::plain);
    intP    (ParamIDs::osc1Semi,   "Osc 1 Semitone", -12, 12, 0, ParamFormat::signedSemitones);
    floatP  (ParamIDs::osc1Fine,   "Osc 1 Fine", NormalisableRange<float> (-100.0f, 100.0f, 1.0f), 0.0f, ParamFormat::cents);

    choiceP (ParamIDs::osc2Wave, "Osc 2 Waveform", Choices::waveforms(), 3);
    intP    (ParamIDs::osc2Octave, "Osc 2 Octave", -3, 3, 0, ParamFormat::plain);
    intP    (ParamIDs::osc2Semi,   "Osc 2 Semitone", -12, 12, 0, ParamFormat::signedSemitones);
    floatP  (ParamIDs::osc2Fine,   "Osc 2 Fine", NormalisableRange<float> (-100.0f, 100.0f, 1.0f), 6.0f, ParamFormat::cents);
    floatP  (ParamIDs::pwm,        "Pulse Width", pctRange(), 0.5f, ParamFormat::percent);
    floatP  (ParamIDs::oscFM,      "FM", pctRange(), 0.0f, ParamFormat::percent);

    //---------------------------------------------------------------- Mixer --
    floatP (ParamIDs::mixOsc1,  "Osc 1 Level", gainRange(), -3.0f, ParamFormat::db);
    floatP (ParamIDs::mixOsc2,  "Osc 2 Level", gainRange(), -6.0f, ParamFormat::db);
    floatP (ParamIDs::mixNoise, "Noise Level", gainRange(), -60.0f, ParamFormat::db);
    floatP (ParamIDs::mixSub,   "Sub Level",   gainRange(), -12.0f, ParamFormat::db);
    choiceP (ParamIDs::unison,    "Unison", Choices::unisonCounts(), 0);
    floatP  (ParamIDs::unisonDet, "Detune", NormalisableRange<float> (0.0f, 50.0f, 0.1f), 12.0f, ParamFormat::cents);
    floatP  (ParamIDs::unisonSpr, "Spread", pctRange(), 0.55f, ParamFormat::percent);

    //--------------------------------------------------------------- Filter --
    choiceP (ParamIDs::filterType, "Filter Type", Choices::filterTypes(), 2);
    floatP  (ParamIDs::cutoff,     "Cutoff", NormalisableRange<float> (20.0f, 20000.0f, 0.1f, 0.3f), 2800.0f, ParamFormat::hz);
    floatP  (ParamIDs::resonance,  "Resonance", pctRange(), 0.34f, ParamFormat::percent);
    floatP  (ParamIDs::keyFollow,  "Key Follow", pctRange(), 0.35f, ParamFormat::percent);
    floatP  (ParamIDs::filterEnv,  "Env Amt", pctRange(), 0.42f, ParamFormat::percent);
    floatP  (ParamIDs::filterDrive,"F Drive", pctRange(), 0.12f, ParamFormat::percent);

    //-------------------------------------------------------------- Envelopes
    floatP (ParamIDs::ampAttack,  "Amp Attack",   timeRange (0.001f, 2.0f),  0.005f, ParamFormat::ms);
    floatP (ParamIDs::ampDecay,   "Amp Decay",    timeRange (0.005f, 5.0f),  0.420f, ParamFormat::ms);
    floatP (ParamIDs::ampSustain, "Amp Sustain",  pctRange(),                0.72f,  ParamFormat::percent);
    floatP (ParamIDs::ampRelease, "Amp Release",  timeRange (0.005f, 10.0f), 0.680f, ParamFormat::ms);

    floatP (ParamIDs::fltAttack,  "Filter Attack",   timeRange (0.001f, 2.0f),  0.002f, ParamFormat::ms);
    floatP (ParamIDs::fltDecay,   "Filter Decay",    timeRange (0.005f, 5.0f),  0.250f, ParamFormat::ms);
    floatP (ParamIDs::fltSustain, "Filter Sustain",  pctRange(),                0.30f,  ParamFormat::percent);
    floatP (ParamIDs::fltRelease, "Filter Release",  timeRange (0.005f, 10.0f), 0.400f, ParamFormat::ms);

    floatP (ParamIDs::pchAttack,  "Pitch Attack",   timeRange (0.001f, 2.0f),  0.001f, ParamFormat::ms);
    floatP (ParamIDs::pchDecay,   "Pitch Decay",    timeRange (0.005f, 5.0f),  0.050f, ParamFormat::ms);
    floatP (ParamIDs::pchSustain, "Pitch Sustain",  pctRange(),                0.0f,   ParamFormat::percent);
    floatP (ParamIDs::pchRelease, "Pitch Release",  timeRange (0.005f, 10.0f), 0.100f, ParamFormat::ms);

    //------------------------------------------------------------------- LFO -
    choiceP (ParamIDs::lfoShape,    "LFO Shape", Choices::lfoShapes(), 1);
    floatP  (ParamIDs::lfoRate,     "LFO Rate", NormalisableRange<float> (0.01f, 20.0f, 0.001f, 0.35f), 4.8f, ParamFormat::hz);
    boolP   (ParamIDs::lfoSync,     "LFO Sync", false);
    choiceP (ParamIDs::lfoSyncRate, "LFO Sync Rate", Choices::noteDivs(), 4);
    floatP  (ParamIDs::lfoDepth,    "LFO Depth", pctRange(), 0.35f, ParamFormat::percent);

    //-------------------------------------------------------------------- FX -
    boolP   (ParamIDs::fxOn,        "FX On", true);
    floatP  (ParamIDs::chorusRate,  "Chorus Rate",  NormalisableRange<float> (0.05f, 5.0f, 0.01f, 0.4f), 1.2f, ParamFormat::hz);
    floatP  (ParamIDs::chorusDepth, "Chorus Depth", pctRange(), 0.40f, ParamFormat::percent);
    floatP  (ParamIDs::phaserRate,  "Phaser Rate",  NormalisableRange<float> (0.05f, 5.0f, 0.01f, 0.4f), 0.8f, ParamFormat::hz);
    floatP  (ParamIDs::phaserDepth, "Phaser Depth", pctRange(), 0.30f, ParamFormat::percent);

    choiceP (ParamIDs::delayTime,     "Delay Time", Choices::noteDivs(), 6);
    floatP  (ParamIDs::delayFeedback, "Delay Feedback", NormalisableRange<float> (0.0f, 0.95f, 0.01f), 0.28f,
             [] (float v, int) { return ParamFormat::percent (v, 0); });
    floatP  (ParamIDs::delayMix,      "Delay Mix", pctRange(), 0.22f, ParamFormat::percent);

    choiceP (ParamIDs::reverbType,  "Reverb Type", Choices::reverbTypes(), 0);
    floatP  (ParamIDs::reverbSize,  "Reverb Size",  pctRange(), 0.60f, ParamFormat::percent);
    floatP  (ParamIDs::reverbDecay, "Reverb Decay", NormalisableRange<float> (0.1f, 20.0f, 0.1f, 0.4f), 2.8f, ParamFormat::seconds);
    floatP  (ParamIDs::reverbMix,   "Reverb Mix",   pctRange(), 0.35f, ParamFormat::percent);

    //------------------------------------------------------------ Arpeggiator
    boolP   (ParamIDs::arpOn,   "Arp On", false);
    choiceP (ParamIDs::arpRate, "Arp Rate", Choices::noteDivs(), 7);
    intP    (ParamIDs::arpOctave, "Arp Octave", 1, 4, 1, ParamFormat::plain);
    for (int i = 0; i < 8; ++i)
        boolP ((juce::String ("ARP_STEP") + juce::String (i + 1)).getCharPointer(),
               ("Arp Step " + juce::String (i + 1)), true);

    //-------------------------------------------------------------- Polyphony
    choiceP (ParamIDs::voices, "Voices", Choices::voiceCounts(), 5);

    return { p.begin(), p.end() };
}
