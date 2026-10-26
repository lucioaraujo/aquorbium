// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/CoreRefinementModules.h"

#include "core/Numeric.h"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace aquorbium
{
void OversampledSineFolder::reset() noexcept
{
    previousInput = 0.0f;
    initialized = false;
}

float OversampledSineFolder::process(float input, float amount,
                                     float drive) noexcept
{
    input = numeric::clampFinite(input, -4.0f, 4.0f, 0.0f);
    amount = numeric::unit(amount, 0.0f);
    drive = numeric::clampFinite(drive, 0.25f, 32.0f, 1.0f);
    if (!initialized)
    {
        previousInput = input;
        initialized = true;
    }
    if (amount == 0.0f)
    {
        previousInput = input;
        return input;
    }

    auto shaped = 0.0f;
    for (int phase = 1; phase <= 4; ++phase)
    {
        const auto fraction = static_cast<float>(phase) * 0.25f;
        const auto interpolated = previousInput
            + (input - previousInput) * fraction;
        shaped += std::sin(interpolated * drive);
    }
    previousInput = input;
    shaped *= 0.25f;
    return std::clamp(input * (1.0f - amount) + shaped * amount,
                      -4.0f, 4.0f);
}

void BoundedMasterFeedback::prepare(double updateRateHz) noexcept
{
    updateRate = std::isfinite(updateRateHz)
        ? std::clamp(updateRateHz, 1.0, 2000.0) : 200.0;
    updateCoefficients();
    reset();
}

void BoundedMasterFeedback::reset() noexcept
{
    current = 0.0f;
}

void BoundedMasterFeedback::setParameters(Parameters value) noexcept
{
    controls.amount = numeric::unit(value.amount, controls.amount);
    controls.attackSeconds = numeric::clampFinite(
        value.attackSeconds, 0.005f, 2.0f, controls.attackSeconds);
    controls.releaseSeconds = numeric::clampFinite(
        value.releaseSeconds, 0.02f, 8.0f, controls.releaseSeconds);
    controls.maximum = numeric::clampFinite(
        value.maximum, 0.0f, 0.75f, controls.maximum);
    updateCoefficients();
}

void BoundedMasterFeedback::updateCoefficients() noexcept
{
    attack = static_cast<float>(-std::expm1(
        -1.0 / (updateRate * controls.attackSeconds)));
    release = static_cast<float>(-std::expm1(
        -1.0 / (updateRate * controls.releaseSeconds)));
}

float BoundedMasterFeedback::process(
    const std::array<float, 3>& bandReductionDb,
    float glueReductionDb, float masterInfluence) noexcept
{
    auto reduction = 0.0f;
    for (const auto value : bandReductionDb)
        reduction = std::max(reduction,
            numeric::clampFinite(value, 0.0f, 60.0f, 0.0f));
    const auto glue = numeric::clampFinite(
        glueReductionDb, 0.0f, 60.0f, 0.0f);
    const auto normalized = 1.0f - std::exp(-(reduction + glue * 0.5f) / 6.0f);
    const auto target = std::min(controls.maximum,
        normalized * controls.amount * numeric::unit(masterInfluence, 0.0f));
    current += (target - current) * (target > current ? attack : release);
    if (std::abs(target - current) < 1.0e-7f) current = target;
    return current;
}

void AirWaterMedium::prepare(double sampleRate) noexcept
{
    preparedRate = numeric::sampleRate(sampleRate);
    setParameters(controls);
    reset();
}

void AirWaterMedium::reset() noexcept
{
    stateLeft = stateRight = 0.0f;
}

void AirWaterMedium::setParameters(Parameters value) noexcept
{
    controls.waterline = numeric::clampFinite(
        value.waterline, -0.8f, 0.8f, controls.waterline);
    controls.transitionWidth = numeric::clampFinite(
        value.transitionWidth, 0.01f, 0.8f, controls.transitionWidth);
    controls.underwaterDamping = numeric::unit(
        value.underwaterDamping, controls.underwaterDamping);
    controls.underwaterLoss = numeric::unit(
        value.underwaterLoss, controls.underwaterLoss);
}

AirWaterMedium::Output AirWaterMedium::process(
    float left, float right, float normalizedHeight,
    float turbidity, float temperature) noexcept
{
    left = numeric::clampFinite(left, -4.0f, 4.0f, 0.0f);
    right = numeric::clampFinite(right, -4.0f, 4.0f, 0.0f);
    normalizedHeight = numeric::clampFinite(
        normalizedHeight, -1.0f, 1.0f, 0.0f);
    turbidity = numeric::unit(turbidity, 0.35f);
    temperature = numeric::unit(temperature, 0.5f);

    const auto lower = controls.waterline - controls.transitionWidth * 0.5f;
    const auto upper = controls.waterline + controls.transitionWidth * 0.5f;
    const auto transition = std::clamp(
        (normalizedHeight - lower) / (upper - lower), 0.0f, 1.0f);
    const auto smooth = transition * transition * (3.0f - 2.0f * transition);
    const auto water = 1.0f - smooth;
    const auto density = water * (0.55f + turbidity * 0.45f);
    const auto brightness = std::clamp(1.0f
        - density * controls.underwaterDamping
        + (temperature - 0.5f) * 0.10f, 0.08f, 1.0f);
    const auto cutoff = 500.0f * std::exp2(brightness * 5.2f);
    const auto coefficient = 1.0f - std::exp(
        -2.0f * std::numbers::pi_v<float> * cutoff
        / static_cast<float>(preparedRate));
    stateLeft += (left - stateLeft) * coefficient;
    stateRight += (right - stateRight) * coefficient;
    const auto gain = 1.0f - density * controls.underwaterLoss;
    return {
        left * (1.0f - water) + stateLeft * gain * water,
        right * (1.0f - water) + stateRight * gain * water,
        water, density, brightness
    };
}
}
