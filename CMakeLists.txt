cmake_minimum_required(VERSION 3.22)

# Static MSVC runtime: the plugin runs on any Windows PC without installing the Visual C++ redistributable.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE STRING "" FORCE)

project(AnNiyamahVoiceEnhance VERSION 1.0.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        7.0.12
    GIT_SHALLOW    ON)
FetchContent_MakeAvailable(JUCE)

juce_add_binary_data(VoiceEnhanceAssets SOURCES Assets/logo.png)

juce_add_plugin(VoiceEnhance
    COMPANY_NAME                "An-Niyamah Production"
    PLUGIN_MANUFACTURER_CODE    Anny
    PLUGIN_CODE                 Vnhc
    FORMATS                     VST3
    PRODUCT_NAME                "An-Niyamah Voice Enhance"
    VST3_CATEGORIES             Fx Restoration
    IS_SYNTH                    FALSE
    NEEDS_MIDI_INPUT            FALSE
    NEEDS_MIDI_OUTPUT           FALSE
    IS_MIDI_EFFECT              FALSE
    COPY_PLUGIN_AFTER_BUILD     FALSE
    VERSION                     ${PROJECT_VERSION})

juce_generate_juce_header(VoiceEnhance)

target_sources(VoiceEnhance PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp)

target_compile_definitions(VoiceEnhance PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_REPORT_APP_USAGE=0)

target_link_libraries(VoiceEnhance
    PRIVATE
        VoiceEnhanceAssets
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags)
