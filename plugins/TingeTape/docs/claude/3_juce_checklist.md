# JUCE Checklist Command for TingeTape Plugin

## Objective
Transform the specification into a detailed, phase-based build checklist with atomic subtasks that enforce strict test-driven development (TDD).

## Instructions for Claude

### Step 1: Analyze Specification
Review `docs/spec.md` and break down all requirements into:
- Atomic, testable tasks (each completable in <4 hours)
- Clear dependencies between tasks
- Specific acceptance criteria for each task
- Test requirements before implementation

### Step 2: Create Development Checklist
Generate `docs/checklist.md` with the following structure:

# TingeTape Plugin Development Checklist

## Development Principles
- **Test-Driven Development**: Write failing tests before implementation
- **Atomic Tasks**: Each task must be completable in one session
- **Continuous Integration**: Run full test suite after each task
- **No Commit Without Tests**: Every implementation must have corresponding tests

## Phase 1: Foundation & Core DSP Components

### Task 1.1: Parameter System Foundation
**Estimated Time**: 2 hours
**Dependencies**: None
**Test First**: Write parameter validation tests

#### Subtasks:
- [ ] **Test**: Create parameter bounds validation test
- [ ] **Test**: Create parameter smoothing test
- [ ] **Test**: Create atomic parameter access test
- [ ] **Implementation**: Implement parameter layout with all TingeTape parameters
- [ ] **Implementation**: Setup atomic parameter pointers
- [ ] **Implementation**: Configure parameter smoothing (50ms)
- [ ] **Validation**: All parameter tests pass
- [ ] **Validation**: pluginval parameter validation passes

#### Acceptance Criteria:
- All parameters accessible via AudioProcessorValueTreeState
- Parameter values within expected ranges (0-1 normalized)
- Atomic parameter access working from audio thread
- Parameter smoothing prevents audio clicks

### Task 1.2: WowEngine - LFO Component
**Estimated Time**: 3 hours
**Dependencies**: Task 1.1
**Test First**: Write LFO behavior tests

#### Subtasks:
- [ ] **Test**: Create LFO frequency accuracy test
- [ ] **Test**: Create LFO waveform correctness test
- [ ] **Test**: Create LFO phase continuity test
- [ ] **Implementation**: Implement juce::dsp::Oscillator setup
- [ ] **Implementation**: Configure sine wave LFO at 0.5Hz
- [ ] **Implementation**: Implement depth parameter control
- [ ] **Validation**: LFO generates expected sine wave
- [ ] **Validation**: Frequency accuracy within 1%
- [ ] **Performance**: LFO processing <0.1ms per buffer

#### Acceptance Criteria:
- LFO generates stable 0.5Hz sine wave
- Depth parameter correctly scales modulation
- No phase discontinuities during parameter changes
- Performance targets met

### Task 1.3: WowEngine - Delay Line Implementation
**Estimated Time**: 4 hours
**Dependencies**: Task 1.2
**Test First**: Write delay line functionality tests

#### Subtasks:
- [ ] **Test**: Create delay line accuracy test (known delay times)
- [ ] **Test**: Create modulated delay smoothness test
- [ ] **Test**: Create delay bounds validation test
- [ ] **Implementation**: Setup juce::dsp::DelayLine with 50ms maximum
- [ ] **Implementation**: Implement variable delay control via LFO
- [ ] **Implementation**: Add delay time smoothing to prevent artifacts
- [ ] **Validation**: Delay line produces expected delay times
- [ ] **Validation**: Smooth delay modulation without clicks
- [ ] **Performance**: Delay processing maintains realtime performance

#### Acceptance Criteria:
- Accurate delay times within 1 sample precision
- Smooth modulation without audio artifacts
- Proper bounds checking prevents buffer overruns
- Performance within CPU targets

### Task 1.4: WowEngine - Integration & Testing
**Estimated Time**: 2 hours
**Dependencies**: Task 1.3
**Test First**: Write complete wow effect tests

#### Subtasks:
- [ ] **Test**: Create wow effect audio characteristic test
- [ ] **Test**: Create depth parameter mapping test
- [ ] **Test**: Create bypass functionality test
- [ ] **Implementation**: Integrate LFO with delay line modulation
- [ ] **Implementation**: Implement wow depth parameter (0-100%)
- [ ] **Implementation**: Add bypass when depth = 0
- [ ] **Validation**: Wow effect produces expected pitch modulation
- [ ] **Validation**: Depth parameter correctly controls modulation amount
- [ ] **Performance**: Complete WowEngine <0.2ms per buffer

#### Acceptance Criteria:
- Convincing tape wow effect at various depth settings
- Smooth parameter transitions
- Proper bypass behavior
- Performance targets achieved

## Phase 2: Filter System Implementation

### Task 2.1: Resonant Filter Foundation
**Estimated Time**: 3 hours
**Dependencies**: Task 1.1
**Test First**: Write filter response tests

#### Subtasks:
- [ ] **Test**: Create frequency response accuracy test
- [ ] **Test**: Create resonance parameter validation test
- [ ] **Test**: Create filter stability test (high Q values)
- [ ] **Implementation**: Setup juce::dsp::IIR::Filter processors
- [ ] **Implementation**: Implement high-pass filter (low-cut)
- [ ] **Implementation**: Implement low-pass filter (high-cut)
- [ ] **Validation**: Filters produce expected frequency response
- [ ] **Validation**: Resonance parameter works correctly
- [ ] **Performance**: Filter processing <0.1ms per buffer

#### Acceptance Criteria:
- Accurate frequency response matching biquad calculations
- Stable operation at high resonance values (Q=10)
- Smooth parameter changes without artifacts
- Performance within targets

### Task 2.2: Filter Parameter Integration
**Estimated Time**: 2 hours
**Dependencies**: Task 2.1
**Test First**: Write filter parameter control tests

#### Subtasks:
- [ ] **Test**: Create frequency parameter mapping test
- [ ] **Test**: Create resonance parameter mapping test
- [ ] **Test**: Create real-time parameter update test
- [ ] **Implementation**: Connect filter parameters to UI controls
- [ ] **Implementation**: Implement logarithmic frequency scaling
- [ ] **Implementation**: Add parameter smoothing for filters
- [ ] **Validation**: Parameter changes produce expected filter response
- [ ] **Validation**: Smooth parameter transitions
- [ ] **Performance**: Parameter updates don't affect realtime performance

#### Acceptance Criteria:
- Intuitive parameter mapping (logarithmic frequency)
- Real-time parameter updates without artifacts
- Stable filter operation during parameter sweeps
- Performance maintained during parameter changes

## Phase 3: Saturation & Tone Processing

### Task 3.1: Tape Saturation Algorithm
**Estimated Time**: 4 hours
**Dependencies**: None (parallel with filters)
**Test First**: Write saturation characteristic tests

#### Subtasks:
- [ ] **Test**: Create harmonic content analysis test
- [ ] **Test**: Create drive parameter linearity test
- [ ] **Test**: Create saturation curve accuracy test
- [ ] **Implementation**: Implement tanh-based saturation curve
- [ ] **Implementation**: Add drive-dependent input gain scaling
- [ ] **Implementation**: Implement high-frequency rolloff simulation
- [ ] **Implementation**: Add output level compensation
- [ ] **Validation**: Saturation produces expected harmonic content
- [ ] **Validation**: Drive parameter behaves linearly
- [ ] **Performance**: Saturation processing <0.1ms per buffer

#### Acceptance Criteria:
- Pleasant harmonic distortion characteristic of tape
- Linear drive parameter response
- No unexpected level jumps
- Performance targets met

### Task 3.2: Tone Control Implementation
**Estimated Time**: 3 hours
**Dependencies**: Task 3.1
**Test First**: Write tone control response tests

#### Subtasks:
- [ ] **Test**: Create tilt filter response test
- [ ] **Test**: Create tone parameter mapping test
- [ ] **Test**: Create filter interaction test
- [ ] **Implementation**: Implement low shelf filter (250Hz)
- [ ] **Implementation**: Implement high shelf filter (5kHz)
- [ ] **Implementation**: Create opposing gain relationship
- [ ] **Validation**: Tilt filter produces expected response
- [ ] **Validation**: Tone parameter correctly controls brightness
- [ ] **Performance**: Tone processing <0.1ms per buffer

#### Acceptance Criteria:
- Smooth tilt filter response (-100% to +100%)
- Intuitive brightness/darkness control
- No unwanted resonances or artifacts
- Performance within targets

## Phase 4: Integration & Optimization

### Task 4.1: Signal Chain Integration
**Estimated Time**: 3 hours
**Dependencies**: All previous tasks
**Test First**: Write full signal chain tests

#### Subtasks:
- [ ] **Test**: Create signal chain order verification test
- [ ] **Test**: Create bypass functionality test
- [ ] **Test**: Create parameter interaction test
- [ ] **Implementation**: Connect all DSP components in proper order
- [ ] **Implementation**: Implement proper bypass functionality
- [ ] **Implementation**: Optimize parameter update frequency
- [ ] **Validation**: Signal chain produces expected output
- [ ] **Validation**: Bypass is transparent
- [ ] **Performance**: Full processing <1% CPU on target hardware

#### Acceptance Criteria:
- Correct signal processing order maintained
- Transparent bypass functionality
- No parameter interaction artifacts
- Performance targets achieved

### Task 4.2: Final Optimization & Polish
**Estimated Time**: 4 hours
**Dependencies**: Task 4.1
**Test First**: Write performance regression tests

#### Subtasks:
- [ ] **Test**: Create CPU usage regression test
- [ ] **Test**: Create memory leak detection test
- [ ] **Test**: Create plugin validation test suite
- [ ] **Implementation**: Profile and optimize critical code paths
- [ ] **Implementation**: Implement efficient parameter smoothing
- [ ] **Implementation**: Add denormal protection
- [ ] **Validation**: All performance tests pass
- [ ] **Validation**: pluginval passes with strictness level 10
- [ ] **Performance**: Sustained operation without performance degradation

#### Acceptance Criteria:
- CPU usage consistently <1% on target hardware
- No memory leaks during extended operation
- All automated tests pass
- Professional plugin validation passes

## Success Criteria for Complete Checklist
- [ ] All tasks completed with passing tests
- [ ] Full test suite passes (unit, integration, performance)
- [ ] pluginval validation passes (strictness level 10)
- [ ] Performance targets met on all supported platforms
- [ ] Manual testing completed in target DAWs
- [ ] Documentation updated and complete

## Next Step
After completing checklist, proceed to `4_juce_build.md` for execution phase.