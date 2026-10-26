// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/ControlEventExtractor.h"
#include "core/Numeric.h"

#include <algorithm>
#include <cmath>

namespace aquorbium
{
void ControlEventExtractor::prepare(double newSampleRate) noexcept
{
    sampleRate = numeric::sampleRate(newSampleRate);
    updateCoefficients();
    reset();
}

void ControlEventExtractor::reset(float initialSignal) noexcept
{
    const auto signal = numeric::clampFinite(initialSignal, -1.0f, 1.0f, 0.0f);
    signState = signal > controls.zeroHysteresis ? 1
        : (signal < -controls.zeroHysteresis ? -1 : 0);
    envelope = std::abs(signal);
    gate = envelope >= controls.envelopeThreshold;
}

void ControlEventExtractor::setParameters(Parameters value) noexcept
{
    controls.zeroHysteresis = numeric::clampFinite(
        value.zeroHysteresis, 0.0f, 0.25f, controls.zeroHysteresis);
    controls.envelopeThreshold = numeric::clampFinite(
        value.envelopeThreshold, 0.001f, 1.0f, controls.envelopeThreshold);
    controls.envelopeHysteresis = numeric::clampFinite(
        value.envelopeHysteresis, 0.0f, 0.5f, controls.envelopeHysteresis);
    controls.attackMs = numeric::clampFinite(
        value.attackMs, 0.1f, 2000.0f, controls.attackMs);
    controls.releaseMs = numeric::clampFinite(
        value.releaseMs, 0.1f, 5000.0f, controls.releaseMs);
    updateCoefficients();
}

void ControlEventExtractor::updateCoefficients() noexcept
{
    attackGain = static_cast<float>(1.0 - std::exp(
        -1.0 / (sampleRate * controls.attackMs * 0.001)));
    releaseGain = static_cast<float>(1.0 - std::exp(
        -1.0 / (sampleRate * controls.releaseMs * 0.001)));
}

ControlEventExtractor::Events ControlEventExtractor::tick(float input) noexcept
{
    const auto signal = numeric::clampFinite(input, -1.0f, 1.0f, 0.0f);
    Events result;
    if (signal > controls.zeroHysteresis && signState <= 0)
    {
        result.risingZero = signState < 0;
        signState = 1;
    }
    else if (signal < -controls.zeroHysteresis && signState >= 0)
    {
        result.fallingZero = signState > 0;
        signState = -1;
    }

    const auto magnitude = std::abs(signal);
    const auto coefficient = magnitude > envelope ? attackGain : releaseGain;
    envelope += (magnitude - envelope) * coefficient;
    const auto lowerThreshold = std::max(
        0.0f, controls.envelopeThreshold - controls.envelopeHysteresis);
    if (!gate && envelope >= controls.envelopeThreshold)
    {
        gate = true;
        result.envelopeRise = true;
    }
    else if (gate && envelope <= lowerThreshold)
    {
        gate = false;
        result.envelopeFall = true;
    }
    result.envelopeGate = gate;
    result.envelope = envelope;
    return result;
}
}
