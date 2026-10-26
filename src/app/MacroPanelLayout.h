// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include "WindowLayoutPolicy.h"
#include <array>
#include <cstddef>

namespace aquorbium::windowing
{
// A viewport scrolls when the card is short; knob hit areas never shrink.
struct MacroPanelLayout
{
    static constexpr int rowHeight = 112;
    static constexpr int labelHeight = 20;
    static constexpr int valueHeight = 20;
    static constexpr int rotaryPadding = 20; // JUCE V4: 10 px on each side
    static constexpr int minimumRotaryDiameter = 52;
    static constexpr int scrollBarWidth = 12;
    static constexpr int minimumContentWidth = 160;
    Bounds viewport;
    Bounds content;
    int columns = 2;
    std::array<Bounds, 16> labels {};
    std::array<Bounds, 16> sliders {};

    [[nodiscard]] static constexpr MacroPanelLayout calculate(Bounds card,
                                                              std::size_t count) noexcept
    {
        MacroPanelLayout result;
        if (card.width < 176 || card.height < 100
            || (count != 3 && count != 4 && count != 5 && count != 6 && count != 9 && count != 8
                && count != 12 && count != 16))
            return result;
        result.viewport = { card.x + 8, card.y + 68, card.width - 16, card.height - 76 };
        const auto allowsThreeColumns = result.viewport.width >= 252
            || (result.viewport.width >= 240 && result.viewport.height >= rowHeight * 2);
        result.columns = (count == 3 || count == 6) && allowsThreeColumns ? 3 : 2;
        const auto height = static_cast<int>((count + static_cast<std::size_t>(result.columns) - 1)
            / static_cast<std::size_t>(result.columns)) * rowHeight;
        const auto width = std::max(minimumContentWidth,
            result.viewport.width - (height > result.viewport.height ? scrollBarWidth : 0));
        result.content = { 0, 0, width, height };
        for (std::size_t index = 0; index < count; ++index)
        {
            const auto column = static_cast<int>(index % static_cast<std::size_t>(result.columns));
            const auto row = static_cast<int>(index / static_cast<std::size_t>(result.columns));
            const auto x = column * width / result.columns;
            const auto cellWidth = (column + 1) * width / result.columns - x;
            result.labels[index] = { x, row * rowHeight, cellWidth, labelHeight };
            result.sliders[index] = { x, row * rowHeight + labelHeight,
                                      cellWidth, rowHeight - labelHeight };
        }
        return result;
    }

    // The wide surface: a compact one-line card header and four knobs per
    // row, sized so the whole panel is visible (no scroll bar). Any count
    // from 1 to 16; knob hit areas stay as large as in the other surfaces.
    static constexpr int wideHeaderHeight = 40;
    static constexpr int wideColumns = 4;
    [[nodiscard]] static constexpr int wideCardHeight(std::size_t count) noexcept
    {
        const auto rows = static_cast<int>((count + wideColumns - 1) / wideColumns);
        return wideHeaderHeight + rows * rowHeight + 6;
    }
    [[nodiscard]] static constexpr MacroPanelLayout calculateWide(Bounds card,
                                                                  std::size_t count) noexcept
    {
        MacroPanelLayout result;
        if (count == 0 || count > 16 || card.width < wideColumns * 72 + 16
            || card.height < wideCardHeight(count))
            return result;
        result.columns = wideColumns;
        result.viewport = { card.x + 8, card.y + wideHeaderHeight, card.width - 16,
                            card.height - wideHeaderHeight - 4 };
        const auto rows = static_cast<int>((count + wideColumns - 1) / wideColumns);
        const auto width = result.viewport.width;
        result.content = { 0, 0, width, rows * rowHeight };
        for (std::size_t index = 0; index < count; ++index)
        {
            const auto column = static_cast<int>(index % wideColumns);
            const auto row = static_cast<int>(index / wideColumns);
            const auto x = column * width / wideColumns;
            const auto cellWidth = (column + 1) * width / wideColumns - x;
            result.labels[index] = { x, row * rowHeight, cellWidth, labelHeight };
            result.sliders[index] = { x, row * rowHeight + labelHeight,
                                      cellWidth, rowHeight - labelHeight };
        }
        return result;
    }
};
}
