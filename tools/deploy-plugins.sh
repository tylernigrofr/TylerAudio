#!/bin/bash

# Tyler Audio Plugin Deployment Script
# Automatically copies built plugins to system VST3 folders for testing

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_ROOT/build"

echo "🚀 Tyler Audio Plugin Deployment"
echo "================================="

# Detect operating system and set VST3 path
detect_vst3_path() {
    case "$(uname -s)" in
        Darwin*)
            VST3_PATH="$HOME/Library/Audio/Plug-Ins/VST3"
            OS_NAME="macOS"
            ;;
        Linux*)
            VST3_PATH="$HOME/.vst3"
            OS_NAME="Linux"
            ;;
        CYGWIN*|MINGW*|MSYS*)
            VST3_PATH="/c/Program Files/Common Files/VST3"
            OS_NAME="Windows"
            ;;
        *)
            echo "❌ Unsupported operating system: $(uname -s)"
            exit 1
            ;;
    esac
    
    echo "🖥️  Detected OS: $OS_NAME"
    echo "📂 VST3 Path: $VST3_PATH"
}

# Create VST3 directory if it doesn't exist
ensure_vst3_directory() {
    if [ ! -d "$VST3_PATH" ]; then
        echo "📁 Creating VST3 directory: $VST3_PATH"
        mkdir -p "$VST3_PATH"
    fi
    
    if [ ! -w "$VST3_PATH" ]; then
        echo "❌ No write permission to VST3 directory: $VST3_PATH"
        echo "💡 You may need to run with sudo or adjust permissions"
        exit 1
    fi
}

# Find and deploy VST3 plugins
deploy_plugins() {
    local plugins_found=false
    local plugins_deployed=0
    
    echo "🔍 Searching for VST3 plugins in build directory..."
    
    find "$BUILD_DIR" -name "*.vst3" -type d | while read -r plugin_path; do
        if [ -d "$plugin_path" ]; then
            plugins_found=true
            local plugin_name=$(basename "$plugin_path")
            local destination="$VST3_PATH/$plugin_name"
            
            echo "📦 Found plugin: $plugin_name"
            
            # Remove existing plugin if present
            if [ -d "$destination" ]; then
                echo "🗑️  Removing existing plugin: $destination"
                rm -rf "$destination"
            fi
            
            # Copy plugin to VST3 directory
            echo "📋 Copying $plugin_name to VST3 directory..."
            if cp -r "$plugin_path" "$destination"; then
                echo "✅ Successfully deployed: $plugin_name"
                plugins_deployed=$((plugins_deployed + 1))
                
                # Verify plugin structure
                if [ -d "$destination/Contents" ]; then
                    echo "  ✓ Plugin bundle structure verified"
                else
                    echo "  ⚠️  Plugin bundle structure may be incomplete"
                fi
                
                # Show plugin info if available
                if [ -f "$destination/Contents/Info.plist" ] && command -v plutil >/dev/null 2>&1; then
                    local bundle_name=$(plutil -extract CFBundleName raw "$destination/Contents/Info.plist" 2>/dev/null || echo "Unknown")
                    local bundle_version=$(plutil -extract CFBundleVersion raw "$destination/Contents/Info.plist" 2>/dev/null || echo "Unknown")
                    echo "  📋 Bundle Name: $bundle_name"
                    echo "  🔢 Version: $bundle_version"
                fi
            else
                echo "❌ Failed to deploy: $plugin_name"
                return 1
            fi
        fi
    done
    
    if [ "$plugins_found" = false ]; then
        echo "⚠️  No VST3 plugins found in build directory"
        echo "💡 Make sure to build your plugins first: cmake --build build"
        exit 1
    fi
    
    echo ""
    echo "🎉 Deployment Complete!"
    echo "======================"
    echo "📊 Plugins deployed: $plugins_deployed"
    echo "📂 Location: $VST3_PATH"
}

# Verify deployment
verify_deployment() {
    echo ""
    echo "🔍 Verifying Deployment..."
    echo "========================="
    
    local plugins_verified=0
    
    find "$VST3_PATH" -maxdepth 1 -name "*.vst3" -type d | while read -r plugin_path; do
        if [ -d "$plugin_path" ]; then
            local plugin_name=$(basename "$plugin_path")
            
            # Check if this is one of our plugins (Tyler Audio)
            if [[ "$plugin_name" == *"Tyler"* ]] || [[ "$plugin_name" == *"Tinge"* ]] || [[ "$plugin_name" == *"Example"* ]]; then
                echo "✅ Verified: $plugin_name"
                plugins_verified=$((plugins_verified + 1))
                
                # Basic structure check
                if [ -d "$plugin_path/Contents" ]; then
                    echo "  ✓ Bundle structure OK"
                else
                    echo "  ❌ Bundle structure invalid"
                fi
            fi
        fi
    done
    
    if [ $plugins_verified -eq 0 ]; then
        echo "⚠️  No Tyler Audio plugins found in VST3 directory"
    else
        echo "🎯 $plugins_verified Tyler Audio plugins verified"
    fi
}

# Show DAW instructions
show_daw_instructions() {
    echo ""
    echo "🎵 DAW Testing Instructions"
    echo "=========================="
    echo "Your plugins have been installed to: $VST3_PATH"
    echo ""
    echo "To test in your DAW:"
    echo "1. 🔄 Rescan plugins in your DAW"
    echo "2. 🔍 Look for 'Tyler Audio' plugins in the plugin list"
    echo "3. 🎛️  Load a plugin on an audio track"
    echo "4. 🎧 Test with audio to verify functionality"
    echo ""
    echo "Common DAW plugin rescan locations:"
    echo "• Reaper: Options → Preferences → Plug-ins → VST → Re-scan"
    echo "• Logic Pro: Logic Pro → Preferences → Plug-In Manager → Reset & Rescan Selection"
    echo "• Pro Tools: Setup → Plug-ins → Plug-in Settings → Scan for Plug-ins"
    echo "• Ableton Live: Preferences → Plug-Ins → Use VST3 Plug-in System Folders"
}

# Cleanup function
cleanup_old_plugins() {
    if [ "$1" = "--clean" ]; then
        echo "🧹 Cleaning up old Tyler Audio plugins..."
        
        find "$VST3_PATH" -maxdepth 1 -name "*Tyler*.vst3" -type d | while read -r plugin_path; do
            echo "🗑️  Removing: $(basename "$plugin_path")"
            rm -rf "$plugin_path"
        done
        
        find "$VST3_PATH" -maxdepth 1 -name "*Tinge*.vst3" -type d | while read -r plugin_path; do
            echo "🗑️  Removing: $(basename "$plugin_path")"
            rm -rf "$plugin_path"
        done
    fi
}

# Main execution
main() {
    echo "🎯 Starting plugin deployment..."
    
    # Handle cleanup flag
    cleanup_old_plugins "$1"
    
    # Check if build directory exists
    if [ ! -d "$BUILD_DIR" ]; then
        echo "❌ Build directory not found: $BUILD_DIR"
        echo "💡 Please run: cmake --build build"
        exit 1
    fi
    
    # Detect system and setup paths
    detect_vst3_path
    
    # Ensure VST3 directory exists
    ensure_vst3_directory
    
    # Deploy plugins
    deploy_plugins
    
    # Verify deployment
    verify_deployment
    
    # Show instructions
    show_daw_instructions
    
    echo ""
    echo "✨ Tyler Audio plugins are ready for testing!"
}

# Show help
if [ "$1" = "--help" ] || [ "$1" = "-h" ]; then
    echo "Tyler Audio Plugin Deployment Script"
    echo ""
    echo "Usage:"
    echo "  $0                Deploy all built plugins to system VST3 folder"
    echo "  $0 --clean       Remove old Tyler Audio plugins first, then deploy"
    echo "  $0 --help        Show this help message"
    echo ""
    exit 0
fi

# Run main function
main "$@"