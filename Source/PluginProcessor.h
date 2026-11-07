#pragma once
#include <JuceHeader.h>
#include <atomic>
#include "DSP/NoiseSuppressor.h"
#include "Presets.h"

class VoiceEnhanceProcessor : public juce::AudioProcessor,
                              private juce::AudioProcessorValueTreeState::Listener,
                              private juce::AsyncUpdater
{
public:
    VoiceEnhanceProcessor();
    ~VoiceEnhanceProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void processBlock (juce::AudioBuffer<double>&, juce::MidiBuffer&) override;
    bool supportsDoublePrecisionProcessing() const override { return true; }

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.05; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;
    std::atomic<float> reductionDb { 0.0f };     // for the meter in the editor

    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

private:
    template <typename T> void run (juce::AudioBuffer<T>& buffer);
    void parameterChanged (const juce::String& id, float value) override;
    void handleAsyncUpdate() override;

    an::NoiseSuppressor dsp;
    int dspChannels = 2;
    std::atomic<float>* pOn  = nullptr;
    std::atomic<float>* pNr  = nullptr;
    std::atomic<float>* pMix = nullptr;
    juce::SmoothedValue<float> sOn, sMix;
    std::vector<float> inF[2], wetF[2], dryF[2];

    std::atomic<bool> presetPending { false }, customPending { false }, applying { false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VoiceEnhanceProcessor)
};
