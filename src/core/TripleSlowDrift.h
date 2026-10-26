// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include "core/SlewRateRandomWalk.h"

#include <array>
#include <cstdint>

namespace aquorbium
{
// Three seeded random walks spanning seconds, minutes and tens of minutes.
class TripleSlowDrift final
{
public:
    struct Output
    {
        float fast = 0.0f;
        float middle = 0.0f;
        float slow = 0.0f;
        float combined = 0.0f;
    };

    void prepare(double newSampleRate) noexcept;
    void setSeed(std::uint32_t value) noexcept;
    void setPeriodsSeconds(std::array<float, 3> value) noexcept;
    void setWeights(std::array<float, 3> value) noexcept;
    void reset() noexcept;
    [[nodiscard]] Output tick() noexcept;

    [[nodiscard]] std::array<float, 3> periodsSeconds() const noexcept
    {
        return periods;
    }
    [[nodiscard]] std::array<float, 3> weights() const noexcept
    {
        return mixWeights;
    }

private:
    void configureWalks() noexcept;

    std::array<SlewRateRandomWalk, 3> walks {};
    std::array<float, 3> periods { 5.0f, 60.0f, 1200.0f };
    std::array<float, 3> mixWeights { 0.5f, 0.3f, 0.2f };
    double sampleRate = 200.0;
    std::uint32_t seed = 1u;
};
}
