// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/PatternRotationMutator.h"

#include <algorithm>

namespace aquorbium
{
PatternRotationMutator::PatternRotationMutator() noexcept
{
    pattern[0] = 1;
    pattern[3] = 1;
    pattern[5] = 1;
}

bool PatternRotationMutator::setPattern(
    const std::array<std::uint8_t, maximumSteps>& values,
    std::size_t count) noexcept
{
    if (count == 0 || count > maximumSteps)
        return false;
    pattern.fill(0);
    activeLength = count;
    for (std::size_t index = 0; index < count; ++index)
        pattern[index] = values[index] == 0 ? 0 : 1;
    resetSchedule();
    return true;
}

void PatternRotationMutator::setSchedule(std::size_t barsPerMutation,
                                         int rotationSteps) noexcept
{
    intervalBars = std::clamp<std::size_t>(barsPerMutation, 1, 1024);
    rotation = std::clamp(rotationSteps, -31, 31);
    barsElapsed %= intervalBars;
}

void PatternRotationMutator::resetSchedule() noexcept
{
    barsElapsed = 0;
    barCounter = 0;
    mutations = 0;
}

PatternRotationMutator::Output PatternRotationMutator::advanceBar() noexcept
{
    ++barCounter;
    ++barsElapsed;
    auto mutated = false;
    if (barsElapsed >= intervalBars)
    {
        barsElapsed = 0;
        auto shift = rotation % static_cast<int>(activeLength);
        if (shift < 0)
            shift += static_cast<int>(activeLength);
        if (shift != 0)
        {
            std::rotate(pattern.begin(), pattern.begin() + shift,
                        pattern.begin() + static_cast<std::ptrdiff_t>(activeLength));
            mutated = true;
            ++mutations;
        }
    }
    return { mutated, intervalBars - barsElapsed, barCounter, mutations };
}

bool PatternRotationMutator::at(std::size_t index) const noexcept
{
    return pattern[index % activeLength] != 0;
}

std::size_t PatternRotationMutator::pulses() const noexcept
{
    std::size_t count = 0;
    for (std::size_t index = 0; index < activeLength; ++index)
        count += pattern[index] == 0 ? 0u : 1u;
    return count;
}
}
