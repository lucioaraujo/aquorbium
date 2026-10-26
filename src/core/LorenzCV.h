// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include <cstdint>

namespace aquorbium
{
// Control-rate Lorenz attractor. An isolated alternative to BiomaBrain drift;
// it is not connected to the live ecology or audio callback.
class LorenzCV final
{
public:
    struct Parameters
    {
        float sigma = 10.0f;
        float rho = 28.0f;
        float beta = 2.6666667f;
        float dT = 0.005f;
    };

    struct Output
    {
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
    };

    void prepare(double newSampleRate) noexcept;
    void setSeed(std::uint32_t value) noexcept;
    void setParameters(Parameters value) noexcept;
    void reset() noexcept;
    [[nodiscard]] Output tick() noexcept;
    [[nodiscard]] Parameters parameters() const noexcept { return controls; }
    [[nodiscard]] std::uint64_t controlSteps() const noexcept { return steps; }
    [[nodiscard]] std::uint64_t recoveries() const noexcept { return recoveryCount; }

private:
    struct State
    {
        double x = 0.1;
        double y = 0.0;
        double z = 0.0;
    };

    [[nodiscard]] State derivative(State value) const noexcept;
    void step() noexcept;
    void resetState() noexcept;

    Parameters controls {};
    State state {};
    Output current {};
    Output target {};
    double sampleRate = 48000.0;
    double controlPhase = 0.0;
    float slewGain = 0.0f;
    std::uint32_t seed = 1u;
    std::uint64_t steps = 0;
    std::uint64_t recoveryCount = 0;
};
}
