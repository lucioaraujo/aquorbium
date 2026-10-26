// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/ShiftRegisterMemory.h"

#include <algorithm>
#include <cmath>

namespace aquorbium
{
void ShiftRegisterMemory::setLength(std::size_t value) noexcept
{
    activeLength = std::clamp(value, std::size_t { 1 }, maximumLength);
    bits &= mask();
}

void ShiftRegisterMemory::setSeed(std::uint32_t value) noexcept
{
    seed = value == 0u ? 1u : value;
    randomState = seed;
}

void ShiftRegisterMemory::reset(std::uint32_t patternValue) noexcept
{
    bits = patternValue & mask();
    randomState = seed;
}

ShiftRegisterMemory::Output ShiftRegisterMemory::advance(
    bool sourceBit, float mutationProbability) noexcept
{
    const auto probability = std::clamp(
        std::isfinite(mutationProbability) ? mutationProbability : 0.0f,
        0.0f, 1.0f);
    const auto mutated = probability >= 1.0f
        || (probability > 0.0f && nextUnit() < probability);
    const auto inserted = sourceBit != mutated;
    const auto outgoing = ((bits >> (activeLength - 1u)) & 1u) != 0u;
    bits = ((bits << 1u) | (inserted ? 1u : 0u)) & mask();
    return { outgoing, inserted, mutated, bits };
}

std::uint32_t ShiftRegisterMemory::mask() const noexcept
{
    return activeLength == maximumLength
        ? 0xffffffffu
        : (1u << activeLength) - 1u;
}

float ShiftRegisterMemory::nextUnit() noexcept
{
    randomState = randomState * 1664525u + 1013904223u;
    return static_cast<float>((randomState >> 8u) & 0x00ffffffu) / 16777215.0f;
}
}
