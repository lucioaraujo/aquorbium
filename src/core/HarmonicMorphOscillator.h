// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include <array>
#include <cstddef>

namespace aquorbium
{
// Morphs harmonic coefficients before synthesis. This is intentionally not a
// crossfade between two completed oscillator outputs.
class HarmonicMorphOscillator final
{
public:
    static constexpr std::size_t maximumHarmonics = 16;

    HarmonicMorphOscillator() noexcept;

    void prepare(double newSampleRate) noexcept;
    void reset(float phase = 0.0f) noexcept;
    [[nodiscard]] bool setFamilies(
        const std::array<float, maximumHarmonics>& first,
        const std::array<float, maximumHarmonics>& second,
        std::size_t count) noexcept;
    void setFrequency(float value) noexcept;
    void setMorph(float value) noexcept;
    void setLevel(float value) noexcept;
    [[nodiscard]] float tick() noexcept;

    [[nodiscard]] float coefficient(std::size_t harmonic) const noexcept;
    [[nodiscard]] std::size_t harmonicCount() const noexcept { return activeHarmonics; }

private:
    void updateCoefficients() noexcept;

    std::array<float, maximumHarmonics> familyA {};
    std::array<float, maximumHarmonics> familyB {};
    std::array<float, maximumHarmonics> coefficients {};
    std::size_t activeHarmonics = 8;
    double sampleRate = 48000.0;
    double phase = 0.0;
    float frequency = 110.0f;
    float morph = 0.0f;
    float level = 0.5f;
    float normalization = 1.0f;
};
}
