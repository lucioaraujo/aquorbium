// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/StepMemoryLayout.h"

#include <algorithm>

namespace aquorbium
{
void StepMemoryLayout::setMode(Mode value) noexcept
{
    const auto raw = static_cast<std::uint8_t>(value);
    selectedMode = raw <= static_cast<std::uint8_t>(Mode::serial1x32)
        ? value : Mode::parallel4x8;
}

StepMemoryLayout::Address StepMemoryLayout::map(
    std::size_t track, std::size_t logicalStep) const noexcept
{
    const auto safeTrack = std::min<std::size_t>(track, 3);
    switch (selectedMode)
    {
        case Mode::parallel4x8:
            return { safeTrack * 8 + logicalStep % 8, 8, safeTrack };
        case Mode::serial2x16:
        {
            const auto group = safeTrack / 2;
            return { group * 16 + logicalStep % 16, 16, group };
        }
        case Mode::serial1x32:
            return { logicalStep % 32, 32, 0 };
    }
    return { safeTrack * 8 + logicalStep % 8, 8, safeTrack };
}
}
