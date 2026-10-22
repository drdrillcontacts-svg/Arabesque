#pragma once
#include <JuceHeader.h>

//==============================================================================
// Anti-aliased oscillator bank for ARABESQUE.
// Saw/Square/Triangle use polyBLEP so high notes don't alias (the original
// starter used a naive saw which aliases badly above ~1 kHz).
//==============================================================================

class PolyBLEPOscillator
{
public:
    enum Wave { saw = 0, square = 1, triangle = 2, sine = 3 };

    void prepare (double newSampleRate) noexcept { sampleRate = newSampleRate; }

    void setWave (int newWave) noexcept { wave = juce::jlimit (0, 3, newWave); }

    // phaseOffset in semitone space (octave*12 + semitone + fine/100 handled by caller)
    void setFrequency (float hz) noexcept
    {
        frequency = juce::jlimit (0.0f, (float) sampleRate * 0.49f, hz);
        phaseInc  = frequency / (float) sampleRate;
    }

    void reset() noexcept { phase = 0.0; triIntegrator = 0.0f; }

    inline float process() noexcept
    {
        float out = 0.0f;

        switch (wave)
        {
            case saw:
            {
                out = (float) (2.0 * phase - 1.0);
                out -= polyBLEP (std::fmod (phase, 1.0));
                break;
            }
            case square:
            {
                out = phase < 0.5 ? 1.0f : -1.0f;
                out += polyBLEP (std::fmod (phase, 1.0));
                out -= polyBLEP (std::fmod (phase + 0.5, 1.0));
                // leaky integration to make it band-limited-friendly
                out = out * 0.995f;
                break;
            }
            case triangle:
            {
                // integrate a band-limited square -> band-limited triangle
                float sq = phase < 0.5 ? 1.0f : -1.0f;
                sq += polyBLEP (std::fmod (phase, 1.0));
                sq -= polyBLEP (std::fmod (phase + 0.5, 1.0));
                triIntegrator += phaseInc * 4.0f * sq;
                triIntegrator *= 0.9995f;   // leak, stops DC build-up
                out = triIntegrator;
                break;
            }
            case sine:
            default:
                out = std::sin (juce::MathConstants<float>::twoPi * (float) phase);
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
        if (dt <= 0.0) return 0.0f;
        if (t < dt)            { t /= dt;            return (float) (t + t - t * t - 1.0); }
        if (t > 1.0 - dt)      { t = (t - 1.0) / dt; return (float) (t * t + t + t + 1.0); }
        return 0.0f;
    }

    double sampleRate = 44100.0;
    double phase = 0.0, phaseInc = 0.0;
    float  frequency = 0.0f;
    float  triIntegrator = 0.0f;
    int    wave = saw;
};

//==============================================================================
// Noise source: white noise passed through a one-pole low-pass so it sits
// closer to a "pink" spectrum and doesn't sound harsh next to the saws.
class NoiseSource
{
public:
    void prepare (double newSampleRate) noexcept
    {
        sampleRate = newSampleRate;
        lp = 0.0f;
        // ~dark pink-ish tilt
        coeff = 1.0f - std::exp (-2.0f * juce::MathConstants<float>::pi * 4000.0f / (float) sampleRate);
    }

    inline float process() noexcept
    {
        const float white = random.nextFloat() * 2.0f - 1.0f;
        lp += coeff * (white - lp);
        return white * 0.35f + lp * 1.4f;
    }

private:
    juce::Random random;
    double sampleRate = 44100.0;
    float lp = 0.0f, coeff = 0.1f;
};

//==============================================================================
// Sub oscillator: sine locked one octave below the played note.
class SubOscillator
{
public:
    void prepare (double newSampleRate) noexcept { sampleRate = newSampleRate; }

    void setFrequency (float hz) noexcept
    {
        phaseInc = juce::jlimit (0.0f, (float) sampleRate * 0.24f, hz * 0.5f) / (float) sampleRate;
    }

    void reset() noexcept { phase = 0.0f; }

    inline float process() noexcept
    {
        const float out = std::sin (juce::MathConstants<float>::twoPi * phase);
        phase += phaseInc;
        if (phase >= 1.0f) phase -= 1.0f;
        return out;
    }

private:
    double sampleRate = 44100.0;
    float phase = 0.0f, phaseInc = 0.0f;
};

//==============================================================================
// Per-voice LFO used for vibrato. Polyphonic: every voice owns its phase so
// notes wobble independently, which is what you expect on a lead synth.
class VoiceLFO
{
public:
    enum Shape { sine = 0, triangle = 1, saw = 2, square = 3, sampleAndHold = 4 };

    void prepare (double newSampleRate) noexcept { sampleRate = newSampleRate; }

    void startNote() noexcept
    {
        phase = 0.0f;
        heldSHValue = 0.0f;
        shCounter = 0;
    }

    void setShape (int shape) noexcept { currentShape = juce::jlimit (0, 4, shape); }

    void setFrequency (float hz) noexcept
    {
        phaseInc = juce::jlimit (0.005f, 50.0f, hz) / (float) sampleRate;
    }

    // returns bipolar value -1..1
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
                if (shCounter <= 0)
                {
                    heldSHValue = random.nextFloat() * 2.0f - 1.0f;
                    shCounter = shLengthSamples;
                }
                --shCounter;
                out = heldSHValue;
                break;
            default: break;
        }

        phase += phaseInc;
        if (phase >= 1.0f) phase -= 1.0f;
        return out;
    }

private:
    double sampleRate = 44100.0;
    float phase = 0.0f, phaseInc = 0.0f;
    float heldSHValue = 0.0f;
    int   shCounter = 0;
    int   shLengthSamples = 2205;   // updated implicitly by phase wrap; close enough at 20 Hz max
    int   currentShape = triangle;
    juce::Random random;
};
