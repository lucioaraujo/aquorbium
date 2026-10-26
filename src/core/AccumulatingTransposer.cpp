// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/AccumulatingTransposer.h"

#include <algorithm>
#include <cmath>

namespace aquorbium
{
void AccumulatingTransposer::setParameters(const Parameters& value) noexcept
{
    auto minimum = std::clamp(value.minimumDegree, -128, 128);
    auto maximum = std::clamp(value.maximumDegree, -128, 128);
    if (minimum > maximum)
        std::swap(minimum, maximum);
    state.minimumDegree = minimum;
    state.maximumDegree = maximum;
    state.stepDegrees = std::clamp(value.stepDegrees, 1, 32);
    current = std::clamp(current, minimum, maximum);
}

void AccumulatingTransposer::reset(int degreeOffset) noexcept
{
    current = std::clamp(
        degreeOffset, state.minimumDegree, state.maximumDegree);
    eventCounter = 0;
}

AccumulatingTransposer::Output AccumulatingTransposer::advance(
    float seasonPhase) noexcept
{
    auto phase = std::isfinite(seasonPhase) ? seasonPhase : 0.0f;
    phase -= std::floor(phase);
    const auto direction = phase < 0.5f ? 1 : -1;
    const auto candidate = current + direction * state.stepDegrees;
    const auto next = std::clamp(
        candidate, state.minimumDegree, state.maximumDegree);

    Output result;
    result.degreeOffset = next;
    result.direction = direction;
    result.boundaryReached = next != candidate;
    result.event = eventCounter++;
    current = next;
    return result;
}
}
