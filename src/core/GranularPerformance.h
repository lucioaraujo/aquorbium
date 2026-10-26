// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include "core/BiomaGranularStriker.h"
#include "core/Numeric.h"

namespace aquorbium
{
// Musical adapter for the granular body's lower-level parameters. The mapping
// is deliberately small: performance controls express intent while the DSP
// engine retains explicit physical parameters for tests and generative use.
struct GranularPerformance final
{
    struct Macros
    {
        float influence = 0.0f;
        float matter = 0.5f;
        float population = 0.5f;
        float scatter = 0.5f;
    };

    [[nodiscard]] static BiomaGranularStriker::PerformanceIntent map(
        Macros value) noexcept
    {
        value.influence = numeric::unit(value.influence, 0.0f);
        value.matter = numeric::unit(value.matter, 0.5f);
        value.population = numeric::unit(value.population, 0.5f);
        value.scatter = numeric::unit(value.scatter, 0.5f);

        // Smoothstep gives more resolution around the musically ambiguous
        // dust/metal transition without changing the exact endpoints.
        const auto shapedMatter = value.matter * value.matter
            * (3.0f - 2.0f * value.matter);

        BiomaGranularStriker::PerformanceIntent result;
        result.influence = value.influence;
        result.grainDurationMs = 3.0f + shapedMatter * 29.0f;
        result.resonance = 0.15f + shapedMatter * 0.85f;
        result.particleDensity = value.population;
        result.spectralSpread = value.scatter;
        result.irregularity = 0.10f + value.scatter * 0.90f;
        return result;
    }
};
}
