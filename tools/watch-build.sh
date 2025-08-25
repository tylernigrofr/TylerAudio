#!/bin/bash

# TylerAudio File Watcher for Automatic Rebuilds
# Watches source files and rebuilds on changes for JSFX-like iteration

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# Configuration
PLUGIN_NAME="${1:-TingeTape}"
WATCH_PATHS=(
    "plugins/${PLUGIN_NAME}/Source"
    "shared"
    "plugins/${PLUGIN_NAME}/CMakeLists.txt"
    "CMakeLists.txt"
)
DEBOUNCE_SECONDS=1
LAST_BUILD_TIME=0

echo -e "${BLUE}👁️  TylerAudio File Watcher${NC}"
echo -e "${BLUE}Plugin: ${PLUGIN_NAME}${NC}"
echo -e "${BLUE}Watching paths: ${WATCH_PATHS[*]}${NC}"

# Function to check if fswatch is available (macOS)
check_dependencies() {
    if command -v fswatch >/dev/null 2>&1; then
        return 0
    elif command -v inotifywait >/dev/null 2>&1; then
        return 0
    else
        echo -e "${YELLOW}Installing file watching tool...${NC}"
        if [[ "$OSTYPE" == "darwin"* ]]; then
            if command -v brew >/dev/null 2>&1; then
                brew install fswatch
            else
                echo -e "${RED}Please install Homebrew first, then run: brew install fswatch${NC}"
                exit 1
            fi
        else
            echo -e "${RED}Please install inotify-tools: sudo apt install inotify-tools${NC}"
            exit 1
        fi
    fi
}

# Build function with debouncing
do_build() {
    local current_time=$(date +%s)
    
    # Debounce: don't build if less than DEBOUNCE_SECONDS have passed
    if (( current_time - LAST_BUILD_TIME < DEBOUNCE_SECONDS )); then
        return 0
    fi
    
    LAST_BUILD_TIME=$current_time
    
    echo -e "\n${YELLOW}📁 File change detected, rebuilding...${NC}"
    echo -e "${BLUE}$(date '+%H:%M:%S')${NC}"
    
    # Run the rapid build
    if ./dev-build.sh "${PLUGIN_NAME}"; then
        echo -e "${GREEN}✅ Auto-rebuild successful${NC}"
        
        # Optional: send system notification
        if command -v osascript >/dev/null 2>&1; then
            osascript -e "display notification \"${PLUGIN_NAME} rebuilt successfully\" with title \"TylerAudio Dev Watch\""
        elif command -v notify-send >/dev/null 2>&1; then
            notify-send "TylerAudio Dev Watch" "${PLUGIN_NAME} rebuilt successfully"
        fi
    else
        echo -e "${RED}❌ Auto-rebuild failed${NC}"
        
        # Send error notification
        if command -v osascript >/dev/null 2>&1; then
            osascript -e "display notification \"Build failed - check terminal\" with title \"TylerAudio Dev Watch\" sound name \"Basso\""
        elif command -v notify-send >/dev/null 2>&1; then
            notify-send -u critical "TylerAudio Dev Watch" "Build failed - check terminal"
        fi
    fi
    
    echo -e "${BLUE}👁️  Watching for changes... (Ctrl+C to stop)${NC}"
}

# Check dependencies first
check_dependencies

# Initial build
echo -e "${YELLOW}🔨 Initial build...${NC}"
./dev-build.sh "${PLUGIN_NAME}"

echo -e "${BLUE}👁️  Watching for changes... (Ctrl+C to stop)${NC}"

# Watch for file changes
if command -v fswatch >/dev/null 2>&1; then
    # macOS with fswatch
    fswatch -o "${WATCH_PATHS[@]}" | while read num; do
        do_build
    done
elif command -v inotifywait >/dev/null 2>&1; then
    # Linux with inotify
    inotifywait -m -r -e close_write,moved_to,create "${WATCH_PATHS[@]}" --format '%w%f' | while read file; do
        # Only trigger on relevant file extensions
        if [[ "$file" =~ \.(cpp|h|hpp|c|mm|cmake|txt)$ ]]; then
            do_build
        fi
    done
else
    echo -e "${RED}❌ No file watching tool available${NC}"
    exit 1
fi