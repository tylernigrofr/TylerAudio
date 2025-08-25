#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include "TylerAudioCommon.h"
#include "audio_test_utils.h"

using namespace TylerAudio;
using namespace TylerAudio::Testing;

TEST_CASE("DSP utility function benchmarks", "[benchmark][dsp]")
{
    const int numIterations = 10000;
    
    BENCHMARK("dB to gain conversion")
    {
        volatile float result = 0.0f;  // volatile prevents optimization
        for (int i = 0; i < numIterations; ++i)
        {
            float db = -60.0f + (i * 120.0f / numIterations);  // -60dB to +60dB
            result += Utils::dbToGain(db);
        }
        return result;
    };
    
    BENCHMARK("Gain to dB conversion") 
    {
        volatile float result = 0.0f;
        for (int i = 0; i < numIterations; ++i)
        {
            float gain = 0.001f + (i * 9.999f / numIterations);  // 0.001 to 10.0
            result += Utils::gainToDb(gain);
        }
        return result;
    };
}

TEST_CASE("Audio buffer processing benchmarks", "[benchmark][audio]")
{
    const int sampleRate = 48000;
    const int bufferSize = 512;  // Typical audio buffer size
    
    auto testBuffer = generateTestTone(440.0f, 0.5f, sampleRate, bufferSize, 2);  // Stereo
    
    BENCHMARK("Audio buffer magnitude calculation")
    {
        return testBuffer.getMagnitude(0, 0, bufferSize);
    };
    
    BENCHMARK("Audio buffer RMS calculation")
    {
        return testBuffer.getRMSLevel(0, 0, bufferSize);
    };
    
    BENCHMARK("Audio buffer clear operation")
    {
        testBuffer.clear();
        return testBuffer.getSample(0, 0);
    };
    
    BENCHMARK("JUCE AudioBlock processing")
    {
        auto block = juce::dsp::AudioBlock<float>(testBuffer);
        block *= 0.5f;  // Simple gain operation
        return block.getSample(0, 0);
    };
}

TEST_CASE("Performance regression thresholds", "[benchmark][regression]")
{
    // These benchmarks establish performance baselines
    // CI should fail if performance degrades by more than 15%
    
    const int bufferSize = 512;
    auto buffer = generateWhiteNoise(0.5f, bufferSize, 2);
    
    BENCHMARK("Baseline audio processing") 
    {
        // Simple processing chain representative of typical plugin operations
        auto block = juce::dsp::AudioBlock<float>(buffer);
        
        // Gain adjustment
        block *= 0.8f;
        
        // Simple filter-like operation
        for (size_t channel = 0; channel < block.getNumChannels(); ++channel)
        {
            auto* samples = block.getChannelPointer(channel);
            float state = 0.0f;
            
            for (size_t sample = 0; sample < block.getNumSamples(); ++sample)
            {
                state = state * 0.99f + samples[sample] * 0.01f;  // Simple lowpass
                samples[sample] = state;
            }
        }
        
        return block.getSample(0, 0);
    };
}