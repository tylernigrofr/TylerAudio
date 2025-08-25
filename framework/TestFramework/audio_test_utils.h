#pragma once
#include <JuceHeader.h>
#include <catch2/catch_test_macros.hpp>
#include <random>

namespace TylerAudio::Testing
{
    // Audio buffer comparison with tolerance
    inline bool buffersMatch(const juce::AudioBuffer<float>& buffer1,
                            const juce::AudioBuffer<float>& buffer2,
                            float tolerance = 1e-6f)
    {
        if (buffer1.getNumChannels() != buffer2.getNumChannels() ||
            buffer1.getNumSamples() != buffer2.getNumSamples())
        {
            return false;
        }
        
        for (int channel = 0; channel < buffer1.getNumChannels(); ++channel)
        {
            auto* data1 = buffer1.getReadPointer(channel);
            auto* data2 = buffer2.getReadPointer(channel);
            
            for (int sample = 0; sample < buffer1.getNumSamples(); ++sample)
            {
                if (std::abs(data1[sample] - data2[sample]) > tolerance)
                {
                    return false;
                }
            }
        }
        return true;
    }
    
    // Generate test tone
    inline juce::AudioBuffer<float> generateTestTone(float frequency, 
                                                    float amplitude, 
                                                    double sampleRate, 
                                                    int numSamples,
                                                    int numChannels = 1)
    {
        juce::AudioBuffer<float> buffer(numChannels, numSamples);
        
        for (int channel = 0; channel < numChannels; ++channel)
        {
            auto* data = buffer.getWritePointer(channel);
            
            for (int sample = 0; sample < numSamples; ++sample)
            {
                double phase = 2.0 * juce::MathConstants<double>::pi * frequency * sample / sampleRate;
                data[sample] = amplitude * static_cast<float>(std::sin(phase));
            }
        }
        
        return buffer;
    }
    
    // Generate white noise
    inline juce::AudioBuffer<float> generateWhiteNoise(float amplitude,
                                                      int numSamples,
                                                      int numChannels = 1,
                                                      int seed = 12345)
    {
        juce::AudioBuffer<float> buffer(numChannels, numSamples);
        std::mt19937 rng(seed);
        std::uniform_real_distribution<float> dist(-amplitude, amplitude);
        
        for (int channel = 0; channel < numChannels; ++channel)
        {
            auto* data = buffer.getWritePointer(channel);
            
            for (int sample = 0; sample < numSamples; ++sample)
            {
                data[sample] = dist(rng);
            }
        }
        
        return buffer;
    }
    
    // Generate impulse (Dirac delta)
    inline juce::AudioBuffer<float> generateImpulse(float amplitude,
                                                   int numSamples,
                                                   int impulsePosition = 0,
                                                   int numChannels = 1)
    {
        juce::AudioBuffer<float> buffer(numChannels, numSamples);
        buffer.clear();
        
        if (impulsePosition >= 0 && impulsePosition < numSamples)
        {
            for (int channel = 0; channel < numChannels; ++channel)
            {
                buffer.setSample(channel, impulsePosition, amplitude);
            }
        }
        
        return buffer;
    }
    
    // Check for denormals in buffer
    inline bool hasDeNormals(const juce::AudioBuffer<float>& buffer)
    {
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        {
            auto* data = buffer.getReadPointer(channel);
            
            for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            {
                if (std::fpclassify(data[sample]) == FP_SUBNORMAL)
                {
                    return true;
                }
            }
        }
        return false;
    }
    
    // Check for NaN or infinite values
    inline bool hasInvalidValues(const juce::AudioBuffer<float>& buffer)
    {
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        {
            auto* data = buffer.getReadPointer(channel);
            
            for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            {
                if (!std::isfinite(data[sample]))
                {
                    return true;
                }
            }
        }
        return false;
    }
    
    // Measure RMS level
    inline float getRMSLevel(const juce::AudioBuffer<float>& buffer, int channel = 0)
    {
        if (channel >= buffer.getNumChannels())
            return 0.0f;
            
        auto* data = buffer.getReadPointer(channel);
        float sum = 0.0f;
        
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            sum += data[sample] * data[sample];
        }
        
        return std::sqrt(sum / buffer.getNumSamples());
    }
    
    // Simple performance timer
    class PerformanceTimer
    {
    public:
        void start() { startTime = std::chrono::high_resolution_clock::now(); }
        
        double getElapsedMilliseconds()
        {
            auto endTime = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
            return duration.count() / 1000.0;
        }
        
    private:
        std::chrono::high_resolution_clock::time_point startTime;
    };
}