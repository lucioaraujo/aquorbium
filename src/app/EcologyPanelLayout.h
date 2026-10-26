// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include "WindowLayoutPolicy.h"

#include <array>

namespace aquorbium::windowing
{
struct EcologyPanelLayout
{
    Bounds toggle;
    Bounds nameEditor;
    Bounds renameButton;
    Bounds undoButton;
    std::array<Bounds, 6> rows {};
    std::array<Bounds, 4> presetButtons {};
    std::array<Bounds, 2> transferButtons {};
    Bounds presetStatus;
    int labelWidth = 0;

    [[nodiscard]] static constexpr EcologyPanelLayout calculate(Bounds panel) noexcept
    {
        EcologyPanelLayout result;
        if (panel.width < 440 || panel.height < 324)
            return result;
        result.toggle = { panel.x + panel.width - 154, panel.y + 12, 138, 26 };
        const auto innerWidth = panel.width - 40;
        result.nameEditor = { panel.x + 20, panel.y + 50, innerWidth - 192, 24 };
        result.renameButton = { result.nameEditor.x + result.nameEditor.width + 4, panel.y + 50, 92, 24 };
        result.undoButton = { result.renameButton.x + 96, panel.y + 50, 92, 24 };
        const auto controlsTop = panel.y + 80;
        const auto presetTop = panel.y + panel.height - 104;
        const auto rowHeight = std::min(46, (presetTop - controlsTop - 8) / 6);
        result.labelWidth = std::min(168, innerWidth / 3);
        for (int index = 0; index < 6; ++index)
            result.rows[static_cast<std::size_t>(index)] =
                { panel.x + 20, controlsTop + index * rowHeight, innerWidth, rowHeight };
        const auto buttonWidth = (innerWidth - 12) / 4;
        for (int index = 0; index < 4; ++index)
            result.presetButtons[static_cast<std::size_t>(index)] =
                { panel.x + 20 + index * (buttonWidth + 4), presetTop, buttonWidth, 28 };
        const auto transferWidth = (innerWidth - 4) / 2;
        for (int index = 0; index < 2; ++index)
            result.transferButtons[static_cast<std::size_t>(index)] =
                { panel.x + 20 + index * (transferWidth + 4), presetTop + 32, transferWidth, 28 };
        result.presetStatus = { panel.x + 20, presetTop + 64, innerWidth, 22 };
        return result;
    }
};
}
