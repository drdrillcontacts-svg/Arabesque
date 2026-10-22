#pragma once
#include <JuceHeader.h>

//==============================================================================
// Band-limited oscillator (PolyBLEP saw/square, naive triangle, sine).
// PWM applies to square. phaseMod is for FM (0..1 cycles).
//==============================================================================
class PolyBLEPOscillator
{
public:
    enum Wave { saw = 0, square = 1, triangle = 2, sine = 3 };

    void prepare (double newSampleRate) noexcept
    {
        sampleRate = newSampleRate > 0.0 ? newSampleRate : 44100.0;
    }

    void setWave (int newWave) noexcept { wave = juce::jlimit (0, 3, newWave); }
    void setPulseWidth (float pw) noexcept { pulseWidth = juce::jlimit (0.05f, 0.95f, pw); }

    void setFrequency (float hz) noexcept
    {
        frequency = juce::jlimit (0.0f, (float) sampleRate * 0.49f, hz);
        phaseInc  = (double) frequency / sampleRate;
    }

    void reset() noexcept { phase = 0.0; triIntegrator = 0.0f; }
    void setPhase (double p) noexcept { phase = p - std::floor (p); }
    void randomisePhase (juce::Random& rng) noexcept { phase = rng.nextDouble(); }

    inline float process (float phaseMod = 0.0f) noexcept
    {
        double p = phase + (double) phaseMod;
        p -= std::floor (p);

        float out = 0.0f;
        switch (wave)
        {
            case saw:
                out = (float) (2.0 * p - 1.0);
                out -= polyBLEP (p);
                break;

            case square:
            {
                const double pw = (double) pulseWidth;
                out = p < pw ? 1.0f : -1.0f;
                out += polyBLEP (p);
                out -= polyBLEP (p - pw);
                break;
            }

            case triangle:
            {
                // Integrate a band-limited square; scale so amplitude stays ~1
                float sq = p < 0.5 ? 1.0f : -1.0f;
                sq += polyBLEP (p);
                sq -= polyBLEP (std::fmod (p + 0.5, 1.0));
                triIntegrator += (float) (phaseInc * 4.0) * sq;
                triIntegrator = juce::jlimit (-1.5f, 1.5f, triIntegrator * 0.9994f);
                out = triIntegrator;
                break;
            }

            case sine:
            default:
                out = std::sin (juce::MathConstants<float>::twoPi * (float) p);
                break;
        }

        phase += phaseInc;
        if (phase >= 1.0) phase -= 1.0;
        return out;
    }

private:
    inline float polyBLEP (double t) const noexcept
    {
        const double dt = phaseInc;
        if (dt <= 1.0e-9) return 0.0f;
        t -= std::floor (t);
        if (t < dt)            { t /= dt;            return (float) (t + t - t * t - 1.0); }
        if (t > 1.0 - dt)      { t = (t - 1.0) / dt; return (float) (t * t + t + t + 1.0); }
        return 0.0f;
    }

    double sampleRate = 44100.0;
    double phase = 0.0, phaseInc = 0.0;
    float  frequency = 0.0f;
    float  triIntegrator = 0.0f;
    float  pulseWidth = 0.5f;
    int    wave = saw;
};

//==============================================================================
class NoiseSource
{
public:
    void prepare (double newSampleRate) noexcept
    {
        sampleRate = newSampleRate;
        lp = 0.0f;
        coeff = 1.0f - std::exp (-2.0f * juce::MathConstants<float>::pi * 2800.0f / (float) sampleRate);
    }

    inline float process() noexcept
    {
        const float white = random.nextFloat() * 2.0f - 1.0f;
        lp += coeff * (white - lp);
        return white * 0.25f + lp * 1.15f;
    }

private:
    juce::Random random { 1 };
    double sampleRate = 44100.0;
    float lp = 0.0f, coeff = 0.1f;
};

//==============================================================================
class SubOscillator
{
public:
    void prepare (double newSampleRate) noexcept { sampleRate = newSampleRate; }

    void setFrequency (float hz) noexcept
    {
        phaseInc = juce::jlimit (0.0f, (float) sampleRate * 0.24f, hz * 0.5f) / (float) sampleRate;
    }

    void reset() noexcept { phase = 0.0f; }
    void randomisePhase (juce::Random& rng) noexcept { phase = rng.nextFloat(); }

    inline float process() noexcept
    {
        // Slightly rounded sine (a touch of 3rd harmonic) so the sub has body
        const float s = std::sin (juce::MathConstants<float>::twoPi * phase);
        const float out = s + 0.12f * s * s * s;
        phase += phaseInc;
        if (phase >= 1.0f) phase -= 1.0f;
        return out;
    }

private:
    double sampleRate = 44100.0;
    float phase = 0.0f, phaseInc = 0.0f;
};

//==============================================================================
class VoiceLFO
{
public:
    enum Shape { sine = 0, triangle = 1, saw = 2, square = 3, sampleAndHold = 4 };

    void prepare (double newSampleRate) noexcept { sampleRate = newSampleRate; }

    void startNote() noexcept
    {
        phase = 0.0f;
        heldSHValue = 0.0f;
    }

    void setShape (int shape) noexcept { currentShape = juce::jlimit (0, 4, shape); }

    void setFrequency (float hz) noexcept
    {
        phaseInc = juce::jlimit (0.005f, 50.0f, hz) / (float) sampleRate;
    }

    inline float process() noexcept
    {
        float out = 0.0f;
        switch (currentShape)
        {
            case sine:     out = std::sin (juce::MathConstants<float>::twoPi * phase); break;
            case triangle: out = phase < 0.25f ? phase * 4.0f
                             : phase < 0.75f ? 2.0f - phase * 4.0f
                                             : phase * 4.0f - 4.0f; break;
            case saw:      out = phase * 2.0f - 1.0f; break;
            case square:   out = phase < 0.5f ? 1.0f : -1.0f; break;
            case sampleAndHold:
                out = heldSHValue;
                break;
            default: break;
        }

        phase += phaseInc;
        if (phase >= 1.0f)
        {
            phase -= 1.0f;
            if (currentShape == sampleAndHold)
                heldSHValue = random.nextFloat() * 2.0f - 1.0f;
        }
        return out;
    }

private:
    double sampleRate = 44100.0;
    float phase = 0.0f, phaseInc = 0.0f;
    float heldSHValue = 0.0f;
    int   currentShape = triangle;
    juce::Random random { 2 };
};
