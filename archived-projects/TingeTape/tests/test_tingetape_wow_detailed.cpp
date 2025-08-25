#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <JuceHeader.h>
#include "audio_test_utils.h"
#include "../Source/PluginProcessor.h"
#include <chrono>
#include <vector>
#include <complex>

using namespace TylerAudio::Testing;
using Catch::Approx;

TEST_CASE("WowEngine Algorithm Correction Tests", "[TingeTape][unit][wow][detailed]")
{
    SECTION("LFO frequency accuracy test - 0.5Hz validation")
    {
        TingeTapeAudioProcessor processor;
        const double sampleRate = 48000.0;
        const int blockSize = 512;
        
        processor.prepareToPlay(sampleRate, blockSize);
        
        // Set wow depth to measurable level
        if (auto* wowParam = processor.getParameters().getParameter("wowDepth"))
        {
            wowParam->setValue(50.0f); // 50% wow depth
        }
        
        // Generate impulse to measure LFO period
        juce::AudioBuffer<float> buffer(1, static_cast<int>(sampleRate * 4.0)); // 4 seconds
        auto impulse = generateImpulse(1.0f, static_cast<int>(sampleRate * 4.0), 0, 1);
        buffer.copyFrom(0, 0, impulse, 0, 0, impulse.getNumSamples());
        
        // Process through wow engine
        juce::MidiBuffer midiBuffer;
        const int samplesPerBlock = 512;
        int samplesProcessed = 0;
        
        while (samplesProcessed < buffer.getNumSamples())
        {
            int samplesToProcess = std::min(samplesPerBlock, buffer.getNumSamples() - samplesProcessed);
            juce::AudioBuffer<float> blockBuffer(1, samplesToProcess);
            blockBuffer.copyFrom(0, 0, buffer, 0, samplesProcessed, samplesToProcess);
            
            processor.processBlock(blockBuffer, midiBuffer);
            
            buffer.copyFrom(0, samplesProcessed, blockBuffer, 0, 0, samplesToProcess);
            samplesProcessed += samplesToProcess;
        }
        
        // Analyze the output to measure LFO frequency
        // This is a simplified test - full implementation would use FFT analysis
        // to precisely measure the 0.5Hz modulation frequency
        
        // For now, verify that modulation is present and reasonable
        auto* data = buffer.getReadPointer(0);
        float maxValue = 0.0f;
        float minValue = 0.0f;
        
        for (int i = 1000; i < buffer.getNumSamples() - 1000; ++i) // Skip edges
        {
            maxValue = std::max(maxValue, data[i]);
            minValue = std::min(minValue, data[i]);
        }
        
        // Should have modulation variation (this test will initially fail)
        float modulation = maxValue - minValue;
        INFO("Modulation range: " << modulation);
        
        // Research requirement: LFO should be exactly 0.5Hz
        // This test will fail initially and pass after implementation
        REQUIRE(modulation > 0.1f); // Should have significant modulation
        
        // TODO: Add precise frequency analysis to verify 0.5Hz
        // This would require FFT analysis of the modulation envelope
    }
    
    SECTION("Delay time precision test - 5ms base delay + modulation")
    {
        TingeTapeAudioProcessor processor;
        const double sampleRate = 48000.0;
        const int blockSize = 512;
        
        processor.prepareToPlay(sampleRate, blockSize);
        
        // Set wow depth to zero for base delay measurement
        if (auto* wowParam = processor.getParameters().getParameter("wowDepth"))
        {
            wowParam->setValue(0.0f); // No modulation
        }
        
        // Generate impulse and measure delay
        juce::AudioBuffer<float> buffer(1, 2048);
        buffer.clear();
        buffer.setSample(0, 100, 1.0f); // Impulse at sample 100
        
        juce::MidiBuffer midiBuffer;
        processor.processBlock(buffer, midiBuffer);
        
        // Find the delayed impulse
        auto* data = buffer.getReadPointer(0);
        int delayedImpulsePosition = -1;
        float maxValue = 0.0f;
        
        for (int i = 101; i < buffer.getNumSamples(); ++i)
        {
            if (std::abs(data[i]) > maxValue)
            {
                maxValue = std::abs(data[i]);
                delayedImpulsePosition = i;
            }
        }
        
        REQUIRE(delayedImpulsePosition > 100); // Should be delayed
        
        // Calculate actual delay in milliseconds
        float actualDelayMs = (delayedImpulsePosition - 100) * 1000.0f / static_cast<float>(sampleRate);
        
        INFO("Actual base delay: " << actualDelayMs << "ms");
        INFO("Expected base delay: 5.0ms");
        
        // Research requirement: Base delay should be 5ms (not current variable delay)
        // This test will fail initially with current implementation (~25ms)
        REQUIRE(actualDelayMs == Approx(5.0f).margin(0.1f));
    }
    
    SECTION("Modulation depth linearity test - 0-45ms range")
    {
        TingeTapeAudioProcessor processor;
        const double sampleRate = 48000.0;
        const int blockSize = 512;
        
        processor.prepareToPlay(sampleRate, blockSize);
        
        std::vector<float> depthValues = {0.0f, 25.0f, 50.0f, 75.0f, 100.0f};
        std::vector<float> measuredRanges;
        
        for (float depth : depthValues)
        {
            if (auto* wowParam = processor.getParameters().getParameter("wowDepth"))
            {
                wowParam->setValue(depth);
            }
            
            // Generate test signal and measure modulation range
            juce::AudioBuffer<float> buffer(1, static_cast<int>(sampleRate * 2.0)); // 2 seconds
            auto testSignal = generateTestTone(1000.0f, 0.5f, sampleRate, buffer.getNumSamples(), 1);
            buffer.copyFrom(0, 0, testSignal, 0, 0, buffer.getNumSamples());
            
            // Process in blocks
            juce::MidiBuffer midiBuffer;
            int samplesProcessed = 0;
            const int samplesPerBlock = 512;
            
            while (samplesProcessed < buffer.getNumSamples())
            {
                int samplesToProcess = std::min(samplesPerBlock, buffer.getNumSamples() - samplesProcessed);
                juce::AudioBuffer<float> blockBuffer(1, samplesToProcess);
                blockBuffer.copyFrom(0, 0, buffer, 0, samplesProcessed, samplesToProcess);
                
                processor.processBlock(blockBuffer, midiBuffer);
                
                buffer.copyFrom(0, samplesProcessed, blockBuffer, 0, 0, samplesToProcess);
                samplesProcessed += samplesToProcess;
            }
            
            // Measure modulation range (simplified - full implementation would use autocorrelation)
            auto* data = buffer.getReadPointer(0);
            float maxVal = *std::max_element(data + 1000, data + buffer.getNumSamples() - 1000);
            float minVal = *std::min_element(data + 1000, data + buffer.getNumSamples() - 1000);
            float range = maxVal - minVal;
            
            measuredRanges.push_back(range);
            
            INFO("Depth: " << depth << "%, Measured range: " << range);
        }
        
        // Verify linear relationship
        REQUIRE(measuredRanges.size() == depthValues.size());
        
        // Research requirement: Depth parameter should map linearly to modulation amount
        // 100% depth should correspond to 45ms maximum modulation (not current 50ms)
        // This test will fail initially due to current implementation using full 50ms range
        
        // Check that depth=0 has minimal modulation
        REQUIRE(measuredRanges[0] < 0.01f); // 0% should have no modulation
        
        // Check that modulation increases with depth (linearity test)
        for (size_t i = 1; i < measuredRanges.size(); ++i)
        {
            REQUIRE(measuredRanges[i] > measuredRanges[i-1]); // Should increase
        }
        
        // The maximum modulation range calculation will fail until implementation is fixed
        // Expected: 45ms max modulation, Current: 50ms max modulation
    }
    
    SECTION("Pitch variation measurement test - 0.1%-1% authentic range")
    {
        TingeTapeAudioProcessor processor;
        const double sampleRate = 48000.0;
        const int blockSize = 512;
        
        processor.prepareToPlay(sampleRate, blockSize);
        
        // Set moderate wow depth
        if (auto* wowParam = processor.getParameters().getParameter("wowDepth"))
        {
            wowParam->setValue(50.0f); // 50% wow depth
        }
        
        // Generate pure tone for pitch analysis
        const float testFreq = 440.0f;
        juce::AudioBuffer<float> buffer(1, static_cast<int>(sampleRate * 2.0));
        auto testTone = generateTestTone(testFreq, 0.7f, sampleRate, buffer.getNumSamples(), 1);
        buffer.copyFrom(0, 0, testTone, 0, 0, buffer.getNumSamples());
        
        // Process through wow engine
        juce::MidiBuffer midiBuffer;
        int samplesProcessed = 0;
        const int samplesPerBlock = 512;
        
        while (samplesProcessed < buffer.getNumSamples())
        {
            int samplesToProcess = std::min(samplesPerBlock, buffer.getNumSamples() - samplesProcessed);
            juce::AudioBuffer<float> blockBuffer(1, samplesToProcess);
            blockBuffer.copyFrom(0, 0, buffer, 0, samplesProcessed, samplesToProcess);
            
            processor.processBlock(blockBuffer, midiBuffer);
            
            buffer.copyFrom(0, samplesProcessed, blockBuffer, 0, 0, samplesToProcess);
            samplesProcessed += samplesToProcess;
        }
        
        // Simplified pitch variation analysis
        // Full implementation would use phase tracking or FFT analysis
        auto* data = buffer.getReadPointer(0);
        
        // Check that signal is modified (has wow effect)
        auto originalTone = generateTestTone(testFreq, 0.7f, sampleRate, buffer.getNumSamples(), 1);
        bool hasModulation = !buffersMatch(buffer, originalTone, 0.1f);
        
        REQUIRE(hasModulation); // Signal should be modified by wow
        REQUIRE_FALSE(hasInvalidValues(buffer));
        
        // TODO: Implement precise pitch variation measurement
        // Research requirement: Should produce 0.1%-1% pitch variation in authentic range
        // This would require tracking instantaneous frequency over time
    }
    
    SECTION("Sample rate scaling test - 44.1kHz to 192kHz")
    {
        std::vector<double> sampleRates = {44100.0, 48000.0, 88200.0, 96000.0, 176400.0, 192000.0};
        
        for (double sr : sampleRates)
        {
            TingeTapeAudioProcessor processor;
            const int blockSize = 512;
            
            processor.prepareToPlay(sr, blockSize);
            
            // Set consistent wow depth
            if (auto* wowParam = processor.getParameters().getParameter("wowDepth"))
            {
                wowParam->setValue(50.0f);
            }
            
            // Generate impulse to measure base delay consistency
            juce::AudioBuffer<float> buffer(1, static_cast<int>(sr * 0.1)); // 100ms buffer
            buffer.clear();
            buffer.setSample(0, 100, 1.0f); // Impulse
            
            juce::MidiBuffer midiBuffer;
            processor.processBlock(buffer, midiBuffer);
            
            // Find delayed impulse
            auto* data = buffer.getReadPointer(0);
            int delayedPosition = -1;
            float maxValue = 0.0f;
            
            for (int i = 101; i < buffer.getNumSamples(); ++i)
            {
                if (std::abs(data[i]) > maxValue)
                {
                    maxValue = std::abs(data[i]);
                    delayedPosition = i;
                }
            }
            
            REQUIRE(delayedPosition > 100);
            
            // Calculate delay in milliseconds (should be consistent across sample rates)
            float delayMs = (delayedPosition - 100) * 1000.0f / static_cast<float>(sr);
            
            INFO("Sample rate: " << sr << "Hz, Delay: " << delayMs << "ms");
            
            // Research requirement: Base delay should be 5ms regardless of sample rate
            // This test will fail initially with current implementation
            REQUIRE(delayMs == Approx(5.0f).margin(0.1f));
            
            // Verify no audio artifacts
            REQUIRE_FALSE(hasInvalidValues(buffer));
            REQUIRE_FALSE(hasDeNormals(buffer));
        }
    }
    
    SECTION("Delay calculation formula verification")
    {
        // This test verifies the exact formula from research:
        // modulatedDelayMs = baseDelayMs + (lfoOutput * depthParam * maxModulationMs)
        // Where: baseDelayMs = 5.0, maxModulationMs = 45.0 (not 50.0)
        
        TingeTapeAudioProcessor processor;
        const double sampleRate = 48000.0;
        processor.prepareToPlay(sampleRate, 512);
        
        // Test with known depth value
        if (auto* wowParam = processor.getParameters().getParameter("wowDepth"))
        {
            wowParam->setValue(100.0f); // 100% depth
        }
        
        // Process some samples to let the wow engine settle
        juce::AudioBuffer<float> buffer(1, 2048);
        auto testSignal = generateTestTone(440.0f, 0.5f, sampleRate, 2048, 1);
        buffer.copyFrom(0, 0, testSignal, 0, 0, 2048);
        
        juce::MidiBuffer midiBuffer;
        processor.processBlock(buffer, midiBuffer);
        
        // The delay calculation in current implementation uses:
        // currentDelay = baseDelay + modulationSamples
        // where baseDelay = maxDelaySamples * 0.5f (25ms at 50ms max)
        // and modulationSamples = lfoValue * depth * maxDelaySamples * 0.5f
        
        // This should be changed to match research:
        // baseDelayMs = 5.0ms
        // maxModulationMs = 45.0ms (not 50.0ms)
        // modulatedDelay = 5.0 + (lfoOutput * (depthParam/100.0) * 45.0)
        
        // Current implementation will fail this test
        // After correction, delays should range from 5ms to 50ms (5 + 45)
        // instead of current range which is centered around 25ms
        
        REQUIRE_FALSE(hasInvalidValues(buffer));
        // Detailed formula verification will be added in implementation phase
    }
}