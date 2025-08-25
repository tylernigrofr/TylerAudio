#!/bin/bash

# package-release.sh - Package JUCE plugin for distribution
# Usage: ./package-release.sh <PluginName> [--version x.y.z] [--formats VST3,AU,Standalone]

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Default values
VERSION="1.0.0"
FORMATS="VST3,AU,Standalone"
BUILD_CONFIG="Release"

usage() {
    echo "Usage: $0 <PluginName> [options]"
    echo ""
    echo "Options:"
    echo "  --version VERSION    Version string (default: 1.0.0)"
    echo "  --formats FORMATS    Comma-separated formats (default: VST3,AU,Standalone)"
    echo "  --config CONFIG      Build configuration (default: Release)"
    echo "  --help              Show this help"
    echo ""
    echo "Examples:"
    echo "  $0 MyReverb"
    echo "  $0 MyReverb --version 2.1.0 --formats VST3,AU"
    echo "  $0 MyReverb --version 1.5.0 --config Debug"
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

# Get current git information
get_git_info() {
    local git_hash=""
    local git_branch=""
    local git_tag=""
    
    if git rev-parse --git-dir > /dev/null 2>&1; then
        git_hash=$(git rev-parse --short HEAD 2>/dev/null || echo "unknown")
        git_branch=$(git branch --show-current 2>/dev/null || echo "unknown")
        git_tag=$(git describe --tags --exact-match 2>/dev/null || echo "")
    fi
    
    echo "$git_hash|$git_branch|$git_tag"
}

# Build plugin in release mode
build_plugin() {
    local plugin_name="$1"
    local config="$2"
    
    log_info "Building $plugin_name in $config mode..."
    
    # Configure build
    cmake --preset release > /dev/null 2>&1 || {
        log_error "Failed to configure build"
        return 1
    }
    
    # Build the plugin
    if cmake --build build-release --config "$config" --target "$plugin_name" --parallel 4; then
        log_success "Build completed successfully"
        return 0
    else
        log_error "Build failed"
        return 1
    fi
}

# Find and copy plugin artifacts
package_artifacts() {
    local plugin_name="$1"
    local formats="$2"
    local version="$3"
    local package_dir="$4"
    
    local build_dir="$PROJECT_ROOT/build-release"
    local artifacts_dir="$build_dir/plugins/$plugin_name/${plugin_name}_artefacts/Release"
    
    IFS=',' read -ra FORMAT_ARRAY <<< "$formats"
    
    for format in "${FORMAT_ARRAY[@]}"; do
        format=$(echo "$format" | xargs) # trim whitespace
        
        case "$format" in
            "VST3")
                local src_path="$artifacts_dir/VST3/$plugin_name.vst3"
                if [[ -d "$src_path" ]]; then
                    cp -R "$src_path" "$package_dir/"
                    log_success "Packaged VST3: $plugin_name.vst3"
                else
                    log_warning "VST3 not found: $src_path"
                fi
                ;;
            "AU")
                local src_path="$artifacts_dir/AU/$plugin_name.component"
                if [[ -d "$src_path" ]]; then
                    cp -R "$src_path" "$package_dir/"
                    log_success "Packaged AU: $plugin_name.component"
                else
                    log_warning "AU not found: $src_path"
                fi
                ;;
            "Standalone")
                local src_path="$artifacts_dir/Standalone/$plugin_name.app"
                if [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "win32" ]]; then
                    src_path="$artifacts_dir/Standalone/$plugin_name.exe"
                elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
                    src_path="$artifacts_dir/Standalone/$plugin_name"
                fi
                
                if [[ -e "$src_path" ]]; then
                    cp -R "$src_path" "$package_dir/"
                    log_success "Packaged Standalone: $(basename "$src_path")"
                else
                    log_warning "Standalone not found: $src_path"
                fi
                ;;
            *)
                log_warning "Unknown format: $format"
                ;;
        esac
    done
}

# Create package info file
create_package_info() {
    local plugin_name="$1"
    local version="$2"
    local formats="$3"
    local package_dir="$4"
    
    local git_info=$(get_git_info)
    IFS='|' read -r git_hash git_branch git_tag <<< "$git_info"
    
    local info_file="$package_dir/PACKAGE_INFO.txt"
    
    cat > "$info_file" << EOF
Plugin Package Information
========================

Plugin Name: $plugin_name
Version: $version
Build Date: $(date -u +"%Y-%m-%d %H:%M:%S UTC")
Build Host: $(hostname)
Build OS: $(uname -s) $(uname -r)

Git Information:
- Hash: $git_hash
- Branch: $git_branch
- Tag: $git_tag

Included Formats: $formats

Installation Instructions:
- VST3: Copy .vst3 to your DAW's VST3 folder
- AU: Copy .component to ~/Library/Audio/Plug-Ins/Components/ (macOS)
- Standalone: Run directly as an application

For support, visit: https://github.com/tylernigro/TylerAudio
EOF
    
    log_info "Created package info file"
}

# Create ZIP archive
create_archive() {
    local plugin_name="$1"
    local version="$2"
    local package_dir="$3"
    local releases_dir="$4"
    
    local platform=""
    case "$OSTYPE" in
        "darwin"*) platform="macOS" ;;
        "linux-gnu"*) platform="Linux" ;;
        "msys"|"win32") platform="Windows" ;;
        *) platform="Unknown" ;;
    esac
    
    local archive_name="${plugin_name}_v${version}_${platform}.zip"
    local archive_path="$releases_dir/$archive_name"
    
    log_info "Creating archive: $archive_name"
    
    # Create archive from package directory
    (cd "$(dirname "$package_dir")" && zip -r "$archive_path" "$(basename "$package_dir")" > /dev/null)
    
    if [[ -f "$archive_path" ]]; then
        local size=$(du -h "$archive_path" | cut -f1)
        log_success "Archive created: $archive_path ($size)"
        return 0
    else
        log_error "Failed to create archive"
        return 1
    fi
}

# Validate packaged plugins
validate_package() {
    local package_dir="$1"
    local plugin_name="$2"
    
    log_info "Validating packaged plugins..."
    
    local validation_failed=false
    
    # Find VST3
    if [[ -d "$package_dir/$plugin_name.vst3" ]]; then
        if command -v pluginval &> /dev/null; then
            if ! pluginval --strictness-level 5 --validate "$package_dir/$plugin_name.vst3" --timeout-ms 30000 > /dev/null 2>&1; then
                log_warning "VST3 validation failed (non-critical)"
            else
                log_success "VST3 validation passed"
            fi
        else
            log_info "Skipping VST3 validation (pluginval not available)"
        fi
    fi
    
    # Basic file checks
    local found_plugins=0
    for file in "$package_dir"/*; do
        if [[ -d "$file" ]] && [[ "$(basename "$file")" == *.vst3 ]] || [[ "$(basename "$file")" == *.component ]] || [[ "$(basename "$file")" == *.app ]]; then
            ((found_plugins++))
        elif [[ -f "$file" ]] && [[ "$(basename "$file")" == "$plugin_name" ]] || [[ "$(basename "$file")" == "$plugin_name.exe" ]]; then
            ((found_plugins++))
        fi
    done
    
    if [[ $found_plugins -eq 0 ]]; then
        log_error "No plugin files found in package"
        return 1
    else
        log_success "Found $found_plugins plugin file(s)"
    fi
    
    return 0
}

# Parse command line arguments
parse_arguments() {
    while [[ $# -gt 0 ]]; do
        case $1 in
            --version)
                VERSION="$2"
                shift 2
                ;;
            --formats)
                FORMATS="$2"
                shift 2
                ;;
            --config)
                BUILD_CONFIG="$2"
                shift 2
                ;;
            --help)
                usage
                ;;
            -*)
                log_error "Unknown option: $1"
                usage
                ;;
            *)
                PLUGIN_NAME="$1"
                shift
                ;;
        esac
    done
    
    if [[ -z "${PLUGIN_NAME:-}" ]]; then
        log_error "Plugin name is required"
        usage
    fi
    
    # Validate version format (basic check)
    if [[ ! "$VERSION" =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
        log_error "Invalid version format: $VERSION. Use x.y.z format"
        exit 1
    fi
}

# Main execution
main() {
    parse_arguments "$@"
    
    log_info "Packaging plugin: $PLUGIN_NAME v$VERSION"
    log_info "Formats: $FORMATS"
    log_info "Build config: $BUILD_CONFIG"
    
    # Create output directories
    local releases_dir="$PROJECT_ROOT/releases"
    local temp_dir=$(mktemp -d)
    local package_dir="$temp_dir/${PLUGIN_NAME}_v${VERSION}"
    
    mkdir -p "$releases_dir"
    mkdir -p "$package_dir"
    
    # Build the plugin
    if ! build_plugin "$PLUGIN_NAME" "$BUILD_CONFIG"; then
        log_error "Build failed, cannot package"
        rm -rf "$temp_dir"
        exit 1
    fi
    
    # Package artifacts
    package_artifacts "$PLUGIN_NAME" "$FORMATS" "$VERSION" "$package_dir"
    
    # Create package info
    create_package_info "$PLUGIN_NAME" "$VERSION" "$FORMATS" "$package_dir"
    
    # Validate package
    if ! validate_package "$package_dir" "$PLUGIN_NAME"; then
        log_error "Package validation failed"
        rm -rf "$temp_dir"
        exit 1
    fi
    
    # Create final archive
    if create_archive "$PLUGIN_NAME" "$VERSION" "$package_dir" "$releases_dir"; then
        echo ""
        log_success "Plugin packaged successfully! 🎉"
        echo ""
        echo -e "${GREEN}Package location:${NC} $releases_dir/"
        echo -e "${GREEN}Ready for distribution!${NC}"
        echo ""
        echo "Installation instructions included in PACKAGE_INFO.txt"
    else
        log_error "Failed to create final package"
        rm -rf "$temp_dir"
        exit 1
    fi
    
    # Clean up
    rm -rf "$temp_dir"
}

main "$@"