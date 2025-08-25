#!/bin/bash

# new-plugin.sh - Create new JUCE plugin from template
# Usage: ./new-plugin.sh <PluginName> [template]
# 
# Templates:
#   effect    - Basic audio effect (default)
#   synth     - Software synthesizer
#   utility   - Utility plugin (analyzer, etc.)

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
TEMPLATES_DIR="$PROJECT_ROOT/templates"
PLUGINS_DIR="$PROJECT_ROOT/plugins"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

usage() {
    echo "Usage: $0 <PluginName> [template]"
    echo ""
    echo "Templates:"
    echo "  effect    - Basic audio effect (default)"
    echo "  synth     - Software synthesizer"  
    echo "  utility   - Utility plugin (analyzer, etc.)"
    echo ""
    echo "Examples:"
    echo "  $0 MyReverb"
    echo "  $0 MySynth synth"
    echo "  $0 MyAnalyzer utility"
    exit 1
}

log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Generate unique 4-character plugin code
generate_plugin_code() {
    local plugin_name="$1"
    # Take first 4 characters of plugin name, uppercase
    local code=$(echo "$plugin_name" | tr '[:lower:]' '[:upper:]' | head -c 4)
    # Pad with 'X' if less than 4 characters
    while [ ${#code} -lt 4 ]; do
        code="${code}X"
    done
    echo "$code"
}

# Check if plugin name is valid
validate_plugin_name() {
    local name="$1"
    
    if [[ ! "$name" =~ ^[A-Za-z][A-Za-z0-9]*$ ]]; then
        log_error "Plugin name must start with a letter and contain only letters and numbers"
        exit 1
    fi
    
    if [ ${#name} -lt 3 ] || [ ${#name} -gt 20 ]; then
        log_error "Plugin name must be between 3 and 20 characters"
        exit 1
    fi
}

# Check if plugin already exists
check_plugin_exists() {
    local name="$1"
    local plugin_dir="$PLUGINS_DIR/$name"
    
    if [ -d "$plugin_dir" ]; then
        log_error "Plugin '$name' already exists at $plugin_dir"
        exit 1
    fi
}

# Copy and customize template
create_plugin_from_template() {
    local plugin_name="$1"
    local template="$2"
    local plugin_code=$(generate_plugin_code "$plugin_name")
    local template_dir="$TEMPLATES_DIR/BasicEffect"  # Start with BasicEffect for now
    local plugin_dir="$PLUGINS_DIR/$plugin_name"
    
    log_info "Creating plugin '$plugin_name' from template '$template'"
    log_info "Generated plugin code: $plugin_code"
    
    # Copy template directory
    cp -r "$template_dir" "$plugin_dir"
    
    # Replace template placeholders in all files
    find "$plugin_dir" -type f -name "*.txt" -o -name "*.h" -o -name "*.cpp" -o -name "*.md" | while read -r file; do
        # Replace placeholders
        sed -i.bak \
            -e "s/BasicEffect/$plugin_name/g" \
            -e "s/BSIC/$plugin_code/g" \
            -e "s/com\.tyleraudio\.basiceffect/com.tyleraudio.$(echo "$plugin_name" | tr '[:upper:]' '[:lower:]')/g" \
            "$file"
        rm "$file.bak"
    done
    
    # Update CMakeLists.txt with new plugin name and code
    local cmake_file="$plugin_dir/CMakeLists.txt"
    sed -i.bak \
        -e "s/set(PLUGIN_NAME BasicEffect)/set(PLUGIN_NAME $plugin_name)/" \
        -e "s/PLUGIN_CODE Bsic/PLUGIN_CODE $plugin_code/" \
        -e "s/PRODUCT_NAME \"BasicEffect\"/PRODUCT_NAME \"$plugin_name\"/" \
        "$cmake_file"
    rm "$cmake_file.bak"
    
    # Add plugin to main plugins CMakeLists.txt
    local plugins_cmake="$PLUGINS_DIR/CMakeLists.txt"
    if ! grep -q "add_subdirectory($plugin_name)" "$plugins_cmake"; then
        echo "add_subdirectory($plugin_name)" >> "$plugins_cmake"
        log_info "Added '$plugin_name' to plugins/CMakeLists.txt"
    fi
    
    log_success "Plugin '$plugin_name' created successfully!"
    log_info "Location: $plugin_dir"
    
    # Show next steps
    echo ""
    echo -e "${GREEN}Next Steps:${NC}"
    echo "1. Build and test your plugin:"
    echo "   ${BLUE}./tools/dev-build.sh $plugin_name --launch${NC}"
    echo ""
    echo "2. Start developing:"
    echo "   - Edit DSP logic in: ${BLUE}plugins/$plugin_name/Source/PluginProcessor.cpp${NC}"
    echo "   - Edit GUI in: ${BLUE}plugins/$plugin_name/Source/PluginEditor.cpp${NC}"
    echo ""
    echo "3. Rapid iteration workflow:"
    echo "   - Save your changes"
    echo "   - Run: ${BLUE}./tools/dev-build.sh $plugin_name${NC}"
    echo "   - Builds in ~3-4 seconds!"
}

# Main execution
main() {
    if [ $# -lt 1 ] || [ $# -gt 2 ]; then
        usage
    fi
    
    local plugin_name="$1"
    local template="${2:-effect}"
    
    # Validate inputs
    validate_plugin_name "$plugin_name"
    check_plugin_exists "$plugin_name"
    
    # Check if template exists (for now, only BasicEffect is available)
    if [ "$template" != "effect" ]; then
        log_warning "Only 'effect' template is currently available. Using BasicEffect template."
        template="effect"
    fi
    
    # Create the plugin
    create_plugin_from_template "$plugin_name" "$template"
}

main "$@"