// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include <algorithm>
#include <cmath>

namespace aquorbium::numeric
{
constexpr double defaultSampleRate = 48000.0;
constexpr double minimumSampleRate = 8000.0;
constexpr double maximumSampleRate = 768000.0;

inline double sampleRate(double value) noexcept
{
    return std::isfinite(value)
        ? std::clamp(value, minimumSampleRate, maximumSampleRate)
        : defaultSampleRate;
}

inline float clampFinite(float value, float minimum, float maximum,
                         float fallback) noexcept
{
    return std::isfinite(value) ? std::clamp(value, minimum, maximum) : fallback;
}

inline float unit(float value, float fallback) noexcept
{
    return clampFinite(value, 0.0f, 1.0f, fallback);
}

inline float wrapUnit(float value, float fallback = 0.0f) noexcept
{
    return std::isfinite(value) ? value - std::floor(value) : fallback;
}
}
