#include "PluginProcessor.h"
#include "PluginEditor.h"

TingeTapeAudioProcessor::TingeTapeAudioProcessor()
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
    wowParameter = parameters.getRawParameterValue(TylerAudio::ParameterIDs::kWow);
    lowCutFreqParameter = parameters.getRawParameterValue(TylerAudio::ParameterIDs::kLowCutFreq);
    lowCutResParameter = parameters.getRawParameterValue(TylerAudio::ParameterIDs::kLowCutRes);
    highCutFreqParameter = parameters.getRawParameterValue(TylerAudio::ParameterIDs::kHighCutFreq);
    highCutResParameter = parameters.getRawParameterValue(TylerAudio::ParameterIDs::kHighCutRes);
    dirtParameter = parameters.getRawParameterValue(TylerAudio::ParameterIDs::kDirt);
    toneParameter = parameters.getRawParameterValue(TylerAudio::ParameterIDs::kTone);
    bypassParameter = parameters.getRawParameterValue(TylerAudio::ParameterIDs::kBypass);
    
    // Set up parameter callbacks for all parameters
    parameters.addParameterListener(TylerAudio::ParameterIDs::kWow, this);
    parameters.addParameterListener(TylerAudio::ParameterIDs::kLowCutFreq, this);
    parameters.addParameterListener(TylerAudio::ParameterIDs::kLowCutRes, this);
    parameters.addParameterListener(TylerAudio::ParameterIDs::kHighCutFreq, this);
    parameters.addParameterListener(TylerAudio::ParameterIDs::kHighCutRes, this);
    parameters.addParameterListener(TylerAudio::ParameterIDs::kDirt, this);
    parameters.addParameterListener(TylerAudio::ParameterIDs::kTone, this);
    parameters.addParameterListener(TylerAudio::ParameterIDs::kBypass, this);
}

TingeTapeAudioProcessor::~TingeTapeAudioProcessor()
{
}

const juce::String TingeTapeAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool TingeTapeAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool TingeTapeAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool TingeTapeAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double TingeTapeAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int TingeTapeAudioProcessor::getNumPrograms()
{
    return 1;
}

int TingeTapeAudioProcessor::getCurrentProgram()
{
    return 0;
}

void TingeTapeAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String TingeTapeAudioProcessor::getProgramName(int index)
{
    return {};
}

void TingeTapeAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

void TingeTapeAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Research-compliant parameter smoothing times
    // Wow parameters: 50ms (prevents modulation artifacts)
    const double wowSmoothingTime = 0.05;
    wowSmoother.setSmoothingTime(wowSmoothingTime, sampleRate);
    
    // Filter parameters: 20ms (prevents clicks)
    const double filterSmoothingTime = 0.02;
    lowCutFreqSmoother.setSmoothingTime(filterSmoothingTime, sampleRate);
    lowCutResSmoother.setSmoothingTime(filterSmoothingTime, sampleRate);
    highCutFreqSmoother.setSmoothingTime(filterSmoothingTime, sampleRate);
    highCutResSmoother.setSmoothingTime(filterSmoothingTime, sampleRate);
    toneSmoother.setSmoothingTime(filterSmoothingTime, sampleRate);  // Tone is filter-based
    
    // Drive parameters: 30ms (prevents level jumps)
    const double driveSmoothingTime = 0.03;
    dirtSmoother.setSmoothingTime(driveSmoothingTime, sampleRate);
    
    // Set smoother targets from current parameter values before snapping
    wowSmoother.setTargetValue(wowParameter->load());
    lowCutFreqSmoother.setTargetValue(lowCutFreqParameter->load());
    lowCutResSmoother.setTargetValue(lowCutResParameter->load());
    highCutFreqSmoother.setTargetValue(highCutFreqParameter->load());
    highCutResSmoother.setTargetValue(highCutResParameter->load());
    dirtSmoother.setTargetValue(dirtParameter->load());
    toneSmoother.setTargetValue(toneParameter->load());
    
    // Snap all smoothers to current values
    wowSmoother.snapToTarget();
    lowCutFreqSmoother.snapToTarget();
    lowCutResSmoother.snapToTarget();
    highCutFreqSmoother.snapToTarget();
    highCutResSmoother.snapToTarget();
    dirtSmoother.snapToTarget();
    toneSmoother.snapToTarget();
    
    // Prepare DSP components
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = static_cast<juce::uint32>(getTotalNumOutputChannels());
    
    // Prepare wow engine
    wowEngine.prepare(sampleRate, samplesPerBlock, static_cast<int>(spec.numChannels));
    
    // Prepare filters
    lowCutFilter.prepare(spec);
    highCutFilter.prepare(spec);
    
    // Prepare saturation and tone control
    tapeSaturation.prepare(sampleRate);
    toneControl.prepare(sampleRate);
    
    // Reset all DSP components
    lowCutFilter.reset();
    highCutFilter.reset();
    wowEngine.reset();
    tapeSaturation.reset();
    toneControl.reset();
    
    // Initialize filter coefficients with current parameter values
    updateFilters();
}

void TingeTapeAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool TingeTapeAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
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

void TingeTapeAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) noexcept
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
    
    // Update filter coefficients with smoothed parameters
    updateFilters();
    
    // Signal Chain: Input → Low-Cut Filter → Dirt/Saturation → Tone Control → High-Cut Filter → Wow Modulation → Output
    
    // Step 1: Apply Low-Cut Filter (High-Pass)
    juce::dsp::ProcessContextReplacing<float> context(block);
    lowCutFilter.process(context);
    
    // Step 2-6: Process each sample through the remaining chain
    for (int sample = 0; sample < numSamples; ++sample)
    {
        // Get smoothed parameters for this sample
        const float dirt = dirtSmoother.getNextValue();
        const float tone = toneSmoother.getNextValue();
        const float wow = wowSmoother.getNextValue();
        
        // Update DSP component parameters
        tapeSaturation.setDrive(dirt);
        toneControl.setTone(tone);
        wowEngine.setDepth(wow);
        
        // Process each channel
        for (size_t channel = 0; channel < block.getNumChannels(); ++channel)
        {
            float sample_val = block.getSample(channel, sample);
            
            // Step 2: Apply tape saturation/dirt
            sample_val = tapeSaturation.processSample(sample_val);
            
            // Step 3: Apply tone control
            sample_val = toneControl.processSample(sample_val);
            
            // Step 4: High-Cut Filter will be applied after sample loop
            
            // Step 5: Apply wow modulation (pitch modulation)
            sample_val = wowEngine.getNextSample(sample_val, static_cast<int>(channel));
            
            // Denormal protection and sanitization
            sample_val = TylerAudio::Utils::sanitizeFloat(sample_val);
            
            block.setSample(channel, sample, sample_val);
        }
    }
    
    // Step 4: Apply High-Cut Filter (Low-Pass) to entire block
    highCutFilter.process(context);
}

// Helper method to update filter coefficients
void TingeTapeAudioProcessor::updateFilters()
{
    const float lowCutFreq = lowCutFreqSmoother.getNextValue();
    const float lowCutRes = lowCutResSmoother.getNextValue();
    const float highCutFreq = highCutFreqSmoother.getNextValue();
    const float highCutRes = highCutResSmoother.getNextValue();
    
    const double sampleRate = getSampleRate();
    
    // Clamp frequencies to safe minimums to prevent 0 Hz coefficients
    const float clampedLowCutFreq = juce::jmax(20.0f, lowCutFreq);
    const float clampedHighCutFreq = juce::jmax(20.0f, highCutFreq);
    
    // Update Low-Cut Filter (High-Pass)
    auto lowCutCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighPass(
        sampleRate, clampedLowCutFreq, lowCutRes);
    *lowCutFilter.state = *lowCutCoeffs;
    
    // Update High-Cut Filter (Low-Pass)  
    auto highCutCoeffs = juce::dsp::IIR::Coefficients<float>::makeLowPass(
        sampleRate, clampedHighCutFreq, highCutRes);
    *highCutFilter.state = *highCutCoeffs;
}

bool TingeTapeAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* TingeTapeAudioProcessor::createEditor()
{
    return new TingeTapeAudioProcessorEditor(*this);
}

void TingeTapeAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void TingeTapeAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
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

juce::AudioProcessorValueTreeState::ParameterLayout TingeTapeAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // Wow parameter (0-100%) - Research-optimized default for subtle warmth
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        TylerAudio::ParameterIDs::kWow,
        "Wow",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f, 1.0f),
        25.0f,  // Optimized default: subtle tape character without being obvious
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(value, 1) + "%"; }
    ));

    // Low-Cut Frequency (20 Hz - 200 Hz) - Research-specified range
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        TylerAudio::ParameterIDs::kLowCutFreq,
        "Low Cut",
        juce::NormalisableRange<float>(20.0f, 200.0f, 1.0f, 0.3f),  // Logarithmic curve
        40.0f,  // Optimized default: gentle rumble removal without affecting bass
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(static_cast<int>(value)) + " Hz"; }
    ));

    // Low-Cut Resonance (0.1 - 2.0) - Research-specified range for musical control
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        TylerAudio::ParameterIDs::kLowCutRes,
        "Low Cut Q",
        juce::NormalisableRange<float>(0.1f, 2.0f, 0.01f, 1.0f),
        0.707f,  // Butterworth response - optimal default
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(value, 2); }
    ));

    // High-Cut Frequency (5 kHz - 20 kHz) - Research-specified range for tape character
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        TylerAudio::ParameterIDs::kHighCutFreq,
        "High Cut",
        juce::NormalisableRange<float>(5000.0f, 20000.0f, 10.0f, 0.3f),  // Logarithmic curve
        15000.0f,  // Optimized default: subtle high-frequency warmth
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { 
            return juce::String(value / 1000.0f, 1) + " kHz";
        }
    ));

    // High-Cut Resonance (0.1 - 2.0) - Research-specified range for musical control
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        TylerAudio::ParameterIDs::kHighCutRes,
        "High Cut Q",
        juce::NormalisableRange<float>(0.1f, 2.0f, 0.01f, 1.0f),
        0.707f,  // Butterworth response - optimal default
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(value, 2); }
    ));

    // Dirt/Saturation (0-100%) - Research-compliant 1x-10x gain scaling
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        TylerAudio::ParameterIDs::kDirt,
        "Dirt",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f, 1.0f),
        25.0f,  // Optimized default: subtle warmth and harmonic enhancement
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(value, 1) + "%"; }
    ));

    // Tone (-100% to +100%, dark to bright)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        TylerAudio::ParameterIDs::kTone,
        "Tone",
        juce::NormalisableRange<float>(-100.0f, 100.0f, 0.1f, 1.0f),
        0.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { 
            if (value > 0.0f)
                return "+" + juce::String(value, 1) + "%";
            else
                return juce::String(value, 1) + "%";
        }
    ));

    // Bypass parameter
    layout.add(std::make_unique<juce::AudioParameterBool>(
        TylerAudio::ParameterIDs::kBypass,
        "Bypass",
        false
    ));

    return layout;
}

void TingeTapeAudioProcessor::parameterChanged(const juce::String& parameterID, float newValue)
{
    if (parameterID == TylerAudio::ParameterIDs::kWow)
    {
        wowSmoother.setTargetValue(newValue);
    }
    else if (parameterID == TylerAudio::ParameterIDs::kLowCutFreq)
    {
        lowCutFreqSmoother.setTargetValue(newValue);
    }
    else if (parameterID == TylerAudio::ParameterIDs::kLowCutRes)
    {
        lowCutResSmoother.setTargetValue(newValue);
    }
    else if (parameterID == TylerAudio::ParameterIDs::kHighCutFreq)
    {
        highCutFreqSmoother.setTargetValue(newValue);
    }
    else if (parameterID == TylerAudio::ParameterIDs::kHighCutRes)
    {
        highCutResSmoother.setTargetValue(newValue);
    }
    else if (parameterID == TylerAudio::ParameterIDs::kDirt)
    {
        dirtSmoother.setTargetValue(newValue);
    }
    else if (parameterID == TylerAudio::ParameterIDs::kTone)
    {
        toneSmoother.setTargetValue(newValue);
    }
    // Bypass is handled directly in processBlock via atomic load
}

// =============================================================================
// DSP Class Implementations
// =============================================================================

// Wow Engine Implementation
void TingeTapeAudioProcessor::WowEngine::prepare(double sampleRate, int maxBlockSize, int numChannels)
{
    this->sampleRate = static_cast<float>(sampleRate);
    this->numChannels = numChannels;
    
    // Prepare delay lines for each channel
    delayLines.resize(numChannels);
    for (auto& delayLine : delayLines)
    {
        delayLine.setMaximumDelayInSamples(static_cast<int>(sampleRate * kMaxDelayMs / 1000.0f));
        delayLine.prepare({sampleRate, static_cast<juce::uint32>(maxBlockSize), 1});
    }
    
    // Prepare LFO - Research specification: 0.5Hz sine wave for authentic tape wow
    lfo.prepare({sampleRate, static_cast<juce::uint32>(maxBlockSize), 1});
    lfo.setFrequency(kWowFrequency);  // 0.5Hz fixed frequency per research
    lfo.initialise([](float x) { return std::sin(x); }, 128);
    
    reset();
}

void TingeTapeAudioProcessor::WowEngine::setDepth(float depth) noexcept
{
    this->depth = juce::jlimit(0.0f, 100.0f, depth) / 100.0f;
}

float TingeTapeAudioProcessor::WowEngine::getNextSample(float input, int channel) noexcept
{
    if (depth <= 0.001f || channel >= numChannels)
        return input;  // Bypass when depth is effectively zero or invalid channel
    
    // Generate LFO modulation (shared across channels for correlated wow)
    const float lfoValue = lfo.processSample(0.0f);
    
    // Research-compliant delay calculation:
    // modulatedDelayMs = baseDelayMs + (lfoOutput * depthParam * maxModulationMs)
    constexpr float baseDelayMs = 5.0f;        // Fixed 5ms base delay per research
    constexpr float maxModulationMs = 45.0f;   // 0-45ms modulation range per research
    
    // Calculate modulated delay in milliseconds
    const float modulatedDelayMs = baseDelayMs + (lfoValue * depth * maxModulationMs);
    
    // Convert to samples
    const float modulatedDelaySamples = modulatedDelayMs * sampleRate / 1000.0f;
    
    // Ensure delay is within valid range
    const float maxDelaySamples = sampleRate * kMaxDelayMs / 1000.0f;
    currentDelay = juce::jlimit(1.0f, maxDelaySamples - 1.0f, modulatedDelaySamples);
    
    // Set delay and get delayed sample for this channel
    delayLines[channel].setDelay(currentDelay);
    delayLines[channel].pushSample(0, input);
    
    return delayLines[channel].popSample(0);
}

void TingeTapeAudioProcessor::WowEngine::reset() noexcept
{
    for (auto& delayLine : delayLines)
        delayLine.reset();
    lfo.reset();
    currentDelay = 0.0f;
}

// Tape Saturation Implementation
void TingeTapeAudioProcessor::TapeSaturation::prepare(double sampleRate)
{
    juce::ignoreUnused(sampleRate);
    reset();
}

void TingeTapeAudioProcessor::TapeSaturation::setDrive(float drive) noexcept
{
    this->drive = juce::jlimit(0.0f, 100.0f, drive) / 100.0f;
}

float TingeTapeAudioProcessor::TapeSaturation::processSample(float input) noexcept
{
    if (drive <= 0.001f)
        return input;  // Bypass when drive is effectively zero
    
    // Research-compliant drive scaling: 1x to 10x gain (not 1x to 5x)
    const float driveGain = 1.0f + (drive * 9.0f);  // drive is 0-1, maps to 1x-10x gain
    
    // Research-compliant tanh saturation with proper normalization
    const float drivenInput = input * driveGain;
    float sample = std::tanh(drivenInput);
    
    // Proper tanh normalization for unity gain: output = tanh(input * driveGain) / tanh(driveGain)
    if (driveGain > 0.001f)
    {
        sample /= std::tanh(driveGain);
    }
    
    // Drive-dependent high-frequency rolloff (more rolloff with more drive)
    const float rolloffAmount = kHighFreqRolloff + (drive * 0.08f);  // Increase rolloff with drive
    const float alpha = juce::jlimit(0.1f, 0.98f, rolloffAmount);
    previousSample = alpha * previousSample + (1.0f - alpha) * sample;
    sample = previousSample;
    
    // Improved level compensation to maintain consistent output levels
    const float compensationFactor = 1.0f / (1.0f + drive * 0.5f);  // Gentle compensation
    sample *= compensationFactor;
    
    // Denormal protection
    if (std::fpclassify(sample) == FP_SUBNORMAL)
        sample = 0.0f;
    
    return sample;
}

void TingeTapeAudioProcessor::TapeSaturation::reset() noexcept
{
    previousSample = 0.0f;
}

// Tone Control Implementation
void TingeTapeAudioProcessor::ToneControl::prepare(double sampleRate)
{
    this->sampleRate = sampleRate;
    
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = 512;  // Not critical for single-sample processing
    spec.numChannels = 1;
    
    lowShelf.prepare(spec);
    highShelf.prepare(spec);
    
    reset();
    updateCoefficients();
}

void TingeTapeAudioProcessor::ToneControl::setTone(float tone) noexcept
{
    const float newTone = juce::jlimit(-100.0f, 100.0f, tone) / 100.0f;  // Normalize to -1.0 to +1.0
    
    if (std::abs(newTone - currentTone) > 0.001f)
    {
        currentTone = newTone;
        updateCoefficients();
    }
}

float TingeTapeAudioProcessor::ToneControl::processSample(float input) noexcept
{
    if (std::abs(currentTone) <= 0.001f)
        return input;  // Bypass when tone is effectively zero
    
    // Process through both shelf filters
    float sample = input;
    sample = lowShelf.processSample(sample);
    sample = highShelf.processSample(sample);
    
    return sample;
}

void TingeTapeAudioProcessor::ToneControl::reset() noexcept
{
    lowShelf.reset();
    highShelf.reset();
}

void TingeTapeAudioProcessor::ToneControl::updateCoefficients()
{
    // Research-compliant shelf frequencies and gain range
    constexpr float lowFreq = 250.0f;    // Low shelf frequency per research
    constexpr float highFreq = 5000.0f;  // High shelf frequency per research  
    constexpr float maxGainDb = 6.0f;    // Research-specified ±6dB range (was ±12dB)
    
    // Calculate gains for tilt filter effect
    const float gainDb = currentTone * maxGainDb;
    
    // Low shelf: boost when tone is negative (darker), cut when positive (brighter)
    const float lowGainDb = -gainDb;
    auto lowCoeffs = juce::dsp::IIR::Coefficients<float>::makeLowShelf(
        sampleRate, lowFreq, 0.707f, juce::Decibels::decibelsToGain(lowGainDb));
    *lowShelf.coefficients = *lowCoeffs;
    
    // High shelf: cut when tone is negative (darker), boost when positive (brighter)  
    const float highGainDb = gainDb;
    auto highCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighShelf(
        sampleRate, highFreq, 0.707f, juce::Decibels::decibelsToGain(highGainDb));
    *highShelf.coefficients = *highCoeffs;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TingeTapeAudioProcessor();
}