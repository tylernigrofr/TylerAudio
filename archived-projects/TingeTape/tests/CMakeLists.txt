# TingeTape Plugin Tests CMakeLists.txt
cmake_minimum_required(VERSION 3.25)

# TingeTape test executable
add_executable(TingeTape_tests
    test_tingetape_smoke.cpp
    test_tingetape_unit.cpp
    test_tingetape_performance.cpp
    test_tingetape_wow_detailed.cpp
    test_tingetape_saturation_detailed.cpp
    test_tingetape_tone_detailed.cpp
    test_tingetape_integration.cpp
    test_tingetape_quality.cpp
    test_tingetape_validation.cpp
)

# Link against required libraries
target_link_libraries(TingeTape_tests 
    PRIVATE 
        Catch2::Catch2WithMain
        juce::juce_audio_basics
        juce::juce_audio_processors
        juce::juce_dsp
        TingeTape  # Link against the plugin itself
)

# Include directories
target_include_directories(TingeTape_tests PRIVATE
    ../../../shared          # Tyler Audio shared utilities
    ../../../tests          # Access to audio_test_utils.h
    ../Source               # TingeTape plugin source
)

# Add test discovery for CTest
include(Catch)
catch_discover_tests(TingeTape_tests)

# Create test data directory for TingeTape-specific test files
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/test_data)

# Copy any TingeTape-specific test audio files if they exist
file(GLOB TINGETAPE_AUDIO_TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/audio_data/*")
if(TINGETAPE_AUDIO_TEST_FILES)
    file(COPY ${TINGETAPE_AUDIO_TEST_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/test_data/)
endif()

# Add pluginval test specifically for TingeTape
if(PLUGINVAL_EXECUTABLE)
    add_custom_target(TingeTape_pluginval
        COMMAND ${PLUGINVAL_EXECUTABLE} --strictness-level 10 --validate $<TARGET_FILE:TingeTape>
        COMMENT "Running pluginval on TingeTape"
        DEPENDS TingeTape
        VERBATIM
    )
    
    # Add to the global pluginval_tests target if it exists
    if(TARGET pluginval_tests)
        add_dependencies(pluginval_tests TingeTape_pluginval)
    endif()
else()
    add_custom_target(TingeTape_pluginval
        COMMAND echo "pluginval not available for TingeTape - install from https://github.com/Tracktion/pluginval"
    )
endif()

# Performance benchmarking target
add_custom_target(TingeTape_benchmark
    COMMAND TingeTape_tests [TingeTape][performance]
    DEPENDS TingeTape_tests
    COMMENT "Running TingeTape performance benchmarks"
)

# Smoke test target (quick validation)
add_custom_target(TingeTape_smoke
    COMMAND TingeTape_tests [TingeTape][smoke]
    DEPENDS TingeTape_tests
    COMMENT "Running TingeTape smoke tests"
)

# Unit test target (comprehensive component testing)
add_custom_target(TingeTape_unit
    COMMAND TingeTape_tests [TingeTape][unit]
    DEPENDS TingeTape_tests
    COMMENT "Running TingeTape unit tests"
)

# All TingeTape tests target
add_custom_target(TingeTape_test_all
    COMMAND TingeTape_tests
    DEPENDS TingeTape_tests
    COMMENT "Running all TingeTape tests"
)