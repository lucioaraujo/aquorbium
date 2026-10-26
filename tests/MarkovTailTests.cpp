// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/AquariumTail.h"
#include "core/BiomaMarkov.h"

#include <bit>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <memory>
#include <utility>

namespace
{
using Markov = aquorbium::BiomaMarkov;
using Tail = aquorbium::AquariumTail;

bool markovIsDeterministicAndContextual()
{
    static_assert(noexcept(std::declval<Markov&>().advance(Markov::Context {})));
    Markov first, repeated, other;
    first.setSeed(0x12345u);
    repeated.setSeed(0x12345u);
    other.setSeed(0x54321u);
    bool seedDiffers = false;
    for (int event = 0; event < 10000; ++event)
    {
        const Markov::Context context {
            static_cast<float>(event % 101) / 100.0f, 0.72f
        };
        const auto value = first.advance(context);
        if (value != repeated.advance(context)) return false;
        seedDiffers = seedDiffers || value != other.advance(context);
    }

    Markov low, high;
    low.setSeed(0x91acu);
    high.setSeed(0x91acu);
    low.setParameters({ 1.0f, 0.0f, 0.0f });
    high.setParameters({ 1.0f, 0.0f, 0.0f });
    double lowMean = 0.0;
    double highMean = 0.0;
    for (int event = 0; event < 20000; ++event)
    {
        lowMean += low.advance({ 0.0f, 1.0f });
        highMean += high.advance({ 1.0f, 1.0f });
    }
    return seedDiffers && highMean / 20000.0 > lowMean / 20000.0 + 1.5;
}

bool markovRowsAndInvalidValuesAreSafe()
{
    Markov chain;
    chain.setParameters({ -1.0f, 2.0f,
        std::numeric_limits<float>::quiet_NaN() });
    const auto parameters = chain.parameters();
    if (parameters.contextInfluence != 0.0f || parameters.inertia != 1.0f
        || parameters.mutation != 0.05f) return false;

    Markov::TransitionRow forced {};
    forced[5] = 9.0f;
    chain.setTransitionRow(0, forced);
    chain.setParameters({ 0.0f, 0.0f, 0.0f });
    chain.reset(0);
    if (chain.advance({ 0.5f, 0.0f }) != 5) return false;

    Markov::TransitionRow invalid {};
    invalid.fill(std::numeric_limits<float>::infinity());
    chain.setTransitionRow(5, invalid);
    chain.reset(5);
    if (chain.advance({ 0.5f, 0.0f }) != 5) return false;
    return chain.transitionCount() == 1;
}

bool tailSilenceImpulseAndDecay()
{
    static_assert(noexcept(std::declval<Tail&>().tick(0.0f, 0.0f)));
    auto tail = std::make_unique<Tail>();
    tail->prepare(48000.0);
    tail->setParameters({ 0.3f, 0.35f, 0.65f, 0.8f, 0.0f, 0.7f });
    for (int sample = 0; sample < 4096; ++sample)
    {
        const auto value = tail->tick(0.0f, 0.0f);
        if (value.left != 0.0f || value.right != 0.0f) return false;
    }

    bool audible = false;
    double earlyEnergy = 0.0;
    double lateEnergy = 0.0;
    for (int sample = 0; sample < 48000 * 3; ++sample)
    {
        const auto input = sample == 0 ? 1.0f : 0.0f;
        const auto value = tail->tick(input, input);
        if (!std::isfinite(value.left) || !std::isfinite(value.right)
            || std::abs(value.left) > 1.0f || std::abs(value.right) > 1.0f)
            return false;
        const auto energy = static_cast<double>(value.left) * value.left
            + static_cast<double>(value.right) * value.right;
        if (sample > 500 && sample < 48000) earlyEnergy += energy;
        if (sample >= 48000 * 2) lateEnergy += energy;
        audible = audible || std::abs(value.left) > 1.0e-5f
            || std::abs(value.right) > 1.0e-5f;
    }
    const auto telemetry = tail->telemetry();
    return audible && earlyEnergy > 1.0e-6 && lateEnergy < earlyEnergy * 0.001
        && telemetry.delaySamples[0] >= 2u && telemetry.feedback[0] < 0.9851f;
}

bool tailParametersChangeTextureAndRemainFinite()
{
    auto clear = std::make_unique<Tail>();
    auto nightmare = std::make_unique<Tail>();
    clear->prepare(96000.0);
    nightmare->prepare(96000.0);
    clear->setParameters({ 1.0f, 8.0f, 0.1f, 1.0f, 0.0f, 1.0f });
    nightmare->setParameters({ 1.0f, 8.0f, 0.1f, 1.0f, 1.0f, 1.0f });
    bool differs = false;
    for (int sample = 0; sample < 96000 * 2; ++sample)
    {
        const auto input = sample == 0
            ? std::numeric_limits<float>::infinity() : (sample == 1 ? 1.0f : 0.0f);
        const auto normal = clear->tick(input, -input);
        const auto twisted = nightmare->tick(input, -input);
        if (!std::isfinite(normal.left) || !std::isfinite(normal.right)
            || !std::isfinite(twisted.left) || !std::isfinite(twisted.right)
            || std::abs(normal.left) > 1.0f || std::abs(normal.right) > 1.0f
            || std::abs(twisted.left) > 1.0f || std::abs(twisted.right) > 1.0f)
            return false;
        differs = differs
            || std::bit_cast<std::uint32_t>(normal.left)
                != std::bit_cast<std::uint32_t>(twisted.left);
    }
    return differs;
}
}

int main()
{
    const std::pair<const char*, bool (*)()> cases[] {
        { "markov deterministic/context", markovIsDeterministicAndContextual },
        { "markov rows/invalid", markovRowsAndInvalidValuesAreSafe },
        { "tail impulse/decay", tailSilenceImpulseAndDecay },
        { "tail texture/finite", tailParametersChangeTextureAndRemainFinite },
    };
    for (const auto& [name, test] : cases)
    {
        if (!test())
        {
            std::cerr << name << " failed\n";
            return 1;
        }
    }
    std::cout << "Markov and synthetic-tail prototypes passed\n";
}
