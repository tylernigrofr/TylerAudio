#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <JuceHeader.h>
#include "audio_test_utils.h"
#include "../Source/PluginProcessor.h"
#include <vector>
#include <cmath>

using namespace TylerAudio::Testing;
using Catch::Approx;

TEST_CASE("ToneControl Calibration Tests", "[TingeTape][unit][tone][detailed]")
{
    SECTION("Gain range validation test - ±6dB instead of ±12dB")
    {
        TingeTapeAudioProcessor processor;
        processor.prepareToPlay(48000.0, 512);
        
        // Test maximum gain range at key frequencies
        std::vector<float> toneValues = {-100.0f, 100.0f}; // Full negative and positive
        std::vector<float> testFrequencies = {250.0f, 5000.0f}; // Shelf frequencies
        
        for (float tonePercent : toneValues)
        {
            if (auto* toneParam = processor.getParameters().getParameter("tone"))
            {
                toneParam->setValue((tonePercent + 100.0f) / 200.0f); // Convert -100..100 to 0..1
            }
            
            for (float freq : testFrequencies)
            {
                juce::AudioBuffer<float> buffer(1, 512);
                auto testSignal = generateTestTone(freq, 0.5f, 48000.0, 512, 1);
                buffer.copyFrom(0, 0, testSignal, 0, 0, 512);
                
                float inputRMS = getRMSLevel(buffer, 0);
                
                juce::MidiBuffer midiBuffer;
                processor.processBlock(buffer, midiBuffer);
                
                float outputRMS = getRMSLevel(buffer, 0);
                float gainDb = 20.0f * std::log10(outputRMS / inputRMS);
                
                INFO("Tone: " << tonePercent << "%, Frequency: " << freq << "Hz, Gain: " << gainDb << "dB");
                
                // Research requirement: Maximum gain should be limited to ±6dB (currently ±12dB)
                if (freq == 250.0f && tonePercent < 0.0f) // Dark setting should boost lows
                {
                    // This test will fail with current implementation (±12dB range)
                    REQUIRE(std::abs(gainDb) <= 6.1f); // Small tolerance for measurement error
                    REQUIRE(gainDb > 3.0f); // Should have significant boost
                }
                else if (freq == 5000.0f && tonePercent > 0.0f) // Bright setting should boost highs
                {
                    REQUIRE(std::abs(gainDb) <= 6.1f); // Should not exceed ±6dB
                    REQUIRE(gainDb > 3.0f); // Should have significant boost
                }
                else if (freq == 250.0f && tonePercent > 0.0f) // Bright setting should cut lows
                {
                    REQUIRE(gainDb < -3.0f); // Should have significant cut
                    REQUIRE(gainDb >= -6.1f); // Should not exceed -6dB cut
                }
                else if (freq == 5000.0f && tonePercent < 0.0f) // Dark setting should cut highs
                {
                    REQUIRE(gainDb < -3.0f); // Should have significant cut
                    REQUIRE(gainDb >= -6.1f); // Should not exceed -6dB cut
                }
            }
        }
    }
    
    SECTION("Shelf frequency accuracy test - 250Hz low, 5kHz high")
    {
        TingeTapeAudioProcessor processor;
        processor.prepareToPlay(48000.0, 512);
        
        // Set moderate tone adjustment to measure shelf behavior
        if (auto* toneParam = processor.getParameters().getParameter("tone"))
        {
            toneParam->setValue(0.75f); // +50% tone (bright)
        }
        
        // Test frequency response around shelf frequencies
        std::vector<float> testFrequencies = {
            // Around low shelf (250Hz)
            125.0f, 175.0f, 250.0f, 350.0f, 500.0f,
            // Around high shelf (5kHz)
            2500.0f, 3500.0f, 5000.0f, 7000.0f, 10000.0f
        };
        
        std::vector<float> responses;
        
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
            responses.push_back(response);
            
            INFO("Frequency: " << freq << "Hz, Response: " << response << " (" << (20.0f * std::log10(response)) << "dB)");
        }
        
        // Verify shelf behavior
        // Low shelf at 250Hz: frequencies below should be less affected than frequencies above
        REQUIRE(responses[0] > responses[4]); // 125Hz should be less affected than 500Hz
        REQUIRE(responses[2] > responses[1]); // 250Hz should be transition point
        
        // High shelf at 5kHz: frequencies above should be more affected than frequencies below
        REQUIRE(responses[9] > responses[5]); // 10kHz should be more affected than 2.5kHz
        REQUIRE(responses[7] > responses[6]); // 5kHz should be transition point
        
        // Research requirement: Shelf frequencies should be exactly 250Hz and 5kHz
        // This is verified by the proper shelving behavior at these frequencies
    }
    
    SECTION("Tilt behavior validation test")
    {
        TingeTapeAudioProcessor processor;
        processor.prepareToPlay(48000.0, 512);
        
        std::vector<float> toneSettings = {-0.8f, -0.4f, 0.0f, 0.4f, 0.8f}; // -80% to +80%
        
        for (float toneSetting : toneSettings)
        {
            if (auto* toneParam = processor.getParameters().getParameter("tone"))
            {
                toneParam->setValue((toneSetting + 1.0f) * 0.5f); // Convert -1..1 to 0..1
            }
            
            // Test at shelf frequencies
            juce::AudioBuffer<float> lowBuffer(1, 512);
            juce::AudioBuffer<float> highBuffer(1, 512);
            
            auto lowSignal = generateTestTone(250.0f, 0.5f, 48000.0, 512, 1);
            auto highSignal = generateTestTone(5000.0f, 0.5f, 48000.0, 512, 1);
            
            lowBuffer.copyFrom(0, 0, lowSignal, 0, 0, 512);
            highBuffer.copyFrom(0, 0, highSignal, 0, 0, 512);
            
            float lowInputRMS = getRMSLevel(lowBuffer, 0);
            float highInputRMS = getRMSLevel(highBuffer, 0);
            
            juce::MidiBuffer midiBuffer;
            processor.processBlock(lowBuffer, midiBuffer);
            processor.processBlock(highBuffer, midiBuffer);
            
            float lowOutputRMS = getRMSLevel(lowBuffer, 0);
            float highOutputRMS = getRMSLevel(highBuffer, 0);
            
            float lowGainDb = 20.0f * std::log10(lowOutputRMS / lowInputRMS);
            float highGainDb = 20.0f * std::log10(highOutputRMS / highInputRMS);
            
            INFO("Tone: " << (toneSetting * 100.0f) << "%, Low gain: " << lowGainDb << "dB, High gain: " << highGainDb << "dB");
            
            // Research requirement: Tilt filter behavior with complementary shelf responses
            if (toneSetting < -0.1f) // Dark setting
            {
                REQUIRE(lowGainDb > highGainDb); // Lows should be boosted relative to highs
                REQUIRE(lowGainDb > 0.0f); // Low shelf should boost
                REQUIRE(highGainDb < 0.0f); // High shelf should cut
            }
            else if (toneSetting > 0.1f) // Bright setting
            {
                REQUIRE(highGainDb > lowGainDb); // Highs should be boosted relative to lows
                REQUIRE(highGainDb > 0.0f); // High shelf should boost
                REQUIRE(lowGainDb < 0.0f); // Low shelf should cut
            }
            else // Near zero
            {
                REQUIRE(std::abs(lowGainDb) < 1.0f); // Should be nearly flat
                REQUIRE(std::abs(highGainDb) < 1.0f);
            }
        }
    }
    
    SECTION("Bypass behavior test - near-zero tone values")
    {
        TingeTapeAudioProcessor processor;
        processor.prepareToPlay(48000.0, 512);
        
        // Test bypass behavior when tone is near zero
        std::vector<float> nearZeroValues = {-0.002f, -0.001f, 0.0f, 0.001f, 0.002f};
        
        for (float toneValue : nearZeroValues)
        {
            if (auto* toneParam = processor.getParameters().getParameter("tone"))
            {
                toneParam->setValue((toneValue + 1.0f) * 0.5f); // Convert to 0..1 range
            }
            
            // Generate test signal with multiple frequencies
            juce::AudioBuffer<float> buffer(1, 2048);
            buffer.clear();
            
            // Add multiple frequency components
            auto lowFreq = generateTestTone(250.0f, 0.2f, 48000.0, 2048, 1);
            auto midFreq = generateTestTone(1000.0f, 0.2f, 48000.0, 2048, 1);
            auto highFreq = generateTestTone(5000.0f, 0.2f, 48000.0, 2048, 1);
            
            for (int i = 0; i < 2048; ++i)
            {
                buffer.setSample(0, i, lowFreq.getSample(0, i) + midFreq.getSample(0, i) + highFreq.getSample(0, i));
            }
            
            auto referenceBuffer = buffer;
            float inputRMS = getRMSLevel(buffer, 0);
            
            juce::MidiBuffer midiBuffer;
            processor.processBlock(buffer, midiBuffer);
            
            float outputRMS = getRMSLevel(buffer, 0);
            float difference = std::abs(outputRMS - inputRMS);
            float relativeChange = difference / inputRMS;
            
            INFO("Tone: " << toneValue << ", Relative change: " << (relativeChange * 100.0f) << "%");
            
            // Research requirement: Bypass should engage when tone parameter near zero
            REQUIRE(relativeChange < 0.05f); // Less than 5% change when bypassed
            
            // Should be very close to original signal
            REQUIRE_FALSE(hasInvalidValues(buffer));
        }
    }
    
    SECTION("Frequency response linearity test")
    {
        TingeTapeAudioProcessor processor;
        processor.prepareToPlay(48000.0, 512);
        
        // Test that tone parameter provides linear control over frequency response
        std::vector<float> toneValues = {-1.0f, -0.5f, 0.0f, 0.5f, 1.0f};
        std::vector<float> lowFreqGains, highFreqGains;
        
        for (float tone : toneValues)
        {
            if (auto* toneParam = processor.getParameters().getParameter("tone"))
            {
                toneParam->setValue((tone + 1.0f) * 0.5f); // Convert -1..1 to 0..1
            }
            
            // Test at shelf frequencies
            juce::AudioBuffer<float> lowBuffer(1, 512);
            juce::AudioBuffer<float> highBuffer(1, 512);
            
            auto lowSignal = generateTestTone(250.0f, 0.5f, 48000.0, 512, 1);
            auto highSignal = generateTestTone(5000.0f, 0.5f, 48000.0, 512, 1);
            
            lowBuffer.copyFrom(0, 0, lowSignal, 0, 0, 512);
            highBuffer.copyFrom(0, 0, highSignal, 0, 0, 512);
            
            float lowInputRMS = getRMSLevel(lowBuffer, 0);
            float highInputRMS = getRMSLevel(highBuffer, 0);
            
            juce::MidiBuffer midiBuffer;
            processor.processBlock(lowBuffer, midiBuffer);
            processor.processBlock(highBuffer, midiBuffer);
            
            float lowOutputRMS = getRMSLevel(lowBuffer, 0);
            float highOutputRMS = getRMSLevel(highBuffer, 0);
            
            float lowGainDb = 20.0f * std::log10(lowOutputRMS / lowInputRMS);
            float highGainDb = 20.0f * std::log10(highOutputRMS / highInputRMS);
            
            lowFreqGains.push_back(lowGainDb);
            highFreqGains.push_back(highGainDb);
            
            INFO("Tone: " << tone << ", Low: " << lowGainDb << "dB, High: " << highGainDb << "dB");
        }
        
        // Verify monotonic relationship (linear control)
        for (size_t i = 1; i < toneValues.size(); ++i)
        {
            // As tone increases, low frequency gain should decrease, high frequency gain should increase
            REQUIRE(lowFreqGains[i] <= lowFreqGains[i-1]); // Low gains should decrease or stay same
            REQUIRE(highFreqGains[i] >= highFreqGains[i-1]); // High gains should increase or stay same
        }
    }
    
    SECTION("Gain calibration test - exact ±6dB measurement")
    {
        TingeTapeAudioProcessor processor;
        processor.prepareToPlay(48000.0, 512);
        
        // Test exact maximum gains at shelf frequencies
        struct TestCase {
            float tonePercent;
            float testFreq;
            float expectedMinGain;
            float expectedMaxGain;
            std::string description;
        };
        
        std::vector<TestCase> testCases = {
            {-100.0f, 250.0f, 5.5f, 6.5f, "Full dark - low shelf boost"},
            {-100.0f, 5000.0f, -6.5f, -5.5f, "Full dark - high shelf cut"},
            {100.0f, 250.0f, -6.5f, -5.5f, "Full bright - low shelf cut"},
            {100.0f, 5000.0f, 5.5f, 6.5f, "Full bright - high shelf boost"}
        };
        
        for (const auto& testCase : testCases)
        {
            if (auto* toneParam = processor.getParameters().getParameter("tone"))
            {
                toneParam->setValue((testCase.tonePercent + 100.0f) / 200.0f);
            }
            
            juce::AudioBuffer<float> buffer(1, 512);
            auto testSignal = generateTestTone(testCase.testFreq, 0.5f, 48000.0, 512, 1);
            buffer.copyFrom(0, 0, testSignal, 0, 0, 512);
            
            float inputRMS = getRMSLevel(buffer, 0);
            
            juce::MidiBuffer midiBuffer;
            processor.processBlock(buffer, midiBuffer);
            
            float outputRMS = getRMSLevel(buffer, 0);
            float gainDb = 20.0f * std::log10(outputRMS / inputRMS);
            
            INFO(testCase.description << ": " << gainDb << "dB (expected: " << testCase.expectedMinGain << " to " << testCase.expectedMaxGain << "dB)");
            
            // Research requirement: Gain should be exactly ±6dB (not ±12dB)
            // This test will fail with current implementation using ±12dB range
            REQUIRE(gainDb >= testCase.expectedMinGain);
            REQUIRE(gainDb <= testCase.expectedMaxGain);
        }
    }
}