#!/bin/bash

# TylerAudio Rapid Development Build Script
# Optimized for fastest possible iteration cycle

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
PLUGIN_NAME="${1:-TingeTape}"
PRESET_NAME="dev-fast"
BUILD_DIR="build-dev"
STANDALONE_TARGET="${PLUGIN_NAME}_Standalone"

echo -e "${BLUE}🚀 TylerAudio Rapid Development Build${NC}"
echo -e "${BLUE}Plugin: ${PLUGIN_NAME}${NC}"

# Function to check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Check dependencies
echo -e "${YELLOW}Checking dependencies...${NC}"

if ! command_exists ninja; then
    echo -e "${YELLOW}⚠️  Ninja not found. Installing via homebrew...${NC}"
    if command_exists brew; then
        brew install ninja
    else
        echo -e "${RED}❌ Please install Ninja build system first${NC}"
        echo "  macOS: brew install ninja"
        echo "  Linux: sudo apt install ninja-build"
        exit 1
    fi
fi

if ! command_exists ccache; then
    echo -e "${YELLOW}⚠️  ccache not found. Installing for faster builds...${NC}"
    if command_exists brew; then
        brew install ccache
    else
        echo -e "${YELLOW}Install ccache for faster incremental builds:${NC}"
        echo "  macOS: brew install ccache"
        echo "  Linux: sudo apt install ccache"
    fi
fi

# Configure if needed or if CMakeCache.txt doesn't exist
if [[ ! -f "${BUILD_DIR}/CMakeCache.txt" ]] || [[ CMakePresets.json -nt "${BUILD_DIR}/CMakeCache.txt" ]]; then
    echo -e "${YELLOW}🔧 Configuring project...${NC}"
    cmake --preset="${PRESET_NAME}"
    if [[ $? -ne 0 ]]; then
        echo -e "${RED}❌ Configuration failed${NC}"
        exit 1
    fi
fi

# Build standalone target for fastest iteration
echo -e "${YELLOW}🔨 Building ${STANDALONE_TARGET}...${NC}"
start_time=$(date +%s)

# Try common standalone target names
ACTUAL_TARGET=""
if cmake --build "${BUILD_DIR}" --target="${STANDALONE_TARGET}" --parallel 2>/dev/null; then
    ACTUAL_TARGET="${STANDALONE_TARGET}"
elif cmake --build "${BUILD_DIR}" --target="${PLUGIN_NAME}" --parallel; then
    ACTUAL_TARGET="${PLUGIN_NAME}"
else
    echo -e "${RED}❌ Could not find standalone target. Available targets:${NC}"
    cmake --build "${BUILD_DIR}" --target help 2>&1 | grep -i standalone || echo "No standalone targets found"
    exit 1
fi

end_time=$(date +%s)
duration=$((end_time - start_time))

if [[ -n "$ACTUAL_TARGET" ]]; then
    build_result=0
    echo -e "${GREEN}✅ Build completed in ${duration}s${NC}"
    
    # Find and report the executable location  
    STANDALONE_PATH=$(find "${BUILD_DIR}" -name "${PLUGIN_NAME}.app" -o -name "${PLUGIN_NAME}" -o -name "${STANDALONE_TARGET}" -o -name "${STANDALONE_TARGET}.app" | head -1)
    if [[ -n "${STANDALONE_PATH}" ]]; then
        echo -e "${GREEN}🎵 Standalone executable: ${STANDALONE_PATH}${NC}"
        
        # Make it easy to launch
        echo -e "${BLUE}💡 Quick launch: ./${STANDALONE_PATH}${NC}"
        
        # Auto-launch if requested
        if [[ "$2" == "--launch" ]]; then
            echo -e "${BLUE}🚀 Launching ${STANDALONE_TARGET}...${NC}"
            "${STANDALONE_PATH}" &
        fi
    fi
else
    echo -e "${RED}❌ Build failed${NC}"
    exit 1
fi

# Optional: Run smoke test
if [[ "$2" == "--test" ]] || [[ "$3" == "--test" ]]; then
    echo -e "${YELLOW}🧪 Running smoke tests...${NC}"
    cmake --build "${BUILD_DIR}" --target="${PLUGIN_NAME}_tests" --parallel
    if command_exists ctest; then
        cd "${BUILD_DIR}"
        ctest --preset="${PRESET_NAME}" --output-on-failure
        cd ..
    fi
fi

# Optional: Install to system for DAW testing
if [[ "$2" == "--install" ]] || [[ "$3" == "--install" ]]; then
    echo -e "${YELLOW}📦 Building and installing VST3/AU for DAW testing...${NC}"
    cmake --build "${BUILD_DIR}" --target="${PLUGIN_NAME}_VST3" --parallel
    if [[ "$OSTYPE" == "darwin"* ]]; then
        cmake --build "${BUILD_DIR}" --target="${PLUGIN_NAME}_AU" --parallel
    fi
    echo -e "${GREEN}✅ Plugins installed to system directories${NC}"
fi

echo -e "${GREEN}🎉 Development build complete! Ready for rapid iteration.${NC}"

# Show ccache stats if available
if command_exists ccache; then
    echo -e "${BLUE}📊 ccache stats:${NC}"
    ccache -s | grep -E "(cache hit|cache miss|cache hit rate)"
fi