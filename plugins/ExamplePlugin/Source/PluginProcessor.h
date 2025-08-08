#pragma once

#include <JuceHeader.h>
#include "TylerAudioCommon.h"

class ExamplePluginAudioProcessor : public juce::AudioProcessor,
                                    public juce::AudioProcessorValueTreeState::Listener
{
public:
    ExamplePluginAudioProcessor();
    ~ExamplePluginAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) noexcept override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // Modern parameter access
    [[nodiscard]] juce::AudioProcessorValueTreeState& getParameters() noexcept { return parameters; }
    [[nodiscard]] const juce::AudioProcessorValueTreeState& getParameters() const noexcept { return parameters; }

private:
    // Parameter tree state for thread-safe parameter management
    juce::AudioProcessorValueTreeState parameters;
    
    // Realtime-safe parameter access
    std::atomic<float>* gainParameter{nullptr};
    std::atomic<float>* bypassParameter{nullptr};
    
    // Parameter smoothing
    TylerAudio::Utils::SmoothingFilter gainSmoother;
    
    // Create parameter layout
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    // AudioProcessorValueTreeState::Listener interface
    void parameterChanged(const juce::String& parameterID, float newValue) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ExamplePluginAudioProcessor)
};