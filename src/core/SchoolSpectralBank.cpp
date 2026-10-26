// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/SchoolSpectralBank.h"

#include "core/Numeric.h"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace aquorbium
{
namespace
{
std::uint32_t nextHash(std::uint32_t& state) noexcept
{
    state ^= state << 13u;
    state ^= state >> 17u;
    state ^= state << 5u;
    return state;
}
}

float SchoolSpectralBank::Biquad::tick(float input) noexcept
{
    const auto output = b0 * input + z1;
    z1 = b1 * input - a1 * output + z2;
    z2 = b2 * input - a2 * output;
    if (!std::isfinite(output) || !std::isfinite(z1) || !std::isfinite(z2))
    {
        reset();
        return 0.0f;
    }
    if (std::abs(z1) < 1.0e-20f) z1 = 0.0f;
    if (std::abs(z2) < 1.0e-20f) z2 = 0.0f;
    return output;
}

void SchoolSpectralBank::prepare(double sampleRate) noexcept
{
    preparedSampleRate = numeric::sampleRate(sampleRate);
    updateCoefficients();
    reset();
}

void SchoolSpectralBank::reset() noexcept
{
    for (auto& filter : filters)
        filter.reset();
}

void SchoolSpectralBank::setParameters(Parameters value) noexcept
{
    controls.centre = numeric::unit(value.centre, controls.centre);
    controls.span = numeric::unit(value.span, controls.span);
    controls.resonance = numeric::unit(value.resonance, controls.resonance);
    updateCoefficients();
}

void SchoolSpectralBank::setIdentity(std::uint32_t schoolId,
                                     std::uint32_t seed) noexcept
{
    auto state = seed ^ (schoolId * 0x9e3779b9u) ^ 0x85ebca6bu;
    if (state == 0u)
        state = 1u;
    auto gainSum = 0.0f;
    for (auto& gain : gains)
    {
        const auto unit = static_cast<float>((nextHash(state) >> 8u) & 0x00ffffffu)
            / 16777215.0f;
        gain = 0.35f + unit * 0.65f;
        gainSum += gain;
    }
    gainNormalization = 1.0f / std::max(gainSum, 1.0f);
}

SchoolSpectralBank::Output SchoolSpectralBank::tick(float input) noexcept
{
    const auto safeInput = numeric::clampFinite(input, -4.0f, 4.0f, 0.0f);
    Output output;
    for (std::size_t index = 0; index < bandCount; ++index)
    {
        output.band[index] = filters[index].tick(safeInput) * gains[index];
        output.mix += output.band[index];
    }
    output.mix *= gainNormalization;
    if (!std::isfinite(output.mix))
    {
        reset();
        return {};
    }
    return output;
}

SchoolSpectralBank::Telemetry SchoolSpectralBank::telemetry() const noexcept
{
    return { frequencies, gains };
}

void SchoolSpectralBank::updateCoefficients() noexcept
{
    const auto centreHz = 80.0f * std::exp2(controls.centre * 7.0f);
    const auto spanOctaves = 0.25f + controls.span * 5.75f;
    const auto q = 0.7f + controls.resonance * 7.3f;
    const auto maximumFrequency = static_cast<float>(preparedSampleRate * 0.42);

    for (std::size_t index = 0; index < bandCount; ++index)
    {
        const auto normalizedIndex = static_cast<float>(index)
            / static_cast<float>(bandCount - 1);
        const auto octaveOffset = (normalizedIndex - 0.5f) * spanOctaves;
        const auto frequency = std::clamp(
            centreHz * std::exp2(octaveOffset), 35.0f, maximumFrequency);
        frequencies[index] = frequency;

        const auto omega = 2.0f * std::numbers::pi_v<float> * frequency
            / static_cast<float>(preparedSampleRate);
        const auto sine = std::sin(omega);
        const auto alpha = sine / (2.0f * q);
        const auto inverseA0 = 1.0f / (1.0f + alpha);
        auto& filter = filters[index];
        filter.b0 = alpha * inverseA0;
        filter.b1 = 0.0f;
        filter.b2 = -alpha * inverseA0;
        filter.a1 = -2.0f * std::cos(omega) * inverseA0;
        filter.a2 = (1.0f - alpha) * inverseA0;
    }
}
}
