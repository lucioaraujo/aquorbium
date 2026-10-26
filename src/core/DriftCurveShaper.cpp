// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/DriftCurveShaper.h"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace aquorbium
{
void DriftCurveShaper::setParameters(const Parameters& value) noexcept
{
    state.smoothness = std::clamp(
        std::isfinite(value.smoothness) ? value.smoothness : state.smoothness,
        0.0f, 1.0f);
    state.curvature = std::clamp(
        std::isfinite(value.curvature) ? value.curvature : state.curvature,
        0.0f, 1.0f);
    const auto raw = static_cast<std::uint8_t>(value.family);
    if (raw <= static_cast<std::uint8_t>(Family::peak))
        state.family = value.family;
}

float DriftCurveShaper::shape(float normalizedPhase) const noexcept
{
    const auto phase = std::clamp(
        std::isfinite(normalizedPhase) ? normalizedPhase : 0.0f,
        0.0f, 1.0f);
    const auto continuous = std::clamp(continuousCurve(phase), 0.0f, 1.0f);
    constexpr auto levels = 8.0f;
    const auto stepped = std::round(continuous * levels) / levels;
    return stepped + (continuous - stepped) * state.smoothness;
}

float DriftCurveShaper::continuousCurve(float phase) const noexcept
{
    const auto exponent = 1.0f + state.curvature * 7.0f;
    switch (state.family)
    {
        case Family::linear:
            return phase;
        case Family::exponential:
            return std::pow(phase, exponent);
        case Family::inverseExponential:
            return 1.0f - std::pow(1.0f - phase, exponent);
        case Family::bezierEase:
        {
            const auto smooth = phase * phase * (3.0f - 2.0f * phase);
            return phase + (smooth - phase) * state.curvature;
        }
        case Family::arch:
            return std::sin(std::numbers::pi_v<float> * phase);
        case Family::peak:
            return 1.0f - std::abs(2.0f * phase - 1.0f);
    }
    return phase;
}
}
