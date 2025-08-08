# JUCE DSP Implementation Patterns

## Overview
Best practices and proven patterns for implementing realtime audio DSP in JUCE framework, specifically tailored for tape emulation plugin development.

## Core JUCE DSP Architecture

### 1. AudioProcessor Lifecycle Management
**Critical Phases**:
```cpp
// prepareToPlay(): Initialize DSP components
void TingeTapeProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    // Spec for all DSP components
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumInputChannels();
    
    // Initialize all DSP components with spec
    wowEngine.prepare(spec);
    lowCutFilter.prepare(spec);
    highCutFilter.prepare(spec);
    tapeSaturation.prepare(spec);
    toneControl.prepare(spec);
    
    // Pre-allocate any additional buffers
    wetBuffer.setSize(spec.numChannels, spec.maximumBlockSize);
}
```

### 2. Parameter Management Best Practices
**AudioProcessorValueTreeState Pattern**:
```cpp
// In constructor
parameters(*this, nullptr, juce::Identifier("TingeTape"), createParameterLayout())
{
    // Atomic parameter pointers for realtime access
    wowDepthParam = parameters.getRawParameterValue("wowDepth");
    driveParam = parameters.getRawParameterValue("drive");
    toneParam = parameters.getRawParameterValue("tone");
}

// Parameter layout creation
juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout() {
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "wowDepth", "Wow Depth",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 25.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(value, 1) + "%"; }
    ));
    
    return { params.begin(), params.end() };
}
```

### 3. Realtime-Safe Parameter Access
**Atomic Parameter Reading**:
```cpp
class SmoothingFilter {
public:
    void setTargetValue(float newValue) noexcept {
        targetValue.store(newValue);
    }
    
    float getNextValue() noexcept {
        float target = targetValue.load();
        currentValue += (target - currentValue) * smoothingCoeff;
        return currentValue;
    }
    
private:
    std::atomic<float> targetValue{0.0f};
    float currentValue{0.0f};
    float smoothingCoeff{0.01f}; // ~50ms at 48kHz
};
```

## DSP Component Implementation Patterns

### 1. DelayLine for Wow Modulation
**Best Practices**:
```cpp
class WowEngine {
public:
    void prepare(const juce::dsp::ProcessSpec& spec) {
        // Size delay line for maximum modulation depth
        auto maxDelayInSamples = static_cast<int>(spec.sampleRate * maxDelayMs / 1000.0);
        delayLine.prepare(spec);
        delayLine.setMaximumDelayInSamples(maxDelayInSamples);
        
        // Initialize LFO
        lfo.prepare(spec);
        lfo.initialise([](float x) { return std::sin(x); });
        lfo.setFrequency(0.5f); // Default wow frequency
    }
    
    template<typename ProcessContext>
    void process(const ProcessContext& context) noexcept {
        auto&& inBlock = context.getInputBlock();
        auto&& outBlock = context.getOutputBlock();
        
        for (size_t channel = 0; channel < inBlock.getNumChannels(); ++channel) {
            auto* input = inBlock.getChannelPointer(channel);
            auto* output = outBlock.getChannelPointer(channel);
            
            for (size_t i = 0; i < inBlock.getNumSamples(); ++i) {
                // Get current LFO value
                float lfoValue = lfo.processSample(0.0f);
                
                // Calculate modulated delay time
                float modulatedDelay = baseDelayMs + (lfoValue * depthSmoothing.getNextValue() * maxDelayMs);
                float delaySamples = modulatedDelay * sampleRate / 1000.0f;
                
                // Set delay and process sample
                delayLine.setDelay(delaySamples);
                output[i] = delayLine.processSample(channel, input[i]);
            }
        }
    }
    
private:
    juce::dsp::DelayLine<float> delayLine{48000}; // 1 second max
    juce::dsp::Oscillator<float> lfo;
    SmoothingFilter depthSmoothing;
    float sampleRate{48000.0f};
    static constexpr float maxDelayMs = 50.0f;
    static constexpr float baseDelayMs = 5.0f;
};
```

### 2. IIR Filter Implementation
**Resonant Filter Pattern**:
```cpp
class ResonantFilter {
public:
    enum FilterType { LowPass, HighPass, BandPass };
    
    void prepare(const juce::dsp::ProcessSpec& spec) {
        filter.prepare(spec);
        updateFilter();
    }
    
    void setParameters(float frequency, float resonance, FilterType type) {
        cutoffFreq = frequency;
        qValue = resonance;
        filterType = type;
        updateFilter();
    }
    
    template<typename ProcessContext>
    void process(const ProcessContext& context) noexcept {
        // Update filter coefficients if parameters changed
        if (needsUpdate.exchange(false)) {
            updateFilter();
        }
        
        filter.process(context);
    }
    
private:
    void updateFilter() {
        auto coefficients = [&]() {
            switch (filterType) {
                case LowPass:
                    return juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, cutoffFreq, qValue);
                case HighPass:
                    return juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, cutoffFreq, qValue);
                case BandPass:
                    return juce::dsp::IIR::Coefficients<float>::makeBandPass(sampleRate, cutoffFreq, qValue);
                default:
                    return juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, cutoffFreq, qValue);
            }
        }();
        
        *filter.state = *coefficients;
    }
    
    juce::dsp::IIR::Filter<float> filter;
    std::atomic<bool> needsUpdate{true};
    float cutoffFreq{1000.0f};
    float qValue{0.7f};
    FilterType filterType{LowPass};
    double sampleRate{48000.0};
};
```

### 3. Saturation/Distortion Pattern
**tanh Saturation Implementation**:
```cpp
class TapeSaturation {
public:
    void prepare(const juce::dsp::ProcessSpec& spec) {
        // Pre-filter for tape-like frequency shaping
        preFilter.prepare(spec);
        preFilter.setType(juce::dsp::FirstOrderTPTFilter<float>::Type::lowpass);
        preFilter.setCutoffFrequency(15000.0f);
    }
    
    template<typename ProcessContext>
    void process(const ProcessContext& context) noexcept {
        auto&& block = context.getOutputBlock();
        
        // Apply pre-filtering
        preFilter.process(context);
        
        // Process saturation
        for (size_t channel = 0; channel < block.getNumChannels(); ++channel) {
            auto* data = block.getChannelPointer(channel);
            
            for (size_t i = 0; i < block.getNumSamples(); ++i) {
                float drive = driveSmoothing.getNextValue();
                float input = data[i] * drive;
                
                // tanh saturation with normalization
                float output = std::tanh(input);
                if (drive > 0.001f) {
                    output /= std::tanh(drive); // Normalize for unity gain
                }
                
                data[i] = output * outputGain;
            }
        }
    }
    
private:
    juce::dsp::FirstOrderTPTFilter<float> preFilter;
    SmoothingFilter driveSmoothing;
    float outputGain{0.7f}; // Compensate for saturation level
};
```

## Performance Optimization Patterns

### 1. Block-Based Processing
**Efficient Block Processing**:
```cpp
void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) override {
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    // Clear unused output channels
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i) {
        buffer.clear(i, 0, buffer.getNumSamples());
    }
    
    // Update parameters from UI thread (atomic reads)
    updateParameters();
    
    // Create AudioBlock for DSP processing
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    
    // Process signal chain
    lowCutFilter.process(context);
    tapeSaturation.process(context);
    toneControl.process(context);
    highCutFilter.process(context);
    wowEngine.process(context);
    
    // Apply bypass if needed
    if (bypassParam->load() > 0.5f) {
        // Implement smooth bypass transition
    }
}

void updateParameters() {
    // Read atomic parameters and update smoothing filters
    wowEngine.setDepth(wowDepthParam->load());
    tapeSaturation.setDrive(driveParam->load());
    toneControl.setAmount(toneParam->load());
    // ... other parameter updates
}
```

### 2. Memory Management Patterns
**Buffer Management**:
```cpp
class TingeTapeProcessor {
private:
    // Pre-allocated working buffers
    juce::AudioBuffer<float> wetBuffer;
    juce::AudioBuffer<float> dryBuffer;
    
    // DSP components with pre-allocated internal buffers
    WowEngine wowEngine;
    ResonantFilter lowCutFilter, highCutFilter;
    TapeSaturation tapeSaturation;
    
public:
    void prepareToPlay(double sampleRate, int samplesPerBlock) override {
        // Resize buffers once during preparation
        wetBuffer.setSize(getTotalNumInputChannels(), samplesPerBlock);
        dryBuffer.setSize(getTotalNumInputChannels(), samplesPerBlock);
        
        // Clear buffers
        wetBuffer.clear();
        dryBuffer.clear();
        
        // Initialize DSP components
        // ... DSP preparation code
    }
};
```

### 3. Denormal Protection
**Preventing CPU Spikes**:
```cpp
inline float denormalKill(float value) noexcept {
    static constexpr float threshold = 1e-15f;
    return (std::abs(value) < threshold) ? 0.0f : value;
}

// In processing loop
for (size_t i = 0; i < numSamples; ++i) {
    float processed = processAudioSample(input[i]);
    output[i] = denormalKill(processed);
}
```

## Thread Safety Patterns

### 1. Lock-Free Parameter Updates
**Atomic Parameter Pattern**:
```cpp
class ParameterManager {
public:
    void setParameterValue(const juce::String& paramID, float value) {
        // Called from UI thread
        if (paramID == "wowDepth") {
            wowDepth.store(value);
        } else if (paramID == "drive") {
            drive.store(value);
        }
        // ... handle other parameters
    }
    
    struct Parameters {
        float wowDepth;
        float drive;
        float tone;
        // ... other parameters
    };
    
    Parameters getCurrentParameters() noexcept {
        // Called from audio thread
        return {
            wowDepth.load(),
            drive.load(),
            tone.load()
            // ... load other parameters
        };
    }
    
private:
    std::atomic<float> wowDepth{25.0f};
    std::atomic<float> drive{50.0f};
    std::atomic<float> tone{0.0f};
    // ... other atomic parameters
};
```

### 2. UI Update Pattern
**Non-Blocking UI Updates**:
```cpp
class TingeTapeEditor : public juce::AudioProcessorEditor, 
                       private juce::Timer {
public:
    TingeTapeEditor(TingeTapeProcessor& p) : processor(p) {
        startTimerHz(30); // 30 FPS UI updates
    }
    
private:
    void timerCallback() override {
        // Update UI elements from parameter values
        // This runs on the message thread
        updateSliderValues();
        updateVUMeters();
    }
    
    void updateSliderValues() {
        auto params = processor.getParameterManager().getCurrentParameters();
        
        if (std::abs(wowSlider.getValue() - params.wowDepth) > 0.01) {
            wowSlider.setValue(params.wowDepth, juce::dontSendNotification);
        }
        
        // ... update other sliders
    }
};
```

## Error Handling and Validation

### 1. Parameter Range Validation
**Safe Parameter Handling**:
```cpp
class ParameterValidator {
public:
    static float clampWowDepth(float value) {
        return juce::jlimit(0.0f, 100.0f, value);
    }
    
    static float clampFrequency(float value) {
        return juce::jlimit(20.0f, 20000.0f, value);
    }
    
    static float clampResonance(float value) {
        return juce::jlimit(0.1f, 10.0f, value);
    }
};
```

### 2. DSP State Validation
**Stability Checks**:
```cpp
bool validateDSPState() const {
    // Check for NaN or infinite values
    if (!std::isfinite(lastOutputSample)) {
        juce::Logger::writeToLog("DSP State Error: Non-finite output detected");
        return false;
    }
    
    // Check filter stability
    if (!lowCutFilter.isStable()) {
        juce::Logger::writeToLog("DSP State Error: Low-cut filter unstable");
        return false;
    }
    
    return true;
}
```

## Testing and Validation Patterns

### 1. Unit Testing DSP Components
**Component Testing Framework**:
```cpp
class WowEngineTest : public juce::UnitTest {
public:
    WowEngineTest() : juce::UnitTest("WowEngine") {}
    
    void runTest() override {
        beginTest("WowEngine basic functionality");
        
        WowEngine engine;
        juce::dsp::ProcessSpec spec{48000.0, 512, 2};
        engine.prepare(spec);
        
        // Test with impulse
        juce::AudioBuffer<float> testBuffer(2, 512);
        testBuffer.clear();
        testBuffer.setSample(0, 0, 1.0f); // Impulse
        
        juce::dsp::AudioBlock<float> block(testBuffer);
        juce::dsp::ProcessContextReplacing<float> context(block);
        
        engine.process(context);
        
        // Verify output is not NaN and within expected range
        auto* output = testBuffer.getReadPointer(0);
        for (int i = 0; i < 512; ++i) {
            expect(std::isfinite(output[i]), "Output should be finite");
            expect(std::abs(output[i]) <= 2.0f, "Output should be reasonable amplitude");
        }
    }
};
```

## Key Takeaways

1. **Parameter Management**: Use AudioProcessorValueTreeState with atomic access
2. **DSP Processing**: Leverage JUCE's ProcessContext pattern for efficiency
3. **Memory Safety**: Pre-allocate all buffers in prepareToPlay()
4. **Thread Safety**: Atomic parameters, lock-free audio processing
5. **Performance**: Block-based processing, denormal protection
6. **Stability**: Parameter validation, DSP state monitoring
7. **Testing**: Comprehensive unit tests for each DSP component

These patterns ensure robust, efficient, and maintainable JUCE audio plugin development while meeting professional quality standards.