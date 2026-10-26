// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/FractionalLogicClock.h"
#include "core/Numeric.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace aquorbium
{
FractionalLogicClock::Ratio FractionalLogicClock::sanitized(Ratio value) noexcept
{
    value.numerator = static_cast<std::uint8_t>(
        std::clamp(static_cast<int>(value.numerator), 1, 32));
    value.denominator = static_cast<std::uint8_t>(
        std::clamp(static_cast<int>(value.denominator), 1, 32));
    return value;
}

void FractionalLogicClock::prepare(double newSampleRate) noexcept
{
    sampleRate = numeric::sampleRate(newSampleRate);
    setParameters(controls);
    reset();
}

void FractionalLogicClock::reset(double newPhaseA, double newPhaseB) noexcept
{
    phaseA = std::isfinite(newPhaseA) ? newPhaseA - std::floor(newPhaseA) : 0.5;
    phaseB = std::isfinite(newPhaseB) ? newPhaseB - std::floor(newPhaseB) : 0.5;
    cyclesA = 0;
    cyclesB = 0;
    previousGate = combine(phaseA < controls.duty, phaseB < controls.duty,
                           controls.logic);
}

void FractionalLogicClock::setParameters(Parameters value) noexcept
{
    controls.beatsPerMinute = numeric::clampFinite(
        value.beatsPerMinute, 20.0f, 400.0f, controls.beatsPerMinute);
    controls.branchA = sanitized(value.branchA);
    controls.branchB = sanitized(value.branchB);
    controls.duty = numeric::clampFinite(value.duty, 0.01f, 0.49f, controls.duty);
    switch (value.logic)
    {
        case Logic::And:
        case Logic::Or:
        case Logic::Xor:
            controls.logic = value.logic;
            break;
        default:
            break;
    }
    const auto beatsPerSecond = static_cast<double>(controls.beatsPerMinute) / 60.0;
    incrementA = beatsPerSecond * controls.branchA.numerator
        / controls.branchA.denominator / sampleRate;
    incrementB = beatsPerSecond * controls.branchB.numerator
        / controls.branchB.denominator / sampleRate;
}

bool FractionalLogicClock::combine(bool left, bool right, Logic logic) noexcept
{
    switch (logic)
    {
        case Logic::And: return left && right;
        case Logic::Or: return left || right;
        case Logic::Xor: return left != right;
    }
    return false;
}

FractionalLogicClock::Output FractionalLogicClock::tick() noexcept
{
    phaseA += incrementA;
    phaseB += incrementB;
    if (phaseA >= 1.0)
    {
        phaseA -= std::floor(phaseA);
        if (cyclesA != std::numeric_limits<std::uint64_t>::max()) ++cyclesA;
    }
    if (phaseB >= 1.0)
    {
        phaseB -= std::floor(phaseB);
        if (cyclesB != std::numeric_limits<std::uint64_t>::max()) ++cyclesB;
    }
    const auto branchAGate = phaseA < controls.duty;
    const auto branchBGate = phaseB < controls.duty;
    const auto gate = combine(branchAGate, branchBGate, controls.logic);
    const auto trigger = gate && !previousGate;
    previousGate = gate;
    return { branchAGate, branchBGate, gate, trigger, cyclesA, cyclesB };
}
}
