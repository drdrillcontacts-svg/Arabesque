#include "Arpeggiator.h"

ArabesqueArpeggiator::ArabesqueArpeggiator (juce::AudioProcessorValueTreeState& state)
    : apvts (state)
{
}

void ArabesqueArpeggiator::prepare (double sampleRate)
{
    sr = sampleRate > 0.0 ? sampleRate : 44100.0;
    heldNotes.reserve (128);
    sequence.reserve (128);
    pendingOffs.reserve (64);
    reset();
}

void ArabesqueArpeggiator::reset()
{
    pendingOffs.clear();
    currentNote = -1;
    sequenceIndex = 0;
    patternIndex = 0;
    countdown = 0.0;
    uiCurrentStep.store (-1);
    heldNotes.clear();
    absoluteSample = 0.0;
}

void ArabesqueArpeggiator::stopPlayingNote (juce::MidiBuffer& midi, int sampleOffset)
{
    if (currentNote >= 0)
    {
        midi.addEvent (juce::MidiMessage::noteOff (1, currentNote), sampleOffset);
        currentNote = -1;
    }
}

void ArabesqueArpeggiator::triggerNotes (juce::MidiBuffer& midi, int sampleOffset)
{
    if (heldNotes.empty())
    {
        stopPlayingNote (midi, sampleOffset);
        uiCurrentStep.store (-1);
        return;
    }

    const int numOctaves = juce::jlimit (1, 4, (int) apvts.getRawParameterValue (ParamIDs::arpOctave)->load());

    // build the full sequence: held notes ascending, repeated over octaves
    sequence.resize (heldNotes.size() * (size_t) numOctaves);
    size_t idx = 0;
    for (int oct = 0; oct < numOctaves; ++oct)
        for (const auto& h : heldNotes)
            sequence[idx++] = { juce::jlimit (0, 127, h.note + 12 * oct), h.velocity };

    // walk the circular 8-step gate pattern; rests advance the sequence
    bool fired = false;
    for (int tries = 0; tries < 8 && ! fired; ++tries)
    {
        const int step = patternIndex % 8;
        patternIndex++;

        const bool gateOn = apvts.getRawParameterValue (juce::String ("ARP_STEP") + juce::String (step + 1))->load() > 0.5f;
        uiCurrentStep.store (step);

        if (! gateOn)
            continue;

        const auto& n = sequence[(size_t) (sequenceIndex % (int) sequence.size())];
        sequenceIndex++;

        stopPlayingNote (midi, sampleOffset);
        midi.addEvent (juce::MidiMessage::noteOn (1, n.note, n.velocity), sampleOffset);
        currentNote = n.note;
        currentVelocity = n.velocity;
        fired = true;
    }
}

void ArabesqueArpeggiator::process (juce::MidiBuffer& midi, int numSamples, double bpm, double startPpq, bool isPlaying)
{
    if (sr <= 0.0)
        return;

    auto* onParam = apvts.getRawParameterValue (ParamIDs::arpOn);
    const bool enabled = onParam != nullptr && onParam->load() > 0.5f;

    if (! enabled)
    {
        // pass everything through; flush any dangling arp note
        if (currentNote >= 0)
            stopPlayingNote (midi, 0);
        return;
    }

    const int rateIndex = (int) apvts.getRawParameterValue (ParamIDs::arpRate)->load();
    const double stepBeats = Choices::divBeats (rateIndex);
    const double samplesPerBeat = sr * 60.0 / juce::jmax (20.0, bpm);
    const double stepSamples = stepBeats * samplesPerBeat;

    //------------------------------------------------------------ collect input
    juce::MidiBuffer generated;
    juce::MidiBuffer passthrough; // non-note messages go straight through

    for (const auto metadata : midi)
    {
        const auto msg = metadata.getMessage();
        if (msg.isNoteOn())
        {
            heldNotes.push_back ({ msg.getNoteNumber(), msg.getFloatVelocity() });
            std::sort (heldNotes.begin(), heldNotes.end(),
                       [] (const HeldNote& a, const HeldNote& b) { return a.note < b.note; });
        }
        else if (msg.isNoteOff())
        {
            const int n = msg.getNoteNumber();
            heldNotes.erase (std::remove_if (heldNotes.begin(), heldNotes.end(),
                                             [n] (const HeldNote& h) { return h.note == n; }),
                             heldNotes.end());
            if (n == currentNote && heldNotes.empty())
                stopPlayingNote (generated, metadata.samplePosition);
        }
        else
            passthrough.addEvent (msg, metadata.samplePosition);
    }

    //------------------------------------------------------------ timing
    // Host-synced path while transport is running, free-run otherwise.
    std::vector<int> triggerPoints;

    if (isPlaying && startPpq >= 0.0)
    {
        const double endPpq = startPpq + (double) numSamples / samplesPerBeat;
        double nextGrid = std::floor (startPpq / stepBeats) * stepBeats + stepBeats;
        // if we start almost exactly on a grid point, fire immediately
        if (std::abs (nextGrid - stepBeats - startPpq) < 1.0e-9)
            nextGrid = startPpq;

        while (nextGrid < endPpq - 1.0e-9)
        {
            const double offset = (nextGrid - startPpq) * samplesPerBeat;
            triggerPoints.push_back (juce::jlimit (0, numSamples - 1, (int) std::lround (offset)));
            nextGrid += stepBeats;
        }
    }
    else
    {
        countdown -= (double) numSamples;
        while (countdown < 0.0)
        {
            const int offset = juce::jlimit (0, numSamples - 1,
                                             numSamples + (int) std::floor (countdown));
            triggerPoints.push_back (offset);
            countdown += stepSamples;
        }
    }

    if (triggerPoints.empty() && currentNote < 0 && ! heldNotes.empty())
        triggerPoints.push_back (0); // just turned the arp on with notes held

    //------------------------------------------------------------ note offs (gate length 60% of a step)
    const double gateSamples = stepSamples * 0.6;

    for (const auto& off : pendingOffs)
    {
        if (off.dueSample <= absoluteSample + numSamples)
        {
            const int offset = juce::jlimit (0, numSamples - 1,
                                             (int) (off.dueSample - absoluteSample));
            generated.addEvent (juce::MidiMessage::noteOff (1, off.note), offset);
        }
    }
    pendingOffs.erase (std::remove_if (pendingOffs.begin(), pendingOffs.end(),
                                       [&] (const NoteOffEvent& e) { return e.dueSample <= absoluteSample + numSamples; }),
                       pendingOffs.end());

    //------------------------------------------------------------ fire new steps
    for (const int tp : triggerPoints)
    {
        if (heldNotes.empty()) { stopPlayingNote (generated, tp); uiCurrentStep.store (-1); continue; }
        triggerNotes (generated, tp);
        if (currentNote >= 0)
            pendingOffs.push_back ({ currentNote, absoluteSample + tp + gateSamples });
    }

    absoluteSample += (double) numSamples;

    midi.clear();
    midi.addEvents (generated, 0, numSamples, 0);
    midi.addEvents (passthrough, 0, numSamples, 0);
}
