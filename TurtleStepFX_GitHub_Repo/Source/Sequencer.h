#pragma once
#include <array>
#include <atomic>

struct StepGrid
{
    // rows = lanes (6 effects), cols = 16 steps
    static constexpr int lanes = 6;
    static constexpr int steps = 16;
    std::array<std::array<std::atomic<bool>, steps>, lanes> on;

    StepGrid()
    {
        for (auto& row : on)
            for (auto& cell : row)
                cell.store(false);
    }
};

struct TransportSlice
{
    double bpm = 120.0;
    double sampleRate = 44100.0;
    // Length (in samples) of one 1/16 note step
    int stepLengthSamples = 22050;
    // Which step index [0..15] the current block starts in
    int currentStep = 0;
    // Offset (in samples) into the current step at block start
    int offsetInStep = 0;
};

class Sequencer
{
public:
    Sequencer() = default;

    void prepare(double sr, double bpm_)
    {
        sampleRate = sr;
        setBPM(bpm_);
    }

    void setBPM(double bpm_) { bpm.store(bpm_); }
    double getBPM() const { return bpm.load(); }

    // Compute current slice state from host position (timeInSamples)
    TransportSlice computeSlice(long long timeInSamples) const
    {
        TransportSlice s{};
        s.sampleRate = sampleRate;
        s.bpm = bpm.load();

        double secPerBeat = 60.0 / s.bpm;
        double secPerStep = secPerBeat / 4.0; // 1/16
        s.stepLengthSamples = static_cast<int>(secPerStep * s.sampleRate + 0.5);

        if (s.stepLengthSamples <= 0) s.stepLengthSamples = 1;

        long long stepIndex = (timeInSamples / s.stepLengthSamples) % StepGrid::steps;
        s.currentStep = (int) (stepIndex < 0 ? (stepIndex + StepGrid::steps) : stepIndex);

        s.offsetInStep = (int) (timeInSamples % s.stepLengthSamples);
        if (s.offsetInStep < 0) s.offsetInStep += s.stepLengthSamples;
        return s;
    }

private:
    std::atomic<double> bpm { 120.0 };
    double sampleRate { 44100.0 };
};
