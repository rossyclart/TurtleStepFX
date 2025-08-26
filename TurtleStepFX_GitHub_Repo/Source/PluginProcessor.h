#pragma once
#include <JuceHeader.h>
#include "Sequencer.h"
#include "Effects.h"

class TurtleStepFXAudioProcessor  : public juce::AudioProcessor
{
public:
    TurtleStepFXAudioProcessor();
    ~TurtleStepFXAudioProcessor() override {}

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    //==============================================================================
    const juce::String getName() const override { return JucePlugin_Name; }

    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect () const override { return false; }
    double getTailLengthSeconds() const override { return 2.0; }

    //==============================================================================
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // Accessors
    StepGrid& getGrid() noexcept { return grid; }
    float getMix() const noexcept { return *mix; }
    float getOutGain() const noexcept { return *outGain; }

private:
    // Parameters
    juce::AudioProcessorValueTreeState apvts;
    std::atomic<float>* mix { nullptr };
    std::atomic<float>* outGain { nullptr };

    // Sequencer & FX
    Sequencer seq;
    StepGrid grid;
    GateFX gate;
    StutterFX stutter;
    FilterFX filter;
    BitcrushFX crush;
    DelayFX delay;
    ReverbFX reverb;

    // helpers
    juce::AudioBuffer<float> work;

    juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TurtleStepFXAudioProcessor)
};
