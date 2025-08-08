#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <JuceHeader.h>
#include "audio_test_utils.h"
#include "../Source/PluginProcessor.h"
#include <vector>
#include <cmath>

using namespace TylerAudio::Testing;
using Catch::Approx;

TEST_CASE("TapeSaturation Drive Scaling Tests", "[TingeTape][unit][saturation][detailed]")
{
    SECTION("Drive parameter mapping test - 1x to 10x gain range")
    {
        TingeTapeAudioProcessor processor;
        processor.prepareToPlay(48000.0, 512);
        
        // Test drive parameter mapping: should map to 1x-10x gain (not current 1x-5x)
        std::vector<float> driveValues = {0.0f, 25.0f, 50.0f, 75.0f, 100.0f};
        std::vector<float> expectedGains = {1.0f, 3.25f, 5.5f, 7.75f, 10.0f}; // Linear 1x to 10x
        std::vector<float> measuredGains;
        
        for (size_t i = 0; i < driveValues.size(); ++i)
        {
            float drivePercent = driveValues[i];
            
            // Set drive parameter
            if (auto* driveParam = processor.getParameters().getParameter("drive"))
            {
                driveParam->setValue(drivePercent / 100.0f); // Convert to 0-1 range
            }
            
            // Generate low-level test signal to measure gain without saturation
            juce::AudioBuffer<float> buffer(1, 512);
            auto testSignal = generateTestTone(1000.0f, 0.1f, 48000.0, 512, 1); // Low level
            buffer.copyFrom(0, 0, testSignal, 0, 0, 512);
            
            float inputRMS = getRMSLevel(buffer, 0);
            
            juce::MidiBuffer midiBuffer;
            processor.processBlock(buffer, midiBuffer);
            
            float outputRMS = getRMSLevel(buffer, 0);
            float measuredGain = outputRMS / inputRMS;
            
            measuredGains.push_back(measuredGain);
            
            INFO("Drive: " << drivePercent << "%, Expected gain: " << expectedGains[i] << "x, Measured: " << measuredGain << "x");
            
            // Research requirement: Drive should map to 1x-10x gain (not current 1x-5x)
            // This test will fail with current implementation
            REQUIRE(measuredGain == Approx(expectedGains[i]).margin(0.2f));
        }
        
        // Verify linear progression
        for (size_t i = 1; i < measuredGains.size(); ++i)
        {
            REQUIRE(measuredGains[i] > measuredGains[i-1]); // Should increase
        }
    }
    
    SECTION("tanh normalization accuracy test")
    {
        TingeTapeAudioProcessor processor;
        processor.prepareToPlay(48000.0, 512);
        
        // Test that tanh normalization maintains unity gain at low levels
        std::vector<float> driveValues = {25.0f, 50.0f, 75.0f, 100.0f};
        
        for (float drive : driveValues)
        {
            if (auto* driveParam = processor.getParameters().getParameter("drive"))
            {
                driveParam->setValue(drive / 100.0f);
            }
            
            // Generate very low level signal (should not saturate significantly)
            juce::AudioBuffer<float> buffer(1, 512);
            auto testSignal = generateTestTone(1000.0f, 0.01f, 48000.0, 512, 1); // Very low level
            buffer.copyFrom(0, 0, testSignal, 0, 0, 512);
            
            float inputRMS = getRMSLevel(buffer, 0);
            
            juce::MidiBuffer midiBuffer;
            processor.processBlock(buffer, midiBuffer);
            
            float outputRMS = getRMSLevel(buffer, 0);
            float gain = outputRMS / inputRMS;
            
            INFO("Drive: " << drive << "%, Low-level gain: " << gain << "x");
            
            // Research requirement: Proper tanh normalization should maintain consistent gain
            // Formula: output = tanh(input * driveGain) / tanh(driveGain)
            // At very low levels, this should provide the expected linear gain
            
            // Calculate expected gain based on research formula
            float driveGain = 1.0f + (drive * 0.01f) * 9.0f; // 1x to 10x mapping
            float expectedGain = driveGain; // At low levels, tanh is approximately linear
            
            // This test will fail with current implementation (no proper normalization)
            REQUIRE(gain == Approx(expectedGain).margin(0.1f));
        }
    }
    
    SECTION("Harmonic content analysis test - odd harmonics dominance")
    {
        TingeTapeAudioProcessor processor;
        processor.prepareToPlay(48000.0, 4096); // Need longer buffer for frequency analysis
        
        // Set moderate drive level
        if (auto* driveParam = processor.getParameters().getParameter("drive"))
        {
            driveParam->setValue(0.6f); // 60% drive
        }
        
        // Generate pure sine wave for harmonic analysis
        const float testFreq = 440.0f; // A4
        juce::AudioBuffer<float> buffer(1, 4096);
        auto sineWave = generateTestTone(testFreq, 0.5f, 48000.0, 4096, 1);
        buffer.copyFrom(0, 0, sineWave, 0, 0, 4096);
        
        juce::MidiBuffer midiBuffer;
        processor.processBlock(buffer, midiBuffer);
        
        // Basic harmonic content validation
        auto* data = buffer.getReadPointer(0);
        
        // Check for increased harmonic content (more complex waveform)
        auto originalSine = generateTestTone(testFreq, 0.5f, 48000.0, 4096, 1);
        bool hasHarmonics = !buffersMatch(buffer, originalSine, 0.1f);
        
        REQUIRE(hasHarmonics); // Should add harmonics
        REQUIRE_FALSE(hasInvalidValues(buffer));
        
        // Verify no excessive clipping or distortion artifacts
        float maxValue = 0.0f;
        for (int i = 0; i < 4096; ++i)
        {
            maxValue = std::max(maxValue, std::abs(data[i]));
        }
        
        REQUIRE(maxValue < 1.5f); // Should not clip excessively
        
        // TODO: Add FFT analysis to verify odd harmonic dominance
        // Research requirement: tanh saturation should produce predominantly odd harmonics
        // at moderate drive levels, with even harmonics appearing at higher drive
    }
    
    SECTION("Level compensation validation test")
    {
        TingeTapeAudioProcessor processor;
        processor.prepareToPlay(48000.0, 512);
        
        // Test that level compensation maintains reasonable output levels
        std::vector<float> driveValues = {0.0f, 20.0f, 40.0f, 60.0f, 80.0f, 100.0f};
        std::vector<float> outputLevels;
        
        const float inputLevel = 0.5f; // Moderate input level
        
        for (float drive : driveValues)
        {
            if (auto* driveParam = processor.getParameters().getParameter("drive"))
            {
                driveParam->setValue(drive / 100.0f);
            }
            
            juce::AudioBuffer<float> buffer(1, 512);
            auto testSignal = generateTestTone(1000.0f, inputLevel, 48000.0, 512, 1);
            buffer.copyFrom(0, 0, testSignal, 0, 0, 512);
            
            juce::MidiBuffer midiBuffer;
            processor.processBlock(buffer, midiBuffer);
            
            float outputRMS = getRMSLevel(buffer, 0);
            outputLevels.push_back(outputRMS);
            
            INFO("Drive: " << drive << "%, Output RMS: " << outputRMS);
            
            // Should not have excessive gain at high drive settings
            REQUIRE(outputRMS < 2.0f * inputLevel); // Reasonable upper bound
            REQUIRE(outputRMS > 0.1f * inputLevel); // Should not disappear
        }
        
        // Research requirement: Level compensation should prevent excessive gain
        // at high drive settings. Current implementation may not be optimal.
        
        // Check that output doesn't grow uncontrollably with drive
        float maxOutput = *std::max_element(outputLevels.begin(), outputLevels.end());
        float minOutput = *std::min_element(outputLevels.begin(), outputLevels.end());
        float variation = maxOutput / minOutput;
        
        INFO("Output level variation: " << variation << "x");
        
        // Level compensation should keep variation within reasonable bounds
        REQUIRE(variation < 5.0f); // Output shouldn't vary by more than 5x across drive range
    }
    
    SECTION("High-frequency rolloff measurement test")
    {
        TingeTapeAudioProcessor processor;
        processor.prepareToPlay(48000.0, 512);
        
        // Test HF rolloff at different drive levels
        std::vector<float> driveValues = {0.0f, 50.0f, 100.0f};
        std::vector<float> testFrequencies = {1000.0f, 5000.0f, 10000.0f, 15000.0f};
        
        for (float drive : driveValues)
        {
            if (auto* driveParam = processor.getParameters().getParameter("drive"))
            {
                driveParam->setValue(drive / 100.0f);
            }
            
            std::vector<float> frequencyResponse;
            
            for (float freq : testFrequencies)
            {
                juce::AudioBuffer<float> buffer(1, 512);
                auto testSignal = generateTestTone(freq, 0.5f, 48000.0, 512, 1);
                buffer.copyFrom(0, 0, testSignal, 0, 0, 512);
                
                float inputRMS = getRMSLevel(buffer, 0);
                
                juce::MidiBuffer midiBuffer;
                processor.processBlock(buffer, midiBuffer);
                
                float outputRMS = getRMSLevel(buffer, 0);
                float response = outputRMS / inputRMS;
                frequencyResponse.push_back(response);
                
                INFO("Drive: " << drive << "%, Freq: " << freq << "Hz, Response: " << response);
            }
            
            // Research requirement: HF rolloff should increase with drive amount
            if (drive > 0.0f)
            {
                // High frequencies should be attenuated more than low frequencies
                REQUIRE(frequencyResponse[3] < frequencyResponse[0]); // 15kHz < 1kHz response
                
                // Higher drive should cause more HF rolloff
                // This will be verified by comparing responses at different drive levels
            }
            
            // All responses should be reasonable
            for (float resp : frequencyResponse)
            {
                REQUIRE(resp > 0.1f); // Should not completely eliminate frequencies
                REQUIRE(resp < 10.0f); // Should not have excessive gain
            }
        }
    }
    
    SECTION("Drive scaling formula verification")
    {
        // Test the exact formula from research:
        // driveGain = 1.0f + (driveParam * 0.01f) * 9.0f; // 1x to 10x
        // output = std::tanh(input * driveGain) / std::tanh(driveGain);
        
        TingeTapeAudioProcessor processor;
        processor.prepareToPlay(48000.0, 512);
        
        // Test specific drive values against expected formula
        std::vector<std::pair<float, float>> testCases = {
            {0.0f, 1.0f},   // 0% drive -> 1x gain
            {50.0f, 5.5f},  // 50% drive -> 5.5x gain
            {100.0f, 10.0f} // 100% drive -> 10x gain
        };
        
        for (auto& testCase : testCases)
        {
            float drivePercent = testCase.first;
            float expectedGain = testCase.second;
            
            if (auto* driveParam = processor.getParameters().getParameter("drive"))
            {
                driveParam->setValue(drivePercent / 100.0f);
            }
            
            // Use very low level signal to measure linear gain
            juce::AudioBuffer<float> buffer(1, 512);
            auto testSignal = generateTestTone(1000.0f, 0.05f, 48000.0, 512, 1);
            buffer.copyFrom(0, 0, testSignal, 0, 0, 512);
            
            float inputRMS = getRMSLevel(buffer, 0);
            
            juce::MidiBuffer midiBuffer;
            processor.processBlock(buffer, midiBuffer);
            
            float outputRMS = getRMSLevel(buffer, 0);
            float actualGain = outputRMS / inputRMS;
            
            INFO("Drive: " << drivePercent << "%, Expected: " << expectedGain << "x, Actual: " << actualGain << "x");
            
            // Current implementation uses 1x to 5x scaling, so this will fail
            // After correction, should match research formula exactly
            REQUIRE(actualGain == Approx(expectedGain).margin(0.1f));
        }
    }
    
    SECTION("tanh normalization mathematical verification")
    {
        // Verify that the tanh normalization formula works correctly
        // This is a mathematical test independent of the audio processing
        
        std::vector<float> driveGains = {1.0f, 2.0f, 5.0f, 10.0f};
        std::vector<float> testInputs = {0.01f, 0.1f, 0.2f, 0.5f}; // Various input levels
        
        for (float driveGain : driveGains)
        {
            for (float input : testInputs)
            {
                // Research formula: output = tanh(input * driveGain) / tanh(driveGain)
                float expectedOutput = std::tanh(input * driveGain) / std::tanh(driveGain);
                
                // At very low input levels, this should approximately equal input * normalizationFactor
                if (input < 0.1f) // Low level where tanh is approximately linear
                {
                    float normalizationFactor = 1.0f / std::tanh(driveGain) * driveGain;
                    float approximateOutput = input * normalizationFactor;
                    
                    INFO("DriveGain: " << driveGain << ", Input: " << input << ", Expected: " << expectedOutput << ", Approx: " << approximateOutput);
                    
                    // At low levels, normalized tanh should be approximately linear
                    REQUIRE(expectedOutput == Approx(approximateOutput).margin(0.05f));
                }
                
                // Output should be finite and reasonable
                REQUIRE(std::isfinite(expectedOutput));
                REQUIRE(std::abs(expectedOutput) < 2.0f * std::abs(input)); // Shouldn't amplify excessively
            }
        }
    }
}