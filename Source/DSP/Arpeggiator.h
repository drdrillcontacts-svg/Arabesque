#pragma once
#include <JuceHeader.h>
#include "../Parameters.h"

//==============================================================================
// Tempo-synced arpeggiator with an 8-step gate pattern and octave cycling.
// When enabled it consumes the incoming note stream and replaces it with
// generated arpeggio notes. All other MIDI (CC, pitch wheel) passes through.
//==============================================================================

class ArabesqueArpeggiator
{
public:
    explicit ArabesqueArpeggiator (juce::AudioProcessorValueTreeState& state);

    void prepare (double sampleRate);
    void reset();

    // midi is rewritten in place. ppq < 0 means "no host position -> free run".
    void process (juce::MidiBuffer& midi, int numSamples, double bpm, double startPpq, bool isPlaying);

    int getCurrentStepForUI() const noexcept { return uiCurrentStep.load(); }

private:
    struct HeldNote
    {
        int note;
        float velocity;
    };

    struct NoteOffEvent
    {
        int note;
        double dueSample; // absolute sample counter
    };

    void triggerNotes (juce::MidiBuffer& midi, int sampleOffset);
    void stopPlayingNote (juce::MidiBuffer& midi, int sampleOffset);

    juce::AudioProcessorValueTreeState& apvts;
    double sr = 44100.0;
    double absoluteSample = 0.0;

    std::vector<HeldNote> heldNotes;
    std::vector<HeldNote> sequence;
    std::vector<NoteOffEvent> pendingOffs;
    int currentNote = -1;
    float currentVelocity = 0.0f;
    int sequenceIndex = 0;
    int patternIndex = 0;
    double countdown = 0.0; // samples until next step in free-run mode

    std::atomic<int> uiCurrentStep { -1 };

    JUCE_DECLARE_NON_COPYABLE (ArabesqueArpeggiator)
};
