#include "PluginProcessor.h"
#include "PluginEditor.h"

ExamplePluginAudioProcessor::ExamplePluginAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
                       .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
                       .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
                      ),
#else
    : 
#endif
      parameters(*this, nullptr, "Parameters", createParameterLayout())
{
    // Get atomic parameter pointers for realtime access
    gainParameter = parameters.getRawParameterValue(TylerAudio::ParameterIDs::kGain);
    bypassParameter = parameters.getRawParameterValue(TylerAudio::ParameterIDs::kBypass);
    
    // Set up parameter callbacks
    parameters.addParameterListener(TylerAudio::ParameterIDs::kGain, this);
    parameters.addParameterListener(TylerAudio::ParameterIDs::kBypass, this);
}

ExamplePluginAudioProcessor::~ExamplePluginAudioProcessor()
{
}

const juce::String ExamplePluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ExamplePluginAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool ExamplePluginAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool ExamplePluginAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double ExamplePluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ExamplePluginAudioProcessor::getNumPrograms()
{
    return 1;
}

int ExamplePluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ExamplePluginAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String ExamplePluginAudioProcessor::getProgramName(int index)
{
    return {};
}

void ExamplePluginAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

void ExamplePluginAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Initialize parameter smoothing
    gainSmoother.setSmoothingTime(0.05, sampleRate);  // 50ms smoothing time
    gainSmoother.snapToTarget();  // Initialize to current value
}

void ExamplePluginAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ExamplePluginAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}
#endif

void ExamplePluginAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) noexcept
{
    juce::ScopedNoDenormals noDenormals;
    juce::ignoreUnused(midiMessages);
    
    const auto totalNumInputChannels = getTotalNumInputChannels();
    const auto totalNumOutputChannels = getTotalNumOutputChannels();
    const auto numSamples = buffer.getNumSamples();

    // Clear any unused output channels
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, numSamples);

    // Check bypass first
    const bool isBypassed = bypassParameter->load() > 0.5f;
    if (isBypassed)
        return;  // Early return for bypass

    // Use JUCE's AudioBlock for efficient processing
    auto block = juce::dsp::AudioBlock<float>(buffer);
    auto context = juce::dsp::ProcessContextReplacing<float>(block);
    
    // Process each sample with smoothed gain
    for (int sample = 0; sample < numSamples; ++sample)
    {
        const float smoothedGain = gainSmoother.getNextValue();
        
        for (size_t channel = 0; channel < block.getNumChannels(); ++channel)
        {
            float inputSample = block.getSample(channel, sample);
            float outputSample = inputSample * smoothedGain;
            
            // Denormal protection and sanitization
            outputSample = TylerAudio::Utils::sanitizeFloat(outputSample);
            
            block.setSample(channel, sample, outputSample);
        }
    }
}

bool ExamplePluginAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* ExamplePluginAudioProcessor::createEditor()
{
    return new ExamplePluginAudioProcessorEditor(*this);
}

void ExamplePluginAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void ExamplePluginAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState.get() != nullptr)
    {
        if (xmlState->hasTagName(parameters.state.getType()))
        {
            parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
        }
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout ExamplePluginAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // Gain parameter with proper range and defaults
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        TylerAudio::ParameterIDs::kGain,
        "Gain",
        juce::NormalisableRange<float>(0.0f, 2.0f, 0.01f, 0.5f),  // 0-2x gain with curve
        TylerAudio::Constants::defaultGain,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(TylerAudio::Utils::gainToDb(value), 1) + " dB"; }
    ));

    // Bypass parameter
    layout.add(std::make_unique<juce::AudioParameterBool>(
        TylerAudio::ParameterIDs::kBypass,
        "Bypass",
        false
    ));

    return layout;
}

void ExamplePluginAudioProcessor::parameterChanged(const juce::String& parameterID, float newValue)
{
    if (parameterID == TylerAudio::ParameterIDs::kGain)
    {
        gainSmoother.setTargetValue(newValue);
    }
    // Bypass is handled directly in processBlock via atomic load
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ExamplePluginAudioProcessor();
}