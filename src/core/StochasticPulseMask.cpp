// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/StochasticPulseMask.h"

#include <algorithm>
#include <cmath>

namespace aquorbium
{
void StochasticPulseMask::setSeed(std::uint32_t value) noexcept
{
    seed = value == 0 ? 1u : value;
    reset();
}

void StochasticPulseMask::reset() noexcept
{
    randomState = seed;
    previousPulse = false;
    passCurrentPulse = false;
    acceptedCount = 0;
    rejectedCount = 0;
}

StochasticPulseMask::Output StochasticPulseMask::process(
    bool pulse, float passProbability) noexcept
{
    auto trigger = false;
    auto masked = false;
    if (pulse && !previousPulse)
    {
        const auto probability = std::clamp(
            std::isfinite(passProbability) ? passProbability : 1.0f,
            0.0f, 1.0f);
        passCurrentPulse = probability >= 1.0f
            || (probability > 0.0f && nextUnit() < probability);
        if (passCurrentPulse)
        {
            ++acceptedCount;
            trigger = true;
        }
        else
        {
            ++rejectedCount;
            masked = true;
        }
    }
    if (!pulse)
        passCurrentPulse = false;
    previousPulse = pulse;
    return { pulse && passCurrentPulse, trigger,
             pulse && !passCurrentPulse ? true : masked,
             acceptedCount, rejectedCount };
}

float StochasticPulseMask::nextUnit() noexcept
{
    randomState = randomState * 1664525u + 1013904223u;
    return static_cast<float>((randomState >> 8u) & 0x00ffffffu) / 16777215.0f;
}
}
