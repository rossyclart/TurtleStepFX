#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class TurtleStepFXAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    explicit TurtleStepFXAudioProcessorEditor (TurtleStepFXAudioProcessor&);
    ~TurtleStepFXAudioProcessorEditor() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent& e) override;

private:
    TurtleStepFXAudioProcessor& audioProcessor;

    juce::Slider mixSlider, gainSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttach, gainAttach;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TurtleStepFXAudioProcessorEditor)
};
