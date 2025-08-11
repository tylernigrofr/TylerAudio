cmake_minimum_required(VERSION 3.25)

project(TylerAudioPlugins VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Ensure Objective-C++ files (.mm) also use the same C++ standard
set(CMAKE_OBJCXX_STANDARD 23)
set(CMAKE_OBJCXX_STANDARD_REQUIRED ON)
set(CMAKE_OBJCXX_EXTENSIONS OFF)

# Xcode-specific: ensure modern C++ and suppress undefined macro warnings in JUCE headers
if(APPLE)
    # Match Xcode's Build Settings: C++ Language Dialect / Standard Library
    set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++20")
    set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
    # Disable "Warn About Undefined Macros" which causes noisy JUCE warnings
    set(CMAKE_XCODE_ATTRIBUTE_GCC_WARN_UNDEFINED_MACRO "NO")
endif()

# Build configuration
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose build type" FORCE)
endif()

# Compiler-specific settings
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    add_compile_options(
        -Wall -Wextra -Wpedantic
        -Wno-unused-parameter
        -Wconversion -Wsign-conversion
        -Wdouble-promotion
        -Wformat=2
        -Werror=return-type
        -fno-common
    )
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(-g3 -O0 -DDEBUG=1)
    elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(-O3 -DNDEBUG=1 -flto)
    endif()
elseif(MSVC)
    add_compile_options(
        /W4
        /permissive-
        /volatile:iso
        /Zc:__cplusplus
        $<$<CONFIG:Debug>:/Od /Z7 /DDEBUG=1>
        $<$<CONFIG:Release>:/O2 /DNDEBUG=1 /GL>
    )
    add_link_options($<$<CONFIG:Release>:/LTCG>)
endif()

# Sanitizers for Debug builds
option(ENABLE_SANITIZERS "Enable sanitizers in Debug builds" ON)
if(ENABLE_SANITIZERS AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        add_compile_options(-fsanitize=address,undefined -fno-sanitize-recover=all)
        add_link_options(-fsanitize=address,undefined)
    endif()
endif()

include(FetchContent)

FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.3
    GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(JUCE)

# Testing framework
option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.7.1
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(Catch2)
    
    enable_testing()
    include(CTest)
    
    # Add custom test target that runs all tests
    add_custom_target(run_tests
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --parallel 4
        DEPENDS tests
        COMMENT "Running all tests"
    )
endif()

# JUCE modules are provided via FetchContent; link juce::* targets directly in plugin targets

add_subdirectory(plugins)

# Add tests directory
if(BUILD_TESTS)
    add_subdirectory(tests)
endif()