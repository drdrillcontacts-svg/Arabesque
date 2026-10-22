#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "GUI/ArabesqueLookAndFeel.h"

//==============================================================================
class ArabesqueAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit ArabesqueAudioProcessorEditor (ArabesqueAudioProcessor&);
    ~ArabesqueAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    class Content;

    ArabesqueAudioProcessor& processor;
    std::unique_ptr<Content> content;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ArabesqueAudioProcessorEditor)
};
