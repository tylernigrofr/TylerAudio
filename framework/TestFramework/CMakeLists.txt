# Tests CMakeLists.txt
cmake_minimum_required(VERSION 3.25)

# Test executable
add_executable(tests
    test_main.cpp
    test_shared_utilities.cpp
    test_example_plugin.cpp
    audio_test_utils.cpp
)

target_link_libraries(tests 
    PRIVATE 
        Catch2::Catch2WithMain
        juce::juce_audio_basics
        juce::juce_audio_processors
        juce::juce_dsp
)

target_include_directories(tests PRIVATE
    ../shared
    ../plugins/ExamplePlugin/Source
)

# Add test discovery
include(Catch)
catch_discover_tests(tests)

# Performance benchmark executable
add_executable(benchmarks
    benchmark_main.cpp
    benchmark_dsp.cpp
)

target_link_libraries(benchmarks 
    PRIVATE 
        Catch2::Catch2WithMain
        juce::juce_audio_basics
        juce::juce_dsp
)

target_include_directories(benchmarks PRIVATE
    ../shared
)

# Audio test data directory
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/test_data)

# Copy test audio files if they exist
file(GLOB AUDIO_TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/audio_data/*")
if(AUDIO_TEST_FILES)
    file(COPY ${AUDIO_TEST_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/test_data/)
endif()

# pluginval integration
find_program(PLUGINVAL_EXECUTABLE pluginval)
if(PLUGINVAL_EXECUTABLE)
    add_custom_target(pluginval_tests
        COMMAND echo "Running pluginval tests..."
        COMMENT "Run pluginval on built plugins"
    )
    
    # This will be populated by individual plugin tests
    set_property(GLOBAL PROPERTY PLUGINVAL_TARGETS "")
    
    function(add_pluginval_test PLUGIN_TARGET)
        get_property(CURRENT_TARGETS GLOBAL PROPERTY PLUGINVAL_TARGETS)
        set_property(GLOBAL PROPERTY PLUGINVAL_TARGETS "${CURRENT_TARGETS};${PLUGIN_TARGET}")
        
        add_custom_command(TARGET pluginval_tests POST_BUILD
            COMMAND ${PLUGINVAL_EXECUTABLE} --strictness-level 10 --validate $<TARGET_FILE:${PLUGIN_TARGET}>
            COMMENT "Running pluginval on ${PLUGIN_TARGET}"
            VERBATIM
        )
    endfunction()
else()
    message(STATUS "pluginval not found - skipping plugin validation tests")
    add_custom_target(pluginval_tests
        COMMAND echo "pluginval not available - install from https://github.com/Tracktion/pluginval"
    )
endif()