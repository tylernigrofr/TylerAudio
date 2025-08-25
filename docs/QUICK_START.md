# TylerAudio Quick Start Guide

**Get from zero to working plugin in 60 seconds.** This guide gets you creating, building, and testing JUCE audio plugins immediately.

## Prerequisites (One-Time Setup)

### Essential Tools
```bash
# macOS
brew install cmake ninja ccache
# Optional but recommended
brew install pluginval

# Verify installation
cmake --version    # Should be 3.25+
ninja --version    # Any recent version
ccache --version   # Any recent version
```

### Audio Routing Setup
```bash
# Install BlackHole for audio routing (macOS)
brew install blackhole-2ch

# After installation:
# 1. Create aggregate device in Audio MIDI Setup
# 2. Add BlackHole 2ch + your interface
# 3. Set as default output in System Preferences
```

## 60-Second Plugin Creation

### 1. Create Your Plugin (10 seconds)
```bash
# Create new plugin from template
./tools/new-plugin.sh MyAwesomeEffect

# Creates fully configured plugin with:
# ✅ Professional CMake setup
# ✅ JUCE integration
# ✅ Basic gain control + bypass
# ✅ GUI ready for customization
```

### 2. Build & Launch (15 seconds)
```bash
# Fast development build with auto-launch
./tools/dev-build.sh MyAwesomeEffect --launch

# This will:
# ✅ Build in 3-4 seconds (vs 45-60s traditional)
# ✅ Auto-install to system
# ✅ Launch standalone app
# ✅ Route audio through BlackHole
```

### 3. Hear Your Plugin (30 seconds)
Your plugin is now running! You'll hear a clean audio path. Time to make some noise:

1. **Generate test tone** (for immediate feedback):
   - Open `plugins/MyAwesomeEffect/Source/PluginProcessor.cpp`
   - Find the `processBlock()` method
   - Add a test sine wave (see code below)
   - Save file

2. **Rebuild instantly**:
   ```bash
   ./tools/dev-build.sh MyAwesomeEffect
   # Builds in ~3 seconds, hot-swaps the plugin
   ```

3. **Hear the change immediately** - your test tone is now playing!

### Test Tone Example Code
```cpp
// Add this to processBlock() in PluginProcessor.cpp
void PluginProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    // Your existing code...
    
    // TEMPORARY: Add test tone for development
    static float phase = 0.0f;
    const float frequency = 440.0f;
    const float sampleRate = static_cast<float>(getSampleRate());
    const float phaseIncrement = frequency * 2.0f * juce::MathConstants<float>::pi / sampleRate;
    
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        float testTone = 0.1f * std::sin(phase);
        
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        {
            // Mix test tone with input (or replace for testing)
            buffer.addSample(channel, sample, testTone);
        }
        
        phase += phaseIncrement;
        if (phase > 2.0f * juce::MathConstants<float>::pi)
            phase -= 2.0f * juce::MathConstants<float>::pi;
    }
    
    // Remember to remove test tone before release!
}
```

### 4. Rapid Iteration Workflow (5 seconds per cycle)

You now have a **3-4 second iteration cycle**:

1. **Edit code** - Modify DSP, add parameters, tweak GUI
2. **Save file** - Auto-detected by file watching (optional)
3. **Build** - `./tools/dev-build.sh MyAwesomeEffect` (3-4 seconds)
4. **Hear changes** - Instant feedback in running plugin

This is **10x faster** than traditional JUCE development!

---

## Development Patterns

### Core Files You'll Edit

| File | Purpose | What You'll Do |
|------|---------|----------------|
| `PluginProcessor.cpp` | **DSP Logic** | Add filters, effects, synthesis |
| `PluginProcessor.h` | **Parameters** | Define controls, states |
| `PluginEditor.cpp` | **GUI** | Create sliders, buttons, displays |
| `PluginEditor.h` | **Layout** | Define interface structure |

### Common Development Tasks

#### Adding Parameters
```cpp
// In PluginProcessor constructor
addParameter(cutoffFreq = new juce::AudioParameterFloat(
    "cutoff", "Cutoff Frequency",
    juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.3f),
    1000.0f));
```

#### Using Parameters in DSP
```cpp
// In processBlock()
float cutoff = cutoffFreq->get();
// Apply to your filter/effect
```

#### Quick GUI Addition
```cpp
// In PluginEditor constructor
addAndMakeVisible(cutoffSlider);
cutoffSlider.setSliderStyle(juce::Slider::Rotary);
cutoffAttachment.reset(new SliderAttachment(valueTreeState, "cutoff", cutoffSlider));
```

---

## Testing & Validation

### Quick Validation
```bash
# Test your plugin thoroughly
./tools/validate-plugin.sh MyAwesomeEffect

# Test specific format
./tools/validate-plugin.sh MyAwesomeEffect --format VST3 --strictness 8
```

### Load in DAW
```bash
# Build release version
cmake --preset release
cmake --build build-release --target MyAwesomeEffect --parallel 4

# Plugins auto-install to:
# VST3: ~/Library/Audio/Plug-Ins/VST3/
# AU: ~/Library/Audio/Plug-Ins/Components/
```

---

## Packaging for Release

### Create Distribution Package
```bash
# Package for distribution
./tools/package-release.sh MyAwesomeEffect --version 1.0.0

# Creates:
# releases/MyAwesomeEffect_v1.0.0_macOS.zip
# ├── MyAwesomeEffect.vst3/
# ├── MyAwesomeEffect.component/
# ├── MyAwesomeEffect.app/
# └── PACKAGE_INFO.txt
```

---

## Project Structure Overview

```
TylerAudio/
├── tools/                    # Development scripts
│   ├── new-plugin.sh        # Create new plugins
│   ├── dev-build.sh         # Rapid development builds
│   ├── validate-plugin.sh   # Plugin validation
│   └── package-release.sh   # Release packaging
├── templates/               # Plugin templates
│   └── BasicEffect/         # Clean effect template
├── plugins/                 # Your plugins
│   └── MyAwesomeEffect/     # Generated plugin
├── framework/               # Shared utilities
├── docs/                    # Documentation
└── CMakeLists.txt          # Root build configuration
```

---

## Troubleshooting Common Issues

### Build Fails
```bash
# Clean and rebuild
rm -rf build-dev build-release
./tools/dev-build.sh MyAwesomeEffect
```

### Plugin Not Found in DAW
```bash
# Check installation
ls ~/Library/Audio/Plug-Ins/VST3/MyAwesome*
ls ~/Library/Audio/Plug-Ins/Components/MyAwesome*

# Force rebuild and install
./tools/dev-build.sh MyAwesomeEffect --clean
```

### Audio Not Working
1. Check BlackHole is set as output device
2. Verify aggregate device includes your interface
3. Check DAW audio routing
4. Test with simple sine wave in `processBlock()`

### Slow Builds
```bash
# Check ccache is working
ccache -s

# Should show cache hits/misses
# If no hits, ccache isn't working properly
```

---

## Next Steps

### Learn More
- **Advanced DSP**: Explore `framework/` for DSP utilities
- **GUI Customization**: Study JUCE LookAndFeel classes  
- **Parameters**: Read JUCE AudioProcessorValueTreeState docs
- **Testing**: Add unit tests in `framework/TestFramework/`

### Best Practices
- **Remove test tones** before release
- **Add parameter automation** for all controls
- **Validate with pluginval** at strictness 8+
- **Test in multiple DAWs** (Logic, Reaper, Ableton)
- **Profile CPU usage** in complex processing

### Community
- Check `CLAUDE.md` for complete development workflows
- Browse `templates/` for different plugin types
- Study existing plugins for patterns

---

**You're ready to build professional audio plugins! 🎛️**

The rapid iteration system gives you JSFX-like development speed with the power and flexibility of JUCE and C++. Happy coding! 🎵