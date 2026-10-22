#include "PluginEditor.h"
#include "BinaryData.h"

using namespace ArabesqueColours;

namespace
{

// Logical size of the whole interface - matches the mock 1:1, then scaled.
constexpr int W = 1536, H = 1024;

//==============================================================================
void drawOrnament (juce::Graphics& g, float cx, float cy, float s, juce::Colour c)
{
    // simplified 4-fold arabesque knot: two rotated diamonds + centre square
    g.setColour (c);
    juce::Path d1, d2;
    d1.addQuadrilateral (cx, cy - s, cx + s, cy, cx, cy + s, cx - s, cy);
    d2.addQuadrilateral (cx, cy - s * 0.62f, cx + s * 0.62f, cy, cx, cy + s * 0.62f, cx - s * 0.62f, cy);
    g.strokePath (d1, juce::PathStrokeType (juce::jmax (1.0f, s * 0.09f)));
    g.strokePath (d2, juce::PathStrokeType (juce::jmax (1.0f, s * 0.07f)));
    const float inner = s * 0.18f;
    g.fillRect (cx - inner, cy - inner, inner * 2.0f, inner * 2.0f);
}

void drawDiamond (juce::Graphics& g, float cx, float cy, float s, juce::Colour c)
{
    juce::Path p;
    p.addQuadrilateral (cx, cy - s, cx + s, cy, cx, cy + s, cx - s, cy);
    g.setColour (c);
    g.fillPath (p);
}

//==============================================================================
// Waveform thumbnail (oscillators + LFO)
class WaveDisplay final : public juce::Component
{
public:
    enum class Style { oscillator, lfo };
    WaveDisplay (Style st, std::function<int()> getShape) : style (st), shapeFn (std::move (getShape)) {}

    void paint (juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat();
        g.setColour (juce::Colour (0xff120f0c));
        g.fillRoundedRectangle (b, 4.0f);
        g.setColour (juce::Colour (0xff3d332a));
        g.drawRoundedRectangle (b.reduced (0.5f), 4.0f, 1.0f);

        const int shape = shapeFn != nullptr ? shapeFn() : 0;
        auto area = b.reduced (6.0f, 8.0f);
        juce::Path p;
        const int n = 96;
        for (int i = 0; i <= n; ++i)
        {
            const float t = (float) i / (float) n;
            float y = 0.0f;
            switch (shape)
            {
                case 0: y = 2.0f * t - 1.0f; break;                                // saw
                case 1: y = t < 0.5f ? 1.0f : -1.0f; break;                        // square
                case 2: y = 1.0f - 4.0f * std::abs (t - 0.5f); break;              // triangle
                case 3: y = std::sin (t * juce::MathConstants<float>::twoPi); break; // sine
                case 4: // S&H (lfo only)
                    y = std::sin (std::floor (t * 6.0f) * 1.7f) * 0.9f;
                    break;
                default: break;
            }
            const float px = area.getX() + t * area.getWidth();
            const float py = area.getCentreY() - y * area.getHeight() * 0.42f;
            if (i == 0) p.startNewSubPath (px, py); else p.lineTo (px, py);
        }
        if (shape == 1) { /* square drawn as steps for crispness */ }
        g.setColour (gold.withAlpha (0.9f));
        g.strokePath (p, juce::PathStrokeType (1.4f));
    }

private:
    Style style;
    std::function<int()> shapeFn;
};

//==============================================================================
// Pitch / mod wheel
class WheelControl final : public juce::Component
{
public:
    WheelControl (const juce::String& caption, bool springs, std::atomic<float>& target)
        : name (caption), springsToZero (springs), targetValue (target) {}

    void setValue (float v) { value = juce::jlimit (minV, maxV, v); targetValue.store (value); repaint(); }

    void paint (juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat();
        auto groove = b.withSizeKeepingCentre (14.0f, b.getHeight()).reduced (0.0f, 4.0f);
        g.setColour (juce::Colour (0xff171310));
        g.fillRoundedRectangle (groove, 6.0f);
        g.setColour (juce::Colour (0xff3a3128));
        g.drawRoundedRectangle (groove.reduced (0.5f), 6.0f, 1.0f);

        const float pos = (value - minV) / (maxV - minV);
        const float ty = juce::jmap (pos, groove.getBottom() - 16.0f, groove.getY() + 2.0f);
        juce::Rectangle<float> thumb (groove.getX() - 3.0f, ty, groove.getWidth() + 6.0f, 14.0f);

        juce::ColourGradient grad (juce::Colour (0xff4a4138), thumb.getX(), thumb.getY(),
                                   juce::Colour (0xff211d19), thumb.getX(), thumb.getBottom(), false);
        g.setGradientFill (grad);
        g.fillRoundedRectangle (thumb, 3.0f);
        g.setColour (gold.withAlpha (0.8f));
        g.fillRect (thumb.getX() + 3.0f, thumb.getCentreY() - 0.75f, thumb.getWidth() - 6.0f, 1.5f);
    }

    void mouseDown (const juce::MouseEvent& e) override    { dragStartY = e.y; dragStartValue = value; }
    void mouseDrag (const juce::MouseEvent& e) override
    {
        const float delta = (float) (dragStartY - e.y) / 120.0f * (maxV - minV) * 0.5f;
        setValue (dragStartValue + delta);
    }
    void mouseUp (const juce::MouseEvent&) override        { if (springsToZero) setValue (0.0f); }
    void mouseDoubleClick (const juce::MouseEvent&) override { setValue (springsToZero ? 0.0f : 0.0f); }

    const juce::String& getCaption() const noexcept { return name; }

private:
    juce::String name;
    bool springsToZero;
    std::atomic<float>& targetValue;
    float minV = -1.0f, maxV = 1.0f;
    float value = 0.0f;
    int dragStartY = 0;
    float dragStartValue = 0.0f;
};

//==============================================================================
// ARP step pads
class StepPad final : public juce::TextButton
{
public:
    void setActive (bool shouldBeActive) { if (active != shouldBeActive) { active = shouldBeActive; repaint(); } }

    void paintButton (juce::Graphics& g, bool highlighted, bool down) override
    {
        juce::ignoreUnused (down);
        auto b = getLocalBounds().toFloat().reduced (1.0f);
        const bool on = getToggleState();
        g.setColour (on ? juce::Colour (0xff3a2415) : juce::Colour (0xff1a1613));
        g.fillRoundedRectangle (b, 3.0f);
        g.setColour (active ? redGlow : on ? copper.brighter (0.3f) : juce::Colour (0xff332b24));
        g.drawRoundedRectangle (b, 3.0f, active ? 2.0f : 1.0f);
        if (active)
        {
            g.setColour (redGlow.withAlpha (0.35f));
            g.fillRoundedRectangle (b, 3.0f);
        }
        if (on)
        {
            g.setColour (active ? redGlow.brighter (0.2f) : gold);
            auto led = b.withSizeKeepingCentre (6.0f, 6.0f).translated (0.0f, -b.getHeight() * 0.22f);
            g.fillEllipse (led);
        }
        if (highlighted)
        {
            g.setColour (gold.withAlpha (0.12f));
            g.fillRoundedRectangle (b, 3.0f);
        }
    }

private:
    bool active = false;
};

//==============================================================================
// Lamp toggle (FX ON / Arp ON / Sync)
class LampToggle final : public juce::TextButton
{
public:
    LampToggle (const juce::String& onText, const juce::String& offText)
        : labelOn (onText), labelOff (offText)
    {
        setClickingTogglesState (true);
    }

    void paintButton (juce::Graphics& g, bool highlighted, bool down) override
    {
        juce::ignoreUnused (down);
        auto b = getLocalBounds().toFloat().reduced (0.5f);
        const bool on = getToggleState();
        g.setColour (on ? juce::Colour (0xff3d1a10) : juce::Colour (0xff1b1714));
        g.fillRoundedRectangle (b, 4.0f);
        g.setColour (on ? redGlow.withAlpha (0.85f) : juce::Colour (0xff3f372e));
        g.drawRoundedRectangle (b, 4.0f, 1.0f);

        if (on)
        {
            g.setColour (redGlow.withAlpha (0.25f));
            g.fillRoundedRectangle (b.reduced (2.0f), 3.0f);
        }

        auto lamp = b.removeFromLeft (18.0f).withSizeKeepingCentre (7.0f, 7.0f);
        g.setColour (on ? redGlow.brighter (0.3f) : juce::Colour (0xff4a4238));
        g.fillEllipse (lamp);

        g.setColour (on ? juce::Colour (0xffffb0a0) : textDim);
        g.setFont (ArabesqueLookAndFeel::captionFont (11.0f));
        g.drawText (on ? labelOn : labelOff, b.reduced (2.0f), juce::Justification::centred);

        if (highlighted)
        {
            g.setColour (gold.withAlpha (0.1f));
            g.fillRoundedRectangle (getLocalBounds().toFloat().reduced (0.5f), 4.0f);
        }
    }

private:
    juce::String labelOn, labelOff;
};

//==============================================================================
// Triangle arrow buttons for preset browsing / octave
class NavArrowButton final : public juce::Button
{
public:
    NavArrowButton (bool pointsLeft) : juce::Button (pointsLeft ? "prev" : "next"), left (pointsLeft) {}

    void paintButton (juce::Graphics& g, bool highlighted, bool down) override
    {
        auto b = getLocalBounds().toFloat().reduced (2.0f);
        g.setColour (down ? juce::Colour (0xff2c2216) : highlighted ? juce::Colour (0xff211a13) : juce::Colour (0xff181310));
        g.fillRoundedRectangle (b, 4.0f);
        g.setColour (panelBorder);
        g.drawRoundedRectangle (b, 4.0f, 1.0f);

        juce::Path tri;
        const float cx = b.getCentreX(), cy = b.getCentreY(), s = juce::jmin (b.getWidth(), b.getHeight()) * 0.22f;
        if (left) { tri.addTriangle (cx + s * 0.5f, cy - s, cx + s * 0.5f, cy + s, cx - s * 0.5f, cy); }
        else      { tri.addTriangle (cx - s * 0.5f, cy - s, cx - s * 0.5f, cy + s, cx + s * 0.5f, cy); }
        g.setColour (gold);
        g.fillPath (tri);
    }

private:
    bool left;
};

//==============================================================================
void drawSectionPanel (juce::Graphics& g, juce::Rectangle<float> r, const juce::String& title)
{
    g.setColour (panelFill);
    g.fillRoundedRectangle (r, 10.0f);
    juce::ColourGradient sheen (panelFillLight.withAlpha (0.55f), r.getX(), r.getY(),
                                panelFill.withAlpha (0.0f), r.getX(), r.getY() + r.getHeight() * 0.5f, false);
    g.setGradientFill (sheen);
    g.fillRoundedRectangle (r.reduced (1.0f), 9.0f);
    g.setColour (panelBorder.withAlpha (0.9f));
    g.drawRoundedRectangle (r.reduced (0.5f), 10.0f, 1.0f);

    if (title.isNotEmpty())
    {
        g.setColour (textCaption);
        g.setFont (ArabesqueLookAndFeel::captionFont (12.0f));
        g.drawText (title.toUpperCase(), (int) r.getX() + 14, (int) r.getY() + 8, 200, 18,
                    juce::Justification::left);
        drawDiamond (g, r.getX() + 9.0f, r.getY() + 17.0f, 3.5f, gold.withAlpha (0.7f));
    }
}

} // namespace

//==============================================================================
class ArabesqueAudioProcessorEditor::Content final : public juce::Component,
                                                     private juce::Timer
{
public:
    explicit Content (ArabesqueAudioProcessor& p)
        : proc (p),
          keyboard (p.keyboardState, juce::KeyboardComponentBase::horizontalKeyboard)
    {
        setSize (W, H);
        artwork = juce::ImageCache::getFromMemory (BinaryData::Artwork_jpg, BinaryData::Artwork_jpgSize);

        buildControls();
        refreshPresetLabel();
        startTimerHz (30);
    }

    ~Content() override { stopTimer(); }

private:
    ArabesqueAudioProcessor& proc;
    ArabesqueLookAndFeel lnf;
    juce::Image artwork;

    //--- header
    std::unique_ptr<NavArrowButton> prevPreset, nextPreset;
    juce::Label presetNameLabel;
    std::unique_ptr<juce::TextButton> saveButton, browseButton;
    std::unique_ptr<juce::ComboBox> voicesCombo;
    juce::Label voicesLabel;
    std::unique_ptr<juce::ShapeButton> settingsButton;

    //--- oscillators
    std::unique_ptr<WaveDisplay> osc1WaveDisplay, osc2WaveDisplay, lfoWaveDisplay;
    std::unique_ptr<juce::ComboBox> osc1WaveCombo, osc2WaveCombo;
    std::unique_ptr<KnobWithLabel> osc1Octave, osc1Semi, osc1Fine, osc1Level;
    std::unique_ptr<KnobWithLabel> osc2Octave, osc2Semi, osc2Fine, osc2Level;

    //--- mixer
    std::unique_ptr<KnobWithLabel> mixOsc1, mixOsc2, mixNoise, mixSub;

    //--- filter
    std::unique_ptr<juce::ComboBox> filterTypeCombo;
    std::unique_ptr<KnobWithLabel> cutoffKnob, resonanceKnob, keyFollowKnob;

    //--- envelopes (tabbed)
    std::vector<std::unique_ptr<juce::TextButton>> envTabs;
    std::unique_ptr<KnobWithLabel> envA, envD, envS, envR;
    int envTabIndex = 0;

    //--- lfo
    std::unique_ptr<juce::ComboBox> lfoShapeCombo;
    std::unique_ptr<LampToggle> syncToggle;
    std::unique_ptr<KnobWithLabel> lfoRateKnob, lfoDepthKnob;

    //--- fx
    std::unique_ptr<LampToggle> fxOnToggle;
    std::unique_ptr<KnobWithLabel> chorusRate, chorusDepth, phaserRate, phaserDepth;
    std::unique_ptr<KnobWithLabel> delayTime, delayFeedback, delayMix;
    std::unique_ptr<juce::ComboBox> reverbTypeCombo;
    std::unique_ptr<KnobWithLabel> reverbSize, reverbDecay, reverbMix;

    //--- arp
    std::unique_ptr<LampToggle> arpOnToggle;
    std::vector<std::unique_ptr<StepPad>> stepPads;
    std::unique_ptr<KnobWithLabel> arpRateKnob;
    std::unique_ptr<NavArrowButton> octDown, octUp;
    juce::Label octValueLabel;

    //--- centre / master
    std::unique_ptr<KnobWithLabel> vibratoKnob, glideKnob, driveKnob, panKnob, mainKnob;

    //--- keyboard area
    std::unique_ptr<WheelControl> pitchWheel, modWheel;
    juce::MidiKeyboardComponent keyboard;

    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>> comboAttachments;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> buttonAttachments;

    juce::String lastShownPresetName;

    //==========================================================================
    std::unique_ptr<KnobWithLabel> mkKnob (const char* id, const juce::String& cap, bool top = false)
    {
        auto k = std::make_unique<KnobWithLabel> (proc.parameters, id, cap, &lnf, top);
        addAndMakeVisible (*k);
        return k;
    }

    std::unique_ptr<juce::ComboBox> mkCombo (const char* id, const juce::StringArray& items)
    {
        auto c = std::make_unique<juce::ComboBox>();
        c->setLookAndFeel (&lnf);
        c->addItemList (items, 1);
        addAndMakeVisible (*c);
        comboAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>
                                    (proc.parameters, id, *c));
        return c;
    }

    void buildControls()
    {
        //---------------------------------------------------------- header --
        prevPreset = std::make_unique<NavArrowButton> (true);
        nextPreset = std::make_unique<NavArrowButton> (false);
        addAndMakeVisible (*prevPreset); addAndMakeVisible (*nextPreset);
        prevPreset->onClick = [this] { stepPreset (-1); };
        nextPreset->onClick = [this] { stepPreset (1); };

        presetNameLabel.setFont (ArabesqueLookAndFeel::captionFont (15.0f));
        presetNameLabel.setColour (juce::Label::textColourId, textWarm);
        presetNameLabel.setJustificationType (juce::Justification::centred);
        addAndMakeVisible (presetNameLabel);

        saveButton = std::make_unique<juce::TextButton> ("SAVE");
        browseButton = std::make_unique<juce::TextButton> ("BROWSE");
        for (auto* b : { saveButton.get(), browseButton.get() })
        {
            b->setLookAndFeel (&lnf);
            addAndMakeVisible (b);
        }
        saveButton->onClick = [this] { doSave(); };
        browseButton->onClick = [this] { showPresetBrowser(); };

        voicesLabel.setText ("Voices:", juce::dontSendNotification);
        voicesLabel.setFont (ArabesqueLookAndFeel::captionFont (12.5f));
        voicesLabel.setColour (juce::Label::textColourId, textDim);
        voicesLabel.setJustificationType (juce::Justification::centredRight);
        addAndMakeVisible (voicesLabel);
        voicesCombo = mkCombo (ParamIDs::voices, Choices::voiceCounts());

        juce::Path cog;
        cog.addStar ({}, 8, 3.2f, 6.5f);
        cog.addEllipse (-2.0f, -2.0f, 4.0f, 4.0f);
        cog.setUsingNonZeroWinding (false);
        settingsButton = std::make_unique<juce::ShapeButton> ("settings", panelFillLight, panelFillLight, gold);
        settingsButton->setShape (cog, false, true, false);
        settingsButton->setOutline (gold, 1.0f);
        addAndMakeVisible (*settingsButton);
        settingsButton->onClick = [this] { showAboutMenu(); };

        //------------------------------------------------------ oscillators --
        auto wavedisp = [this] (std::unique_ptr<juce::ComboBox>& comboForShape)
        {
            auto d = std::make_unique<WaveDisplay> (WaveDisplay::Style::oscillator,
                        [this, &comboForShape] { return comboForShape != nullptr ? comboForShape->getSelectedItemIndex() : 0; });
            addAndMakeVisible (*d);
            return d;
        };
        osc1WaveCombo = mkCombo (ParamIDs::osc1Wave, Choices::waveforms());
        osc2WaveCombo = mkCombo (ParamIDs::osc2Wave, Choices::waveforms());
        osc1WaveDisplay = wavedisp (osc1WaveCombo);
        osc2WaveDisplay = wavedisp (osc2WaveCombo);
        osc1WaveCombo->onChange = [this] { osc1WaveDisplay->repaint(); };
        osc2WaveCombo->onChange = [this] { osc2WaveDisplay->repaint(); };

        osc1Octave = mkKnob (ParamIDs::osc1Octave, "Octave");
        osc1Semi   = mkKnob (ParamIDs::osc1Semi,   "Semitone");
        osc1Fine   = mkKnob (ParamIDs::osc1Fine,   "Fine");
        osc1Level  = mkKnob (ParamIDs::mixOsc1,    "Level");   // mirrors mixer knob
        osc2Octave = mkKnob (ParamIDs::osc2Octave, "Octave");
        osc2Semi   = mkKnob (ParamIDs::osc2Semi,   "Semitone");
        osc2Fine   = mkKnob (ParamIDs::osc2Fine,   "Fine");
        osc2Level  = mkKnob (ParamIDs::mixOsc2,    "Level");

        //----------------------------------------------------------- mixer --
        mixOsc1  = mkKnob (ParamIDs::mixOsc1,  "Osc 1");
        mixOsc2  = mkKnob (ParamIDs::mixOsc2,  "Osc 2");
        mixNoise = mkKnob (ParamIDs::mixNoise, "Noise");
        mixSub   = mkKnob (ParamIDs::mixSub,   "Sub");

        //---------------------------------------------------------- filter --
        filterTypeCombo = mkCombo (ParamIDs::filterType, Choices::filterTypes());
        cutoffKnob    = mkKnob (ParamIDs::cutoff,    "Cutoff");
        resonanceKnob = mkKnob (ParamIDs::resonance, "Resonance");
        keyFollowKnob = mkKnob (ParamIDs::keyFollow, "Key Follow");

        //-------------------------------------------------------- envelopes --
        const juce::StringArray tabNames { "AMP", "FILTER", "PITCH" };
        for (int i = 0; i < 3; ++i)
        {
            auto t = std::make_unique<juce::TextButton> (tabNames[i]);
            t->setLookAndFeel (&lnf);
            t->setClickingTogglesState (true);
            t->setRadioGroupId (77, juce::dontSendNotification);
            t->onClick = [this, i] { if (envTabs[(size_t) i]->getToggleState()) selectEnvTab (i); };
            addAndMakeVisible (*t);
            envTabs.push_back (std::move (t));
        }
        envA = mkKnob (ParamIDs::ampAttack,  "Attack");
        envD = mkKnob (ParamIDs::ampDecay,   "Decay");
        envS = mkKnob (ParamIDs::ampSustain, "Sustain");
        envR = mkKnob (ParamIDs::ampRelease, "Release");
        selectEnvTab (0);

        //-------------------------------------------------------------- lfo --
        lfoShapeCombo = mkCombo (ParamIDs::lfoShape, Choices::lfoShapes());
        syncToggle = std::make_unique<LampToggle> ("Sync", "Sync");
        addAndMakeVisible (*syncToggle);
        buttonAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>
                                     (proc.parameters, ParamIDs::lfoSync, *syncToggle));
        lfoWaveDisplay = std::make_unique<WaveDisplay> (WaveDisplay::Style::lfo,
                            [this] { return lfoShapeCombo != nullptr ? lfoShapeCombo->getSelectedItemIndex() : 1; });
        addAndMakeVisible (*lfoWaveDisplay);
        lfoShapeCombo->onChange = [this] { lfoWaveDisplay->repaint(); };
        lfoRateKnob  = mkKnob (ParamIDs::lfoRate,  "Rate");
        lfoDepthKnob = mkKnob (ParamIDs::lfoDepth, "Depth");

        //--------------------------------------------------------------- fx --
        fxOnToggle = std::make_unique<LampToggle> ("ON", "OFF");
        addAndMakeVisible (*fxOnToggle);
        buttonAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>
                                     (proc.parameters, ParamIDs::fxOn, *fxOnToggle));

        chorusRate  = mkKnob (ParamIDs::chorusRate,  "Rate");
        chorusDepth = mkKnob (ParamIDs::chorusDepth, "Depth");
        phaserRate  = mkKnob (ParamIDs::phaserRate,  "Rate");
        phaserDepth = mkKnob (ParamIDs::phaserDepth, "Depth");
        delayTime     = mkKnob (ParamIDs::delayTime,     "Time");
        delayFeedback = mkKnob (ParamIDs::delayFeedback, "Feedback");
        delayMix      = mkKnob (ParamIDs::delayMix,      "Mix");
        reverbTypeCombo = mkCombo (ParamIDs::reverbType, Choices::reverbTypes());
        reverbSize  = mkKnob (ParamIDs::reverbSize,  "Size");
        reverbDecay = mkKnob (ParamIDs::reverbDecay, "Decay");
        reverbMix   = mkKnob (ParamIDs::reverbMix,   "Mix");

        //-------------------------------------------------------------- arp --
        arpOnToggle = std::make_unique<LampToggle> ("ON", "OFF");
        addAndMakeVisible (*arpOnToggle);
        buttonAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>
                                     (proc.parameters, ParamIDs::arpOn, *arpOnToggle));
        for (int i = 0; i < 8; ++i)
        {
            auto pad = std::make_unique<StepPad>();
            pad->setClickingTogglesState (true);
            addAndMakeVisible (*pad);
            buttonAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>
                                         (proc.parameters, juce::String ("ARP_STEP") + juce::String (i + 1), *pad));
            stepPads.push_back (std::move (pad));
        }
        arpRateKnob = mkKnob (ParamIDs::arpRate, "Rate");
        octDown = std::make_unique<NavArrowButton> (true);
        octUp   = std::make_unique<NavArrowButton> (false);
        addAndMakeVisible (*octDown); addAndMakeVisible (*octUp);
        octDown->onClick = [this] { stepOctave (-1); };
        octUp->onClick   = [this] { stepOctave (1);  };
        octValueLabel.setText ("1", juce::dontSendNotification);
        octValueLabel.setFont (ArabesqueLookAndFeel::captionFont (13.0f));
        octValueLabel.setColour (juce::Label::textColourId, textWarm);
        octValueLabel.setJustificationType (juce::Justification::centred);
        addAndMakeVisible (octValueLabel);

        //--------------------------------------------------- centre master --
        vibratoKnob = mkKnob (ParamIDs::vibrato, "Vibrato");
        glideKnob   = mkKnob (ParamIDs::glide,   "Glide");
        mainKnob    = mkKnob (ParamIDs::mainGain, "MAIN", true);
        driveKnob   = mkKnob (ParamIDs::drive,   "Drive");
        panKnob     = mkKnob (ParamIDs::pan,     "Pan");

        //------------------------------------------------------- keyboard ----
        pitchWheel = std::make_unique<WheelControl> ("PITCH", true,  proc.uiPitchBend);
        modWheel   = std::make_unique<WheelControl> ("MOD",   false, proc.uiModWheel);
        modWheel->setValue (0.0f);
        addAndMakeVisible (*pitchWheel);
        addAndMakeVisible (*modWheel);

        keyboard.setLowestVisibleKey (24);
        addAndMakeVisible (keyboard);
    }

    //==========================================================================
    void stepPreset (int delta)
    {
        const int n = (int) ArabesqueAudioProcessor::numPresets();
        int idx = (proc.getCurrentProgram() + delta) % n;
        if (idx < 0) idx += n;
        proc.setCurrentProgram (idx);
        refreshPresetLabel();
        refreshAllKnobTexts();
    }

    void refreshPresetLabel()
    {
        const auto name = proc.getCurrentPresetName();
        presetNameLabel.setText (name, juce::dontSendNotification);
        lastShownPresetName = name;
    }

    void refreshAllKnobTexts()
    {
        for (auto* k : allKnobs())
            if (k != nullptr) k->refreshValueText();
        octaveLabelRefresh();
    }

    std::vector<KnobWithLabel*> allKnobs()
    {
        return { osc1Octave.get(), osc1Semi.get(), osc1Fine.get(), osc1Level.get(),
                 osc2Octave.get(), osc2Semi.get(), osc2Fine.get(), osc2Level.get(),
                 mixOsc1.get(), mixOsc2.get(), mixNoise.get(), mixSub.get(),
                 cutoffKnob.get(), resonanceKnob.get(), keyFollowKnob.get(),
                 envA.get(), envD.get(), envS.get(), envR.get(),
                 lfoRateKnob.get(), lfoDepthKnob.get(),
                 chorusRate.get(), chorusDepth.get(), phaserRate.get(), phaserDepth.get(),
                 delayTime.get(), delayFeedback.get(), delayMix.get(),
                 reverbSize.get(), reverbDecay.get(), reverbMix.get(), arpRateKnob.get(),
                 vibratoKnob.get(), glideKnob.get(), mainKnob.get(), driveKnob.get(), panKnob.get() };
    }

    void selectEnvTab (int idx)
    {
        envTabIndex = idx;
        static const char* ids[3][4] = {
            { ParamIDs::ampAttack, ParamIDs::ampDecay, ParamIDs::ampSustain, ParamIDs::ampRelease },
            { ParamIDs::fltAttack, ParamIDs::fltDecay, ParamIDs::fltSustain, ParamIDs::fltRelease },
            { ParamIDs::pchAttack, ParamIDs::pchDecay, ParamIDs::pchSustain, ParamIDs::pchRelease }
        };
        envA->attach (proc.parameters, ids[idx][0]);
        envD->attach (proc.parameters, ids[idx][1]);
        envS->attach (proc.parameters, ids[idx][2]);
        envR->attach (proc.parameters, ids[idx][3]);
        for (int i = 0; i < 3; ++i)
            envTabs[(size_t) i]->setToggleState (i == idx, juce::dontSendNotification);
    }

    void stepOctave (int delta)
    {
        if (auto* p = proc.parameters.getParameter (ParamIDs::arpOctave))
        {
            const int current = (int) proc.parameters.getRawParameterValue (ParamIDs::arpOctave)->load();
            const int next = juce::jlimit (1, 4, current + delta);
            if (auto* rp = dynamic_cast<juce::RangedAudioParameter*> (p))
                rp->setValueNotifyingHost (rp->convertTo0to1 ((float) next));
            octaveLabelRefresh();
        }
    }

    void octaveLabelRefresh()
    {
        octValueLabel.setText (juce::String ((int) proc.parameters.getRawParameterValue (ParamIDs::arpOctave)->load()),
                               juce::dontSendNotification);
    }

    void doSave()
    {
        proc.saveCurrentAsUserPreset (proc.getCurrentPresetName());
        refreshPresetLabel();
    }

    void showPresetBrowser()
    {
        juce::PopupMenu menu;
        juce::PopupMenu factory;
        for (size_t i = 0; i < ArabesqueAudioProcessor::numPresets(); ++i)
            factory.addItem ((int) i + 1, proc.getProgramName ((int) i), true, (int) i == proc.getCurrentProgram());
        menu.addSubMenu ("Factory", factory);

        auto userFiles = proc.getUserPresets();
        if (! userFiles.empty())
        {
            juce::PopupMenu user;
            for (size_t i = 0; i < userFiles.size(); ++i)
                user.addItem (100 + (int) i, userFiles[i].getFileNameWithoutExtension());
            menu.addSubMenu ("User", user);
        }

        menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (browseButton.get()),
                            [safeThis = juce::Component::SafePointer<Content> (this), userFiles] (int result)
                            {
                                if (safeThis == nullptr) return;
                                if (result >= 1 && result <= (int) ArabesqueAudioProcessor::numPresets())
                                {
                                    safeThis->proc.setCurrentProgram (result - 1);
                                    safeThis->refreshPresetLabel();
                                    safeThis->refreshAllKnobTexts();
                                }
                                else if (result >= 100 && result - 100 < (int) userFiles.size())
                                {
                                    safeThis->proc.loadUserPreset (userFiles[(size_t) (result - 100)]);
                                    safeThis->refreshPresetLabel();
                                    safeThis->refreshAllKnobTexts();
                                }
                            });
    }

    void showAboutMenu()
    {
        juce::PopupMenu menu;
        menu.addItem ("ARABESQUE v1.0.0", false, false, [] {});
        menu.addItem ("Turkish Emotional Synthesizer", false, false, [] {});
        menu.addItem ("DA Production Instruments", false, false, [] {});
        menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (settingsButton.get()),
                            [] (int) {});
    }

    //==========================================================================
    void timerCallback() override
    {
        // arp step pads follow the audio thread
        const int currentStep = proc.getCurrentArpStepForUI();
        for (int i = 0; i < 8; ++i)
            stepPads[(size_t) i]->setActive (i == currentStep);

        // keep the preset label in sync with DAW program changes
        if (proc.getCurrentPresetName() != lastShownPresetName)
            refreshPresetLabel();

        // keep knob readouts in sync with host automation (cheap at 30 Hz)
        refreshAllKnobTexts();
    }

    //==========================================================================
    void paint (juce::Graphics& g) override
    {
        //----------------------------------------------------- background ---
        juce::ColourGradient bg (juce::Colour (0xff100d0a), 0.0f, 0.0f,
                                 juce::Colour (0xff070605), 0.0f, (float) H, false);
        g.setGradientFill (bg);
        g.fillAll();

        //------------------------------------------------------------------
        // header
        drawOrnament (g, 46.0f, 38.0f, 20.0f, gold);
        g.setColour (goldBright);
        g.setFont (ArabesqueLookAndFeel::serifFont (30.0f, true));
        g.drawText ("ARABESQUE", 78, 16, 230, 34, juce::Justification::centredLeft);
        g.setColour (textDim);
        g.setFont (ArabesqueLookAndFeel::captionFont (10.5f));
        g.drawText ("T U R K I S H   E M O T I O N A L   S Y N T H E S I Z E R", 80, 50, 320, 16,
                    juce::Justification::centredLeft);

        g.setColour (gold.withAlpha (0.85f));
        g.setFont (ArabesqueLookAndFeel::serifFont (22.0f, true));
        g.drawText ("DA", 380, 18, 60, 30, juce::Justification::centredLeft);
        g.setColour (textDim);
        g.setFont (ArabesqueLookAndFeel::captionFont (10.5f));
        g.drawText ("Production Instruments", 380, 46, 160, 16, juce::Justification::centredLeft);

        g.setColour (panelBorder.withAlpha (0.6f));
        g.fillRect (350, 18, 1, 50);

        // preset frame behind the name
        juce::Rectangle<float> presetFrame (627.0f, 20.0f, 286.0f, 40.0f);
        g.setColour (juce::Colour (0xff1a1510));
        g.fillRoundedRectangle (presetFrame, 5.0f);
        g.setColour (panelBorder.brighter (0.2f));
        g.drawRoundedRectangle (presetFrame, 5.0f, 1.0f);

        // gold divider with ornamental centre diamond
        juce::ColourGradient divider (juce::Colours::transparentBlack, 0.0f, 103.0f,
                                      gold.withAlpha (0.9f), (float) W * 0.5f, 103.0f, true);
        g.setGradientFill (divider);
        g.fillRect (0, 102, W, 2);
        drawDiamond (g, (float) W * 0.5f, 103.0f, 6.0f, gold);

        //------------------------------------------------------------------
        // left column panels
        g.setColour (textCaption);
        g.setFont (ArabesqueLookAndFeel::captionFont (12.0f));
        g.drawText ("O S C I L L A T O R S", 34, 118, 260, 16, juce::Justification::centredLeft);
        drawSectionPanel (g, { 20.0f, 140.0f, 450.0f, 160.0f }, "Osc 1");
        drawSectionPanel (g, { 20.0f, 308.0f, 450.0f, 160.0f }, "Osc 2");
        drawSectionPanel (g, { 20.0f, 496.0f, 450.0f, 150.0f }, "Mixer");

        //------------------------------------------------------------------
        // centre artwork
        {
            juce::Rectangle<float> art (486.0f, 118.0f, 566.0f, 352.0f);
            {
                juce::Graphics::ScopedSaveState save (g);
                juce::Path clip;
                clip.addRoundedRectangle (art.reduced (2.0f), 8.0f);
                g.reduceClipRegion (clip);
                g.drawImageWithin (artwork, (int) art.getX() + 2, (int) art.getY() + 2,
                                   (int) art.getWidth() - 4, (int) art.getHeight() - 4,
                                   juce::RectanglePlacement::stretchToFit, false);
                // subtle warm vignette over photo
                juce::ColourGradient v (juce::Colours::transparentBlack, 0.0f, art.getY(),
                                        juce::Colour (0x55000000), 0.0f, art.getBottom(), false);
                g.setGradientFill (v);
                g.fillRect (art);
            }
            g.setColour (gold.withAlpha (0.95f));
            g.drawRoundedRectangle (art.reduced (1.0f), 8.0f, 2.0f);
            g.setColour (juce::Colour (0xff5a4326));
            g.drawRoundedRectangle (art.reduced (5.5f), 6.0f, 1.0f);
            drawDiamond (g, art.getCentreX(), art.getBottom() + 2.0f, 7.0f, goldBright);
        }

        //------------------------------------------------------------------
        // right column panels
        drawSectionPanel (g, { 1068.0f, 118.0f, 444.0f, 212.0f }, "Filter");
        drawSectionPanel (g, { 1068.0f, 338.0f, 444.0f, 170.0f }, "Envelopes");
        drawSectionPanel (g, { 1068.0f, 516.0f, 444.0f, 186.0f }, "LFO");

        // LFO sub-labels
        g.setColour (textCaption);
        g.setFont (ArabesqueLookAndFeel::captionFont (10.0f));
        g.drawText ("Rate", 1376, 660, 60, 12, juce::Justification::centred);
        g.drawText ("Depth", 1442, 660, 60, 12, juce::Justification::centred);

        //------------------------------------------------------------------
        // fx strip
        drawSectionPanel (g, { 20.0f, 712.0f, 130.0f, 150.0f }, "FX");
        drawSectionPanel (g, { 158.0f, 712.0f, 227.0f, 150.0f }, "Chorus");
        drawSectionPanel (g, { 393.0f, 712.0f, 227.0f, 150.0f }, "Phaser");
        drawSectionPanel (g, { 628.0f, 712.0f, 280.0f, 150.0f }, "Delay");
        drawSectionPanel (g, { 916.0f, 712.0f, 360.0f, 150.0f }, "Reverb");
        drawSectionPanel (g, { 1284.0f, 712.0f, 228.0f, 150.0f }, "Arpeggiator");

        //------------------------------------------------------------------
        // keyboard strip
        g.setColour (juce::Colour (0xff12100d));
        g.fillRoundedRectangle (20.0f, 874.0f, 1492.0f, 144.0f, 8.0f);
        g.setColour (panelBorder.withAlpha (0.7f));
        g.drawRoundedRectangle (20.5f, 874.5f, 1491.0f, 143.0f, 8.0f, 1.0f);

        g.setColour (textCaption);
        g.setFont (ArabesqueLookAndFeel::captionFont (10.0f));
        g.drawText ("PITCH", 44, 996, 50, 12, juce::Justification::centred);
        g.drawText ("MOD",   100, 996, 50, 12, juce::Justification::centred);

        // bottom-right brand
        g.setColour (gold.withAlpha (0.8f));
        g.setFont (ArabesqueLookAndFeel::serifFont (17.0f, true));
        g.drawText ("ARABESQUE", 1380, 938, 132, 22, juce::Justification::centred);
        g.setColour (textDim);
        g.setFont (ArabesqueLookAndFeel::captionFont (9.0f));
        g.drawText ("B Y   S O U N D C R A F T", 1380, 962, 132, 12, juce::Justification::centred);
        drawDiamond (g, 1446.0f, 930.0f, 4.0f, gold.withAlpha (0.7f));
    }

    //==========================================================================
    void resized() override
    {
        //------------------------------ header
        prevPreset->setBounds (627, 20, 40, 40);
        nextPreset->setBounds (873, 20, 40, 40);
        presetNameLabel.setBounds (667, 20, 206, 40);
        saveButton->setBounds   (1004, 30, 62, 26);
        browseButton->setBounds (1076, 30, 72, 26);
        voicesLabel.setBounds   (1180, 30, 58, 26);
        voicesCombo->setBounds  (1242, 30, 76, 26);
        settingsButton->setBounds (1458, 18, 44, 44);

        //------------------------------ osc panels (140..300 / 308..468)
        auto layoutOscPanel = [] (int py, WaveDisplay* disp, juce::ComboBox* combo,
                                  KnobWithLabel* oct, KnobWithLabel* semi, KnobWithLabel* fine,
                                  KnobWithLabel* level)
        {
            disp->setBounds  (44, py + 36, 78, 62);
            combo->setBounds (140, py + 52, 96, 26);
            oct->setBounds   (238, py + 36, 52, 112);
            semi->setBounds  (290, py + 36, 52, 112);
            fine->setBounds  (342, py + 36, 52, 112);
            level->setBounds (396, py + 30, 66, 124);
        };
        layoutOscPanel (140, osc1WaveDisplay.get(), osc1WaveCombo.get(),
                        osc1Octave.get(), osc1Semi.get(), osc1Fine.get(), osc1Level.get());
        layoutOscPanel (308, osc2WaveDisplay.get(), osc2WaveCombo.get(),
                        osc2Octave.get(), osc2Semi.get(), osc2Fine.get(), osc2Level.get());

        //------------------------------ mixer (496..646)
        mixOsc1->setBounds  (36,  508, 96, 130);
        mixOsc2->setBounds  (146, 508, 96, 130);
        mixNoise->setBounds (256, 508, 96, 130);
        mixSub->setBounds   (366, 508, 92, 130);

        //------------------------------ centre knobs row
        vibratoKnob->setBounds (503, 500, 84, 160);
        glideKnob->setBounds   (598, 500, 84, 160);
        mainKnob->setBounds    (689, 472, 160, 220);
        driveKnob->setBounds   (858, 500, 84, 160);
        panKnob->setBounds     (953, 500, 84, 160);

        //------------------------------ filter (1068..1512, 118..330)
        filterTypeCombo->setBounds (1190, 148, 256, 26);
        cutoffKnob->setBounds    (1086, 176, 118, 146);
        resonanceKnob->setBounds (1214, 176, 118, 146);
        keyFollowKnob->setBounds (1346, 176, 110, 146);

        //------------------------------ envelopes (338..508)
        envTabs[0]->setBounds (1150, 368, 88, 26);
        envTabs[1]->setBounds (1246, 368, 88, 26);
        envTabs[2]->setBounds (1342, 368, 88, 26);
        envA->setBounds (1086, 400, 78, 100);
        envD->setBounds (1188, 400, 78, 100);
        envS->setBounds (1290, 400, 78, 100);
        envR->setBounds (1392, 400, 78, 100);

        //------------------------------ lfo (516..702)
        lfoShapeCombo->setBounds (1178, 546, 140, 26);
        syncToggle->setBounds    (1370, 546, 74, 26);
        lfoWaveDisplay->setBounds (1096, 582, 240, 76);
        lfoRateKnob->setBounds  (1362, 580, 60, 84);
        lfoDepthKnob->setBounds (1430, 580, 60, 84);

        //------------------------------ fx strip (712..862)
        fxOnToggle->setBounds (44, 768, 82, 30);
        chorusRate->setBounds  (170, 726, 100, 128);
        chorusDepth->setBounds (272, 726, 100, 128);
        phaserRate->setBounds  (405, 726, 100, 128);
        phaserDepth->setBounds (507, 726, 100, 128);
        delayTime->setBounds     (636, 726, 88, 128);
        delayFeedback->setBounds (724, 726, 88, 128);
        delayMix->setBounds      (812, 726, 88, 128);
        reverbTypeCombo->setBounds (1076, 724, 176, 24);
        reverbSize->setBounds  (936, 750, 108, 104);
        reverbDecay->setBounds (1050, 726, 108, 128);
        reverbMix->setBounds   (1164, 726, 100, 128);

        //------------------------------ arp (1284..1512)
        arpOnToggle->setBounds (1420, 722, 80, 26);
        for (int i = 0; i < 8; ++i)
        {
            const int col = i % 4, row = i / 4;
            stepPads[(size_t) i]->setBounds (1302 + col * 28, 762 + row * 28, 24, 24);
        }
        arpRateKnob->setBounds (1416, 752, 80, 104);
        octDown->setBounds (1302, 826, 34, 24);
        octUp->setBounds   (1372, 826, 34, 24);
        octValueLabel.setBounds (1338, 826, 32, 24);

        //------------------------------ keyboard area
        pitchWheel->setBounds (54, 890, 30, 98);
        modWheel->setBounds   (110, 890, 30, 98);
        keyboard.setBounds (170, 888, 1198, 126);
    }
};

//==============================================================================
ArabesqueAudioProcessorEditor::ArabesqueAudioProcessorEditor (ArabesqueAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    content = std::make_unique<Content> (p);
    addAndMakeVisible (*content);

    setSize (W, H);
    setResizable (true, true);
    setResizeLimits (1024, 683, 2048, 1365);
    getConstrainer()->setFixedAspectRatio ((double) W / (double) H);
}

ArabesqueAudioProcessorEditor::~ArabesqueAudioProcessorEditor() = default;

void ArabesqueAudioProcessorEditor::resized()
{
    const float s = (float) getWidth() / (float) W;
    content->setTransform (juce::AffineTransform::scale (s));
    content->setBounds (0, 0, (int) std::round (getWidth() / s), (int) std::round (getHeight() / s));
}
