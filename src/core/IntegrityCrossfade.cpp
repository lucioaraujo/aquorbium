// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/IntegrityCrossfade.h"

#include "core/Numeric.h"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace aquorbium
{
void IntegrityCrossfade::prepare(double newSampleRate) noexcept
{
    sampleRate = numeric::sampleRate(newSampleRate);
    updateCoefficient();
    reset();
}

void IntegrityCrossfade::setThresholds(float healthy, float cracked) noexcept
{
    const auto low = numeric::unit(healthy, healthyThreshold);
    const auto high = numeric::unit(cracked, crackedThreshold);
    healthyThreshold = std::min(low, high - 0.01f);
    healthyThreshold = std::max(0.0f, healthyThreshold);
    crackedThreshold = std::max(high, healthyThreshold + 0.01f);
    crackedThreshold = std::min(1.0f, crackedThreshold);
}

void IntegrityCrossfade::setTransitionSeconds(float value) noexcept
{
    transitionSeconds = numeric::clampFinite(
        value, 0.001f, 60.0f, transitionSeconds);
    updateCoefficient();
}

void IntegrityCrossfade::setStress(float value) noexcept
{
    target = mapStress(numeric::unit(value, target));
}

void IntegrityCrossfade::reset(float stress) noexcept
{
    target = mapStress(numeric::unit(stress, 0.0f));
    current = target;
}

IntegrityCrossfade::Output IntegrityCrossfade::tick() noexcept
{
    current += (target - current) * coefficient;
    current = std::clamp(current, 0.0f, 1.0f);
    const auto angle = current * std::numbers::pi_v<float> * 0.5f;
    return { current, std::cos(angle), std::sin(angle) };
}

void IntegrityCrossfade::updateCoefficient() noexcept
{
    coefficient = static_cast<float>(1.0 - std::exp(
        -6.907755278982137 / (static_cast<double>(transitionSeconds) * sampleRate)));
}

float IntegrityCrossfade::mapStress(float value) const noexcept
{
    const auto normalized = std::clamp(
        (value - healthyThreshold) / (crackedThreshold - healthyThreshold),
        0.0f, 1.0f);
    return normalized * normalized * (3.0f - 2.0f * normalized);
}
}
