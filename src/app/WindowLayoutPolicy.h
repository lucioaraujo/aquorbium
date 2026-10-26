// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include <algorithm>

namespace aquorbium::windowing
{
struct Bounds
{
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;

    [[nodiscard]] constexpr bool isValid() const noexcept
    {
        return width > 0 && height > 0;
    }

    [[nodiscard]] friend constexpr bool operator==(Bounds, Bounds) noexcept = default;
};

enum class SurfaceMode
{
    emergency,
    compact,
    full,
    // 2026-10-20, the author: "prefiro quando os controles são visíveis".
    // From ~1800x960 every knob of every card fits without a scroll bar.
    wide
};

struct WindowLayoutPolicy
{
    static constexpr int preferredWidth = 1280;
    static constexpr int preferredHeight = 800;
    static constexpr int emergencyWidth = 960;
    static constexpr int emergencyHeight = 600;
    static constexpr int compactWidth = 1024;
    static constexpr int compactHeight = 650;
    static constexpr int fullWidth = 1200;
    static constexpr int fullHeight = 720;
    static constexpr int initialCoveragePercent = 88;
    static constexpr int wideWidth = 1800;
    static constexpr int wideHeight = 960;

    [[nodiscard]] static constexpr SurfaceMode classifySurface(int width,
                                                                int height) noexcept
    {
        if (width >= wideWidth && height >= wideHeight)
            return SurfaceMode::wide;
        if (width >= fullWidth && height >= fullHeight)
            return SurfaceMode::full;
        if (width >= compactWidth && height >= compactHeight)
            return SurfaceMode::compact;
        return SurfaceMode::emergency;
    }

    [[nodiscard]] static constexpr Bounds initialBounds(Bounds userArea) noexcept
    {
        if (!userArea.isValid())
            return { 0, 0, preferredWidth, preferredHeight };
        // a screen that holds the wide surface: open on the whole work area,
        // so that every control is visible from the first moment
        if (userArea.width >= wideWidth && userArea.height >= wideHeight + 40)
            return userArea;

        const auto coveredWidth = userArea.width * initialCoveragePercent / 100;
        const auto coveredHeight = userArea.height * initialCoveragePercent / 100;
        const auto maximumCoveredWidth = userArea.width * 90 / 100;
        const auto maximumCoveredHeight = userArea.height * 90 / 100;
        const auto minimumWidth = maximumCoveredWidth >= compactWidth
            ? compactWidth : std::min(emergencyWidth, userArea.width);
        const auto minimumHeight = maximumCoveredHeight >= compactHeight
            ? compactHeight : std::min(emergencyHeight, userArea.height);
        const auto width = std::max(minimumWidth,
                                    std::min(preferredWidth, coveredWidth));
        const auto height = std::max(minimumHeight,
                                     std::min(preferredHeight, coveredHeight));
        return centred(userArea, width, height);
    }

    [[nodiscard]] static constexpr Bounds restoreBounds(Bounds saved,
                                                         Bounds userArea) noexcept
    {
        if (!userArea.isValid())
            return saved.isValid() ? saved : initialBounds(userArea);
        if (!saved.isValid())
            return initialBounds(userArea);

        const auto minimumWidth = std::min(emergencyWidth, userArea.width);
        const auto minimumHeight = std::min(emergencyHeight, userArea.height);
        const auto width = std::clamp(saved.width, minimumWidth, userArea.width);
        const auto height = std::clamp(saved.height, minimumHeight, userArea.height);
        const auto maximumX = userArea.x + userArea.width - width;
        const auto maximumY = userArea.y + userArea.height - height;
        return {
            std::clamp(saved.x, userArea.x, maximumX),
            std::clamp(saved.y, userArea.y, maximumY),
            width,
            height
        };
    }

    [[nodiscard]] static constexpr bool isRecoverable(Bounds window,
                                                       Bounds userArea) noexcept
    {
        if (!window.isValid() || !userArea.isValid())
            return false;

        const auto left = std::max(window.x, userArea.x);
        const auto top = std::max(window.y, userArea.y);
        const auto right = std::min(window.x + window.width,
                                    userArea.x + userArea.width);
        const auto bottom = std::min(window.y + window.height,
                                     userArea.y + userArea.height);
        const auto visibleWidth = std::max(0, right - left);
        const auto visibleHeight = std::max(0, bottom - top);
        return visibleWidth >= std::min(160, window.width)
            && visibleHeight >= std::min(64, window.height);
    }

private:
    [[nodiscard]] static constexpr Bounds centred(Bounds area, int width,
                                                   int height) noexcept
    {
        return {
            area.x + (area.width - width) / 2,
            area.y + (area.height - height) / 2,
            width,
            height
        };
    }
};
}
