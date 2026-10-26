// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include <cstddef>
#include <cstdint>

namespace aquorbium
{
// Coordinates existing musical layers at shared ecological boundaries. Its
// four-part cycle is structural, not a tempo-synchronised bar/beat clock.
class CompositionalVariationDirector final
{
public:
    // the longest a segment may last, in collective boundaries
    static constexpr std::size_t maximumSegmentLength = 8u;

    enum class Segment : std::uint8_t
    {
        establish,
        variation,
        answer,
        suspension
    };

    struct Context
    {
        std::size_t formState = 3;
        float density = 0.5f;
        float calm = 0.5f;
        float cohesion = 0.5f;
        float danger = 0.5f;
    };

    struct Plan
    {
        Segment segment = Segment::establish;
        std::uint64_t number = 0;
        float influence = 0.0f;
        float rhythmicDensity = 0.82f;
        float rhythmRotation = 0.0f;
        float memoryWeight = 0.78f;
        float contourRotation = 0.0f;
        float harmonicPhaseOffset = 0.0f;
        float pulseWeight = 0.76f;
        float tonalWeight = 1.0f;
        float textureWeight = 0.54f;
        float movementWeight = 0.58f;
        float spaceWeight = 0.62f;
        float accentWeight = 0.75f;
        bool retrogradeContour = false;
        std::uint8_t eventsInSegment = 0;
        std::uint8_t segmentLength = 1;
    };

    void reset() noexcept;
    [[nodiscard]] Plan advance(Context context) noexcept;
    [[nodiscard]] Plan plan() const noexcept { return current; }

private:
    std::uint64_t segmentCount = 0;
    std::uint8_t eventsInSegment = 0;
    bool hasCurrentSegment = false;
    Plan current {};
};
}
