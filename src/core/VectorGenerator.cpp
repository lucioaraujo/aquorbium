// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/VectorGenerator.h"

#include "core/Numeric.h"

#include <algorithm>
#include <cmath>

namespace aquorbium
{
void VectorGenerator::prepare(double newSampleRate) noexcept
{
    sampleRate = std::isfinite(newSampleRate)
        ? std::clamp(newSampleRate, 1.0, numeric::maximumSampleRate)
        : numeric::defaultSampleRate;
    setRateHz(rateHz);
    reset();
}

void VectorGenerator::reset(float newPhase) noexcept
{
    phase = static_cast<double>(numeric::wrapUnit(newPhase));
    running = lifecycle == Mode::loop;
}

void VectorGenerator::setMode(Mode value) noexcept
{
    lifecycle = value;
    reset();
}

void VectorGenerator::setRateHz(float value) noexcept
{
    rateHz = numeric::clampFinite(
        value, 0.0001f, static_cast<float>(sampleRate * 0.45), rateHz);
}

void VectorGenerator::setShape(Shape value) noexcept
{
    controls.skew = numeric::clampFinite(
        value.skew, 0.01f, 0.99f, controls.skew);
    controls.curve = numeric::clampFinite(
        value.curve, -1.0f, 1.0f, controls.curve);
    controls.bipolar = value.bipolar;
}

void VectorGenerator::trigger() noexcept
{
    phase = 0.0;
    running = true;
}

float VectorGenerator::tick() noexcept
{
    if (!running)
        return 0.0f;
    const auto output = evaluate(static_cast<float>(phase));
    phase += static_cast<double>(rateHz) / sampleRate;
    constexpr auto wrapTolerance = 1.0e-12;
    if (phase >= 1.0 - wrapTolerance)
    {
        if (lifecycle == Mode::loop)
            phase = phase < 1.0 ? 0.0 : phase - std::floor(phase);
        else
        {
            phase = 1.0;
            running = false;
        }
    }
    return output;
}

float VectorGenerator::currentPhase() const noexcept
{
    return static_cast<float>(phase);
}

float VectorGenerator::evaluate(float value) const noexcept
{
    auto unipolar = value < controls.skew
        ? value / controls.skew
        : 1.0f - (value - controls.skew) / (1.0f - controls.skew);
    unipolar = std::clamp(unipolar, 0.0f, 1.0f);
    const auto exponent = std::exp2(std::abs(controls.curve) * 3.0f);
    if (controls.curve > 0.0f)
        unipolar = std::pow(unipolar, exponent);
    else if (controls.curve < 0.0f)
        unipolar = 1.0f - std::pow(1.0f - unipolar, exponent);
    return controls.bipolar ? unipolar * 2.0f - 1.0f : unipolar;
}
}
