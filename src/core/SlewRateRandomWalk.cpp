// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/SlewRateRandomWalk.h"
#include "core/Numeric.h"

#include <algorithm>
#include <cmath>

namespace aquorbium
{
void SlewRateRandomWalk::prepare(double newSampleRate) noexcept
{
    sampleRate = std::isfinite(newSampleRate)
        ? std::clamp(newSampleRate, 1.0, numeric::maximumSampleRate)
        : numeric::defaultSampleRate;
    updateCoefficient();
    reset();
}

void SlewRateRandomWalk::setSeed(std::uint32_t value) noexcept
{
    seed = value == 0u ? 1u : value;
    reset();
}

void SlewRateRandomWalk::reset() noexcept
{
    randomState = seed;
    eventPhase = 0.0;
    target = 0.0f;
    current = 0.0f;
}

void SlewRateRandomWalk::setParameters(Parameters value) noexcept
{
    controls.stepsPerSecond = numeric::clampFinite(value.stepsPerSecond,
        0.0001f, 8.0f, controls.stepsPerSecond);
    controls.stepSize = numeric::unit(value.stepSize, controls.stepSize);
    controls.smoothingHz = numeric::clampFinite(value.smoothingHz,
        0.0001f, 1.9f, controls.smoothingHz);
    updateCoefficient();
}

void SlewRateRandomWalk::updateCoefficient() noexcept
{
    smoothingGain = static_cast<float>(1.0 - std::exp(
        -6.28318530717958647692 * controls.smoothingHz / sampleRate));
}

float SlewRateRandomWalk::nextRandomSigned() noexcept
{
    randomState = randomState * 1664525u + 1013904223u;
    return static_cast<float>((randomState >> 8) & 0x00ffffffu)
        / 8388607.5f - 1.0f;
}

float SlewRateRandomWalk::tick() noexcept
{
    eventPhase += static_cast<double>(controls.stepsPerSecond) / sampleRate;
    if (eventPhase >= 1.0)
    {
        eventPhase -= 1.0;
        // Reflection preserves movement near the limits instead of producing
        // a hard clipped plateau.
        auto candidate = target + nextRandomSigned() * controls.stepSize;
        if (candidate > 1.0f) candidate = 2.0f - candidate;
        if (candidate < -1.0f) candidate = -2.0f - candidate;
        target = std::clamp(candidate, -1.0f, 1.0f);
    }
    current += (target - current) * smoothingGain;
    return current;
}
}
