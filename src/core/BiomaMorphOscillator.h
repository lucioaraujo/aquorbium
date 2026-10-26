// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include "core/Numeric.h"

#include <cmath>

namespace aquorbium
{
// Continuous morph between a sine, a PolyBLEP saw and a PolyBLEP pulse.
// The discontinuous shapes are corrected at their edges so density can move
// the organism's spectrum without turning high notes into uncontrolled aliasing.
class BiomaMorphOscillator
{
public:
    void prepare(double value) noexcept { sampleRate = numeric::sampleRate(value); }
    void reset(float value = 0.0f) noexcept { phase = numeric::wrapUnit(value); }
    void setFrequency(float value) noexcept
    {
        frequency = numeric::clampFinite(value, 0.0f,
                                         static_cast<float>(sampleRate * 0.45), frequency);
    }
    void setShape(float value) noexcept { shape = numeric::unit(value, shape); }
    void setPulseWidth(float value) noexcept
    {
        pulseWidth = numeric::clampFinite(value, 0.05f, 0.95f, pulseWidth);
    }

    [[nodiscard]] float tick() noexcept
    {
        const auto increment = frequency / static_cast<float>(sampleRate);
        const auto sine = std::sin(phase * twoPi);
        const auto saw = 2.0f * phase - 1.0f - polyBlep(phase, increment);
        auto pulse = phase < pulseWidth ? 1.0f : -1.0f;
        pulse += polyBlep(phase, increment);
        auto shifted = phase + 1.0f - pulseWidth;
        shifted -= std::floor(shifted);
        pulse -= polyBlep(shifted, increment);

        float result = 0.0f;
        if (shape < 0.5f)
            result = interpolate(sine, saw, shape * 2.0f);
        else
            result = interpolate(saw, pulse, (shape - 0.5f) * 2.0f);

        phase += increment;
        phase -= std::floor(phase);
        return result;
    }

private:
    [[nodiscard]] static float interpolate(float left, float right, float amount) noexcept
    {
        return left + (right - left) * amount;
    }

    [[nodiscard]] static float polyBlep(float time, float increment) noexcept
    {
        if (increment <= 0.0f) return 0.0f;
        if (time < increment)
        {
            const auto x = time / increment;
            return x + x - x * x - 1.0f;
        }
        if (time > 1.0f - increment)
        {
            const auto x = (time - 1.0f) / increment;
            return x * x + x + x + 1.0f;
        }
        return 0.0f;
    }

    static constexpr float twoPi = 6.28318530717958647692f;
    double sampleRate = 48000.0;
    float frequency = 110.0f;
    float phase = 0.0f;
    float shape = 0.0f;
    float pulseWidth = 0.5f;
};
}
