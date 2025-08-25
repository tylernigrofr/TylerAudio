#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <memory>
#include <vector>
#include <chrono>
#include <functional>
#include <thread>
#include <atomic>

namespace TylerAudio {
namespace PerformanceTestFramework {

//==============================================================================
/** Advanced CPU usage measurement */
class CPUProfiler {
public:
    struct ProfileResult {
        double averageCPUPercent;
        double peakCPUPercent;
        double minimumCPUPercent;
        std::vector<double> cpuHistory;
        double processingTimeMs;
        double realTimeRatio; // processing_time / available_time
        bool realtimeSafe;
        size_t numSamples;
    };
    
    /** Profile CPU usage of audio processing function */
    static ProfileResult profileAudioProcessing(
        std::function<void(juce::AudioBuffer<float>&, juce::MidiBuffer&)> processor,
        double sampleRate = 48000.0,
        int blockSize = 512,
        int durationSeconds = 10,
        bool includeHistory = true);
    
    /** Profile plugin CPU usage with parameter automation */
    static ProfileResult profileWithParameterAutomation(
        juce::AudioProcessor& processor,
        const std::vector<int>& parameterIndices,
        double sampleRate = 48000.0,
        int blockSize = 512,
        int durationSeconds = 10);
    
    /** Stress test with multiple instances */
    static std::vector<ProfileResult> profileMultipleInstances(
        std::function<std::unique_ptr<juce::AudioProcessor>()> createProcessor,
        int numInstances,
        double sampleRate = 48000.0,
        int blockSize = 512,
        int durationSeconds = 5);
    
    /** Profile at different sample rates */
    static std::vector<std::pair<double, ProfileResult>> profileAcrossSampleRates(
        juce::AudioProcessor& processor,
        const std::vector<double>& sampleRates = {44100.0, 48000.0, 88200.0, 96000.0, 192000.0},
        int blockSize = 512,
        int durationSeconds = 5);
    
    /** Profile with different buffer sizes */
    static std::vector<std::pair<int, ProfileResult>> profileAcrossBufferSizes(
        juce::AudioProcessor& processor,
        const std::vector<int>& bufferSizes = {32, 64, 128, 256, 512, 1024, 2048},
        double sampleRate = 48000.0,
        int durationSeconds = 3);
    
    /** Validate real-time performance compliance */
    static bool validateRealtimeCompliance(const ProfileResult& result, double maxCPUPercent = 50.0);
    
    /** Compare performance between two processors */
    static double compareProcessors(juce::AudioProcessor& processor1, 
                                   juce::AudioProcessor& processor2,
                                   double sampleRate = 48000.0,
                                   int blockSize = 512,
                                   int durationSeconds = 5);
};

//==============================================================================
/** Memory usage analysis */
class MemoryProfiler {
public:
    struct MemorySnapshot {
        size_t totalAllocatedBytes;
        size_t peakAllocatedBytes;
        size_t currentUsageBytes;
        std::vector<size_t> allocationSizes;
        int numAllocations;
        int numDeallocations;
        bool hasLeaks;
        double allocationRate; // allocations per second
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
    
    /** Profile memory usage with parameter changes */
    static MemorySnapshot profileWithParameterChanges(
        juce::AudioProcessor& processor,
        int numParameterChanges = 1000,
        double sampleRate = 48000.0);
    
    /** Analyze memory fragmentation */
    static double analyzeFragmentation(
        std::function<void()> processor,
        int numIterations = 1000);
    
    /** Profile preset loading memory impact */
    static std::vector<MemorySnapshot> profilePresetLoading(
        juce::AudioProcessor& processor,
        const std::vector<juce::MemoryBlock>& presets);
    
    /** Monitor real-time memory allocation violations */
    static bool monitorRealtimeViolations(
        juce::AudioProcessor& processor,
        double sampleRate = 48000.0,
        int blockSize = 512,
        int durationSeconds = 10);
};

//==============================================================================
/** Threading and concurrency analysis */
class ConcurrencyProfiler {
public:
    struct ThreadingAnalysis {
        bool threadSafe;
        std::vector<std::string> potentialRaceConditions;
        std::vector<std::string> lockContentions;
        double lockWaitTimeMs;
        int numThreadingViolations;
        bool realtimeSafe;
    };
    
    /** Analyze thread safety of audio processor */
    static ThreadingAnalysis analyzeThreadSafety(
        juce::AudioProcessor& processor,
        int numThreads = 4,
        int durationSeconds = 10);
    
    /** Test concurrent parameter access */
    static bool testConcurrentParameterAccess(
        juce::AudioProcessor& processor,
        int numThreads = 8,
        int numIterations = 10000);
    
    /** Profile lock contention in audio processing */
    static ThreadingAnalysis profileLockContention(
        juce::AudioProcessor& processor,
        double sampleRate = 48000.0,
        int blockSize = 512,
        int durationSeconds = 10);
    
    /** Test real-time thread violations */
    static std::vector<std::string> detectRealtimeViolations(
        juce::AudioProcessor& processor,
        double sampleRate = 48000.0,
        int blockSize = 512,
        int durationSeconds = 10);
    
    /** Stress test with DAW-like threading scenarios */
    static bool stressTestDAWScenario(
        juce::AudioProcessor& processor,
        int numAudioThreads = 2,
        int numUIThreads = 1,
        int durationSeconds = 30);
};

//==============================================================================
/** Audio quality performance analysis */
class AudioQualityProfiler {
public:
    struct QualityMetrics {
        double latencyMs;
        double phaseTolerance;
        double frequencyResponseDeviation;
        double thdPlusNoise;
        double signalToNoiseRatio;
        double dynamicRange;
        std::vector<double> harmonicDistortion;
        bool hasArtifacts;
        double audioQualityScore; // 0-100
    };
    
    /** Comprehensive audio quality analysis */
    static QualityMetrics analyzeAudioQuality(
        juce::AudioProcessor& processor,
        double sampleRate = 48000.0,
        const std::vector<float>& testFrequencies = {100.0f, 1000.0f, 5000.0f, 10000.0f});
    
    /** Measure processing latency */
    static double measureLatency(
        juce::AudioProcessor& processor,
        double sampleRate = 48000.0,
        int blockSize = 512);
    
    /** Analyze frequency response accuracy */
    static std::vector<std::pair<float, float>> analyzeFrequencyResponse(
        juce::AudioProcessor& processor,
        double sampleRate = 48000.0,
        float startFreq = 20.0f,
        float endFreq = 20000.0f,
        int numPoints = 100);
    
    /** Test dynamic range preservation */
    static double testDynamicRange(
        juce::AudioProcessor& processor,
        double sampleRate = 48000.0);
    
    /** Detect audio artifacts */
    static std::vector<std::string> detectAudioArtifacts(
        juce::AudioProcessor& processor,
        const juce::AudioBuffer<float>& testSignal,
        double sampleRate = 48000.0);
    
    /** Compare audio quality before/after processing */
    static QualityMetrics compareAudioQuality(
        const juce::AudioBuffer<float>& input,
        const juce::AudioBuffer<float>& output,
        double sampleRate = 48000.0);
};

//==============================================================================
/** Scalability testing */
class ScalabilityTester {
public:
    struct ScalabilityResults {
        std::vector<std::pair<int, double>> instanceVsCPU;
        std::vector<std::pair<double, double>> sampleRateVsCPU;
        std::vector<std::pair<int, double>> bufferSizeVsCPU;
        int maxRealtimeInstances;
        double cpuScalingFactor;
        bool linearScaling;
        std::string bottleneckAnalysis;
    };
    
    /** Test performance scaling with multiple instances */
    static ScalabilityResults testInstanceScaling(
        std::function<std::unique_ptr<juce::AudioProcessor>()> createProcessor,
        int maxInstances = 50,
        double sampleRate = 48000.0,
        int blockSize = 512);
    
    /** Test performance across different system configurations */
    static std::vector<ScalabilityResults> testAcrossConfigurations(
        std::function<std::unique_ptr<juce::AudioProcessor>()> createProcessor,
        const std::vector<std::pair<double, int>>& configurations); // sampleRate, blockSize pairs
    
    /** Find maximum real-time instance count */
    static int findMaxRealtimeInstances(
        std::function<std::unique_ptr<juce::AudioProcessor>()> createProcessor,
        double maxCPUPercent = 80.0,
        double sampleRate = 48000.0,
        int blockSize = 512);
    
    /** Benchmark against reference implementations */
    static std::vector<std::pair<std::string, double>> benchmarkAgainstReferences(
        juce::AudioProcessor& testProcessor,
        const std::vector<std::pair<std::string, std::unique_ptr<juce::AudioProcessor>>>& references);
    
    /** Project performance on different hardware */
    static std::vector<std::pair<std::string, ScalabilityResults>> projectHardwarePerformance(
        juce::AudioProcessor& processor,
        const std::vector<std::pair<std::string, double>>& hardwareSpecs); // name, relativePerformance
};

//==============================================================================
/** Regression testing framework */
class RegressionTester {
public:
    struct RegressionResult {
        bool passed;
        std::string testName;
        double baselineValue;
        double currentValue;
        double changePercent;
        std::string metric;
        bool significantChange;
    };
    
    /** Run comprehensive performance regression tests */
    static std::vector<RegressionResult> runPerformanceRegression(
        juce::AudioProcessor& processor,
        const std::string& baselineFilePath,
        double tolerancePercent = 10.0);
    
    /** Create performance baseline */
    static bool createPerformanceBaseline(
        juce::AudioProcessor& processor,
        const std::string& baselineFilePath);
    
    /** Compare against historical performance data */
    static std::vector<RegressionResult> compareWithHistory(
        juce::AudioProcessor& processor,
        const std::string& historyFilePath,
        int numHistoricalPoints = 10);
    
    /** Auto-update baseline if improvements detected */
    static bool autoUpdateBaseline(
        const std::vector<RegressionResult>& results,
        const std::string& baselineFilePath,
        double improvementThreshold = 5.0);
    
    /** Generate regression test report */
    static void generateRegressionReport(
        const std::vector<RegressionResult>& results,
        const std::string& reportPath);
};

//==============================================================================
/** Comprehensive performance test suite */
class PerformanceTestSuite {
public:
    struct TestSuiteResults {
        CPUProfiler::ProfileResult cpuProfile;
        MemoryProfiler::MemorySnapshot memoryProfile;
        ConcurrencyProfiler::ThreadingAnalysis threadingAnalysis;
        AudioQualityProfiler::QualityMetrics audioQuality;
        ScalabilityTester::ScalabilityResults scalability;
        std::vector<RegressionTester::RegressionResult> regressionResults;
        bool overallPassed;
        double performanceScore; // 0-100
        std::vector<std::string> recommendations;
    };
    
    /** Run complete performance test suite */
    static TestSuiteResults runComprehensiveTests(
        juce::AudioProcessor& processor,
        const std::string& baselineDirectory = "");
    
    /** Quick performance validation */
    static bool quickPerformanceCheck(
        juce::AudioProcessor& processor,
        double maxCPUPercent = 50.0,
        double sampleRate = 48000.0,
        int blockSize = 512);
    
    /** Generate performance optimization recommendations */
    static std::vector<std::string> generateOptimizationRecommendations(
        const TestSuiteResults& results);
    
    /** Export results to various formats */
    static void exportResults(
        const TestSuiteResults& results,
        const std::string& outputDirectory,
        const std::vector<std::string>& formats = {"json", "csv", "html"});
    
    /** Compare two processors comprehensively */
    static TestSuiteResults compareProcessors(
        juce::AudioProcessor& processor1,
        juce::AudioProcessor& processor2,
        const std::string& comparison_name = "Processor Comparison");
    
    /** Continuous performance monitoring */
    static void startContinuousMonitoring(
        juce::AudioProcessor& processor,
        const std::string& outputDirectory,
        int monitoringIntervalMinutes = 30);
    
    static void stopContinuousMonitoring();
};

//==============================================================================
/** Performance utilities and helpers */
class PerformanceUtils {
public:
    /** Get system performance capabilities */
    static std::map<std::string, double> getSystemCapabilities();
    
    /** Warm up system for accurate measurements */
    static void warmupSystem(int durationSeconds = 5);
    
    /** Set thread priority for accurate measurements */
    static void setHighPriorityMode(bool enable);
    
    /** Generate realistic test audio content */
    static std::unique_ptr<juce::AudioBuffer<float>> generateTestAudio(
        int numChannels,
        int numSamples,
        double sampleRate,
        const std::string& contentType = "music");
    
    /** Calculate processing overhead */
    static double calculateOverhead(
        double processingTime,
        double sampleRate,
        int blockSize);
    
    /** Estimate memory bandwidth usage */
    static double estimateMemoryBandwidth(
        int numChannels,
        int blockSize,
        double sampleRate,
        int bytesPerSample = 4);
    
    /** Get current system resource usage */
    static std::map<std::string, double> getCurrentSystemUsage();
    
    /** Log performance metrics */
    static void logPerformanceMetrics(
        const std::string& testName,
        const std::map<std::string, double>& metrics,
        const std::string& logFile = "performance.log");
};

} // namespace PerformanceTestFramework
} // namespace TylerAudio