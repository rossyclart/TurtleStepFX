#include "PluginProcessor.h"
#include "PluginEditor.h"

TurtleStepFXAudioProcessor::TurtleStepFXAudioProcessor()
: juce::AudioProcessor (BusesProperties()
#if ! JucePlugin_IsMidiEffect
 #if ! JucePlugin_IsSynth
    .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
 #endif
    .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
#endif
), apvts(*this, nullptr, "PARAMS", createLayout())
{
    mix = apvts.getRawParameterValue("mix");
    outGain = apvts.getRawParameterValue("outGain");
}

juce::AudioProcessorValueTreeState::ParameterLayout TurtleStepFXAudioProcessor::createLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    params.push_back(std::make_unique<juce::AudioParameterFloat>("mix", "Mix", 0.0f, 1.0f, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("outGain", "Output Gain (dB)", -24.0f, 24.0f, 0.0f));
    return { params.begin(), params.end() };
}

void TurtleStepFXAudioProcessor::prepareToPlay (double sampleRate, int /*samplesPerBlock*/)
{
    seq.prepare(sampleRate, 120.0);
    delay.prepare(sampleRate);
    reverb.prepare(sampleRate);
    filter.prepare(sampleRate);
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool TurtleStepFXAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void TurtleStepFXAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    auto playHead = getPlayHead();
    juce::AudioPlayHead::CurrentPositionInfo pos;
    if (playHead != nullptr && playHead->getCurrentPosition(pos))
        seq.setBPM(pos.bpm > 0 ? pos.bpm : 120.0);

    const auto slice = seq.computeSlice(pos.timeInSamples);

    delay.setBPM(seq.getBPM());

    // Work buffer for slice processing
    work.makeCopyOf(buffer, true);

    // Compute step slice boundaries for this block
    const int numSamples = buffer.getNumSamples();
    int processed = 0;
    while (processed < numSamples)
    {
        int stepRemaining = slice.stepLengthSamples - ((slice.offsetInStep + processed) % slice.stepLengthSamples);
        int toProcess = juce::jmin(stepRemaining, numSamples - processed);

        int stepIndex = (slice.currentStep + ((slice.offsetInStep + processed) / slice.stepLengthSamples)) % StepGrid::steps;

        // Start/len for this sub-slice
        int start = processed;
        int len = toProcess;

        // Apply per-lane FX if step is on
        if (grid.on[0][stepIndex].load()) gate.process(work, start, len);
        if (grid.on[1][stepIndex].load()) stutter.process(work, start, len);
        if (grid.on[2][stepIndex].load()) filter.process(work, start, len);
        if (grid.on[3][stepIndex].load()) crush.process(work, start, len);
        if (grid.on[4][stepIndex].load()) delay.process(work, start, len);
        if (grid.on[5][stepIndex].load()) reverb.process(work, start, len);

        processed += toProcess;
    }

    // Mix & output gain
    float wet = *mix;
    float dry = 1.0f - wet;
    float linGain = juce::Decibels::decibelsToGain(*outGain);

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* d = buffer.getWritePointer(ch);
        auto* w = work.getReadPointer(ch);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
            d[i] = (d[i] * dry + w[i] * wet) * linGain;
    }
}

void TurtleStepFXAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream mos(destData, false);
    apvts.state.writeToStream(mos);

    // serialize grid
    for (int r = 0; r < StepGrid::lanes; ++r)
        for (int c = 0; c < StepGrid::steps; ++c)
            mos.writeBool(grid.on[r][c].load());
}

void TurtleStepFXAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    juce::MemoryInputStream mis(data, (size_t) sizeInBytes, false);
    apvts.state = juce::ValueTree::readFromStream(mis);

    for (int r = 0; r < StepGrid::lanes; ++r)
        for (int c = 0; c < StepGrid::steps; ++c)
            if (mis.getNumBytesRemaining() > 0)
                grid.on[r][c].store(mis.readBool());
}
