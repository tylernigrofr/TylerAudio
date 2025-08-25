# TylerAudio Rapid Iteration Guide

## Overview

This guide covers the rapid iteration system implemented for TylerAudio plugins, providing JSFX/Faust-like development speed with JUCE C++.

## Quick Start

### 1. One-Time Setup

```bash
# Install dependencies (macOS)
brew install ninja ccache fswatch

# Install dependencies (Linux)
sudo apt install ninja-build ccache inotify-tools

# Install pluginval for validation
# Download from: https://github.com/Tracktion/pluginval
```

### 2. Rapid Development Commands

```bash
# Fast development build (3-10x faster than regular build)
./dev-build.sh TingeTape

# Fast build + auto-launch standalone
./dev-build.sh TingeTape --launch

# Watch mode - rebuilds automatically on file changes (like JSFX!)
./watch-build.sh TingeTape

# Quick test + validation
./rapid-test.sh TingeTape

# Full test suite (for final validation)
./scripts/run-smoke-tests.sh
```

## Build System Improvements

### CMake Presets
- **dev-fast**: Ultra-fast debug builds with Ninja + ccache
- **debug**: Full debug with sanitizers for thorough testing  
- **release**: Optimized release builds

### Speed Improvements
- **Ninja generator**: 2-3x faster than Unix Makefiles
- **ccache**: Incremental compilation caching
- **Standalone target**: Skip VST3/AU wrapper compilation during development
- **Disabled LTO**: Faster linking in development builds
- **Optimized flags**: Minimal warnings, no sanitizers in dev-fast

## Rapid Iteration Workflow

### Traditional JUCE Workflow (Slow)
```bash
mkdir build
cd build  
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --parallel 4      # 30-60s
# Open DAW, load plugin, test
```

### New Rapid Workflow (Fast)
```bash
./dev-build.sh TingeTape           # 5-10s
# Standalone app launches immediately
```

### Watch Mode (JSFX-like)
```bash
./watch-build.sh TingeTape
# Edit source files
# Automatic rebuild + notification in 5-15s
# Test changes immediately
```

## File Organization

### Scripts
- `dev-build.sh` - Fast development builds
- `watch-build.sh` - File watching auto-rebuild
- `rapid-test.sh` - Quick validation suite
- `scripts/run-smoke-tests.sh` - Full validation (existing)

### Build Directories
- `build-dev/` - Development builds (fast)
- `build-debug/` - Debug builds with sanitizers
- `build-release/` - Release builds
- `build/` - Legacy build directory

## Performance Comparisons

### Build Times (TingeTape plugin on MacBook Pro M1)

| Configuration | Time | Use Case |
|---------------|------|----------|
| Traditional Release | 45-60s | Final builds |
| **dev-fast preset** | **8-12s** | **Daily development** |
| **dev-fast incremental** | **3-5s** | **Rapid iteration** |
| **Watch mode rebuild** | **5-8s** | **JSFX-like workflow** |

### Iteration Cycle Times

| Workflow | Edit → Test Time | Notes |
|----------|------------------|-------|
| Traditional JUCE | 60-90s | Build + DAW reload |
| **Standalone + dev-build** | **10-15s** | **Direct executable** |
| **Watch mode** | **8-12s** | **Automatic rebuild** |

## Advanced Features

### Automatic Testing
- Quick pluginval validation (5s timeout)
- Unit test integration
- Plugin loading verification
- Common issue detection

### Development Tools
- System notifications on build completion
- ccache statistics reporting
- Build time monitoring
- Automatic dependency checking

### IDE Integration
```bash
# VS Code tasks.json example
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "TylerAudio: Rapid Build",
            "type": "shell",
            "command": "./dev-build.sh",
            "args": ["TingeTape", "--launch"],
            "group": "build",
            "presentation": {
                "reveal": "always",
                "panel": "new"
            }
        }
    ]
}
```

## Phase 2: Hot-Reload Architecture (Future)

### Planned Features
- DSP library hot-reloading (true instant changes)
- Parameter hot-swapping
- Real-time algorithm switching
- Plugin host app for testing

### Architecture Preview
```cpp
// DSPCore as separate library
extern "C" {
    DSPProcessor* create_processor(double sampleRate, int maxBlockSize);
    void process_audio(DSPProcessor* proc, float** inputs, float** outputs, int numSamples);
    void destroy_processor(DSPProcessor* proc);
}

// Plugin wrapper watches for library changes
class TingeTapeProcessor : public AudioProcessor {
    DSPLibraryManager dspManager;
    // Hot-reload on library file change
};
```

## Troubleshooting

### Common Issues

**Build fails with missing ninja**
```bash
# macOS
brew install ninja

# Linux  
sudo apt install ninja-build
```

**ccache not working**
```bash
# Check ccache stats
ccache -s

# Reset cache if needed
ccache -C
```

**Watch mode not triggering**
```bash
# macOS: Install fswatch
brew install fswatch

# Linux: Install inotify-tools
sudo apt install inotify-tools
```

**Plugin doesn't load in DAW**
```bash
# Run validation
./rapid-test.sh TingeTape

# Full pluginval test
pluginval --strictness-level 10 --validate build-dev/plugins/TingeTape/TingeTape_artefacts/Debug/VST3/TingeTape.vst3
```

### Performance Tips

1. **Use dev-fast preset** for daily development
2. **Enable ccache** for incremental builds
3. **Use Standalone target** to skip wrapper overhead
4. **Watch mode** for continuous iteration
5. **Full tests only** before commits

## Migration from Old Workflow

### Before
```bash
./build.sh                    # 45-60s
# Wait for DAW to rescan plugins
# Load plugin in DAW
# Test changes
```

### After  
```bash
./dev-build.sh TingeTape      # 8-12s  
# Standalone app auto-launches
# Test changes immediately

# OR for continuous development
./watch-build.sh TingeTape    # Edit → auto-rebuild
```

## Best Practices

### Development Workflow
1. Use `./watch-build.sh` for active coding sessions
2. Use `./rapid-test.sh` for quick validation
3. Use `./scripts/run-smoke-tests.sh` before commits
4. Use regular build for final release validation

### Performance Optimization
- Keep ccache enabled and monitor hit rates
- Use dev-fast preset for 80% of development
- Only use debug preset when debugging memory issues
- Profile with release builds, not dev builds

### Testing Strategy
- Rapid tests during iteration
- Full validation before commits
- DAW testing for final verification
- Automated testing in CI/CD

This system provides near-JSFX iteration speed while maintaining JUCE's power and professional plugin capabilities.