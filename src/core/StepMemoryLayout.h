// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include <cstddef>
#include <cstdint>

namespace aquorbium
{
// Maps four logical 8-step tracks onto the 32-address shared step memory.
class StepMemoryLayout final
{
public:
    enum class Mode : std::uint8_t
    {
        parallel4x8,
        serial2x16,
        serial1x32
    };

    struct Address
    {
        std::size_t index = 0;
        std::size_t logicalLength = 8;
        std::size_t group = 0;
    };

    void setMode(Mode value) noexcept;
    [[nodiscard]] Mode mode() const noexcept { return selectedMode; }
    [[nodiscard]] Address map(std::size_t track,
                              std::size_t logicalStep) const noexcept;

private:
    Mode selectedMode = Mode::parallel4x8;
};
}
