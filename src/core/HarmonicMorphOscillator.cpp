// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/HarmonicMorphOscillator.h"

#include "core/Numeric.h"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace aquorbium
{
HarmonicMorphOscillator::HarmonicMorphOscillator() noexcept
{
    familyA[0] = 1.0f;
    for (std::size_t index = 0; index < activeHarmonics; ++index)
        familyB[index] = 1.0f / static_cast<float>(index + 1);
    updateCoefficients();
}

void HarmonicMorphOscillator::prepare(double newSampleRate) noexcept
{
    sampleRate = numeric::sampleRate(newSampleRate);
    reset();
}

void HarmonicMorphOscillator::reset(float newPhase) noexcept
{
    phase = static_cast<double>(numeric::wrapUnit(newPhase));
}

bool HarmonicMorphOscillator::setFamilies(
    const std::array<float, maximumHarmonics>& first,
    const std::array<float, maximumHarmonics>& second,
    std::size_t count) noexcept
{
    if (count == 0 || count > maximumHarmonics)
        return false;
    auto energyA = 0.0f;
    auto energyB = 0.0f;
    for (std::size_t index = 0; index < count; ++index)
    {
        if (!std::isfinite(first[index]) || !std::isfinite(second[index]))
            return false;
        energyA += std::abs(first[index]);
        energyB += std::abs(second[index]);
    }
    if (energyA <= 0.0f || energyB <= 0.0f)
        return false;
    familyA.fill(0.0f);
    familyB.fill(0.0f);
    for (std::size_t index = 0; index < count; ++index)
    {
        familyA[index] = std::clamp(first[index], -1.0f, 1.0f);
        familyB[index] = std::clamp(second[index], -1.0f, 1.0f);
    }
    activeHarmonics = count;
    updateCoefficients();
    return true;
}

void HarmonicMorphOscillator::setFrequency(float value) noexcept
{
    frequency = numeric::clampFinite(
        value, 0.0f, static_cast<float>(sampleRate * 0.45), frequency);
}

void HarmonicMorphOscillator::setMorph(float value) noexcept
{
    morph = numeric::unit(value, morph);
    updateCoefficients();
}

void HarmonicMorphOscillator::setLevel(float value) noexcept
{
    level = numeric::unit(value, level);
}

float HarmonicMorphOscillator::tick() noexcept
{
    auto result = 0.0;
    const auto harmonicLimit = sampleRate * 0.45;
    for (std::size_t index = 0; index < activeHarmonics; ++index)
    {
        const auto harmonic = static_cast<double>(index + 1);
        if (static_cast<double>(frequency) * harmonic >= harmonicLimit)
            break;
        result += static_cast<double>(coefficients[index])
            * std::sin(2.0 * std::numbers::pi_v<double> * phase * harmonic);
    }
    phase += static_cast<double>(frequency) / sampleRate;
    phase -= std::floor(phase);
    return std::clamp(static_cast<float>(result) * normalization * level,
                      -1.0f, 1.0f);
}

float HarmonicMorphOscillator::coefficient(std::size_t harmonic) const noexcept
{
    return harmonic < activeHarmonics ? coefficients[harmonic] : 0.0f;
}

void HarmonicMorphOscillator::updateCoefficients() noexcept
{
    auto absoluteSum = 0.0f;
    for (std::size_t index = 0; index < activeHarmonics; ++index)
    {
        coefficients[index] = familyA[index]
            + (familyB[index] - familyA[index]) * morph;
        absoluteSum += std::abs(coefficients[index]);
    }
    normalization = absoluteSum > 1.0f ? 1.0f / absoluteSum : 1.0f;
}
}
