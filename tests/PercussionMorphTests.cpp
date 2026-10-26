// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/HarmonicMorphOscillator.h"
#include "core/SamplePercussionVoice.h"
#include "core/StochasticPulseMask.h"

#include <array>
#include <bit>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <utility>

namespace
{
using Morph = aquorbium::HarmonicMorphOscillator;
using PulseMask = aquorbium::StochasticPulseMask;
using SampleVoice = aquorbium::SamplePercussionVoice;

bool sampleSelectionPitchAndDecayWork()
{
    static_assert(noexcept(std::declval<SampleVoice&>().tick()));
    SampleVoice voice;
    voice.prepare(8000.0);
    std::array<float, 512> positive {};
    std::array<float, 512> negative {};
    positive.fill(1.0f);
    negative.fill(-1.0f);
    if (!voice.loadSample(0, positive.data(), positive.size())
        || !voice.loadSample(1, negative.data(), negative.size()))
        return false;

    if (!voice.trigger(0, 0.0f, 1.0f, 0.8f) || voice.tick() <= 0.0f)
        return false;
    if (!voice.trigger(1, 0.0f, 1.0f, 0.8f) || voice.tick() >= 0.0f)
        return false;

    auto countFrames = [&](float pitch, float decay) {
        (void)voice.trigger(0, pitch, decay, 1.0f);
        int frames = 0;
        double energy = 0.0;
        while (voice.active() && frames < 10000)
        {
            const auto sample = voice.tick();
            energy += static_cast<double>(sample) * sample;
            ++frames;
        }
        return std::pair { frames, energy };
    };
    const auto normal = countFrames(0.0f, 1.0f);
    const auto octave = countFrames(12.0f, 1.0f);
    const auto shortDecay = countFrames(0.0f, 0.01f);
    return normal.first > octave.first * 1.9
        && shortDecay.second < normal.second * 0.2;
}

bool sampleVoiceRejectsInvalidData()
{
    SampleVoice voice;
    voice.prepare(std::numeric_limits<double>::quiet_NaN());
    std::array<float, 4> sample {
        std::numeric_limits<float>::quiet_NaN(), 2.0f, -2.0f, 0.0f
    };
    if (voice.loadSample(SampleVoice::maximumSamples, sample.data(), sample.size())
        || voice.loadSample(0, nullptr, sample.size())
        || !voice.loadSample(0, sample.data(), sample.size())
        || voice.trigger(1, 0.0f, 1.0f, 1.0f))
        return false;
    if (!voice.trigger(0, std::numeric_limits<float>::infinity(),
                       std::numeric_limits<float>::quiet_NaN(), 2.0f))
        return false;
    for (int frame = 0; frame < 32; ++frame)
    {
        const auto output = voice.tick();
        if (!std::isfinite(output) || std::abs(output) > 1.0f)
            return false;
    }
    voice.clearSample(0);
    return !voice.active();
}

bool harmonicCoefficientsMorphBeforeSynthesis()
{
    static_assert(noexcept(std::declval<Morph&>().tick()));
    Morph oscillator;
    oscillator.prepare(48000.0);
    std::array<float, Morph::maximumHarmonics> first {};
    std::array<float, Morph::maximumHarmonics> second {};
    first[0] = 1.0f;
    first[2] = 0.25f;
    second[0] = 0.5f;
    second[1] = 1.0f;
    if (!oscillator.setFamilies(first, second, 4))
        return false;
    oscillator.setMorph(0.0f);
    if (oscillator.coefficient(0) != 1.0f
        || oscillator.coefficient(1) != 0.0f
        || oscillator.coefficient(2) != 0.25f)
        return false;
    oscillator.setMorph(0.5f);
    if (oscillator.coefficient(0) != 0.75f
        || oscillator.coefficient(1) != 0.5f
        || oscillator.coefficient(2) != 0.125f)
        return false;
    oscillator.setMorph(1.0f);
    return oscillator.coefficient(0) == 0.5f
        && oscillator.coefficient(1) == 1.0f
        && oscillator.coefficient(2) == 0.0f;
}

bool harmonicOutputIsFiniteAndDeterministic()
{
    Morph first, repeated;
    for (auto* oscillator : { &first, &repeated })
    {
        oscillator->prepare(48000.0);
        oscillator->setFrequency(9000.0f);
        oscillator->setMorph(0.72f);
        oscillator->setLevel(1.0f);
    }
    for (int sample = 0; sample < 10000; ++sample)
    {
        const auto left = first.tick();
        const auto right = repeated.tick();
        if (std::bit_cast<std::uint32_t>(left)
                != std::bit_cast<std::uint32_t>(right)
            || !std::isfinite(left) || std::abs(left) > 1.0f)
            return false;
    }
    std::array<float, Morph::maximumHarmonics> empty {};
    auto invalid = empty;
    invalid[0] = std::numeric_limits<float>::quiet_NaN();
    return !first.setFamilies(empty, empty, 4)
        && !first.setFamilies(invalid, empty, 4);
}

bool pulseMaskDecidesOnlyAtOnsets()
{
    static_assert(noexcept(std::declval<PulseMask&>().process(false, 0.0f)));
    PulseMask mask;
    mask.setSeed(42u);
    const auto accepted = mask.process(true, 1.0f);
    const auto held = mask.process(true, 0.0f);
    const auto release = mask.process(false, 0.0f);
    const auto rejected = mask.process(true, 0.0f);
    return accepted.gate && accepted.trigger && accepted.accepted == 1
        && held.gate && !held.trigger && held.accepted == 1
        && !release.gate && !release.masked
        && !rejected.gate && rejected.masked && rejected.rejected == 1;
}

bool pulseMaskIsSeededAndBounded()
{
    PulseMask first, repeated;
    first.setSeed(1234u);
    repeated.setSeed(1234u);
    int accepted = 0;
    int rejected = 0;
    for (int pulse = 0; pulse < 1000; ++pulse)
    {
        const auto a = first.process(true, 0.37f);
        const auto b = repeated.process(true, 0.37f);
        if (a.gate != b.gate || a.trigger != b.trigger
            || a.accepted != b.accepted || a.rejected != b.rejected)
            return false;
        accepted += a.gate ? 1 : 0;
        rejected += a.masked ? 1 : 0;
        (void)first.process(false, 0.37f);
        (void)repeated.process(false, 0.37f);
    }
    const auto invalid = first.process(true,
        std::numeric_limits<float>::quiet_NaN());
    return accepted > 250 && accepted < 500 && accepted + rejected == 1000
        && invalid.gate;
}
}

int main()
{
    const std::pair<const char*, bool (*)()> cases[] {
        { "sample voice", sampleSelectionPitchAndDecayWork },
        { "sample limits", sampleVoiceRejectsInvalidData },
        { "harmonic morph", harmonicCoefficientsMorphBeforeSynthesis },
        { "harmonic output", harmonicOutputIsFiniteAndDeterministic },
        { "pulse onset", pulseMaskDecidesOnlyAtOnsets },
        { "pulse seed", pulseMaskIsSeededAndBounded },
    };
    for (const auto& [name, test] : cases)
    {
        if (!test())
        {
            std::cerr << name << " failed\n";
            return EXIT_FAILURE;
        }
    }
    std::cout << "Sample percussion, harmonic morph and pulse mask passed\n";
}
