#!/bin/bash

# validate-plugin.sh - Validate JUCE plugin with comprehensive testing
# Usage: ./validate-plugin.sh <PluginName> [--format VST3|AU|Standalone] [--strictness 1-10]

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
FORMAT="VST3"
STRICTNESS="8"
TIMEOUT="60"

usage() {
    echo "Usage: $0 <PluginName> [options]"
    echo ""
    echo "Options:"
    echo "  --format FORMAT      Plugin format to validate (VST3, AU, Standalone)"
    echo "  --strictness LEVEL   Validation strictness 1-10 (default: 8)"
    echo "  --timeout SECONDS    Timeout for validation (default: 60)"
    echo "  --help              Show this help"
    echo ""
    echo "Examples:"
    echo "  $0 MyReverb"
    echo "  $0 MyReverb --format AU --strictness 10"
    echo "  $0 MyReverb --format Standalone --timeout 120"
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

# Check if pluginval is available
check_pluginval() {
    if ! command -v pluginval &> /dev/null; then
        log_error "pluginval not found. Please install pluginval:"
        echo ""
        echo "macOS: brew install pluginval"
        echo "Or download from: https://github.com/Tracktion/pluginval/releases"
        exit 1
    fi
    
    local version=$(pluginval --version 2>&1 || echo "unknown")
    log_info "Using pluginval version: $version"
}

# Find plugin binary
find_plugin_binary() {
    local plugin_name="$1"
    local format="$2"
    local build_dir="$PROJECT_ROOT/build-dev"
    
    # Try different possible locations
    local possible_paths=(
        "$build_dir/plugins/$plugin_name/${plugin_name}_artefacts/Debug/$format"
        "$build_dir/plugins/$plugin_name/${plugin_name}_artefacts/Release/$format"
        "$PROJECT_ROOT/build/plugins/$plugin_name/${plugin_name}_artefacts/Debug/$format"
        "$PROJECT_ROOT/build/plugins/$plugin_name/${plugin_name}_artefacts/Release/$format"
    )
    
    for path in "${possible_paths[@]}"; do
        case "$format" in
            "VST3")
                local plugin_file="$path/$plugin_name.vst3"
                ;;
            "AU")
                local plugin_file="$path/$plugin_name.component"
                ;;
            "Standalone")
                local plugin_file="$path/$plugin_name"
                if [[ "$OSTYPE" == "darwin"* ]]; then
                    plugin_file="$path/$plugin_name.app"
                elif [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "win32" ]]; then
                    plugin_file="$path/$plugin_name.exe"
                fi
                ;;
        esac
        
        if [[ -e "$plugin_file" ]]; then
            echo "$plugin_file"
            return 0
        fi
    done
    
    return 1
}

# Validate plugin with pluginval
validate_with_pluginval() {
    local plugin_path="$1"
    local strictness="$2"
    local timeout="$3"
    
    log_info "Validating plugin: $(basename "$plugin_path")"
    log_info "Strictness level: $strictness"
    log_info "Timeout: ${timeout}s"
    
    # Create temp file for output
    local output_file=$(mktemp)
    
    # Run pluginval with timeout
    local start_time=$(date +%s)
    
    if timeout "${timeout}s" pluginval \
        --strictness-level "$strictness" \
        --validate "$plugin_path" \
        --output-dir "$(dirname "$output_file")" \
        --verbose \
        > "$output_file" 2>&1; then
        
        local end_time=$(date +%s)
        local duration=$((end_time - start_time))
        
        log_success "Plugin validation PASSED in ${duration}s"
        
        # Show summary from output
        if grep -q "PASSED" "$output_file"; then
            local passed_tests=$(grep -c "PASSED" "$output_file" || echo "0")
            local failed_tests=$(grep -c "FAILED" "$output_file" || echo "0")
            log_info "Tests passed: $passed_tests, failed: $failed_tests"
        fi
        
        # Clean up
        rm -f "$output_file"
        return 0
    else
        local exit_code=$?
        local end_time=$(date +%s)
        local duration=$((end_time - start_time))
        
        log_error "Plugin validation FAILED after ${duration}s (exit code: $exit_code)"
        
        # Show errors from output
        echo ""
        echo -e "${RED}Validation Output:${NC}"
        cat "$output_file"
        
        # Clean up
        rm -f "$output_file"
        return 1
    fi
}

# Quick smoke test
smoke_test() {
    local plugin_path="$1"
    
    log_info "Running quick smoke test..."
    
    # Basic file existence and permissions
    if [[ ! -r "$plugin_path" ]]; then
        log_error "Plugin file is not readable: $plugin_path"
        return 1
    fi
    
    # Quick validation with minimal strictness
    local output_file=$(mktemp)
    
    if timeout 15s pluginval \
        --strictness-level 1 \
        --validate "$plugin_path" \
        --timeout-ms 10000 \
        > "$output_file" 2>&1; then
        log_success "Smoke test passed"
        rm -f "$output_file"
        return 0
    else
        log_error "Smoke test failed"
        echo ""
        echo -e "${RED}Smoke Test Output:${NC}"
        cat "$output_file" | head -20
        rm -f "$output_file"
        return 1
    fi
}

# Parse command line arguments
parse_arguments() {
    while [[ $# -gt 0 ]]; do
        case $1 in
            --format)
                FORMAT="$2"
                shift 2
                ;;
            --strictness)
                STRICTNESS="$2"
                shift 2
                ;;
            --timeout)
                TIMEOUT="$2"
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
    
    # Validate format
    case "$FORMAT" in
        "VST3"|"AU"|"Standalone") ;;
        *)
            log_error "Invalid format: $FORMAT. Must be VST3, AU, or Standalone"
            exit 1
            ;;
    esac
    
    # Validate strictness
    if [[ ! "$STRICTNESS" =~ ^[1-9]$|^10$ ]]; then
        log_error "Invalid strictness: $STRICTNESS. Must be 1-10"
        exit 1
    fi
}

# Main execution
main() {
    parse_arguments "$@"
    
    log_info "Validating plugin: $PLUGIN_NAME ($FORMAT format)"
    
    # Check prerequisites
    check_pluginval
    
    # Find plugin binary
    log_info "Searching for plugin binary..."
    if ! plugin_path=$(find_plugin_binary "$PLUGIN_NAME" "$FORMAT"); then
        log_error "Plugin binary not found for $PLUGIN_NAME ($FORMAT format)"
        echo ""
        echo "Make sure the plugin is built first:"
        echo "  ${BLUE}./tools/dev-build.sh $PLUGIN_NAME${NC}"
        exit 1
    fi
    
    log_success "Found plugin: $plugin_path"
    
    # Run smoke test first
    if ! smoke_test "$plugin_path"; then
        log_error "Smoke test failed. Plugin has serious issues."
        exit 1
    fi
    
    echo ""
    log_info "Starting full validation..."
    
    # Run full validation
    if validate_with_pluginval "$plugin_path" "$STRICTNESS" "$TIMEOUT"; then
        echo ""
        log_success "Plugin validation completed successfully! 🎉"
        echo ""
        echo -e "${GREEN}Your plugin is ready for testing in a DAW!${NC}"
    else
        echo ""
        log_error "Plugin validation failed. Please fix the issues above."
        exit 1
    fi
}

main "$@"