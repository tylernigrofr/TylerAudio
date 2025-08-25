# BasicEffect Template

A clean, minimal plugin template for audio effects. Perfect starting point for simple processors like gain, filters, EQ, etc.

## Features
- ✅ **Clean architecture** - Ready for immediate development
- ✅ **Gain control** with smooth parameter changes  
- ✅ **Bypass functionality** built-in
- ✅ **Professional parameter management** using JUCE AudioProcessorValueTreeState
- ✅ **Realtime-safe processing** with atomic parameter access
- ✅ **Cross-platform** - Windows, macOS, Linux ready
- ✅ **All plugin formats** - VST3, AU, Standalone

## Quick Start

1. **Create new plugin** using tools:
   ```bash
   ./tools/new-plugin.sh MyEffect effect
   ```

2. **Start rapid development**:
   ```bash
   ./tools/dev-build.sh MyEffect --launch
   ```

3. **Edit DSP in Source/PluginProcessor.cpp** - see `processBlock()` method

4. **Iterate quickly** - save file, rebuild in 3-4 seconds, hear changes immediately

## Template Structure

```
BasicEffect/
├── Source/
│   ├── PluginProcessor.h     # Main plugin logic
│   ├── PluginProcessor.cpp   # DSP processing
│   ├── PluginEditor.h        # GUI interface  
│   └── PluginEditor.cpp      # GUI implementation
├── CMakeLists.txt            # Build configuration
└── README.md                 # This file
```

## Key Implementation Details

### Parameters
- **Gain**: 0.0-2.0x with smooth transitions
- **Bypass**: True/false toggle

### Processing
- **Thread-safe** parameter access via atomics
- **Denormal protection** built-in
- **Efficient sample-by-sample** processing
- **JUCE AudioBlock** integration ready

This template follows all TylerAudio best practices and is ready for professional development.