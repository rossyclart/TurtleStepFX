#pragma once
#include <JuceHeader.h>
#include <array>

struct GateFX
{
    void process(juce::AudioBuffer<float>& buffer, int start, int num) noexcept
    {
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            buffer.clear(ch, start, num);
    }
};

struct StutterFX
{
    // retrigger the first half of the slice over the slice
    void process(juce::AudioBuffer<float>& buffer, int start, int num) noexcept
    {
        int half = juce::jmax(1, num / 2);
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            auto* d = buffer.getWritePointer(ch);
            for (int i = 0; i < num; ++i)
            {
                int src = start + (i % half);
                d[start + i] = d[src];
            }
        }
    }
};

struct BitcrushFX
{
    // naive downsample + quantize to 8-bit
    void process(juce::AudioBuffer<float>& buffer, int start, int num) noexcept
    {
        const int down = 2; // simple decimation factor
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            auto* d = buffer.getWritePointer(ch);
            float hold = 0.0f;
            for (int i = 0; i < num; ++i)
            {
                if ((i % down) == 0) hold = d[start + i];
                // 8-bit quantize
                float q = std::round(juce::jlimit(-1.0f, 1.0f, hold) * 127.0f) / 127.0f;
                d[start + i] = q;
            }
        }
    }
};

struct ReverseFX
{
    void process(juce::AudioBuffer<float>& buffer, int start, int num) noexcept
    {
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            auto* d = buffer.getWritePointer(ch);
            for (int i = 0; i < num / 2; ++i)
                std::swap(d[start + i], d[start + num - 1 - i]);
        }
    }
};

struct DelayFX
{
    void prepare(double sr)
    {
        sampleRate = sr;
        const int maxDelay = (int) (2.0 * sampleRate); // 2 seconds
        for (auto& cb : circ)
        {
            cb.setSize(1, maxDelay);
            cb.clear();
        }
    }

    void setBPM(double bpm)
    {
        double secPerBeat = 60.0 / bpm;
        delaySamples = (int) (0.5 * secPerBeat * sampleRate); // 1/8 note
        if (delaySamples < 1) delaySamples = 1;
        if (delaySamples > circ[0].getNumSamples()) delaySamples = circ[0].getNumSamples();
    }

    void process(juce::AudioBuffer<float>& io, int start, int num)
    {
        const float feedback = 0.35f;
        const float mix = 0.35f;

        for (int ch = 0; ch < io.getNumChannels(); ++ch)
        {
            auto* d = io.getWritePointer(ch);
            auto& cb = circ[ch % 2];
            for (int i = 0; i < num; ++i)
            {
                int readIdx = (wrIdx - delaySamples);
                while (readIdx < 0) readIdx += cb.getNumSamples();
                float delayed = cb.getSample(0, readIdx);
                float in = d[start + i];
                float out = in * (1.0f - mix) + delayed * mix;
                d[start + i] = out;
                cb.setSample(0, wrIdx, in + delayed * feedback);
                ++wrIdx;
                if (wrIdx >= cb.getNumSamples()) wrIdx = 0;
            }
        }
    }

private:
    double sampleRate { 44100.0 };
    int delaySamples { 22050 };
    int wrIdx { 0 };
    std::array<juce::AudioBuffer<float>, 2> circ;
};

struct ReverbFX
{
    void prepare(double sr)
    {
        juce::dsp::ProcessSpec spec { sr, 512, 2 };
        reverb.reset();
        reverb.prepare(spec);
        juce::dsp::Reverb::Parameters p;
        p.roomSize = 0.35f; p.wetLevel = 0.25f; p.dryLevel = 0.75f; p.width = 0.9f; p.damping = 0.3f;
        reverb.setParameters(p);
    }

    void process(juce::AudioBuffer<float>& buffer, int start, int num)
    {
        juce::AudioBlock<float> block (buffer);
        auto slice = block.getSubBlock((size_t) start, (size_t) num);
        juce::dsp::ProcessContextReplacing<float> ctx (slice);
        reverb.process(ctx);
    }

private:
    juce::dsp::Reverb reverb;
};

struct FilterFX
{
    void prepare(double sr)
    {
        juce::dsp::ProcessSpec spec { sr, 512, 2 };
        ladder.reset();
        ladder.prepare(spec);
        ladder.setMode(juce::dsp::LadderFilter<float>::Mode::LPF24);
        ladder.setCutoffFrequencyHz(1200.0f);
        ladder.setResonance(0.5f);
        ladder.setDrive(1.0f);
    }

    void process(juce::AudioBuffer<float>& buffer, int start, int num)
    {
        juce::AudioBlock<float> block (buffer);
        auto slice = block.getSubBlock((size_t) start, (size_t) num);
        juce::dsp::ProcessContextReplacing<float> ctx (slice);
        ladder.process(ctx);
    }

private:
    juce::dsp::LadderFilter<float> ladder;
};
