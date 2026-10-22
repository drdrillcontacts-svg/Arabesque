#pragma once
#include <JuceHeader.h>

// One shared sound descriptor for every voice.
class ArabesqueSound final : public juce::SynthesiserSound
{
public:
    bool appliesToNote (int) override    { return true; }
    bool appliesToChannel (int) override { return true; }
};
