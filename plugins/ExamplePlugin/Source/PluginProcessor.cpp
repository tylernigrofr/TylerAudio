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
                      )
#endif
{
    addParameter(gainParameter = new juce::AudioParameterFloat("gain", "Gain", 0.0f, 1.0f, 0.5f));
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

void ExamplePluginAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    float gain = *gainParameter;
    
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            channelData[sample] *= gain;
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
}

void ExamplePluginAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ExamplePluginAudioProcessor();
}