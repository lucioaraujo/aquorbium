// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include <array>
#include <cstddef>

namespace aquorbium
{
// Bounded control-rate memory between continuously renewed input and a fixed
// repeating loop. At intermediate values, the loop ages toward the live input.
class RandomToLoop final
{
public:
    static constexpr std::size_t maximumLength = 64;

    struct Output
    {
        float value = 0.0f;
        std::size_t index = 0;
        bool loopReady = false;
        bool wrapped = false;
    };

    void setLength(std::size_t value) noexcept;
    void reset() noexcept;
    [[nodiscard]] Output process(float liveValue, float loopAmount) noexcept;

    [[nodiscard]] std::size_t length() const noexcept { return activeLength; }
    [[nodiscard]] std::size_t captured() const noexcept { return capturedCount; }

private:
    std::array<float, maximumLength> memory {};
    std::size_t activeLength = 16;
    std::size_t cursor = 0;
    std::size_t capturedCount = 0;
};
}
