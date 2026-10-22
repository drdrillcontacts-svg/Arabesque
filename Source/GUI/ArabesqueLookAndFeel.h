#pragma once
#include <JuceHeader.h>

//==============================================================================
namespace ArabesqueColours
{
    const juce::Colour background      { 0xff10100e };
    const juce::Colour headerFill      { 0xff161410 };
    const juce::Colour panelFill       { 0xff17150f };
    const juce::Colour panelFillLight  { 0xff1c1913 };
    const juce::Colour panelBorder     { 0xff5a4a32 };
    const juce::Colour gold            { 0xffc4a36a };
    const juce::Colour goldBright      { 0xffe0c48a };
    const juce::Colour copper          { 0xff9a6a42 };
    const juce::Colour orangeGlow      { 0xffc47a3a };
    const juce::Colour redGlow         { 0xffc44a32 };
    const juce::Colour textWarm        { 0xffe8dcc8 };
    const juce::Colour textDim         { 0xff8e8476 };
    const juce::Colour textCaption     { 0xff7a7064 };
    const juce::Colour knobBody        { 0xff1a1814 };
    const juce::Colour arcTrack        { 0xff322c24 };
    const juce::Colour keyWhite        { 0xffe8dcc8 };
}

//==============================================================================
class ArabesqueLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    ArabesqueLookAndFeel()
    {
        setColour (juce::ComboBox::backgroundColourId,          ArabesqueColours::panelFillLight);
        setColour (juce::ComboBox::textColourId,                ArabesqueColours::textWarm);
        setColour (juce::ComboBox::outlineColourId,             ArabesqueColours::panelBorder);
        setColour (juce::ComboBox::arrowColourId,               ArabesqueColours::gold);
        setColour (juce::PopupMenu::backgroundColourId,         juce::Colour (0xff1a1712));
        setColour (juce::PopupMenu::textColourId,               ArabesqueColours::textWarm);
        setColour (juce::PopupMenu::highlightedBackgroundColourId, juce::Colour (0xff3a2c18));
        setColour (juce::PopupMenu::highlightedTextColourId,    ArabesqueColours::goldBright);
        setColour (juce::TextButton::buttonColourId,            juce::Colour (0xff1c1812));
        setColour (juce::TextButton::buttonOnColourId,          juce::Colour (0xff3a2a16));
        setColour (juce::TextButton::textColourOffId,           ArabesqueColours::textDim);
        setColour (juce::TextButton::textColourOnId,            ArabesqueColours::goldBright);
        setColour (juce::Label::textColourId,                   ArabesqueColours::textWarm);
        setColour (juce::Label::backgroundColourId,             juce::Colours::transparentBlack);
        setColour (juce::Label::outlineColourId,                juce::Colours::transparentBlack);
        setColour (juce::Slider::trackColourId,                 ArabesqueColours::gold);
        setColour (juce::Slider::backgroundColourId,            juce::Colour (0xff161410));
        setColour (juce::Slider::thumbColourId,                 ArabesqueColours::goldBright);
        setColour (juce::MidiKeyboardComponent::whiteNoteColourId,              ArabesqueColours::keyWhite);
        setColour (juce::MidiKeyboardComponent::blackNoteColourId,              juce::Colour (0xff161410));
        setColour (juce::MidiKeyboardComponent::keySeparatorLineColourId,       juce::Colour (0xff6a5a40));
        setColour (juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId,    juce::Colour (0x55c4a36a));
        setColour (juce::MidiKeyboardComponent::keyDownOverlayColourId,         juce::Colour (0x99c47a3a));
        setColour (juce::MidiKeyboardComponent::textLabelColourId,              juce::Colours::transparentBlack);
        setColour (juce::MidiKeyboardComponent::upDownButtonBackgroundColourId, juce::Colours::transparentBlack);
        setColour (juce::MidiKeyboardComponent::upDownButtonArrowColourId,      ArabesqueColours::gold);
        setColour (juce::MidiKeyboardComponent::shadowColourId,                 juce::Colours::transparentBlack);
    }

    static juce::Font uiFont (float size, bool bold = false)
    {
        juce::ignoreUnused (bold);
        return juce::Font (juce::FontOptions (size));
    }

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                           juce::Slider&) override
    {
        auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (2.0f);
        const float radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
        const float cx = bounds.getCentreX(), cy = bounds.getCentreY();
        const float endAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        const bool big = radius > 32.0f;

        g.setColour (juce::Colour (0xff141210));
        g.fillEllipse (cx - radius, cy - radius, radius * 2.0f, radius * 2.0f);
        g.setColour (juce::Colour (0xff2a2620));
        g.drawEllipse (cx - radius + 0.5f, cy - radius + 0.5f, radius * 2.0f - 1.0f, radius * 2.0f - 1.0f, 1.0f);

        const float arcR = radius - (big ? 5.0f : 3.5f);
        juce::Path track;
        track.addCentredArc (cx, cy, arcR, arcR, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
        g.setColour (ArabesqueColours::arcTrack);
        g.strokePath (track, juce::PathStrokeType (big ? 2.6f : 2.0f, juce::PathStrokeType::curved,
                                                    juce::PathStrokeType::rounded));

        if (sliderPos > 0.002f)
        {
            juce::Path value;
            value.addCentredArc (cx, cy, arcR, arcR, 0.0f, rotaryStartAngle, endAngle, true);
            g.setColour (ArabesqueColours::gold);
            g.strokePath (value, juce::PathStrokeType (big ? 2.8f : 2.2f, juce::PathStrokeType::curved,
                                                       juce::PathStrokeType::rounded));
        }

        const float body = arcR - (big ? 6.0f : 4.5f);
        juce::ColourGradient bodyGrad (juce::Colour (0xff2a2622), cx, cy - body,
                                       juce::Colour (0xff12100e), cx, cy + body, false);
        g.setGradientFill (bodyGrad);
        g.fillEllipse (cx - body, cy - body, body * 2.0f, body * 2.0f);
        g.setColour (juce::Colour (0xff3a342c));
        g.drawEllipse (cx - body + 0.5f, cy - body + 0.5f, body * 2.0f - 1.0f, body * 2.0f - 1.0f, 1.0f);

        juce::Path pointer;
        const float th = big ? 2.2f : 1.8f;
        pointer.addRectangle (-th * 0.5f, -body * 0.92f, th, body * 0.62f);
        g.setColour (ArabesqueColours::goldBright);
        g.fillPath (pointer, juce::AffineTransform::rotation (endAngle).translated (cx, cy));

        g.setColour (juce::Colour (0xff0c0b0a));
        g.fillEllipse (cx - 2.2f, cy - 2.2f, 4.4f, 4.4f);
    }

    void drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float, float, const juce::Slider::SliderStyle style,
                           juce::Slider&) override
    {
        auto b = juce::Rectangle<float> ((float) x, (float) y, (float) width, (float) height);
        if (style == juce::Slider::LinearVertical)
        {
            auto track = b.withSizeKeepingCentre (8.0f, b.getHeight());
            g.setColour (juce::Colour (0xff141210));
            g.fillRoundedRectangle (track, 3.0f);
            g.setColour (juce::Colour (0xff2e2820));
            g.drawRoundedRectangle (track.reduced (0.5f), 3.0f, 1.0f);

            const float pos = juce::jlimit (track.getY(), track.getBottom(), sliderPos);
            auto fill = juce::Rectangle<float> (track.getX(), pos, track.getWidth(), track.getBottom() - pos);
            g.setColour (ArabesqueColours::gold);
            g.fillRoundedRectangle (fill, 3.0f);

            juce::Rectangle<float> thumb (track.getX() - 3.0f, pos - 5.0f, track.getWidth() + 6.0f, 10.0f);
            g.setColour (juce::Colour (0xff242018));
            g.fillRoundedRectangle (thumb, 2.0f);
            g.setColour (ArabesqueColours::gold);
            g.drawRoundedRectangle (thumb, 2.0f, 1.0f);
        }
        else
        {
            auto track = b.withSizeKeepingCentre (b.getWidth(), 6.0f);
            g.setColour (juce::Colour (0xff141210));
            g.fillRoundedRectangle (track, 3.0f);
            auto fill = track.withWidth (juce::jmax (6.0f, sliderPos - track.getX()));
            g.setColour (ArabesqueColours::gold);
            g.fillRoundedRectangle (fill, 3.0f);
        }
    }

    void drawComboBox (juce::Graphics& g, int width, int height, bool, int, int, int, int,
                       juce::ComboBox& box) override
    {
        auto bounds = juce::Rectangle<float> (0.5f, 0.5f, (float) width - 1.0f, (float) height - 1.0f);
        g.setColour (findColour (juce::ComboBox::backgroundColourId));
        g.fillRoundedRectangle (bounds, 3.0f);
        g.setColour (box.hasKeyboardFocus (false) ? ArabesqueColours::gold : findColour (juce::ComboBox::outlineColourId));
        g.drawRoundedRectangle (bounds, 3.0f, 1.0f);

        juce::Path arrow;
        const float ax = (float) width - 12.0f, ay = (float) height * 0.5f;
        arrow.addTriangle (ax - 3.5f, ay - 2.0f, ax + 3.5f, ay - 2.0f, ax, ay + 3.0f);
        g.setColour (findColour (juce::ComboBox::arrowColourId));
        g.fillPath (arrow);
    }

    juce::Font getComboBoxFont (juce::ComboBox&) override { return uiFont (12.0f); }
    juce::Font getPopupMenuFont() override               { return uiFont (13.0f); }

    void getIdealPopupMenuItemSize (const juce::String& text, bool isSeparator, int standardItemHeight,
                                    int& idealWidth, int& idealHeight) override
    {
        if (isSeparator) { idealWidth = 50; idealHeight = 8; return; }
        idealHeight = juce::jmax (22, standardItemHeight - 2);
        idealWidth = (int) juce::GlyphArrangement::getStringWidth (getPopupMenuFont(), text) + 36;
    }

    void drawButtonBackground (juce::Graphics& g, juce::Button& button, const juce::Colour&,
                               bool highlighted, bool down) override
    {
        auto bounds = button.getLocalBounds().toFloat().reduced (0.5f);
        const bool on = button.getToggleState();
        g.setColour (down ? juce::Colour (0xff2a2218)
                     : on ? juce::Colour (0xff2e2416)
                     : highlighted ? juce::Colour (0xff201c16) : juce::Colour (0xff181510));
        g.fillRoundedRectangle (bounds, 3.0f);
        g.setColour (on ? ArabesqueColours::gold : ArabesqueColours::panelBorder);
        g.drawRoundedRectangle (bounds, 3.0f, 1.0f);
    }

    juce::Font getTextButtonFont (juce::TextButton&, int) override { return uiFont (11.0f); }
    juce::Font getLabelFont (juce::Label&) override { return uiFont (12.0f); }
};

//==============================================================================
struct KnobWithLabel : public juce::Component
{
    KnobWithLabel (juce::AudioProcessorValueTreeState& s, const juce::String& paramId,
                   const juce::String& caption, juce::LookAndFeel* lnf,
                   bool captionOnTop = false)
        : apvts (&s), id (paramId), captionAtTop (captionOnTop)
    {
        slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        slider.setLookAndFeel (lnf);
        slider.setDoubleClickReturnValue (false, 0.0);
        slider.setPopupDisplayEnabled (false, false, nullptr);
        addAndMakeVisible (slider);

        captionLabel.setText (caption, juce::dontSendNotification);
        captionLabel.setFont (ArabesqueLookAndFeel::uiFont (10.5f));
        captionLabel.setColour (juce::Label::textColourId, ArabesqueColours::textCaption);
        captionLabel.setJustificationType (juce::Justification::centred);
        captionLabel.setInterceptsMouseClicks (false, false);
        addAndMakeVisible (captionLabel);

        valueLabel.setFont (ArabesqueLookAndFeel::uiFont (10.5f));
        valueLabel.setColour (juce::Label::textColourId, ArabesqueColours::textWarm);
        valueLabel.setJustificationType (juce::Justification::centred);
        valueLabel.setInterceptsMouseClicks (false, false);
        addAndMakeVisible (valueLabel);

        attach (s, paramId);
    }

    ~KnobWithLabel() override { slider.setLookAndFeel (nullptr); }

    void attach (juce::AudioProcessorValueTreeState& s, const juce::String& paramId)
    {
        apvts = &s;
        id = paramId;
        attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (s, paramId, slider);
        refreshValueText();
        slider.onValueChange = [this] { refreshValueText(); };
    }

    void refreshValueText()
    {
        juce::String t;
        if (apvts != nullptr)
            if (auto* p = apvts->getParameter (id))
                t = p->getText (p->getValue(), 64);
        if (t.isEmpty())
            t = slider.getTextFromValue (slider.getValue());
        if (valueLabel.getText() != t)
            valueLabel.setText (t, juce::dontSendNotification);
    }

    void resized() override
    {
        const int capH = 13, valH = 13;
        auto b = getLocalBounds();
        if (captionAtTop)
        {
            captionLabel.setBounds (b.removeFromTop (capH));
            valueLabel.setBounds (b.removeFromBottom (valH));
        }
        else
        {
            valueLabel.setBounds (b.removeFromBottom (valH));
            captionLabel.setBounds (b.removeFromBottom (capH));
        }
        const int d = juce::jmin (b.getWidth(), b.getHeight());
        slider.setBounds (b.withSizeKeepingCentre (d, d));
    }

    juce::Slider slider;
    juce::Label captionLabel, valueLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    juce::AudioProcessorValueTreeState* apvts = nullptr;
    juce::String id;
    bool captionAtTop = false;
};

//==============================================================================
struct FaderWithLabel : public juce::Component
{
    FaderWithLabel (juce::AudioProcessorValueTreeState& s, const juce::String& paramId,
                    const juce::String& caption, juce::LookAndFeel* lnf)
        : apvts (&s), id (paramId)
    {
        slider.setSliderStyle (juce::Slider::LinearVertical);
        slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        slider.setLookAndFeel (lnf);
        addAndMakeVisible (slider);

        captionLabel.setText (caption, juce::dontSendNotification);
        captionLabel.setFont (ArabesqueLookAndFeel::uiFont (10.0f));
        captionLabel.setColour (juce::Label::textColourId, ArabesqueColours::textCaption);
        captionLabel.setJustificationType (juce::Justification::centred);
        addAndMakeVisible (captionLabel);

        valueLabel.setFont (ArabesqueLookAndFeel::uiFont (10.0f));
        valueLabel.setColour (juce::Label::textColourId, ArabesqueColours::textWarm);
        valueLabel.setJustificationType (juce::Justification::centred);
        addAndMakeVisible (valueLabel);

        attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (s, paramId, slider);
        refreshValueText();
        slider.onValueChange = [this] { refreshValueText(); };
    }

    ~FaderWithLabel() override { slider.setLookAndFeel (nullptr); }

    void refreshValueText()
    {
        juce::String t;
        if (apvts != nullptr)
            if (auto* p = apvts->getParameter (id))
                t = p->getText (p->getValue(), 64);
        if (valueLabel.getText() != t)
            valueLabel.setText (t, juce::dontSendNotification);
    }

    void resized() override
    {
        auto b = getLocalBounds();
        captionLabel.setBounds (b.removeFromTop (14));
        valueLabel.setBounds (b.removeFromBottom (14));
        slider.setBounds (b.reduced (6, 2));
    }

    juce::Slider slider;
    juce::Label captionLabel, valueLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    juce::AudioProcessorValueTreeState* apvts = nullptr;
    juce::String id;
};
