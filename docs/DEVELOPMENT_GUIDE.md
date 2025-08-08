# Tyler Audio Framework Development Guide

## Overview
The Tyler Audio Framework is a professional-grade JUCE-based audio plugin development system designed for efficiency, quality, and maintainability. This guide provides comprehensive documentation for developers working with the framework.

## Table of Contents
1. [Getting Started](#getting-started)
2. [Framework Architecture](#framework-architecture)
3. [Development Workflow](#development-workflow)
4. [Testing Systems](#testing-systems)
5. [Performance Optimization](#performance-optimization)
6. [Code Quality Standards](#code-quality-standards)
7. [Deployment and CI/CD](#deployment-and-cicd)
8. [Best Practices](#best-practices)
9. [Troubleshooting](#troubleshooting)
10. [Contributing](#contributing)

## Getting Started

### Prerequisites
- **Build System**: CMake 3.25+
- **C++ Compiler**: 
  - Windows: MSVC 2019+ or Clang 12+
  - macOS: Xcode 12+ or Clang 12+
  - Linux: GCC 10+ or Clang 12+
- **C++ Standard**: C++23
- **Audio Framework**: JUCE 8.0.3 (automatically fetched)
- **Testing Framework**: Catch2 v3 (automatically integrated)

### Quick Setup
```bash
# Clone the framework
git clone <your-repo-url> TylerAudio
cd TylerAudio

# Initialize build system
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release

# Build framework and examples
cmake --build build --config Release

# Run tests to verify setup
cd build && ctest --output-on-failure
```

### Creating Your First Plugin
```bash
# Create new plugin using the enhanced creation system
./scripts/create-plugin.sh MyFirstPlugin effect

# Follow the four-prompt development system
# 1. Research: plugins/MyFirstPlugin/docs/claude/1_juce_research.md
# 2. Specification: plugins/MyFirstPlugin/docs/claude/2_juce_specification.md
# 3. Checklist: plugins/MyFirstPlugin/docs/claude/3_juce_checklist.md
# 4. Build: plugins/MyFirstPlugin/docs/claude/4_juce_build.md

# Build your plugin
cmake --build build --target MyFirstPlugin

# Deploy for testing
./scripts/deploy-plugins.sh
```

## Framework Architecture

### Core Components

#### 1. Shared Libraries
- **TylerAudioCommon.h**: Core utilities, parameter smoothing, realtime-safe components
- **TestUtilities.{h,cpp}**: Comprehensive audio testing framework
- **UITestUtilities.{h,cpp}**: UI component testing and validation
- **PerformanceTestFramework.h**: Advanced performance profiling
- **IntegrationTestFramework.h**: DAW compatibility and integration testing

#### 2. Build System
- **Root CMakeLists.txt**: Main build configuration with JUCE integration
- **plugins/CMakeLists.txt**: Plugin discovery and build orchestration
- **Individual Plugin CMake**: Per-plugin configuration with shared standards

#### 3. Development Tools
- **scripts/create-plugin.sh**: Enhanced plugin scaffolding with full environment setup
- **scripts/deploy-plugins.sh**: Automated plugin deployment to system folders
- **scripts/run-smoke-tests.sh**: Comprehensive plugin validation
- **.clang-format**: Consistent code formatting
- **.clang-tidy**: Static analysis and code quality

#### 4. Documentation System
- **Four-Prompt Development Workflow**: Research → Specification → Checklist → Build
- **Comprehensive Research Templates**: Algorithm analysis, implementation patterns
- **Structured Development Process**: TDD enforcement with atomic tasks

### Plugin Architecture Pattern

```cpp
// Standard Tyler Audio Plugin Structure
class MyPluginAudioProcessor : public juce::AudioProcessor {
public:
    // Constructor with parameter setup
    MyPluginAudioProcessor();
    
    // Core AudioProcessor interface
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void releaseResources() override;
    
    // Parameter management
    juce::AudioProcessorValueTreeState& getParameters() { return parameters; }
    
    // State management
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;
    
    // Editor creation
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

private:
    // Parameter system with atomic access
    juce::AudioProcessorValueTreeState parameters;
    std::atomic<float>* parameterPointers[NUM_PARAMETERS];
    
    // DSP components (realtime-safe)
    // Example: DSPComponent dspComponent;
    
    // Parameter smoothing
    TylerAudio::SmoothingFilter parameterSmoothers[NUM_PARAMETERS];
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MyPluginAudioProcessor)
};
```

## Development Workflow

### The Four-Prompt System
The Tyler Audio Framework uses a structured four-prompt development system designed to ensure comprehensive, high-quality plugin development:

#### 1. Research Phase (`1_juce_research.md`)
- **Context7 MCP Integration**: Query up-to-date JUCE documentation
- **Algorithm Research**: Analyze existing implementations and academic sources
- **Performance Analysis**: Study optimization patterns and benchmarks
- **Technical Specifications**: Define requirements and constraints

#### 2. Specification Phase (`2_juce_specification.md`)
- **Functional Requirements**: Define what the plugin does
- **Technical Architecture**: Specify DSP pipeline and components
- **Performance Targets**: Set CPU, memory, and quality benchmarks
- **Risk Assessment**: Identify potential challenges and mitigation strategies

#### 3. Checklist Phase (`3_juce_checklist.md`)
- **Task Breakdown**: Atomic, testable tasks (<4 hours each)
- **TDD Enforcement**: Test-first development requirements
- **Dependencies**: Clear task ordering and prerequisites
- **Acceptance Criteria**: Specific, measurable success conditions

#### 4. Build Phase (`4_juce_build.md`)
- **Systematic Execution**: Follow checklist with strict TDD
- **Continuous Validation**: Test suite execution after each task
- **Performance Monitoring**: Real-time performance tracking
- **Quality Gates**: Automated validation before task completion

### Development Standards

#### Test-Driven Development (TDD)
```cpp
// Example TDD workflow for DSP component

// 1. Write failing test first
TEST_CASE("Reverb produces expected decay", "[Reverb][DSP]") {
    ReverbProcessor reverb;
    reverb.prepare({48000.0, 512, 2});
    
    // Generate impulse
    juce::AudioBuffer<float> buffer(2, 1024);
    buffer.clear();
    buffer.setSample(0, 0, 1.0f);
    
    // Process and measure decay
    reverb.setDecayTime(2.0f); // 2 seconds
    juce::MidiBuffer midi;
    reverb.processBlock(buffer, midi);
    
    // Verify decay characteristics
    float decayAt1Second = /* measure decay */;
    REQUIRE(decayAt1Second == Catch::Approx(0.5f).margin(0.1f));
}

// 2. Implement minimum code to pass
class ReverbProcessor {
    void setDecayTime(float seconds) { /* implementation */ }
    // ... rest of implementation
};

// 3. Refactor while keeping tests green
```

#### Parameter Management Pattern
```cpp
// Atomic parameter access for realtime safety
class ParameterManager {
    // In constructor
    parameters(*this, nullptr, juce::Identifier("MyPlugin"), createParameterLayout()),
    gainParam(parameters.getRawParameterValue("gain")),
    frequencyParam(parameters.getRawParameterValue("frequency"))
    {
        // Initialize parameter smoothing
        gainSmoothing.setTargetValue(gainParam->load());
        frequencySmoothing.setTargetValue(frequencyParam->load());
    }
    
    // In processBlock
    void updateParameters() {
        gainSmoothing.setTargetValue(gainParam->load());
        frequencySmoothing.setTargetValue(frequencyParam->load());
    }
    
private:
    juce::AudioProcessorValueTreeState parameters;
    std::atomic<float>* gainParam;
    std::atomic<float>* frequencyParam;
    TylerAudio::SmoothingFilter gainSmoothing, frequencySmoothing;
};
```

## Testing Systems

### Comprehensive Test Coverage

#### 1. Unit Tests
- **Component Isolation**: Test individual DSP components
- **Parameter Validation**: Boundary conditions and edge cases
- **Algorithm Correctness**: Mathematical accuracy verification

#### 2. Integration Tests
- **Signal Chain Testing**: Complete audio processing pipeline
- **Parameter Interaction**: Cross-component behavior validation
- **Format Compliance**: VST3/AU specification adherence

#### 3. Performance Tests
- **CPU Profiling**: Real-time performance validation (<1% target)
- **Memory Analysis**: Leak detection and allocation tracking
- **Scalability Testing**: Multiple instance performance

#### 4. UI Tests
- **Component Behavior**: User interface interaction validation
- **Visual Regression**: Automated screenshot comparison
- **Accessibility**: Screen reader and keyboard navigation support

### Running Tests
```bash
# Run all tests
cd build && ctest --output-on-failure

# Run specific test categories
ctest -R "smoke"          # Smoke tests
ctest -R "performance"    # Performance benchmarks
ctest -R "MyPlugin"       # Plugin-specific tests

# Run with detailed output
ctest --verbose

# Generate test reports
ctest --output-junit results.xml
```

### Test Utilities Usage
```cpp
// Example comprehensive test using framework utilities
#include "shared/TestUtilities.h"

TEST_CASE("Comprehensive Audio Quality Test") {
    MyPluginProcessor processor;
    processor.prepareToPlay(48000.0, 512);
    
    // Generate test signal
    auto testBuffer = TestUtilities::TestFixtures::createTestBuffer(
        2, 1024, TestUtilities::SignalGenerator::SignalType::Sine, 1000.0f, 48000.0);
    
    auto outputBuffer = *testBuffer;
    juce::MidiBuffer midiBuffer;
    processor.processBlock(outputBuffer, midiBuffer);
    
    // Comprehensive audio analysis
    auto qualityMetrics = TestUtilities::AudioQualityValidator::analyzeAudioQuality(
        *testBuffer, outputBuffer, 48000.0);
    
    // Validate results
    REQUIRE(qualityMetrics.thdPlusN < 0.1f);
    REQUIRE(qualityMetrics.snr > 80.0f);
    REQUIRE_FALSE(qualityMetrics.hasAudioDropouts);
    REQUIRE_FALSE(qualityMetrics.hasClipping);
    
    // Performance validation
    auto perfResult = TestUtilities::PerformanceMeter::measureProcessingTime(
        [&]() { processor.processBlock(outputBuffer, midiBuffer); });
    
    REQUIRE(perfResult.averageTimeMs < 1.0); // <1ms for 512 samples at 48kHz
}
```

## Performance Optimization

### CPU Optimization Strategies

#### 1. Algorithmic Efficiency
- **SIMD Instructions**: Utilize vectorized operations where possible
- **Lookup Tables**: Pre-compute expensive mathematical operations
- **Block Processing**: Process audio in chunks rather than sample-by-sample
- **Branch Prediction**: Minimize conditional statements in hot paths

#### 2. Memory Management
- **Pre-allocation**: All buffers allocated in `prepareToPlay()`
- **Cache Efficiency**: Structure data for optimal cache usage
- **Alignment**: Ensure proper memory alignment for SIMD operations

#### 3. Threading Considerations
- **Lock-free Design**: Atomic operations for parameter updates
- **Thread Affinity**: Audio thread isolation from UI thread
- **Realtime Safety**: No memory allocation in audio callback

### Performance Validation
```cpp
// Automated performance regression testing
TEST_CASE("Performance Regression Test") {
    MyPluginProcessor processor;
    
    // Baseline measurement
    auto baseline = TestUtilities::PerformanceMeter::measureProcessingTime(
        [&]() { /* baseline implementation */ });
    
    // Current measurement
    auto current = TestUtilities::PerformanceMeter::measureProcessingTime(
        [&]() { processor.processBlock(buffer, midi); });
    
    // Validate no significant regression (>10% slower)
    REQUIRE(current.averageTimeMs < baseline.averageTimeMs * 1.1);
}
```

## Code Quality Standards

### Static Analysis Integration
- **.clang-tidy**: Comprehensive static analysis rules
- **.clang-format**: Consistent code formatting
- **CMake Integration**: Automatic formatting and analysis

### Code Review Checklist
- [ ] **Performance**: CPU usage <1% at 48kHz/512 samples
- [ ] **Realtime Safety**: No allocation in processBlock()
- [ ] **Thread Safety**: Atomic parameter access
- [ ] **Error Handling**: Graceful failure modes
- [ ] **Documentation**: Comprehensive inline comments
- [ ] **Testing**: 100% test coverage for new code
- [ ] **Compatibility**: Works across all supported platforms

### Documentation Standards
```cpp
/**
 * @brief Processes audio with tape saturation characteristics
 * 
 * Implements a tanh-based saturation algorithm with drive-dependent
 * high-frequency rolloff to simulate analog tape behavior.
 * 
 * @param buffer Audio buffer to process (modified in-place)
 * @param numSamples Number of samples to process
 * @param drive Saturation amount (0.0-1.0, 0.5 = moderate saturation)
 * 
 * @note This function is realtime-safe and performs no memory allocation
 * @warning High drive values (>0.8) may cause significant harmonic distortion
 * 
 * @performance Typical CPU usage: <0.1ms for 512 samples at 48kHz
 * @tested Comprehensive unit tests in TapeSaturationTest.cpp
 */
void processTapeSaturation(juce::AudioBuffer<float>& buffer, 
                          int numSamples, 
                          float drive) noexcept;
```

## Deployment and CI/CD

### Automated Build Pipeline
```yaml
# Example GitHub Actions workflow
name: Build and Test
on: [push, pull_request]

jobs:
  build-and-test:
    strategy:
      matrix:
        os: [windows-latest, macos-latest, ubuntu-latest]
        config: [Debug, Release]
    
    runs-on: ${{ matrix.os }}
    
    steps:
    - uses: actions/checkout@v3
    
    - name: Configure CMake
      run: cmake -B build -DCMAKE_BUILD_TYPE=${{ matrix.config }}
    
    - name: Build
      run: cmake --build build --config ${{ matrix.config }}
    
    - name: Test
      run: cd build && ctest --output-on-failure
    
    - name: Performance Regression Test
      run: ./scripts/performance-regression-test.sh
    
    - name: Deploy Artifacts
      if: success() && matrix.config == 'Release'
      run: ./scripts/package-plugins.sh
```

### Release Process
1. **Version Bump**: Update version numbers and changelog
2. **Comprehensive Testing**: Full test suite including performance regression
3. **Documentation Update**: Ensure all documentation is current
4. **Binary Creation**: Multi-platform plugin compilation
5. **Validation**: Final pluginval and DAW compatibility testing
6. **Release Notes**: Detailed changelog and upgrade instructions

## Best Practices

### DSP Implementation
- **Start Simple**: Begin with basic implementation, optimize later
- **Validate Continuously**: Test after each component implementation
- **Profile Early**: Identify performance bottlenecks during development
- **Consider Edge Cases**: Handle extreme parameter values gracefully

### UI Development
- **Responsive Design**: Support various screen sizes and DPI scales
- **Accessibility First**: Implement screen reader support from the beginning
- **Performance Conscious**: Minimize redraws and heavy operations
- **User-Centered**: Prioritize workflow efficiency over visual complexity

### Parameter Design
- **Meaningful Ranges**: Use musically relevant parameter ranges
- **Default Values**: Choose sensible defaults for immediate usability
- **Smooth Automation**: Implement proper parameter smoothing
- **Clear Naming**: Use intuitive parameter names and units

## Troubleshooting

### Common Build Issues

#### CMake Configuration Problems
```bash
# Clear CMake cache
rm -rf build
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release

# Verbose build for debugging
cmake --build build --verbose

# Check JUCE version
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release -DJUCE_VERSION_CHECK=ON
```

#### Plugin Loading Issues
```bash
# Verify plugin format
./scripts/validate-plugin-format.sh plugins/MyPlugin/build/MyPlugin.vst3

# Check dependencies
# Windows: dumpbin /dependents MyPlugin.dll
# macOS: otool -L MyPlugin.component/Contents/MacOS/MyPlugin
# Linux: ldd MyPlugin.so

# Test in isolated environment
./scripts/test-plugin-isolation.sh MyPlugin
```

### Performance Issues
- **CPU Spikes**: Check for memory allocation in processBlock()
- **Audio Dropouts**: Verify realtime-safe code paths
- **Memory Leaks**: Use memory profiling tools and automated tests
- **Thread Contention**: Review atomic operations and lock usage

### Testing Failures
- **Flaky Tests**: Check for timing dependencies and race conditions
- **Platform-Specific Issues**: Use CI logs to identify platform differences
- **Performance Regressions**: Compare with baseline measurements

## Contributing

### Development Setup
1. Fork the repository
2. Create feature branch: `git checkout -b feature/my-enhancement`
3. Follow the four-prompt development system
4. Ensure 100% test coverage for new code
5. Run full test suite including performance tests
6. Update documentation for new features
7. Submit pull request with detailed description

### Code Contributions
- **Follow TDD**: Write tests before implementation
- **Maintain Performance**: Ensure no regression in benchmarks
- **Document Thoroughly**: Update both code comments and user documentation
- **Test Comprehensively**: Include unit, integration, and performance tests

### Documentation Contributions
- **Keep Current**: Update documentation with code changes
- **Add Examples**: Include practical usage examples
- **Improve Clarity**: Enhance existing explanations
- **Test Instructions**: Verify all documented procedures work correctly

## Additional Resources

### Learning Materials
- **JUCE Documentation**: [juce.com/learn](https://juce.com/learn)
- **DSP Theory**: Recommended books and papers in `docs/references/`
- **Audio Programming**: Best practices and patterns
- **Performance Optimization**: CPU and memory optimization techniques

### Community Resources
- **JUCE Forum**: [forum.juce.com](https://forum.juce.com)
- **Audio Programming Discord**: Community discussions
- **KVR Audio**: Plugin development discussions
- **GitHub Issues**: Framework-specific questions and bug reports

### Tools and Utilities
- **Plugin Validators**: pluginval, PACE iLok, etc.
- **Profiling Tools**: Intel VTune, Apple Instruments, Valgrind
- **DAW Testing**: Recommended DAWs for compatibility testing
- **Audio Analysis**: Tools for frequency response and distortion analysis

---

*This development guide is continuously updated to reflect the latest framework capabilities and best practices. For questions or clarifications, please refer to the GitHub issues or community resources.*