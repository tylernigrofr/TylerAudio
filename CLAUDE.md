# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

### Primary Build Commands
- **Full build**: `build.bat` (Windows) or `./build.sh` (Unix)
- **Manual build**: `cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build --config Release --parallel 4`
- **Clean build**: Delete `build/` directory, then rebuild

### Build Artifacts Location
Plugin artifacts are generated in: `build/plugins/{PluginName}/{PluginName}_artefacts/Release/`
- `VST3/` - VST3 plugins (.vst3)
- `AU/` - Audio Units (.component, macOS only) 
- `Standalone/` - Standalone applications (.exe/.app)

## Architecture Overview

### Monorepo Structure
This is a JUCE-based audio plugin monorepo using CMake with FetchContent to automatically download JUCE 8.0.3. The architecture is designed for scalable plugin development:

### Root Level Architecture
- **Root CMakeLists.txt**: Configures JUCE framework, adds required JUCE modules, and includes plugins subdirectory
- **JUCE Integration**: Uses FetchContent to fetch JUCE 8.0.3 from GitHub, enabling all standard audio plugin modules
- **Plugin Registration**: New plugins are added by creating subdirectories in `plugins/` and updating `plugins/CMakeLists.txt`

### Plugin Architecture Pattern
Each plugin follows the standard JUCE plugin pattern:
- **CMakeLists.txt**: Defines plugin using `juce_add_plugin()` with metadata (company name "Tyler Audio", manufacturer code "Tylr")
- **Source/PluginProcessor.h/cpp**: Core audio processing logic inheriting from `juce::AudioProcessor`
- **Source/PluginEditor.h/cpp**: GUI implementation inheriting from `juce::AudioProcessorEditor`
- **Plugin Formats**: Configured for VST3, AU, and Standalone builds
- **Parameter System**: Uses JUCE's AudioParameterFloat for automation-compatible parameters

### Shared Code Architecture
- **TylerAudio namespace**: All shared utilities are namespaced under `TylerAudio::`
- **Constants**: Common default values (window dimensions, gain levels)
- **Utils**: Audio utility functions (dB/gain conversions)
- **Header-only design**: Shared code is implemented in headers for easy inclusion

### Build System Architecture
- **Two-tier CMake**: Root CMakeLists.txt handles JUCE setup, individual plugin CMakeLists.txt handle plugin-specific configuration
- **JUCE Module System**: Uses `juce_add_modules()` to include required JUCE components
- **Cross-platform**: Single CMake configuration works on Windows (Visual Studio), macOS (Xcode), and Linux (GCC/Clang)

## Plugin Development Workflow

### Adding New Plugins
1. Copy `plugins/ExamplePlugin/` to `plugins/{NewPluginName}/`
2. Update `plugins/{NewPluginName}/CMakeLists.txt` - change `PLUGIN_NAME`, `PLUGIN_CODE`, and `PRODUCT_NAME`
3. Add `add_subdirectory({NewPluginName})` to `plugins/CMakeLists.txt`
4. Modify source files in `Source/` directory
5. Update unique plugin codes: `PLUGIN_MANUFACTURER_CODE` and `PLUGIN_CODE` must be unique 4-character codes

### JUCE Plugin Configuration
Key CMakeLists.txt parameters for plugins:
- `COMPANY_NAME "Tyler Audio"` - standardized across all plugins
- `PLUGIN_MANUFACTURER_CODE Tylr` - 4-char manufacturer identifier
- `PLUGIN_CODE` - unique 4-char plugin identifier (must be unique per plugin)
- `FORMATS AU VST3 Standalone` - supported plugin formats
- `COPY_PLUGIN_AFTER_BUILD TRUE` - automatically installs plugins after build

### Parameter Management
- Use `AudioParameterFloat` for automatable parameters
- Add parameters in processor constructor with `addParameter()`
- Access parameter values in `processBlock()` using pointer dereference: `*parameterName`

### CI/CD Integration
- GitHub Actions automatically builds on all platforms
- Artifacts are uploaded for each platform with format-specific organization
- Pipeline triggers on pushes to main/master/develop branches and pull requests

## Development Philosophy

### Core Principles
- **Product-First Engineering**: Start every plugin with "Who is this for? What sonic pain are we ending?"
- **Quality First**: Prioritize audio fidelity, stability, and deterministic DSP over premature optimization
- **Fail Fast**: Propose incremental, test-driven changes; never implement >200 LOC without testing
- **Evidence-Driven**: Back recommendations with citations to official JUCE docs, audio research, or benchmark data

### Definition of Rock-Solid Plugin Quality

| Pillar | Target Metric |
|--------|---------------|
| **Sound** | <0.1 dB deviation vs reference; no denormals under -300 dBFS |
| **Performance** | <1% CPU per mono instance at 48 kHz on modern hardware |
| **Stability** | 0 crashes in extended pluginval testing |
| **UX** | First-use to "makes sound I love" ≤ 60 seconds |

## C++ Coding Rules for Audio Plugins

### Realtime Safety (Critical)
- **No heap allocations in `processBlock()`** - use pre-allocated buffers and `juce::dsp::AudioBlock`
- **Lock-free DSP code** - all audio thread code must be lock-free
- **UI→DSP communication** - use `std::atomic` or `juce::AudioProcessorValueTreeState` only
- **Exception safety** - use `noexcept` on audio processing functions

### Modern C++ Practices
- **Naming conventions**: PascalCase classes, camelCase functions, kConstLikeThis constants
- **Type safety**: Use `[[nodiscard]]` for functions returning values that must be checked
- **RAII patterns**: Automatic resource management, avoid manual memory management
- **Compile-time safety**: Prefer `constexpr` and template metaprogramming where applicable

### DSP Code Structure
```cpp
// Example: Realtime-safe parameter access
void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) noexcept
{
    const auto gain = gainParameter->load(); // atomic read
    
    // Use JUCE's AudioBlock for efficient processing
    auto block = juce::dsp::AudioBlock<float>(buffer);
    block *= gain;
}
```

## Testing Strategy

### Mandatory Testing
- **DSP Golden Tests**: Feed impulse responses, verify magnitude/phase characteristics against reference
- **Fuzz Testing**: Random input testing to catch denormals, NaNs, and edge cases
- **pluginval**: Run strictness level 10 on all built binaries before release
- **Performance Benchmarks**: Measure CPU usage regression between versions

### Test Implementation Pattern
```cpp
// Example test structure for DSP components
TEST_CASE("GainProcessor maintains audio fidelity")
{
    GainProcessor processor;
    processor.setGain(0.5f);
    
    // Golden reference test
    auto inputBuffer = generateTestTone(440.0f, 1.0f, 48000);
    auto expectedOutput = inputBuffer * 0.5f;
    
    processor.process(inputBuffer);
    
    REQUIRE(bufferMatchesWithTolerance(inputBuffer, expectedOutput, 0.0001f));
}
```

## Decision-Making Framework

### When Adding New Features
1. **Collect Options**: List at least 2 viable approaches with pros/cons
2. **Score Against KPIs**: Rate by audio quality, CPU efficiency, maintainability, UX impact
3. **Choose & Document**: State decision + reasoning + fallback plan
4. **Monitor**: Set metrics to validate decision post-implementation

### Code Review Checklist
- [ ] No allocations in audio thread (`processBlock()` and callbacks)
- [ ] Thread-safe parameter access (atomics or APVTS)
- [ ] Proper handling of denormals and edge cases
- [ ] Unit tests for all public DSP functions
- [ ] Performance benchmarks for CPU-intensive operations

## Advanced Development

### Testing Commands
- **Run pluginval**: `pluginval --strictness-level 10 --validate "path/to/plugin.vst3"`
- **Profile CPU usage**: Use DAW's built-in CPU meter or tools like Intel VTune
- **Memory leak detection**: Enable AddressSanitizer in Debug builds

### Quality Assurance
- Test plugins in multiple DAWs (Reaper, Logic, Pro Tools, Ableton Live)
- Verify automation works correctly with different automation curves
- Test edge cases: very high/low sample rates, extreme parameter values
- Validate that plugin state saves/loads correctly

## Context7 MCP Integration & Research Requirements

### Mandatory Research Protocol
**CRITICAL**: Always query the Context7 MCP server for up-to-date, verified JUCE documentation before writing or modifying any code. This prevents hallucinations and ensures current best practices.

**JUCE Library ID**: `juce` (use this for efficient Context7 queries)

### Research Requirements
- **Never assume** how to implement or debug a JUCE feature without prior research via Context7
- If Context7 data is insufficient, perform detailed web search with citations
- Always check for recent JUCE API changes before implementing legacy patterns
- Verify DSP algorithms against official JUCE documentation and examples

### Example Context7 Usage
```
Before implementing audio processing:
1. Query Context7 for juce::dsp module documentation
2. Research current best practices for the specific DSP component
3. Check for performance optimization recommendations
4. Verify thread safety and realtime requirements
```

## Automated Build & Deployment

### Plugin Build Automation
After successful compilation, automatically:
1. **Build the JUCE plugin** using the appropriate build configuration
2. **Copy artifacts** to system VST3 folder for immediate testing:
   - Windows: `%COMMONPROGRAMFILES%\VST3\`
   - macOS: `~/Library/Audio/Plug-Ins/VST3/`
   - Linux: `~/.vst3/`
3. **Verify installation** by checking file presence and basic metadata

## Continuous Validation Standards

### Smoke Stress Test (Run on Every Commit)
**Purpose**: Prevent wasted development effort by catching basic failures early

1. **Plugin Loading Test**: Verify plugin can instantiate without crashes
2. **Parameter Validation**: Confirm all parameters are accessible and within bounds
3. **Basic Audio Processing**: Pump silence through plugin, ensure no exceptions
4. **Memory Leak Check**: Monitor for memory leaks during instantiation/destruction

### Audio Validation Test
**Purpose**: Ensure expected audio behavior and prevent feedback/artifacts

1. **Sine Wave Test**: Process a 440Hz sine wave, verify output characteristics
2. **Noise Test**: Process white noise, check for unexpected artifacts
3. **Silence Test**: Confirm silence in = silence out (when appropriate)
4. **Dynamic Range Test**: Verify no unexpected clipping or level changes

### Performance Benchmark Test
**Purpose**: Catch performance regressions and CPU spikes

1. **CPU Usage Measurement**: Monitor processing time per audio buffer
2. **Memory Usage Tracking**: Check for excessive memory allocation
3. **Realtime Safety**: Verify no blocking operations in audio thread
4. **Regression Detection**: Fail if performance degrades >15% from baseline

### UI Validation (When Applicable)
**Purpose**: Ensure professional, functional user interface

1. **Element Overlap Detection**: Check for overlapping UI components
2. **Resizability Test**: Verify proper scaling and layout at different sizes
3. **Color/Font Consistency**: Validate against design guidelines
4. **Accessibility Check**: Ensure screen reader compatibility

## Plugin Development Workflow Integration

### Pre-Implementation Checklist
- [ ] Research completed via Context7 MCP
- [ ] Technical specification documented
- [ ] Test cases defined before implementation
- [ ] Performance benchmarks established

### Post-Implementation Validation
- [ ] Smoke test passes
- [ ] Audio validation passes  
- [ ] Performance benchmark within limits
- [ ] Plugin builds and installs successfully
- [ ] Manual testing in target DAW completed

### Commit Gate Requirements
**No commits allowed unless all tests pass:**
- Compilation successful on target platforms
- Smoke test suite passes
- Audio validation passes
- Performance within acceptable range
- No memory leaks detected