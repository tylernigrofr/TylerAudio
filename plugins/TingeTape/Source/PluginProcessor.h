#pragma once

#include <JuceHeader.h>
#include "TylerAudioCommon.h"

class TingeTapeAudioProcessor : public juce::AudioProcessor,
                                    public juce::AudioProcessorValueTreeState::Listener
{
public:
    TingeTapeAudioProcessor();
    ~TingeTapeAudioProcessor() override;

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
    
    // Realtime-safe parameter access for all tape emulation parameters
    std::atomic<float>* wowParameter{nullptr};
    std::atomic<float>* lowCutFreqParameter{nullptr};
    std::atomic<float>* lowCutResParameter{nullptr};
    std::atomic<float>* highCutFreqParameter{nullptr};
    std::atomic<float>* highCutResParameter{nullptr};
    std::atomic<float>* dirtParameter{nullptr};
    std::atomic<float>* toneParameter{nullptr};
    std::atomic<float>* bypassParameter{nullptr};
    
    // Parameter smoothing for all parameters
    TylerAudio::Utils::SmoothingFilter wowSmoother;
    TylerAudio::Utils::SmoothingFilter lowCutFreqSmoother;
    TylerAudio::Utils::SmoothingFilter lowCutResSmoother;
    TylerAudio::Utils::SmoothingFilter highCutFreqSmoother;
    TylerAudio::Utils::SmoothingFilter highCutResSmoother;
    TylerAudio::Utils::SmoothingFilter dirtSmoother;
    TylerAudio::Utils::SmoothingFilter toneSmoother;
    
    // DSP Components
    
    // Wow modulation engine
    class WowEngine
    {
    public:
        void prepare(double sampleRate, int maxBlockSize, int numChannels = 2);
        void setDepth(float depth) noexcept;
        float getNextSample(float input, int channel) noexcept;
        void reset() noexcept;
        
    private:
        static constexpr float kWowFrequency = 0.5f;  // Hz
        static constexpr int kMaxDelayMs = 50;        // Maximum delay for pitch modulation
        
        std::vector<juce::dsp::DelayLine<float>> delayLines;
        juce::dsp::Oscillator<float> lfo;
        float depth{0.0f};
        float sampleRate{44100.0f};
        float currentDelay{0.0f};
        int numChannels{2};
    };
    
    // Resonant filter pair
    using FilterType = juce::dsp::IIR::Filter<float>;
    using Coefficients = juce::dsp::IIR::Coefficients<float>;
    
    juce::dsp::ProcessorDuplicator<FilterType, Coefficients> lowCutFilter;
    juce::dsp::ProcessorDuplicator<FilterType, Coefficients> highCutFilter;
    
    // Tape saturation processor
    class TapeSaturation
    {
    public:
        void prepare(double sampleRate);
        void setDrive(float drive) noexcept;
        float processSample(float input) noexcept;
        void reset() noexcept;
        
    private:
        float drive{0.0f};
        float previousSample{0.0f};  // For drive-dependent HF rolloff filter
        
        // Research-compliant constants
        static constexpr float kHighFreqRolloff = 0.9f;  // Base rolloff, increases with drive
    };
    
    // Tone control (tilt filter)
    class ToneControl
    {
    public:
        void prepare(double sampleRate);
        void setTone(float tone) noexcept;  // -1.0 to +1.0
        float processSample(float input) noexcept;
        void reset() noexcept;
        
    private:
        juce::dsp::IIR::Filter<float> lowShelf;
        juce::dsp::IIR::Filter<float> highShelf;
        float currentTone{0.0f};
        double sampleRate{44100.0};
        
        void updateCoefficients();
    };
    
    // DSP instances
    WowEngine wowEngine;
    TapeSaturation tapeSaturation;
    ToneControl toneControl;
    
    // Create parameter layout
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    // AudioProcessorValueTreeState::Listener interface
    void parameterChanged(const juce::String& parameterID, float newValue) override;
    
    // Helper methods
    void updateFilters();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TingeTapeAudioProcessor)
};