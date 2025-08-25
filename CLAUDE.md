# TylerAudio Plugin Development Guide

**Single source of truth for audio plugin development with JUCE**

This guide provides complete workflows from plugin creation to release packaging. Perfect for both returning to the project after weeks and onboarding new developers (including Claude Code).

---

## 🚀 QUICK START - New Plugin in 60 Seconds

### Prerequisites (One-time setup)
```bash
# macOS - Install dependencies
brew install ninja ccache fswatch

# Verify JUCE development environment
xcode-select --install  # If needed
```

### Create & Test New Plugin
```bash
# 1. Create new plugin from template (30s)
./tools/new-plugin.sh MyAwesomeEQ filter

# 2. Build and launch for testing (30s)  
./tools/dev-build.sh MyAwesomeEQ --launch

# ✅ You now have a working plugin ready for development!
```

---

## 🔧 RAPID DEVELOPMENT WORKFLOW

### The 3-4 Second Iteration Cycle
```bash
# Start development session
./tools/dev-build.sh MyAwesomeEQ --launch

# Edit Source/PluginProcessor.cpp → Save file
# Automatic rebuild in 3-4 seconds → Hear changes immediately

# OR use watch mode for automatic rebuilds
./tools/watch-build.sh MyAwesomeEQ
```

### Development Cycle
1. **Edit DSP** in `Source/PluginProcessor.cpp` (processBlock method)
2. **Save file** 
3. **Rebuild** automatically or run `./tools/dev-build.sh MyAwesomeEQ`
4. **Test changes** in standalone app (3-4 second cycle)
5. **Iterate** rapidly until satisfied

### Audio Testing Setup
- **Route audio**: Use BlackHole (macOS) or VB-Cable (Windows) to route DAW → Plugin → Monitoring
- **Standalone mode**: Direct testing without DAW complexity
- **Real-time feedback**: Hear DSP changes within seconds of code changes

---

## 🧪 TESTING & VALIDATION

### Quick Testing During Development
```bash
# Quick smoke test (10-15 seconds)
./tools/validate-plugin.sh MyAwesomeEQ --quick

# Full validation (30-60 seconds)  
./tools/validate-plugin.sh MyAwesomeEQ
```

### Testing Checklist
- ✅ **Plugin loads** without crashes
- ✅ **Parameters work** (automation, GUI interaction)
- ✅ **Audio processing** functions correctly
- ✅ **No memory leaks** or performance issues
- ✅ **pluginval** passes validation
- ✅ **Cross-platform** compatibility

---

## 🎨 GUI DEVELOPMENT

### Basic GUI (Included in templates)
- Templates include **basic parameter controls**
- **Automatic parameter binding** via JUCE AudioProcessorValueTreeState
- **Professional styling** with TylerAudio look and feel

### Custom GUI Development
1. **Edit** `Source/PluginEditor.h/.cpp`
2. **Add components** (sliders, buttons, visualizers)
3. **Bind parameters** using AudioProcessorValueTreeState attachments
4. **Test responsiveness** with rapid rebuild cycle
5. **Polish design** with custom graphics and animations

### GUI Development Tips
- Use **JUCE's built-in components** for rapid prototyping
- **Separate concerns**: GUI logic in Editor, DSP in Processor
- **Test UI responsiveness** across different host environments
- **Follow accessibility** guidelines for broad compatibility

---

## 📦 PACKAGING FOR RELEASE

### Build Release Packages
```bash
# Build all formats (VST3, AU, Standalone)
./tools/package-release.sh MyAwesomeEQ

# Artifacts created in: releases/MyAwesomeEQ/
# ├── MyAwesomeEQ-v1.0.0-macOS.zip
# ├── MyAwesomeEQ-v1.0.0-Windows.zip  
# └── Documentation/
```

### Release Process
1. **Final testing** with full validation suite
2. **Version bump** in CMakeLists.txt
3. **Build release** packages for all platforms
4. **Code signing** (macOS notarization, Windows certificates)
5. **Distribution** via website, plugin stores, etc.

### Pre-Release Checklist
- ✅ All tests pass on target platforms
- ✅ Version number updated
- ✅ Documentation complete
- ✅ Code signed and notarized
- ✅ Installation tested on clean systems

---

## 🔄 COMMON WORKFLOWS

### Starting New Plugin Development
```bash
# Create plugin from appropriate template
./tools/new-plugin.sh PluginName <type>

# Types: effect, filter, dynamics, modulation, saturation, delay, synthesis
```

### Daily Development Session
```bash
# Start rapid development
./tools/dev-build.sh PluginName --launch

# Option A: Manual rebuilds (when debugging)
# Edit code → ./tools/dev-build.sh PluginName → Test

# Option B: Automatic rebuilds (when iterating)
./tools/watch-build.sh PluginName
# Edit code → Save → Auto-rebuild → Test
```

### Pre-Commit Validation
```bash
# Full validation before committing changes
./tools/validate-plugin.sh PluginName

# Commit only if all tests pass
git add . && git commit -m "Description of changes"
```

---

## 📁 PROJECT STRUCTURE

```
TylerAudio/
├── 📁 plugins/              # Active plugin development
│   └── MyPlugin/            # Your plugin source code
├── 📁 templates/            # Clean plugin templates  
│   ├── BasicEffect/         # Simple effect template
│   └── AdvancedEffect/      # Full-featured template
├── 📁 framework/            # Shared utilities & DSP components
│   ├── TylerAudioCommon.h   # Core utilities and constants
│   ├── DSPUtils/            # Reusable DSP building blocks
│   └── TestFramework/       # Testing infrastructure
├── 📁 tools/                # Development tools
│   ├── new-plugin.sh        # Create new plugin from template
│   ├── dev-build.sh         # Rapid development builds (3-4s)
│   ├── watch-build.sh       # Auto-rebuild on file changes
│   ├── validate-plugin.sh   # Testing & validation
│   └── package-release.sh   # Release packaging
├── 📁 archived-projects/    # Completed/reference projects
│   └── TingeTape/           # Reference implementation
└── 📄 CLAUDE.md            # This guide (single source of truth)
```

---

## ⚙️ BUILD SYSTEM DETAILS

### CMake Presets
- **dev-fast**: Ultra-fast iteration builds (3-4 seconds)
- **debug**: Full debug with sanitizers for thorough testing
- **release**: Optimized production builds

### Build Optimizations
- **Ninja generator**: 2-3x faster than Unix Makefiles
- **ccache**: Incremental compilation caching
- **Standalone priority**: Skip wrapper compilation during development
- **Minimal warnings**: Rapid iteration focused

### Performance Results
- **Build time**: 3-4 seconds for incremental changes
- **Edit-to-test cycle**: 5-8 seconds total
- **Compared to traditional**: 12-20x faster development cycle

---

## 🛠️ PLUGIN DEVELOPMENT PATTERNS

### Parameter Management
```cpp
// In PluginProcessor.h
std::atomic<float>* gainParameter{nullptr};
TylerAudio::Utils::SmoothingFilter gainSmoother;

// In constructor
gainParameter = parameters.getRawParameterValue("gain");

// In processBlock
float smoothedGain = gainSmoother.getNextValue();
```

### DSP Processing Template
```cpp
void processBlock(AudioBuffer<float>& buffer, MidiBuffer&) noexcept {
    // 1. Check bypass
    if (bypassParameter->load() > 0.5f) return;
    
    // 2. Process with smoothed parameters  
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
        float gain = gainSmoother.getNextValue();
        
        // 3. Process each channel
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
            float input = buffer.getSample(channel, sample);
            float output = processSample(input, gain);  // Your DSP here
            buffer.setSample(channel, sample, output);
        }
    }
}
```

### Thread Safety
- **Atomic parameters**: Use `std::atomic<float>*` for realtime access
- **Parameter smoothing**: Prevent audio clicks/pops  
- **No allocations**: In processBlock() - use pre-allocated buffers
- **Lock-free**: All audio thread code must be lock-free

---

## 🔍 DEBUGGING & TROUBLESHOOTING

### Common Issues
```bash
# Build fails → Check compiler errors
./tools/dev-build.sh PluginName 2>&1 | grep error

# Plugin won't load → Run validation
./tools/validate-plugin.sh PluginName

# Performance issues → Profile with release build
cmake --preset release && cmake --build build-release
```

### Development Tips
- **Use standalone target** for fastest iteration
- **BlackHole audio routing** for real-time testing
- **Watch mode** for continuous development
- **Quick validation** catches issues early
- **Full validation** before commits

---

## 📚 REFERENCE

### Key Files to Edit
- **`Source/PluginProcessor.cpp`**: Main DSP logic (processBlock method)
- **`Source/PluginEditor.cpp`**: GUI implementation
- **`CMakeLists.txt`**: Plugin metadata and build settings

### Essential Tools
- **`./tools/new-plugin.sh`**: Create new plugins
- **`./tools/dev-build.sh`**: Rapid development cycle
- **`./tools/validate-plugin.sh`**: Testing and validation
- **`./tools/package-release.sh`**: Release builds

### Framework Components  
- **`framework/TylerAudioCommon.h`**: Constants, utilities, parameter IDs
- **`framework/DSPUtils/`**: Reusable DSP components
- **`framework/TestFramework/`**: Testing infrastructure

This guide covers 90% of plugin development workflows. For advanced topics, see individual documentation in the `docs/` directory.

---

**🎯 Remember: The goal is rapid iteration with professional results. Use the fast development cycle for creativity, then validate thoroughly before release.**