// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/CompositionalVariationDirector.h"

#include "core/Numeric.h"

#include <algorithm>
#include <array>

namespace aquorbium
{
void CompositionalVariationDirector::reset() noexcept
{
    segmentCount = 0;
    eventsInSegment = 0;
    hasCurrentSegment = false;
    current = {};
}

CompositionalVariationDirector::Plan
CompositionalVariationDirector::advance(Context context) noexcept
{
    // Each segment has a protagonist (2026-10-20, after the first A/B render:
    // the four segments differed by only ~3 dB and a few hundred hertz, and
    // the whole cycle lasted ~20 s). The bank's family weights use their full
    // 0.25-1.25 range so that the ear can tell the parts apart:
    //   A  - establish: the melodic bodies lead, pulse and texture held back;
    //   A' - vary: the PULSE leads (percussion/clocks), denser and accented;
    //   B  - answer: TEXTURE and MOVEMENT lead (granular, fractal, fluid),
    //        retrograde reading, another harmonic phase, little pulse;
    //   suspension: almost no pulse, sparse, the space opens.
    static constexpr Plan templates[] {
        //            segment  num  infl  rDens  rRot  memW  contR  harm
        //            pulse  tonal  textr  movem  space  accent  retro
        { Segment::establish, 0, 0.0f, 0.86f, 0.00f, 0.84f, 0.00f, 0.000f,
          0.72f, 1.18f, 0.42f, 0.48f, 0.66f, 0.74f, false },
        { Segment::variation, 0, 0.0f, 1.36f, 1.00f, 1.20f, 0.125f, 0.035f,
          1.25f, 0.92f, 0.36f, 0.72f, 0.60f, 1.32f, false },
        { Segment::answer, 0, 0.0f, 0.92f, -1.00f, 1.26f, 0.25f, -0.045f,
          0.52f, 0.78f, 1.25f, 1.22f, 0.92f, 0.96f, true },
        { Segment::suspension, 0, 0.0f, 0.44f, 0.50f, 0.30f, 0.50f, 0.000f,
          0.28f, 0.62f, 0.92f, 0.46f, 1.25f, 0.34f, false }
    };

    auto enterSegment = !hasCurrentSegment;
    if (hasCurrentSegment)
    {
        if (static_cast<std::uint8_t>(eventsInSegment + 1u) < current.segmentLength)
        {
            ++eventsInSegment;
            current.eventsInSegment = eventsInSegment;
            return current;
        }

        ++segmentCount;
        eventsInSegment = 0;
        enterSegment = true;
    }

    if (enterSegment)
    {
        const auto index = static_cast<std::size_t>(segmentCount % 4u);
        current = templates[index];
        current.number = segmentCount;
        current.eventsInSegment = 0;
        hasCurrentSegment = true;
        // in collective boundaries (~4 s each in the audition render): the
        // cycle breathes in about a minute and a quarter instead of ~20 s
        static constexpr std::array<std::uint8_t, 4> baseSegmentLengths {
            6u, 4u, 6u, 3u
        };
        const auto contextualHold = (context.formState + segmentCount) % 3u == 0u
            ? 1u : 0u;
        const auto calm = numeric::unit(context.calm, 0.5f);
        const auto danger = numeric::unit(context.danger, 0.5f);
        const auto tensionHold = calm < 0.25f && danger > 0.70f ? 1u : 0u;
        current.segmentLength = static_cast<std::uint8_t>(std::min<std::size_t>(
            maximumSegmentLength, baseSegmentLengths[index] + contextualHold + tensionHold));
    }

    const auto density = numeric::unit(context.density, 0.5f);
    const auto calm = numeric::unit(context.calm, 0.5f);
    const auto cohesion = numeric::unit(context.cohesion, 0.5f);
    const auto danger = numeric::unit(context.danger, 0.5f);
    const auto formVariation = static_cast<float>(
        static_cast<int>(context.formState % 3u) - 1) * 0.025f;

    current.rhythmicDensity = std::clamp(current.rhythmicDensity
        * (0.86f + density * 0.20f + danger * 0.06f), 0.35f, 1.45f);
    current.memoryWeight = std::clamp(current.memoryWeight
        * (0.92f + cohesion * 0.12f + calm * 0.04f), 0.20f, 1.35f);
    current.contourRotation = std::clamp(
        current.contourRotation + formVariation, 0.0f, 0.95f);
    current.accentWeight = std::clamp(current.accentWeight, 0.25f, 1.40f);
    current.harmonicPhaseOffset = std::clamp(
        current.harmonicPhaseOffset + formVariation * 0.35f,
        -0.06f, 0.06f);
    current.pulseWeight = std::clamp(current.pulseWeight
        * (0.88f + density * 0.16f + danger * 0.08f), 0.25f, 1.25f);
    current.tonalWeight = std::clamp(current.tonalWeight
        * (0.90f + cohesion * 0.16f + calm * 0.05f), 0.55f, 1.25f);
    current.textureWeight = std::clamp(current.textureWeight
        * (0.88f + (1.0f - cohesion) * 0.18f + danger * 0.06f),
        0.30f, 1.25f);
    current.movementWeight = std::clamp(current.movementWeight
        * (0.88f + (1.0f - calm) * 0.12f + density * 0.08f),
        0.30f, 1.25f);
    current.spaceWeight = std::clamp(current.spaceWeight
        * (0.90f + calm * 0.10f + (1.0f - density) * 0.08f),
        0.35f, 1.25f);
    return current;
}
}
