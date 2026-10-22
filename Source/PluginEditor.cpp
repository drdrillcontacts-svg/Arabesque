#include "PluginEditor.h"
#include "BinaryData.h"
#include <functional>

using namespace ArabesqueColours;

namespace
{

// Logical size of the skin (the mockup is 1536x1024).
constexpr int W = 1536, H = 1024;

//==============================================================================
class WaveDisplay final : public juce::Component
{
public:
    explicit WaveDisplay (std::function<int()> getShape) : shapeFn (std::move (getShape))
    {
        setOpaque (true);
        setInterceptsMouseClicks (false, false);
    }

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
                case 0: y = 2.0f * t - 1.0f; break;
                case 1: y = t < 0.5f ? 1.0f : -1.0f; break;
                case 2: y = 1.0f - 4.0f * std::abs (t - 0.5f); break;
                case 3: y = std::sin (t * juce::MathConstants<float>::twoPi); break;
                case 4: y = std::sin (std::floor (t * 6.0f) * 1.7f) * 0.9f; break;
                default: break;
            }
            const float px = area.getX() + t * area.getWidth();
            const float py = area.getCentreY() - y * area.getHeight() * 0.42f;
            if (i == 0) p.startNewSubPath (px, py); else p.lineTo (px, py);
        }
        g.setColour (gold.withAlpha (0.9f));
        g.strokePath (p, juce::PathStrokeType (1.4f));
    }

private:
    std::function<int()> shapeFn;
};

//==============================================================================
class WheelControl final : public juce::Component
{
public:
    WheelControl (bool springs, std::atomic<float>& target, float minValue, float maxValue, float initial)
        : springsToZero (springs), targetValue (target), minV (minValue), maxV (maxValue)
    {
        setOpaque (false);
        setValue (initial);
    }

    void setValue (float v)
    {
        value = juce::jlimit (minV, maxV, v);
        targetValue.store (value);
        repaint();
    }

    void paint (juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat();
        auto groove = b.withSizeKeepingCentre (14.0f, b.getHeight()).reduced (0.0f, 4.0f);
        g.setColour (juce::Colour (0xff171310));
        g.fillRoundedRectangle (groove, 6.0f);
        g.setColour (juce::Colour (0xff3a3128));
        g.drawRoundedRectangle (groove.reduced (0.5f), 6.0f, 1.0f);

        const float span = juce::jmax (0.0001f, maxV - minV);
        const float pos = (value - minV) / span;
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
    void mouseDoubleClick (const juce::MouseEvent&) override { setValue (springsToZero ? 0.0f : minV); }

private:
    bool springsToZero;
    std::atomic<float>& targetValue;
    float minV, maxV;
    float value = 0.0f;
    int dragStartY = 0;
    float dragStartValue = 0.0f;
};

//==============================================================================
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
class HitButton final : public juce::Button
{
public:
    explicit HitButton (const juce::String& name) : juce::Button (name)
    {
        setMouseCursor (juce::MouseCursor::PointingHandCursor);
    }

    void paintButton (juce::Graphics& g, bool highlighted, bool down) override
    {
        if (highlighted || down)
        {
            g.setColour (gold.withAlpha (down ? 0.22f : 0.12f));
            g.fillRoundedRectangle (getLocalBounds().toFloat(), 4.0f);
        }
    }
};

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
        setOpaque (true);
        setLookAndFeel (&lnf);

        if (BinaryData::GuiSkin_pngSize > 0)
            skin = juce::ImageCache::getFromMemory (BinaryData::GuiSkin_png, BinaryData::GuiSkin_pngSize);
        if (! skin.isValid())
            skin = juce::ImageCache::getFromMemory (BinaryData::Artwork_jpg, BinaryData::Artwork_jpgSize);

        buildControls();
        refreshPresetLabel();
        startTimerHz (24);
    }

    ~Content() override
    {
        stopTimer();
        keyboard.setLookAndFeel (nullptr);
        setLookAndFeel (nullptr);
    }

private:
    ArabesqueAudioProcessor& proc;
    ArabesqueLookAndFeel lnf;
    juce::Image skin;

    std::unique_ptr<HitButton> prevPreset, nextPreset, saveButton, browseButton, settingsButton;
    juce::Label presetNameLabel;
    std::unique_ptr<juce::ComboBox> voicesCombo;
    juce::Label voicesLabel;

    std::unique_ptr<juce::ComboBox> osc1WaveCombo, osc2WaveCombo;
    std::unique_ptr<WaveDisplay> osc1WaveDisplay, osc2WaveDisplay, lfoWaveDisplay;
    std::unique_ptr<KnobWithLabel> osc1Octave, osc1Semi, osc1Fine, osc1Level;
    std::unique_ptr<KnobWithLabel> osc2Octave, osc2Semi, osc2Fine, osc2Level;

    std::unique_ptr<KnobWithLabel> mixOsc1, mixOsc2, mixNoise, mixSub;

    std::unique_ptr<juce::ComboBox> filterTypeCombo;
    std::unique_ptr<KnobWithLabel> cutoffKnob, resonanceKnob;
    std::unique_ptr<FaderWithLabel> keyFollowFader;

    std::vector<std::unique_ptr<juce::TextButton>> envTabs;
    std::unique_ptr<KnobWithLabel> envA, envD, envS, envR;
    int envTabIndex = 0;

    std::unique_ptr<juce::ComboBox> lfoShapeCombo;
    std::unique_ptr<LampToggle> syncToggle;
    std::unique_ptr<KnobWithLabel> lfoRateKnob, lfoDepthKnob;

    std::unique_ptr<LampToggle> fxOnToggle;
    std::unique_ptr<KnobWithLabel> chorusRate, chorusDepth, phaserRate, phaserDepth;
    std::unique_ptr<KnobWithLabel> delayTime, delayFeedback, delayMix;
    std::unique_ptr<juce::ComboBox> reverbTypeCombo;
    std::unique_ptr<KnobWithLabel> reverbSize, reverbDecay, reverbMix;

    std::unique_ptr<LampToggle> arpOnToggle;
    std::vector<std::unique_ptr<StepPad>> stepPads;
    std::unique_ptr<KnobWithLabel> arpRateKnob;
    std::unique_ptr<HitButton> octDown, octUp;
    juce::Label octValueLabel;

    std::unique_ptr<KnobWithLabel> vibratoKnob, glideKnob, driveKnob, panKnob, mainKnob;

    std::unique_ptr<WheelControl> pitchWheel, modWheel;
    juce::MidiKeyboardComponent keyboard;

    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>> comboAttachments;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> buttonAttachments;

    juce::String lastShownPresetName;
    float scx = 1.0f, scy = 1.0f;

    juce::Rectangle<int> R (int x, int y, int w, int h) const
    {
        return { juce::roundToInt ((float) x * scx), juce::roundToInt ((float) y * scy),
                 juce::roundToInt ((float) w * scx), juce::roundToInt ((float) h * scy) };
    }

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

    int paramChoice (const char* id) const
    {
        if (auto* p = proc.parameters.getRawParameterValue (id))
            return (int) p->load();
        return 0;
    }

    void buildControls()
    {
        prevPreset = std::make_unique<HitButton> ("prev");
        nextPreset = std::make_unique<HitButton> ("next");
        addAndMakeVisible (*prevPreset); addAndMakeVisible (*nextPreset);
        prevPreset->onClick = [this] { stepPreset (-1); };
        nextPreset->onClick = [this] { stepPreset (1); };

        presetNameLabel.setFont (ArabesqueLookAndFeel::captionFont (15.0f));
        presetNameLabel.setColour (juce::Label::textColourId, textWarm);
        presetNameLabel.setColour (juce::Label::backgroundColourId, juce::Colour (0xff1a1510));
        presetNameLabel.setOpaque (true);
        presetNameLabel.setJustificationType (juce::Justification::centred);
        addAndMakeVisible (presetNameLabel);

        saveButton = std::make_unique<HitButton> ("save");
        browseButton = std::make_unique<HitButton> ("browse");
        addAndMakeVisible (*saveButton); addAndMakeVisible (*browseButton);
        saveButton->onClick = [this] { doSave(); };
        browseButton->onClick = [this] { showPresetBrowser(); };

        voicesLabel.setText ("Voices:", juce::dontSendNotification);
        voicesLabel.setFont (ArabesqueLookAndFeel::captionFont (12.5f));
        voicesLabel.setColour (juce::Label::textColourId, textDim);
        voicesLabel.setColour (juce::Label::backgroundColourId, background);
        voicesLabel.setOpaque (true);
        voicesLabel.setJustificationType (juce::Justification::centredRight);
        addAndMakeVisible (voicesLabel);
        voicesCombo = mkCombo (ParamIDs::voices, Choices::voiceCounts());

        settingsButton = std::make_unique<HitButton> ("settings");
        addAndMakeVisible (*settingsButton);
        settingsButton->onClick = [this] { showAboutMenu(); };

        osc1WaveCombo = mkCombo (ParamIDs::osc1Wave, Choices::waveforms());
        osc2WaveCombo = mkCombo (ParamIDs::osc2Wave, Choices::waveforms());

        osc1WaveDisplay = std::make_unique<WaveDisplay> ([this] { return paramChoice (ParamIDs::osc1Wave); });
        osc2WaveDisplay = std::make_unique<WaveDisplay> ([this] { return paramChoice (ParamIDs::osc2Wave); });
        addAndMakeVisible (*osc1WaveDisplay);
        addAndMakeVisible (*osc2WaveDisplay);
        osc1WaveCombo->onChange = [this] { osc1WaveDisplay->repaint(); };
        osc2WaveCombo->onChange = [this] { osc2WaveDisplay->repaint(); };

        osc1Octave = mkKnob (ParamIDs::osc1Octave, "Octave");
        osc1Semi   = mkKnob (ParamIDs::osc1Semi,   "Semitone");
        osc1Fine   = mkKnob (ParamIDs::osc1Fine,   "Fine");
        osc1Level  = mkKnob (ParamIDs::mixOsc1,    "Level");
        osc2Octave = mkKnob (ParamIDs::osc2Octave, "Octave");
        osc2Semi   = mkKnob (ParamIDs::osc2Semi,   "Semitone");
        osc2Fine   = mkKnob (ParamIDs::osc2Fine,   "Fine");
        osc2Level  = mkKnob (ParamIDs::mixOsc2,    "Level");

        mixOsc1  = mkKnob (ParamIDs::mixOsc1,  "Osc 1");
        mixOsc2  = mkKnob (ParamIDs::mixOsc2,  "Osc 2");
        mixNoise = mkKnob (ParamIDs::mixNoise, "Noise");
        mixSub   = mkKnob (ParamIDs::mixSub,   "Sub");

        filterTypeCombo = mkCombo (ParamIDs::filterType, Choices::filterTypes());
        cutoffKnob    = mkKnob (ParamIDs::cutoff,    "Cutoff");
        resonanceKnob = mkKnob (ParamIDs::resonance, "Resonance");
        keyFollowFader = std::make_unique<FaderWithLabel> (proc.parameters, ParamIDs::keyFollow, &lnf);
        addAndMakeVisible (*keyFollowFader);

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

        lfoShapeCombo = mkCombo (ParamIDs::lfoShape, Choices::lfoShapes());
        syncToggle = std::make_unique<LampToggle> ("Sync", "Sync");
        addAndMakeVisible (*syncToggle);
        buttonAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>
                                     (proc.parameters, ParamIDs::lfoSync, *syncToggle));
        lfoWaveDisplay = std::make_unique<WaveDisplay> ([this] { return paramChoice (ParamIDs::lfoShape); });
        addAndMakeVisible (*lfoWaveDisplay);
        lfoShapeCombo->onChange = [this] { lfoWaveDisplay->repaint(); };
        lfoRateKnob  = mkKnob (ParamIDs::lfoRate,  "Rate");
        lfoDepthKnob = mkKnob (ParamIDs::lfoDepth, "Depth");

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
        octDown = std::make_unique<HitButton> ("octDown");
        octUp   = std::make_unique<HitButton> ("octUp");
        addAndMakeVisible (*octDown); addAndMakeVisible (*octUp);
        octDown->onClick = [this] { stepOctave (-1); };
        octUp->onClick   = [this] { stepOctave (1);  };
        octValueLabel.setFont (ArabesqueLookAndFeel::captionFont (13.0f));
        octValueLabel.setColour (juce::Label::textColourId, textWarm);
        octValueLabel.setColour (juce::Label::backgroundColourId, panelFill);
        octValueLabel.setOpaque (true);
        octValueLabel.setJustificationType (juce::Justification::centred);
        addAndMakeVisible (octValueLabel);

        vibratoKnob = mkKnob (ParamIDs::vibrato, "Vibrato");
        glideKnob   = mkKnob (ParamIDs::glide,   "Glide");
        mainKnob    = mkKnob (ParamIDs::mainGain, "MAIN", true);
        driveKnob   = mkKnob (ParamIDs::drive,   "Drive");
        panKnob     = mkKnob (ParamIDs::pan,     "Pan");

        pitchWheel = std::make_unique<WheelControl> (true,  proc.uiPitchBend, -1.0f, 1.0f, 0.0f);
        modWheel   = std::make_unique<WheelControl> (false, proc.uiModWheel,   0.0f, 1.0f, 0.0f);
        addAndMakeVisible (*pitchWheel);
        addAndMakeVisible (*modWheel);

        keyboard.setLookAndFeel (&lnf);
        keyboard.setLowestVisibleKey (24);
        keyboard.setAvailableRange (24, 96);
        keyboard.setBlackNoteLengthProportion (0.64f);
        keyboard.setOctaveForMiddleC (4);
        keyboard.setWantsKeyboardFocus (true);
        addAndMakeVisible (keyboard);
    }

    void stepPreset (int delta)
    {
        const int n = (int) ArabesqueAudioProcessor::numPresets();
        if (n <= 0) return;
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
        if (keyFollowFader != nullptr) keyFollowFader->refreshValueText();
        octaveLabelRefresh();
        if (osc1WaveDisplay) osc1WaveDisplay->repaint();
        if (osc2WaveDisplay) osc2WaveDisplay->repaint();
        if (lfoWaveDisplay)  lfoWaveDisplay->repaint();
    }

    std::vector<KnobWithLabel*> allKnobs()
    {
        return { osc1Octave.get(), osc1Semi.get(), osc1Fine.get(), osc1Level.get(),
                 osc2Octave.get(), osc2Semi.get(), osc2Fine.get(), osc2Level.get(),
                 mixOsc1.get(), mixOsc2.get(), mixNoise.get(), mixSub.get(),
                 cutoffKnob.get(), resonanceKnob.get(),
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

    void timerCallback() override
    {
        const int currentStep = proc.getCurrentArpStepForUI();
        for (int i = 0; i < 8; ++i)
            if ((int) stepPads.size() > i)
                stepPads[(size_t) i]->setActive (i == currentStep);

        if (proc.getCurrentPresetName() != lastShownPresetName)
            refreshPresetLabel();

        refreshAllKnobTexts();
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (background);
        if (skin.isValid())
            g.drawImage (skin, getLocalBounds().toFloat());
    }

    void resized() override
    {
        scx = (float) getWidth()  / (float) W;
        scy = (float) getHeight() / (float) H;

        prevPreset->setBounds (R (627, 20, 40, 40));
        nextPreset->setBounds (R (873, 20, 40, 40));
        presetNameLabel.setBounds (R (667, 20, 206, 40));
        saveButton->setBounds   (R (1004, 24, 62, 36));
        browseButton->setBounds (R (1076, 24, 78, 36));
        voicesLabel.setBounds   (R (1174, 30, 58, 26));
        voicesCombo->setBounds  (R (1234, 28, 86, 28));
        settingsButton->setBounds (R (1458, 18, 44, 44));

        auto layoutOscPanel = [this] (int py, WaveDisplay* disp, juce::ComboBox* combo,
                                      KnobWithLabel* oct, KnobWithLabel* semi, KnobWithLabel* fine,
                                      KnobWithLabel* level)
        {
            disp->setBounds  (R (44, py + 36, 78, 62));
            combo->setBounds (R (140, py + 52, 96, 26));
            oct->setBounds   (R (238, py + 28, 52, 120));
            semi->setBounds  (R (290, py + 28, 52, 120));
            fine->setBounds  (R (342, py + 28, 52, 120));
            level->setBounds (R (396, py + 22, 66, 130));
        };
        layoutOscPanel (140, osc1WaveDisplay.get(), osc1WaveCombo.get(),
                        osc1Octave.get(), osc1Semi.get(), osc1Fine.get(), osc1Level.get());
        layoutOscPanel (308, osc2WaveDisplay.get(), osc2WaveCombo.get(),
                        osc2Octave.get(), osc2Semi.get(), osc2Fine.get(), osc2Level.get());

        mixOsc1->setBounds  (R (36,  508, 96, 130));
        mixOsc2->setBounds  (R (146, 508, 96, 130));
        mixNoise->setBounds (R (256, 508, 96, 130));
        mixSub->setBounds   (R (366, 508, 92, 130));

        vibratoKnob->setBounds (R (503, 500, 84, 160));
        glideKnob->setBounds   (R (598, 500, 84, 160));
        mainKnob->setBounds    (R (689, 478, 160, 210));
        driveKnob->setBounds   (R (858, 500, 84, 160));
        panKnob->setBounds     (R (953, 500, 84, 160));

        filterTypeCombo->setBounds (R (1190, 148, 256, 26));
        cutoffKnob->setBounds    (R (1086, 176, 118, 146));
        resonanceKnob->setBounds (R (1214, 176, 118, 146));
        keyFollowFader->setBounds (R (1360, 170, 90, 152));

        envTabs[0]->setBounds (R (1150, 368, 88, 26));
        envTabs[1]->setBounds (R (1246, 368, 88, 26));
        envTabs[2]->setBounds (R (1342, 368, 88, 26));
        envA->setBounds (R (1086, 400, 78, 100));
        envD->setBounds (R (1188, 400, 78, 100));
        envS->setBounds (R (1290, 400, 78, 100));
        envR->setBounds (R (1392, 400, 78, 100));

        lfoShapeCombo->setBounds (R (1178, 546, 140, 26));
        syncToggle->setBounds    (R (1370, 546, 74, 26));
        lfoWaveDisplay->setBounds (R (1096, 582, 240, 76));
        lfoRateKnob->setBounds  (R (1362, 572, 60, 96));
        lfoDepthKnob->setBounds (R (1430, 572, 60, 96));

        fxOnToggle->setBounds (R (44, 768, 82, 30));
        chorusRate->setBounds  (R (170, 726, 100, 128));
        chorusDepth->setBounds (R (272, 726, 100, 128));
        phaserRate->setBounds  (R (405, 726, 100, 128));
        phaserDepth->setBounds (R (507, 726, 100, 128));
        delayTime->setBounds     (R (636, 726, 88, 128));
        delayFeedback->setBounds (R (724, 726, 88, 128));
        delayMix->setBounds      (R (812, 726, 88, 128));
        reverbTypeCombo->setBounds (R (1076, 724, 176, 24));
        reverbSize->setBounds  (R (936, 750, 108, 104));
        reverbDecay->setBounds (R (1050, 726, 108, 128));
        reverbMix->setBounds   (R (1164, 726, 100, 128));

        arpOnToggle->setBounds (R (1420, 722, 80, 26));
        for (int i = 0; i < 8; ++i)
        {
            const int col = i % 4, row = i / 4;
            stepPads[(size_t) i]->setBounds (R (1302 + col * 28, 762 + row * 28, 24, 24));
        }
        arpRateKnob->setBounds (R (1416, 752, 80, 104));
        octDown->setBounds (R (1302, 826, 34, 24));
        octUp->setBounds   (R (1372, 826, 34, 24));
        octValueLabel.setBounds (R (1338, 826, 32, 24));

        pitchWheel->setBounds (R (54, 890, 30, 98));
        modWheel->setBounds   (R (110, 890, 30, 98));
        keyboard.setBounds (R (170, 888, 1198, 126));

        const int whiteKeys = 43; // C1..C7
        const float kw = (float) keyboard.getWidth() / (float) whiteKeys;
        if (kw > 1.0f)
            keyboard.setKeyWidth (kw);
    }
};

//==============================================================================
ArabesqueAudioProcessorEditor::ArabesqueAudioProcessorEditor (ArabesqueAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    content = std::make_unique<Content> (p);
    addAndMakeVisible (*content);

    // Default size fits 1080p DAW windows; skin is 1536x1024 and scales.
    setSize (1152, 768);
    setResizable (true, true);
    setResizeLimits (768, 512, 1920, 1280);
    if (auto* c = getConstrainer())
        c->setFixedAspectRatio ((double) W / (double) H);
}

ArabesqueAudioProcessorEditor::~ArabesqueAudioProcessorEditor() = default;

void ArabesqueAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff0b0a09));
}

void ArabesqueAudioProcessorEditor::resized()
{
    if (content != nullptr)
        content->setBounds (getLocalBounds());
}
