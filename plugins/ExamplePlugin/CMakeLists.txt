set(PLUGIN_NAME ExamplePlugin)

juce_add_plugin(${PLUGIN_NAME}
    VERSION 1.0.0
    COMPANY_NAME "Tyler Audio"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE Tylr
    PLUGIN_CODE Expl
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "${PLUGIN_NAME}"
)

juce_generate_juce_header(${PLUGIN_NAME})

target_sources(${PLUGIN_NAME}
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
)

target_include_directories(${PLUGIN_NAME}
    PRIVATE
        ../../shared
)

target_compile_definitions(${PLUGIN_NAME}
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(${PLUGIN_NAME}
    PRIVATE
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# Add pluginval test if available
if(BUILD_TESTS AND TARGET pluginval_tests)
    add_pluginval_test(${PLUGIN_NAME})
endif()