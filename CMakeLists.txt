cmake_minimum_required(VERSION 3.15)

project(Cossin
    VERSION      0.1.0
    DESCRIPTION  "A multi-purpose effector plugin."
    HOMEPAGE_URL "https://github.com/ElandaSunshine/Cossin"
    LANGUAGES    CXX)

set(CMAKE_CXX_STANDARD            17)
set(CMAKE_CXX_STANDARD_REQUIRED   ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(JUCE CONFIG REQUIRED)

juce_add_plugin(Cossin
    VERSION                  ${COSSIN_VERSION}
    COMPANY_NAME             "ElandaSunshine"
    PRODUCT_NAME             "ES Cossin"
    PLUGIN_MANUFACTURER_CODE ELSA
    PLUGIN_CODE              EsCs
    FORMATS                  VST3 Standalone
    VST3_CATEGORIES          "Fx")

add_subdirectory(externals)
add_subdirectory(src)

target_compile_definitions(Cossin PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_USE_CUSTOM_PLUGIN_STANDALONE_APP=1
    JUCE_OPENGL_ALLOW_NON_POWER_OF_TWO_TEXTURES=1
    FF_AUDIO_ALLOW_ALLOCATIONS_IN_MEASURE_BLOCK=0)

target_link_libraries(Cossin PRIVATE
    yaml-cpp
    foleys::ff_meters
    jaut::jaut_gui
    jaut::jaut_provider
    jaut::jaut_util
    jaut::jaut_audio
    juce::juce_recommended_warning_flags
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_audio_devices
    juce::juce_audio_plugin_client
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_opengl)
