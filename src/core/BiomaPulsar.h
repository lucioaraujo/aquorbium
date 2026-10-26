// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include "core/Numeric.h"
#include "core/StochasticPulseMask.h"

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace aquorbium
{
// A pulse train whose pulse-to-silence ratio is one parameter. The half-sine
// pulse starts and ends at zero, avoiding the hard click of a rectangular gate.
class BiomaPulsar
{
public:
    void prepare(double value) noexcept { sampleRate = numeric::sampleRate(value); }
    void setSeed(std::uint32_t value) noexcept { pulseMask.setSeed(value); }
    void reset(float value = 0.0f) noexcept
    {
        phase = numeric::wrapUnit(value);
        pulseMask.reset();
    }
    void setFrequency(float value) noexcept
    {
        frequency = numeric::clampFinite(value, 0.0f,
                                         static_cast<float>(sampleRate * 0.45), frequency);
    }
    void setDensity(float value) noexcept { density = numeric::unit(value, density); }
    void setEcology(float crack, float strengthValue, float agilityValue,
                    float intelligenceValue) noexcept
    {
        crackStress = numeric::unit(crack, crackStress);
        strength = numeric::unit(strengthValue, strength);
        agility = numeric::unit(agilityValue, agility);
        intelligence = numeric::unit(intelligenceValue, intelligence);
    }

    [[nodiscard]] float tick() noexcept
    {
        // Integrity opens silence between pulsarets. Agility creates bounded
        // internal clusters, intelligence preserves more complete pulses and
        // strength controls their contour without becoming a gain macro.
        const auto ecologicalDensity = density * (1.0f - crackStress * 0.82f);
        const auto duty = 0.012f + ecologicalDensity * 0.76f;
        const auto pulse = phase < duty;
        const auto passProbability = std::clamp(
            1.0f - crackStress * (0.35f + (1.0f - intelligence) * 0.45f),
            0.12f, 1.0f);
        const auto mask = pulseMask.process(pulse, passProbability);
        float result = 0.0f;
        if (mask.gate)
        {
            const auto localPhase = phase / duty;
            const auto envelope = std::sin(localPhase * pi);
            const auto clusterCount = 1.0f
                + static_cast<float>(std::lround(agility * 4.0f));
            const auto clustered = std::sin(localPhase * pi * clusterCount);
            const auto clusterMix = agility * (0.18f + crackStress * 0.42f);
            const auto contour = envelope * (1.0f - clusterMix)
                + clustered * envelope * clusterMix;
            result = contour * (0.82f + strength * 0.18f);
        }
        phase += frequency / static_cast<float>(sampleRate);
        phase -= std::floor(phase);
        return result;
    }

private:
    static constexpr float pi = 3.14159265358979323846f;
    double sampleRate = 48000.0;
    float frequency = 110.0f;
    float density = 0.4f;
    float phase = 0.0f;
    float crackStress = 0.0f;
    float strength = 0.5f;
    float agility = 0.0f;
    float intelligence = 0.5f;
    StochasticPulseMask pulseMask;
};
}
