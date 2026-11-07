#include "PluginEditor.h"
#include "AboutWindow.h"
#include "BinaryData.h"

using namespace juce;

// ================================================================== look and feel
AnLookAndFeel::AnLookAndFeel()
{
    setColour (ResizableWindow::backgroundColourId, theme::bg);
    setColour (ComboBox::backgroundColourId, theme::panel2);
    setColour (ComboBox::outlineColourId, theme::border);
    setColour (ComboBox::textColourId, theme::text);
    setColour (ComboBox::arrowColourId, theme::muted);
    setColour (ComboBox::focusedOutlineColourId, theme::blue);
    setColour (PopupMenu::backgroundColourId, theme::panel2);
    setColour (PopupMenu::textColourId, theme::text);
    setColour (PopupMenu::highlightedBackgroundColourId, theme::blue);
    setColour (PopupMenu::highlightedTextColourId, Colours::white);
    setColour (TextButton::buttonColourId, theme::panel3);
    setColour (TextButton::buttonOnColourId, theme::blue);
    setColour (TextButton::textColourOffId, theme::text);
    setColour (TextButton::textColourOnId, Colours::white);
    setColour (ScrollBar::thumbColourId, theme::panel3);
    setColour (TooltipWindow::backgroundColourId, theme::panel2);
    setColour (TooltipWindow::textColourId, theme::text);
    setColour (TooltipWindow::outlineColourId, theme::border);
    setColour (HyperlinkButton::textColourId, theme::blueH);
}

void AnLookAndFeel::drawRotarySlider (Graphics& g, int x, int y, int w, int h, float pos, float, float, Slider& s)
{
    const auto b = Rectangle<float> ((float) x, (float) y, (float) w, (float) h);
    const float cx = b.getCentreX(), cy = b.getY() + b.getHeight() * 0.40f;
    const float r = jmin (b.getWidth() * 0.5f - 12.0f, b.getHeight() * 0.40f - 6.0f);
    const float a0 = -2.3561945f, a1 = 2.3561945f, a = a0 + pos * (a1 - a0);   // -135 .. +135 degrees from 12 o'clock

    Path trackPath;
    trackPath.addCentredArc (cx, cy, r, r, 0.0f, a0, a1, true);
    g.setColour (theme::track);
    g.strokePath (trackPath, PathStrokeType (6.0f, PathStrokeType::curved, PathStrokeType::rounded));

    if (pos > 0.002f)
    {
        Path val;
        val.addCentredArc (cx, cy, r, r, 0.0f, a0, a, true);
        g.setColour (s.isEnabled() ? theme::blue : theme::off);
        g.strokePath (val, PathStrokeType (6.0f, PathStrokeType::curved, PathStrokeType::rounded));
    }

    const float ri = r - 10.0f;
    ColourGradient grad (theme::panel3.brighter (0.25f), cx - ri * 0.4f, cy - ri * 0.5f,
                         theme::panel2.darker (0.3f), cx + ri * 0.6f, cy + ri * 0.8f, true);
    g.setGradientFill (grad);
    g.fillEllipse (cx - ri, cy - ri, 2 * ri, 2 * ri);
    g.setColour (theme::border);
    g.drawEllipse (cx - ri, cy - ri, 2 * ri, 2 * ri, 1.2f);

    g.setColour (theme::text);
    g.drawLine (cx + std::sin (a) * ri * 0.28f, cy - std::cos (a) * ri * 0.28f,
                cx + std::sin (a) * (ri - 5.0f), cy - std::cos (a) * (ri - 5.0f), 3.0f);

    g.setColour (theme::text);
    g.setFont (Font (20.0f, Font::bold));
    g.drawText (s.getTextFromValue (s.getValue()), Rectangle<float> (b.getX(), cy + r + 8.0f, b.getWidth(), 24.0f), Justification::centred);
    g.setColour (theme::muted);
    g.setFont (Font (14.0f));
    g.drawText (s.getName(), Rectangle<float> (b.getX(), cy + r + 32.0f, b.getWidth(), 20.0f), Justification::centred);
}

// ================================================================== small widgets
void PowerButton::paintButton (Graphics& g, bool over, bool)
{
    const auto b = getLocalBounds().toFloat().reduced (2.0f);
    const bool on = getToggleState();
    g.setColour (over ? theme::hover : theme::panel2);
    g.fillEllipse (b);
    g.setColour (on ? theme::green.withAlpha (0.6f) : theme::border);
    g.drawEllipse (b, 1.5f);

    const float cx = b.getCentreX(), cy = b.getCentreY(), r = b.getWidth() * 0.28f;
    const auto col = on ? theme::green : theme::muted;
    g.setColour (col);
    Path arc;
    arc.addCentredArc (cx, cy, r, r, 0.0f, degreesToRadians (35.0f), degreesToRadians (325.0f), true);
    g.strokePath (arc, PathStrokeType (2.6f, PathStrokeType::curved, PathStrokeType::rounded));
    g.drawLine (cx, cy - r * 1.15f, cx, cy - r * 0.05f, 2.6f);
}

void PresetPill::paintButton (Graphics& g, bool over, bool down)
{
    const auto b = getLocalBounds().toFloat().reduced (1.0f);
    Colour fill = selected ? theme::blue : (down ? theme::panel : (over ? theme::hover : theme::panel3));
    g.setColour (fill);
    g.fillRoundedRectangle (b, 9.0f);
    g.setColour (selected ? theme::blueH : theme::border);
    g.drawRoundedRectangle (b, 9.0f, 1.2f);
    g.setColour (selected ? Colours::white : theme::text);
    g.setFont (Font (20.0f, Font::bold));
    g.drawText (String (n), getLocalBounds(), Justification::centred);
}

void AboutMenuButton::paintButton (Graphics& g, bool over, bool down)
{
    if (over || down)
    {
        g.setColour (theme::blue);
        g.fillRoundedRectangle (getLocalBounds().toFloat().reduced (2.0f, 3.0f), 4.0f);
    }
    g.setColour (over || down ? Colours::white : theme::text);
    g.setFont (Font (15.0f));
    g.drawText ("About", getLocalBounds(), Justification::centred);
}

// ================================================================== editor
VoiceEnhanceEditor::VoiceEnhanceEditor (VoiceEnhanceProcessor& p) : AudioProcessorEditor (&p), proc (p)
{
    setLookAndFeel (&laf);
    logo = ImageCache::getFromMemory (BinaryData::logo_png, BinaryData::logo_pngSize);

    aboutButton.onClick = [this] { showAboutMenu(); };
    addAndMakeVisible (aboutButton);

    power.setTooltip ("Voice Enhance ON / OFF");
    addAndMakeVisible (power);
    powerAtt = std::make_unique<ButtonAtt> (proc.apvts, "on", power);

    setupKnob (nrKnob, "Noise Reduction", kPresets[kDefaultPreset - 1].nr);
    setupKnob (mixKnob, "Enhanced Mix", kPresets[kDefaultPreset - 1].mix);
    nrKnob.setTooltip ("How much noise is removed. Drag, use the mouse wheel, or double-click to reset.");
    mixKnob.setTooltip ("Blend between the original voice and the cleaned voice.");
    nrAtt  = std::make_unique<SliderAtt> (proc.apvts, "nr", nrKnob);
    mixAtt = std::make_unique<SliderAtt> (proc.apvts, "mix", mixKnob);

    if (auto* choice = dynamic_cast<AudioParameterChoice*> (proc.apvts.getParameter ("preset")))
        presetBox.addItemList (choice->choices, 1);
    presetBox.setTooltip ("Noise Reduction presets 1 - 10");
    addAndMakeVisible (presetBox);
    presetAtt = std::make_unique<ComboAtt> (proc.apvts, "preset", presetBox);

    resetButton.setTooltip ("Back to the default preset");
    resetButton.onClick = [this] { setPreset (kDefaultPreset); };
    addAndMakeVisible (resetButton);

    for (int i = 1; i <= 10; ++i)
    {
        auto pill = std::make_unique<PresetPill> (i);
        pill->setTooltip (kPresets[i - 1].name);
        pill->onClick = [this, i] { setPreset (i); };
        addAndMakeVisible (*pill);
        pills.push_back (std::move (pill));
    }
    startTimerHz (30);
    setSize (680, 450);      // must be LAST: it calls resized(), which needs every child to exist
}

VoiceEnhanceEditor::~VoiceEnhanceEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

void VoiceEnhanceEditor::setupKnob (Slider& s, const String& name, double def)
{
    s.setName (name);
    s.setSliderStyle (Slider::RotaryVerticalDrag);
    s.setRotaryParameters (MathConstants<float>::pi * 1.25f, MathConstants<float>::pi * 2.75f, true);
    s.setTextBoxStyle (Slider::NoTextBox, true, 0, 0);
    s.setTextValueSuffix (" %");
    s.setNumDecimalPlacesToDisplay (0);
    s.setMouseDragSensitivity (160);
    s.setDoubleClickReturnValue (true, def);
    s.setMouseCursor (MouseCursor::PointingHandCursor);
    addAndMakeVisible (s);
}

void VoiceEnhanceEditor::setPreset (int index)
{
    if (auto* prm = proc.apvts.getParameter ("preset"))
    {
        prm->beginChangeGesture();
        prm->setValueNotifyingHost (prm->convertTo0to1 ((float) index));
        prm->endChangeGesture();
    }
}

void VoiceEnhanceEditor::showAboutMenu()
{
    PopupMenu m;
    m.setLookAndFeel (&laf);
    m.addItem (1, "About This Plugin");
    m.addItem (2, "User Guide");
    m.addItem (3, "About Builders");
    m.addItem (4, "About An-Niyamah Production & Brand");
    Component::SafePointer<VoiceEnhanceEditor> safe (this);
    m.showMenuAsync (PopupMenu::Options().withTargetComponent (&aboutButton).withMinimumWidth (300),
                     [safe] (int r)
                     {
                         if (safe == nullptr || r == 0) return;
                         auto* c = safe.getComponent();
                         switch (r)
                         {
                             case 1: showAboutWindow ("About This Plugin", about::pluginInfo(), c); break;
                             case 2: showAboutWindow ("User Guide", about::userGuide(), c); break;
                             case 3: showAboutWindow ("About Builders & Management", about::builders(), c); break;
                             case 4: showAboutWindow ("About An-Niyamah Production & Brand", about::brand(), c); break;
                             default: break;
                         }
                     });
}

void VoiceEnhanceEditor::timerCallback()
{
    const int cur = (int) std::lround (proc.apvts.getRawParameterValue ("preset")->load());
    if (cur != shownPreset)
    {
        shownPreset = cur;
        for (int i = 0; i < (int) pills.size(); ++i) pills[(size_t) i]->selected = (cur == i + 1);
        for (auto& p : pills) p->repaint();
        repaint (card);
    }
    const float target = jlimit (0.0f, 1.0f, -proc.reductionDb.load() / 40.0f);
    meterValue += 0.25f * (target - meterValue);
    repaint (meterArea);
}

void VoiceEnhanceEditor::resized()
{
    if (pills.size() < 10) return;      // children not created yet
    auto r = getLocalBounds();
    topBar = r.removeFromTop (34);
    aboutButton.setBounds (topBar.getX() + 10, topBar.getY(), 70, topBar.getHeight());

    headerArea = r.removeFromTop (84);
    power.setBounds (24, headerArea.getY() + 24, 36, 36);
    auto right = headerArea.withTrimmedRight (22).withTrimmedLeft (headerArea.getWidth() - 400);
    resetButton.setBounds (right.removeFromRight (70).withSizeKeepingCentre (70, 32));
    right.removeFromRight (10);
    presetBox.setBounds (right.removeFromRight (230).withSizeKeepingCentre (230, 32));

    card = r.removeFromTop (240).reduced (18, 6);
    nrKnob.setBounds (card.getX() + 26, card.getY() + 12, 150, 210);
    mixKnob.setBounds (card.getX() + 190, card.getY() + 12, 150, 210);

    const int px = card.getX() + 392, py = card.getY() + 58, ps = 48, gap = 9;
    for (int i = 0; i < 10; ++i)
        pills[(size_t) i]->setBounds (px + (i % 5) * (ps + gap), py + (i / 5) * (ps + gap), ps, ps);

    r.removeFromTop (6);
    meterArea = r.removeFromTop (34).reduced (18, 4);
}

void VoiceEnhanceEditor::paint (Graphics& g)
{
    g.fillAll (theme::bg);
    g.setColour (theme::header);
    g.fillRect (topBar);

    // header: logo + title
    if (logo.isValid())
        g.drawImage (logo, Rectangle<float> (74.0f, (float) headerArea.getY() + 14.0f, 56.0f, 56.0f), RectanglePlacement::centred);
    g.setColour (theme::text);
    g.setFont (Font (26.0f, Font::bold));
    g.drawText ("VOICE ENHANCE", 142, headerArea.getY() + 14, 300, 32, Justification::centredLeft);
    g.setColour (theme::muted);
    g.setFont (Font (14.0f));
    g.drawText ("An-Niyamah Production   -   Noise Reduction", 144, headerArea.getY() + 46, 320, 20, Justification::centredLeft);
    g.drawText ("Preset", presetBox.getX() - 56, presetBox.getY(), 50, presetBox.getHeight(), Justification::centredRight);

    // main card
    g.setColour (theme::panel);
    g.fillRoundedRectangle (card.toFloat(), 12.0f);
    g.setColour (theme::border);
    g.drawRoundedRectangle (card.toFloat().reduced (0.5f), 12.0f, 1.0f);

    const int px = card.getX() + 392;
    g.setColour (theme::muted);
    g.setFont (Font (13.0f, Font::bold));
    g.drawText ("NOISE REDUCTION LEVEL", px, card.getY() + 18, 270, 20, Justification::centredLeft);
    g.setFont (Font (13.0f));
    g.drawText ("1  Light", px, card.getBottom() - 70, 130, 18, Justification::centredLeft);
    g.drawText ("Maximum  10", px + 140, card.getBottom() - 70, 130, 18, Justification::centredRight);

    const int idx = (int) std::lround (proc.apvts.getRawParameterValue ("preset")->load());
    g.setColour (idx >= 1 ? theme::blueH : theme::muted);
    g.setFont (Font (16.0f, Font::bold));
    g.drawText (idx >= 1 ? String (kPresets[idx - 1].name) : String ("Custom"), px, card.getBottom() - 46, 270, 24, Justification::centredLeft);

    // divider between knobs and presets
    g.setColour (theme::border);
    g.drawLine ((float) card.getX() + 372.0f, (float) card.getY() + 20.0f, (float) card.getX() + 372.0f, (float) card.getBottom() - 20.0f, 1.0f);

    // noise-removed meter
    g.setColour (theme::muted);
    g.setFont (Font (13.0f, Font::bold));
    g.drawText ("NOISE REMOVED", meterArea.getX(), meterArea.getY(), 130, meterArea.getHeight(), Justification::centredLeft);
    const Rectangle<float> bar ((float) meterArea.getX() + 140.0f, (float) meterArea.getCentreY() - 8.0f,
                                (float) meterArea.getWidth() - 230.0f, 16.0f);
    g.setColour (theme::panel2);
    g.fillRoundedRectangle (bar, 8.0f);
    g.setColour (theme::border);
    g.drawRoundedRectangle (bar.reduced (0.5f), 8.0f, 1.0f);
    if (meterValue > 0.005f)
    {
        auto fill = bar.reduced (2.0f).withWidth (jmax (10.0f, (bar.getWidth() - 4.0f) * meterValue));
        g.setGradientFill (ColourGradient (theme::violet, fill.getX(), 0.0f, theme::pink, bar.getRight(), 0.0f, false));
        g.fillRoundedRectangle (fill, 6.0f);
    }
    g.setColour (theme::text);
    g.setFont (Font (14.0f, Font::bold));
    g.drawText (String (-proc.reductionDb.load(), 1) + " dB", Rectangle<int> ((int) bar.getRight() + 10, meterArea.getY(), 80, meterArea.getHeight()), Justification::centredLeft);

    // footer
    g.setColour (theme::muted);
    g.setFont (Font (12.0f));
    g.drawText (String (CharPointer_UTF8 ("\xC2\xA9 2026 ")) + about::kBrand + "   -   Version " + about::kVersion + "   -   VST3",
                getLocalBounds().removeFromBottom (26), Justification::centred);
}
