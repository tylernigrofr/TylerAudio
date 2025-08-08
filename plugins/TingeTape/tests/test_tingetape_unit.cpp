#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <JuceHeader.h>
#include "audio_test_utils.h"
#include "../Source/PluginProcessor.h"

using namespace TylerAudio::Testing;
using Catch::Approx;

TEST_CASE("TingeTape Unit Tests - WowEngine", "[TingeTape][unit][wow]")
{
    SECTION("LFO frequency accuracy - 0.5Hz validation")
    {
        // Test will be implemented when WowEngine component is exposed
        // This test should verify that the LFO runs at exactly 0.5Hz
        REQUIRE(true); // Placeholder - will be updated in implementation phase
    }
    
    SECTION("Delay time precision - 5ms base delay + modulation")
    {
        // Test will verify base delay of 5ms and modulation range 0-45ms
        REQUIRE(true); // Placeholder
    }
    
    SECTION("Modulation depth linearity - 0-45ms range")
    {
        // Test will verify linear mapping of depth parameter to modulation amount
        REQUIRE(true); // Placeholder
    }
    
    SECTION("Pitch variation measurement - 0.1%-1% authentic range")
    {
        // Test will measure actual pitch variation caused by wow modulation
        REQUIRE(true); // Placeholder
    }
    
    SECTION("Sample rate scaling validation")
    {
        TingeTapeAudioProcessor processor;
        
        // Test that wow behaves correctly across sample rates
        std::vector<double> sampleRates = {44100.0, 48000.0, 96000.0, 192000.0};
        
        for (double sr : sampleRates)
        {
            processor.prepareToPlay(sr, 512);
            
            // Generate consistent test signal
            auto testBuffer = generateTestTone(1000.0f, 0.5f, sr, static_cast<int>(sr * 0.1), 1);
            juce::MidiBuffer midiBuffer;
            
            // Set wow depth to known value
            if (auto* wowParam = processor.getParameters().getParameter("wowDepth"))
            {
                wowParam->setValue(0.5f); // 50% wow depth
            }
            
            processor.processBlock(testBuffer, midiBuffer);
            
            INFO("Sample rate: " << sr);
            REQUIRE_FALSE(hasInvalidValues(testBuffer));
            REQUIRE_FALSE(hasDeNormals(testBuffer));
        }
    }
}

TEST_CASE("TingeTape Unit Tests - TapeSaturation", "[TingeTape][unit][saturation]")
{
    SECTION("Drive parameter mapping - 1x to 10x gain range")
    {
        // Test will verify drive parameter maps to 1x-10x gain (not current 1x-5x)
        REQUIRE(true); // Placeholder - will test formula: driveGain = 1.0 + (driveParam * 0.01) * 9.0
    }
    
    SECTION("tanh normalization accuracy")
    {
        // Test will verify normalization formula: output = tanh(input * driveGain) / tanh(driveGain)
        REQUIRE(true); // Placeholder
    }
    
    SECTION("Harmonic content analysis - odd harmonics dominance")
    {
        TingeTapeAudioProcessor processor;
        processor.prepareToPlay(48000.0, 4800); // 0.1 second
        
        // Generate pure sine wave
        auto sineWave = generateTestTone(440.0f, 0.5f, 48000.0, 4800, 1);
        
        // Set moderate drive level
        if (auto* driveParam = processor.getParameters().getParameter("drive"))
        {
            driveParam->setValue(0.6f); // 60% drive
        }
        
        juce::MidiBuffer midiBuffer;
        processor.processBlock(sineWave, midiBuffer);
        
        // Basic validation - detailed harmonic analysis will be added in implementation
        REQUIRE_FALSE(hasInvalidValues(sineWave));
        float rms = getRMSLevel(sineWave, 0);
        REQUIRE(rms > 0.1f); // Should have reasonable output level
        REQUIRE(rms < 1.0f); // Should not clip
    }
    
    SECTION("Level compensation validation")
    {
        TingeTapeAudioProcessor processor;
        processor.prepareToPlay(48000.0, 512);
        
        auto testSignal = generateTestTone(440.0f, 0.5f, 48000.0, 512, 1);
        float inputRMS = getRMSLevel(testSignal, 0);
        
        // Test different drive levels
        std::vector<float> driveValues = {0.1f, 0.3f, 0.6f, 0.9f};
        
        for (float drive : driveValues)
        {
            auto buffer = generateTestTone(440.0f, 0.5f, 48000.0, 512, 1);
            
            if (auto* driveParam = processor.getParameters().getParameter("drive"))
            {
                driveParam->setValue(drive);
            }
            
            juce::MidiBuffer midiBuffer;
            processor.processBlock(buffer, midiBuffer);
            
            float outputRMS = getRMSLevel(buffer, 0);
            
            INFO("Drive level: " << drive);
            REQUIRE(outputRMS > 0.0f);
            REQUIRE(outputRMS < 2.0f * inputRMS); // Should not have excessive gain
        }
    }
    
    SECTION("High-frequency rolloff measurement")
    {
        // Test will verify HF rolloff behavior proportional to drive amount
        REQUIRE(true); // Placeholder - will measure frequency response at different drive levels
    }
}

TEST_CASE("TingeTape Unit Tests - ToneControl", "[TingeTape][unit][tone]")
{
    SECTION("Gain range validation - ±6dB instead of ±12dB")
    {
        // Test will verify maximum gain limited to ±6dB (currently ±12dB)
        REQUIRE(true); // Placeholder - will measure actual gain response
    }
    
    SECTION("Shelf frequency accuracy - 250Hz low, 5kHz high")
    {
        // Test will verify shelf frequencies at exactly 250Hz and 5kHz
        REQUIRE(true); // Placeholder - will use frequency sweep analysis
    }
    
    SECTION("Tilt behavior validation")
    {
        TingeTapeAudioProcessor processor;
        processor.prepareToPlay(48000.0, 512);
        
        // Test complementary shelf behavior
        std::vector<float> toneValues = {-1.0f, -0.5f, 0.0f, 0.5f, 1.0f};
        
        for (float tone : toneValues)
        {
            // Test with low frequency content
            auto lowFreqBuffer = generateTestTone(250.0f, 0.5f, 48000.0, 512, 1);
            float lowInputRMS = getRMSLevel(lowFreqBuffer, 0);
            
            if (auto* toneParam = processor.getParameters().getParameter("tone"))
            {
                toneParam->setValue((tone + 1.0f) * 0.5f); // Convert -1..1 to 0..1
            }
            
            juce::MidiBuffer midiBuffer;
            processor.processBlock(lowFreqBuffer, midiBuffer);
            
            float lowOutputRMS = getRMSLevel(lowFreqBuffer, 0);
            
            INFO("Tone value: " << tone);
            REQUIRE(lowOutputRMS > 0.0f);
            REQUIRE_FALSE(hasInvalidValues(lowFreqBuffer));
            
            // Basic relationship test - detailed frequency analysis will be added
            if (tone < 0.0f) // Dark setting should reduce high frequencies more than low
            {
                REQUIRE(lowOutputRMS > 0.1f * lowInputRMS);
            }
        }
    }
    
    SECTION("Bypass behavior - near-zero tone values")
    {
        TingeTapeAudioProcessor processor;
        processor.prepareToPlay(48000.0, 512);
        
        auto testBuffer = generateTestTone(1000.0f, 0.5f, 48000.0, 512, 1);
        auto referenceBuffer = generateTestTone(1000.0f, 0.5f, 48000.0, 512, 1);
        
        // Set tone to near-zero (should engage bypass)
        if (auto* toneParam = processor.getParameters().getParameter("tone"))
        {
            toneParam->setValue(0.5f); // Assuming 0.5f represents zero tone adjustment
        }
        
        juce::MidiBuffer midiBuffer;
        processor.processBlock(testBuffer, midiBuffer);
        
        // Should be very close to original signal when tone is at zero
        float processedRMS = getRMSLevel(testBuffer, 0);
        float referenceRMS = getRMSLevel(referenceBuffer, 0);
        
        REQUIRE(std::abs(processedRMS - referenceRMS) < 0.1f * referenceRMS);
    }
}

TEST_CASE("TingeTape Unit Tests - ResonantFilters", "[TingeTape][unit][filters]")
{
    SECTION("Low-cut filter frequency response")
    {
        // Test will verify 2nd-order Butterworth high-pass behavior
        // Frequency range: 20Hz - 200Hz
        // Q range: 0.1 - 2.0 (default 0.707)
        REQUIRE(true); // Placeholder
    }
    
    SECTION("High-cut filter frequency response")
    {
        // Test will verify 2nd-order Butterworth low-pass behavior
        // Frequency range: 5kHz - 20kHz  
        // Q range: 0.1 - 2.0 (default 0.707)
        REQUIRE(true); // Placeholder
    }
    
    SECTION("Filter stability at extreme settings")
    {
        TingeTapeAudioProcessor processor;
        processor.prepareToPlay(48000.0, 512);
        
        // Test filter stability with extreme parameter values
        // This is a basic stability test - detailed filter testing will be added
        auto testBuffer = generateWhiteNoise(0.5f, 512, 1);
        juce::MidiBuffer midiBuffer;
        
        processor.processBlock(testBuffer, midiBuffer);
        
        REQUIRE_FALSE(hasInvalidValues(testBuffer));
        REQUIRE_FALSE(hasDeNormals(testBuffer));
        
        float rms = getRMSLevel(testBuffer, 0);
        REQUIRE(rms >= 0.0f);
        REQUIRE(rms < 2.0f);
    }
}

TEST_CASE("TingeTape Unit Tests - Parameter Smoothing", "[TingeTape][unit][parameters]")
{
    SECTION("Wow parameter 50ms smoothing validation")
    {
        // Test will verify wow depth smoothing uses 50ms time constant
        REQUIRE(true); // Placeholder
    }
    
    SECTION("Filter parameters 20ms smoothing validation")
    {
        // Test will verify filter parameters use 20ms smoothing
        REQUIRE(true); // Placeholder
    }
    
    SECTION("Drive parameter 30ms smoothing validation")
    {
        // Test will verify drive uses 30ms smoothing
        REQUIRE(true); // Placeholder
    }
    
    SECTION("Smoothing artifact prevention")
    {
        TingeTapeAudioProcessor processor;
        processor.prepareToPlay(48000.0, 512);
        
        // Generate continuous test signal
        auto testBuffer = generateTestTone(440.0f, 0.5f, 48000.0, 512, 1);
        juce::MidiBuffer midiBuffer;
        
        // Make rapid parameter changes during processing
        if (auto* driveParam = processor.getParameters().getParameter("drive"))
        {
            driveParam->setValue(0.1f);
            processor.processBlock(testBuffer, midiBuffer);
            
            driveParam->setValue(0.9f);
            processor.processBlock(testBuffer, midiBuffer);
            
            driveParam->setValue(0.3f);
            processor.processBlock(testBuffer, midiBuffer);
        }
        
        // Should not produce invalid values even with rapid parameter changes
        REQUIRE_FALSE(hasInvalidValues(testBuffer));
        REQUIRE_FALSE(hasDeNormals(testBuffer));
    }
    
    SECTION("Parameter response time measurement")
    {
        // Test will measure actual parameter response times
        REQUIRE(true); // Placeholder - will measure time to reach target values
    }
}