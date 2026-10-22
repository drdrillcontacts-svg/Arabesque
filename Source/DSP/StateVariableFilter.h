#pragma once
#include <JuceHeader.h>

//==============================================================================
// TPT state-variable filter with optional input drive.
//==============================================================================
class TPTStateVariableFilter
{
public:
    enum class Mode { lp6 = 0, lp12, lp24, hp12, bp12 };

    void prepare (double newSampleRate)
    {
        sampleRate = newSampleRate > 0.0 ? newSampleRate : 44100.0;
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
    void setDrive (float d) { drive = juce::jlimit (0.0f, 1.0f, d); }

    void setCutoffAndResonance (float cutoffHz, float resonance01)
    {
        cutoffHz = juce::jlimit (20.0f, (float) sampleRate * 0.22f, cutoffHz);
        const double wd = juce::MathConstants<double>::twoPi * (double) cutoffHz;
        const double wa = 2.0 * sampleRate * std::tan (wd * 0.5 / sampleRate);
        g = (float) juce::jlimit (0.0, 0.98, wa / (2.0 * sampleRate));
        R = 1.0f - juce::jlimit (0.0f, 0.97f, resonance01) * 0.96f;
    }

    inline float process (int channel, float input)
    {
        channel = juce::jlimit (0, 1, channel);
        if (drive > 0.001f)
        {
            const float gDrive = 1.0f + drive * 6.0f;
            input = std::tanh (input * gDrive) / std::tanh (gDrive);
        }

        switch (mode)
        {
            case Mode::lp6:  return processOnePole (channel, input);
            case Mode::lp12: return processStage12 (s1[channel], s2[channel], input).lp;
            case Mode::hp12: return processStage12 (s1[channel], s2[channel], input).hp;
            case Mode::bp12: return processStage12 (s1[channel], s2[channel], input).bp;
            case Mode::lp24:
            default:
            {
                const float first = processStage12 (s1[channel],  s2[channel],  input).lp;
                return processStage12 (sB1[channel], sB2[channel], first).lp;
            }
        }
    }

private:
    struct SvfOut { float lp, bp, hp; };

    inline SvfOut processStage12 (float& st1, float& st2, float in) const
    {
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
        const float g1 = g / (1.0f + g);
        const float v  = (in - z1[ch]) * g1;
        const float lp = v + z1[ch];
        z1[ch] = lp + v;
        return lp;
    }

    double sampleRate = 44100.0;
    Mode   mode       = Mode::lp24;
    float  g = 0.1f, R = 0.7f, drive = 0.0f;
    float  s1[2]  { 0.0f, 0.0f },  s2[2]  { 0.0f, 0.0f };
    float  sB1[2] { 0.0f, 0.0f },  sB2[2] { 0.0f, 0.0f };
    float  z1[2]  { 0.0f, 0.0f };
};
