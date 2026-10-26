// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/EvolutionLoopMachine.h"

#include <algorithm>
#include <cmath>

namespace aquorbium
{
void EvolutionLoopMachine::setSeed(std::uint32_t value) noexcept
{
    seed = value == 0u ? 1u : value;
    reset();
}

void EvolutionLoopMachine::setLength(std::size_t value) noexcept
{
    activeLength = std::clamp(value, std::size_t { 1 }, maximumLength);
    reset();
}

void EvolutionLoopMachine::reset() noexcept
{
    randomState = seed;
    for (auto& value : memory)
        value = nextSigned();
    cursor = 0;
}

EvolutionLoopMachine::Output EvolutionLoopMachine::advance(
    float evolutionProbability) noexcept
{
    const auto index = cursor;
    auto value = memory[index];
    auto mutated = false;
    if (currentMode == Mode::random)
    {
        value = nextSigned();
        memory[index] = value;
        mutated = true;
    }
    else if (currentMode == Mode::evolving)
    {
        const auto probability = std::clamp(
            std::isfinite(evolutionProbability) ? evolutionProbability : 0.0f,
            0.0f, 1.0f);
        mutated = probability >= 1.0f
            || (probability > 0.0f && nextUnit() < probability);
        if (mutated)
        {
            value = (value + nextSigned()) * 0.5f;
            memory[index] = value;
        }
    }

    auto wrapped = false;
    if (currentMode != Mode::locked)
    {
        cursor = (cursor + 1u) % activeLength;
        wrapped = cursor == 0;
    }
    return { value, index, mutated, wrapped, currentMode };
}

float EvolutionLoopMachine::memoryValue(std::size_t index) const noexcept
{
    return index < activeLength ? memory[index] : 0.0f;
}

float EvolutionLoopMachine::nextUnit() noexcept
{
    randomState = randomState * 1664525u + 1013904223u;
    return static_cast<float>((randomState >> 8u) & 0x00ffffffu) / 16777215.0f;
}
}
