#include "PluginEditor.h"
#include <functional>

using namespace ArabesqueColours;

namespace
{
constexpr int W = 1280, H = 760;

class WaveDisplay final : public juce::Component
{
public:
    enum Kind { osc, lfo };
    WaveDisplay (Kind k, std::function<int()> getShape) : kind (k), shapeFn (std::move (getShape))
    {
        setInterceptsMouseClicks (false, false);
    }

    void paint (juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat();
        g.setColour (juce::Colour (0xff10100c));
        g.fillRoundedRectangle (b, 3.0f);
        g.setColour (juce::Colour (0xff2e2820));
        g.drawRoundedRectangle (b.reduced (0.5f), 3.0f, 1.0f);

        int shape = shapeFn != nullptr ? shapeFn() : 0;
        // LFO: 0 sine, 1 tri, 2 saw, 3 square, 4 s&h
        // OSC: 0 saw, 1 square, 2 tri, 3 sine
        if (kind == osc)
        {
            static const int map[] = { 2, 3, 1, 0 };
            shape = map[juce::jlimit (0, 3, shape)];
        }

        auto area = b.reduced (6.0f, 7.0f);
        juce::Path p;
        const int n = 80;
        for (int i = 0; i <= n; ++i)
        {
            const float t = (float) i / (float) n;
            float y = 0.0f;
            switch (shape)
            {
                case 0: y = std::sin (t * juce::MathConstants<float>::twoPi); break;
                case 1: y = 1.0f - 4.0f * std::abs (t - 0.5f); break;
                case 2: y = 2.0f * t - 1.0f; break;
                case 3: y = t < 0.5f ? 1.0f : -1.0f; break;
                case 4: y = std::sin (std::floor (t * 6.0f) * 1.7f) * 0.9f; break;
                default: break;
            }
            const float px = area.getX() + t * area.getWidth();
            const float py = area.getCentreY() - y * area.getHeight() * 0.38f;
            if (i == 0) p.startNewSubPath (px, py); else p.lineTo (px, py);
        }
        g.setColour (gold.withAlpha (0.85f));
        g.strokePath (p, juce::PathStrokeType (1.3f));
    }

private:
    Kind kind;
    std::function<int()> shapeFn;
};

class WheelControl final : public juce::Component
{
public:
    WheelControl (bool springs, std::atomic<float>& target, float minValue, float maxValue, float initial)
        : springsToZero (springs), targetValue (target), minV (minValue), maxV (maxValue)
    {
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
        auto groove = b.withSizeKeepingCentre (11.0f, b.getHeight() - 4.0f);
        g.setColour (juce::Colour (0xff12100e));
        g.fillRoundedRectangle (groove, 5.0f);
        g.setColour (juce::Colour (0xff2e2820));
        g.drawRoundedRectangle (groove.reduced (0.5f), 5.0f, 1.0f);

        const float pos = (value - minV) / juce::jmax (0.0001f, maxV - minV);
        const float ty = juce::jmap (pos, groove.getBottom() - 14.0f, groove.getY() + 2.0f);
        juce::Rectangle<float> thumb (groove.getX() - 2.0f, ty, groove.getWidth() + 4.0f, 12.0f);
        g.setColour (juce::Colour (0xff2a2620));
        g.fillRoundedRectangle (thumb, 2.0f);
        g.setColour (gold);
        g.fillRect (thumb.getX() + 2.0f, thumb.getCentreY() - 0.6f, thumb.getWidth() - 4.0f, 1.2f);
    }

    void mouseDown (const juce::MouseEvent& e) override { dragStartY = e.y; dragStartValue = value; }
    void mouseDrag (const juce::MouseEvent& e) override
    {
        setValue (dragStartValue + (float) (dragStartY - e.y) / 110.0f * (maxV - minV) * 0.5f);
    }
    void mouseUp (const juce::MouseEvent&) override { if (springsToZero) setValue (0.0f); }
    void mouseDoubleClick (const juce::MouseEvent&) override { setValue (springsToZero ? 0.0f : minV); }

private:
    bool springsToZero;
    std::atomic<float>& targetValue;
    float minV, maxV, value = 0.0f;
    int dragStartY = 0;
    float dragStartValue = 0.0f;
};

class StepPad final : public juce::TextButton
{
public:
    void setActive (bool a) { if (active != a) { active = a; repaint(); } }

    void paintButton (juce::Graphics& g, bool highlighted, bool) override
    {
        auto b = getLocalBounds().toFloat().reduced (1.0f);
        const bool on = getToggleState();
        g.setColour (on ? juce::Colour (0xff2a1c12) : juce::Colour (0xff141210));
        g.fillRoundedRectangle (b, 2.0f);
        g.setColour (active ? redGlow : on ? gold : juce::Colour (0xff2e2820));
        g.drawRoundedRectangle (b, 2.0f, active ? 1.6f : 1.0f);
        if (on)
        {
            g.setColour ((active ? redGlow : gold).withAlpha (0.85f));
            g.fillEllipse (b.withSizeKeepingCentre (5.0f, 5.0f));
        }
        if (highlighted)
        {
            g.setColour (gold.withAlpha (0.10f));
            g.fillRoundedRectangle (b, 2.0f);
        }
    }
private:
    bool active = false;
};

class LampToggle final : public juce::TextButton
{
public:
    LampToggle (const juce::String& onText, const juce::String& offText)
        : labelOn (onText), labelOff (offText) { setClickingTogglesState (true); }

    void paintButton (juce::Graphics& g, bool highlighted, bool) override
    {
        auto b = getLocalBounds().toFloat().reduced (0.5f);
        const bool on = getToggleState();
        g.setColour (on ? juce::Colour (0xff2a1610) : juce::Colour (0xff161410));
        g.fillRoundedRectangle (b, 3.0f);
        g.setColour (on ? redGlow.withAlpha (0.9f) : juce::Colour (0xff3a342c));
        g.drawRoundedRectangle (b, 3.0f, 1.0f);

        auto lamp = b.removeFromLeft (16.0f).withSizeKeepingCentre (6.0f, 6.0f);
        g.setColour (on ? redGlow.brighter (0.2f) : juce::Colour (0xff3a342c));
        g.fillEllipse (lamp);

        g.setColour (on ? juce::Colour (0xfff0c8b8) : textDim);
        g.setFont (ArabesqueLookAndFeel::uiFont (11.0f));
        g.drawText (on ? labelOn : labelOff, b, juce::Justification::centred);
        if (highlighted) { g.setColour (gold.withAlpha (0.08f)); g.fillRoundedRectangle (getLocalBounds().toFloat(), 3.0f); }
    }
private:
    juce::String labelOn, labelOff;
};

void drawPanel (juce::Graphics& g, juce::Rectangle<float> r, const juce::String& title)
{
    g.setColour (panelFill);
    g.fillRoundedRectangle (r, 6.0f);
    g.setColour (panelBorder.withAlpha (0.85f));
    g.drawRoundedRectangle (r.reduced (0.5f), 6.0f, 1.0f);
    if (title.isNotEmpty())
    {
        g.setColour (textCaption);
        g.setFont (ArabesqueLookAndFeel::uiFont (11.0f));
        g.drawText (title, (int) r.getX() + 12, (int) r.getY() + 6, 280, 16, juce::Justification::left);
    }
}

void drawStar (juce::Graphics& g, juce::Point<float> c, float r, juce::Colour col, float stroke)
{
    auto sq = [&] (float rot, float scale)
    {
        juce::Path p;
        p.addRectangle (-1.0f, -1.0f, 2.0f, 2.0f);
        g.setColour (col);
        g.strokePath (p, juce::PathStrokeType (stroke),
                      juce::AffineTransform::rotation (rot).scaled (r * scale).translated (c.x, c.y));
    };
    sq (0.0f, 1.0f);
    sq (juce::MathConstants<float>::pi * 0.25f, 1.0f);
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
        setOpaque (true);
        setLookAndFeel (&lnf);
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

    std::unique_ptr<juce::TextButton> prevPreset, nextPreset, saveButton, browseButton;
    juce::Label presetNameLabel, voicesLabel;
    std::unique_ptr<juce::ComboBox> voicesCombo;

    std::unique_ptr<juce::ComboBox> osc1WaveCombo, osc2WaveCombo;
    std::unique_ptr<WaveDisplay> osc1WaveDisplay, osc2WaveDisplay, lfoWaveDisplay;
    std::unique_ptr<KnobWithLabel> osc1Octave, osc1Semi, osc1Fine, osc1Level, pwmKnob;
    std::unique_ptr<KnobWithLabel> osc2Octave, osc2Semi, osc2Fine, osc2Level, fmKnob;
    std::unique_ptr<KnobWithLabel> mixOsc1, mixOsc2, mixNoise, mixSub, uniDet, uniSpr;
    std::unique_ptr<juce::ComboBox> unisonCombo;

    std::unique_ptr<juce::ComboBox> filterTypeCombo;
    std::unique_ptr<KnobWithLabel> cutoffKnob, resonanceKnob, envAmtKnob, fDriveKnob;
    std::unique_ptr<FaderWithLabel> keyFollowFader;

    std::vector<std::unique_ptr<juce::TextButton>> envTabs;
    std::unique_ptr<KnobWithLabel> envA, envD, envS, envR;

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
    std::unique_ptr<juce::TextButton> octDown, octUp;
    juce::Label octValueLabel, octCaption;

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
        auto mkNav = [this] (const juce::String& t) {
            auto b = std::make_unique<juce::TextButton> (t);
            addAndMakeVisible (*b);
            return b;
        };
        prevPreset = mkNav ("<");
        nextPreset = mkNav (">");
        prevPreset->onClick = [this] { stepPreset (-1); };
        nextPreset->onClick = [this] { stepPreset (1); };

        presetNameLabel.setJustificationType (juce::Justification::centred);
        presetNameLabel.setColour (juce::Label::textColourId, textWarm);
        addAndMakeVisible (presetNameLabel);

        saveButton = mkNav ("SAVE");
        browseButton = mkNav ("BROWSE");
        saveButton->onClick = [this] { proc.saveCurrentAsUserPreset (proc.getCurrentPresetName()); refreshPresetLabel(); };
        browseButton->onClick = [this] { showPresetBrowser(); };

        voicesLabel.setText ("Voices", juce::dontSendNotification);
        voicesLabel.setColour (juce::Label::textColourId, textDim);
        voicesLabel.setJustificationType (juce::Justification::centredRight);
        addAndMakeVisible (voicesLabel);
        voicesCombo = mkCombo (ParamIDs::voices, Choices::voiceCounts());

        osc1WaveCombo = mkCombo (ParamIDs::osc1Wave, Choices::waveforms());
        osc2WaveCombo = mkCombo (ParamIDs::osc2Wave, Choices::waveforms());
        osc1WaveDisplay = std::make_unique<WaveDisplay> (WaveDisplay::osc, [this] { return paramChoice (ParamIDs::osc1Wave); });
        osc2WaveDisplay = std::make_unique<WaveDisplay> (WaveDisplay::osc, [this] { return paramChoice (ParamIDs::osc2Wave); });
        addAndMakeVisible (*osc1WaveDisplay); addAndMakeVisible (*osc2WaveDisplay);
        osc1WaveCombo->onChange = [this] { osc1WaveDisplay->repaint(); };
        osc2WaveCombo->onChange = [this] { osc2WaveDisplay->repaint(); };

        osc1Octave = mkKnob (ParamIDs::osc1Octave, "Oct");
        osc1Semi   = mkKnob (ParamIDs::osc1Semi,   "Semi");
        osc1Fine   = mkKnob (ParamIDs::osc1Fine,   "Fine");
        pwmKnob    = mkKnob (ParamIDs::pwm,        "PWM");
        osc1Level  = mkKnob (ParamIDs::mixOsc1,    "Level");
        osc2Octave = mkKnob (ParamIDs::osc2Octave, "Oct");
        osc2Semi   = mkKnob (ParamIDs::osc2Semi,   "Semi");
        osc2Fine   = mkKnob (ParamIDs::osc2Fine,   "Fine");
        fmKnob     = mkKnob (ParamIDs::oscFM,      "FM");
        osc2Level  = mkKnob (ParamIDs::mixOsc2,    "Level");

        mixOsc1  = mkKnob (ParamIDs::mixOsc1,  "Osc 1");
        mixOsc2  = mkKnob (ParamIDs::mixOsc2,  "Osc 2");
        mixNoise = mkKnob (ParamIDs::mixNoise, "Noise");
        mixSub   = mkKnob (ParamIDs::mixSub,   "Sub");
        unisonCombo = mkCombo (ParamIDs::unison, Choices::unisonCounts());
        uniDet = mkKnob (ParamIDs::unisonDet, "Detune");
        uniSpr = mkKnob (ParamIDs::unisonSpr, "Spread");

        filterTypeCombo = mkCombo (ParamIDs::filterType, Choices::filterTypes());
        cutoffKnob    = mkKnob (ParamIDs::cutoff,      "Cutoff");
        resonanceKnob = mkKnob (ParamIDs::resonance,   "Reso");
        envAmtKnob    = mkKnob (ParamIDs::filterEnv,    "Env");
        fDriveKnob    = mkKnob (ParamIDs::filterDrive,  "Drive");
        keyFollowFader = std::make_unique<FaderWithLabel> (proc.parameters, ParamIDs::keyFollow, "Key", &lnf);
        addAndMakeVisible (*keyFollowFader);

        const juce::StringArray tabNames { "AMP", "FILTER", "PITCH" };
        for (int i = 0; i < 3; ++i)
        {
            auto t = std::make_unique<juce::TextButton> (tabNames[i]);
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
        lfoWaveDisplay = std::make_unique<WaveDisplay> (WaveDisplay::lfo, [this] { return paramChoice (ParamIDs::lfoShape); });
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
        delayFeedback = mkKnob (ParamIDs::delayFeedback, "Fdbk");
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
        octDown = mkNav ("<"); octUp = mkNav (">");
        octDown->onClick = [this] { stepOctave (-1); };
        octUp->onClick   = [this] { stepOctave (1); };
        octCaption.setText ("Oct", juce::dontSendNotification);
        octCaption.setColour (juce::Label::textColourId, textCaption);
        octCaption.setJustificationType (juce::Justification::centred);
        octValueLabel.setJustificationType (juce::Justification::centred);
        octValueLabel.setColour (juce::Label::textColourId, textWarm);
        addAndMakeVisible (octCaption); addAndMakeVisible (octValueLabel);

        vibratoKnob = mkKnob (ParamIDs::vibrato,  "Vibrato");
        glideKnob   = mkKnob (ParamIDs::glide,    "Glide");
        mainKnob    = mkKnob (ParamIDs::mainGain, "MAIN", true);
        driveKnob   = mkKnob (ParamIDs::drive,    "Drive");
        panKnob     = mkKnob (ParamIDs::pan,      "Pan");

        pitchWheel = std::make_unique<WheelControl> (true,  proc.uiPitchBend, -1.0f, 1.0f, 0.0f);
        modWheel   = std::make_unique<WheelControl> (false, proc.uiModWheel,   0.0f, 1.0f, 0.0f);
        addAndMakeVisible (*pitchWheel); addAndMakeVisible (*modWheel);

        keyboard.setLowestVisibleKey (24);
        keyboard.setAvailableRange (24, 96);
        keyboard.setBlackNoteLengthProportion (0.64f);
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
        const int idx = proc.getCurrentProgram();
        auto t = juce::String (idx + 1).paddedLeft ('0', 2) + "  " + proc.getCurrentPresetName();
        presetNameLabel.setText (t, juce::dontSendNotification);
        lastShownPresetName = proc.getCurrentPresetName();
    }

    void refreshAllKnobTexts()
    {
        for (auto* k : allKnobs())
            if (k != nullptr) k->refreshValueText();
        if (keyFollowFader) keyFollowFader->refreshValueText();
        octaveLabelRefresh();
        if (osc1WaveDisplay) osc1WaveDisplay->repaint();
        if (osc2WaveDisplay) osc2WaveDisplay->repaint();
        if (lfoWaveDisplay)  lfoWaveDisplay->repaint();
    }

    std::vector<KnobWithLabel*> allKnobs()
    {
        return { osc1Octave.get(), osc1Semi.get(), osc1Fine.get(), osc1Level.get(), pwmKnob.get(),
                 osc2Octave.get(), osc2Semi.get(), osc2Fine.get(), osc2Level.get(), fmKnob.get(),
                 mixOsc1.get(), mixOsc2.get(), mixNoise.get(), mixSub.get(), uniDet.get(), uniSpr.get(),
                 cutoffKnob.get(), resonanceKnob.get(), envAmtKnob.get(), fDriveKnob.get(),
                 envA.get(), envD.get(), envS.get(), envR.get(),
                 lfoRateKnob.get(), lfoDepthKnob.get(),
                 chorusRate.get(), chorusDepth.get(), phaserRate.get(), phaserDepth.get(),
                 delayTime.get(), delayFeedback.get(), delayMix.get(),
                 reverbSize.get(), reverbDecay.get(), reverbMix.get(), arpRateKnob.get(),
                 vibratoKnob.get(), glideKnob.get(), mainKnob.get(), driveKnob.get(), panKnob.get() };
    }

    void selectEnvTab (int idx)
    {
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

        g.setColour (headerFill);
        g.fillRect (0, 0, getWidth(), R (0, 0, 0, 52).getHeight());
        g.setColour (gold.withAlpha (0.35f));
        g.fillRect (0, R (0, 51, 0, 1).getY(), getWidth(), juce::jmax (1, R (0, 0, 0, 1).getHeight()));

        // logo mark + title
        const auto logoC = juce::Point<float> ((float) R (28, 26, 0, 0).getX(), (float) R (0, 26, 0, 0).getY());
        drawStar (g, logoC, 9.0f * scx, gold.withAlpha (0.85f), 1.1f);
        g.setColour (goldBright);
        g.setFont (ArabesqueLookAndFeel::uiFont (20.0f * juce::jmin (scx, scy)));
        g.drawText ("ARABESQUE", R (44, 8, 200, 24), juce::Justification::centredLeft);
        g.setColour (textDim);
        g.setFont (ArabesqueLookAndFeel::uiFont (10.0f * juce::jmin (scx, scy)));
        g.drawText ("TURKISH EMOTIONAL SYNTH", R (44, 30, 220, 14), juce::Justification::centredLeft);

        g.setColour (panelBorder.withAlpha (0.5f));
        g.fillRect (R (268, 14, 1, 26));
        g.setColour (textDim);
        g.setFont (ArabesqueLookAndFeel::uiFont (10.0f * juce::jmin (scx, scy)));
        g.drawText ("DA Production", R (278, 12, 120, 14), juce::Justification::centredLeft);
        g.drawText ("Instruments",   R (278, 26, 120, 14), juce::Justification::centredLeft);

        drawPanel (g, R (10, 60, 390, 148).toFloat(), "OSCILLATOR 1");
        drawPanel (g, R (10, 216, 390, 148).toFloat(), "OSCILLATOR 2");
        drawPanel (g, R (10, 372, 390, 140).toFloat(), "MIXER  /  UNISON");

        auto art = R (410, 60, 460, 248).toFloat();
        g.setColour (panelFill);
        g.fillRoundedRectangle (art, 6.0f);
        g.setColour (panelBorder);
        g.drawRoundedRectangle (art.reduced (0.5f), 6.0f, 1.0f);
        const auto c = art.getCentre().translated (0.0f, -16.0f);
        drawStar (g, c, 48.0f * juce::jmin (scx, scy), gold.withAlpha (0.28f), 1.15f);
        drawStar (g, c, 28.0f * juce::jmin (scx, scy), gold.withAlpha (0.50f), 1.15f);
        drawStar (g, c, 14.0f * juce::jmin (scx, scy), gold.withAlpha (0.75f), 1.2f);
        g.setColour (gold);
        g.fillEllipse (c.x - 2.5f, c.y - 2.5f, 5.0f, 5.0f);
        g.setColour (goldBright);
        g.setFont (ArabesqueLookAndFeel::uiFont (26.0f * juce::jmin (scx, scy)));
        g.drawText ("ARABESQUE", R (410, 210, 460, 32), juce::Justification::centred);
        g.setColour (textDim);
        g.setFont (ArabesqueLookAndFeel::uiFont (10.5f * juce::jmin (scx, scy)));
        g.drawText ("T U R K I S H   E M O T I O N A L   S Y N T H E S I Z E R",
                    R (410, 242, 460, 18), juce::Justification::centred);

        drawPanel (g, R (410, 316, 460, 196).toFloat(), {});
        drawPanel (g, R (880, 60, 390, 168).toFloat(), "FILTER");
        drawPanel (g, R (880, 236, 390, 140).toFloat(), "ENVELOPES");
        drawPanel (g, R (880, 384, 390, 128).toFloat(), "LFO");

        drawPanel (g, R (10, 522, 70, 118).toFloat(), "FX");
        drawPanel (g, R (86, 522, 168, 118).toFloat(), "CHORUS");
        drawPanel (g, R (260, 522, 168, 118).toFloat(), "PHASER");
        drawPanel (g, R (434, 522, 230, 118).toFloat(), "DELAY");
        drawPanel (g, R (670, 522, 250, 118).toFloat(), "REVERB");
        drawPanel (g, R (926, 522, 344, 118).toFloat(), "ARPEGGIATOR");

        g.setColour (juce::Colour (0xff141210));
        g.fillRoundedRectangle (R (10, 650, 1260, 100).toFloat(), 6.0f);
        g.setColour (panelBorder.withAlpha (0.7f));
        g.drawRoundedRectangle (R (10, 650, 1260, 100).toFloat().reduced (0.5f), 6.0f, 1.0f);

        g.setColour (textCaption);
        g.setFont (ArabesqueLookAndFeel::uiFont (9.0f * juce::jmin (scx, scy)));
        g.drawText ("PITCH", R (22, 738, 40, 12), juce::Justification::centred);
        g.drawText ("MOD",   R (64, 738, 40, 12), juce::Justification::centred);
        g.setColour (gold.withAlpha (0.7f));
        g.setFont (ArabesqueLookAndFeel::uiFont (12.0f * juce::jmin (scx, scy)));
        g.drawText ("ARABESQUE", R (1140, 688, 120, 16), juce::Justification::centred);
        g.setColour (textDim);
        g.setFont (ArabesqueLookAndFeel::uiFont (9.0f * juce::jmin (scx, scy)));
        g.drawText ("DA PRODUCTION", R (1140, 706, 120, 12), juce::Justification::centred);
    }

    void resized() override
    {
        scx = (float) getWidth()  / (float) W;
        scy = (float) getHeight() / (float) H;

        prevPreset->setBounds (R (410, 12, 32, 28));
        presetNameLabel.setBounds (R (444, 12, 250, 28));
        nextPreset->setBounds (R (696, 12, 32, 28));
        saveButton->setBounds (R (740, 12, 58, 28));
        browseButton->setBounds (R (802, 12, 70, 28));
        voicesLabel.setBounds (R (1100, 12, 52, 28));
        voicesCombo->setBounds (R (1154, 12, 70, 28));

        osc1WaveDisplay->setBounds (R (22, 84, 78, 52));
        osc1WaveCombo->setBounds    (R (22, 140, 78, 22));
        osc1Octave->setBounds (R (108, 78, 54, 118));
        osc1Semi->setBounds   (R (162, 78, 54, 118));
        osc1Fine->setBounds   (R (216, 78, 54, 118));
        pwmKnob->setBounds    (R (270, 78, 54, 118));
        osc1Level->setBounds  (R (328, 74, 64, 126));

        osc2WaveDisplay->setBounds (R (22, 240, 78, 52));
        osc2WaveCombo->setBounds    (R (22, 296, 78, 22));
        osc2Octave->setBounds (R (108, 234, 54, 118));
        osc2Semi->setBounds   (R (162, 234, 54, 118));
        osc2Fine->setBounds   (R (216, 234, 54, 118));
        fmKnob->setBounds     (R (270, 234, 54, 118));
        osc2Level->setBounds  (R (328, 230, 64, 126));

        mixOsc1->setBounds  (R (18,  392, 52, 112));
        mixOsc2->setBounds  (R (70,  392, 52, 112));
        mixNoise->setBounds (R (122, 392, 52, 112));
        mixSub->setBounds   (R (174, 392, 52, 112));
        unisonCombo->setBounds (R (236, 430, 48, 22));
        uniDet->setBounds   (R (288, 392, 52, 112));
        uniSpr->setBounds   (R (340, 392, 52, 112));

        vibratoKnob->setBounds (R (428, 332, 72, 168));
        glideKnob->setBounds   (R (504, 332, 72, 168));
        mainKnob->setBounds    (R (580, 324, 120, 180));
        driveKnob->setBounds   (R (704, 332, 72, 168));
        panKnob->setBounds     (R (780, 332, 72, 168));

        filterTypeCombo->setBounds (R (980, 82, 274, 22));
        cutoffKnob->setBounds    (R (890, 108, 68, 112));
        resonanceKnob->setBounds (R (958, 108, 68, 112));
        envAmtKnob->setBounds    (R (1026, 108, 68, 112));
        fDriveKnob->setBounds    (R (1094, 108, 68, 112));
        keyFollowFader->setBounds (R (1164, 86, 90, 134));

        envTabs[0]->setBounds (R (980, 258, 84, 22));
        envTabs[1]->setBounds (R (1068, 258, 84, 22));
        envTabs[2]->setBounds (R (1156, 258, 84, 22));
        envA->setBounds (R (892, 282, 88, 86));
        envD->setBounds (R (984, 282, 88, 86));
        envS->setBounds (R (1076, 282, 88, 86));
        envR->setBounds (R (1168, 282, 88, 86));

        lfoShapeCombo->setBounds (R (980, 406, 140, 22));
        syncToggle->setBounds    (R (1130, 406, 64, 22));
        lfoWaveDisplay->setBounds (R (892, 434, 210, 68));
        lfoRateKnob->setBounds   (R (1110, 430, 72, 78));
        lfoDepthKnob->setBounds  (R (1184, 430, 72, 78));

        fxOnToggle->setBounds (R (18, 560, 54, 48));
        chorusRate->setBounds  (R (94,  540, 76, 92));
        chorusDepth->setBounds (R (170, 540, 76, 92));
        phaserRate->setBounds  (R (268, 540, 76, 92));
        phaserDepth->setBounds (R (344, 540, 76, 92));
        delayTime->setBounds     (R (442, 540, 70, 92));
        delayFeedback->setBounds (R (514, 540, 70, 92));
        delayMix->setBounds      (R (586, 540, 70, 92));
        reverbTypeCombo->setBounds (R (760, 540, 150, 20));
        reverbSize->setBounds  (R (678, 562, 76, 72));
        reverbDecay->setBounds (R (756, 562, 76, 72));
        reverbMix->setBounds   (R (834, 562, 76, 72));

        arpOnToggle->setBounds (R (1178, 540, 80, 22));
        for (int i = 0; i < 8; ++i)
        {
            const int col = i % 4, row = i / 4;
            stepPads[(size_t) i]->setBounds (R (940 + col * 26, 568 + row * 26, 22, 22));
        }
        arpRateKnob->setBounds (R (1050, 548, 70, 86));
        octCaption.setBounds (R (1124, 552, 48, 14));
        octDown->setBounds (R (1124, 568, 22, 22));
        octValueLabel.setBounds (R (1146, 568, 24, 22));
        octUp->setBounds (R (1170, 568, 22, 22));

        pitchWheel->setBounds (R (28, 662, 28, 74));
        modWheel->setBounds   (R (70, 662, 28, 74));
        keyboard.setBounds (R (112, 658, 1010, 86));
        const float kw = (float) keyboard.getWidth() / 43.0f;
        if (kw > 1.0f) keyboard.setKeyWidth (kw);
    }
};

ArabesqueAudioProcessorEditor::ArabesqueAudioProcessorEditor (ArabesqueAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    content = std::make_unique<Content> (p);
    addAndMakeVisible (*content);
    setSize (1152, 684);
    setResizable (true, true);
    setResizeLimits (900, 534, 1600, 950);
    if (auto* c = getConstrainer())
        c->setFixedAspectRatio ((double) W / (double) H);
}

ArabesqueAudioProcessorEditor::~ArabesqueAudioProcessorEditor() = default;

void ArabesqueAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff10100e));
}

void ArabesqueAudioProcessorEditor::resized()
{
    if (content != nullptr)
        content->setBounds (getLocalBounds());
}
