# TylerAudio Plugin Framework

**Professional JUCE audio plugin development with rapid iteration capabilities.**

Create, build, and test audio plugins in **3-4 seconds** instead of 45-60 seconds.

## Quick Start (60 seconds)

```bash
# 1. Create new plugin
./tools/new-plugin.sh MyEffect

# 2. Build & launch  
./tools/dev-build.sh MyEffect --launch

# 3. Edit code, rebuild in ~3 seconds
./tools/dev-build.sh MyEffect
```

🎯 **[Complete Quick Start Guide →](docs/QUICK_START.md)**

## Features

- ⚡ **Rapid Iteration**: 3-4 second rebuild cycles (10x faster than traditional JUCE)
- 🛠️ **Professional Templates**: Clean, production-ready plugin templates
- 🔄 **CMake Presets**: Optimized build configurations with Ninja + ccache
- ✅ **Automated Testing**: Built-in pluginval integration
- 📦 **Easy Packaging**: One-command release packaging
- 🎛️ **Cross-Platform**: VST3, AU, Standalone on macOS, Windows, Linux

## Project Structure

```
TylerAudio/
├── tools/           # Development scripts
├── templates/       # Plugin templates  
├── plugins/         # Your plugins
├── framework/       # Shared utilities
├── docs/           # Documentation
└── CLAUDE.md       # Complete development guide
```

## Development Workflow

```bash
# Create plugin
./tools/new-plugin.sh MyReverb

# Rapid development (3-4s cycles)
./tools/dev-build.sh MyReverb --launch

# Validate thoroughly  
./tools/validate-plugin.sh MyReverb

# Package for release
./tools/package-release.sh MyReverb --version 1.0.0
```

## Documentation

- 📚 **[Quick Start Guide](docs/QUICK_START.md)** - Zero to working plugin in 60 seconds
- 🔧 **[CLAUDE.md](CLAUDE.md)** - Complete development workflows & architecture
- 📖 **[Templates](templates/)** - Plugin template documentation

## Requirements

- CMake 3.25+
- Ninja build system
- ccache (recommended)
- JUCE 8.0.3 (auto-downloaded)

## Built With

- **JUCE 8.0.3** - Cross-platform audio framework
- **CMake** - Modern build system
- **Ninja** - Fast parallel builds
- **ccache** - Compilation caching

---

**Ready to build professional audio plugins with lightning-fast iteration? 🚀**

[Get Started →](docs/QUICK_START.md)