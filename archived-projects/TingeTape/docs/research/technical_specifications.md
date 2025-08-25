# TingeTape Technical Specifications

## Overview
Detailed technical specifications for the TingeTape tape emulation plugin, derived from research and optimized for musical performance with minimal system impact.

## Plugin Specifications

### 1. Format Support
**Supported Formats**:
- VST3 (Windows, macOS)
- Audio Unit (macOS)

**Unsupported Formats** (by design):
- VST2 (deprecated)
- AAX (licensing complexity)
- Standalone (unnecessary for tape emulation)

### 2. System Requirements
**Minimum Requirements**:
- **CPU**: Intel Core i5 / AMD equivalent (2015 or later)
- **RAM**: 4GB system memory
- **Plugin Memory**: <50KB per instance
- **Sample Rate**: 44.1kHz - 192kHz
- **Buffer Size**: 32 - 2048 samples

**Recommended Requirements**:
- **CPU**: Intel Core i7 / AMD equivalent (2018 or later)
- **RAM**: 8GB+ system memory
- **Sample Rate**: 48kHz or higher
- **Buffer Size**: 256-512 samples for optimal performance

### 3. Performance Targets
**CPU Usage**: <1% on recommended hardware (48kHz, 512 samples)
**Latency**: Minimal processing delay (~5ms from internal delay line)
**Memory Usage**: <50KB per instance
**Multiple Instances**: Support 50+ simultaneous instances

## DSP Architecture Specifications

### 1. Signal Processing Chain
**Processing Order** (fixed, optimized for tape characteristics):
```
Input → Low-Cut Filter → Tape Saturation → Tone Control → High-Cut Filter → Wow Engine → Output
```

**Rationale**:
- **Pre-filtering**: Removes subsonic content before saturation
- **Saturation**: Core tape character with harmonic generation
- **Tone Shaping**: Integrated frequency response control
- **Post-filtering**: Final frequency contouring
- **Wow Modulation**: Applied after all other processing for authentic tape behavior

### 2. WowEngine Specifications

#### LFO Characteristics:
- **Waveform**: Sine wave (authentic tape wow behavior)
- **Frequency**: Fixed at 0.5Hz (authentic tape speed)
- **Phase**: Synchronized across stereo channels
- **Stability**: No random drift (consistent behavior)

#### Delay Line Specifications:
- **Type**: `juce::dsp::DelayLine<float>` with linear interpolation
- **Maximum Delay**: 50ms (sufficient for all wow effects)
- **Base Delay**: 5ms (prevents zero-delay issues)
- **Modulation Range**: 0-45ms additional delay
- **Sample Rate Scaling**: Automatic scaling for all supported sample rates

#### Parameter Mapping:
- **Wow Depth**: 0-100% linear mapping
  - 0%: No modulation (bypass)
  - 25%: Subtle tape character (default)
  - 50%: Noticeable wow effect
  - 100%: Extreme vintage tape effect

**Technical Implementation**:
```cpp
float modulatedDelayMs = baseDelayMs + (lfoOutput * depthParam * maxModulationMs);
float delaySamples = modulatedDelayMs * sampleRate / 1000.0f;
```

### 3. Tape Saturation Specifications

#### Algorithm: Hyperbolic Tangent (tanh)
**Mathematical Model**:
```
output = tanh(input * driveGain) / tanh(driveGain)
```

#### Drive Parameter Mapping:
- **Range**: 0-100% (user interface)
- **Internal Mapping**: Linear to exponential gain conversion
- **Gain Calculation**: `driveGain = 1.0 + (driveParam * 0.01) * 9.0` (1x to 10x gain)
- **Output Compensation**: Divide by `tanh(driveGain)` for consistent level

#### Frequency Response Integration:
- **Pre-emphasis**: Subtle high-frequency boost before saturation
- **De-emphasis**: High-frequency rolloff after saturation  
- **Implementation**: `juce::dsp::FirstOrderTPTFilter<float>` at 15kHz

#### Harmonic Content Targets:
- **Low Drive (0-30%)**: Primarily 3rd harmonic
- **Medium Drive (30-70%)**: 3rd and 5th harmonics
- **High Drive (70-100%)**: Full harmonic spectrum with even harmonics

### 4. Resonant Filter Specifications

#### Low-Cut (High-Pass) Filter:
- **Type**: 2nd-order Butterworth high-pass
- **Implementation**: `juce::dsp::IIR::Filter<float>`
- **Frequency Range**: 20Hz - 200Hz
- **Default Setting**: 40Hz (removes subsonic rumble)
- **Resonance Range**: Q = 0.5 - 2.0
- **Default Resonance**: Q = 0.7 (natural response)

#### High-Cut (Low-Pass) Filter:
- **Type**: 2nd-order Butterworth low-pass  
- **Implementation**: `juce::dsp::IIR::Filter<float>`
- **Frequency Range**: 5kHz - 20kHz
- **Default Setting**: 15kHz (authentic tape rolloff)
- **Resonance Range**: Q = 0.5 - 2.0
- **Default Resonance**: Q = 0.7 (natural response)

#### Filter Update Strategy:
- **Coefficient Calculation**: On parameter change only
- **Thread Safety**: Atomic parameter updates, coefficient calculation on audio thread
- **Stability Validation**: Automatic coefficient validation and limiting

### 5. Tone Control Specifications

#### Implementation: Dual Shelving Filters
**Low Shelf Filter**:
- **Frequency**: 250Hz (fixed)
- **Gain Range**: -6dB to +6dB
- **Type**: `juce::dsp::IIR::Coefficients<float>::makeLowShelf`

**High Shelf Filter**:
- **Frequency**: 5kHz (fixed)
- **Gain Range**: +6dB to -6dB (inverse of low shelf)
- **Type**: `juce::dsp::IIR::Coefficients<float>::makeHighShelf`

#### Parameter Mapping:
- **Tone Control**: -100% to +100% (single parameter)
- **-100%**: Low shelf +6dB, High shelf -6dB (warm/dark)
- **0%**: Flat response (no tone change)
- **+100%**: Low shelf -6dB, High shelf +6dB (bright/crisp)

#### Mathematical Relationship:
```cpp
float toneParam = toneControl->load(); // -1.0 to +1.0
float lowShelfGain = toneParam * 6.0f;  // -6 to +6 dB
float highShelfGain = -toneParam * 6.0f; // +6 to -6 dB
```

## Parameter System Specifications

### 1. Parameter List and Ranges

| Parameter | ID | Range | Default | Unit | Automation |
|---|---|---|---|---|---|
| Wow Depth | `wowDepth` | 0-100 | 25 | % | Yes |
| Low-Cut Frequency | `lowCutFreq` | 20-200 | 40 | Hz | Yes |
| Low-Cut Resonance | `lowCutRes` | 0.5-2.0 | 0.7 | Q | Yes |
| Drive | `drive` | 0-100 | 50 | % | Yes |
| Tone | `tone` | -100 to +100 | 0 | % | Yes |
| High-Cut Frequency | `highCutFreq` | 5000-20000 | 15000 | Hz | Yes |
| High-Cut Resonance | `highCutRes` | 0.5-2.0 | 0.7 | Q | Yes |
| Bypass | `bypass` | Off/On | Off | Toggle | Yes |

### 2. Parameter Implementation Details

#### AudioProcessorValueTreeState Setup:
```cpp
parameters(*this, nullptr, juce::Identifier("TingeTape"), {
    std::make_unique<juce::AudioParameterFloat>(
        "wowDepth", "Wow Depth",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 25.0f,
        juce::String(), juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(value, 1) + "%"; }
    ),
    // ... additional parameters
});
```

#### Atomic Parameter Access:
```cpp
// In constructor
wowDepthParam = parameters.getRawParameterValue("wowDepth");
driveParam = parameters.getRawParameterValue("drive");

// In processBlock()
float currentWowDepth = wowDepthParam->load();
float currentDrive = driveParam->load();
```

#### Parameter Smoothing:
- **Smoothing Time**: 50ms for all parameters
- **Implementation**: First-order low-pass filter
- **Update Rate**: Per-sample smoothing in audio thread

### 3. Bypass Implementation
**True Bypass**: Direct input-to-output connection when bypassed
**Fade Time**: 10ms crossfade to prevent clicks
**State Preservation**: DSP state maintained during bypass for seamless re-engagement

## Memory Management Specifications

### 1. Buffer Allocation Strategy
**Preparation Phase** (`prepareToPlay`):
- All buffers allocated based on maximum expected block size
- Delay line sized for maximum delay at maximum sample rate
- Working buffers for dry/wet processing

**Processing Phase** (`processBlock`):
- **Zero Memory Allocation**: No `new`, `malloc`, or dynamic allocation
- **Stack-Only Variables**: All temporary variables on stack
- **Pre-allocated Access**: Only access to pre-allocated buffers

### 2. Memory Usage Breakdown

| Component | Memory Usage | Notes |
|---|---|---|---|
| DelayLine Buffer | ~38KB | 50ms at 192kHz stereo |
| Filter State Variables | 128 bytes | 4 biquads × 2 channels |
| Parameter Smoothing | 64 bytes | 8 parameters × 8 bytes |
| Working Buffers | 16KB | Temp processing space |
| **Total Maximum** | **~55KB** | **Per plugin instance** |

### 3. Thread Safety Architecture
**Audio Thread**:
- Read-only access to atomic parameter values
- All DSP processing
- Parameter smoothing updates
- No memory allocation

**Message Thread**:
- Parameter updates from UI
- State save/restore
- Preset loading
- Memory allocation allowed

## Performance Specifications

### 1. CPU Usage Targets

| Sample Rate | Block Size | Target CPU | Maximum CPU |
|---|---|---|---|
| 44.1kHz | 512 samples | <0.5% | <1.0% |
| 48kHz | 512 samples | <0.6% | <1.0% |
| 88.2kHz | 512 samples | <0.8% | <1.5% |
| 96kHz | 512 samples | <0.9% | <1.5% |
| 192kHz | 512 samples | <1.2% | <2.0% |

**Benchmark Hardware**: Intel Core i7-9700K @ 3.6GHz

### 2. Optimization Strategies
**DSP Optimizations**:
- Single-precision floating point only
- Minimize transcendental function calls
- Block-based parameter updates
- Efficient filter coefficient calculation

**Memory Optimizations**:
- Cache-friendly data access patterns
- Minimize memory bandwidth usage
- Pre-computed lookup tables where beneficial
- Stack allocation preference

**Threading Optimizations**:
- Lock-free parameter communication
- Atomic operations for thread safety
- Minimal message thread interaction

## Quality Assurance Specifications

### 1. Audio Quality Targets
**Dynamic Range**: >120dB (no processing artifacts)
**THD+N**: <0.1% at moderate settings, <1% at extreme settings
**Frequency Response**: ±0.1dB deviation from design targets
**Signal-to-Noise Ratio**: >100dB (limited by input signal)

### 2. Stability Requirements
**Parameter Sweeps**: Stable operation during continuous parameter automation
**Filter Stability**: No coefficient instability at any supported setting
**Denormal Handling**: Automatic protection against denormal numbers
**Extreme Settings**: Graceful behavior at all parameter extremes

### 3. Compatibility Requirements
**DAW Compatibility**:
- Reaper (Windows/macOS)
- Ableton Live (Windows/macOS)
- Logic Pro (macOS)
- Pro Tools (Windows/macOS)
- Studio One (Windows/macOS)
- FL Studio (Windows/macOS)

**Host Feature Support**:
- Parameter automation
- Preset save/restore
- Multi-channel processing (up to stereo)
- Sample rate changes
- Block size changes

## Testing and Validation Specifications

### 1. Automated Test Suite
**Unit Tests**:
- Individual DSP component validation
- Parameter range validation
- Performance regression tests
- Memory leak detection

**Integration Tests**:
- Full signal chain processing
- Multi-instance stability
- DAW compatibility validation
- Preset system functionality

### 2. Performance Validation
**CPU Profiling**: Continuous integration performance monitoring
**Memory Profiling**: Automated memory usage validation
**Audio Quality**: Automated THD and frequency response testing
**Stress Testing**: Extended operation with parameter automation

### 3. Manual Testing Requirements
**Musical Context Testing**: Real-world usage scenarios
**DAW Integration Testing**: Manual testing in each supported DAW
**User Interface Testing**: Parameter interaction validation
**Preset Testing**: Comprehensive preset library validation

## Development and Maintenance Specifications

### 1. Code Quality Standards
**C++ Standard**: C++20 or later
**Compiler Support**: 
- MSVC 2019+ (Windows)
- Clang 12+ (macOS)
- GCC 10+ (Linux development)

**Code Style**: 
- JUCE coding conventions
- Automated formatting with .clang-format
- Static analysis with .clang-tidy

### 2. Version Control and Releases
**Repository Structure**: Monorepo with plugin-specific folders
**Branching Strategy**: GitFlow with feature branches
**Release Process**: Automated builds with CMake
**Versioning**: Semantic versioning (MAJOR.MINOR.PATCH)

### 3. Documentation Requirements
**Code Documentation**: Comprehensive inline documentation
**API Documentation**: Generated with Doxygen
**User Documentation**: Comprehensive user manual
**Developer Documentation**: Architecture and contribution guidelines

## Conclusion

These technical specifications provide a comprehensive blueprint for TingeTape development, ensuring:

1. **Performance**: <1% CPU usage with professional audio quality
2. **Compatibility**: Broad DAW and system compatibility
3. **Maintainability**: Clean architecture with comprehensive testing
4. **Musicality**: Parameter ranges and behavior optimized for musical use
5. **Reliability**: Robust error handling and validation throughout

The specifications balance technical excellence with practical development constraints, ensuring TingeTape delivers professional-quality tape emulation with minimal system impact.