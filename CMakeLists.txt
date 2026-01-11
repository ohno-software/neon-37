cmake_minimum_required(VERSION 3.15)

project(Neon37 VERSION 0.0.1)

find_package(Git REQUIRED)

# Standard C++20 is recommended for JUCE 8 and modern MacOS
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# On MacOS, it's good practice to set the deployment target. 
# JUCE 8 requires building with MacOS 11.1 SDK (Xcode 12.4+) but can target older OSs.
if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum macOS version to target")
endif()

# Fetch JUCE
include(FetchContent)
FetchContent_Declare(
    juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.4
    PATCH_COMMAND  ${CMAKE_COMMAND}
        -DSOURCE_DIR=<SOURCE_DIR>
        -DGIT_EXECUTABLE=${GIT_EXECUTABLE}
        -DPATCH_FILE=${CMAKE_CURRENT_LIST_DIR}/cmake/juce-ladderfilter-smoothing.patch
        -P ${CMAKE_CURRENT_LIST_DIR}/cmake/apply_patch_if_needed.cmake
)
FetchContent_MakeAvailable(juce)

# Check if we are running in a CI environment to disable automatic copying
if(DEFINED ENV{CI})
    set(DEFAULT_COPY_AFTER_BUILD FALSE)
else()
    set(DEFAULT_COPY_AFTER_BUILD TRUE)
endif()

juce_add_plugin(Neon37
    COMPANY_NAME "Neon Audio"
    BUNDLE_ID "com.NeonAudio.Neon37"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_AFTER_BUILD ${DEFAULT_COPY_AFTER_BUILD}
    PLUGIN_MANUFACTURER_CODE Neon
    PLUGIN_CODE N37p
    FORMATS AU VST3 Standalone
)

target_sources(Neon37
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
)

target_compile_definitions(Neon37
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_LADDERFILTER_SMOOTHER_RAMP_TIME_SEC=0.0
)

target_link_libraries(Neon37
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
