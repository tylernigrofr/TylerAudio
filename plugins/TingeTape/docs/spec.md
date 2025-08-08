# TingeTape Plugin Comprehensive Specification

## Vision & Purpose

### Plugin Mission
TingeTape delivers authentic analog tape character through research-backed digital signal processing, focusing on the essential elements that define tape's musical appeal: subtle wow modulation, musical saturation, and frequency response characteristics. Unlike complex multi-model simulations, TingeTape prioritizes efficiency and musicality.

### Target Users
- **Mixing Engineers**: Seeking authentic tape warmth and character on individual tracks or busses
- **Mastering Engineers**: Requiring subtle tape glue and harmonic enhancement
- **Music Producers**: Wanting vintage tape aesthetics without workflow complexity
- **Home Studio Musicians**: Needing professional tape sound with minimal CPU impact

### Differentiation
- **Research-Backed Algorithms**: Every component based on comprehensive technical research
- **Performance Priority**: <1% CPU usage enables creative use across entire projects
- **Essential Parameters Only**: 7 carefully chosen controls avoid decision paralysis
- **Authentic Character**: Focuses on musical tape behaviors, not technical accuracy

## Functional Requirements

### Core Features

#### Wow Modulation
- **Characteristics**: 0.5Hz sine wave LFO creating pitch modulation via delay line
- **Control Range**: 0-100% depth control (0.1%-1% pitch variation authentic range)
- **Implementation**: Variable delay line with 5ms base delay, 0-45ms modulation range
- **Musical Impact**: Subtle pitch instability characteristic of analog tape transport

#### Resonant Filters
**Low-Cut (High-Pass) Filter**:
- **Type**: 2nd-order Butterworth high-pass
- **Frequency Range**: 20Hz - 200Hz
- **Resonance Range**: Q = 0.1 - 2.0 (default 0.707 Butterworth)
- **Purpose**: Remove subsonic rumble, focus bass response

**High-Cut (Low-Pass) Filter**:
- **Type**: 2nd-order Butterworth low-pass  
- **Frequency Range**: 5kHz - 20kHz
- **Resonance Range**: Q = 0.1 - 2.0 (default 0.707 Butterworth)
- **Purpose**: Tape-like high-frequency rolloff, reduce harshness

#### Tape Saturation
- **Algorithm**: Hyperbolic tangent (tanh) with normalization
- **Mathematical Model**: `output = tanh(input * driveGain) / tanh(driveGain)`
- **Drive Mapping**: Exponential scaling from 1x to 10x gain
- **Harmonic Content**: Predominantly odd harmonics, even harmonics at higher drive
- **Frequency Integration**: Drive-dependent high-frequency rolloff simulation

#### Tone Control
- **Implementation**: Dual shelving filters creating tilt response
- **Low Shelf**: 250Hz, ±6dB gain range
- **High Shelf**: 5kHz, ±6dB gain range (inverse of low shelf)
- **Control Range**: -100% (dark) to +100% (bright), 0% = flat response
- **Character**: Single control provides intuitive brightness/darkness adjustment

#### Bypass Functionality
- **Type**: True bypass with 10ms crossfade
- **Implementation**: Direct input-to-output connection when engaged
- **State Preservation**: DSP state maintained during bypass for seamless re-engagement

### Parameter Specifications

| Parameter | Range | Default | Units | Mapping | Display Format |
|-----------|-------|---------|-------|---------|----------------|
| Wow Depth | 0-100 | 25 | % | Linear | "25.0%" |
| Low-Cut Freq | 20-200 | 40 | Hz | Logarithmic | "40 Hz" |
| Low-Cut Q | 0.1-2.0 | 0.707 | Q | Linear | "0.71" |
| High-Cut Freq | 5k-20k | 15k | Hz | Logarithmic | "15.0 kHz" |
| High-Cut Q | 0.1-2.0 | 0.707 | Q | Linear | "0.71" |
| Dirt/Drive | 0-100 | 25 | % | Linear | "25.0%" |
| Tone | -100 to +100 | 0 | % | Linear | "+25.0%" / "-25.0%" |
| Bypass | Off/On | Off | Toggle | Boolean | "Bypass" |

### Parameter Automation Behavior
- **Smoothing Time**: 50ms for Wow (avoid modulation artifacts), 20ms for filters (prevent clicks), 30ms for Drive (prevent level jumps)
- **Thread Safety**: Atomic parameter access from UI thread, realtime-safe audio thread
- **Host Integration**: Full DAW automation support with proper parameter notification

### User Interface Requirements

#### Layout Design
- **Dimensions**: 400x300 pixels (scalable for HiDPI)
- **Organization**: Signal flow layout matching processing order
- **Visual Hierarchy**: Primary controls (Wow, Drive, Tone) prominent, filters secondary
- **Branding**: Tyler Audio consistent visual identity

#### Control Types
- **Primary Controls**: Large knobs for Wow, Drive, Tone
- **Filter Controls**: Smaller knobs for frequency/Q pairs
- **Bypass**: Toggle button with clear visual state
- **Visual Feedback**: Real-time parameter value display, LED-style indicators

#### Accessibility Considerations
- **Keyboard Navigation**: Full keyboard control support
- **Screen Reader**: Comprehensive accessibility handler implementation  
- **High Contrast**: Visual elements visible in high contrast modes
- **Scaling**: Support for 100%-200% scaling factors

## Technical Architecture

### Signal Flow Design
```
Input → Low-Cut Filter → Tape Saturation → Tone Control → High-Cut Filter → Wow Engine → Output
```

**Signal Flow Rationale**:
1. **Low-Cut First**: Remove subsonic content before saturation prevents unwanted artifacts
2. **Saturation Early**: Generate harmonics before frequency shaping for authentic tape behavior
3. **Tone Control**: Shape overall frequency balance after harmonic generation
4. **High-Cut**: Final frequency contouring matches tape machine behavior
5. **Wow Last**: Pitch modulation applied to complete processed signal maintains authenticity

### DSP Component Specifications

#### WowEngine Technical Specs
- **LFO**: `juce::dsp::Oscillator<float>` with sine wave, 0.5Hz fixed frequency
- **Delay Line**: `juce::dsp::DelayLine<float>` with linear interpolation
- **Maximum Delay**: 50ms (sufficient for all wow effects at any sample rate)
- **Base Delay**: 5ms (prevents zero-delay issues and processing artifacts)
- **Modulation Calculation**: `modulatedDelay = baseDelay + (lfoOutput * depthParam * maxModulation)`
- **Depth Mapping**: Linear 0-100% to 0-45ms modulation range

#### ResonantFilters Technical Specs
- **Implementation**: `juce::dsp::IIR::Filter<float>` with coefficient updates
- **Filter Type**: 2nd-order Butterworth for natural response
- **Coefficient Calculation**: Real-time update when parameters change
- **Stability Validation**: Automatic coefficient validation and limiting
- **Threading**: Coefficient calculation on audio thread for realtime safety

#### TapeSaturation Technical Specs
- **Core Algorithm**: `std::tanh(input * driveGain) / std::tanh(driveGain)`
- **Drive Scaling**: `driveGain = 1.0 + (driveParam * 0.01) * 9.0` (1x to 10x)
- **HF Rolloff**: Simple one-pole filter, drive-dependent cutoff frequency
- **Level Compensation**: Output scaling to maintain consistent levels
- **Denormal Protection**: Flush-to-zero for sub-threshold values

#### ToneControl Technical Specs  
- **Low Shelf**: `juce::dsp::IIR::Coefficients<float>::makeLowShelf(250Hz, Q=0.707, gain)`
- **High Shelf**: `juce::dsp::IIR::Coefficients<float>::makeHighShelf(5kHz, Q=0.707, -gain)`
- **Gain Calculation**: `gainDb = toneParam * 6.0` (±6dB range)
- **Tilt Behavior**: Complementary gains create natural tilt filter response
- **Bypass Logic**: Skip processing when tone parameter near zero

### Performance Requirements

#### CPU Usage Targets
- **Primary Target**: <1% CPU usage at 48kHz/512 samples on modern hardware
- **Benchmark Hardware**: Intel Core i7-9700K @ 3.6GHz or equivalent
- **Scaling**: Linear scaling with sample rate, logarithmic with buffer size
- **Multiple Instances**: Support 50+ instances simultaneously

#### Memory Management
- **Pre-allocation Strategy**: All buffers allocated in `prepareToPlay()`
- **Delay Line Memory**: 50ms * maxSampleRate * numChannels (~38KB at 192kHz stereo)
- **Total Memory Usage**: <50KB per plugin instance
- **Realtime Safety**: Zero memory allocation in `processBlock()`

#### Threading Architecture
- **Audio Thread**: DSP processing, atomic parameter reads, state updates
- **Message Thread**: Parameter updates, UI communication, preset management  
- **Thread Safety**: Lock-free atomic variables, no shared mutable state
- **Communication**: Atomic parameter pointers for realtime-safe access

### Sample Rate and Buffer Support
- **Sample Rate Range**: 44.1kHz - 192kHz with automatic scaling
- **Buffer Size Range**: 32 - 2048 samples with consistent performance
- **Latency**: Minimal processing delay (~5ms from internal delay line only)
- **Quality**: Maintain audio fidelity across all supported configurations

## Validation Strategy

### Testing Approach

#### Unit Tests (Component-Level)
- **WowEngine Tests**: LFO frequency accuracy, delay time precision, modulation depth linearity
- **TapeSaturation Tests**: Harmonic content analysis, drive mapping validation, level compensation
- **Filter Tests**: Frequency response accuracy, Q factor validation, stability at extremes
- **ToneControl Tests**: Shelf frequency precision, gain accuracy, tilt behavior verification
- **Parameter Tests**: Bounds validation, smoothing behavior, atomic access safety

#### Integration Tests (System-Level)  
- **Signal Chain Tests**: Complete processing pipeline validation
- **Parameter Interaction**: Cross-component behavior verification
- **State Management**: Save/restore consistency, preset compatibility
- **DAW Compatibility**: Major DAW integration and automation testing
- **Format Compliance**: VST3/AU specification adherence validation

#### Performance Tests
- **CPU Usage Validation**: Sustained operation under target thresholds
- **Memory Leak Detection**: Extended operation without memory growth
- **Realtime Safety**: No allocation/blocking operations in audio thread
- **Scalability Testing**: Multiple instance performance characteristics
- **Platform Testing**: Windows/macOS performance parity validation

### Quality Metrics

#### Audio Fidelity Standards
- **Bypass Transparency**: <0.01dB deviation from direct signal path
- **Dynamic Range**: >120dB (no processing artifacts introduced)
- **THD+N**: <0.1% at moderate settings, <1% at extreme settings
- **Frequency Response**: ±0.1dB deviation from design specifications
- **Signal-to-Noise Ratio**: >100dB (limited only by input signal quality)

#### Stability Requirements
- **Crash-Free Operation**: Zero crashes in 10,000 pluginval test iterations
- **Parameter Automation**: Stable during continuous parameter sweeps
- **Sample Rate Changes**: Seamless adaptation without artifacts
- **Extended Operation**: 24+ hour continuous operation without degradation
- **Edge Case Handling**: Graceful behavior at all parameter extremes

#### Usability Standards
- **Parameter Response**: Immediate audio feedback to parameter changes
- **Automation Behavior**: Smooth, musical parameter automation curves
- **Preset Consistency**: Identical sound across save/load cycles
- **UI Responsiveness**: <16ms UI update latency (60fps capable)
- **Cross-Platform**: Identical behavior on Windows and macOS

### Reference Standards

#### Plugin Format Compliance
- **VST3 Specification**: Full compliance with Steinberg VST3 requirements
- **Audio Unit Compliance**: Apple Audio Unit specification adherence
- **Host Compatibility**: Tested compatibility with major DAW applications
- **Automation Support**: Complete host automation integration
- **State Management**: Robust preset and project state handling

#### Performance Benchmarks
- **Industry Comparison**: Competitive or superior to commercial alternatives
- **Efficiency Rating**: Best-in-class CPU/memory usage for feature set
- **Scalability**: Superior multiple-instance performance characteristics
- **Reliability**: Professional-grade stability and robustness

## Implementation Phases

### Phase 1: Core DSP Foundation (Week 1)
**Deliverables**:
- Research-compliant WowEngine implementation
- Corrected TapeSaturation algorithm with proper drive scaling
- Enhanced parameter management with proper smoothing timing
- Basic unit tests for each component

**Success Criteria**:
- All DSP components match research specifications exactly
- Unit tests achieve 100% pass rate
- Performance targets met for individual components

### Phase 2: Integration & Testing (Week 2)
**Deliverables**:
- Complete signal chain integration
- Comprehensive test suite implementation
- Performance validation and optimization
- Audio quality validation against specifications

**Success Criteria**:
- Full integration tests pass with audio quality validation
- Performance targets met for complete plugin
- Comprehensive test coverage achieved

### Phase 3: Polish & Validation (Week 3)
**Deliverables**:
- Final parameter calibration and optimization
- Complete documentation linking code to research
- DAW compatibility testing and validation
- Professional quality assurance completion

**Success Criteria**:
- Professional quality standards met across all metrics
- Research specifications fully implemented and validated
- Ready for professional deployment

## Risk Assessment

### Technical Risks
- **Algorithm Complexity**: Complex wow modulation may impact performance
- **Filter Stability**: High resonance settings may cause instability  
- **Performance Targets**: Research-quality algorithms may challenge CPU targets
- **Cross-Platform Consistency**: Ensuring identical behavior across platforms

### Mitigation Strategies
- **Incremental Development**: Implement and test each component individually
- **Performance Profiling**: Continuous monitoring throughout development
- **Comprehensive Testing**: Extensive automated and manual validation
- **Research Validation**: Every implementation decision backed by research findings

## Success Criteria

This specification is complete when:
- [x] All functional requirements clearly defined with measurable criteria
- [x] Technical architecture fully specified with implementation details  
- [x] Performance targets established with validation methodology
- [x] Testing strategy comprehensive with quality metrics
- [x] Implementation phases planned with concrete deliverables
- [x] Risk assessment completed with mitigation strategies

## Implementation Ready
This specification provides complete guidance for implementing a professional-quality tape emulation plugin based on comprehensive research findings. Every technical decision is backed by research data, ensuring authentic tape character with modern plugin reliability and performance.