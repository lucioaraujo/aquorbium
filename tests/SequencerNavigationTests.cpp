// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/AccumulatingTransposer.h"
#include "core/ThresholdStepSelector.h"

#include <array>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <utility>

namespace
{
using Selector = aquorbium::ThresholdStepSelector;
using Transposer = aquorbium::AccumulatingTransposer;

bool seasonControlsAccumulationDirection()
{
    static_assert(noexcept(std::declval<Transposer&>().advance(0.0f)));
    Transposer transposer;
    transposer.setParameters({ -2, 2, 2 });
    transposer.reset();

    const auto up = transposer.advance(0.25f);
    const auto upperBoundary = transposer.advance(0.25f);
    const auto down = transposer.advance(0.75f);
    const auto lower = transposer.advance(0.75f);
    const auto lowerBoundary = transposer.advance(0.75f);
    return up.degreeOffset == 2 && up.direction == 1 && !up.boundaryReached
        && upperBoundary.degreeOffset == 2 && upperBoundary.boundaryReached
        && down.degreeOffset == 0 && down.direction == -1
        && lower.degreeOffset == -2 && !lower.boundaryReached
        && lowerBoundary.degreeOffset == -2 && lowerBoundary.boundaryReached
        && lowerBoundary.event == 4;
}

bool transposerSanitizesAndWrapsPhase()
{
    Transposer transposer;
    transposer.setParameters({ 200, -200, 0 });
    const auto parameters = transposer.parameters();
    if (parameters.minimumDegree != -128 || parameters.maximumDegree != 128
        || parameters.stepDegrees != 1)
        return false;
    transposer.reset(900);
    const auto wrappedPhase = transposer.advance(1.25f);
    const auto invalidPhase = transposer.advance(
        std::numeric_limits<float>::quiet_NaN());
    return wrappedPhase.degreeOffset == 128 && wrappedPhase.direction == 1
        && wrappedPhase.boundaryReached && invalidPhase.degreeOffset == 128
        && invalidPhase.direction == 1 && invalidPhase.boundaryReached;
}

bool evenThresholdsSelectAndReportCrossings()
{
    static_assert(noexcept(std::declval<Selector&>().select(0.0f)));
    Selector selector;
    selector.setLength(4);
    const auto first = selector.select(0.0f);
    const auto held = selector.select(0.24f);
    const auto second = selector.select(0.25f);
    const auto fourth = selector.select(1.0f);
    const auto back = selector.select(-1.0f);
    return first.index == 0 && first.changed && first.direction == 0
        && first.transitionCount == 1
        && held.index == 0 && !held.changed && held.transitionCount == 1
        && second.index == 1 && second.changed && second.direction == 1
        && fourth.index == 3 && fourth.changed && fourth.direction == 1
        && back.index == 0 && back.changed && back.direction == -1
        && back.transitionCount == 4;
}

bool customThresholdsAreValidated()
{
    Selector selector;
    std::array<float, Selector::maximumSteps> values {};
    values[0] = 0.10f;
    values[1] = 0.70f;
    values[2] = 0.90f;
    values[3] = 1.0f;
    if (!selector.setThresholds(values, 4) || selector.length() != 4
        || selector.threshold(1) != 0.70f)
        return false;
    if (selector.select(0.09f).index != 0
        || selector.select(0.10f).index != 1
        || selector.select(0.89f).index != 2
        || selector.select(0.95f).index != 3)
        return false;

    auto invalid = values;
    invalid[2] = 0.60f;
    if (selector.setThresholds(invalid, 4) || selector.length() != 4)
        return false;
    invalid = values;
    invalid[3] = std::numeric_limits<float>::infinity();
    if (selector.setThresholds(invalid, 4))
        return false;
    return selector.select(std::numeric_limits<float>::quiet_NaN()).index == 0;
}
}

int main()
{
    const std::pair<const char*, bool (*)()> cases[] {
        { "seasonal accumulation", seasonControlsAccumulationDirection },
        { "transposer limits", transposerSanitizesAndWrapsPhase },
        { "threshold crossings", evenThresholdsSelectAndReportCrossings },
        { "custom thresholds", customThresholdsAreValidated },
    };
    for (const auto& [name, test] : cases)
    {
        if (!test())
        {
            std::cerr << name << " failed\n";
            return EXIT_FAILURE;
        }
    }
    std::cout << "Accumulating transposer and threshold selector passed\n";
}
