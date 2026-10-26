// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/TripleSlowDrift.h"

#include "core/Numeric.h"

#include <algorithm>
#include <cmath>

namespace aquorbium
{
void TripleSlowDrift::prepare(double newSampleRate) noexcept
{
    sampleRate = std::isfinite(newSampleRate)
        ? std::clamp(newSampleRate, 1.0, numeric::maximumSampleRate)
        : 200.0;
    for (auto& walk : walks)
        walk.prepare(sampleRate);
    configureWalks();
    setSeed(seed);
}

void TripleSlowDrift::setSeed(std::uint32_t value) noexcept
{
    seed = value == 0u ? 1u : value;
    for (std::size_t index = 0; index < walks.size(); ++index)
        walks[index].setSeed(seed + static_cast<std::uint32_t>(index) * 0x9e3779b9u);
}

void TripleSlowDrift::setPeriodsSeconds(std::array<float, 3> value) noexcept
{
    for (std::size_t index = 0; index < periods.size(); ++index)
        periods[index] = std::clamp(
            std::isfinite(value[index]) ? value[index] : periods[index],
            0.1f, 3600.0f);
    configureWalks();
}

void TripleSlowDrift::setWeights(std::array<float, 3> value) noexcept
{
    auto sum = 0.0f;
    for (std::size_t index = 0; index < mixWeights.size(); ++index)
    {
        mixWeights[index] = std::clamp(
            std::isfinite(value[index]) ? value[index] : mixWeights[index],
            0.0f, 1.0f);
        sum += mixWeights[index];
    }
    if (sum <= 0.0f)
        mixWeights = { 0.5f, 0.3f, 0.2f };
}

void TripleSlowDrift::reset() noexcept
{
    for (auto& walk : walks)
        walk.reset();
}

TripleSlowDrift::Output TripleSlowDrift::tick() noexcept
{
    const std::array<float, 3> values {
        walks[0].tick(), walks[1].tick(), walks[2].tick()
    };
    const auto weightSum = mixWeights[0] + mixWeights[1] + mixWeights[2];
    const auto combined = (values[0] * mixWeights[0]
        + values[1] * mixWeights[1] + values[2] * mixWeights[2]) / weightSum;
    return { values[0], values[1], values[2],
             std::clamp(combined, -1.0f, 1.0f) };
}

void TripleSlowDrift::configureWalks() noexcept
{
    constexpr std::array<float, 3> stepSizes { 0.65f, 0.45f, 0.3f };
    for (std::size_t index = 0; index < walks.size(); ++index)
    {
        const auto eventRate = 1.0f / periods[index];
        walks[index].setParameters({
            eventRate, stepSizes[index], std::max(0.0001f, eventRate * 2.0f)
        });
    }
}
}
