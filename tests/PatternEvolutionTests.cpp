// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/RandomToLoop.h"
#include "core/StepPatternBank.h"

#include <array>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <utility>

namespace
{
using Bank = aquorbium::StepPatternBank;
using Loop = aquorbium::RandomToLoop;
using Memory = aquorbium::AddressableStepMemory;

bool snapshotsRestoreIndependentPatterns()
{
    static_assert(noexcept(std::declval<Bank&>().save(
        0, std::declval<const Memory&>())));
    Memory source;
    source.setLength(4);
    for (std::size_t index = 0; index < 4; ++index)
        if (!source.setStep(index, {
                static_cast<float>(index * 5),
                0.1f * static_cast<float>(index + 1),
                -0.5f + static_cast<float>(index) * 0.25f,
                0.2f * static_cast<float>(index + 1) }))
            return false;

    Bank bank;
    if (!bank.save(2, source) || !bank.hasPattern(2)
        || bank.save(Bank::maximumSlots, source))
        return false;

    Memory destination;
    destination.setLength(1);
    if (!bank.restore(2, destination, 3) || destination.length() != 4
        || destination.cursor() != 3)
        return false;
    for (std::size_t index = 0; index < 4; ++index)
    {
        const auto& step = destination.step(index);
        if (step.pitchSemitones != static_cast<float>(index * 5)
            || step.desireAmount != 0.2f * static_cast<float>(index + 1))
            return false;
    }
    return !bank.restore(1, destination);
}

bool chainsValidateAndWrap()
{
    Bank bank;
    Memory memory;
    memory.setLength(2);
    if (!bank.save(1, memory) || !bank.save(3, memory))
        return false;
    std::array<std::uint8_t, Bank::maximumChain> chain {};
    chain[0] = 1;
    chain[1] = 3;
    chain[2] = 1;
    if (!bank.setChain(chain, 3) || bank.chainLength() != 3)
        return false;
    const auto first = bank.advanceChain();
    const auto second = bank.advanceChain();
    const auto third = bank.advanceChain();
    const auto wrapped = bank.advanceChain();
    if (first.slot != 1 || first.chainIndex != 0 || first.event != 0
        || second.slot != 3 || second.event != 1
        || third.slot != 1 || !third.wrapped || third.event != 2
        || wrapped.slot != 1 || wrapped.event != 3)
        return false;
    chain[1] = 7;
    return !bank.setChain(chain, 3) && bank.chainLength() == 3;
}

bool loopExtremesAreExact()
{
    static_assert(noexcept(std::declval<Loop&>().process(0.0f, 0.0f)));
    Loop loop;
    loop.setLength(4);
    const std::array<float, 4> seed { -0.8f, -0.2f, 0.3f, 0.9f };
    for (std::size_t index = 0; index < seed.size(); ++index)
    {
        const auto output = loop.process(seed[index], 1.0f);
        if (output.value != seed[index] || output.index != index
            || output.loopReady != (index == seed.size() - 1))
            return false;
    }
    for (std::size_t index = 0; index < seed.size(); ++index)
    {
        const auto output = loop.process(1.0f - seed[index], 1.0f);
        if (output.value != seed[index])
            return false;
    }
    for (std::size_t index = 0; index < seed.size(); ++index)
    {
        const auto live = -1.0f + static_cast<float>(index) * 0.5f;
        if (loop.process(live, 0.0f).value != live)
            return false;
    }
    return true;
}

bool loopMorphAndInvalidInputsAreBounded()
{
    Loop loop;
    loop.setLength(1);
    (void)loop.process(1.0f, 1.0f);
    const auto halfway = loop.process(-1.0f, 0.5f);
    const auto held = loop.process(-1.0f, 1.0f);
    if (halfway.value != 0.0f || held.value != 0.0f
        || !halfway.loopReady || !halfway.wrapped)
        return false;
    const auto invalid = loop.process(
        std::numeric_limits<float>::quiet_NaN(),
        std::numeric_limits<float>::infinity());
    loop.setLength(900);
    return std::isfinite(invalid.value) && invalid.value == 0.0f
        && loop.length() == Loop::maximumLength && loop.captured() == 0;
}
}

int main()
{
    const std::pair<const char*, bool (*)()> cases[] {
        { "snapshots", snapshotsRestoreIndependentPatterns },
        { "chain", chainsValidateAndWrap },
        { "loop extremes", loopExtremesAreExact },
        { "loop morph", loopMorphAndInvalidInputsAreBounded },
    };
    for (const auto& [name, test] : cases)
    {
        if (!test())
        {
            std::cerr << name << " failed\n";
            return EXIT_FAILURE;
        }
    }
    std::cout << "Pattern snapshots and random-to-loop passed\n";
}
