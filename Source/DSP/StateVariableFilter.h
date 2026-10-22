#pragma once
#include <JuceHeader.h>

//==============================================================================
// TPT (Zavalishin, "The Art of VA Filter Design") state variable filter.
// Real resonance up to near self-oscillation - the starter's "resonance" knob
// was only a makeup gain on a one-pole filter, so this is one of the biggest
// sound upgrades.
//
// Modes mirror the UI: LP 6 / LP 12 / LP 24 / HP 12 / BP 12.  Stereo.
//==============================================================================

class TPTStateVariableFilter
{
public:
    enum class Mode { lp6 = 0, lp12, lp24, hp12, bp12 };

    void prepare (double newSampleRate)
    {
        sampleRate = newSampleRate;
        reset();
    }

    void reset()
    {
        for (int ch = 0; ch < 2; ++ch)
        {
            s1[ch] = s2[ch] = 0.0f;
            sB1[ch] = sB2[ch] = 0.0f;
            z1[ch] = 0.0f;
        }
    }

    void setMode (int m)  { mode = (Mode) juce::jlimit (0, 4, m); }
    void setMode (Mode m) { mode = m; }

    // resonance input 0..1 -> damping R (R = 1 means no resonance)
    void setCutoffAndResonance (float cutoffHz, float resonance01)
    {
        cutoffHz = juce::jlimit (10.0f, (float) sampleRate * 0.24f, cutoffHz);
        const double wd = juce::MathConstants<double>::twoPi * (double) cutoffHz;
        const double wa = 2.0 * sampleRate * std::tan (wd * 0.5 / sampleRate); // pre-warped
        g = (float) juce::jlimit (0.0, 0.98, wa / (2.0 * sampleRate));
        R = 1.0f - juce::jlimit (0.0f, 0.99f, resonance01) * 0.97f;
    }

    inline float process (int channel, float input)
    {
        switch (mode)
        {
            case Mode::lp6:  return processOnePole (channel, input);
            case Mode::lp12: return processStage12 (s1[channel], s2[channel], input).lp;
            case Mode::hp12: return processStage12 (s1[channel], s2[channel], input).hp;
            case Mode::bp12: return processStage12 (s1[channel], s2[channel], input).bp;
            case Mode::lp24:
            default:
            {
                const float first = processStage12  (s1[channel],  s2[channel],  input).lp;
                return processStage12 (sB1[channel], sB2[channel], first).lp;
            }
        }
    }

private:
    struct SvfOut { float lp, bp, hp; };

    inline SvfOut processStage12 (float& st1, float& st2, float in) const
    {
        // textbook TPT SVF (trapezoidal integrators), stable for any g < Nyquist
        const float hp = (in - (2.0f * R + g) * st1 - st2)
                       / (1.0f + 2.0f * R * g + g * g);
        const float v1 = g * hp;
        const float bp = v1 + st1;
        st1 = bp + v1;
        const float v2 = g * bp;
        const float lp = v2 + st2;
        st2 = lp + v2;
        return { lp, bp, hp };
    }

    inline float processOnePole (int ch, float in)
    {
        // Zavalishin 1-pole low pass
        const float g1 = g / (1.0f + g);
        const float v  = (in - z1[ch]) * g1;
        const float lp = v + z1[ch];
        z1[ch] = lp + v;
        return lp;
    }

    double sampleRate = 44100.0;
    Mode   mode       = Mode::lp24;
    float  g = 0.1f, R = 0.7f;
    float  s1[2]  { 0.0f, 0.0f },  s2[2]  { 0.0f, 0.0f };  // stage A
    float  sB1[2] { 0.0f, 0.0f },  sB2[2] { 0.0f, 0.0f };  // stage B (LP24 only)
    float  z1[2]  { 0.0f, 0.0f };                          // one-pole state
};
