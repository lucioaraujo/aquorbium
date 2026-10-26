// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/StepPatternBank.h"

#include <algorithm>

namespace aquorbium
{
bool StepPatternBank::save(std::size_t slot,
                           const AddressableStepMemory& source) noexcept
{
    if (slot >= maximumSlots)
        return false;
    auto& pattern = patterns[slot];
    pattern.length = source.length();
    for (std::size_t index = 0; index < pattern.length; ++index)
        pattern.steps[index] = source.step(index);
    pattern.valid = true;
    return true;
}

bool StepPatternBank::restore(std::size_t slot,
                              AddressableStepMemory& destination,
                              std::size_t startIndex) const noexcept
{
    if (slot >= maximumSlots || !patterns[slot].valid)
        return false;
    const auto& pattern = patterns[slot];
    destination.setLength(pattern.length);
    for (std::size_t index = 0; index < pattern.length; ++index)
        (void)destination.setStep(index, pattern.steps[index]);
    destination.reset(startIndex);
    return true;
}

bool StepPatternBank::setChain(
    const std::array<std::uint8_t, maximumChain>& slots,
    std::size_t count) noexcept
{
    if (count == 0 || count > maximumChain)
        return false;
    for (std::size_t index = 0; index < count; ++index)
        if (slots[index] >= maximumSlots || !patterns[slots[index]].valid)
            return false;
    chain = slots;
    chainSize = count;
    resetChain();
    return true;
}

void StepPatternBank::resetChain(std::size_t startIndex) noexcept
{
    chainCursor = chainSize == 0 ? 0 : startIndex % chainSize;
    eventCounter = 0;
}

StepPatternBank::ChainOutput StepPatternBank::advanceChain() noexcept
{
    ChainOutput result;
    if (chainSize == 0)
        return result;
    result.chainIndex = chainCursor;
    result.slot = chain[chainCursor];
    result.event = eventCounter++;
    chainCursor = (chainCursor + 1) % chainSize;
    result.wrapped = chainCursor == 0;
    return result;
}

bool StepPatternBank::hasPattern(std::size_t slot) const noexcept
{
    return slot < maximumSlots && patterns[slot].valid;
}
}
