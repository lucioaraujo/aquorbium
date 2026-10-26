// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/ThresholdStepSelector.h"

#include <algorithm>
#include <cmath>

namespace aquorbium
{
ThresholdStepSelector::ThresholdStepSelector() noexcept
{
    makeEvenThresholds();
}

void ThresholdStepSelector::setLength(std::size_t value) noexcept
{
    activeLength = std::clamp<std::size_t>(value, 1, maximumSteps);
    makeEvenThresholds();
    reset();
}

bool ThresholdStepSelector::setThresholds(
    const std::array<float, maximumSteps>& values, std::size_t count) noexcept
{
    if (count == 0 || count > maximumSteps)
        return false;

    auto previous = 0.0f;
    for (std::size_t index = 0; index < count; ++index)
    {
        const auto value = values[index];
        if (!std::isfinite(value) || value <= previous || value > 1.0f)
            return false;
        previous = value;
    }
    if (values[count - 1] != 1.0f)
        return false;

    thresholds = values;
    activeLength = count;
    reset();
    return true;
}

void ThresholdStepSelector::reset() noexcept
{
    previousIndex = maximumSteps;
    transitions = 0;
}

ThresholdStepSelector::Output ThresholdStepSelector::select(
    float normalizedSignal) noexcept
{
    const auto signal = std::clamp(
        std::isfinite(normalizedSignal) ? normalizedSignal : 0.0f,
        0.0f, 1.0f);
    std::size_t index = 0;
    while (index + 1 < activeLength && signal >= thresholds[index])
        ++index;

    Output result;
    result.index = index;
    result.changed = index != previousIndex;
    if (result.changed)
    {
        if (previousIndex < activeLength)
            result.direction = index > previousIndex ? 1 : -1;
        previousIndex = index;
        ++transitions;
    }
    result.transitionCount = transitions;
    return result;
}

float ThresholdStepSelector::threshold(std::size_t index) const noexcept
{
    return thresholds[index % activeLength];
}

void ThresholdStepSelector::makeEvenThresholds() noexcept
{
    for (std::size_t index = 0; index < activeLength; ++index)
        thresholds[index] = static_cast<float>(index + 1)
            / static_cast<float>(activeLength);
}
}
