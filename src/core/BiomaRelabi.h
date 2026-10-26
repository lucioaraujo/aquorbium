// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include "core/Numeric.h"

#include <array>
#include <cmath>
#include <numbers>

namespace aquorbium
{
// A deterministic four-rate temporal field. Unlike a clock or a random
// source, its related events recur without resolving into one fixed cycle.
// It is intended for control-rate decisions, never as an audio oscillator.
class BiomaRelabi
{
public:
    static constexpr std::size_t oscillatorCount = 4;

    void prepare(double newSampleRate) noexcept
    {
        // Relabi is also used on control clocks well below audio rate.
        sampleRate = std::isfinite(newSampleRate)
            ? std::clamp(newSampleRate, 1.0, numeric::maximumSampleRate)
            : numeric::defaultSampleRate;
        reset();
    }

    void reset() noexcept { phase.fill(0.0); }

    [[nodiscard]] float tick() noexcept
    {
        double sum = 0.0;
        for (std::size_t index = 0; index < oscillatorCount; ++index)
        {
            sum += std::sin(phase[index]);
            phase[index] += twoPi * rates[index] / sampleRate;
            if (phase[index] >= twoPi) phase[index] -= twoPi;
        }
        return static_cast<float>(sum / static_cast<double>(oscillatorCount));
    }

private:
    static constexpr double twoPi = 2.0 * std::numbers::pi_v<double>;
    double sampleRate = 48000.0;
    std::array<double, oscillatorCount> phase {};
    // Ratios deliberately avoid simple common periods in performance time.
    std::array<double, oscillatorCount> rates { 0.137, 0.211, 0.347, 0.509 };
};
}
