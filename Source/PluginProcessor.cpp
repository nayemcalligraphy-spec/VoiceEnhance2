#include "PluginProcessor.h"
#include "PluginEditor.h"

juce::AudioProcessorValueTreeState::ParameterLayout VoiceEnhanceProcessor::createLayout()
{
    using namespace juce;
    std::vector<std::unique_ptr<RangedAudioParameter>> p;

    p.push_back (std::make_unique<AudioParameterBool> (ParameterID { "on", 1 }, "Voice Enhance On", true));

    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "nr", 1 }, "Noise Reduction",
                    NormalisableRange<float> (0.0f, 100.0f, 1.0f), kPresets[kDefaultPreset - 1].nr,
                    AudioParameterFloatAttributes().withLabel ("%")));

    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "mix", 1 }, "Enhanced Mix",
                    NormalisableRange<float> (0.0f, 100.0f, 1.0f), kPresets[kDefaultPreset - 1].mix,
                    AudioParameterFloatAttributes().withLabel ("%")));

    StringArray names { "Custom" };
    for (auto& pr : kPresets) names.add (pr.name);
    p.push_back (std::make_unique<AudioParameterChoice> (ParameterID { "preset", 1 }, "Preset", names, kDefaultPreset));

    return { p.begin(), p.end() };
}

VoiceEnhanceProcessor::VoiceEnhanceProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "STATE", createLayout())
{
    pOn  = apvts.getRawParameterValue ("on");
    pNr  = apvts.getRawParameterValue ("nr");
    pMix = apvts.getRawParameterValue ("mix");
    apvts.addParameterListener ("preset", this);
    apvts.addParameterListener ("nr", this);
    apvts.addParameterListener ("mix", this);
}

VoiceEnhanceProcessor::~VoiceEnhanceProcessor()
{
    cancelPendingUpdate();
    apvts.removeParameterListener ("preset", this);
    apvts.removeParameterListener ("nr", this);
    apvts.removeParameterListener ("mix", this);
}

// ---- preset <-> knobs logic (runs on the message thread through AsyncUpdater)
void VoiceEnhanceProcessor::parameterChanged (const juce::String& id, float)
{
    if (applying.load()) return;
    if (id == "preset")                 presetPending = true;
    else                                customPending = true;     // a knob was moved -> preset becomes "Custom"
    triggerAsyncUpdate();
}

void VoiceEnhanceProcessor::handleAsyncUpdate()
{
    auto* preset = apvts.getParameter ("preset");
    if (preset == nullptr) return;

    if (presetPending.exchange (false))
    {
        const int idx = (int) std::lround (preset->convertFrom0to1 (preset->getValue()));
        customPending = false;
        if (idx >= 1 && idx <= 10)
        {
            applying = true;
            for (auto [id, v] : { std::pair<const char*, float> { "nr", kPresets[idx - 1].nr },
                                  std::pair<const char*, float> { "mix", kPresets[idx - 1].mix } })
                if (auto* prm = apvts.getParameter (id))
                {
                    prm->beginChangeGesture();
                    prm->setValueNotifyingHost (prm->convertTo0to1 (v));
                    prm->endChangeGesture();
                }
            applying = false;
        }
    }
    else if (customPending.exchange (false))
    {
        const int idx = (int) std::lround (preset->convertFrom0to1 (preset->getValue()));
        if (idx >= 1 && idx <= 10)
        {
            const float nr = pNr->load(), mix = pMix->load();
            if (std::abs (nr - kPresets[idx - 1].nr) > 0.5f || std::abs (mix - kPresets[idx - 1].mix) > 0.5f)
            {
                applying = true;
                preset->beginChangeGesture();
                preset->setValueNotifyingHost (preset->convertTo0to1 (0.0f));
                preset->endChangeGesture();
                applying = false;
            }
        }
    }
}

// ---- audio
bool VoiceEnhanceProcessor::isBusesLayoutSupported (const BusesLayout& l) const
{
    const auto in = l.getMainInputChannelSet(), out = l.getMainOutputChannelSet();
    return in == out && (in == juce::AudioChannelSet::mono() || in == juce::AudioChannelSet::stereo());
}

void VoiceEnhanceProcessor::prepareToPlay (double sr, int blockSize)
{
    dspChannels = juce::jmax (1, juce::jmin (2, getTotalNumInputChannels()));
    dsp.prepare (sr, dspChannels);
    dsp.setAmount (pNr->load());
    setLatencySamples (dsp.getLatencySamples());
    for (int c = 0; c < 2; ++c)
    {
        const size_t chunk = (size_t) juce::jmax (blockSize, 512);
        inF[c].assign (chunk, 0.0f);
        wetF[c].assign (chunk, 0.0f);
        dryF[c].assign (chunk, 0.0f);
    }
    sOn.reset (sr, 0.015);  sOn.setCurrentAndTargetValue (pOn->load() > 0.5f ? 1.0f : 0.0f);
    sMix.reset (sr, 0.03);  sMix.setCurrentAndTargetValue (pMix->load() * 0.01f);
}

template <typename T>
void VoiceEnhanceProcessor::run (juce::AudioBuffer<T>& buffer)
{
    juce::ScopedNoDenormals noDenormals;
    const int n = buffer.getNumSamples();
    const int nch = juce::jmin (buffer.getNumChannels(), dspChannels);
    for (int c = nch; c < buffer.getNumChannels(); ++c) buffer.clear (c, 0, n);
    if (nch == 0 || n == 0) return;

    dsp.setAmount (pNr->load());
    sOn.setTargetValue (pOn->load() > 0.5f ? 1.0f : 0.0f);
    sMix.setTargetValue (pMix->load() * 0.01f);

    int done = 0;
    while (done < n)
    {
        const int m = juce::jmin (n - done, (int) inF[0].size());
        const float* ip[2] = { inF[0].data(), inF[1].data() };
        float* wp[2] = { wetF[0].data(), wetF[1].data() };
        float* dp[2] = { dryF[0].data(), dryF[1].data() };

        for (int c = 0; c < nch; ++c)
        {
            auto* src = buffer.getReadPointer (c) + done;
            for (int i = 0; i < m; ++i) inF[c][(size_t) i] = (float) src[i];
        }
        dsp.process (ip, wp, dp, m);

        for (int i = 0; i < m; ++i)
        {
            const float on = sOn.getNextValue(), mix = sMix.getNextValue();
            const float amountWet = on * mix;                 // off -> dry (delayed, so it stays in time)
            for (int c = 0; c < nch; ++c)
            {
                const float y = amountWet * wetF[c][(size_t) i] + (1.0f - amountWet) * dryF[c][(size_t) i];
                buffer.getWritePointer (c)[done + i] = (T) y;
            }
        }
        done += m;
    }
    reductionDb.store (pOn->load() > 0.5f ? dsp.getReductionDb() : 0.0f);
}

void VoiceEnhanceProcessor::processBlock (juce::AudioBuffer<float>& b, juce::MidiBuffer&)  { run (b); }
void VoiceEnhanceProcessor::processBlock (juce::AudioBuffer<double>& b, juce::MidiBuffer&) { run (b); }

// ---- state
void VoiceEnhanceProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    if (auto xml = apvts.copyState().createXml()) copyXmlToBinary (*xml, dest);
}

void VoiceEnhanceProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (apvts.state.getType()))
        {
            applying = true;
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
            applying = false;
        }
}

juce::AudioProcessorEditor* VoiceEnhanceProcessor::createEditor() { return new VoiceEnhanceEditor (*this); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new VoiceEnhanceProcessor(); }
