#!/bin/bash
# Enhanced Tyler Audio Plugin Creation Script
# Creates complete development environment with testing, documentation, and structured workflows

set -e

# Color output for better visibility
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

if [ $# -lt 1 ] || [ $# -gt 2 ]; then
    echo -e "${RED}Usage: $0 <PluginName> [PluginType]${NC}"
    echo -e "Example: $0 MyAwesomeEQ filter"
    echo -e "Example: $0 TapeEmulation effect"
    echo -e ""
    echo -e "Available PluginTypes:"
    echo -e "  • effect (default) - General audio effect"
    echo -e "  • filter - Filtering/EQ plugin"
    echo -e "  • dynamics - Compression/limiting plugin"
    echo -e "  • modulation - Chorus/flanger/phaser plugin"
    echo -e "  • saturation - Distortion/saturation plugin"
    echo -e "  • delay - Delay/reverb plugin"
    echo -e "  • synthesis - Synthesizer/generator plugin"
    exit 1
fi

PLUGIN_NAME="$1"
PLUGIN_TYPE="${2:-effect}"
PLUGIN_DIR="plugins/$PLUGIN_NAME"

# Validate plugin name
if [[ ! "$PLUGIN_NAME" =~ ^[A-Za-z][A-Za-z0-9]*$ ]]; then
    echo -e "${RED}Error: Plugin name must start with a letter and contain only letters and numbers${NC}"
    exit 1
fi

if [ -d "$PLUGIN_DIR" ]; then
    echo -e "${RED}Error: Plugin directory $PLUGIN_DIR already exists${NC}"
    exit 1
fi

echo -e "${GREEN}🚀 Creating new Tyler Audio plugin: $PLUGIN_NAME${NC}"
echo -e "${BLUE}📋 Plugin type: $PLUGIN_TYPE${NC}"

# Step 1: Copy base template
echo -e "${BLUE}📁 Creating plugin structure...${NC}"
cp -r plugins/ExamplePlugin "$PLUGIN_DIR"

# Generate unique 4-character plugin code
PLUGIN_CODE=$(echo "$PLUGIN_NAME" | head -c 4 | tr '[:lower:]' '[:upper:]')
if [ ${#PLUGIN_CODE} -lt 4 ]; then
    # Pad with numbers if name is too short
    PLUGIN_CODE="${PLUGIN_CODE}$(seq -f "%g" 1 $((4 - ${#PLUGIN_CODE})) | tr -d '\n')"
fi

echo -e "${BLUE}🏷️  Using plugin code: $PLUGIN_CODE${NC}"

# Step 2: Replace template content in CMakeLists.txt
echo -e "${BLUE}🔧 Configuring build system...${NC}"
sed -i.bak "s/ExamplePlugin/$PLUGIN_NAME/g" "$PLUGIN_DIR/CMakeLists.txt"
sed -i.bak "s/Expl/$PLUGIN_CODE/g" "$PLUGIN_DIR/CMakeLists.txt"
rm "$PLUGIN_DIR/CMakeLists.txt.bak"

# Step 3: Replace template content in source files
echo -e "${BLUE}💻 Updating source files...${NC}"
find "$PLUGIN_DIR/Source" -name "*.cpp" -o -name "*.h" | while read -r file; do
    sed -i.bak "s/ExamplePlugin/$PLUGIN_NAME/g" "$file"
    rm "$file.bak"
done

# Step 4: Create comprehensive documentation structure
echo -e "${BLUE}📚 Setting up documentation structure...${NC}"
create_docs_structure() {
    local docs_dir="$PLUGIN_DIR/docs"
    local claude_dir="$docs_dir/claude"
    local research_dir="$docs_dir/research"
    
    # Create directories
    mkdir -p "$claude_dir" "$research_dir"
    
    # Create four-prompt command system
    create_research_command "$claude_dir" "$PLUGIN_NAME" "$PLUGIN_TYPE"
    create_specification_command "$claude_dir" "$PLUGIN_NAME" "$PLUGIN_TYPE"
    create_checklist_command "$claude_dir" "$PLUGIN_NAME" "$PLUGIN_TYPE"
    create_build_command "$claude_dir" "$PLUGIN_NAME" "$PLUGIN_TYPE"
    
    # Create research template files
    create_research_templates "$research_dir" "$PLUGIN_NAME" "$PLUGIN_TYPE"
}

# Function to create research command
create_research_command() {
    local claude_dir="$1"
    local plugin_name="$2"
    local plugin_type="$3"
    
    cat > "$claude_dir/1_juce_research.md" << EOF
# JUCE Research Command for $plugin_name Plugin

## Objective
Research and compile comprehensive information for $plugin_type plugin development using JUCE framework.

## Instructions for Claude

### Step 1: Classify Plugin Concept
Analyze the $plugin_name plugin concept and classify it within the audio plugin ecosystem:
- Plugin category (e.g., $plugin_type, analog modeling, creative effect)
- Target use cases (mixing, mastering, creative processing)
- Technical complexity level
- Market positioning relative to existing plugins

### Step 2: Context7 MCP Research (MANDATORY)
**CRITICAL**: Query Context7 MCP server for up-to-date JUCE documentation on:
- \`juce::dsp\` module for DSP processing
- \`juce::AudioProcessor\` best practices
- \`juce::AudioProcessorValueTreeState\` for parameter management  
- Relevant DSP components for $plugin_type plugins
- Performance optimization techniques for realtime audio

### Step 3: Compile Research Sources
Search and compile from multiple sources:

#### A. Official Documentation
- JUCE official documentation and tutorials
- JUCE GitHub repository examples
- JUCE DSP module documentation

#### B. Academic Sources
- Recent papers on $plugin_type algorithms
- Digital signal processing research (last 5 years)
- Psychoacoustic research relevant to $plugin_type
- Performance optimization studies

#### C. Industry Best Practices
- Open source $plugin_type plugins for reference
- Professional plugin development patterns
- JUCE community discussions and solutions

### Step 4: Apply Relevance Filtering
Filter and prioritize sources based on:
- Relevance to $plugin_type DSP
- JUCE framework compatibility
- Realtime performance requirements
- Code quality and maintainability
- Recent publication/update date

### Step 5: Create Research Output
Generate the following files in \`docs/research/\`:

#### \`${plugin_type}_algorithms.md\`
- Core algorithms for $plugin_type processing
- Mathematical models and implementations
- Performance considerations
- Parameter mapping strategies

#### \`juce_dsp_patterns.md\`
- Best practices for JUCE DSP implementation
- Realtime safety patterns
- Parameter smoothing techniques
- Performance optimization strategies

#### \`reference_implementations.md\`
- Analysis of existing open-source $plugin_type plugins
- Code patterns and architectural decisions
- Performance benchmarks and comparisons

#### \`technical_specifications.md\`
- Detailed technical requirements for $plugin_name
- DSP algorithm specifications
- Performance targets and constraints
- API design considerations

## Success Criteria
Research is complete when:
- [ ] Context7 MCP queries completed for all JUCE modules
- [ ] Minimum 5 academic sources found and analyzed
- [ ] 3+ reference implementations analyzed
- [ ] Technical specifications clearly defined
- [ ] All research files created with comprehensive information
- [ ] Sources properly cited with links and publication dates

## Next Step
After completing research, proceed to \`2_juce_specification.md\` to distill findings into actionable specification.
EOF
}

# Function to create specification command
create_specification_command() {
    local claude_dir="$1"
    local plugin_name="$2"
    local plugin_type="$3"
    
    cat > "$claude_dir/2_juce_specification.md" << EOF
# JUCE Specification Command for $plugin_name Plugin

## Objective
Transform research findings into a comprehensive, actionable specification for $plugin_name plugin development.

## Instructions for Claude

### Step 1: Analyze Research
Review all files in \`docs/research/\` and synthesize findings:
- Identify key technical insights from research
- Extract proven algorithms and implementation patterns
- Consolidate performance requirements and constraints
- Highlight potential implementation challenges

### Step 2: Create Comprehensive Specification
Generate \`docs/spec.md\` with the following structure:

## Vision & Purpose
- **Plugin Mission**: Clear statement of $plugin_name's purpose and unique value
- **Target Users**: Specific user personas and use cases
- **Differentiation**: How $plugin_name differs from existing $plugin_type plugins

## Functional Requirements

### Core Features
Define the essential $plugin_type processing features:
- Primary processing algorithms
- Parameter specifications and control ranges
- User interface requirements
- Performance characteristics

### Parameter Specifications
For each parameter, define:
- Control range and default values
- Mapping curves (linear, logarithmic, etc.)
- Display formatting and units
- Automation behavior
- Parameter grouping and relationships

### User Interface Requirements
- Layout and visual design direction
- Control types (knobs, sliders, switches)
- Visual feedback requirements
- Accessibility considerations

## Technical Architecture

### Signal Flow Design
Define the complete DSP signal chain for $plugin_type processing

### DSP Component Specifications
- Core processing components and their specifications
- Parameter ranges and mappings
- Performance requirements
- Memory management strategy

### Performance Requirements
- **CPU Usage**: Target <1% on modern hardware (48kHz, 512 samples)
- **Latency**: Minimize latency while maintaining audio quality
- **Sample Rate Support**: 44.1kHz to 192kHz
- **Threading**: Realtime-safe audio thread, parameter updates on message thread

### Memory Management
- **Allocation Strategy**: Pre-allocate all buffers in \`prepareToPlay()\`
- **Buffer Sizes**: Calculate maximum memory requirements
- **Thread Safety**: Atomic parameter access, lock-free audio processing

## Validation Strategy

### Testing Approach
- **Unit Tests**: Individual DSP component validation
- **Integration Tests**: Full signal chain testing
- **Performance Tests**: CPU and memory usage validation
- **Compatibility Tests**: Multiple DAW compatibility verification

### Quality Metrics
- **Audio Fidelity**: Define acceptable deviation from reference
- **Stability**: Zero crashes in extended operation
- **Performance**: Consistent processing time, no audio dropouts
- **Usability**: Intuitive parameter behavior, immediate audio feedback

### Reference Standards
- **Plugin Format Compliance**: VST3 and AU specification adherence
- **Host Compatibility**: Tested with major DAWs
- **Platform Support**: Windows and macOS compatibility

## Implementation Phases

### Phase 1: Core DSP (Week 1-2)
- Primary processing implementation and testing
- Basic parameter system setup
- Performance optimization

### Phase 2: Advanced Features (Week 2-3)  
- Advanced processing components
- Complete parameter integration
- User interface development

### Phase 3: Integration & Testing (Week 3-4)
- Full signal chain integration
- Comprehensive testing suite
- Performance validation

### Phase 4: Polish & Release (Week 4+)
- Final optimization and testing
- Documentation and preset creation
- Release preparation

## Risk Assessment

### Technical Risks
Identify potential technical challenges specific to $plugin_type development

### Mitigation Strategies
- **Incremental Development**: Build and test components individually
- **Performance Profiling**: Regular performance monitoring
- **Comprehensive Testing**: Extensive automated and manual testing

## Success Criteria
Specification is complete when:
- [ ] All functional requirements clearly defined
- [ ] Technical architecture fully specified
- [ ] Performance targets established
- [ ] Testing strategy outlined
- [ ] Implementation phases planned
- [ ] Risks identified and mitigation strategies defined

## Next Step
After completing specification, proceed to \`3_juce_checklist.md\` to create detailed implementation checklist.
EOF
}

# Function to create checklist command
create_checklist_command() {
    local claude_dir="$1"
    local plugin_name="$2"
    local plugin_type="$3"
    
    cat > "$claude_dir/3_juce_checklist.md" << EOF
# JUCE Checklist Command for $plugin_name Plugin

## Objective
Transform the specification into a detailed, phase-based build checklist with atomic subtasks that enforce strict test-driven development (TDD).

## Instructions for Claude

### Step 1: Analyze Specification
Review \`docs/spec.md\` and break down all requirements into:
- Atomic, testable tasks (each completable in <4 hours)
- Clear dependencies between tasks
- Specific acceptance criteria for each task
- Test requirements before implementation

### Step 2: Create Development Checklist
Generate \`docs/checklist.md\` with the following structure:

# $plugin_name Plugin Development Checklist

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
- [ ] **Implementation**: Implement parameter layout with all $plugin_name parameters
- [ ] **Implementation**: Setup atomic parameter pointers
- [ ] **Implementation**: Configure parameter smoothing (50ms)
- [ ] **Validation**: All parameter tests pass
- [ ] **Validation**: pluginval parameter validation passes

#### Acceptance Criteria:
- All parameters accessible via AudioProcessorValueTreeState
- Parameter values within expected ranges (0-1 normalized)
- Atomic parameter access working from audio thread
- Parameter smoothing prevents audio clicks

### Task 1.2: Core DSP Component - [Primary Processing]
**Estimated Time**: 3-4 hours
**Dependencies**: Task 1.1
**Test First**: Write core processing behavior tests

#### Subtasks:
- [ ] **Test**: Create core algorithm accuracy test
- [ ] **Test**: Create parameter response test
- [ ] **Test**: Create stability test
- [ ] **Implementation**: Implement primary $plugin_type processing
- [ ] **Implementation**: Configure parameter control
- [ ] **Implementation**: Add safety checks and validation
- [ ] **Validation**: Processing produces expected results
- [ ] **Validation**: Parameter changes work correctly
- [ ] **Performance**: Processing meets performance targets

#### Acceptance Criteria:
- Core $plugin_type processing works as specified
- Parameter control functions correctly
- No stability issues under normal operation
- Performance targets achieved

[Continue with additional phases and tasks based on plugin type...]

## Phase 2: Advanced Processing & Features
[Plugin-specific advanced features]

## Phase 3: Integration & Optimization
[Integration tasks and performance optimization]

## Phase 4: Testing & Polish
[Final testing and polishing tasks]

## Success Criteria for Complete Checklist
- [ ] All tasks completed with passing tests
- [ ] Full test suite passes (unit, integration, performance)
- [ ] pluginval validation passes (strictness level 10)
- [ ] Performance targets met on all supported platforms
- [ ] Manual testing completed in target DAWs
- [ ] Documentation updated and complete

## Next Step
After completing checklist, proceed to \`4_juce_build.md\` for execution phase.
EOF
}

# Function to create build command
create_build_command() {
    local claude_dir="$1"
    local plugin_name="$2"
    local plugin_type="$3"
    
    cat > "$claude_dir/4_juce_build.md" << EOF
# JUCE Build Command for $plugin_name Plugin

## Objective
Execute every task in the checklist sequentially using strict test-driven development (TDD), mark tasks as complete, and commit changes with proper validation.

## Instructions for Claude

### Execution Principles
- **Strict TDD**: Always write failing tests before implementing functionality
- **Atomic Commits**: Commit after each completed task with descriptive messages
- **Continuous Validation**: Run full test suite after each implementation
- **No Skipping Tests**: Every implementation must have corresponding tests
- **Performance Monitoring**: Check performance after each major component

### Build Execution Process

#### Step 1: Initialize Build Environment
\`\`\`bash
# Ensure clean build state
cmake --build build --clean-first
# Run initial test suite to verify baseline
cd build && ctest --output-on-failure
\`\`\`

#### Step 2: Execute Checklist Tasks Sequentially

For each task in \`docs/checklist.md\`:

1. **Pre-Task Validation**
   - [ ] Verify all dependencies completed
   - [ ] Review task requirements and acceptance criteria
   - [ ] Estimate time and prepare development environment

2. **Test-First Implementation**
   - [ ] Write failing tests first (following TDD principles)
   - [ ] Verify tests fail as expected
   - [ ] Run test suite to ensure no regressions

3. **Minimal Implementation**
   - [ ] Implement minimum code to make tests pass
   - [ ] Focus on correctness over optimization initially
   - [ ] Use Context7 MCP for JUCE API verification

4. **Refactor & Optimize**
   - [ ] Improve code quality while keeping tests green
   - [ ] Optimize for performance if needed
   - [ ] Add error handling and edge case management

5. **Task Validation**
   - [ ] Run full test suite (unit + integration + performance)
   - [ ] Verify acceptance criteria met
   - [ ] Check performance targets achieved
   - [ ] Run smoke tests on plugin

6. **Documentation & Commit**
   - [ ] Update relevant documentation
   - [ ] Mark task as complete in checklist
   - [ ] Commit with descriptive message
   - [ ] Tag commit with task ID for tracking

#### Step 3: Continuous Integration Checks

After each task completion:
\`\`\`bash
# Build plugin
cmake --build build --config Release --target $plugin_name

# Run comprehensive test suite
./scripts/run-smoke-tests.sh

# Deploy for testing
./scripts/deploy-plugins.sh

# Run pluginval if available
pluginval --strictness-level 10 --validate [plugin-path]
\`\`\`

#### Step 4: Phase Completion Validation

At the end of each phase:
- [ ] All phase tasks completed and marked
- [ ] Full regression test suite passes
- [ ] Performance benchmarks within targets
- [ ] Manual testing in DAW completed
- [ ] Phase completion commit with summary

### Task Execution Template

For each task, follow this template:

\`\`\`markdown
## Task X.Y: [Task Name] - [Status: In Progress/Complete]

### Started: [Date/Time]
### Estimated: [X hours]
### Actual: [Y hours]

### Tests Written:
- [ ] Test 1: [Description] - [Pass/Fail]
- [ ] Test 2: [Description] - [Pass/Fail]
- [ ] Test 3: [Description] - [Pass/Fail]

### Implementation Notes:
- Context7 queries performed: [List API queries]
- Key design decisions: [Brief notes]
- Performance measurements: [CPU/Memory stats]

### Validation Results:
- [ ] Acceptance criteria 1: [Met/Not Met]
- [ ] Acceptance criteria 2: [Met/Not Met]
- [ ] Performance target: [Within/Exceeded/Below target]

### Commit Information:
- Commit hash: [SHA]
- Commit message: [Message]
- Files changed: [List key files]

### Next Task Dependencies:
- [List any tasks now unblocked]
\`\`\`

### Error Handling Protocol

If any task fails:
1. **Document Failure**: Record specific failure mode and error messages
2. **Rollback if Needed**: Use git to rollback to last stable state
3. **Research Solution**: Use Context7 MCP to research JUCE best practices
4. **Revise Approach**: Modify implementation strategy if needed
5. **Update Checklist**: Adjust task if requirements were unrealistic

### Performance Monitoring

Track these metrics throughout build:
- **CPU Usage**: Per-component and total processing time
- **Memory Usage**: Peak and steady-state memory consumption
- **Test Execution Time**: Ensure test suite remains fast (<30 seconds)
- **Build Time**: Monitor for build performance regressions

### Quality Gates

Before marking any task complete:
- [ ] All task tests pass
- [ ] No regression in existing tests
- [ ] Performance within acceptable range
- [ ] Code follows project style guidelines
- [ ] Documentation updated if needed

### Final Build Validation

When all tasks complete:
1. **Comprehensive Test Suite**
   \`\`\`bash
   # Run all tests
   cd build && ctest --output-on-failure --parallel 4
   
   # Run smoke tests
   ./scripts/run-smoke-tests.sh
   
   # Run benchmarks
   ./benchmarks --reporter compact
   \`\`\`

2. **Plugin Validation**
   \`\`\`bash
   # Run pluginval
   pluginval --strictness-level 10 --validate [plugin-paths]
   
   # Deploy plugins
   ./scripts/deploy-plugins.sh
   \`\`\`

3. **Manual Testing Checklist**
   - [ ] Plugin loads in target DAWs without errors
   - [ ] All parameters respond correctly
   - [ ] Audio processing sounds as expected
   - [ ] No audio dropouts or glitches
   - [ ] Automation works properly
   - [ ] Preset save/load functions correctly

4. **Performance Validation**
   - [ ] CPU usage <1% on target hardware
   - [ ] No memory leaks during extended operation
   - [ ] Stable operation at all supported sample rates
   - [ ] Proper threading behavior

### Success Criteria

Build is complete when:
- [ ] All checklist tasks marked complete
- [ ] All automated tests pass
- [ ] Plugin passes pluginval validation
- [ ] Performance targets met
- [ ] Manual testing completed successfully
- [ ] Documentation updated
- [ ] Clean commit history with descriptive messages

### Completion Report

Generate final report including:
- Total development time vs. estimates
- Performance benchmarks achieved
- Test coverage statistics
- Known issues or limitations
- Recommendations for future development

## Execution Command

To begin build execution:
\`\`\`bash
# Start build process
echo "🚀 Starting $plugin_name Plugin Build Process"
echo "Following TDD principles with continuous validation"

# Initialize environment
./scripts/dev-setup.sh  # If needed

# Begin task execution from Phase 1, Task 1.1
# Each task will be executed with full TDD cycle
\`\`\`

Ready to begin systematic plugin development with professional quality standards!
EOF
}

# Function to create research template files
create_research_templates() {
    local research_dir="$1"
    local plugin_name="$2"
    local plugin_type="$3"
    
    # Create algorithm research template
    cat > "$research_dir/${plugin_type}_algorithms.md" << EOF
# $plugin_type Plugin Algorithms Research

## Overview
Research findings on $plugin_type processing algorithms for the $plugin_name plugin implementation.

## Core Algorithms
[To be populated during research phase]

## Implementation Considerations
[To be populated during research phase]

## Performance Optimization
[To be populated during research phase]

## References
[To be populated during research phase]
EOF

    # Create JUCE patterns template
    cat > "$research_dir/juce_dsp_patterns.md" << EOF
# JUCE DSP Implementation Patterns for $plugin_name

## Overview
JUCE-specific implementation patterns and best practices for $plugin_type plugins.

## Core Architecture Patterns
[To be populated during research phase]

## Parameter Management
[To be populated during research phase]

## Performance Patterns
[To be populated during research phase]

## Thread Safety
[To be populated during research phase]

## References
[To be populated during research phase]
EOF

    # Create reference implementations template
    cat > "$research_dir/reference_implementations.md" << EOF
# Reference Implementation Analysis for $plugin_name

## Overview
Analysis of existing $plugin_type implementations and architectural decisions.

## Open Source Examples
[To be populated during research phase]

## Commercial References
[To be populated during research phase]

## Key Takeaways
[To be populated during research phase]

## Implementation Recommendations
[To be populated during research phase]

## References
[To be populated during research phase]
EOF

    # Create technical specifications template
    cat > "$research_dir/technical_specifications.md" << EOF
# Technical Specifications for $plugin_name

## Overview
Detailed technical specifications derived from research for the $plugin_name plugin.

## Plugin Specifications
[To be populated during research phase]

## DSP Architecture
[To be populated during research phase]

## Performance Requirements
[To be populated during research phase]

## Quality Assurance
[To be populated during research phase]

## Testing Strategy
[To be populated during research phase]
EOF
}

# Step 5: Set up testing framework
echo -e "${BLUE}🧪 Setting up testing framework...${NC}"
setup_testing_framework() {
    local tests_dir="$PLUGIN_DIR/tests"
    mkdir -p "$tests_dir"
    
    # Create plugin-specific smoke test
    create_plugin_smoke_test "$tests_dir" "$PLUGIN_NAME" "$PLUGIN_TYPE"
    
    # Create unit test framework
    create_unit_test_framework "$tests_dir" "$PLUGIN_NAME" "$PLUGIN_TYPE"
    
    # Create performance test
    create_performance_test "$tests_dir" "$PLUGIN_NAME"
}

# Function to create plugin-specific smoke test
create_plugin_smoke_test() {
    local tests_dir="$1"
    local plugin_name="$2" 
    local plugin_type="$3"
    
    cat > "$tests_dir/${plugin_name}_smoke_test.cpp" << EOF
#include <catch2/catch_test_macros.hpp>
#include "../Source/PluginProcessor.h"
#include "../../shared/TestUtilities.h"

TEST_CASE("$plugin_name Smoke Test", "[$plugin_name][smoke]") {
    ${plugin_name}AudioProcessor processor;
    
    SECTION("Plugin instantiation") {
        REQUIRE_NOTHROW(processor.getName());
        REQUIRE(processor.getName() == "$plugin_name");
    }
    
    SECTION("Parameter system") {
        auto& params = processor.getParameters();
        REQUIRE(params.size() > 0);
        
        for (auto* param : params) {
            REQUIRE(param != nullptr);
            REQUIRE_NOTHROW(param->getValue());
        }
    }
    
    SECTION("Audio processing preparation") {
        REQUIRE_NOTHROW(processor.prepareToPlay(48000.0, 512));
        REQUIRE(processor.getSampleRate() == 48000.0);
        REQUIRE(processor.getBlockSize() == 512);
    }
    
    SECTION("Basic audio processing") {
        processor.prepareToPlay(48000.0, 512);
        
        juce::AudioBuffer<float> buffer(2, 512);
        buffer.clear();
        
        // Add test signal
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
            for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
                buffer.setSample(channel, sample, 0.5f * sin(2.0 * M_PI * 1000.0 * sample / 48000.0));
            }
        }
        
        juce::MidiBuffer midiBuffer;
        REQUIRE_NOTHROW(processor.processBlock(buffer, midiBuffer));
        
        // Verify output is not NaN or infinite
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
            for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
                float value = buffer.getSample(channel, sample);
                REQUIRE(std::isfinite(value));
                REQUIRE(std::abs(value) <= 10.0f); // Reasonable amplitude limit
            }
        }
    }
    
    SECTION("Performance validation") {
        processor.prepareToPlay(48000.0, 512);
        
        juce::AudioBuffer<float> buffer(2, 512);
        juce::MidiBuffer midiBuffer;
        
        // Warm up
        for (int i = 0; i < 10; ++i) {
            processor.processBlock(buffer, midiBuffer);
        }
        
        // Measure processing time
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < 1000; ++i) {
            processor.processBlock(buffer, midiBuffer);
        }
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        double avgTimePerBlock = duration.count() / 1000.0;
        double maxAllowedTime = (512.0 / 48000.0) * 1000000.0 * 0.1; // 10% of real-time
        
        INFO("Average processing time: " << avgTimePerBlock << " microseconds");
        INFO("Maximum allowed time: " << maxAllowedTime << " microseconds");
        REQUIRE(avgTimePerBlock < maxAllowedTime);
    }
}

TEST_CASE("$plugin_name $plugin_type-specific tests", "[$plugin_name][$plugin_type]") {
    ${plugin_name}AudioProcessor processor;
    processor.prepareToPlay(48000.0, 512);
    
    // Add plugin-type-specific tests here
    // Examples based on plugin type:
    
    SECTION("Audio quality validation") {
        juce::AudioBuffer<float> input(2, 512);
        juce::AudioBuffer<float> output(2, 512);
        
        // Generate test signal appropriate for $plugin_type
        TestUtilities::generateTestSignal(input, TestUtilities::SignalType::Sine, 1000.0f, 48000.0);
        
        output.makeCopyOf(input);
        juce::MidiBuffer midiBuffer;
        processor.processBlock(output, midiBuffer);
        
        // Verify processing maintains reasonable audio characteristics
        auto inputRMS = TestUtilities::calculateRMS(input);
        auto outputRMS = TestUtilities::calculateRMS(output);
        
        // Level should be reasonable (not silent, not excessive)
        REQUIRE(outputRMS > 0.001f);  // Not silent
        REQUIRE(outputRMS < 2.0f);    // Not excessive
    }
}
EOF
}

# Function to create unit test framework
create_unit_test_framework() {
    local tests_dir="$1"
    local plugin_name="$2"
    local plugin_type="$3"
    
    cat > "$tests_dir/${plugin_name}_unit_tests.cpp" << EOF
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "../Source/PluginProcessor.h"

class ${plugin_name}TestFixture {
public:
    ${plugin_name}TestFixture() {
        processor.prepareToPlay(48000.0, 512);
    }
    
    ${plugin_name}AudioProcessor processor;
};

TEST_CASE_METHOD(${plugin_name}TestFixture, "$plugin_name Parameter Tests", "[$plugin_name][parameters]") {
    
    SECTION("Parameter bounds validation") {
        auto& params = processor.getParameters();
        
        for (auto* param : params) {
            REQUIRE(param != nullptr);
            
            // Test minimum value
            param->setValue(0.0f);
            REQUIRE(param->getValue() == Catch::Approx(0.0f).margin(0.001f));
            
            // Test maximum value
            param->setValue(1.0f);
            REQUIRE(param->getValue() == Catch::Approx(1.0f).margin(0.001f));
            
            // Test middle value
            param->setValue(0.5f);
            REQUIRE(param->getValue() == Catch::Approx(0.5f).margin(0.001f));
        }
    }
    
    SECTION("Parameter automation") {
        auto& params = processor.getParameters();
        
        if (!params.empty()) {
            auto* firstParam = params[0];
            
            // Test parameter change during processing
            juce::AudioBuffer<float> buffer(2, 512);
            juce::MidiBuffer midiBuffer;
            
            firstParam->setValue(0.0f);
            processor.processBlock(buffer, midiBuffer);
            
            firstParam->setValue(1.0f);
            processor.processBlock(buffer, midiBuffer);
            
            // Should not crash or produce invalid audio
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
                for (int s = 0; s < buffer.getNumSamples(); ++s) {
                    REQUIRE(std::isfinite(buffer.getSample(ch, s)));
                }
            }
        }
    }
}

TEST_CASE_METHOD(${plugin_name}TestFixture, "$plugin_name DSP Component Tests", "[$plugin_name][dsp]") {
    
    SECTION("Silence in, appropriate out") {
        juce::AudioBuffer<float> buffer(2, 512);
        buffer.clear();
        
        juce::MidiBuffer midiBuffer;
        processor.processBlock(buffer, midiBuffer);
        
        // For most $plugin_type plugins, silence in should produce silence out
        // (unless it's a generator or adds noise)
        bool allSilent = true;
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
            for (int s = 0; s < buffer.getNumSamples(); ++s) {
                if (std::abs(buffer.getSample(ch, s)) > 0.001f) {
                    allSilent = false;
                    break;
                }
            }
        }
        
        // Most effects should pass silence (adjust this test based on plugin type)
        INFO("Plugin type: $plugin_type");
        // REQUIRE(allSilent); // Uncomment if plugin should pass silence
    }
    
    SECTION("DC offset handling") {
        juce::AudioBuffer<float> buffer(2, 512);
        
        // Fill with DC offset
        buffer.clear();
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
            for (int s = 0; s < buffer.getNumSamples(); ++s) {
                buffer.setSample(ch, s, 0.5f);
            }
        }
        
        juce::MidiBuffer midiBuffer;
        processor.processBlock(buffer, midiBuffer);
        
        // Output should be finite
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
            for (int s = 0; s < buffer.getNumSamples(); ++s) {
                REQUIRE(std::isfinite(buffer.getSample(ch, s)));
            }
        }
    }
    
    SECTION("Sample rate changes") {
        // Test different sample rates
        std::vector<double> sampleRates = {44100.0, 48000.0, 88200.0, 96000.0};
        
        for (auto sr : sampleRates) {
            REQUIRE_NOTHROW(processor.prepareToPlay(sr, 512));
            
            juce::AudioBuffer<float> buffer(2, 512);
            juce::MidiBuffer midiBuffer;
            
            // Should not crash at any supported sample rate
            REQUIRE_NOTHROW(processor.processBlock(buffer, midiBuffer));
        }
    }
}

// Add plugin-type-specific unit tests here
TEST_CASE_METHOD(${plugin_name}TestFixture, "$plugin_name $plugin_type-specific Unit Tests", "[$plugin_name][$plugin_type][unit]") {
    
    SECTION("Placeholder for $plugin_type-specific tests") {
        // Add tests specific to the plugin type:
        // - Filter plugins: frequency response tests
        // - Dynamic plugins: compression ratio tests  
        // - Modulation plugins: LFO behavior tests
        // - Saturation plugins: harmonic content tests
        // - Delay plugins: delay time accuracy tests
        // - Synthesis plugins: oscillator tests
        
        REQUIRE(true); // Placeholder - implement actual tests
    }
}
EOF
}

# Function to create performance test
create_performance_test() {
    local tests_dir="$1"
    local plugin_name="$2"
    
    cat > "$tests_dir/${plugin_name}_performance_test.cpp" << EOF
#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include "../Source/PluginProcessor.h"
#include <random>

TEST_CASE("$plugin_name Performance Benchmarks", "[$plugin_name][performance][!benchmark]") {
    ${plugin_name}AudioProcessor processor;
    
    SECTION("Processing performance at 48kHz") {
        processor.prepareToPlay(48000.0, 512);
        
        juce::AudioBuffer<float> buffer(2, 512);
        juce::MidiBuffer midiBuffer;
        
        // Fill buffer with realistic audio content
        std::random_device rd;
        std::mt19937 gen(rd());
        std::normal_distribution<float> dist(0.0f, 0.1f);
        
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
            for (int s = 0; s < buffer.getNumSamples(); ++s) {
                buffer.setSample(ch, s, dist(gen));
            }
        }
        
        BENCHMARK("Process 512 samples at 48kHz") {
            processor.processBlock(buffer, midiBuffer);
            return buffer.getSample(0, 0);
        };
    }
    
    SECTION("Processing performance at 96kHz") {
        processor.prepareToPlay(96000.0, 512);
        
        juce::AudioBuffer<float> buffer(2, 512);
        juce::MidiBuffer midiBuffer;
        
        BENCHMARK("Process 512 samples at 96kHz") {
            processor.processBlock(buffer, midiBuffer);
            return buffer.getSample(0, 0);
        };
    }
    
    SECTION("Parameter update performance") {
        processor.prepareToPlay(48000.0, 512);
        
        auto& params = processor.getParameters();
        if (!params.empty()) {
            auto* param = params[0];
            
            BENCHMARK("Parameter update") {
                param->setValue(static_cast<float>(rand()) / RAND_MAX);
                return param->getValue();
            };
        }
    }
    
    SECTION("Multiple instance performance") {
        std::vector<std::unique_ptr<${plugin_name}AudioProcessor>> processors;
        std::vector<juce::AudioBuffer<float>> buffers;
        
        const int numInstances = 10;
        
        for (int i = 0; i < numInstances; ++i) {
            processors.push_back(std::make_unique<${plugin_name}AudioProcessor>());
            processors.back()->prepareToPlay(48000.0, 512);
            
            buffers.emplace_back(2, 512);
            buffers.back().clear();
        }
        
        BENCHMARK("Process 10 instances simultaneously") {
            juce::MidiBuffer midiBuffer;
            for (int i = 0; i < numInstances; ++i) {
                processors[i]->processBlock(buffers[i], midiBuffer);
            }
            return buffers[0].getSample(0, 0);
        };
    }
}

TEST_CASE("$plugin_name Memory Usage", "[$plugin_name][memory]") {
    
    SECTION("Memory allocation during processing") {
        ${plugin_name}AudioProcessor processor;
        processor.prepareToPlay(48000.0, 512);
        
        juce::AudioBuffer<float> buffer(2, 512);
        juce::MidiBuffer midiBuffer;
        
        // Process multiple blocks to ensure no memory leaks
        for (int i = 0; i < 10000; ++i) {
            processor.processBlock(buffer, midiBuffer);
        }
        
        // This test mainly ensures we don't crash from memory issues
        REQUIRE(true);
    }
    
    SECTION("State save/restore memory") {
        ${plugin_name}AudioProcessor processor;
        
        // Save state
        juce::MemoryBlock state;
        processor.getStateInformation(state);
        
        // Restore state
        REQUIRE_NOTHROW(processor.setStateInformation(state.getData(), state.getSize()));
        
        REQUIRE(state.getSize() > 0);
        REQUIRE(state.getSize() < 1024 * 1024); // Should be less than 1MB
    }
}
EOF
}

# Call the functions
create_docs_structure
setup_testing_framework

# Step 6: Update plugins/CMakeLists.txt to include new plugin
echo -e "${BLUE}🔧 Updating build configuration...${NC}"
if ! grep -q "add_subdirectory($PLUGIN_NAME)" plugins/CMakeLists.txt; then
    echo "add_subdirectory($PLUGIN_NAME)" >> plugins/CMakeLists.txt
    echo -e "${GREEN}✅ Added $PLUGIN_NAME to plugins/CMakeLists.txt${NC}"
fi

# Step 7: Create README for the plugin
echo -e "${BLUE}📖 Creating plugin documentation...${NC}"
cat > "$PLUGIN_DIR/README.md" << EOF
# $PLUGIN_NAME

## Overview
$PLUGIN_NAME is a $plugin_type plugin built with the Tyler Audio framework, focusing on professional quality and efficient performance.

## Development Workflow

### Quick Start
1. **Research Phase**: Start with \`docs/claude/1_juce_research.md\`
2. **Specification**: Continue to \`docs/claude/2_juce_specification.md\`  
3. **Implementation**: Follow \`docs/claude/3_juce_checklist.md\`
4. **Execution**: Use \`docs/claude/4_juce_build.md\`

### Testing
\`\`\`bash
# Run all plugin tests
ctest -R $PLUGIN_NAME

# Run smoke tests specifically
ctest -R "${PLUGIN_NAME}_smoke"

# Run performance benchmarks
ctest -R "${PLUGIN_NAME}_performance"
\`\`\`

### Building
\`\`\`bash
# Build this plugin specifically
cmake --build build --target $PLUGIN_NAME

# Deploy to system plugin folder
./scripts/deploy-plugins.sh
\`\`\`

## Plugin Information
- **Plugin Code**: $PLUGIN_CODE
- **Plugin Type**: $plugin_type
- **Formats**: VST3, AU
- **Channels**: Stereo

## Documentation Structure
- \`docs/claude/\` - Structured development commands
- \`docs/research/\` - Research findings and references
- \`tests/\` - Comprehensive test suite
- \`Source/\` - Plugin source code

## Development Standards
- **Performance Target**: <1% CPU usage at 48kHz/512 samples
- **Testing**: 100% TDD with comprehensive test coverage
- **Code Quality**: Professional standards with automated validation
- **Documentation**: Complete technical and user documentation

## Getting Started
Run the research command to begin structured development:
\`\`\`bash
# Follow the four-prompt development system
# 1. Research: docs/claude/1_juce_research.md
# 2. Specification: docs/claude/2_juce_specification.md
# 3. Checklist: docs/claude/3_juce_checklist.md
# 4. Build: docs/claude/4_juce_build.md
\`\`\`

Generated with Tyler Audio Plugin Creation System 🎵
EOF

echo -e "${GREEN}✅ $PLUGIN_NAME created successfully with complete development environment!${NC}"
echo -e ""
echo -e "${YELLOW}🚀 Next Steps:${NC}"
echo -e "${BLUE}1.${NC} Start with research: ${YELLOW}docs/claude/1_juce_research.md${NC}"
echo -e "${BLUE}2.${NC} Follow the four-prompt development system for structured implementation"
echo -e "${BLUE}3.${NC} Run tests: ${YELLOW}ctest -R $PLUGIN_NAME${NC}"
echo -e "${BLUE}4.${NC} Build plugin: ${YELLOW}cmake --build build --target $PLUGIN_NAME${NC}"
echo -e "${BLUE}5.${NC} Deploy for testing: ${YELLOW}./scripts/deploy-plugins.sh${NC}"
echo -e ""
echo -e "${GREEN}📊 Plugin Summary:${NC}"
echo -e "  • Plugin Name: ${YELLOW}$PLUGIN_NAME${NC}"
echo -e "  • Plugin Code: ${YELLOW}$PLUGIN_CODE${NC}"
echo -e "  • Plugin Type: ${YELLOW}$plugin_type${NC}"
echo -e "  • Directory: ${YELLOW}$PLUGIN_DIR${NC}"
echo -e ""
echo -e "${BLUE}📁 Created Structure:${NC}"
echo -e "  • Complete source code with professional architecture"
echo -e "  • Four-prompt Claude development system"
echo -e "  • Comprehensive research templates"
echo -e "  • Extensive testing framework (smoke, unit, performance)"
echo -e "  • Documentation and build integration"
echo -e ""
echo -e "${GREEN}🎵 Ready for professional plugin development!${NC}"