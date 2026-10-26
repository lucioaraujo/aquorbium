// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/SoundChannel.h"

#include <algorithm>
#include <cmath>

namespace aquorbium
{
float SoundChannel::axisHeight(float temperatureC) noexcept
{
    // the axis depth as a fraction of the sphere's height: 0.06 (just under
    // the surface) in polar water, 0.36 in warm water
    const auto warmth = std::clamp(temperatureC / 30.0f, 0.0f, 1.0f);
    const auto fraction = 0.06f + 0.30f * warmth;
    return sphereRadius - 2.0f * sphereRadius * fraction;
}

float SoundChannel::speedRatio(float y, float axisY) noexcept
{
    constexpr float eps = 0.00737f;
    constexpr float widthMetres = 1300.0f;
    const auto metresPerUnit = oceanDepthMetres / (2.0f * sphereRadius);
    const auto eta = 2.0f * (axisY - y) * metresPerUnit / widthMetres;  // depth below the axis
    return 1.0f + eps * (eta + std::exp(-eta) - 1.0f);
}

float SoundChannel::trapping(float y, float axisY) noexcept
{
    const auto x = (y - axisY) / halfWidth;
    return std::exp(-x * x);
}

float SoundChannel::gain(float sourceY, float listenerY, float distance, float axisY) noexcept
{
    const auto d = std::max(0.0f, distance);
    const auto s = trapping(sourceY, axisY);
    const auto l = trapping(listenerY, axisY);
    // ducted: cylindrical spreading, 1/sqrt(1+d) instead of 1/(1+d)
    const auto ducted = s * l;
    const auto along = ducted * std::sqrt(1.0f + d) + (1.0f - ducted);
    // shadow: one end in the channel, the other far from it; it deepens
    // with range (near the source the direct path still arrives)
    const auto mismatch = std::abs(s - l);
    const auto range = std::clamp(d / 1.5f, 0.0f, 1.0f);
    const auto shadow = 1.0f - 0.8f * mismatch * range;
    return along * shadow;
}
}
