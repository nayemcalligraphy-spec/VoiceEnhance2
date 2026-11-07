#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Theme.h"

// ------------------------------------------------------------------ look and feel (same style as the desktop app)
class AnLookAndFeel : public juce::LookAndFeel_V4
{
public:
    AnLookAndFeel();
    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos, float, float, juce::Slider&) override;
    juce::Font getComboBoxFont (juce::ComboBox&) override { return juce::Font (15.0f); }
    juce::Font getPopupMenuFont() override { return juce::Font (15.0f); }
    juce::Font getTextButtonFont (juce::TextButton&, int) override { return juce::Font (15.0f, juce::Font::bold); }
};

class PowerButton : public juce::Button
{
public:
    PowerButton() : juce::Button ("power") { setClickingTogglesState (true); setMouseCursor (juce::MouseCursor::PointingHandCursor); }
    void paintButton (juce::Graphics&, bool over, bool down) override;
};

class PresetPill : public juce::Button
{
public:
    explicit PresetPill (int number) : juce::Button (juce::String (number)), n (number)
    { setMouseCursor (juce::MouseCursor::PointingHandCursor); }
    void paintButton (juce::Graphics&, bool over, bool down) override;
    bool selected = false;
private:
    int n;
};

class AboutMenuButton : public juce::Button
{
public:
    AboutMenuButton() : juce::Button ("About") { setMouseCursor (juce::MouseCursor::PointingHandCursor); }
    void paintButton (juce::Graphics&, bool over, bool down) override;
};

// ------------------------------------------------------------------ editor
class VoiceEnhanceEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit VoiceEnhanceEditor (VoiceEnhanceProcessor&);
    ~VoiceEnhanceEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void showAboutMenu();
    void setPreset (int index);
    void setupKnob (juce::Slider&, const juce::String& name, double defaultValue);

    VoiceEnhanceProcessor& proc;
    AnLookAndFeel laf;
    juce::TooltipWindow tooltip { this, 600 };

    AboutMenuButton aboutButton;
    PowerButton power;
    juce::Slider nrKnob, mixKnob;
    juce::ComboBox presetBox;
    juce::TextButton resetButton { "Reset" };
    std::vector<std::unique_ptr<PresetPill>> pills;

    juce::Image logo;
    juce::Rectangle<int> topBar, headerArea, card, meterArea;
    float meterValue = 0.0f;
    int shownPreset = -1;

    using SliderAtt = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAtt = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using ComboAtt  = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    std::unique_ptr<SliderAtt> nrAtt, mixAtt;
    std::unique_ptr<ButtonAtt> powerAtt;
    std::unique_ptr<ComboAtt> presetAtt;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VoiceEnhanceEditor)
};
