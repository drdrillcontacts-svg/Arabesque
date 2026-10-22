cmake_minimum_required(VERSION 3.22)
project(ARABESQUE VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT DEFINED JUCE_DIR)
    set(JUCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/JUCE" CACHE PATH "Path to JUCE")
endif()
add_subdirectory("${JUCE_DIR}" JUCE)

if(APPLE)
    set(ARABESQUE_FORMATS VST3 AU)
else()
    set(ARABESQUE_FORMATS VST3)
endif()

juce_add_plugin(ARABESQUE
    COMPANY_NAME "DA Production Instruments"
    BUNDLE_ID "com.daproductioninstruments.arabesque"
    PLUGIN_MANUFACTURER_CODE DaPr
    PLUGIN_CODE Arb1
    FORMATS ${ARABESQUE_FORMATS}
    PRODUCT_NAME "ARABESQUE"
    DESCRIPTION "Turkish emotional synthesizer"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_CATEGORIES "Instrument" "Synth"
    AU_MAIN_TYPE kAudioUnitType_MusicDevice
)

juce_generate_juce_header(ARABESQUE)

target_sources(ARABESQUE PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/DSP/SynthVoice.cpp
    Source/DSP/EffectsChain.cpp
    Source/DSP/Arpeggiator.cpp
)

target_compile_definitions(ARABESQUE PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_STRICT_REFCOUNTEDPOINTER=1
    JUCE_MODAL_LOOPS_PERMITTED=1
)

target_link_libraries(ARABESQUE PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags
)
