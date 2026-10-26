// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/ProbabilityDistribution.h"

#include <algorithm>
#include <cmath>

namespace aquorbium
{
void ProbabilityDistribution::setSeed(std::uint32_t value) noexcept
{
    seed = value == 0u ? 1u : value;
    reset();
}

void ProbabilityDistribution::reset() noexcept
{
    randomState = seed;
}

float ProbabilityDistribution::sample(float gaussianAmount) noexcept
{
    const auto amount = std::clamp(
        std::isfinite(gaussianAmount) ? gaussianAmount : 0.0f, 0.0f, 1.0f);
    const auto uniform = nextSigned();
    auto sum = 0.0f;
    for (int draw = 0; draw < 6; ++draw)
        sum += nextSigned();
    const auto gaussian = std::clamp(sum / 3.0f, -1.0f, 1.0f);
    return std::clamp(uniform + (gaussian - uniform) * amount, -1.0f, 1.0f);
}

float ProbabilityDistribution::nextSigned() noexcept
{
    randomState = randomState * 1664525u + 1013904223u;
    return static_cast<float>((randomState >> 8u) & 0x00ffffffu)
        / 8388607.5f - 1.0f;
}
}
