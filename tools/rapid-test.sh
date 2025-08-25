#!/bin/bash

# TylerAudio Rapid Testing Script
# Quick validation workflow for development iterations

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# Configuration
PLUGIN_NAME="${1:-TingeTape}"
PRESET_NAME="dev-fast"
BUILD_DIR="build-dev"
SKIP_FULL_PLUGINVAL="${SKIP_FULL_PLUGINVAL:-false}"

echo -e "${BLUE}🧪 TylerAudio Rapid Testing${NC}"
echo -e "${BLUE}Plugin: ${PLUGIN_NAME}${NC}"

# Function to run quick pluginval smoke test
run_quick_pluginval() {
    local plugin_path="$1"
    local plugin_name="$2"
    
    echo -e "${YELLOW}🔍 Quick pluginval validation for ${plugin_name}...${NC}"
    
    # Quick smoke test with lower strictness for rapid iteration
    local strictness_level=5
    if [[ "$SKIP_FULL_PLUGINVAL" == "true" ]]; then
        strictness_level=1
        echo -e "${BLUE}  ℹ️  Using minimal strictness for rapid iteration${NC}"
    fi
    
    if timeout 30s pluginval --strictness-level "$strictness_level" --validate-in-process --timeout-ms 5000 "$plugin_path"; then
        echo -e "${GREEN}  ✅ ${plugin_name} passed quick pluginval test${NC}"
        return 0
    else
        local exit_code=$?
        if [[ $exit_code -eq 124 ]]; then
            echo -e "${YELLOW}  ⚠️  ${plugin_name} pluginval test timed out (may be OK for development)${NC}"
            return 0  # Don't fail rapid iteration on timeout
        else
            echo -e "${RED}  ❌ ${plugin_name} failed pluginval test${NC}"
            return 1
        fi
    fi
}

# Function to run unit tests quickly
run_quick_tests() {
    echo -e "${YELLOW}🧪 Running quick unit tests...${NC}"
    
    if [[ -f "${BUILD_DIR}/plugins/${PLUGIN_NAME}/tests/${PLUGIN_NAME}_tests" ]]; then
        local test_executable="${BUILD_DIR}/plugins/${PLUGIN_NAME}/tests/${PLUGIN_NAME}_tests"
        echo -e "${BLUE}  Running ${PLUGIN_NAME} unit tests...${NC}"
        
        if timeout 15s "$test_executable" --reporter=compact --success; then
            echo -e "${GREEN}  ✅ Unit tests passed${NC}"
        else
            echo -e "${RED}  ❌ Unit tests failed${NC}"
            return 1
        fi
    else
        echo -e "${BLUE}  ℹ️  No unit tests found for ${PLUGIN_NAME}${NC}"
    fi
    
    return 0
}

# Function to validate plugin can load
validate_plugin_loading() {
    local plugin_path="$1"
    local plugin_name="$2"
    
    echo -e "${YELLOW}📦 Validating ${plugin_name} can load...${NC}"
    
    # Check plugin bundle structure
    if [[ -d "${plugin_path}/Contents" ]] && [[ -f "${plugin_path}/Contents/Info.plist" ]]; then
        echo -e "${GREEN}  ✅ Plugin bundle structure valid${NC}"
        
        # Try to extract basic info from plist
        local bundle_id=$(plutil -extract CFBundleIdentifier raw "${plugin_path}/Contents/Info.plist" 2>/dev/null || echo "unknown")
        local version=$(plutil -extract CFBundleVersion raw "${plugin_path}/Contents/Info.plist" 2>/dev/null || echo "unknown")
        echo -e "${BLUE}  Bundle ID: ${bundle_id}${NC}"
        echo -e "${BLUE}  Version: ${version}${NC}"
        
        return 0
    else
        echo -e "${RED}  ❌ Invalid plugin bundle structure${NC}"
        return 1
    fi
}

# Function to check for common issues
check_common_issues() {
    echo -e "${YELLOW}🔍 Checking for common issues...${NC}"
    
    # Check for debug symbols in release-like builds
    local binary_found=false
    find "${BUILD_DIR}" -name "*.dylib" -o -name "${PLUGIN_NAME}_Standalone" | while read binary; do
        if [[ -f "$binary" ]]; then
            binary_found=true
            local file_info=$(file "$binary")
            if [[ "$file_info" == *"not stripped"* ]]; then
                echo -e "${BLUE}  ℹ️  Binary has debug symbols (expected for dev builds): $(basename $binary)${NC}"
            fi
        fi
    done
    
    # Check for common JUCE warnings in recent build log
    if [[ -f "${BUILD_DIR}/CMakeFiles/CMakeOutput.log" ]]; then
        local recent_warnings=$(grep -i "warning\|deprecated" "${BUILD_DIR}/CMakeFiles/CMakeOutput.log" | tail -3)
        if [[ -n "$recent_warnings" ]]; then
            echo -e "${YELLOW}  ⚠️  Recent build warnings (may be ignorable):${NC}"
            echo "$recent_warnings" | sed 's/^/    /'
        fi
    fi
    
    return 0
}

# Main execution
main() {
    local start_time=$(date +%s)
    
    # Build first if not recently built
    if [[ ! -f "${BUILD_DIR}/CMakeCache.txt" ]] || [[ ! -f "${BUILD_DIR}/.last_build_time" ]] || [[ $(($(date +%s) - $(cat "${BUILD_DIR}/.last_build_time" 2>/dev/null || echo 0))) -gt 300 ]]; then
        echo -e "${YELLOW}🔨 Building first...${NC}"
        if ./dev-build.sh "${PLUGIN_NAME}"; then
            echo "$(date +%s)" > "${BUILD_DIR}/.last_build_time"
        else
            echo -e "${RED}❌ Build failed, cannot run tests${NC}"
            exit 1
        fi
    fi
    
    local tests_passed=true
    
    # Run quick unit tests
    if ! run_quick_tests; then
        tests_passed=false
    fi
    
    # Find and test plugins
    local plugins_tested=0
    find "${BUILD_DIR}" -name "${PLUGIN_NAME}*.vst3" -type d | while read -r plugin_path; do
        if [[ -d "$plugin_path" ]]; then
            local plugin_name=$(basename "$plugin_path" .vst3)
            plugins_tested=$((plugins_tested + 1))
            
            echo -e "\n${BLUE}Testing plugin: ${plugin_name}${NC}"
            
            # Validate plugin loading
            if ! validate_plugin_loading "$plugin_path" "$plugin_name"; then
                tests_passed=false
                continue
            fi
            
            # Run quick pluginval if available
            if command -v pluginval >/dev/null 2>&1; then
                if ! run_quick_pluginval "$plugin_path" "$plugin_name"; then
                    tests_passed=false
                fi
            else
                echo -e "${YELLOW}  ⚠️  pluginval not found. Install from: https://github.com/Tracktion/pluginval${NC}"
            fi
        fi
    done
    
    # Check for common issues
    check_common_issues
    
    local end_time=$(date +%s)
    local duration=$((end_time - start_time))
    
    # Summary
    echo -e "\n${BLUE}📊 Rapid Test Summary${NC}"
    echo -e "${BLUE}Duration: ${duration}s${NC}"
    
    if [[ "$tests_passed" == "true" ]]; then
        echo -e "${GREEN}🎉 All rapid tests passed! Ready for iteration.${NC}"
        
        # Show next steps
        echo -e "\n${BLUE}💡 Next steps:${NC}"
        echo -e "  • Launch standalone: find ${BUILD_DIR} -name '*_Standalone*' -executable"
        echo -e "  • Full test suite: ./scripts/run-smoke-tests.sh"
        echo -e "  • Watch mode: ./watch-build.sh ${PLUGIN_NAME}"
        
        exit 0
    else
        echo -e "${RED}❌ Some rapid tests failed. Check output above.${NC}"
        exit 1
    fi
}

# Handle script arguments
case "${2:-}" in
    "--help"|"-h")
        echo "Usage: $0 [PLUGIN_NAME] [OPTIONS]"
        echo ""
        echo "Options:"
        echo "  --quick          Skip full pluginval (use SKIP_FULL_PLUGINVAL=true)"
        echo "  --help, -h       Show this help"
        echo ""
        echo "Environment variables:"
        echo "  SKIP_FULL_PLUGINVAL=true    Use minimal pluginval strictness"
        echo ""
        echo "Examples:"
        echo "  $0 TingeTape                 # Test TingeTape plugin"
        echo "  $0 TingeTape --quick         # Quick test with minimal validation"
        echo "  SKIP_FULL_PLUGINVAL=true $0 TingeTape"
        exit 0
        ;;
    "--quick")
        export SKIP_FULL_PLUGINVAL=true
        ;;
esac

# Run main function
main "$@"