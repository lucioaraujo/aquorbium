// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace aquorbium
{
// One master clock feeding members at exact rational frequency relations.
class HarmonicPhaseBank final
{
public:
    static constexpr std::size_t maximumMembers = 8;

    struct Ratio
    {
        std::uint8_t numerator = 1;
        std::uint8_t denominator = 1;
    };

    struct Frame
    {
        std::array<float, maximumMembers> phase {};
        std::size_t count = 1;
        float masterPhase = 0.0f;
    };

    HarmonicPhaseBank() noexcept;
    void prepare(double newSampleRate) noexcept;
    void setRateHz(float value) noexcept;
    void setMemberCount(std::size_t value) noexcept;
    void setRatio(std::size_t member, Ratio value) noexcept;
    void reset(float phase = 0.0f) noexcept;
    [[nodiscard]] Frame advance(float rateMultiplier = 1.0f) noexcept;

    [[nodiscard]] std::size_t memberCount() const noexcept { return activeMembers; }
    [[nodiscard]] Ratio ratio(std::size_t member) const noexcept;

private:
    [[nodiscard]] static float wrapped(double value) noexcept;

    std::array<Ratio, maximumMembers> ratios {};
    std::size_t activeMembers = 4;
    double sampleRate = 200.0;
    double masterCycles = 0.0;
    float rateHz = 0.2f;
};
}
