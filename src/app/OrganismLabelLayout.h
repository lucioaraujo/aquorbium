// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include "WindowLayoutPolicy.h"
#include <array>
#include <cstddef>

namespace aquorbium::windowing
{
struct OrganismLabelLayout
{
    static constexpr std::size_t count = 5;
    static constexpr int width = 136;
    static constexpr int height = 20;

    [[nodiscard]] static constexpr bool overlaps(Bounds a, Bounds b) noexcept
    {
        return a.x < b.x + b.width && a.x + a.width > b.x
            && a.y < b.y + b.height && a.y + a.height > b.y;
    }

    [[nodiscard]] static constexpr std::array<Bounds, count> calculate(
        Bounds canvas, const std::array<Bounds, count>& bodies, Bounds lens = {}) noexcept
    {
        std::array<Bounds, count> labels {};
        if (canvas.width < width || canvas.height < height) return labels;
        for (std::size_t index = 0; index < count; ++index)
        {
            const auto tryPosition = [&](int x, int y) {
                const Bounds candidate {
                    std::clamp(x, canvas.x, canvas.x + canvas.width - width),
                    std::clamp(y, canvas.y, canvas.y + canvas.height - height), width, height };
                if (lens.isValid() && overlaps(candidate, lens)) return false;
                for (const auto body : bodies)
                    if (overlaps(candidate, body)) return false;
                for (std::size_t previous = 0; previous < index; ++previous)
                    if (overlaps(candidate, labels[previous])) return false;
                labels[index] = candidate;
                return true;
            };
            // Prefer labels next to their body. Bounded search, never shrink
            // type or paint over an already placed label/organism halo.
            for (int attempt = 0; attempt < 48 && !labels[index].isValid(); ++attempt)
            {
                const auto step = attempt / 2;
                const auto offset = ((step + 1) / 2) * 24 * (step % 2 == 0 ? 1 : -1);
                const auto x = attempt % 2 == 0 ? bodies[index].x + bodies[index].width + 4
                                              : bodies[index].x - width - 4;
                static_cast<void>(tryPosition(x, bodies[index].y + bodies[index].height / 2 - height / 2 + offset));
            }
            // If neighbours fill the preferred area, use a small finite grid
            // in the same canvas. If even that is full, omit rather than overlap.
            for (int row = 0; row < 32 && !labels[index].isValid(); ++row)
                for (int column = 0; column < 4 && !labels[index].isValid(); ++column)
                    static_cast<void>(tryPosition(canvas.x + column * (canvas.width - width) / 3,
                                                  canvas.y + row * 24));
        }
        return labels;
    }
};
}
