#include "PluginEditor.h"
#include "PluginProcessor.h"

namespace {
constexpr int lanes = StepGrid::lanes;
constexpr int steps = StepGrid::steps;
const char* laneNames[lanes] = { "Gate", "Stutter", "Filter", "Bitcrush", "Delay", "Reverb" };
}

TurtleStepFXAudioProcessorEditor::TurtleStepFXAudioProcessorEditor (TurtleStepFXAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setResizable(true, true);
    setSize (720, 360);

    addAndMakeVisible(mixSlider);
    mixSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    mixSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 18);
    mixSlider.setRange(0.0, 1.0, 0.001);
    mixSlider.setTooltip("Wet/Dry Mix");
    mixAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        p.getParameters(), "mix", mixSlider);

    addAndMakeVisible(gainSlider);
    gainSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    gainSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 18);
    gainSlider.setRange(-24.0, 24.0, 0.01);
    gainSlider.setTooltip("Output Gain (dB)");
    gainAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        p.getParameters(), "outGain", gainSlider);
}

void TurtleStepFXAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
    auto area = getLocalBounds().reduced(10);

    // Header
    auto header = area.removeFromTop(40);
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(20.0f, juce::Font::bold));
    g.drawText("TurtleStep FX — step sequenced multi‑FX", header.removeFromLeft(360), juce::Justification::centredLeft);
    g.setFont(juce::Font(12.0f));
    g.drawText("Wet/Dry", header.removeFromLeft(160).reduced(6), juce::Justification::centredLeft);
    g.drawText("Output", header.reduced(6), juce::Justification::centredLeft);

    // Sliders (drawn by JUCE)
    // Grid
    auto gridArea = area.reduced(4);
    int cellW = gridArea.getWidth() / steps;
    int cellH = gridArea.getHeight() / lanes;

    for (int r = 0; r < lanes; ++r)
    {
        for (int c = 0; c < steps; ++c)
        {
            auto cell = juce::Rectangle<int>(gridArea.getX() + c * cellW, gridArea.getY() + r * cellH, cellW - 2, cellH - 2);
            bool on = audioProcessor.getGrid().on[r][c].load();
            g.setColour(on ? juce::Colours::chartreuse : juce::Colours::darkgrey);
            g.fillRoundedRectangle(cell.toFloat(), 4.0f);
            g.setColour(juce::Colours::black);
            g.drawRoundedRectangle(cell.toFloat(), 4.0f, 1.0f);
        }
        // row labels
        g.setColour(juce::Colours::white);
        g.drawText(laneNames[r], gridArea.removeFromLeft(0), juce::Justification::topLeft);
    }
    // lane names on the left
    auto left = getLocalBounds().withWidth(90).translated(10, 40);
    for (int r = 0; r < lanes; ++r)
    {
        auto rowRect = juce::Rectangle<int>(10, 50 + r * (gridArea.getHeight()/lanes), 80, 20);
        g.setColour(juce::Colours::lightgrey);
        g.drawText(laneNames[r], rowRect, juce::Justification::centredLeft);
    }
}

void TurtleStepFXAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(10);
    auto header = area.removeFromTop(40);
    auto mixArea = header.removeFromLeft(160).reduced(6);
    mixSlider.setBounds(mixArea.removeFromRight(mixArea.getWidth() - 54));
    auto gainArea = header.reduced(6);
    gainSlider.setBounds(gainArea.removeFromRight(gainArea.getWidth() - 54));
}

void TurtleStepFXAudioProcessorEditor::mouseDown (const juce::MouseEvent& e)
{
    auto area = getLocalBounds().reduced(10);
    area.removeFromTop(40);
    auto gridArea = area.reduced(4);
    int cellW = gridArea.getWidth() / steps;
    int cellH = gridArea.getHeight() / lanes;

    if (! gridArea.contains(e.getPosition()))
        return;

    int c = juce::jlimit(0, steps-1, (e.x - gridArea.getX()) / cellW);
    int r = juce::jlimit(0, lanes-1, (e.y - gridArea.getY()) / cellH);

    auto& cell = audioProcessor.getGrid().on[r][c];
    cell.store(! cell.load());
    repaint();
}
