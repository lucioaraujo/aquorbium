// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/RandomToLoop.h"

#include <algorithm>
#include <cmath>

namespace aquorbium
{
void RandomToLoop::setLength(std::size_t value) noexcept
{
    activeLength = std::clamp<std::size_t>(value, 1, maximumLength);
    reset();
}

void RandomToLoop::reset() noexcept
{
    memory.fill(0.0f);
    cursor = 0;
    capturedCount = 0;
}

RandomToLoop::Output RandomToLoop::process(float liveValue,
                                           float loopAmount) noexcept
{
    const auto live = std::clamp(
        std::isfinite(liveValue) ? liveValue : 0.0f, -1.0f, 1.0f);
    const auto amount = std::clamp(
        std::isfinite(loopAmount) ? loopAmount : 0.0f, 0.0f, 1.0f);
    const auto index = cursor;

    float output = live;
    if (capturedCount < activeLength)
    {
        memory[index] = live;
        ++capturedCount;
    }
    else
    {
        const auto previous = memory[index];
        if (amount <= 0.0f)
        {
            output = live;
            memory[index] = live;
        }
        else if (amount >= 1.0f)
        {
            output = previous;
        }
        else
        {
            output = live + (previous - live) * amount;
            memory[index] = output;
        }
    }

    cursor = (cursor + 1) % activeLength;
    return { output, index, capturedCount == activeLength, cursor == 0 };
}
}
