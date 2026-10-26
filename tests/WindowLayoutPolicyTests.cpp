// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "app/WindowLayoutPolicy.h"
#include "app/EcologyPanelLayout.h"
#include "app/MacroPanelLayout.h"
#include "app/OrganismLabelLayout.h"

#include <cstdlib>
#include <iostream>

#define REQUIRE(condition) do { if (!(condition)) { \
    std::cerr << "requirement failed at line " << __LINE__ << ": " #condition "\n"; \
    return EXIT_FAILURE; } } while (false)

int main()
{
    using aquorbium::windowing::Bounds;
    using aquorbium::windowing::SurfaceMode;
    using aquorbium::windowing::WindowLayoutPolicy;

    REQUIRE(WindowLayoutPolicy::classifySurface(1920, 1080) == SurfaceMode::wide);
    REQUIRE(WindowLayoutPolicy::classifySurface(1800, 960) == SurfaceMode::wide);
    REQUIRE(WindowLayoutPolicy::classifySurface(1799, 1080) == SurfaceMode::full);
    REQUIRE(WindowLayoutPolicy::classifySurface(1920, 959) == SurfaceMode::full);
    REQUIRE(WindowLayoutPolicy::classifySurface(1200, 720) == SurfaceMode::full);
    REQUIRE(WindowLayoutPolicy::classifySurface(1024, 650) == SurfaceMode::compact);
    REQUIRE(WindowLayoutPolicy::classifySurface(1023, 650) == SurfaceMode::emergency);
    REQUIRE(WindowLayoutPolicy::classifySurface(1024, 649) == SurfaceMode::emergency);

    // a large screen opens on the whole work area (every control visible)
    REQUIRE((WindowLayoutPolicy::initialBounds({ 0, 0, 1920, 1040 })
             == Bounds { 0, 0, 1920, 1040 }));
    REQUIRE((WindowLayoutPolicy::initialBounds({ 0, 0, 1600, 900 })
             == Bounds { 160, 54, 1280, 792 }));
    REQUIRE((WindowLayoutPolicy::initialBounds({ 0, 0, 1366, 728 })
             == Bounds { 82, 39, 1202, 650 }));
    REQUIRE((WindowLayoutPolicy::initialBounds({ 1920, 0, 1024, 728 })
             == Bounds { 1952, 39, 960, 650 }));
    REQUIRE((WindowLayoutPolicy::initialBounds({ 0, 0, 800, 500 })
             == Bounds { 0, 0, 800, 500 }));

    REQUIRE((WindowLayoutPolicy::restoreBounds({ 2300, -200, 1280, 800 },
                                                { 0, 0, 1920, 1040 })
             == Bounds { 640, 0, 1280, 800 }));
    REQUIRE((WindowLayoutPolicy::restoreBounds({ 2000, 30, 700, 400 },
                                                { 1920, 0, 1024, 728 })
             == Bounds { 1984, 30, 960, 600 }));
    REQUIRE(WindowLayoutPolicy::restoreBounds({}, { 0, 0, 1920, 1040 })
            == WindowLayoutPolicy::initialBounds({ 0, 0, 1920, 1040 }));
    REQUIRE((WindowLayoutPolicy::restoreBounds({ 0, 0, 3000, 2000 },
                                                { -1280, 40, 1280, 720 })
             == Bounds { -1280, 40, 1280, 720 }));
    REQUIRE(WindowLayoutPolicy::isRecoverable({ 1760, 20, 800, 600 },
                                               { 0, 0, 1920, 1040 }));
    REQUIRE(!WindowLayoutPolicy::isRecoverable({ 1880, 1000, 800, 600 },
                                                { 0, 0, 1920, 1040 }));
    REQUIRE(!WindowLayoutPolicy::isRecoverable({ 2400, 0, 800, 600 },
                                                { 0, 0, 1920, 1040 }));

    const auto contained = [](Bounds parent, Bounds child) {
        return child.isValid() && child.x >= parent.x && child.y >= parent.y
            && child.x + child.width <= parent.x + parent.width
            && child.y + child.height <= parent.y + parent.height;
    };
    for (const auto panel : { Bounds { 280, 88, 720, 472 },
                              Bounds { 232, 66, 576, 428 },
                              Bounds { 208, 58, 544, 348 },
                              Bounds { 0, 0, 440, 324 } })
    {
        const auto layout = aquorbium::windowing::EcologyPanelLayout::calculate(panel);
        REQUIRE(contained(panel, layout.toggle));
        REQUIRE(contained(panel, layout.presetStatus));
        REQUIRE(contained(panel, layout.nameEditor));
        REQUIRE(contained(panel, layout.renameButton));
        REQUIRE(contained(panel, layout.undoButton));
        REQUIRE(layout.nameEditor.width >= 208);
        REQUIRE(layout.toggle.y + layout.toggle.height <= layout.nameEditor.y);
        REQUIRE(layout.nameEditor.x + layout.nameEditor.width <= layout.renameButton.x);
        REQUIRE(layout.renameButton.x + layout.renameButton.width <= layout.undoButton.x);
        REQUIRE(layout.undoButton.y + layout.undoButton.height <= layout.rows[0].y);
        for (std::size_t index = 0; index < layout.rows.size(); ++index)
        {
            const auto row = layout.rows[index];
            REQUIRE(contained(panel, row));
            REQUIRE(row.height >= 22);
            REQUIRE(layout.labelWidth > 0 && row.width - layout.labelWidth >= 90);
            REQUIRE(row.y + row.height <= layout.presetButtons[0].y);
            if (index > 0)
                REQUIRE(layout.rows[index - 1].y + layout.rows[index - 1].height <= row.y);
        }
        for (std::size_t index = 0; index < layout.presetButtons.size(); ++index)
        {
            const auto button = layout.presetButtons[index];
            REQUIRE(contained(panel, button));
            REQUIRE(button.width >= 90);
            REQUIRE(button.y + button.height <= layout.transferButtons[0].y);
            if (index > 0)
                REQUIRE(layout.presetButtons[index - 1].x + layout.presetButtons[index - 1].width
                        <= button.x);
        }
        for (const auto button : layout.transferButtons)
        {
            REQUIRE(contained(panel, button));
            REQUIRE(button.width >= 90);
            REQUIRE(button.y + button.height <= layout.presetStatus.y);
        }
        REQUIRE(layout.transferButtons[0].x + layout.transferButtons[0].width
                <= layout.transferButtons[1].x);
    }
    REQUIRE(!aquorbium::windowing::EcologyPanelLayout::calculate({ 0, 0, 100, 100 }).toggle.isValid());
    REQUIRE(!aquorbium::windowing::EcologyPanelLayout::calculate({ 0, 0, 439, 324 }).toggle.isValid());
    REQUIRE(!aquorbium::windowing::EcologyPanelLayout::calculate({ 0, 0, 440, 323 }).toggle.isValid());

    // Full, compact/emergency and screenshot-sized cards. A short viewport
    // scrolls, rather than shrinking the knob/label/value geometry.
    for (const auto card : { Bounds { 1654, 120, 242, 216 },
                             Bounds { 0, 0, 320, 260 },
                             Bounds { 0, 0, 260, 140 },
                             Bounds { 0, 0, 176, 108 } })
    for (const std::size_t count : { 3u, 4u, 6u, 8u, 12u, 16u })
    {
        const auto layout = aquorbium::windowing::MacroPanelLayout::calculate(card, count);
        REQUIRE(contained(card, layout.viewport));
        REQUIRE(layout.viewport.y >= card.y + 68);
        for (std::size_t index = 0; index < count; ++index)
        {
            REQUIRE(contained(layout.content, layout.labels[index]));
            REQUIRE(contained(layout.content, layout.sliders[index]));
            REQUIRE(layout.sliders[index].width >= 80);
            REQUIRE(layout.sliders[index].height == 92);
            REQUIRE(std::min(layout.sliders[index].width, layout.sliders[index].height
                - aquorbium::windowing::MacroPanelLayout::valueHeight)
                - aquorbium::windowing::MacroPanelLayout::rotaryPadding
                >= aquorbium::windowing::MacroPanelLayout::minimumRotaryDiameter);
            REQUIRE(layout.labels[index].y + layout.labels[index].height <= layout.sliders[index].y);
            const auto columns = static_cast<std::size_t>(layout.columns);
            if (index >= columns)
                REQUIRE(layout.sliders[index - columns].y + layout.sliders[index - columns].height
                        <= layout.labels[index].y);
        }
    }
    REQUIRE(!aquorbium::windowing::MacroPanelLayout::calculate({ 0, 0, 176, 99 }, 4).viewport.isValid());

    // The wide surface: every knob visible, no scroll bar, the same hit areas.
    using aquorbium::windowing::MacroPanelLayout;
    for (const std::size_t count : { 4u, 6u, 12u, 16u })
    {
        const Bounds card { 16, 64, 336, MacroPanelLayout::wideCardHeight(count) };
        const auto layout = MacroPanelLayout::calculateWide(card, count);
        REQUIRE(contained(card, layout.viewport));
        REQUIRE(layout.content.height <= layout.viewport.height);
        REQUIRE(layout.content.width <= layout.viewport.width);
        for (std::size_t index = 0; index < count; ++index)
        {
            REQUIRE(contained(layout.content, layout.labels[index]));
            REQUIRE(contained(layout.content, layout.sliders[index]));
            REQUIRE(layout.sliders[index].width >= 80);
            REQUIRE(layout.sliders[index].height == 92);
        }
    }
    REQUIRE(!MacroPanelLayout::calculateWide({ 0, 0, 300, 400 }, 4).viewport.isValid());
    REQUIRE(!MacroPanelLayout::calculateWide({ 0, 0, 336, 100 }, 4).viewport.isValid());

    // Coincident organisms and bodies at opposite edges: labels remain
    // readable without changing artistic/ecological positions.
    for (const auto canvas : { Bounds { 20, 68, 504, 206 }, Bounds { 20, 68, 1160, 550 } })
    for (const auto clustered : { true, false })
    {
        std::array<Bounds, 5> bodies;
        for (std::size_t index = 0; index < bodies.size(); ++index)
            bodies[index] = clustered
                ? Bounds { canvas.x + canvas.width / 2 - 36, canvas.y + canvas.height / 2 - 36, 72, 72 }
                : Bounds { canvas.x + static_cast<int>(index) * (canvas.width - 72) / 4,
                           canvas.y + (index % 2 == 0 ? 0 : canvas.height - 72), 72, 72 };
        const Bounds lens { canvas.x + canvas.width * 3 / 4, canvas.y + canvas.height / 2 - 20, 40, 40 };
        const auto labels = aquorbium::windowing::OrganismLabelLayout::calculate(canvas, bodies, lens);
        for (std::size_t index = 0; index < labels.size(); ++index)
        {
            REQUIRE(contained(canvas, labels[index]));
            REQUIRE(!aquorbium::windowing::OrganismLabelLayout::overlaps(labels[index], lens));
            for (const auto body : bodies)
                REQUIRE(!aquorbium::windowing::OrganismLabelLayout::overlaps(labels[index], body));
            for (std::size_t previous = 0; previous < index; ++previous)
                REQUIRE(!aquorbium::windowing::OrganismLabelLayout::overlaps(labels[index], labels[previous]));
        }
    }

    return EXIT_SUCCESS;
}
