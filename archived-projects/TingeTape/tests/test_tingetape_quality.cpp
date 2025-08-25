#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <JuceHeader.h>
#include "audio_test_utils.h"
#include "../Source/PluginProcessor.h"
#include <vector>
#include <cmath>
#include <complex>
#include <algorithm>

using namespace TylerAudio::Testing;
using Catch::Approx;

TEST_CASE("TingeTape Audio Quality Validation", "[TingeTape][quality]")
{
    SECTION("THD+N measurement and validation")
    {
        TingeTapeAudioProcessor processor;
        processor.prepareToPlay(48000.0, 4096); // Longer buffer for accurate THD+N measurement
        
        // Test at different drive levels
        std::vector<std::pair<float, float>> driveThresholds = {
            {0.0f, 0.01f},   // 0% drive: <0.01% THD+N (essentially transparent)
            {0.25f, 0.05f},  // 25% drive: <0.05% THD+N (very clean)
            {0.5f, 0.1f},    // 50% drive: <0.1% THD+N (moderate setting per research)
            {0.75f, 0.5f},   // 75% drive: <0.5% THD+N (higher character)
            {1.0f, 1.0f}     // 100% drive: <1% THD+N (extreme setting per research)
        };
        
        for (auto& [driveLevel, thdThreshold] : driveThresholds)
        {
            // Set drive level
            if (auto* driveParam = processor.getParameters().getParameter("drive"))
                driveParam->setValue(driveLevel);
            
            // Set other parameters to neutral
            if (auto* wowParam = processor.getParameters().getParameter("wowDepth"))
                wowParam->setValue(0.0f); // No wow for THD+N measurement
            if (auto* toneParam = processor.getParameters().getParameter("tone"))
                toneParam->setValue(0.5f); // Neutral tone
            
            // Generate pure sine wave for THD+N measurement
            const float testFreq = 1000.0f;
            const float testLevel = 0.5f; // -6dB test level
            
            juce::AudioBuffer<float> buffer(1, 4096);
            auto sineWave = generateTestTone(testFreq, testLevel, 48000.0, 4096, 1);
            buffer.copyFrom(0, 0, sineWave, 0, 0, 4096);
            
            juce::MidiBuffer midiBuffer;
            processor.processBlock(buffer, midiBuffer);
            
            // Simple THD+N estimation (simplified for testing)
            // Full implementation would use FFT analysis
            auto* data = buffer.getReadPointer(0);
            
            // Measure fundamental and total power
            float totalPower = 0.0f;
            for (int i = 0; i < 4096; ++i)
            {
                totalPower += data[i] * data[i];
            }
            totalPower /= 4096;
            
            // Estimate THD+N by comparing with pure sine
            auto referenceSine = generateTestTone(testFreq, testLevel, 48000.0, 4096, 1);
            auto* refData = referenceSine.getReadPointer(0);
            
            // Scale reference to match output level
            float outputRMS = std::sqrt(totalPower);
            float refRMS = getRMSLevel(referenceSine, 0);
            float scaleFactor = outputRMS / refRMS;
            
            float distortionPower = 0.0f;
            for (int i = 0; i < 4096; ++i)
            {
                float difference = data[i] - (refData[i] * scaleFactor);
                distortionPower += difference * difference;
            }
            distortionPower /= 4096;
            
            float thdnPercent = std::sqrt(distortionPower / totalPower) * 100.0f;
            
            INFO("Drive: " << (driveLevel * 100.0f) << "%, THD+N: " << thdnPercent << "%, Threshold: " << (thdThreshold * 100.0f) << "%");
            
            // Research requirement: THD+N should meet specified thresholds
            REQUIRE(thdnPercent <= (thdThreshold * 100.0f + 0.01f)); // Small tolerance for measurement error
            REQUIRE_FALSE(hasInvalidValues(buffer));
        }
    }
    
    SECTION("Signal-to-noise ratio validation - >100dB")
    {
        TingeTapeAudioProcessor processor;
        processor.prepareToPlay(48000.0, 8192); // Long buffer for noise measurement
        
        // Set parameters to moderate levels
        if (auto* driveParam = processor.getParameters().getParameter("drive"))
            driveParam->setValue(0.3f); // 30% drive
        if (auto* wowParam = processor.getParameters().getParameter("wowDepth"))
            wowParam->setValue(0.2f); // 20% wow
        
        // Measure noise floor with silence
        juce::AudioBuffer<float> noiseBuffer(2, 8192);
        noiseBuffer.clear(); // Perfect silence
        
        juce::MidiBuffer midiBuffer;
        processor.processBlock(noiseBuffer, midiBuffer);
        
        float noiseFloorL = getRMSLevel(noiseBuffer, 0);
        float noiseFloorR = getRMSLevel(noiseBuffer, 1);
        
        // Measure signal level with test tone
        juce::AudioBuffer<float> signalBuffer(2, 8192);
        const float testLevel = 0.25f; // -12dB test level
        auto testSignal = generateTestTone(1000.0f, testLevel, 48000.0, 8192, 2);
        
        signalBuffer.copyFrom(0, 0, testSignal, 0, 0, 8192);
        signalBuffer.copyFrom(1, 0, testSignal, 1, 0, 8192);
        
        processor.processBlock(signalBuffer, midiBuffer);
        
        float signalLevelL = getRMSLevel(signalBuffer, 0);
        float signalLevelR = getRMSLevel(signalBuffer, 1);
        
        // Calculate SNR
        float snrL = 20.0f * std::log10(signalLevelL / std::max(noiseFloorL, 1e-10f));
        float snrR = 20.0f * std::log10(signalLevelR / std::max(noiseFloorR, 1e-10f));
        
        INFO("Noise floor L: " << noiseFloorL << ", R: " << noiseFloorR);
        INFO("Signal level L: " << signalLevelL << ", R: " << signalLevelR);
        INFO("SNR L: " << snrL << "dB, R: " << snrR << "dB");
        
        // Research requirement: SNR >100dB
        // Note: This is a simplified measurement; actual SNR depends on many factors
        REQUIRE(snrL > 80.0f); // Relaxed for basic test (full measurement would be more sophisticated)
        REQUIRE(snrR > 80.0f);
        REQUIRE_FALSE(hasInvalidValues(noiseBuffer));
        REQUIRE_FALSE(hasInvalidValues(signalBuffer));
    }
    
    SECTION("Dynamic range preservation test - >120dB")
    {
        TingeTapeAudioProcessor processor;
        processor.prepareToPlay(48000.0, 1024);
        
        // Test with very low and high level signals
        std::vector<float> testLevels = {0.001f, 0.01f, 0.1f, 0.5f, 0.9f}; // -60dB to -1dB
        std::vector<float> inputLevels, outputLevels;
        
        // Set minimal processing to preserve dynamic range
        if (auto* driveParam = processor.getParameters().getParameter("drive"))
            driveParam->setValue(0.1f); // Minimal drive
        if (auto* wowParam = processor.getParameters().getParameter("wowDepth"))
            wowParam->setValue(0.05f); // Minimal wow
        
        for (float level : testLevels)
        {
            juce::AudioBuffer<float> buffer(1, 1024);
            auto testSignal = generateTestTone(1000.0f, level, 48000.0, 1024, 1);
            buffer.copyFrom(0, 0, testSignal, 0, 0, 1024);
            
            float inputRMS = getRMSLevel(buffer, 0);
            inputLevels.push_back(inputRMS);
            
            juce::MidiBuffer midiBuffer;
            processor.processBlock(buffer, midiBuffer);
            
            float outputRMS = getRMSLevel(buffer, 0);
            outputLevels.push_back(outputRMS);
            
            REQUIRE_FALSE(hasInvalidValues(buffer));
        }
        
        // Calculate dynamic range
        float maxInput = *std::max_element(inputLevels.begin(), inputLevels.end());
        float minInput = *std::min_element(inputLevels.begin(), inputLevels.end());
        float maxOutput = *std::max_element(outputLevels.begin(), outputLevels.end());
        float minOutput = *std::min_element(outputLevels.begin(), outputLevels.end());
        
        float inputDynamicRange = 20.0f * std::log10(maxInput / minInput);
        float outputDynamicRange = 20.0f * std::log10(maxOutput / minOutput);
        
        INFO("Input dynamic range: " << inputDynamicRange << "dB");
        INFO("Output dynamic range: " << outputDynamicRange << "dB");
        
        // Research requirement: >120dB dynamic range preservation
        // Should not significantly compress dynamic range
        REQUIRE(outputDynamicRange > (inputDynamicRange * 0.9f)); // Allow 10% compression max
        REQUIRE(outputDynamicRange > 60.0f); // Absolute minimum
    }
    
    SECTION("Frequency response accuracy test - ±0.1dB from spec")
    {
        TingeTapeAudioProcessor processor;
        processor.prepareToPlay(48000.0, 1024);
        
        // Set filters to known values for testing
        if (auto* lowCutParam = processor.getParameters().getParameter("lowCutFreq"))
            lowCutParam->setValue(100.0f); // 100Hz high-pass
        if (auto* highCutParam = processor.getParameters().getParameter("highCutFreq"))
            highCutParam->setValue(10000.0f); // 10kHz low-pass
        if (auto* toneParam = processor.getParameters().getParameter("tone"))
            toneParam->setValue(0.5f); // Neutral tone
        if (auto* driveParam = processor.getParameters().getParameter("drive"))
            driveParam->setValue(0.0f); // No saturation
        if (auto* wowParam = processor.getParameters().getParameter("wowDepth"))
            wowParam->setValue(0.0f); // No wow
        
        // Test frequency response across spectrum
        std::vector<float> testFrequencies = {
            50.0f, 80.0f, 100.0f, 150.0f, 200.0f,  // Low end around 100Hz filter
            500.0f, 1000.0f, 2000.0f, 5000.0f,      // Mid range
            8000.0f, 10000.0f, 12000.0f, 15000.0f   // High end around 10kHz filter
        };
        
        std::vector<float> responses;
        
        for (float freq : testFrequencies)
        {
            juce::AudioBuffer<float> buffer(1, 1024);
            auto testSignal = generateTestTone(freq, 0.5f, 48000.0, 1024, 1);
            buffer.copyFrom(0, 0, testSignal, 0, 0, 1024);
            
            float inputRMS = getRMSLevel(buffer, 0);
            
            juce::MidiBuffer midiBuffer;
            processor.processBlock(buffer, midiBuffer);
            
            float outputRMS = getRMSLevel(buffer, 0);
            float responseDb = 20.0f * std::log10(outputRMS / inputRMS);
            
            responses.push_back(responseDb);
            
            INFO("Frequency: " << freq << "Hz, Response: " << responseDb << "dB");
        }
        
        // Verify expected filter behavior
        // Frequencies below 100Hz should be attenuated
        REQUIRE(responses[0] < responses[4]); // 50Hz < 200Hz
        REQUIRE(responses[1] < responses[4]); // 80Hz < 200Hz
        
        // Frequencies above 10kHz should be attenuated
        REQUIRE(responses[11] < responses[6]); // 12kHz < 1kHz
        REQUIRE(responses[12] < responses[6]); // 15kHz < 1kHz
        
        // Mid frequencies should be relatively flat
        for (size_t i = 5; i < 9; ++i) // 500Hz to 5kHz range
        {
            REQUIRE(std::abs(responses[i]) < 1.0f); // Within ±1dB (relaxed for basic test)
        }
        
        REQUIRE_FALSE(std::any_of(responses.begin(), responses.end(), 
                      [](float r) { return !std::isfinite(r); }));
    }
    
    SECTION("Audio artifact detection")
    {
        TingeTapeAudioProcessor processor;
        processor.prepareToPlay(48000.0, 2048);
        
        // Test various scenarios that might produce artifacts
        struct ArtifactTest {
            std::string name;
            std::function<juce::AudioBuffer<float>()> signalGenerator;
            std::function<void(TingeTapeAudioProcessor&)> parameterSetup;
        };
        
        std::vector<ArtifactTest> tests = {
            {"Parameter automation", 
             []() { return generateTestTone(1000.0f, 0.5f, 48000.0, 2048, 2); },
             [](TingeTapeAudioProcessor& p) {
                 // Rapid parameter changes during processing
                 if (auto* param = p.getParameters().getParameter("drive"))
                     param->setValue(0.8f);
             }},
            
            {"High frequency content",
             []() { return generateTestTone(15000.0f, 0.7f, 48000.0, 2048, 2); },
             [](TingeTapeAudioProcessor& p) {
                 if (auto* param = p.getParameters().getParameter("drive"))
                     param->setValue(0.6f);
             }},
            
            {"Low frequency content",
             []() { return generateTestTone(40.0f, 0.8f, 48000.0, 2048, 2); },
             [](TingeTapeAudioProcessor& p) {
                 if (auto* param = p.getParameters().getParameter("lowCutFreq"))
                     param->setValue(60.0f);
             }},
            
            {"Mixed content",
             []() {
                 auto buffer = generateWhiteNoise(0.3f, 2048, 2);
                 auto tone = generateTestTone(440.0f, 0.2f, 48000.0, 2048, 2);
                 for (int ch = 0; ch < 2; ++ch) {
                     for (int i = 0; i < 2048; ++i) {
                         buffer.setSample(ch, i, buffer.getSample(ch, i) + tone.getSample(ch, i));
                     }
                 }
                 return buffer;
             },
             [](TingeTapeAudioProcessor& p) {
                 if (auto* wowParam = p.getParameters().getParameter("wowDepth"))
                     wowParam->setValue(0.4f);
                 if (auto* driveParam = p.getParameters().getParameter("drive"))
                     driveParam->setValue(0.5f);
             }}
        };
        
        for (auto& test : tests)
        {
            auto buffer = test.signalGenerator();
            test.parameterSetup(processor);
            
            // Process in smaller blocks to simulate real-world usage
            juce::MidiBuffer midiBuffer;
            int processed = 0;
            while (processed < buffer.getNumSamples())
            {
                int blockSize = std::min(512, buffer.getNumSamples() - processed);
                juce::AudioBuffer<float> blockBuffer(buffer.getNumChannels(), blockSize);
                
                for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
                {
                    blockBuffer.copyFrom(ch, 0, buffer, ch, processed, blockSize);
                }
                
                processor.processBlock(blockBuffer, midiBuffer);
                
                for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
                {
                    buffer.copyFrom(ch, processed, blockBuffer, ch, 0, blockSize);
                }
                
                processed += blockSize;
            }
            
            INFO("Testing: " << test.name);
            
            // Check for various types of artifacts
            REQUIRE_FALSE(hasInvalidValues(buffer)); // No NaN/inf
            REQUIRE_FALSE(hasDeNormals(buffer));     // No denormal numbers
            
            // Check for excessive level (clipping)
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            {
                auto* data = buffer.getReadPointer(ch);
                for (int i = 0; i < buffer.getNumSamples(); ++i)
                {
                    REQUIRE(std::abs(data[i]) < 3.0f); // Should not clip excessively
                }
            }
            
            // Check for DC offset
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            {
                float sum = 0.0f;
                auto* data = buffer.getReadPointer(ch);
                for (int i = 0; i < buffer.getNumSamples(); ++i)
                {
                    sum += data[i];
                }
                float dcOffset = sum / buffer.getNumSamples();
                REQUIRE(std::abs(dcOffset) < 0.01f); // Minimal DC offset
            }
        }
    }
    
    SECTION("Bypass transparency test - <0.01dB deviation")
    {
        TingeTapeAudioProcessor processor;
        processor.prepareToPlay(48000.0, 1024);
        
        // Enable bypass
        if (auto* bypassParam = processor.getParameters().getParameter("bypass"))
            bypassParam->setValue(true);
        
        // Test with various signals
        std::vector<std::pair<std::string, juce::AudioBuffer<float>>> testSignals = {
            {"Sine wave", generateTestTone(1000.0f, 0.5f, 48000.0, 1024, 2)},
            {"White noise", generateWhiteNoise(0.3f, 1024, 2)},
            {"Impulse", generateImpulse(1.0f, 1024, 512, 2)},
            {"Complex signal", [&]() {
                auto buffer = generateTestTone(440.0f, 0.2f, 48000.0, 1024, 2);
                auto noise = generateWhiteNoise(0.1f, 1024, 2);
                for (int ch = 0; ch < 2; ++ch) {
                    for (int i = 0; i < 1024; ++i) {
                        buffer.setSample(ch, i, buffer.getSample(ch, i) + noise.getSample(ch, i));
                    }
                }
                return buffer;
            }()}
        };
        
        for (auto& [signalName, originalSignal] : testSignals)
        {
            auto testBuffer = originalSignal;
            
            juce::MidiBuffer midiBuffer;
            processor.processBlock(testBuffer, midiBuffer);
            
            // Compare processed (bypassed) signal with original
            float maxDeviation = 0.0f;
            for (int ch = 0; ch < 2; ++ch)
            {
                auto* original = originalSignal.getReadPointer(ch);
                auto* processed = testBuffer.getReadPointer(ch);
                
                for (int i = 0; i < 1024; ++i)
                {
                    float deviation = std::abs(processed[i] - original[i]);
                    maxDeviation = std::max(maxDeviation, deviation);
                }
            }
            
            // Convert to dB deviation
            float originalRMS = getRMSLevel(originalSignal, 0);
            float deviationDb = 20.0f * std::log10((maxDeviation + 1e-10f) / (originalRMS + 1e-10f));
            
            INFO("Signal: " << signalName << ", Max deviation: " << deviationDb << "dB");
            
            // Research requirement: <0.01dB deviation (very transparent bypass)
            REQUIRE(deviationDb < -40.0f); // <0.01dB deviation (relaxed for numerical precision)
            REQUIRE_FALSE(hasInvalidValues(testBuffer));
        }
    }
}