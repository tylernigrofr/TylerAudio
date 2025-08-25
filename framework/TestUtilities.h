#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <memory>
#include <vector>
#include <functional>
#include <chrono>
#include <complex>

namespace TylerAudio {
namespace TestUtilities {

//==============================================================================
/** Signal generation utilities for testing */
class SignalGenerator {
public:
    enum class SignalType {
        Silence,
        Sine,
        Cosine,
        WhiteNoise,
        PinkNoise,
        Impulse,
        Step,
        Chirp,
        Square,
        Sawtooth,
        Triangle
    };
    
    /** Generate test signal into buffer */
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
                                 
    /** Generate pink noise with specific characteristics */
    static void generatePinkNoise(juce::AudioBuffer<float>& buffer,
                                 float amplitude = 1.0f,
                                 int seed = 12345);
                                 
    /** Generate frequency sweep (chirp) */
    static void generateChirp(juce::AudioBuffer<float>& buffer,
                             float startFreq = 20.0f,
                             float endFreq = 20000.0f,
                             double sampleRate = 48000.0,
                             float amplitude = 1.0f);
};

//==============================================================================
/** Audio analysis utilities */
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
                              
    /** Calculate frequency response using swept sine */
    static std::vector<std::complex<float>> calculateFrequencyResponse(
        std::function<void(juce::AudioBuffer<float>&)> processor,
        double sampleRate = 48000.0,
        int fftSize = 2048,
        float startFreq = 20.0f,
        float endFreq = 20000.0f);
        
    /** Calculate phase response */
    static std::vector<float> calculatePhaseResponse(
        std::function<void(juce::AudioBuffer<float>&)> processor,
        const std::vector<float>& frequencies,
        double sampleRate = 48000.0);
        
    /** Calculate signal-to-noise ratio */
    static float calculateSNR(const juce::AudioBuffer<float>& signal,
                             const juce::AudioBuffer<float>& noise);
                             
    /** Calculate cross-correlation between two signals */
    static std::vector<float> crossCorrelate(const juce::AudioBuffer<float>& sig1,
                                           const juce::AudioBuffer<float>& sig2);
                                           
    /** Find delay between two signals using cross-correlation */
    static int findDelay(const juce::AudioBuffer<float>& reference,
                        const juce::AudioBuffer<float>& delayed);
};

//==============================================================================
/** Performance measurement utilities */
class PerformanceMeter {
public:
    struct MeasurementResults {
        double averageTimeMs;
        double minTimeMs;
        double maxTimeMs;
        double stdDeviationMs;
        size_t numSamples;
        double cpuUsagePercent;
        size_t memoryUsageBytes;
    };
    
    /** Measure processing time with multiple iterations */
    static MeasurementResults measureProcessingTime(
        std::function<void()> processor,
        int numIterations = 1000,
        int warmupIterations = 100);
        
    /** Measure memory usage during processing */
    static size_t measureMemoryUsage(
        std::function<void()> processor,
        int numIterations = 1000);
        
    /** Measure CPU usage percentage */
    static double measureCPUUsage(
        std::function<void()> processor,
        int durationSeconds = 10,
        double sampleRate = 48000.0,
        int blockSize = 512);
        
    /** Performance regression test */
    static bool verifyPerformanceRegression(
        const MeasurementResults& baseline,
        const MeasurementResults& current,
        double tolerancePercent = 10.0);
        
    /** Real-time performance validation */
    static bool validateRealtimePerformance(
        std::function<void()> processor,
        double sampleRate = 48000.0,
        int blockSize = 512,
        double maxCPUPercent = 50.0);
};

//==============================================================================
/** Audio quality validation */
class AudioQualityValidator {
public:
    struct QualityMetrics {
        float dynamicRange;    // dB
        float thdPlusN;        // %
        float snr;             // dB
        float frequencyResponse; // Max deviation in dB
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
                           
    /** Validate frequency response within tolerance */
    static bool validateFrequencyResponse(
        std::function<void(juce::AudioBuffer<float>&)> processor,
        const std::vector<std::pair<float, float>>& expectedResponse, // freq, gain_dB
        double sampleRate = 48000.0,
        float tolerancedB = 0.5f);
        
    /** Validate phase response linearity */
    static bool validatePhaseLinearity(
        std::function<void(juce::AudioBuffer<float>&)> processor,
        double sampleRate = 48000.0,
        float toleranceDegrees = 5.0f);
};

//==============================================================================
/** Plugin-specific testing utilities */
class PluginTester {
public:
    /** Test plugin with various buffer sizes */
    static bool testBufferSizes(juce::AudioProcessor& processor,
                               const std::vector<int>& bufferSizes = {32, 64, 128, 256, 512, 1024, 2048});
                               
    /** Test plugin with various sample rates */
    static bool testSampleRates(juce::AudioProcessor& processor,
                               const std::vector<double>& sampleRates = {44100.0, 48000.0, 88200.0, 96000.0, 192000.0});
                               
    /** Test parameter automation with ramps */
    static bool testParameterAutomation(juce::AudioProcessor& processor,
                                       int parameterIndex,
                                       float startValue,
                                       float endValue,
                                       int rampLengthSamples = 1024);
                                       
    /** Test plugin state save/restore */
    static bool testStateConsistency(juce::AudioProcessor& processor,
                                   int numIterations = 100);
                                   
    /** Test plugin with extreme parameter values */
    static bool testParameterExtremes(juce::AudioProcessor& processor);
    
    /** Test plugin threading safety */
    static bool testThreadSafety(juce::AudioProcessor& processor,
                                int numThreads = 4,
                                int durationSeconds = 10);
                                
    /** Test plugin with DAW-like scenarios */
    static bool testDAWScenarios(juce::AudioProcessor& processor);
    
    /** Validate plugin meets format specifications */
    static bool validatePluginFormat(juce::AudioProcessor& processor);
};

//==============================================================================
/** Statistical utilities for test data analysis */
class Statistics {
public:
    static double mean(const std::vector<double>& data);
    static double standardDeviation(const std::vector<double>& data);
    static double median(std::vector<double> data);
    static std::pair<double, double> minMax(const std::vector<double>& data);
    static double percentile(std::vector<double> data, double percentile);
    
    /** Perform t-test between two datasets */
    static bool tTest(const std::vector<double>& group1,
                     const std::vector<double>& group2,
                     double significanceLevel = 0.05);
                     
    /** Calculate correlation coefficient */
    static double correlation(const std::vector<double>& x,
                             const std::vector<double>& y);
};

//==============================================================================
/** Test result reporting and logging */
class TestReporter {
public:
    enum class LogLevel {
        Info,
        Warning,
        Error,
        Debug
    };
    
    /** Log test result with timestamp */
    static void log(LogLevel level, const juce::String& message);
    
    /** Generate comprehensive test report */
    static void generateReport(const juce::String& testName,
                              const std::vector<juce::String>& results,
                              const juce::String& outputPath);
                              
    /** Export performance data to CSV */
    static void exportPerformanceData(
        const std::vector<PerformanceMeter::MeasurementResults>& data,
        const juce::String& filename);
        
    /** Create performance comparison chart */
    static void createPerformanceChart(
        const std::vector<std::pair<juce::String, double>>& data,
        const juce::String& outputPath);
};

//==============================================================================
/** Utilities for creating test fixtures and mock objects */
class TestFixtures {
public:
    /** Create mock audio processor for testing */
    static std::unique_ptr<juce::AudioProcessor> createMockProcessor(
        int numInputs = 2,
        int numOutputs = 2,
        bool acceptsMidi = false,
        bool producesMidi = false);
        
    /** Create test audio buffer with specific characteristics */
    static std::unique_ptr<juce::AudioBuffer<float>> createTestBuffer(
        int numChannels = 2,
        int numSamples = 1024,
        SignalGenerator::SignalType signalType = SignalGenerator::SignalType::Sine,
        float frequency = 1000.0f,
        double sampleRate = 48000.0);
        
    /** Create parameter layout for testing */
    static juce::AudioProcessorValueTreeState::ParameterLayout createTestParameterLayout();
    
    /** Create MIDI buffer with test data */
    static juce::MidiBuffer createTestMidiBuffer(
        int numNotes = 5,
        int startTime = 0,
        int duration = 1000,
        int velocity = 100);
};

} // namespace TestUtilities
} // namespace TylerAudio