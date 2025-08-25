set(PLUGIN_NAME ExamplePlugin)

juce_add_plugin(${PLUGIN_NAME}
    VERSION 1.0.0
    COMPANY_NAME "TylerAudio"
    BUNDLE_ID "com.tyleraudio.exampleplugin"
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
        ../../framework
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
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# Workaround for AppleClang builds where <type_traits> may not be pulled in
# by Objective-C++ module translation units, leading to std::alignment_of_v
# not being found in JUCE's AudioSampleBuffer. Forcibly include <type_traits>.
if(APPLE)
    target_compile_options(${PLUGIN_NAME}
        PRIVATE
            $<$<COMPILE_LANGUAGE:CXX>:-include type_traits>
            $<$<COMPILE_LANGUAGE:OBJCXX>:-include type_traits>
            $<$<COMPILE_LANGUAGE:CXX>:-std=gnu++20>
            $<$<COMPILE_LANGUAGE:OBJCXX>:-std=gnu++20>
    )
    # Apply the same fixes to all wrapper targets
    foreach(target_suffix "_AU" "_VST3" "_Standalone")
        if(TARGET ${PLUGIN_NAME}${target_suffix})
            target_compile_options(${PLUGIN_NAME}${target_suffix}
                PRIVATE
                    $<$<COMPILE_LANGUAGE:CXX>:-include type_traits>
                    $<$<COMPILE_LANGUAGE:OBJCXX>:-include type_traits>
                    $<$<COMPILE_LANGUAGE:CXX>:-std=gnu++20>
                    $<$<COMPILE_LANGUAGE:OBJCXX>:-std=gnu++20>
            )
        endif()
    endforeach()
endif()

# Add pluginval test if available
if(BUILD_TESTS AND TARGET pluginval_tests)
    add_pluginval_test(${PLUGIN_NAME})
endif()