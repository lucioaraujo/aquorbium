// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include <cstdint>

namespace aquorbium
{
// Slow, bounded, seeded control signal; kept separate from the existing Brain
// until musical and real-time integration can be evaluated.
class SlewRateRandomWalk final
{
public:
    struct Parameters
    {
        float stepsPerSecond = 0.6f;
        float stepSize = 0.35f;
        float smoothingHz = 0.8f;
    };

    void prepare(double newSampleRate) noexcept;
    void setSeed(std::uint32_t value) noexcept;
    void reset() noexcept;
    void setParameters(Parameters value) noexcept;
    [[nodiscard]] Parameters parameters() const noexcept { return controls; }
    [[nodiscard]] float tick() noexcept;
    [[nodiscard]] float value() const noexcept { return current; }

private:
    [[nodiscard]] float nextRandomSigned() noexcept;
    void updateCoefficient() noexcept;

    Parameters controls {};
    double sampleRate = 48000.0;
    double eventPhase = 0.0;
    float target = 0.0f;
    float current = 0.0f;
    float smoothingGain = 0.0f;
    std::uint32_t seed = 1u;
    std::uint32_t randomState = 1u;
};
}
