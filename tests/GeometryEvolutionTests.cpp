// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/DriftCurveShaper.h"
#include "core/PatternRotationMutator.h"
#include "core/StepMemoryLayout.h"

#include <array>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <utility>

namespace
{
using Layout = aquorbium::StepMemoryLayout;
using Mutator = aquorbium::PatternRotationMutator;
using Shaper = aquorbium::DriftCurveShaper;

bool close(float left, float right, float tolerance = 1.0e-6f)
{
    return std::abs(left - right) <= tolerance;
}

bool rotationSchedulePreservesPulses()
{
    static_assert(noexcept(std::declval<Mutator&>().advanceBar()));
    Mutator mutator;
    std::array<std::uint8_t, Mutator::maximumSteps> pattern {};
    pattern[0] = 1;
    pattern[3] = 1;
    if (!mutator.setPattern(pattern, 8))
        return false;
    mutator.setSchedule(3, 2);
    const auto first = mutator.advanceBar();
    const auto second = mutator.advanceBar();
    const auto third = mutator.advanceBar();
    return !first.mutated && first.barsUntilMutation == 2
        && !second.mutated && second.barsUntilMutation == 1
        && third.mutated && third.barsUntilMutation == 3
        && third.mutationCount == 1 && mutator.pulses() == 2
        && mutator.at(1) && mutator.at(6);
}

bool rotationDirectionAndInvalidPatternAreDefined()
{
    Mutator mutator;
    std::array<std::uint8_t, Mutator::maximumSteps> pattern {};
    pattern[0] = 5;
    if (!mutator.setPattern(pattern, 8)
        || mutator.setPattern(pattern, 0)
        || mutator.setPattern(pattern, Mutator::maximumSteps + 1))
        return false;
    mutator.setSchedule(0, -1);
    const auto output = mutator.advanceBar();
    if (!output.mutated || !mutator.at(1) || mutator.pulses() != 1)
        return false;
    mutator.setSchedule(1, 0);
    return !mutator.advanceBar().mutated && mutator.at(1);
}

bool memoryLayoutsCoverAllThirtyTwoAddresses()
{
    Layout layout;
    for (std::size_t track = 0; track < 4; ++track)
    {
        const auto address = layout.map(track, 9);
        if (address.index != track * 8 + 1 || address.logicalLength != 8
            || address.group != track)
            return false;
    }
    layout.setMode(Layout::Mode::serial2x16);
    const auto pairA = layout.map(1, 17);
    const auto pairB = layout.map(3, 15);
    if (pairA.index != 1 || pairA.logicalLength != 16 || pairA.group != 0
        || pairB.index != 31 || pairB.group != 1)
        return false;
    layout.setMode(Layout::Mode::serial1x32);
    const auto serial = layout.map(99, 33);
    if (serial.index != 1 || serial.logicalLength != 32 || serial.group != 0)
        return false;
    layout.setMode(static_cast<Layout::Mode>(255));
    return layout.mode() == Layout::Mode::parallel4x8;
}

bool smoothnessAndFamilyAreOrthogonal()
{
    static_assert(noexcept(std::declval<const Shaper&>().shape(0.0f)));
    Shaper shaper;
    shaper.setParameters({ 1.0f, 0.5f, Shaper::Family::linear });
    const auto continuous = shaper.shape(0.30f);
    shaper.setParameters({ 0.0f, 0.5f, Shaper::Family::linear });
    const auto stepped = shaper.shape(0.30f);
    if (!close(continuous, 0.30f) || !close(stepped, 0.25f))
        return false;

    shaper.setParameters({ 1.0f, 0.8f, Shaper::Family::exponential });
    const auto exponential = shaper.shape(0.40f);
    shaper.setParameters({ 1.0f, 0.8f, Shaper::Family::inverseExponential });
    const auto inverse = shaper.shape(0.40f);
    shaper.setParameters({ 1.0f, 0.8f, Shaper::Family::arch });
    const auto arch = shaper.shape(0.25f);
    shaper.setParameters({ 1.0f, 0.8f, Shaper::Family::peak });
    const auto peak = shaper.shape(0.25f);
    return exponential < 0.40f && inverse > 0.40f
        && arch > peak && close(peak, 0.5f);
}

bool curveInputsAreBounded()
{
    Shaper shaper;
    shaper.setParameters({ -2.0f, 4.0f, Shaper::Family::bezierEase });
    const auto parameters = shaper.parameters();
    if (parameters.smoothness != 0.0f || parameters.curvature != 1.0f)
        return false;
    const auto invalid = shaper.shape(std::numeric_limits<float>::quiet_NaN());
    shaper.setParameters({ std::numeric_limits<float>::infinity(),
        std::numeric_limits<float>::quiet_NaN(),
        static_cast<Shaper::Family>(255) });
    return invalid == 0.0f
        && shaper.parameters().family == Shaper::Family::bezierEase
        && shaper.parameters().smoothness == 0.0f
        && shaper.parameters().curvature == 1.0f;
}
}

int main()
{
    const std::pair<const char*, bool (*)()> cases[] {
        { "rotation schedule", rotationSchedulePreservesPulses },
        { "rotation limits", rotationDirectionAndInvalidPatternAreDefined },
        { "memory layout", memoryLayoutsCoverAllThirtyTwoAddresses },
        { "curve axes", smoothnessAndFamilyAreOrthogonal },
        { "curve limits", curveInputsAreBounded },
    };
    for (const auto& [name, test] : cases)
    {
        if (!test())
        {
            std::cerr << name << " failed\n";
            return EXIT_FAILURE;
        }
    }
    std::cout << "Pattern rotation, memory layout and drift curves passed\n";
}
