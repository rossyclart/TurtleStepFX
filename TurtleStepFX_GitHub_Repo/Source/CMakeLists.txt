juce_add_plugin(TurtleStep_FX
    COMPANY_NAME "WiseTortoise Audio"
    BUNDLE_ID com.wisetortoise.turtlestepfx
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE WTfx
    PLUGIN_CODE Tsfx
    FORMATS VST3
    PRODUCT_NAME "TurtleStep FX"
)

target_sources(TurtleStep_FX PRIVATE
    PluginProcessor.h
    PluginProcessor.cpp
    PluginEditor.h
    PluginEditor.cpp
    Sequencer.h
    Sequencer.cpp
    Effects.h
    Effects.cpp
)

target_compile_definitions(TurtleStep_FX
    PRIVATE
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_REPORT_APP_USAGE=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(TurtleStep_FX PRIVATE
    juce::juce_dsp
    juce::juce_audio_utils
    juce::juce_audio_devices
    juce::juce_audio_basics
    juce::juce_audio_processors
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_graphics
    juce::juce_core
)
