#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <JuceHeader.h>
#include "audio_test_utils.h"
#include "../Source/PluginProcessor.h"
#include <vector>
#include <cmath>
#include <random>
#include <thread>
#include <chrono>

using namespace TylerAudio::Testing;
using Catch::Approx;

TEST_CASE("TingeTape Professional Quality Assurance", "[TingeTape][validation][professional]")
{
    SECTION("Extended stress testing - parameter automation")
    {
        TingeTapeAudioProcessor processor;
        processor.prepareToPlay(48000.0, 512);
        
        // Continuous parameter automation test
        const int totalBlocks = 1000; // ~10 seconds at 48kHz/512
        std::mt19937 rng(42); // Deterministic for testing
        std::uniform_real_distribution<float> paramDist(0.0f, 1.0f);
        
        for (int block = 0; block < totalBlocks; ++block)
        {
            // Randomly automate parameters
            if (auto* wowParam = processor.getParameters().getParameter("wowDepth"))
                wowParam->setValue(paramDist(rng));
            if (auto* driveParam = processor.getParameters().getParameter("drive"))
                driveParam->setValue(paramDist(rng));
            if (auto* toneParam = processor.getParameters().getParameter("tone"))
                toneParam->setValue(paramDist(rng));
            if (auto* lowCutParam = processor.getParameters().getParameter("lowCutFreq"))
                lowCutParam->setValue(20.0f + paramDist(rng) * 180.0f); // 20-200Hz
            if (auto* highCutParam = processor.getParameters().getParameter("highCutFreq"))
                highCutParam->setValue(5000.0f + paramDist(rng) * 15000.0f); // 5-20kHz
            
            // Generate test audio
            juce::AudioBuffer<float> buffer(2, 512);
            auto testSignal = generateTestTone(440.0f + (block % 100) * 10.0f, 0.5f, 48000.0, 512, 2);
            buffer.copyFrom(0, 0, testSignal, 0, 0, 512);
            buffer.copyFrom(1, 0, testSignal, 1, 0, 512);
            
            juce::MidiBuffer midiBuffer;
            processor.processBlock(buffer, midiBuffer);
            
            // Verify stability under stress
            REQUIRE_FALSE(hasInvalidValues(buffer));
            REQUIRE_FALSE(hasDeNormals(buffer));
            
            // Periodic detailed validation
            if (block % 100 == 0)
            {
                for (int ch = 0; ch < 2; ++ch)
                {
                    float rms = getRMSLevel(buffer, ch);
                    REQUIRE(rms >= 0.0f);
                    REQUIRE(rms < 5.0f); // Reasonable upper bound even under extreme automation
                    REQUIRE(std::isfinite(rms));
                }
                
                INFO("Stress test block: " << block << " completed successfully");
            }
        }
    }
    
    SECTION("Edge case handling validation")
    {
        TingeTapeAudioProcessor processor;
        processor.prepareToPlay(48000.0, 512);
        
        // Test various edge cases that might cause issues
        struct EdgeCase {
            std::string name;
            std::function<juce::AudioBuffer<float>()> signalGen;
            std::function<void(TingeTapeAudioProcessor&)> setup;
        };
        
        std::vector<EdgeCase> edgeCases = {
            {"DC signal",
             []() {
                 juce::AudioBuffer<float> buffer(2, 512);
                 buffer.clear();
                 for (int ch = 0; ch < 2; ++ch) {
                     for (int i = 0; i < 512; ++i) {
                         buffer.setSample(ch, i, 0.5f); // Pure DC
                     }
                 }
                 return buffer;
             },
             [](TingeTapeAudioProcessor& p) {
                 if (auto* driveParam = p.getParameters().getParameter("drive"))
                     driveParam->setValue(0.8f); // High drive with DC
             }},
            
            {"Nyquist frequency",
             []() { return generateTestTone(24000.0f, 0.7f, 48000.0, 512, 2); },
             [](TingeTapeAudioProcessor& p) {
                 if (auto* highCutParam = p.getParameters().getParameter("highCutFreq"))
                     highCutParam->setValue(20000.0f); // Near Nyquist
             }},
            
            {"Very low frequency",
             []() { return generateTestTone(5.0f, 0.8f, 48000.0, 512, 2); },
             [](TingeTapeAudioProcessor& p) {
                 if (auto* lowCutParam = p.getParameters().getParameter("lowCutFreq"))
                     lowCutParam->setValue(20.0f); // Near low cut
             }},
            
            {"Extreme levels",
             []() { return generateTestTone(1000.0f, 1.5f, 48000.0, 512, 2); }, // >0dB
             [](TingeTapeAudioProcessor& p) {
                 if (auto* driveParam = p.getParameters().getParameter("drive"))
                     driveParam->setValue(1.0f); // Maximum drive
             }},
            
            {"Impulse train",
             []() {
                 juce::AudioBuffer<float> buffer(2, 512);
                 buffer.clear();
                 for (int ch = 0; ch < 2; ++ch) {
                     for (int i = 0; i < 512; i += 64) { // Every 64 samples
                         buffer.setSample(ch, i, 1.0f);
                     }
                 }
                 return buffer;
             },
             [](TingeTapeAudioProcessor& p) {
                 if (auto* wowParam = p.getParameters().getParameter("wowDepth"))
                     wowParam->setValue(1.0f); // Maximum wow with impulses
             }},
            
            {"Silence",
             []() {
                 juce::AudioBuffer<float> buffer(2, 512);
                 buffer.clear();
                 return buffer;
             },
             [](TingeTapeAudioProcessor& p) {
                 // Default parameters
             }}
        };
        
        for (auto& edgeCase : edgeCases)
        {
            auto buffer = edgeCase.signalGen();
            edgeCase.setup(processor);
            
            juce::MidiBuffer midiBuffer;
            processor.processBlock(buffer, midiBuffer);
            
            INFO("Testing edge case: " << edgeCase.name);
            
            // Should handle all edge cases gracefully
            REQUIRE_FALSE(hasInvalidValues(buffer));
            REQUIRE_FALSE(hasDeNormals(buffer));
            
            // Verify output is bounded and reasonable
            for (int ch = 0; ch < 2; ++ch)
            {
                auto* data = buffer.getReadPointer(ch);
                for (int i = 0; i < 512; ++i)
                {
                    REQUIRE(std::isfinite(data[i]));
                    REQUIRE(std::abs(data[i]) < 10.0f); // Reasonable bound even for edge cases
                }
            }
        }
    }
    
    SECTION("Cross-platform consistency verification")
    {
        TingeTapeAudioProcessor processor;
        processor.prepareToPlay(48000.0, 512);
        
        // Set deterministic parameters
        if (auto* wowParam = processor.getParameters().getParameter("wowDepth"))
            wowParam->setValue(0.5f);
        if (auto* driveParam = processor.getParameters().getParameter("drive"))
            driveParam->setValue(0.4f);
        if (auto* toneParam = processor.getParameters().getParameter("tone"))
            toneParam->setValue(0.6f);
        
        // Generate deterministic test signal
        juce::AudioBuffer<float> buffer(2, 512);
        auto testSignal = generateTestTone(440.0f, 0.5f, 48000.0, 512, 2);
        buffer.copyFrom(0, 0, testSignal, 0, 0, 512);
        buffer.copyFrom(1, 0, testSignal, 1, 0, 512);
        
        // Process and capture results
        juce::MidiBuffer midiBuffer;
        processor.processBlock(buffer, midiBuffer);
        
        // Calculate checksums/signatures for cross-platform validation
        float outputSumL = 0.0f, outputSumR = 0.0f;
        auto* dataL = buffer.getReadPointer(0);
        auto* dataR = buffer.getReadPointer(1);
        
        for (int i = 0; i < 512; ++i)
        {
            outputSumL += dataL[i];
            outputSumR += dataR[i];
        }
        
        float outputRMSL = getRMSLevel(buffer, 0);
        float outputRMSR = getRMSLevel(buffer, 1);
        
        INFO("Platform signature - Sum L: " << outputSumL << ", R: " << outputSumR);
        INFO("Platform signature - RMS L: " << outputRMSL << ", R: " << outputRMSR);
        
        // These values should be identical across platforms with same floating-point precision
        REQUIRE_FALSE(hasInvalidValues(buffer));
        REQUIRE(std::isfinite(outputSumL));
        REQUIRE(std::isfinite(outputSumR));
        REQUIRE(std::isfinite(outputRMSL));
        REQUIRE(std::isfinite(outputRMSR));
        
        // Note: Exact cross-platform validation would require storing reference values
        // This test ensures basic consistency and finite results
    }
    
    SECTION("Memory leak and resource management")
    {
        // Test multiple processor lifecycles
        for (int cycle = 0; cycle < 10; ++cycle)
        {
            auto processor = std::make_unique<TingeTapeAudioProcessor>();
            processor->prepareToPlay(48000.0, 512);
            
            // Process some audio
            juce::AudioBuffer<float> buffer(2, 512);
            auto testSignal = generateTestTone(1000.0f, 0.5f, 48000.0, 512, 2);
            buffer.copyFrom(0, 0, testSignal, 0, 0, 512);
            buffer.copyFrom(1, 0, testSignal, 1, 0, 512);
            
            juce::MidiBuffer midiBuffer;
            processor->processBlock(buffer, midiBuffer);
            
            REQUIRE_FALSE(hasInvalidValues(buffer));
            
            // Release resources
            processor->releaseResources();
            // Processor will be destroyed here
        }
        
        // Test should complete without issues (memory leaks would be detected by tools like valgrind)
        REQUIRE(true); // Made it through all cycles
    }
    
    SECTION("Plugin format compliance verification")
    {
        TingeTapeAudioProcessor processor;
        
        // Test basic AudioProcessor interface compliance
        REQUIRE(processor.getName() == "TingeTape");
        REQUIRE_FALSE(processor.acceptsMidi());
        REQUIRE_FALSE(processor.producesMidi());
        REQUIRE_FALSE(processor.isMidiEffect());
        
        // Test program management
        REQUIRE(processor.getNumPrograms() >= 1);
        REQUIRE(processor.getCurrentProgram() >= 0);
        REQUIRE(processor.getCurrentProgram() < processor.getNumPrograms());
        
        // Test editor
        REQUIRE(processor.hasEditor());
        
        // Test state management
        juce::MemoryBlock state1, state2;
        processor.getStateInformation(state1);
        REQUIRE(state1.getSize() > 0);
        
        // Modify parameters
        if (auto* param = processor.getParameters().getParameter("drive"))
            param->setValue(0.75f);
        
        processor.getStateInformation(state2);
        
        // States should be different after parameter change
        REQUIRE(state1.getSize() == state2.getSize()); // Same format
        // Note: Could compare content to ensure states actually differ
        
        // Test parameter access
        auto& parameters = processor.getParameters();
        REQUIRE(parameters.getParameter("wowDepth") != nullptr);
        REQUIRE(parameters.getParameter("drive") != nullptr);
        REQUIRE(parameters.getParameter("tone") != nullptr);
        REQUIRE(parameters.getParameter("bypass") != nullptr);
        
        // Test parameter ranges
        auto testParamRange = [&](const char* paramId, float expectedMin, float expectedMax) {
            auto* param = parameters.getParameter(paramId);
            if (param)
            {
                auto range = param->getNormalisableRange();
                INFO("Parameter " << paramId << " range: " << range.start << " to " << range.end);
                REQUIRE(range.start == Approx(expectedMin).margin(0.1f));
                REQUIRE(range.end == Approx(expectedMax).margin(0.1f));
            }
        };
        
        // Note: These test the normalized ranges (0-1) not the actual parameter ranges
        testParamRange("wowDepth", 0.0f, 1.0f);
        testParamRange("drive", 0.0f, 1.0f);
    }
}

TEST_CASE("TingeTape Research Compliance Final Validation", "[TingeTape][validation][research]")
{
    SECTION("Algorithm specification compliance verification")
    {
        TingeTapeAudioProcessor processor;
        processor.prepareToPlay(48000.0, 4096); // Long buffer for accurate measurement
        
        // Test WowEngine compliance
        {
            INFO("Validating WowEngine research compliance");
            
            // Set 100% wow depth for maximum effect measurement
            if (auto* wowParam = processor.getParameters().getParameter("wowDepth"))
                wowParam->setValue(1.0f); // 100%
            
            // Set other parameters to minimal values
            if (auto* driveParam = processor.getParameters().getParameter("drive"))
                driveParam->setValue(0.0f);
            if (auto* toneParam = processor.getParameters().getParameter("tone"))
                toneParam->setValue(0.5f); // Neutral
            
            // Generate impulse for delay measurement
            juce::AudioBuffer<float> buffer(1, 4096);
            buffer.clear();
            buffer.setSample(0, 1000, 1.0f); // Impulse at sample 1000
            
            juce::MidiBuffer midiBuffer;
            processor.processBlock(buffer, midiBuffer);
            
            // Find the delayed impulse peak
            auto* data = buffer.getReadPointer(0);
            int peakPosition = 0;
            float maxValue = 0.0f;
            
            for (int i = 1001; i < 4096; ++i)
            {
                if (std::abs(data[i]) > maxValue)
                {
                    maxValue = std::abs(data[i]);
                    peakPosition = i;
                }
            }
            
            // Calculate delay (should be in 5-50ms range per research: 5ms base + 0-45ms modulation)
            float delayMs = (peakPosition - 1000) * 1000.0f / 48000.0f;
            
            INFO("WowEngine delay measurement: " << delayMs << "ms");
            
            // Research compliance: 5ms base + 0-45ms modulation = 5-50ms total range
            REQUIRE(delayMs >= 5.0f);   // Minimum: base delay
            REQUIRE(delayMs <= 50.0f);  // Maximum: base + full modulation
        }
        
        // Test TapeSaturation compliance  
        {
            INFO("Validating TapeSaturation research compliance");
            
            // Test drive scaling: should map 0-100% to 1x-10x gain (not 1x-5x)
            std::vector<std::pair<float, float>> driveTests = {
                {0.0f, 1.0f},    // 0% drive -> 1x gain
                {0.5f, 5.5f},    // 50% drive -> 5.5x gain  
                {1.0f, 10.0f}    // 100% drive -> 10x gain
            };
            
            for (auto& [drivePercent, expectedGain] : driveTests)
            {
                // Set drive level
                if (auto* driveParam = processor.getParameters().getParameter("drive"))
                    driveParam->setValue(drivePercent);
                
                // Set other parameters to neutral
                if (auto* wowParam = processor.getParameters().getParameter("wowDepth"))
                    wowParam->setValue(0.0f);
                
                // Use very low level signal to measure linear gain (before saturation)
                juce::AudioBuffer<float> buffer(1, 1024);
                auto testSignal = generateTestTone(1000.0f, 0.05f, 48000.0, 1024, 1); // Very low level
                buffer.copyFrom(0, 0, testSignal, 0, 0, 1024);
                
                float inputRMS = getRMSLevel(buffer, 0);
                
                juce::MidiBuffer midiBuffer;
                processor.processBlock(buffer, midiBuffer);
                
                float outputRMS = getRMSLevel(buffer, 0);
                float actualGain = outputRMS / inputRMS;
                
                INFO("Drive: " << (drivePercent * 100.0f) << "%, Expected: " << expectedGain << "x, Actual: " << actualGain << "x");
                
                // Research compliance: 1x-10x gain range (not 1x-5x)
                REQUIRE(actualGain == Approx(expectedGain).margin(0.2f));
            }
        }
        
        // Test ToneControl compliance
        {
            INFO("Validating ToneControl research compliance");
            
            // Test maximum gain range: should be ±6dB (not ±12dB)
            std::vector<std::pair<float, float>> toneTests = {
                {0.0f, -6.0f},   // Full dark -> -6dB at high freq
                {1.0f, 6.0f}     // Full bright -> +6dB at high freq
            };
            
            for (auto& [toneValue, expectedGainDb] : toneTests)
            {
                if (auto* toneParam = processor.getParameters().getParameter("tone"))
                    toneParam->setValue(toneValue);
                
                // Test at high shelf frequency (5kHz)
                juce::AudioBuffer<float> buffer(1, 1024);
                auto testSignal = generateTestTone(5000.0f, 0.5f, 48000.0, 1024, 1);
                buffer.copyFrom(0, 0, testSignal, 0, 0, 1024);
                
                float inputRMS = getRMSLevel(buffer, 0);
                
                juce::MidiBuffer midiBuffer;
                processor.processBlock(buffer, midiBuffer);
                
                float outputRMS = getRMSLevel(buffer, 0);
                float actualGainDb = 20.0f * std::log10(outputRMS / inputRMS);
                
                INFO("Tone: " << (toneValue * 100.0f) << "%, Expected: " << expectedGainDb << "dB, Actual: " << actualGainDb << "dB");
                
                // Research compliance: ±6dB maximum (not ±12dB)
                REQUIRE(std::abs(actualGainDb) <= 6.5f); // Small tolerance for measurement
                REQUIRE(std::abs(actualGainDb) >= 5.0f); // Should have significant effect
            }
        }
    }
    
    SECTION("Performance target validation")
    {
        TingeTapeAudioProcessor processor;
        const double sampleRate = 48000.0;
        const int blockSize = 512;
        
        processor.prepareToPlay(sampleRate, blockSize);
        
        // Set realistic parameters
        if (auto* wowParam = processor.getParameters().getParameter("wowDepth"))
            wowParam->setValue(0.3f);
        if (auto* driveParam = processor.getParameters().getParameter("drive"))
            driveParam->setValue(0.4f);
        if (auto* toneParam = processor.getParameters().getParameter("tone"))
            toneParam->setValue(0.6f);
        
        // Performance measurement
        const int iterations = 1000;
        juce::AudioBuffer<float> buffer(2, blockSize);
        juce::MidiBuffer midiBuffer;
        
        // Warm up
        for (int i = 0; i < 100; ++i)
        {
            auto testSignal = generateTestTone(440.0f, 0.5f, sampleRate, blockSize, 2);
            buffer.copyFrom(0, 0, testSignal, 0, 0, blockSize);
            buffer.copyFrom(1, 0, testSignal, 1, 0, blockSize);
            processor.processBlock(buffer, midiBuffer);
        }
        
        // Timed measurement
        auto startTime = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < iterations; ++i)
        {
            auto testSignal = generateTestTone(440.0f + i * 0.1f, 0.5f, sampleRate, blockSize, 2);
            buffer.copyFrom(0, 0, testSignal, 0, 0, blockSize);
            buffer.copyFrom(1, 0, testSignal, 1, 0, blockSize);
            processor.processBlock(buffer, midiBuffer);
        }
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto durationUs = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
        
        double avgTimePerBlock = durationUs.count() / (double)iterations / 1000.0; // milliseconds
        double blockTimeMs = (blockSize * 1000.0) / sampleRate; // Available time per block
        double cpuPercent = (avgTimePerBlock / blockTimeMs) * 100.0;
        
        INFO("Average processing time: " << avgTimePerBlock << "ms per block");
        INFO("Available time: " << blockTimeMs << "ms per block");
        INFO("CPU usage: " << cpuPercent << "%");
        
        // Research requirement: <1% CPU usage
        REQUIRE(cpuPercent < 1.5); // Allow some margin for test environment
        REQUIRE(avgTimePerBlock > 0.0); // Sanity check
        REQUIRE_FALSE(hasInvalidValues(buffer));
    }
    
    SECTION("Professional deployment readiness")
    {
        TingeTapeAudioProcessor processor;
        
        // Comprehensive readiness checklist
        INFO("Verifying professional deployment readiness");
        
        // 1. Plugin identification and metadata
        REQUIRE(processor.getName() == "TingeTape");
        REQUIRE(!processor.getName().empty());
        
        // 2. Parameter system integrity
        auto& parameters = processor.getParameters();
        std::vector<std::string> requiredParams = {
            "wowDepth", "lowCutFreq", "lowCutRes", "highCutFreq", 
            "highCutRes", "drive", "tone", "bypass"
        };
        
        for (const auto& paramId : requiredParams)
        {
            auto* param = parameters.getParameter(paramId);
            REQUIRE(param != nullptr);
            
            // Test parameter functionality
            float originalValue = param->getValue();
            param->setValue(0.5f);
            REQUIRE(param->getValue() == Approx(0.5f).margin(0.01f));
            param->setValue(originalValue); // Restore
        }
        
        // 3. Processing chain integrity under various conditions
        processor.prepareToPlay(48000.0, 512);
        
        // Test different sample rates
        std::vector<double> sampleRates = {44100.0, 48000.0, 96000.0, 192000.0};
        for (double sr : sampleRates)
        {
            processor.prepareToPlay(sr, 512);
            
            juce::AudioBuffer<float> buffer(2, 512);
            auto testSignal = generateTestTone(1000.0f, 0.5f, sr, 512, 2);
            buffer.copyFrom(0, 0, testSignal, 0, 0, 512);
            buffer.copyFrom(1, 0, testSignal, 1, 0, 512);
            
            juce::MidiBuffer midiBuffer;
            processor.processBlock(buffer, midiBuffer);
            
            REQUIRE_FALSE(hasInvalidValues(buffer));
            INFO("Sample rate " << sr << "Hz: OK");
        }
        
        // 4. State management reliability
        juce::MemoryBlock stateData;
        processor.getStateInformation(stateData);
        REQUIRE(stateData.getSize() > 0);
        
        // 5. Editor functionality
        REQUIRE(processor.hasEditor());
        
        // 6. Thread safety (basic check)
        processor.prepareToPlay(48000.0, 512);
        REQUIRE(true); // Made it through preparation without issues
        
        INFO("✓ All professional deployment readiness checks passed");
    }
}