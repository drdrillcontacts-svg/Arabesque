#pragma once
#include <JuceHeader.h>

//==============================================================================
namespace ArabesqueColours
{
    const juce::Colour background      { 0xff0b0a09 };
    const juce::Colour panelFill       { 0xff16130f };
    const juce::Colour panelFillLight  { 0xff1e1913 };
    const juce::Colour panelBorder     { 0xff6e5636 };
    const juce::Colour gold            { 0xffd8a860 };
    const juce::Colour goldBright      { 0xfff0cd8d };
    const juce::Colour copper          { 0xffb06a3c };
    const juce::Colour orangeGlow      { 0xffe8842f };
    const juce::Colour redGlow         { 0xffe23c22 };
    const juce::Colour textWarm        { 0xfff3e7d2 };
    const juce::Colour textDim         { 0xff9d9181 };
    const juce::Colour textCaption     { 0xff877c6e };
    const juce::Colour knobBody        { 0xff211d1a };
    const juce::Colour arcTrack        { 0xff3a3128 };
    const juce::Colour keyWhite        { 0xffefe5cf };
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
        setColour (juce::PopupMenu::backgroundColourId,         juce::Colour (0xff1c1712));
        setColour (juce::PopupMenu::textColourId,               ArabesqueColours::textWarm);
        setColour (juce::PopupMenu::highlightedBackgroundColourId, juce::Colour (0xff4a3620));
        setColour (juce::PopupMenu::highlightedTextColourId,    ArabesqueColours::goldBright);
        setColour (juce::TextButton::buttonColourId,            juce::Colour (0xff201a14));
        setColour (juce::TextButton::buttonOnColourId,          juce::Colour (0xff3d2c17));
        setColour (juce::TextButton::textColourOffId,           ArabesqueColours::textDim);
        setColour (juce::TextButton::textColourOnId,            ArabesqueColours::goldBright);
        setColour (juce::Label::textColourId,                   ArabesqueColours::textWarm);
        setColour (juce::Label::backgroundColourId,             juce::Colours::transparentBlack);
        setColour (juce::Label::outlineColourId,                juce::Colours::transparentBlack);
        setColour (juce::Slider::textBoxTextColourId,           ArabesqueColours::textWarm);
        setColour (juce::Slider::textBoxBackgroundColourId,     juce::Colour (0xff14110d));
        setColour (juce::Slider::textBoxOutlineColourId,        juce::Colour (0xff3d332a));
        setColour (juce::Slider::trackColourId,                 ArabesqueColours::gold);
        setColour (juce::Slider::backgroundColourId,            juce::Colour (0xff171310));
        setColour (juce::Slider::thumbColourId,                 ArabesqueColours::goldBright);
        setColour (juce::MidiKeyboardComponent::whiteNoteColourId,              ArabesqueColours::keyWhite);
        setColour (juce::MidiKeyboardComponent::blackNoteColourId,              juce::Colour (0xff1b1713));
        setColour (juce::MidiKeyboardComponent::keySeparatorLineColourId,       juce::Colour (0xff8a6c42));
        setColour (juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId,    juce::Colour (0x66d8a860));
        setColour (juce::MidiKeyboardComponent::keyDownOverlayColourId,         juce::Colour (0xaae8842f));
        setColour (juce::MidiKeyboardComponent::textLabelColourId,              juce::Colours::transparentBlack);
        setColour (juce::MidiKeyboardComponent::upDownButtonBackgroundColourId, juce::Colours::transparentBlack);
        setColour (juce::MidiKeyboardComponent::upDownButtonArrowColourId,      ArabesqueColours::gold);
        setColour (juce::MidiKeyboardComponent::shadowColourId,                 juce::Colours::transparentBlack);
    }

    static juce::Font serifFont (float size, bool bold = false)
    {
        juce::ignoreUnused (bold);
        return juce::Font (juce::FontOptions (size));
    }

    static juce::Font captionFont (float size)
    {
        return juce::Font (juce::FontOptions (size));
    }

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                           juce::Slider&) override
    {
        auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (1.0f);
        const float radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
        const float cx = bounds.getCentreX(), cy = bounds.getCentreY();
        const float startAngle = rotaryStartAngle;
        const float endAngle = startAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        const bool big = radius > 34.0f;

        // Cover the skin knob (including its baked arc) so live values don't double-draw.
        g.setColour (ArabesqueColours::panelFill);
        g.fillEllipse (cx - radius, cy - radius, radius * 2.0f, radius * 2.0f);

        if (big)
        {
            g.setColour (ArabesqueColours::orangeGlow.withAlpha (0.10f));
            g.fillEllipse (cx - radius - 4.0f, cy - radius - 4.0f, (radius + 4.0f) * 2.0f, (radius + 4.0f) * 2.0f);
        }

        juce::ColourGradient bezelGrad (juce::Colour (0xff2e2924), cx, cy - radius,
                                        juce::Colour (0xff141110), cx, cy + radius, false);
        g.setGradientFill (bezelGrad);
        g.fillEllipse (cx - radius, cy - radius, radius * 2.0f, radius * 2.0f);

        g.setColour (juce::Colour (0xff0a0908));
        g.drawEllipse (cx - radius + 0.5f, cy - radius + 0.5f, radius * 2.0f - 1.0f, radius * 2.0f - 1.0f, 1.0f);

        const float arcRadius = radius - (big ? 6.0f : 4.0f);
        juce::Path trackArc;
        trackArc.addCentredArc (cx, cy, arcRadius, arcRadius, 0.0f, startAngle, rotaryEndAngle, true);
        g.setColour (ArabesqueColours::arcTrack);
        g.strokePath (trackArc, juce::PathStrokeType (big ? 3.5f : 2.5f, juce::PathStrokeType::curved,
                                                      juce::PathStrokeType::rounded));

        if (sliderPos > 0.001f)
        {
            juce::Path valueArc;
            valueArc.addCentredArc (cx, cy, arcRadius, arcRadius, 0.0f, startAngle, endAngle, true);
            juce::ColourGradient arcGrad (ArabesqueColours::goldBright, cx - arcRadius, cy,
                                          ArabesqueColours::orangeGlow, cx + arcRadius, cy, false);
            g.setGradientFill (arcGrad);
            g.strokePath (valueArc, juce::PathStrokeType (big ? 4.5f : 3.0f, juce::PathStrokeType::curved,
                                                          juce::PathStrokeType::rounded));
        }

        const float bodyRadius = arcRadius - (big ? 8.0f : 6.0f);
        juce::ColourGradient bodyGrad (juce::Colour (0xff3a332c), cx, cy - bodyRadius,
                                       juce::Colour (0xff0f0d0b), cx, cy + bodyRadius, false);
        g.setGradientFill (bodyGrad);
        g.fillEllipse (cx - bodyRadius, cy - bodyRadius, bodyRadius * 2.0f, bodyRadius * 2.0f);

        g.setColour (juce::Colour (0xff54483b).withAlpha (0.6f));
        g.drawEllipse (cx - bodyRadius + 0.5f, cy - bodyRadius + 0.5f,
                       bodyRadius * 2.0f - 1.0f, bodyRadius * 2.0f - 1.0f, 1.0f);

        juce::Path pointer;
        const float pointerLen = bodyRadius * 0.9f;
        const float pointerThickness = big ? 3.2f : 2.4f;
        pointer.addRectangle (-pointerThickness * 0.5f, -bodyRadius - 3.0f, pointerThickness, pointerLen);
        g.setColour (ArabesqueColours::goldBright);
        g.fillPath (pointer, juce::AffineTransform::rotation (endAngle).translated (cx, cy));

        g.setColour (juce::Colour (0xff0d0b09));
        g.fillEllipse (cx - 3.0f, cy - 3.0f, 6.0f, 6.0f);
    }

    void drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float, float, const juce::Slider::SliderStyle style,
                           juce::Slider&) override
    {
        auto b = juce::Rectangle<float> ((float) x, (float) y, (float) width, (float) height);

        if (style == juce::Slider::LinearVertical)
        {
            auto track = b.withSizeKeepingCentre (10.0f, b.getHeight());
            g.setColour (juce::Colour (0xff171310));
            g.fillRoundedRectangle (track, 4.0f);
            g.setColour (juce::Colour (0xff3a3128));
            g.drawRoundedRectangle (track.reduced (0.5f), 4.0f, 1.0f);

            const float pos = juce::jlimit (track.getY(), track.getBottom(), sliderPos);
            auto fill = juce::Rectangle<float> (track.getX(), pos, track.getWidth(), track.getBottom() - pos);
            juce::ColourGradient grad (ArabesqueColours::goldBright, fill.getX(), fill.getY(),
                                       ArabesqueColours::copper, fill.getX(), fill.getBottom(), false);
            g.setGradientFill (grad);
            g.fillRoundedRectangle (fill, 4.0f);

            juce::Rectangle<float> thumb (track.getX() - 4.0f, pos - 6.0f, track.getWidth() + 8.0f, 12.0f);
            g.setColour (juce::Colour (0xff2a241e));
            g.fillRoundedRectangle (thumb, 3.0f);
            g.setColour (ArabesqueColours::gold);
            g.drawRoundedRectangle (thumb, 3.0f, 1.0f);
        }
        else
        {
            auto track = b.withSizeKeepingCentre (b.getWidth(), 8.0f);
            g.setColour (juce::Colour (0xff171310));
            g.fillRoundedRectangle (track, 4.0f);
            auto fill = track.withWidth (juce::jmax (8.0f, sliderPos - track.getX()));
            g.setColour (ArabesqueColours::gold);
            g.fillRoundedRectangle (fill, 4.0f);
        }
    }

    void drawComboBox (juce::Graphics& g, int width, int height, bool, int, int, int, int,
                       juce::ComboBox& box) override
    {
        auto bounds = juce::Rectangle<float> (0.5f, 0.5f, (float) width - 1.0f, (float) height - 1.0f);
        g.setColour (findColour (juce::ComboBox::backgroundColourId));
        g.fillRoundedRectangle (bounds, 4.0f);
        g.setColour (box.hasKeyboardFocus (false) ? ArabesqueColours::gold : findColour (juce::ComboBox::outlineColourId));
        g.drawRoundedRectangle (bounds, 4.0f, 1.0f);

        juce::Path arrow;
        const float ax = (float) width - 14.0f, ay = (float) height * 0.5f;
        arrow.startNewSubPath (ax - 3.5f, ay - 2.0f);
        arrow.lineTo (ax + 3.5f, ay - 2.0f);
        arrow.lineTo (ax, ay + 3.0f);
        arrow.closeSubPath();
        g.setColour (findColour (juce::ComboBox::arrowColourId));
        g.fillPath (arrow);
    }

    juce::Font getComboBoxFont (juce::ComboBox&) override { return captionFont (12.0f); }
    juce::Font getPopupMenuFont() override               { return captionFont (13.0f); }

    void getIdealPopupMenuItemSize (const juce::String& text, bool isSeparator, int standardItemHeight,
                                    int& idealWidth, int& idealHeight) override
    {
        if (isSeparator) { idealWidth = 50; idealHeight = 10; return; }
        idealHeight = juce::jmax (24, standardItemHeight - 2);
        idealWidth = (int) juce::GlyphArrangement::getStringWidth (getPopupMenuFont(), text) + 40;
    }

    void drawButtonBackground (juce::Graphics& g, juce::Button& button, const juce::Colour&,
                               bool highlighted, bool down) override
    {
        auto bounds = button.getLocalBounds().toFloat().reduced (0.5f);
        g.setColour (down ? juce::Colour (0xff2e2418)
                     : highlighted ? juce::Colour (0xff241d15) : juce::Colour (0xff1a1510));
        g.fillRoundedRectangle (bounds, 4.0f);
        g.setColour (ArabesqueColours::panelBorder.brighter (0.15f));
        g.drawRoundedRectangle (bounds, 4.0f, 1.0f);
    }

    juce::Font getTextButtonFont (juce::TextButton&, int) override { return captionFont (11.5f); }
    juce::Font getLabelFont (juce::Label&) override { return captionFont (12.0f); }
};

//==============================================================================
// Rotary knob + live value readout. Captions live on the skin image.
struct KnobWithLabel : public juce::Component
{
    KnobWithLabel (juce::AudioProcessorValueTreeState& s, const juce::String& paramId,
                   const juce::String& caption, juce::LookAndFeel* lnf,
                   bool captionOnTop = false)
        : apvts (&s), id (paramId), captionAtTop (captionOnTop)
    {
        juce::ignoreUnused (caption, captionAtTop);
        slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        slider.setLookAndFeel (lnf);
        slider.setDoubleClickReturnValue (false, 0.0);
        slider.setPopupDisplayEnabled (false, false, nullptr);
        addAndMakeVisible (slider);

        valueLabel.setFont (ArabesqueLookAndFeel::captionFont (11.0f));
        valueLabel.setColour (juce::Label::textColourId, ArabesqueColours::textWarm);
        valueLabel.setColour (juce::Label::backgroundColourId, ArabesqueColours::panelFill);
        valueLabel.setJustificationType (juce::Justification::centred);
        valueLabel.setOpaque (true);
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
        if (auto* p = s.getParameter (paramId))
        {
            if (p->getNumSteps() > 1 && p->getNumSteps() < 64)
                slider.setNumDecimalPlacesToDisplay (0);
        }
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
        const int valueH = juce::jmax (12, getHeight() / 8);
        auto b = getLocalBounds();
        valueLabel.setBounds (b.removeFromBottom (valueH));
        const int d = juce::jmin (b.getWidth(), b.getHeight());
        slider.setBounds (b.withSizeKeepingCentre (d, d));
    }

    juce::Slider slider;
    juce::Label valueLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    juce::AudioProcessorValueTreeState* apvts = nullptr;
    juce::String id;
    bool captionAtTop = false;
};

//==============================================================================
struct FaderWithLabel : public juce::Component
{
    FaderWithLabel (juce::AudioProcessorValueTreeState& s, const juce::String& paramId, juce::LookAndFeel* lnf)
        : apvts (&s), id (paramId)
    {
        slider.setSliderStyle (juce::Slider::LinearVertical);
        slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        slider.setLookAndFeel (lnf);
        addAndMakeVisible (slider);

        valueLabel.setFont (ArabesqueLookAndFeel::captionFont (11.0f));
        valueLabel.setColour (juce::Label::textColourId, ArabesqueColours::textWarm);
        valueLabel.setColour (juce::Label::backgroundColourId, ArabesqueColours::panelFill);
        valueLabel.setJustificationType (juce::Justification::centred);
        valueLabel.setOpaque (true);
        valueLabel.setInterceptsMouseClicks (false, false);
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
        if (t.isEmpty())
            t = slider.getTextFromValue (slider.getValue());
        if (valueLabel.getText() != t)
            valueLabel.setText (t, juce::dontSendNotification);
    }

    void resized() override
    {
        auto b = getLocalBounds();
        valueLabel.setBounds (b.removeFromBottom (16));
        slider.setBounds (b.reduced (8, 2));
    }

    juce::Slider slider;
    juce::Label valueLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    juce::AudioProcessorValueTreeState* apvts = nullptr;
    juce::String id;
};
