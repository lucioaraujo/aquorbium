// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace aquorbium
{
// Fixed structural mutation: rotates a binary pattern after a configurable
// number of completed bars while preserving its number of active pulses.
class PatternRotationMutator final
{
public:
    static constexpr std::size_t maximumSteps = 32;

    struct Output
    {
        bool mutated = false;
        std::size_t barsUntilMutation = 0;
        std::uint64_t bar = 0;
        std::uint64_t mutationCount = 0;
    };

    PatternRotationMutator() noexcept;

    [[nodiscard]] bool setPattern(
        const std::array<std::uint8_t, maximumSteps>& values,
        std::size_t count) noexcept;
    void setSchedule(std::size_t barsPerMutation, int rotationSteps) noexcept;
    void resetSchedule() noexcept;
    [[nodiscard]] Output advanceBar() noexcept;

    [[nodiscard]] bool at(std::size_t index) const noexcept;
    [[nodiscard]] std::size_t length() const noexcept { return activeLength; }
    [[nodiscard]] std::size_t pulses() const noexcept;

private:
    std::array<std::uint8_t, maximumSteps> pattern {};
    std::size_t activeLength = 8;
    std::size_t intervalBars = 4;
    std::size_t barsElapsed = 0;
    int rotation = 1;
    std::uint64_t barCounter = 0;
    std::uint64_t mutations = 0;
};
}
