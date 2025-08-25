#!/bin/bash
# Development environment setup script

set -e

echo "Setting up Tyler Audio development environment..."

# Detect OS
OS="$(uname -s)"
case "${OS}" in
    Linux*)     MACHINE=Linux;;
    Darwin*)    MACHINE=Mac;;
    CYGWIN*|MINGW*) MACHINE=Windows;;
    *)          MACHINE="UNKNOWN:${OS}"
esac

echo "Detected OS: ${MACHINE}"

# Install dependencies based on OS
case "${MACHINE}" in
    Linux)
        echo "Installing Linux dependencies..."
        sudo apt-get update
        sudo apt-get install -y \
            build-essential \
            cmake \
            ninja-build \
            clang-17 \
            clang-tidy-17 \
            clang-format-17 \
            cppcheck \
            libasound2-dev \
            libx11-dev \
            libxext-dev \
            libxrandr-dev \
            libxss-dev \
            libxinerama-dev \
            libxcursor-dev \
            libfreetype6-dev \
            libfontconfig1-dev \
            git
        
        # Install pluginval
        if ! command -v pluginval &> /dev/null; then
            echo "Installing pluginval..."
            wget -O pluginval.zip "https://github.com/Tracktion/pluginval/releases/latest/download/pluginval_Linux.zip"
            unzip pluginval.zip
            chmod +x pluginval
            sudo mv pluginval /usr/local/bin/
            rm pluginval.zip
        fi
        ;;
        
    Mac)
        echo "Installing macOS dependencies..."
        if ! command -v brew &> /dev/null; then
            echo "Please install Homebrew first: https://brew.sh/"
            exit 1
        fi
        
        brew install cmake ninja clang-format cppcheck
        
        # Install pluginval
        if ! command -v pluginval &> /dev/null; then
            echo "Installing pluginval..."
            wget -O pluginval.zip "https://github.com/Tracktion/pluginval/releases/latest/download/pluginval_macOS.zip"
            unzip pluginval.zip
            chmod +x pluginval
            sudo mv pluginval /usr/local/bin/
            rm pluginval.zip
        fi
        ;;
        
    Windows)
        echo "Windows setup requires manual installation:"
        echo "1. Install Visual Studio 2022 with C++ workload"
        echo "2. Install CMake from https://cmake.org/download/"
        echo "3. Install Git from https://git-scm.com/download/win"
        echo "4. Download pluginval from https://github.com/Tracktion/pluginval/releases"
        echo "5. Add tools to PATH environment variable"
        ;;
        
    *)
        echo "Unsupported OS: ${MACHINE}"
        exit 1
        ;;
esac

# Configure git hooks
if [ -d ".git" ]; then
    echo "Setting up git hooks..."
    cp scripts/pre-commit .git/hooks/pre-commit
    chmod +x .git/hooks/pre-commit
fi

# Initial build
echo "Running initial build..."
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON -GNinja
cmake --build . --parallel $(nproc 2>/dev/null || echo 4)

echo "Development environment setup complete!"
echo ""
echo "Next steps:"
echo "1. Open VS Code and install recommended extensions"
echo "2. Run 'cmake --build build' to build"
echo "3. Run 'cd build && ctest' to run tests"
echo "4. Run 'cd build && ./pluginval_tests' to validate plugins"