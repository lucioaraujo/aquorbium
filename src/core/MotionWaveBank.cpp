// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/MotionWaveBank.h"

#include "core/Numeric.h"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace aquorbium
{
float MotionWaveBank::evaluate(Shape shape, float phase) noexcept
{
    const auto wrapped = numeric::wrapUnit(phase);
    const auto angle = wrapped * 2.0f * std::numbers::pi_v<float>;
    switch (shape)
    {
        case Shape::triangle:
            return 1.0f - 4.0f * std::abs(wrapped - 0.5f);
        case Shape::smoothPulse:
        {
            const auto sine = std::sin(angle);
            return std::clamp((sine + 0.33f * sine * sine * sine) / 1.33f,
                              -1.0f, 1.0f);
        }
        case Shape::doubleSine:
            return std::sin(angle * 2.0f) * 0.7f + std::sin(angle) * 0.3f;
        case Shape::rampUp:
            return wrapped * 2.0f - 1.0f;
        case Shape::rampDown:
            return 1.0f - wrapped * 2.0f;
        case Shape::square:
            return wrapped < 0.5f ? 1.0f : -1.0f;
        case Shape::asymmetric:
        {
            constexpr auto peak = 0.28f;
            return wrapped < peak
                ? -1.0f + 2.0f * wrapped / peak
                : 1.0f - 2.0f * (wrapped - peak) / (1.0f - peak);
        }
        case Shape::sine:
        case Shape::count:
        default:
            return std::sin(angle);
    }
}

MotionWaveBank::Shape MotionWaveBank::shapeForIdentity(
    std::uint32_t species, std::uint32_t strategy, std::uint32_t seed) noexcept
{
    auto hash = seed ^ (species + 0x9e3779b9u + (seed << 6u) + (seed >> 2u));
    hash ^= strategy + 0x85ebca6bu + (hash << 6u) + (hash >> 2u);
    hash ^= hash >> 16u;
    hash *= 0x7feb352du;
    hash ^= hash >> 15u;
    return static_cast<Shape>(hash % static_cast<std::uint32_t>(shapeCount));
}
}
