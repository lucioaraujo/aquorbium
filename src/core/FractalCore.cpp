// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/FractalCore.h"

#include "core/Numeric.h"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace aquorbium
{
void FractalCore::prepare(double newSampleRate) noexcept
{
    sampleRate = std::isfinite(newSampleRate)
        ? std::clamp(newSampleRate, 1.0, numeric::maximumSampleRate)
        : numeric::defaultSampleRate;
    setParameters(controls);
    reset();
}

void FractalCore::reset(float newPhase) noexcept
{
    phase = static_cast<double>(numeric::wrapUnit(newPhase));
}

void FractalCore::setParameters(Parameters value) noexcept
{
    controls.frequencyHz = numeric::clampFinite(
        value.frequencyHz, 0.0001f, static_cast<float>(sampleRate * 0.45),
        controls.frequencyHz);
    controls.replicationRatio = numeric::clampFinite(
        value.replicationRatio, 1.25f, 8.0f, controls.replicationRatio);
    controls.roughness = numeric::clampFinite(
        value.roughness, 0.0f, 0.95f, controls.roughness);
    controls.iterations = std::clamp(
        value.iterations, std::size_t { 1 }, maximumIterations);
}

float FractalCore::tick() noexcept
{
    auto localPhase = phase;
    auto amplitude = 1.0;
    auto normalization = 1.0;
    auto result = std::sin(2.0 * std::numbers::pi_v<double> * localPhase);
    for (std::size_t iteration = 1; iteration < controls.iterations; ++iteration)
    {
        localPhase = localPhase * static_cast<double>(controls.replicationRatio)
            + static_cast<double>(controls.roughness) * result * 0.125;
        localPhase -= std::floor(localPhase);
        amplitude *= static_cast<double>(controls.roughness);
        result += amplitude
            * std::sin(2.0 * std::numbers::pi_v<double> * localPhase);
        normalization += amplitude;
    }
    phase += static_cast<double>(controls.frequencyHz) / sampleRate;
    phase -= std::floor(phase);
    return std::clamp(static_cast<float>(result / normalization), -1.0f, 1.0f);
}

float FractalCore::currentPhase() const noexcept
{
    return static_cast<float>(phase);
}
}
