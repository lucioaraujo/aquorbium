// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/SlewRateRandomWalk.h"
#include "core/SynthFMVoice.h"

#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <type_traits>
#include <utility>

namespace
{
bool fmVoiceBoundaries()
{
    using Voice = aquorbium::SynthFMVoice;
    static_assert(noexcept(std::declval<Voice&>().tick()));
    Voice voice;
    voice.prepare(48000.0);
    voice.setParameters({ 80.0f, 173.0f, 350.0f, 1.0f, 0.7f });
    bool reversed = false;
    bool audible = false;
    for (int sample = 0; sample < 96000; ++sample)
    {
        const auto value = voice.tick();
        if (!std::isfinite(value) || std::abs(value) > 0.70001f) return false;
        reversed = reversed || voice.lastCarrierFrequencyHz() < -10.0f;
        audible = audible || std::abs(value) > 0.1f;
    }
    voice.setParameters({ 80.0f, 173.0f, 350.0f, 1.0f, 0.0f });
    for (int sample = 0; sample < 1000; ++sample)
        if (voice.tick() != 0.0f) return false;
    return reversed && audible;
}

bool fmVoiceShapeAndPartition()
{
    using Voice = aquorbium::SynthFMVoice;
    Voice plain, folded, partitioned;
    for (auto* voice : { &plain, &folded, &partitioned })
        voice->prepare(48000.0);
    plain.setParameters({ 220.0f, 330.0f, 0.0f, 0.0f, 0.5f });
    folded.setParameters({ 220.0f, 330.0f, 0.0f, 1.0f, 0.5f });
    partitioned.setParameters(plain.parameters());
    bool shapeDiffers = false;
    std::array<float, 16384> reference {};
    for (auto& direct : reference)
    {
        direct = plain.tick();
        const auto shaped = folded.tick();
        if (std::abs(direct - shaped) > 0.08f) shapeDiffers = true;
    }
    for (std::size_t offset = 0; offset < reference.size();)
    {
        const auto end = std::min(reference.size(), offset + 97);
        for (; offset < end; ++offset)
            if (std::bit_cast<std::uint32_t>(reference[offset])
                != std::bit_cast<std::uint32_t>(partitioned.tick())) return false;
    }
    return shapeDiffers;
}

bool fmVoiceInvalidInputs()
{
    aquorbium::SynthFMVoice voice;
    voice.prepare(std::numeric_limits<double>::quiet_NaN());
    voice.setParameters({ std::numeric_limits<float>::infinity(),
        -9.0f, std::numeric_limits<float>::quiet_NaN(),
        2.0f, -1.0f });
    const auto parameters = voice.parameters();
    if (parameters.modulatorHz != 0.0f || parameters.fold != 1.0f
        || parameters.level != 0.0f || !std::isfinite(parameters.carrierHz)
        || !std::isfinite(parameters.depthHz)) return false;
    for (int sample = 0; sample < 10000; ++sample)
        if (!std::isfinite(voice.tick())) return false;
    return true;
}

bool randomWalkDeterminismAndBounds()
{
    using Walk = aquorbium::SlewRateRandomWalk;
    static_assert(noexcept(std::declval<Walk&>().tick()));
    Walk first, repeated, another;
    for (auto* walk : { &first, &repeated, &another })
    {
        walk->prepare(8000.0);
        walk->setParameters({ 5.0f, 0.8f, 1.9f });
    }
    first.setSeed(1234u);
    repeated.setSeed(1234u);
    another.setSeed(5678u);
    bool seedsDiffer = false;
    float previous = 0.0f;
    for (int sample = 0; sample < 80000; ++sample)
    {
        const auto value = first.tick();
        if (!std::isfinite(value) || std::abs(value) > 1.0f
            || std::abs(value - previous) > 0.002f) return false;
        previous = value;
        if (std::bit_cast<std::uint32_t>(value)
            != std::bit_cast<std::uint32_t>(repeated.tick())) return false;
        if (value != another.tick()) seedsDiffer = true;
    }
    return seedsDiffer;
}

bool randomWalkInvalidInputs()
{
    aquorbium::SlewRateRandomWalk walk;
    walk.prepare(std::numeric_limits<double>::infinity());
    walk.setParameters({ -1.0f, 2.0f,
        std::numeric_limits<float>::quiet_NaN() });
    const auto parameters = walk.parameters();
    if (parameters.stepsPerSecond != 0.0001f || parameters.stepSize != 1.0f
        || parameters.smoothingHz != 0.8f) return false;
    walk.setSeed(0u);
    for (int sample = 0; sample < 480000; ++sample)
        if (!std::isfinite(walk.tick()) || std::abs(walk.value()) > 1.0f)
            return false;
    return true;
}
}

int main()
{
    if (!fmVoiceBoundaries() || !fmVoiceShapeAndPartition()
        || !fmVoiceInvalidInputs() || !randomWalkDeterminismAndBounds()
        || !randomWalkInvalidInputs())
    {
        std::cerr << "Pending module prototype test failed\n";
        return 1;
    }
    std::cout << "FM voice and random-walk prototypes passed\n";
}
