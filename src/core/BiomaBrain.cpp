// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/BiomaBrain.h"
#include "core/Numeric.h"

#include <algorithm>
#include <cmath>

namespace aquorbium
{
void BiomaBrain::prepare(double newSampleRate) noexcept
{
    sampleRate = numeric::sampleRate(newSampleRate);
    reset();
}

void BiomaBrain::setSeed(std::uint32_t value) noexcept
{
    randomState = value == 0 ? 1u : value;
    reset();
}

void BiomaBrain::reset() noexcept
{
    ecologicalFeedback = 0.0f;
    registerState = static_cast<std::uint8_t>((randomState & 0xffu) | 1u);
    clockPhase = 0.0;
    phase.fill(0.0);
    configureMotion();
    heldCorrelation = correlatedValues();
}

void BiomaBrain::setEcologicalFeedback(float value) noexcept
{
    ecologicalFeedback = numeric::unit(value, 0.0f);
}

float BiomaBrain::nextRandomUnit() noexcept
{
    randomState = randomState * 1664525u + 1013904223u;
    return static_cast<float>((randomState >> 8) & 0x00ffffffu) / 16777215.0f;
}

void BiomaBrain::configureMotion() noexcept
{
    static constexpr std::array<double, organismCount> baseRatios {
        1.0, 1.5, 2.0, 2.5, 3.25
    };
    for (std::size_t index = 0; index < organismCount; ++index)
    {
        ratio[index] = baseRatios[index] * (0.96 + static_cast<double>(nextRandomUnit()) * 0.08);
        phase[index] = static_cast<double>(nextRandomUnit());
    }
}

void BiomaBrain::clockCorrelatedRegister() noexcept
{
    const auto feedback = static_cast<std::uint8_t>(
        ((registerState >> 7u) ^ (registerState >> 5u) ^ (registerState >> 4u) ^ (registerState >> 3u)) & 1u);
    registerState = static_cast<std::uint8_t>((registerState << 1u) | feedback);
    if (registerState == 0)
        registerState = 1u;
    heldCorrelation = correlatedValues();
}

std::array<float, BiomaBrain::organismCount> BiomaBrain::correlatedValues() const noexcept
{
    static constexpr std::array<std::array<float, 8>, organismCount> weights {{
        {{0.50f, 0.25f, 0.13f, 0.06f, 0.03f, 0.015f, 0.008f, 0.004f}},
        {{0.004f, 0.008f, 0.015f, 0.03f, 0.06f, 0.13f, 0.25f, 0.50f}},
        {{0.13f, 0.50f, 0.06f, 0.25f, 0.008f, 0.03f, 0.004f, 0.015f}},
        {{0.03f, 0.004f, 0.25f, 0.008f, 0.50f, 0.015f, 0.13f, 0.06f}},
        {{0.25f, 0.03f, 0.004f, 0.50f, 0.015f, 0.13f, 0.06f, 0.008f}},
    }};
    std::array<float, organismCount> result {};
    for (std::size_t output = 0; output < organismCount; ++output)
    {
        for (std::size_t bit = 0; bit < 8; ++bit)
        {
            const auto isSet = ((registerState >> bit) & 1u) != 0u;
            result[output] += (isSet ? 1.0f : -1.0f) * weights[output][bit];
        }
    }
    if (coordinationValue > 0.0f)
    {
        // the organisms lean together: towards their common mean
        auto mean = 0.0f;
        for (const auto v : result)
            mean += v;
        mean /= static_cast<float>(organismCount);
        for (auto& v : result)
            v += (mean - v) * coordinationValue;
    }
    return result;
}

std::array<float, BiomaBrain::organismCount> BiomaBrain::nextDrift(float amount) noexcept
{
    const auto boundedAmount = std::min(1.0f,
        numeric::unit(amount, 0.0f) * (1.0f + ecologicalFeedback * 0.18f));
    clockPhase += (0.23 * static_cast<double>(clockScaleValue) * (1.0 + static_cast<double>(ecologicalFeedback) * 0.22))
        / sampleRate;
    if (clockPhase >= 1.0)
    {
        clockPhase -= std::floor(clockPhase);
        clockCorrelatedRegister();
    }

    std::array<float, organismCount> result {};
    for (std::size_t index = 0; index < organismCount; ++index)
    {
        phase[index] += (0.018 * ratio[index]) / sampleRate;
        phase[index] -= std::floor(phase[index]);
        const auto wave = static_cast<float>(std::sin(phase[index] * 6.283185307179586));
        result[index] = (wave * 0.72f + heldCorrelation[index] * 0.28f) * boundedAmount;
    }
    return result;
}
}
