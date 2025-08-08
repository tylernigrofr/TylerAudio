# Tyler Audio Framework API Reference

## Overview
This document provides comprehensive API documentation for the Tyler Audio Framework's core components, utilities, and testing systems.

## Core Framework Components

### TylerAudio::Common

#### SmoothingFilter
Provides smooth parameter transitions to avoid audio artifacts.

```cpp
class SmoothingFilter {
public:
    /** Sets the target value for the filter */
    void setTargetValue(float newValue) noexcept;
    
    /** Returns the next smoothed value */
    float getNextValue() noexcept;
    
    /** Sets the smoothing time constant */
    void setSmoothingTime(float timeMs, double sampleRate) noexcept;
    
    /** Resets the filter state */
    void reset(float initialValue = 0.0f) noexcept;

private:
    std::atomic<float> targetValue{0.0f};
    float currentValue{0.0f};
    float smoothingCoeff{0.01f};
};
```

**Usage Example:**
```cpp
TylerAudio::SmoothingFilter gainSmoother;
gainSmoother.setSmoothingTime(50.0f, 48000.0); // 50ms smoothing

// In processBlock
gainSmoother.setTargetValue(gainParam->load());
float smoothedGain = gainSmoother.getNextValue();
```

#### RealtimeUtils
Utilities for realtime-safe audio processing.

```cpp
namespace RealtimeUtils {
    /** Check if value is denormal and flush to zero if needed */
    inline float flushDenormal(float value) noexcept;
    
    /** Fast approximation of tanh for saturation */
    inline float fastTanh(float x) noexcept;
    
    /** Linear interpolation */
    inline float lerp(float a, float b, float t) noexcept;
    
    /** Clamp value between min and max */
    inline float clamp(float value, float min, float max) noexcept;
}
```

---

## Testing Framework API

### TestUtilities::SignalGenerator
Generates various test signals for audio testing.

```cpp
class SignalGenerator {
public:
    enum class SignalType {
        Silence, Sine, Cosine, WhiteNoise, PinkNoise, 
        Impulse, Step, Chirp, Square, Sawtooth, Triangle
    };
    
    /** Generate signal into existing buffer */
    static void generateSignal(juce::AudioBuffer<float>& buffer, 
                              SignalType type, 
                              float frequency = 1000.0f,
                              double sampleRate = 48000.0,
                              float amplitude = 1.0f,
                              float phase = 0.0f);
                              
    /** Generate multi-tone signal for harmonic analysis */
    static void generateMultiTone(juce::AudioBuffer<float>& buffer,
                                 const std::vector<float>& frequencies,
                                 const std::vector<float>& amplitudes,
                                 double sampleRate = 48000.0,
                                 float phase = 0.0f);
};
```

**Usage Example:**
```cpp
juce::AudioBuffer<float> testBuffer(2, 1024);
TestUtilities::SignalGenerator::generateSignal(
    testBuffer, 
    TestUtilities::SignalGenerator::SignalType::Sine,
    1000.0f,  // 1kHz
    48000.0   // Sample rate
);
```

### TestUtilities::AudioAnalyzer
Analyzes audio content for testing purposes.

```cpp
class AudioAnalyzer {
public:
    /** Calculate RMS level of buffer */
    static float calculateRMS(const juce::AudioBuffer<float>& buffer, 
                             int channel = -1);
                             
    /** Calculate peak level of buffer */
    static float calculatePeak(const juce::AudioBuffer<float>& buffer,
                              int channel = -1);
                              
    /** Calculate THD+N (Total Harmonic Distortion + Noise) */
    static float calculateTHDN(const juce::AudioBuffer<float>& input,
                              const juce::AudioBuffer<float>& output,
                              float fundamentalFreq,
                              double sampleRate,
                              int numHarmonics = 5);
                              
    /** Calculate signal-to-noise ratio */
    static float calculateSNR(const juce::AudioBuffer<float>& signal,
                             const juce::AudioBuffer<float>& noise);
};
```

**Usage Example:**
```cpp
float rms = TestUtilities::AudioAnalyzer::calculateRMS(audioBuffer);
float peak = TestUtilities::AudioAnalyzer::calculatePeak(audioBuffer);
float thd = TestUtilities::AudioAnalyzer::calculateTHDN(input, output, 1000.0f, 48000.0);
```

### TestUtilities::PerformanceMeter
Measures and analyzes performance characteristics.

```cpp
class PerformanceMeter {
public:
    struct MeasurementResults {
        double averageTimeMs;
        double minTimeMs;
        double maxTimeMs;
        double stdDeviationMs;
        size_t numSamples;
        double cpuUsagePercent;
        bool realtimeSafe;
    };
    
    /** Measure processing time with multiple iterations */
    static MeasurementResults measureProcessingTime(
        std::function<void()> processor,
        int numIterations = 1000,
        int warmupIterations = 100);
        
    /** Validate real-time performance compliance */
    static bool validateRealtimePerformance(
        std::function<void()> processor,
        double sampleRate = 48000.0,
        int blockSize = 512,
        double maxCPUPercent = 50.0);
};
```

**Usage Example:**
```cpp
auto results = TestUtilities::PerformanceMeter::measureProcessingTime(
    [&processor, &buffer, &midi]() {
        processor.processBlock(buffer, midi);
    }
);

REQUIRE(results.averageTimeMs < 1.0); // <1ms for realtime safety
```

### TestUtilities::AudioQualityValidator
Validates audio quality and detects common problems.

```cpp
class AudioQualityValidator {
public:
    struct QualityMetrics {
        float dynamicRange;    // dB
        float thdPlusN;        // %
        float snr;             // dB
        bool hasAudioDropouts;
        bool hasClipping;
        bool hasInfiniteValues;
        bool hasNaNValues;
    };
    
    /** Comprehensive audio quality analysis */
    static QualityMetrics analyzeAudioQuality(
        const juce::AudioBuffer<float>& input,
        const juce::AudioBuffer<float>& output,
        double sampleRate = 48000.0);
        
    /** Validate audio for common problems */
    static bool validateAudioIntegrity(const juce::AudioBuffer<float>& buffer);
    
    /** Check for audio dropouts */
    static bool hasDropouts(const juce::AudioBuffer<float>& buffer,
                           float threshold = -60.0f);
                           
    /** Check for clipping */
    static bool hasClipping(const juce::AudioBuffer<float>& buffer,
                           float threshold = 0.95f);
};
```

**Usage Example:**
```cpp
auto metrics = TestUtilities::AudioQualityValidator::analyzeAudioQuality(
    inputBuffer, outputBuffer, 48000.0);

REQUIRE_FALSE(metrics.hasClipping);
REQUIRE_FALSE(metrics.hasAudioDropouts);
REQUIRE(metrics.snr > 80.0f);
```

### TestUtilities::PluginTester
Specialized testing utilities for audio plugins.

```cpp
class PluginTester {
public:
    /** Test plugin with various buffer sizes */
    static bool testBufferSizes(juce::AudioProcessor& processor,
                               const std::vector<int>& bufferSizes = {32, 64, 128, 256, 512, 1024});
                               
    /** Test plugin with various sample rates */
    static bool testSampleRates(juce::AudioProcessor& processor,
                               const std::vector<double>& sampleRates = {44100.0, 48000.0, 96000.0});
                               
    /** Test parameter automation with ramps */
    static bool testParameterAutomation(juce::AudioProcessor& processor,
                                       int parameterIndex,
                                       float startValue,
                                       float endValue,
                                       int rampLengthSamples = 1024);
                                       
    /** Test plugin state save/restore */
    static bool testStateConsistency(juce::AudioProcessor& processor,
                                   int numIterations = 100);
};
```

**Usage Example:**
```cpp
// Test plugin across different buffer sizes
bool bufferSizeTest = TestUtilities::PluginTester::testBufferSizes(processor);
REQUIRE(bufferSizeTest);

// Test parameter automation
bool automationTest = TestUtilities::PluginTester::testParameterAutomation(
    processor, 0, 0.0f, 1.0f, 2048);
REQUIRE(automationTest);
```

---

## UI Testing API

### UITestUtilities::ComponentTester
Testing utilities for JUCE components.

```cpp
class ComponentTester {
public:
    /** Test component visibility and bounds */
    static bool testComponentVisibility(juce::Component& component);
    
    /** Test component resizing behavior */
    static bool testComponentResizing(juce::Component& component,
                                     const std::vector<juce::Rectangle<int>>& testBounds);
    
    /** Test component painting (no crashes during paint) */
    static bool testComponentPainting(juce::Component& component,
                                     int width = 400, int height = 300);
    
    /** Test component accessibility */
    static bool testComponentAccessibility(juce::Component& component);
    
    /** Test keyboard navigation */
    static bool testKeyboardNavigation(juce::Component& component);
};
```

### UITestUtilities::VisualTester
Visual testing and regression detection.

```cpp
class VisualTester {
public:
    struct VisualTestResult {
        bool passed;
        juce::String errorMessage;
        juce::Image capturedImage;
        std::vector<juce::Rectangle<int>> problemAreas;
    };
    
    /** Capture component as image for visual comparison */
    static juce::Image captureComponent(juce::Component& component);
    
    /** Compare two images for visual differences */
    static VisualTestResult compareImages(const juce::Image& reference,
                                         const juce::Image& test,
                                         float tolerance = 0.95f);
    
    /** Test component rendering at different DPI scales */
    static bool testDPIScaling(juce::Component& component,
                              const std::vector<float>& dpiScales = {1.0f, 1.25f, 1.5f, 2.0f});
};
```

---

## Performance Testing API

### PerformanceTestFramework::CPUProfiler
Advanced CPU usage analysis for audio processing.

```cpp
class CPUProfiler {
public:
    struct ProfileResult {
        double averageCPUPercent;
        double peakCPUPercent;
        double minimumCPUPercent;
        std::vector<double> cpuHistory;
        bool realtimeSafe;
        size_t numSamples;
    };
    
    /** Profile CPU usage of audio processing function */
    static ProfileResult profileAudioProcessing(
        std::function<void(juce::AudioBuffer<float>&, juce::MidiBuffer&)> processor,
        double sampleRate = 48000.0,
        int blockSize = 512,
        int durationSeconds = 10);
    
    /** Profile plugin CPU usage with parameter automation */
    static ProfileResult profileWithParameterAutomation(
        juce::AudioProcessor& processor,
        const std::vector<int>& parameterIndices,
        double sampleRate = 48000.0,
        int blockSize = 512,
        int durationSeconds = 10);
};
```

### PerformanceTestFramework::MemoryProfiler
Memory usage analysis and leak detection.

```cpp
class MemoryProfiler {
public:
    struct MemorySnapshot {
        size_t totalAllocatedBytes;
        size_t peakAllocatedBytes;
        size_t currentUsageBytes;
        int numAllocations;
        int numDeallocations;
        bool hasLeaks;
        double allocationRate;
    };
    
    /** Profile memory usage during audio processing */
    static MemorySnapshot profileMemoryUsage(
        std::function<void()> processor,
        int durationSeconds = 10);
    
    /** Detect memory leaks during extended operation */
    static bool detectMemoryLeaks(
        juce::AudioProcessor& processor,
        int numIterations = 10000,
        double sampleRate = 48000.0,
        int blockSize = 512);
};
```

---

## Integration Testing API

### IntegrationTestFramework::DAWSimulator
Simulate different DAW environments for compatibility testing.

```cpp
class DAWSimulator {
public:
    enum class DAWType {
        Generic, Reaper, LogicPro, ProTools, 
        AbletonLive, StudioOne, FLStudio, Cubase
    };
    
    struct DAWEnvironment {
        DAWType type;
        double sampleRate;
        int bufferSize;
        bool supportsVST3;
        bool supportsAU;
        std::vector<std::string> quirks;
    };
    
    /** Simulate plugin loading in DAW */
    static bool simulatePluginLoad(
        juce::AudioProcessor& processor,
        const DAWEnvironment& environment);
    
    /** Simulate typical DAW workflow */
    static bool simulateDAWWorkflow(
        juce::AudioProcessor& processor,
        const DAWEnvironment& environment,
        int durationMinutes = 5);
};
```

### IntegrationTestFramework::FormatCompatibilityTester
Test plugin format compliance and compatibility.

```cpp
class FormatCompatibilityTester {
public:
    struct FormatTestResult {
        bool vst3Compatible;
        bool auCompatible;
        std::vector<std::string> vst3Issues;
        std::vector<std::string> auIssues;
        bool meetsFormatSpec;
    };
    
    /** Test VST3 format compliance */
    static FormatTestResult testVST3Compatibility(juce::AudioProcessor& processor);
    
    /** Test Audio Unit format compliance */
    static FormatTestResult testAUCompatibility(juce::AudioProcessor& processor);
    
    /** Test plugin with pluginval */
    static std::vector<std::string> runPluginval(
        const std::string& pluginPath,
        int strictnessLevel = 5);
};
```

---

## Utility Functions

### Parameter Layout Helpers
Common parameter layout patterns for quick plugin development.

```cpp
namespace ParameterLayouts {
    /** Create standard gain parameter */
    std::unique_ptr<juce::AudioParameterFloat> createGainParameter(
        const juce::String& paramID = "gain",
        const juce::String& paramName = "Gain",
        float minGain = -60.0f,
        float maxGain = 12.0f,
        float defaultGain = 0.0f);
    
    /** Create frequency parameter with logarithmic scaling */
    std::unique_ptr<juce::AudioParameterFloat> createFrequencyParameter(
        const juce::String& paramID,
        const juce::String& paramName,
        float minFreq = 20.0f,
        float maxFreq = 20000.0f,
        float defaultFreq = 1000.0f);
    
    /** Create standard envelope ADSR parameters */
    juce::AudioProcessorValueTreeState::ParameterLayout createADSRParameters(
        const juce::String& prefix = "");
    
    /** Create standard filter parameters */
    juce::AudioProcessorValueTreeState::ParameterLayout createFilterParameters(
        const juce::String& prefix = "filter");
}
```

### DSP Utilities
Common DSP utility functions.

```cpp
namespace DSPUtils {
    /** Convert frequency to normalized value (0-1) */
    float frequencyToNormalized(float frequency, float minFreq, float maxFreq);
    
    /** Convert normalized value to frequency */
    float normalizedToFrequency(float normalized, float minFreq, float maxFreq);
    
    /** Convert gain in dB to linear */
    inline float dBToLinear(float dB) { return std::pow(10.0f, dB * 0.05f); }
    
    /** Convert linear gain to dB */
    inline float linearTodB(float linear) { return 20.0f * std::log10(linear); }
    
    /** Bipolar to unipolar conversion */
    inline float bipolarToUnipolar(float bipolar) { return (bipolar + 1.0f) * 0.5f; }
    
    /** Unipolar to bipolar conversion */
    inline float unipolarToBipolar(float unipolar) { return unipolar * 2.0f - 1.0f; }
}
```

---

## Macros and Constants

### Framework Constants
```cpp
namespace TylerAudio {
    namespace Constants {
        static constexpr float PI = 3.14159265359f;
        static constexpr float TWO_PI = 6.28318530718f;
        static constexpr float SQRT2 = 1.41421356237f;
        
        // Audio constants
        static constexpr double DEFAULT_SAMPLE_RATE = 48000.0;
        static constexpr int DEFAULT_BLOCK_SIZE = 512;
        static constexpr float MIN_FREQUENCY = 20.0f;
        static constexpr float MAX_FREQUENCY = 20000.0f;
        
        // Performance targets
        static constexpr double MAX_CPU_PERCENT = 1.0;
        static constexpr double MAX_PROCESSING_TIME_MS = 10.67; // 512 samples at 48kHz
    }
}
```

### Convenience Macros
```cpp
// Parameter creation shortcuts
#define TYLER_AUDIO_PARAM_FLOAT(id, name, min, max, def) \
    std::make_unique<juce::AudioParameterFloat>(id, name, \
        juce::NormalisableRange<float>(min, max), def)

#define TYLER_AUDIO_PARAM_BOOL(id, name, def) \
    std::make_unique<juce::AudioParameterBool>(id, name, def)

#define TYLER_AUDIO_PARAM_CHOICE(id, name, choices, def) \
    std::make_unique<juce::AudioParameterChoice>(id, name, choices, def)

// Testing shortcuts
#define TYLER_AUDIO_TEST_PROCESSOR(ProcessorClass) \
    ProcessorClass processor; \
    processor.prepareToPlay(48000.0, 512)

#define TYLER_AUDIO_REQUIRE_REALTIME_SAFE(processor) \
    REQUIRE(TestUtilities::PerformanceMeter::validateRealtimePerformance( \
        [&processor]() { /* test processing */ }))
```

---

## Error Handling

### Exception Types
```cpp
namespace TylerAudio {
    /** Base exception for framework errors */
    class FrameworkException : public std::exception {
    public:
        explicit FrameworkException(const std::string& message) : message_(message) {}
        const char* what() const noexcept override { return message_.c_str(); }
    private:
        std::string message_;
    };
    
    /** Exception for realtime violations */
    class RealtimeViolationException : public FrameworkException {
    public:
        explicit RealtimeViolationException(const std::string& details)
            : FrameworkException("Realtime violation: " + details) {}
    };
    
    /** Exception for performance violations */
    class PerformanceException : public FrameworkException {
    public:
        explicit PerformanceException(const std::string& details)
            : FrameworkException("Performance violation: " + details) {}
    };
}
```

### Error Reporting
```cpp
namespace TylerAudio {
    /** Error reporting and logging */
    class ErrorReporter {
    public:
        enum class Severity { Info, Warning, Error, Critical };
        
        static void report(Severity severity, const std::string& message);
        static void setLogFile(const std::string& filename);
        static void enableConsoleOutput(bool enable);
    };
}
```

---

## Version Compatibility

### Version Macros
```cpp
#define TYLER_AUDIO_VERSION_MAJOR 1
#define TYLER_AUDIO_VERSION_MINOR 0
#define TYLER_AUDIO_VERSION_PATCH 0

#define TYLER_AUDIO_VERSION \
    ((TYLER_AUDIO_VERSION_MAJOR << 16) | \
     (TYLER_AUDIO_VERSION_MINOR << 8) | \
     TYLER_AUDIO_VERSION_PATCH)

// Check minimum required version
#define TYLER_AUDIO_REQUIRE_VERSION(major, minor, patch) \
    static_assert(TYLER_AUDIO_VERSION >= ((major << 16) | (minor << 8) | patch), \
                  "Tyler Audio Framework version too old")
```

### Compatibility Functions
```cpp
namespace TylerAudio {
    /** Get current framework version */
    struct Version {
        int major, minor, patch;
        std::string toString() const;
        bool isCompatibleWith(const Version& other) const;
    };
    
    Version getCurrentVersion();
    bool isVersionCompatible(const Version& required);
}
```

---

## Best Practices

### Recommended Patterns
```cpp
// 1. Parameter Management Pattern
class MyProcessor : public juce::AudioProcessor {
    MyProcessor() 
        : parameters(*this, nullptr, "MyProcessor", createParameterLayout())
        , gainParam(parameters.getRawParameterValue("gain"))
    {}
    
private:
    juce::AudioProcessorValueTreeState parameters;
    std::atomic<float>* gainParam;
    TylerAudio::SmoothingFilter gainSmoother;
};

// 2. DSP Component Pattern  
class MyDSPComponent {
public:
    void prepare(const juce::dsp::ProcessSpec& spec) { /* setup */ }
    void reset() { /* clear state */ }
    void process(const juce::dsp::ProcessContextReplacing<float>& context) { /* process */ }
};

// 3. Testing Pattern
TEST_CASE("My Component Test") {
    MyDSPComponent component;
    component.prepare({48000.0, 512, 2});
    
    // Create test data
    auto testBuffer = TestUtilities::TestFixtures::createTestBuffer(2, 512);
    
    // Process and validate
    juce::dsp::AudioBlock<float> block(*testBuffer);
    component.process(juce::dsp::ProcessContextReplacing<float>(block));
    
    // Assert results
    REQUIRE(TestUtilities::AudioQualityValidator::validateAudioIntegrity(*testBuffer));
}
```

This API reference provides comprehensive documentation for all major components of the Tyler Audio Framework. For usage examples and detailed tutorials, refer to the Development Guide and Learning Path documentation.