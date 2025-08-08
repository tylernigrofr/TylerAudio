#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "TylerAudioCommon.h"
#include "audio_test_utils.h"
#include <cmath>

using namespace TylerAudio;
using namespace TylerAudio::Testing;

TEST_CASE("TylerAudio::Utils dB conversion functions are accurate", "[utils][dsp]")
{
    SECTION("Gain to dB conversion")
    {
        REQUIRE(Utils::gainToDb(1.0f) == Catch::Approx(0.0f).epsilon(1e-6f));
        REQUIRE(Utils::gainToDb(0.5f) == Catch::Approx(-6.0206f).epsilon(1e-3f));
        REQUIRE(Utils::gainToDb(0.1f) == Catch::Approx(-20.0f).epsilon(1e-6f));
        REQUIRE(Utils::gainToDb(2.0f) == Catch::Approx(6.0206f).epsilon(1e-3f));
        
        // Test very small values don't cause issues
        REQUIRE(std::isfinite(Utils::gainToDb(0.000001f)));
        REQUIRE(Utils::gainToDb(0.000001f) < -100.0f);  // Should be very negative
    }
    
    SECTION("dB to gain conversion")
    {
        REQUIRE(Utils::dbToGain(0.0f) == Catch::Approx(1.0f).epsilon(1e-6f));
        REQUIRE(Utils::dbToGain(-6.0206f) == Catch::Approx(0.5f).epsilon(1e-3f));
        REQUIRE(Utils::dbToGain(-20.0f) == Catch::Approx(0.1f).epsilon(1e-6f));
        REQUIRE(Utils::dbToGain(6.0206f) == Catch::Approx(2.0f).epsilon(1e-3f));
        
        // Test extreme values
        REQUIRE(Utils::dbToGain(-60.0f) == Catch::Approx(0.001f).epsilon(1e-6f));
        REQUIRE(Utils::dbToGain(-120.0f) < 1e-6f);  // Should be very small
    }
    
    SECTION("Round-trip conversion accuracy")
    {
        std::vector<float> testGains = {0.001f, 0.1f, 0.5f, 1.0f, 1.5f, 2.0f, 10.0f};
        
        for (float originalGain : testGains)
        {
            float db = Utils::gainToDb(originalGain);
            float convertedGain = Utils::dbToGain(db);
            
            REQUIRE(convertedGain == Catch::Approx(originalGain).epsilon(1e-5f));
        }
    }
}

TEST_CASE("TylerAudio::Constants have reasonable values", "[constants]")
{
    REQUIRE(Constants::defaultWidth > 0);
    REQUIRE(Constants::defaultHeight > 0);
    REQUIRE(Constants::defaultGain >= 0.0f);
    REQUIRE(Constants::defaultGain <= 1.0f);
    
    // Test that default dimensions are reasonable for a plugin GUI
    REQUIRE(Constants::defaultWidth >= 200);  // Not too small
    REQUIRE(Constants::defaultWidth <= 2000); // Not too large
    REQUIRE(Constants::defaultHeight >= 150);
    REQUIRE(Constants::defaultHeight <= 1500);
}

TEST_CASE("Audio test utilities work correctly", "[test_utils]")
{
    const int sampleRate = 48000;
    const int bufferSize = 1024;
    const float frequency = 440.0f;
    const float amplitude = 0.5f;
    
    SECTION("Test tone generation")
    {
        auto buffer = generateTestTone(frequency, amplitude, sampleRate, bufferSize);
        
        REQUIRE(buffer.getNumChannels() == 1);
        REQUIRE(buffer.getNumSamples() == bufferSize);
        
        // Check amplitude is approximately correct
        float maxSample = buffer.getMagnitude(0, 0, bufferSize);
        REQUIRE(maxSample == Catch::Approx(amplitude).epsilon(0.1f));
        
        // Should not have invalid values
        REQUIRE_FALSE(hasInvalidValues(buffer));
        REQUIRE_FALSE(hasDeNormals(buffer));
    }
    
    SECTION("White noise generation")
    {
        auto buffer = generateWhiteNoise(amplitude, bufferSize, 1, 12345);
        
        REQUIRE(buffer.getNumChannels() == 1);
        REQUIRE(buffer.getNumSamples() == bufferSize);
        
        // Check amplitude bounds
        auto range = buffer.findMinMax(0, 0, bufferSize);
        REQUIRE(range.getStart() >= -amplitude);
        REQUIRE(range.getEnd() <= amplitude);
        
        // Should not have invalid values
        REQUIRE_FALSE(hasInvalidValues(buffer));
    }
    
    SECTION("Impulse generation")
    {
        auto buffer = generateImpulse(1.0f, bufferSize, 100);
        
        REQUIRE(buffer.getNumChannels() == 1);
        REQUIRE(buffer.getNumSamples() == bufferSize);
        
        // Check impulse is at correct position
        REQUIRE(buffer.getSample(0, 100) == Catch::Approx(1.0f));
        
        // Check other samples are zero
        REQUIRE(buffer.getSample(0, 99) == Catch::Approx(0.0f));
        REQUIRE(buffer.getSample(0, 101) == Catch::Approx(0.0f));
    }
    
    SECTION("Buffer comparison")
    {
        auto buffer1 = generateTestTone(frequency, amplitude, sampleRate, bufferSize);
        auto buffer2 = generateTestTone(frequency, amplitude, sampleRate, bufferSize);
        auto buffer3 = generateTestTone(frequency * 2.0f, amplitude, sampleRate, bufferSize);
        
        // Identical buffers should match
        REQUIRE(buffersMatch(buffer1, buffer2, 1e-6f));
        
        // Different buffers should not match
        REQUIRE_FALSE(buffersMatch(buffer1, buffer3, 1e-6f));
    }
    
    SECTION("RMS level measurement")
    {
        // Test with known DC signal
        juce::AudioBuffer<float> dcBuffer(1, bufferSize);
        dcBuffer.clear();
        dcBuffer.applyGain(0.5f);  // Set all samples to 0.5
        
        float rms = getRMSLevel(dcBuffer);
        REQUIRE(rms == Catch::Approx(0.5f).epsilon(1e-6f));
        
        // Test with sine wave (RMS should be amplitude / sqrt(2))
        auto sineBuffer = generateTestTone(frequency, amplitude, sampleRate, bufferSize);
        float sineRMS = getRMSLevel(sineBuffer);
        float expectedRMS = amplitude / std::sqrt(2.0f);
        REQUIRE(sineRMS == Catch::Approx(expectedRMS).epsilon(0.01f));
    }
}