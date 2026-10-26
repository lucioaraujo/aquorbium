// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/EvolutionLoopMachine.h"
#include "core/ProbabilityDistribution.h"
#include "core/ShiftRegisterMemory.h"

#include <array>
#include <bit>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <utility>

namespace
{
bool shiftRegisterShiftsAndMutates()
{
    static_assert(noexcept(std::declval<aquorbium::ShiftRegisterMemory&>()
        .advance(false, 0.0f)));
    aquorbium::ShiftRegisterMemory memory;
    memory.setLength(4);
    memory.reset(0b1011u);
    const auto first = memory.advance(false, 0.0f);
    const auto second = memory.advance(true, 1.0f);
    return first.outputBit && !first.insertedBit && !first.mutated
        && first.pattern == 0b0110u && !second.outputBit
        && !second.insertedBit && second.mutated
        && second.pattern == 0b1100u;
}

bool shiftRegisterIsSeededAndBounded()
{
    aquorbium::ShiftRegisterMemory first, repeated;
    for (auto* memory : { &first, &repeated })
    {
        memory->setLength(32);
        memory->setSeed(9241u);
        memory->reset(0xa55aa55au);
    }
    for (int step = 0; step < 1000; ++step)
    {
        const auto a = first.advance(step % 3 == 0, 0.37f);
        const auto b = repeated.advance(step % 3 == 0, 0.37f);
        if (a.outputBit != b.outputBit || a.insertedBit != b.insertedBit
            || a.mutated != b.mutated || a.pattern != b.pattern)
            return false;
    }
    first.setLength(0);
    first.reset(0xffffffffu);
    return first.length() == 1 && first.pattern() == 1u;
}

bool probabilityMorphChangesDistribution()
{
    static_assert(noexcept(std::declval<aquorbium::ProbabilityDistribution&>()
        .sample(0.0f)));
    aquorbium::ProbabilityDistribution uniform, gaussian;
    uniform.setSeed(371u);
    gaussian.setSeed(371u);
    int uniformCentre = 0;
    int gaussianCentre = 0;
    constexpr int sampleCount = 20000;
    for (int index = 0; index < sampleCount; ++index)
    {
        const auto flat = uniform.sample(0.0f);
        const auto bell = gaussian.sample(1.0f);
        if (!std::isfinite(flat) || !std::isfinite(bell)
            || std::abs(flat) > 1.0f || std::abs(bell) > 1.0f)
            return false;
        uniformCentre += std::abs(flat) < 0.25f ? 1 : 0;
        gaussianCentre += std::abs(bell) < 0.25f ? 1 : 0;
    }
    return uniformCentre > 4500 && uniformCentre < 5500
        && gaussianCentre > uniformCentre + 2500;
}

bool probabilityMorphIsDeterministic()
{
    aquorbium::ProbabilityDistribution first, repeated;
    first.setSeed(883u);
    repeated.setSeed(883u);
    for (int sample = 0; sample < 4096; ++sample)
    {
        const auto amount = static_cast<float>(sample % 101) / 100.0f;
        const auto a = first.sample(amount);
        const auto b = repeated.sample(amount);
        if (std::bit_cast<std::uint32_t>(a)
            != std::bit_cast<std::uint32_t>(b))
            return false;
    }
    return std::isfinite(first.sample(
        std::numeric_limits<float>::quiet_NaN()));
}

bool evolutionMachineModesAreDistinct()
{
    static_assert(noexcept(std::declval<aquorbium::EvolutionLoopMachine&>()
        .advance(0.0f)));
    aquorbium::EvolutionLoopMachine machine;
    machine.setLength(8);
    machine.setSeed(4771u);
    machine.setMode(aquorbium::EvolutionLoopMachine::Mode::looping);
    std::array<float, 8> firstLoop {};
    for (auto& value : firstLoop)
        value = machine.advance().value;
    for (const auto expected : firstLoop)
        if (std::bit_cast<std::uint32_t>(expected)
            != std::bit_cast<std::uint32_t>(machine.advance().value))
            return false;

    machine.rewind();
    machine.setMode(aquorbium::EvolutionLoopMachine::Mode::evolving);
    for (std::size_t index = 0; index < firstLoop.size(); ++index)
    {
        const auto evolved = machine.advance(1.0f);
        if (!evolved.mutated || evolved.value == firstLoop[index])
            return false;
    }

    machine.rewind();
    machine.setMode(aquorbium::EvolutionLoopMachine::Mode::locked);
    const auto held = machine.advance().value;
    for (int repeat = 0; repeat < 32; ++repeat)
        if (machine.advance().value != held)
            return false;
    return true;
}

bool evolutionMachineIsSeededAndFinite()
{
    aquorbium::EvolutionLoopMachine first, repeated;
    for (auto* machine : { &first, &repeated })
    {
        machine->setLength(64);
        machine->setSeed(7142u);
        machine->setMode(aquorbium::EvolutionLoopMachine::Mode::random);
    }
    for (int step = 0; step < 10000; ++step)
    {
        const auto a = first.advance().value;
        const auto b = repeated.advance().value;
        if (std::bit_cast<std::uint32_t>(a)
                != std::bit_cast<std::uint32_t>(b)
            || !std::isfinite(a) || std::abs(a) > 1.0f)
            return false;
    }
    first.setLength(0);
    return first.length() == 1 && first.memoryValue(1) == 0.0f;
}
}

int main()
{
    const std::pair<const char*, bool (*)()> cases[] {
        { "shift operation", shiftRegisterShiftsAndMutates },
        { "shift seed", shiftRegisterIsSeededAndBounded },
        { "distribution shape", probabilityMorphChangesDistribution },
        { "distribution seed", probabilityMorphIsDeterministic },
        { "evolution modes", evolutionMachineModesAreDistinct },
        { "evolution seed", evolutionMachineIsSeededAndFinite },
    };
    for (const auto& [name, test] : cases)
    {
        if (!test())
        {
            std::cerr << name << " failed\n";
            return EXIT_FAILURE;
        }
    }
    std::cout << "Probabilistic memory tests passed\n";
}
