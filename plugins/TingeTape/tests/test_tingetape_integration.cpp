#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <JuceHeader.h>
#include "audio_test_utils.h"
#include "../Source/PluginProcessor.h"
#include <vector>
#include <cmath>
#include <random>

using namespace TylerAudio::Testing;
using Catch::Approx;

TEST_CASE("TingeTape Integration Tests", "[TingeTape][integration]")
{
    SECTION("Complete signal chain integration test")
    {
        TingeTapeAudioProcessor processor;
        const double sampleRate = 48000.0;
        const int blockSize = 512;
        
        processor.prepareToPlay(sampleRate, blockSize);
        
        // Set all parameters to moderate, realistic values
        if (auto* wowParam = processor.getParameters().getParameter("wowDepth"))
            wowParam->setValue(25.0f / 100.0f); // 25% wow
        if (auto* lowCutParam = processor.getParameters().getParameter("lowCutFreq"))
            lowCutParam->setValue(40.0f); // 40Hz low cut
        if (auto* lowCutQParam = processor.getParameters().getParameter("lowCutRes"))
            lowCutQParam->setValue(0.707f); // Butterworth Q
        if (auto* highCutParam = processor.getParameters().getParameter("highCutFreq"))
            highCutParam->setValue(15000.0f); // 15kHz high cut
        if (auto* highCutQParam = processor.getParameters().getParameter("highCutRes"))
            highCutQParam->setValue(0.707f); // Butterworth Q
        if (auto* driveParam = processor.getParameters().getParameter("drive"))
            driveParam->setValue(40.0f / 100.0f); // 40% drive
        if (auto* toneParam = processor.getParameters().getParameter("tone"))
            toneParam->setValue(0.6f); // Slightly bright
        
        // Test with complex audio signal (multiple frequencies)
        juce::AudioBuffer<float> buffer(2, static_cast<int>(sampleRate * 2.0)); // 2 seconds, stereo
        buffer.clear();
        
        // Create complex test signal with multiple frequency components
        auto lowFreq = generateTestTone(80.0f, 0.15f, sampleRate, buffer.getNumSamples(), 2);
        auto midFreq = generateTestTone(1000.0f, 0.2f, sampleRate, buffer.getNumSamples(), 2);
        auto highFreq = generateTestTone(8000.0f, 0.15f, sampleRate, buffer.getNumSamples(), 2);
        auto noise = generateWhiteNoise(0.05f, buffer.getNumSamples(), 2, 12345);
        
        // Mix all components
        for (int ch = 0; ch < 2; ++ch)
        {
            for (int i = 0; i < buffer.getNumSamples(); ++i)
            {
                float sample = lowFreq.getSample(ch, i) + 
                              midFreq.getSample(ch, i) + 
                              highFreq.getSample(ch, i) + 
                              noise.getSample(ch, i);
                buffer.setSample(ch, i, sample);
            }
        }
        
        float inputRMSL = getRMSLevel(buffer, 0);
        float inputRMSR = getRMSLevel(buffer, 1);
        
        // Process through complete signal chain in blocks
        juce::MidiBuffer midiBuffer;
        int samplesProcessed = 0;
        const int processingBlockSize = 512;
        
        while (samplesProcessed < buffer.getNumSamples())
        {
            int samplesToProcess = std::min(processingBlockSize, buffer.getNumSamples() - samplesProcessed);
            juce::AudioBuffer<float> blockBuffer(2, samplesToProcess);
            
            for (int ch = 0; ch < 2; ++ch)
            {
                blockBuffer.copyFrom(ch, 0, buffer, ch, samplesProcessed, samplesToProcess);
            }
            
            processor.processBlock(blockBuffer, midiBuffer);
            
            for (int ch = 0; ch < 2; ++ch)
            {
                buffer.copyFrom(ch, samplesProcessed, blockBuffer, ch, 0, samplesToProcess);
            }
            
            samplesProcessed += samplesToProcess;
        }
        
        float outputRMSL = getRMSLevel(buffer, 0);
        float outputRMSR = getRMSLevel(buffer, 1);
        
        INFO("Input RMS L: " << inputRMSL << ", R: " << inputRMSR);
        INFO("Output RMS L: " << outputRMSL << ", R: " << outputRMSR);
        
        // Validate complete signal chain processing
        REQUIRE_FALSE(hasInvalidValues(buffer));
        REQUIRE_FALSE(hasDeNormals(buffer));
        
        // Output should be modified but reasonable
        REQUIRE(outputRMSL > 0.01f); // Not silent
        REQUIRE(outputRMSR > 0.01f);
        REQUIRE(outputRMSL < 2.0f * inputRMSL); // Not excessive gain
        REQUIRE(outputRMSR < 2.0f * inputRMSR);
        
        // Stereo processing should be consistent
        float stereoBalance = std::abs(outputRMSL - outputRMSR) / std::max(outputRMSL, outputRMSR);
        REQUIRE(stereoBalance < 0.1f); // <10% difference between channels
        
        // Signal should show tape character (different from input)
        auto originalBuffer = buffer;
        bool hasTapeCharacter = !buffersMatch(buffer, lowFreq, 0.2f); // Should be significantly different
        REQUIRE(hasTapeCharacter);
    }
    
    SECTION("Parameter interaction validation")
    {
        TingeTapeAudioProcessor processor;
        processor.prepareToPlay(48000.0, 512);
        
        // Test parameter combinations that should work well together
        struct ParameterSet {
            std::string name;
            float wow, lowCut, highCut, drive, tone;
        };
        
        std::vector<ParameterSet> testSets = {
            {"Subtle warmth", 10.0f, 30.0f, 18000.0f, 20.0f, 10.0f},
            {"Vintage tape", 50.0f, 60.0f, 12000.0f, 60.0f, -20.0f},
            {"Modern polish", 5.0f, 20.0f, 20000.0f, 10.0f, 30.0f},
            {"Heavy character", 80.0f, 100.0f, 8000.0f, 80.0f, -50.0f},
            {"Clean reference", 0.0f, 20.0f, 20000.0f, 0.0f, 0.0f}
        };
        
        for (const auto& paramSet : testSets)
        {
            // Set parameters
            if (auto* param = processor.getParameters().getParameter("wowDepth"))
                param->setValue(paramSet.wow / 100.0f);
            if (auto* param = processor.getParameters().getParameter("lowCutFreq"))
                param->setValue(paramSet.lowCut);
            if (auto* param = processor.getParameters().getParameter("highCutFreq"))
                param->setValue(paramSet.highCut);
            if (auto* param = processor.getParameters().getParameter("drive"))
                param->setValue(paramSet.drive / 100.0f);
            if (auto* param = processor.getParameters().getParameter("tone"))
                param->setValue((paramSet.tone + 100.0f) / 200.0f);
            
            // Test with broadband signal
            juce::AudioBuffer<float> buffer(2, 1024);
            auto testSignal = generateWhiteNoise(0.3f, 1024, 2, 54321);
            buffer.copyFrom(0, 0, testSignal, 0, 0, 1024);
            buffer.copyFrom(1, 0, testSignal, 1, 0, 1024);
            
            juce::MidiBuffer midiBuffer;
            processor.processBlock(buffer, midiBuffer);
            
            INFO("Testing parameter set: " << paramSet.name);
            
            // Validate parameter interaction doesn't cause issues
            REQUIRE_FALSE(hasInvalidValues(buffer));
            REQUIRE_FALSE(hasDeNormals(buffer));
            
            float outputRMS = getRMSLevel(buffer, 0);
            REQUIRE(outputRMS > 0.01f); // Not silent
            REQUIRE(outputRMS < 2.0f); // Not excessive
            
            // Test parameter automation doesn't cause artifacts
            for (int i = 0; i < 10; ++i)
            {
                auto testBuffer = generateTestTone(1000.0f, 0.3f, 48000.0, 512, 2);
                
                // Slightly vary parameters during processing
                if (auto* wowParam = processor.getParameters().getParameter("wowDepth"))
                {
                    float variation = paramSet.wow + (i - 5) * 2.0f; // ±10% variation
                    wowParam->setValue(juce::jlimit(0.0f, 100.0f, variation) / 100.0f);
                }
                
                processor.processBlock(testBuffer, midiBuffer);
                
                REQUIRE_FALSE(hasInvalidValues(testBuffer));
                REQUIRE_FALSE(hasDeNormals(testBuffer));
            }
        }
    }
    
    SECTION("State save/restore consistency test")
    {
        TingeTapeAudioProcessor processor1, processor2;
        processor1.prepareToPlay(48000.0, 512);
        processor2.prepareToPlay(48000.0, 512);
        
        // Set specific parameter values on processor1
        if (auto* param = processor1.getParameters().getParameter("wowDepth"))
            param->setValue(0.42f); // 42%
        if (auto* param = processor1.getParameters().getParameter("drive"))
            param->setValue(0.73f); // 73%
        if (auto* param = processor1.getParameters().getParameter("tone"))
            param->setValue(0.31f); // -38%
        if (auto* param = processor1.getParameters().getParameter("lowCutFreq"))
            param->setValue(85.0f);
        if (auto* param = processor1.getParameters().getParameter("highCutFreq"))
            param->setValue(12500.0f);
        
        // Save state from processor1
        juce::MemoryBlock stateData;
        processor1.getStateInformation(stateData);
        
        // Restore state to processor2
        processor2.setStateInformation(stateData.getData(), static_cast<int>(stateData.getSize()));
        
        // Verify parameters match
        auto compareParam = [&](const char* paramId) {
            auto* param1 = processor1.getParameters().getParameter(paramId);
            auto* param2 = processor2.getParameters().getParameter(paramId);
            if (param1 && param2)
            {
                INFO("Parameter " << paramId << ": P1=" << param1->getValue() << ", P2=" << param2->getValue());
                REQUIRE(param1->getValue() == Approx(param2->getValue()).margin(0.001f));
            }
        };
        
        compareParam("wowDepth");
        compareParam("drive");
        compareParam("tone");
        compareParam("lowCutFreq");
        compareParam("highCutFreq");
        
        // Test that both processors produce identical output
        auto testSignal = generateTestTone(1000.0f, 0.5f, 48000.0, 512, 2);
        
        juce::AudioBuffer<float> buffer1(2, 512), buffer2(2, 512);
        buffer1.copyFrom(0, 0, testSignal, 0, 0, 512);
        buffer1.copyFrom(1, 0, testSignal, 1, 0, 512);
        buffer2.copyFrom(0, 0, testSignal, 0, 0, 512);
        buffer2.copyFrom(1, 0, testSignal, 1, 0, 512);
        
        juce::MidiBuffer midiBuffer;
        processor1.processBlock(buffer1, midiBuffer);
        processor2.processBlock(buffer2, midiBuffer);
        
        // Outputs should be very similar (allowing for minor numerical differences)
        REQUIRE(buffersMatch(buffer1, buffer2, 0.001f));
    }
    
    SECTION("Multiple processing contexts test")
    {
        TingeTapeAudioProcessor processor;
        processor.prepareToPlay(48000.0, 512);
        
        // Set moderate parameters
        if (auto* param = processor.getParameters().getParameter("wowDepth"))
            param->setValue(0.3f);
        if (auto* param = processor.getParameters().getParameter("drive"))
            param->setValue(0.4f);
        
        // Test different block sizes
        std::vector<int> blockSizes = {32, 128, 256, 512, 1024, 2048};
        
        for (int blockSize : blockSizes)
        {
            juce::AudioBuffer<float> buffer(2, blockSize);
            auto testSignal = generateTestTone(440.0f, 0.5f, 48000.0, blockSize, 2);
            buffer.copyFrom(0, 0, testSignal, 0, 0, blockSize);
            buffer.copyFrom(1, 0, testSignal, 1, 0, blockSize);
            
            juce::MidiBuffer midiBuffer;
            processor.processBlock(buffer, midiBuffer);
            
            INFO("Block size: " << blockSize);
            REQUIRE_FALSE(hasInvalidValues(buffer));
            REQUIRE_FALSE(hasDeNormals(buffer));
            
            float rms = getRMSLevel(buffer, 0);
            REQUIRE(rms > 0.1f);
            REQUIRE(rms < 2.0f);
        }
        
        // Test different channel configurations
        std::vector<int> channelCounts = {1, 2}; // Mono and stereo
        
        for (int numChannels : channelCounts)
        {
            juce::AudioBuffer<float> buffer(numChannels, 512);
            auto testSignal = generateTestTone(1000.0f, 0.4f, 48000.0, 512, numChannels);
            
            for (int ch = 0; ch < numChannels; ++ch)
            {
                buffer.copyFrom(ch, 0, testSignal, ch, 0, 512);
            }
            
            juce::MidiBuffer midiBuffer;
            processor.processBlock(buffer, midiBuffer);
            
            INFO("Channels: " << numChannels);
            REQUIRE_FALSE(hasInvalidValues(buffer));
            
            for (int ch = 0; ch < numChannels; ++ch)
            {
                float rms = getRMSLevel(buffer, ch);
                REQUIRE(rms > 0.05f);
                REQUIRE(rms < 2.0f);
            }
        }
    }
    
    SECTION("Parameter boundary testing")
    {
        TingeTapeAudioProcessor processor;
        processor.prepareToPlay(48000.0, 512);
        
        // Test all parameters at extreme values
        struct ParamTest {
            std::string paramId;
            std::vector<float> testValues;
        };
        
        std::vector<ParamTest> paramTests = {
            {"wowDepth", {0.0f, 0.01f, 0.5f, 0.99f, 1.0f}},
            {"drive", {0.0f, 0.01f, 0.5f, 0.99f, 1.0f}},
            {"tone", {0.0f, 0.25f, 0.5f, 0.75f, 1.0f}}, // 0.5 = center/neutral
            {"lowCutFreq", {20.0f, 50.0f, 200.0f}},
            {"highCutFreq", {5000.0f, 15000.0f, 20000.0f}}
        };
        
        for (const auto& paramTest : paramTests)
        {
            for (float value : paramTest.testValues)
            {
                // Reset all parameters to safe defaults
                if (auto* param = processor.getParameters().getParameter("wowDepth"))
                    param->setValue(0.2f);
                if (auto* param = processor.getParameters().getParameter("drive"))
                    param->setValue(0.2f);
                if (auto* param = processor.getParameters().getParameter("tone"))
                    param->setValue(0.5f);
                
                // Set the test parameter
                if (auto* param = processor.getParameters().getParameter(paramTest.paramId))
                {
                    param->setValue(value);
                }
                
                // Test with various signals
                std::vector<std::pair<std::string, juce::AudioBuffer<float>>> testSignals;
                testSignals.emplace_back("sine", generateTestTone(1000.0f, 0.5f, 48000.0, 512, 2));
                testSignals.emplace_back("noise", generateWhiteNoise(0.3f, 512, 2));
                testSignals.emplace_back("impulse", generateImpulse(1.0f, 512, 256, 2));
                
                for (auto& signalPair : testSignals)
                {
                    juce::AudioBuffer<float> buffer = signalPair.second;
                    
                    juce::MidiBuffer midiBuffer;
                    processor.processBlock(buffer, midiBuffer);
                    
                    INFO("Param: " << paramTest.paramId << "=" << value << ", Signal: " << signalPair.first);
                    
                    // Should never produce invalid values regardless of parameter settings
                    REQUIRE_FALSE(hasInvalidValues(buffer));
                    REQUIRE_FALSE(hasDeNormals(buffer));
                    
                    // Output should be reasonable
                    for (int ch = 0; ch < 2; ++ch)
                    {
                        float rms = getRMSLevel(buffer, ch);
                        REQUIRE(std::isfinite(rms));
                        REQUIRE(rms >= 0.0f);
                        if (signalPair.first != "impulse") // Impulse might have very low RMS after processing
                        {
                            REQUIRE(rms < 5.0f); // Reasonable upper bound
                        }
                    }
                }
            }
        }
    }
}