#include "TestUtilities.h"
#include <random>
#include <algorithm>
#include <fstream>
#include <thread>
#include <atomic>

namespace TylerAudio {
namespace TestUtilities {

//==============================================================================
// SignalGenerator Implementation

void SignalGenerator::generateSignal(juce::AudioBuffer<float>& buffer, 
                                    SignalType type, 
                                    float frequency,
                                    double sampleRate,
                                    float amplitude,
                                    float phase) {
    const auto numSamples = buffer.getNumSamples();
    const auto numChannels = buffer.getNumChannels();
    
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::normal_distribution<float> whiteNoiseDist(0.0f, 1.0f);
    
    for (int channel = 0; channel < numChannels; ++channel) {
        auto* channelData = buffer.getWritePointer(channel);
        
        for (int sample = 0; sample < numSamples; ++sample) {
            float value = 0.0f;
            
            switch (type) {
                case SignalType::Silence:
                    value = 0.0f;
                    break;
                    
                case SignalType::Sine:
                    value = std::sin(2.0f * juce::MathConstants<float>::pi * frequency * sample / sampleRate + phase);
                    break;
                    
                case SignalType::Cosine:
                    value = std::cos(2.0f * juce::MathConstants<float>::pi * frequency * sample / sampleRate + phase);
                    break;
                    
                case SignalType::WhiteNoise:
                    value = whiteNoiseDist(gen);
                    break;
                    
                case SignalType::PinkNoise:
                    // Simplified pink noise using white noise filtering
                    value = whiteNoiseDist(gen) * 0.5f;
                    break;
                    
                case SignalType::Impulse:
                    value = (sample == 0) ? 1.0f : 0.0f;
                    break;
                    
                case SignalType::Step:
                    value = (sample >= numSamples / 2) ? 1.0f : 0.0f;
                    break;
                    
                case SignalType::Chirp:
                    // Linear frequency sweep
                    {
                        float t = static_cast<float>(sample) / sampleRate;
                        float instantFreq = frequency + (frequency * 10.0f - frequency) * t / (numSamples / sampleRate);
                        value = std::sin(2.0f * juce::MathConstants<float>::pi * instantFreq * t + phase);
                    }
                    break;
                    
                case SignalType::Square:
                    value = (std::sin(2.0f * juce::MathConstants<float>::pi * frequency * sample / sampleRate + phase) >= 0.0f) ? 1.0f : -1.0f;
                    break;
                    
                case SignalType::Sawtooth:
                    {
                        float t = std::fmod(frequency * sample / sampleRate + phase / (2.0f * juce::MathConstants<float>::pi), 1.0f);
                        value = 2.0f * t - 1.0f;
                    }
                    break;
                    
                case SignalType::Triangle:
                    {
                        float t = std::fmod(frequency * sample / sampleRate + phase / (2.0f * juce::MathConstants<float>::pi), 1.0f);
                        value = (t < 0.5f) ? (4.0f * t - 1.0f) : (3.0f - 4.0f * t);
                    }
                    break;
            }
            
            channelData[sample] = value * amplitude;
        }
    }
}

void SignalGenerator::generateMultiTone(juce::AudioBuffer<float>& buffer,
                                       const std::vector<float>& frequencies,
                                       const std::vector<float>& amplitudes,
                                       double sampleRate,
                                       float phase) {
    jassert(frequencies.size() == amplitudes.size());
    
    buffer.clear();
    
    for (size_t i = 0; i < frequencies.size(); ++i) {
        juce::AudioBuffer<float> tempBuffer(buffer.getNumChannels(), buffer.getNumSamples());
        generateSignal(tempBuffer, SignalType::Sine, frequencies[i], sampleRate, amplitudes[i], phase);
        
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
            buffer.addFrom(channel, 0, tempBuffer, channel, 0, buffer.getNumSamples());
        }
    }
}

//==============================================================================
// AudioAnalyzer Implementation

float AudioAnalyzer::calculateRMS(const juce::AudioBuffer<float>& buffer, int channel) {
    float sum = 0.0f;
    int totalSamples = 0;
    
    if (channel >= 0 && channel < buffer.getNumChannels()) {
        // Single channel
        const auto* data = buffer.getReadPointer(channel);
        for (int i = 0; i < buffer.getNumSamples(); ++i) {
            sum += data[i] * data[i];
        }
        totalSamples = buffer.getNumSamples();
    } else {
        // All channels
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
            const auto* data = buffer.getReadPointer(ch);
            for (int i = 0; i < buffer.getNumSamples(); ++i) {
                sum += data[i] * data[i];
            }
        }
        totalSamples = buffer.getNumSamples() * buffer.getNumChannels();
    }
    
    return std::sqrt(sum / totalSamples);
}

float AudioAnalyzer::calculatePeak(const juce::AudioBuffer<float>& buffer, int channel) {
    float peak = 0.0f;
    
    if (channel >= 0 && channel < buffer.getNumChannels()) {
        peak = buffer.getMagnitude(channel, 0, buffer.getNumSamples());
    } else {
        peak = buffer.getMagnitude(0, buffer.getNumSamples());
    }
    
    return peak;
}

float AudioAnalyzer::calculateTHDN(const juce::AudioBuffer<float>& input,
                                  const juce::AudioBuffer<float>& output,
                                  float fundamentalFreq,
                                  double sampleRate,
                                  int numHarmonics) {
    // Simplified THD+N calculation
    // In a real implementation, this would use FFT to analyze harmonic content
    
    float inputRMS = calculateRMS(input);
    float outputRMS = calculateRMS(output);
    float distortionRMS = calculateRMS(output) - calculateRMS(input);
    
    if (outputRMS < 1e-6f) return 0.0f;
    
    return std::abs(distortionRMS / outputRMS) * 100.0f;
}

//==============================================================================
// PerformanceMeter Implementation

PerformanceMeter::MeasurementResults PerformanceMeter::measureProcessingTime(
    std::function<void()> processor,
    int numIterations,
    int warmupIterations) {
    
    std::vector<double> times;
    times.reserve(numIterations);
    
    // Warmup
    for (int i = 0; i < warmupIterations; ++i) {
        processor();
    }
    
    // Actual measurements
    for (int i = 0; i < numIterations; ++i) {
        auto start = std::chrono::high_resolution_clock::now();
        processor();
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
        times.push_back(duration.count() / 1000000.0); // Convert to milliseconds
    }
    
    MeasurementResults results;
    results.numSamples = times.size();
    results.averageTimeMs = Statistics::mean(times);
    results.stdDeviationMs = Statistics::standardDeviation(times);
    auto minMax = Statistics::minMax(times);
    results.minTimeMs = minMax.first;
    results.maxTimeMs = minMax.second;
    
    // Simple CPU usage estimation (not completely accurate)
    results.cpuUsagePercent = results.averageTimeMs / 10.67; // Assuming 48kHz/512 samples ≈ 10.67ms
    results.memoryUsageBytes = 0; // Would need platform-specific implementation
    
    return results;
}

bool PerformanceMeter::validateRealtimePerformance(
    std::function<void()> processor,
    double sampleRate,
    int blockSize,
    double maxCPUPercent) {
    
    double blockTimeMs = (blockSize / sampleRate) * 1000.0;
    double maxAllowedTimeMs = blockTimeMs * (maxCPUPercent / 100.0);
    
    auto results = measureProcessingTime(processor, 1000, 100);
    
    return results.averageTimeMs < maxAllowedTimeMs && results.maxTimeMs < blockTimeMs;
}

//==============================================================================
// AudioQualityValidator Implementation

AudioQualityValidator::QualityMetrics AudioQualityValidator::analyzeAudioQuality(
    const juce::AudioBuffer<float>& input,
    const juce::AudioBuffer<float>& output,
    double sampleRate) {
    
    QualityMetrics metrics;
    
    // Check for problematic values
    metrics.hasInfiniteValues = !validateAudioIntegrity(output);
    metrics.hasNaNValues = metrics.hasInfiniteValues;
    metrics.hasClipping = hasClipping(output);
    metrics.hasAudioDropouts = hasDropouts(output);
    
    // Calculate basic metrics
    float outputRMS = AudioAnalyzer::calculateRMS(output);
    float outputPeak = AudioAnalyzer::calculatePeak(output);
    
    metrics.dynamicRange = 20.0f * std::log10(outputPeak / (outputRMS + 1e-10f));
    metrics.thdPlusN = AudioAnalyzer::calculateTHDN(input, output, 1000.0f, sampleRate);
    metrics.snr = 20.0f * std::log10(outputRMS / 1e-6f); // Simplified SNR calculation
    metrics.frequencyResponse = 0.0f; // Would require frequency domain analysis
    
    return metrics;
}

bool AudioQualityValidator::validateAudioIntegrity(const juce::AudioBuffer<float>& buffer) {
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        const auto* data = buffer.getReadPointer(channel);
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            if (!std::isfinite(data[sample])) {
                return false;
            }
        }
    }
    return true;
}

bool AudioQualityValidator::hasDropouts(const juce::AudioBuffer<float>& buffer, float threshold) {
    float thresholdLinear = juce::Decibels::decibelsToGain(threshold);
    
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        const auto* data = buffer.getReadPointer(channel);
        int consecutiveQuietSamples = 0;
        
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            if (std::abs(data[sample]) < thresholdLinear) {
                consecutiveQuietSamples++;
                if (consecutiveQuietSamples > 1024) { // Arbitrary threshold
                    return true;
                }
            } else {
                consecutiveQuietSamples = 0;
            }
        }
    }
    return false;
}

bool AudioQualityValidator::hasClipping(const juce::AudioBuffer<float>& buffer, float threshold) {
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        const auto* data = buffer.getReadPointer(channel);
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            if (std::abs(data[sample]) >= threshold) {
                return true;
            }
        }
    }
    return false;
}

//==============================================================================
// PluginTester Implementation

bool PluginTester::testBufferSizes(juce::AudioProcessor& processor, 
                                  const std::vector<int>& bufferSizes) {
    double sampleRate = 48000.0;
    
    for (int bufferSize : bufferSizes) {
        try {
            processor.prepareToPlay(sampleRate, bufferSize);
            
            juce::AudioBuffer<float> buffer(processor.getTotalNumInputChannels(), bufferSize);
            juce::MidiBuffer midiBuffer;
            
            SignalGenerator::generateSignal(buffer, SignalGenerator::SignalType::Sine, 1000.0f, sampleRate);
            
            processor.processBlock(buffer, midiBuffer);
            
            if (!AudioQualityValidator::validateAudioIntegrity(buffer)) {
                return false;
            }
        } catch (...) {
            return false;
        }
    }
    
    return true;
}

bool PluginTester::testSampleRates(juce::AudioProcessor& processor, 
                                  const std::vector<double>& sampleRates) {
    int bufferSize = 512;
    
    for (double sampleRate : sampleRates) {
        try {
            processor.prepareToPlay(sampleRate, bufferSize);
            
            juce::AudioBuffer<float> buffer(processor.getTotalNumInputChannels(), bufferSize);
            juce::MidiBuffer midiBuffer;
            
            SignalGenerator::generateSignal(buffer, SignalGenerator::SignalType::Sine, 1000.0f, sampleRate);
            
            processor.processBlock(buffer, midiBuffer);
            
            if (!AudioQualityValidator::validateAudioIntegrity(buffer)) {
                return false;
            }
        } catch (...) {
            return false;
        }
    }
    
    return true;
}

bool PluginTester::testParameterAutomation(juce::AudioProcessor& processor,
                                          int parameterIndex,
                                          float startValue,
                                          float endValue,
                                          int rampLengthSamples) {
    processor.prepareToPlay(48000.0, 512);
    
    auto& parameters = processor.getParameters();
    if (parameterIndex >= parameters.size()) {
        return false;
    }
    
    auto* parameter = parameters[parameterIndex];
    
    juce::AudioBuffer<float> buffer(processor.getTotalNumInputChannels(), 512);
    juce::MidiBuffer midiBuffer;
    
    int numBlocks = (rampLengthSamples + 511) / 512;
    
    for (int block = 0; block < numBlocks; ++block) {
        float progress = static_cast<float>(block) / numBlocks;
        float currentValue = startValue + (endValue - startValue) * progress;
        
        parameter->setValue(currentValue);
        
        SignalGenerator::generateSignal(buffer, SignalGenerator::SignalType::Sine, 1000.0f, 48000.0);
        processor.processBlock(buffer, midiBuffer);
        
        if (!AudioQualityValidator::validateAudioIntegrity(buffer)) {
            return false;
        }
    }
    
    return true;
}

bool PluginTester::testStateConsistency(juce::AudioProcessor& processor, int numIterations) {
    processor.prepareToPlay(48000.0, 512);
    
    juce::MemoryBlock originalState;
    processor.getStateInformation(originalState);
    
    for (int i = 0; i < numIterations; ++i) {
        juce::MemoryBlock state;
        processor.getStateInformation(state);
        processor.setStateInformation(state.getData(), static_cast<int>(state.getSize()));
        
        juce::MemoryBlock restoredState;
        processor.getStateInformation(restoredState);
        
        if (state.getSize() != restoredState.getSize() || 
            std::memcmp(state.getData(), restoredState.getData(), state.getSize()) != 0) {
            return false;
        }
    }
    
    return true;
}

//==============================================================================
// Statistics Implementation

double Statistics::mean(const std::vector<double>& data) {
    if (data.empty()) return 0.0;
    
    double sum = 0.0;
    for (double value : data) {
        sum += value;
    }
    return sum / data.size();
}

double Statistics::standardDeviation(const std::vector<double>& data) {
    if (data.size() < 2) return 0.0;
    
    double avg = mean(data);
    double sum = 0.0;
    
    for (double value : data) {
        double diff = value - avg;
        sum += diff * diff;
    }
    
    return std::sqrt(sum / (data.size() - 1));
}

double Statistics::median(std::vector<double> data) {
    if (data.empty()) return 0.0;
    
    std::sort(data.begin(), data.end());
    
    size_t n = data.size();
    if (n % 2 == 0) {
        return (data[n/2 - 1] + data[n/2]) / 2.0;
    } else {
        return data[n/2];
    }
}

std::pair<double, double> Statistics::minMax(const std::vector<double>& data) {
    if (data.empty()) return {0.0, 0.0};
    
    auto result = std::minmax_element(data.begin(), data.end());
    return {*result.first, *result.second};
}

//==============================================================================
// TestReporter Implementation

void TestReporter::log(LogLevel level, const juce::String& message) {
    auto timestamp = juce::Time::getCurrentTime().toString(true, true, true, true);
    
    juce::String levelStr;
    switch (level) {
        case LogLevel::Info:    levelStr = "INFO";    break;
        case LogLevel::Warning: levelStr = "WARNING"; break;
        case LogLevel::Error:   levelStr = "ERROR";   break;
        case LogLevel::Debug:   levelStr = "DEBUG";   break;
    }
    
    juce::String logMessage = "[" + timestamp + "] [" + levelStr + "] " + message;
    
    // Output to console and/or file
    std::cout << logMessage.toStdString() << std::endl;
    
    // Could also write to a log file here
}

void TestReporter::generateReport(const juce::String& testName,
                                 const std::vector<juce::String>& results,
                                 const juce::String& outputPath) {
    juce::File reportFile(outputPath);
    
    juce::String report;
    report << "Test Report: " << testName << "\n";
    report << "Generated: " << juce::Time::getCurrentTime().toString(true, true, true, true) << "\n";
    report << "=" << juce::String::repeatedString("=", 50) << "\n\n";
    
    for (const auto& result : results) {
        report << result << "\n";
    }
    
    reportFile.replaceWithText(report);
}

//==============================================================================
// TestFixtures Implementation

std::unique_ptr<juce::AudioBuffer<float>> TestFixtures::createTestBuffer(
    int numChannels,
    int numSamples,
    SignalGenerator::SignalType signalType,
    float frequency,
    double sampleRate) {
    
    auto buffer = std::make_unique<juce::AudioBuffer<float>>(numChannels, numSamples);
    SignalGenerator::generateSignal(*buffer, signalType, frequency, sampleRate);
    return buffer;
}

juce::AudioProcessorValueTreeState::ParameterLayout TestFixtures::createTestParameterLayout() {
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> parameters;
    
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        "testParam1", "Test Parameter 1",
        juce::NormalisableRange<float>(0.0f, 100.0f), 50.0f));
        
    parameters.push_back(std::make_unique<juce::AudioParameterBool>(
        "testParam2", "Test Parameter 2", false));
        
    parameters.push_back(std::make_unique<juce::AudioParameterChoice>(
        "testParam3", "Test Parameter 3",
        juce::StringArray{"Option1", "Option2", "Option3"}, 0));
    
    return {parameters.begin(), parameters.end()};
}

juce::MidiBuffer TestFixtures::createTestMidiBuffer(int numNotes, int startTime, int duration, int velocity) {
    juce::MidiBuffer buffer;
    
    for (int i = 0; i < numNotes; ++i) {
        int noteNumber = 60 + i; // C4 and up
        int noteOnTime = startTime + i * 100;
        int noteOffTime = noteOnTime + duration;
        
        buffer.addEvent(juce::MidiMessage::noteOn(1, noteNumber, static_cast<float>(velocity) / 127.0f), noteOnTime);
        buffer.addEvent(juce::MidiMessage::noteOff(1, noteNumber), noteOffTime);
    }
    
    return buffer;
}

} // namespace TestUtilities
} // namespace TylerAudio