# Tyler Audio Plugins

A modern JUCE-based monorepo for audio plugin development with complete CI/CD pipeline.

## Features

- ✅ Latest JUCE framework (8.0.3) integration
- ✅ CMake build system
- ✅ Cross-platform support (Windows, macOS, Linux)
- ✅ GitHub Actions CI/CD pipeline
- ✅ VST3, AU, and Standalone plugin formats
- ✅ Example plugin template
- ✅ Shared utilities and common code

## Project Structure

```
TylerAudio/
├── plugins/                    # Individual plugin projects
│   └── ExamplePlugin/         # Example plugin template
│       ├── Source/            # Plugin source code
│       └── CMakeLists.txt     # Plugin build configuration
├── shared/                    # Shared utilities and common code
├── tests/                     # Test files
├── .github/workflows/         # CI/CD pipeline
├── CMakeLists.txt            # Root build configuration
├── build.bat                 # Windows build script
└── build.sh                  # Unix build script
```

## Prerequisites

- CMake 3.22 or higher
- C++17 compatible compiler
- Git (for JUCE submodule)

### Platform-specific requirements:

**Windows:**
- Visual Studio 2019 or later
- Windows SDK

**macOS:**
- Xcode 12 or later
- macOS 10.15 or later

**Linux:**
- GCC 7+ or Clang 6+
- Development packages: `libasound2-dev libx11-dev libxext-dev libxrandr-dev libxss-dev libxinerama-dev libxcursor-dev libfreetype6-dev libfontconfig1-dev`

## Quick Start

1. **Clone the repository:**
   ```bash
   git clone <your-repo-url>
   cd TylerAudio
   ```

2. **Build all plugins:**
   
   **Windows:**
   ```cmd
   build.bat
   ```
   
   **macOS/Linux:**
   ```bash
   ./build.sh
   ```

3. **Manual build:**
   ```bash
   mkdir build && cd build
   cmake .. -DCMAKE_BUILD_TYPE=Release
   cmake --build . --config Release --parallel 4
   ```

## Plugin Development

### Creating a New Plugin

1. Copy the `ExamplePlugin` directory in `plugins/`
2. Rename the directory and update `CMakeLists.txt`
3. Modify the plugin code in the `Source/` directory
4. Add the new plugin to `plugins/CMakeLists.txt`

### Example Plugin Features

The included example plugin demonstrates:
- Basic gain processing
- Parameter management
- Simple GUI with slider control
- Cross-platform compatibility

## Build Artifacts

After building, plugin artifacts are located in:
```
build/plugins/{PluginName}/{PluginName}_artefacts/Release/
├── VST3/           # VST3 plugins
├── AU/             # Audio Units (macOS only)
└── Standalone/     # Standalone applications
```

## CI/CD Pipeline

The GitHub Actions workflow automatically:
- Builds on Windows, macOS, and Linux
- Tests compilation for all target formats
- Uploads build artifacts
- Triggers on pushes to main branches and pull requests

## Shared Utilities

The `shared/` directory contains common utilities:
- `TylerAudioCommon.h`: Common constants and utility functions
- Shared DSP algorithms
- Common GUI components

## Development Guidelines

1. **Code Style**: Follow JUCE coding conventions
2. **Testing**: Add tests for new DSP algorithms
3. **Documentation**: Update README when adding new plugins
4. **Dependencies**: Keep external dependencies minimal

## License

[Specify your license here]

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests if applicable
5. Submit a pull request

## Support

For issues and questions, please open a GitHub issue or contact [your-email].