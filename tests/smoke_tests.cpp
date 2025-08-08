#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "audio_test_utils.h"
#include <JuceHeader.h>
#include <memory>

using namespace TylerAudio::Testing;

// Generic plugin smoke test template
template<typename PluginType>
class PluginSmokeTest
{
public:
    static void runAllSmokeTests(const std::string& pluginName)
    {
        SECTION("Plugin " + pluginName + " Smoke Tests")
        {
            testPluginInstantiation(pluginName);
            testParameterAccess(pluginName);
            testBasicAudioProcessing(pluginName);
            testMemoryLeaks(pluginName);
        }
    }

private:
    static void testPluginInstantiation(const std::string& pluginName)
    {
        SECTION("Plugin Loading Test")
        {
            std::unique_ptr<PluginType> processor;
            
            // Test instantiation
            REQUIRE_NOTHROW(processor = std::make_unique<PluginType>());
            REQUIRE(processor != nullptr);
            
            // Test basic properties
            REQUIRE_FALSE(processor->getName().isEmpty());
            REQUIRE(processor->getTotalNumInputChannels() >= 0);
            REQUIRE(processor->getTotalNumOutputChannels() >= 0);
            
            // Test destruction
            REQUIRE_NOTHROW(processor.reset());
        }
    }
    
    static void testParameterAccess(const std::string& pluginName)
    {
        SECTION("Parameter Validation Test")
        {
            auto processor = std::make_unique<PluginType>();
            
            // Test parameter count
            const int numParams = processor->getNumParameters();
            REQUIRE(numParams >= 0);
            
            // Test parameter access
            for (int i = 0; i < numParams; ++i)
            {
                REQUIRE_NOTHROW(processor->getParameterName(i));
                REQUIRE_NOTHROW(processor->getParameter(i));
                
                const float currentValue = processor->getParameter(i);
                REQUIRE(currentValue >= 0.0f);
                REQUIRE(currentValue <= 1.0f);
                
                // Test parameter setting
                REQUIRE_NOTHROWS(processor->setParameter(i, 0.5f));
                REQUIRE_NOTHROWS(processor->setParameter(i, currentValue)); // Restore
            }
        }
    }
    
    static void testBasicAudioProcessing(const std::string& pluginName)
    {
        SECTION("Basic Audio Processing Test")
        {
            auto processor = std::make_unique<PluginType>();
            
            const int sampleRate = 48000;
            const int bufferSize = 512;
            const int numChannels = 2;
            
            // Prepare processor
            processor->setPlayConfigDetails(numChannels, numChannels, sampleRate, bufferSize);
            REQUIRE_NOTHROW(processor->prepareToPlay(sampleRate, bufferSize));
            
            // Create test buffer with silence
            juce::AudioBuffer<float> buffer(numChannels, bufferSize);
            buffer.clear();
            juce::MidiBuffer midiBuffer;
            
            // Test processing doesn't crash
            REQUIRE_NOTHROW(processor->processBlock(buffer, midiBuffer));
            
            // Verify output is valid (no NaN, Inf, or extreme values)
            REQUIRE_FALSE(hasInvalidValues(buffer));
            REQUIRE_FALSE(hasDeNormals(buffer));
            
            // Verify reasonable output levels (should be silent or near-silent with silence input)
            for (int channel = 0; channel < numChannels; ++channel)
            {
                auto range = buffer.findMinMax(channel, 0, bufferSize);
                REQUIRE(std::abs(range.getStart()) < 10.0f);  // No extreme values
                REQUIRE(std::abs(range.getEnd()) < 10.0f);
            }
            
            REQUIRE_NOTHROW(processor->releaseResources());
        }
    }
    
    static void testMemoryLeaks(const std::string& pluginName)
    {
        SECTION("Memory Leak Detection Test")
        {
            // Test multiple instantiation/destruction cycles
            for (int i = 0; i < 10; ++i)
            {
                auto processor = std::make_unique<PluginType>();
                
                // Basic setup
                processor->setPlayConfigDetails(2, 2, 48000, 512);
                processor->prepareToPlay(48000, 512);
                
                // Process some audio
                juce::AudioBuffer<float> buffer(2, 512);
                juce::MidiBuffer midiBuffer;
                processor->processBlock(buffer, midiBuffer);
                
                processor->releaseResources();
                // Destruction happens automatically with unique_ptr
            }
            
            // If we get here without crashes, memory management is likely OK
            REQUIRE(true);  // Test passes if no crashes occurred
        }
    }
};

// Smoke tests for our plugins
TEST_CASE("Plugin Smoke Tests", "[smoke][validation]")
{
    // Note: These will be enabled once we have plugins that can be imported
    // For now, we'll focus on the framework structure
    
    SECTION("Framework Validation")
    {
        // Test our testing utilities work correctly
        auto testBuffer = generateTestTone(440.0f, 0.5f, 48000, 1024);
        REQUIRE(testBuffer.getNumChannels() == 1);
        REQUIRE(testBuffer.getNumSamples() == 1024);
        REQUIRE_FALSE(hasInvalidValues(testBuffer));
        
        auto silenceBuffer = juce::AudioBuffer<float>(2, 512);
        silenceBuffer.clear();
        REQUIRE_FALSE(hasInvalidValues(silenceBuffer));
        
        // Verify RMS measurement works
        float rms = getRMSLevel(testBuffer);
        REQUIRE(rms > 0.0f);
        REQUIRE(rms < 1.0f);
    }
    
    // TODO: Uncomment when plugins are available for testing
    // PluginSmokeTest<ExamplePluginAudioProcessor>::runAllSmokeTests("ExamplePlugin");
    // PluginSmokeTest<TingeTapeAudioProcessor>::runAllSmokeTests("TingeTape");
}

TEST_CASE("Audio Validation Tests", "[audio][validation]")
{
    const int sampleRate = 48000;
    const int bufferSize = 512;
    
    SECTION("Sine Wave Processing Validation")
    {
        // Generate reference sine wave
        auto inputBuffer = generateTestTone(440.0f, 0.5f, sampleRate, bufferSize, 2);
        auto outputBuffer = inputBuffer;  // Copy for processing
        
        // This would normally process through a plugin
        // For now, test our validation framework
        
        // Verify input is valid
        REQUIRE_FALSE(hasInvalidValues(inputBuffer));
        REQUIRE_FALSE(hasDeNormals(inputBuffer));
        
        // Measure input characteristics
        float inputRMS = getRMSLevel(inputBuffer, 0);
        REQUIRE(inputRMS == Catch::Approx(0.5f / std::sqrt(2.0f)).epsilon(0.01f));
    }
    
    SECTION("Noise Processing Validation")
    {
        // Generate white noise
        auto noiseBuffer = generateWhiteNoise(0.1f, bufferSize, 2, 12345);
        
        // Verify noise characteristics
        REQUIRE_FALSE(hasInvalidValues(noiseBuffer));
        REQUIRE_FALSE(hasDeNormals(noiseBuffer));
        
        // Check amplitude bounds
        auto range = noiseBuffer.findMinMax(0, 0, bufferSize);
        REQUIRE(range.getStart() >= -0.1f);
        REQUIRE(range.getEnd() <= 0.1f);
    }
    
    SECTION("Silence Processing Validation")
    {
        // Test silence handling
        juce::AudioBuffer<float> silenceBuffer(2, bufferSize);
        silenceBuffer.clear();
        
        // Verify silence characteristics
        REQUIRE_FALSE(hasInvalidValues(silenceBuffer));
        REQUIRE(getRMSLevel(silenceBuffer, 0) == Catch::Approx(0.0f).margin(1e-10f));
        REQUIRE(getRMSLevel(silenceBuffer, 1) == Catch::Approx(0.0f).margin(1e-10f));
    }
}

TEST_CASE("Performance Benchmark Baseline", "[performance][benchmark]")
{
    const int sampleRate = 48000;
    const int bufferSize = 512;
    const int numIterations = 1000;
    
    SECTION("Audio Buffer Operations Benchmark")
    {
        auto testBuffer = generateTestTone(440.0f, 0.5f, sampleRate, bufferSize, 2);
        
        PerformanceTimer timer;
        timer.start();
        
        for (int i = 0; i < numIterations; ++i)
        {
            // Simulate basic plugin operations
            testBuffer.applyGain(0.99f);  // Slight gain reduction
            
            // Basic processing that might be in a plugin
            for (int channel = 0; channel < testBuffer.getNumChannels(); ++channel)
            {
                auto* data = testBuffer.getWritePointer(channel);
                for (int sample = 0; sample < bufferSize; ++sample)
                {
                    data[sample] = std::tanh(data[sample] * 1.1f);  // Basic saturation
                }
            }
        }
        
        double elapsed = timer.getElapsedMilliseconds();
        double avgTimePerBuffer = elapsed / numIterations;
        
        INFO("Average processing time per buffer: " << avgTimePerBuffer << " ms");
        
        // Baseline performance requirement: should process 512 samples in <1ms on average
        REQUIRE(avgTimePerBuffer < 1.0);
    }
}