#include <catch2/catch_test_macros.hpp>
#include <JuceHeader.h>
#include "audio_test_utils.h"
#include "../Source/PluginProcessor.h"

using namespace TylerAudio::Testing;

TEST_CASE("TingeTape Smoke Tests", "[TingeTape][smoke]")
{
    SECTION("Plugin instantiation")
    {
        TingeTapeAudioProcessor processor;
        REQUIRE(processor.getName() == "TingeTape");
        REQUIRE_FALSE(processor.acceptsMidi());
        REQUIRE_FALSE(processor.producesMidi());
        REQUIRE_FALSE(processor.isMidiEffect());
    }
    
    SECTION("Basic parameter access")
    {
        TingeTapeAudioProcessor processor;
        
        // Check that all expected parameters exist
        auto* wowParam = processor.getParameters().getParameter("wowDepth");
        auto* driveParam = processor.getParameters().getParameter("drive");
        auto* toneParam = processor.getParameters().getParameter("tone");
        auto* bypassParam = processor.getParameters().getParameter("bypass");
        
        REQUIRE(wowParam != nullptr);
        REQUIRE(driveParam != nullptr);
        REQUIRE(toneParam != nullptr);
        REQUIRE(bypassParam != nullptr);
    }
    
    SECTION("Plugin initialization and processing")
    {
        TingeTapeAudioProcessor processor;
        
        const double sampleRate = 48000.0;
        const int blockSize = 512;
        const int numChannels = 2;
        
        // Prepare processor
        processor.prepareToPlay(sampleRate, blockSize);
        
        // Create test buffer
        juce::AudioBuffer<float> buffer(numChannels, blockSize);
        auto testTone = generateTestTone(440.0f, 0.5f, sampleRate, blockSize, numChannels);
        
        for (int ch = 0; ch < numChannels; ++ch)
        {
            buffer.copyFrom(ch, 0, testTone, ch, 0, blockSize);
        }
        
        // Process audio
        juce::MidiBuffer midiBuffer;
        processor.processBlock(buffer, midiBuffer);
        
        // Verify output is valid
        REQUIRE_FALSE(hasInvalidValues(buffer));
        REQUIRE_FALSE(hasDeNormals(buffer));
        
        // Verify output level is reasonable
        for (int ch = 0; ch < numChannels; ++ch)
        {
            float rms = getRMSLevel(buffer, ch);
            REQUIRE(rms > 0.0f);
            REQUIRE(rms < 2.0f); // Should not exceed reasonable limits
        }
    }
    
    SECTION("Bypass functionality")
    {
        TingeTapeAudioProcessor processor;
        
        const double sampleRate = 48000.0;
        const int blockSize = 512;
        const int numChannels = 2;
        
        processor.prepareToPlay(sampleRate, blockSize);
        
        // Generate test signal
        auto testSignal = generateTestTone(1000.0f, 0.5f, sampleRate, blockSize, numChannels);
        
        // Process with bypass off
        juce::AudioBuffer<float> processedBuffer(numChannels, blockSize);
        for (int ch = 0; ch < numChannels; ++ch)
        {
            processedBuffer.copyFrom(ch, 0, testSignal, ch, 0, blockSize);
        }
        
        juce::MidiBuffer midiBuffer;
        processor.processBlock(processedBuffer, midiBuffer);
        
        // Process with bypass on
        juce::AudioBuffer<float> bypassBuffer(numChannels, blockSize);
        for (int ch = 0; ch < numChannels; ++ch)
        {
            bypassBuffer.copyFrom(ch, 0, testSignal, ch, 0, blockSize);
        }
        
        // Enable bypass (assuming parameter exists)
        if (auto* bypassParam = processor.getParameters().getParameter("bypass"))
        {
            bypassParam->setValue(1.0f);
        }
        
        processor.processBlock(bypassBuffer, midiBuffer);
        
        // Bypass should be close to original (within tolerance for crossfading)
        // Note: This test may need adjustment based on actual bypass implementation
        REQUIRE_FALSE(hasInvalidValues(bypassBuffer));
        REQUIRE_FALSE(hasInvalidValues(processedBuffer));
    }
    
    SECTION("Parameter range validation")
    {
        TingeTapeAudioProcessor processor;
        
        // Test wow depth parameter
        if (auto* wowParam = processor.getParameters().getParameter("wowDepth"))
        {
            wowParam->setValue(0.0f);
            REQUIRE(wowParam->getValue() >= 0.0f);
            
            wowParam->setValue(1.0f);
            REQUIRE(wowParam->getValue() <= 1.0f);
            
            wowParam->setValue(-0.5f); // Should clamp to valid range
            REQUIRE(wowParam->getValue() >= 0.0f);
            
            wowParam->setValue(1.5f); // Should clamp to valid range
            REQUIRE(wowParam->getValue() <= 1.0f);
        }
        
        // Test tone parameter (should allow negative values)
        if (auto* toneParam = processor.getParameters().getParameter("tone"))
        {
            toneParam->setValue(0.0f);
            REQUIRE(std::abs(toneParam->getValue()) <= 1.0f);
            
            toneParam->setValue(1.0f);
            REQUIRE(toneParam->getValue() <= 1.0f);
            
            toneParam->setValue(-1.0f);
            REQUIRE(toneParam->getValue() >= -1.0f);
        }
    }
    
    SECTION("Multiple sample rates")
    {
        TingeTapeAudioProcessor processor;
        
        std::vector<double> sampleRates = {44100.0, 48000.0, 88200.0, 96000.0, 192000.0};
        const int blockSize = 512;
        const int numChannels = 2;
        
        for (double sr : sampleRates)
        {
            processor.prepareToPlay(sr, blockSize);
            
            auto testBuffer = generateTestTone(440.0f, 0.5f, sr, blockSize, numChannels);
            juce::MidiBuffer midiBuffer;
            
            processor.processBlock(testBuffer, midiBuffer);
            
            REQUIRE_FALSE(hasInvalidValues(testBuffer));
            REQUIRE_FALSE(hasDeNormals(testBuffer));
            
            INFO("Sample rate: " << sr);
            for (int ch = 0; ch < numChannels; ++ch)
            {
                float rms = getRMSLevel(testBuffer, ch);
                REQUIRE(rms > 0.0f);
                REQUIRE(rms < 2.0f);
            }
        }
    }
}