// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace aquorbium
{
// Converts a continuous normalized signal into discrete step transitions.
// Thresholds are upper boundaries; crossing one selects another address.
class ThresholdStepSelector final
{
public:
    static constexpr std::size_t maximumSteps = 32;

    struct Output
    {
        std::size_t index = 0;
        bool changed = false;
        int direction = 0;
        std::uint64_t transitionCount = 0;
    };

    ThresholdStepSelector() noexcept;

    void setLength(std::size_t value) noexcept;
    [[nodiscard]] bool setThresholds(
        const std::array<float, maximumSteps>& values,
        std::size_t count) noexcept;
    void reset() noexcept;
    [[nodiscard]] Output select(float normalizedSignal) noexcept;

    [[nodiscard]] std::size_t length() const noexcept { return activeLength; }
    [[nodiscard]] float threshold(std::size_t index) const noexcept;

private:
    void makeEvenThresholds() noexcept;

    std::array<float, maximumSteps> thresholds {};
    std::size_t activeLength = 8;
    std::size_t previousIndex = maximumSteps;
    std::uint64_t transitions = 0;
};
}
