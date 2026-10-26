// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include <cstddef>

namespace aquorbium
{
// One bounded self-similar generator that can run at control or audio rate.
// The same phase model is used in both domains; only the clock and frequency
// change.
class FractalCore final
{
public:
    static constexpr std::size_t maximumIterations = 8;

    struct Parameters
    {
        float frequencyHz = 0.25f;
        float replicationRatio = 2.0f;
        float roughness = 0.55f;
        std::size_t iterations = 4;
    };

    void prepare(double newSampleRate) noexcept;
    void reset(float phase = 0.0f) noexcept;
    void setParameters(Parameters value) noexcept;
    [[nodiscard]] Parameters parameters() const noexcept { return controls; }
    [[nodiscard]] float tick() noexcept;
    [[nodiscard]] float currentPhase() const noexcept;

private:
    Parameters controls {};
    double sampleRate = 48000.0;
    double phase = 0.0;
};
}
