// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/AddressableStepMemory.h"

#include <algorithm>
#include <cmath>

namespace aquorbium
{
namespace
{
float finiteOr(float value, float fallback) noexcept
{
    return std::isfinite(value) ? value : fallback;
}
}

AddressableStepMemory::AddressableStepMemory() noexcept
{
    for (std::size_t index = 0; index < maximumSteps; ++index)
    {
        steps[index].pitchSemitones = static_cast<float>(index % 8) * 2.0f;
        steps[index].gateDuration = 0.5f;
        steps[index].strategyBias = 0.0f;
        steps[index].desireAmount = 0.5f;
    }
}

void AddressableStepMemory::setLength(std::size_t value) noexcept
{
    activeLength = std::clamp<std::size_t>(value, 1, maximumSteps);
    sequentialCursor %= activeLength;
}

bool AddressableStepMemory::setStep(std::size_t index, const Step& value) noexcept
{
    if (index >= maximumSteps)
        return false;
    steps[index] = sanitized(value);
    return true;
}

const AddressableStepMemory::Step& AddressableStepMemory::step(
    std::size_t index) const noexcept
{
    return steps[index % activeLength];
}

void AddressableStepMemory::reset(std::size_t startIndex) noexcept
{
    sequentialCursor = startIndex % activeLength;
    eventCounter = 0;
}

AddressableStepMemory::Output AddressableStepMemory::advance() noexcept
{
    const auto index = sequentialCursor;
    sequentialCursor = (sequentialCursor + 1) % activeLength;
    return outputFor(index, sequentialCursor == 0);
}

AddressableStepMemory::Output AddressableStepMemory::address(
    float normalizedAddress) noexcept
{
    const auto addressValue = std::clamp(
        finiteOr(normalizedAddress, 0.0f), 0.0f, 1.0f);
    const auto scaled = addressValue * static_cast<float>(activeLength);
    const auto index = std::min(
        static_cast<std::size_t>(scaled), activeLength - 1);
    return outputFor(index, false);
}

AddressableStepMemory::Step AddressableStepMemory::sanitized(
    const Step& value) noexcept
{
    Step result;
    result.pitchSemitones = std::clamp(
        finiteOr(value.pitchSemitones, 0.0f), -48.0f, 48.0f);
    result.gateDuration = std::clamp(
        finiteOr(value.gateDuration, 0.5f), 0.0f, 1.0f);
    result.strategyBias = std::clamp(
        finiteOr(value.strategyBias, 0.0f), -1.0f, 1.0f);
    result.desireAmount = std::clamp(
        finiteOr(value.desireAmount, 0.5f), 0.0f, 1.0f);
    return result;
}

AddressableStepMemory::Output AddressableStepMemory::outputFor(
    std::size_t index, bool wrapped) noexcept
{
    Output result;
    result.step = steps[index];
    result.index = index;
    result.wrapped = wrapped;
    result.event = eventCounter++;
    return result;
}
}
