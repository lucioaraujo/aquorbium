// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/PopulationCycles.h"

#include <algorithm>
#include <cmath>

namespace aquorbium
{
void PopulationCycles::reset(float prey, float predators) noexcept
{
    logPrey = std::log(std::max(prey, 1.0e-3f));
    logPredators = std::log(std::max(predators, 1.0e-3f));
}

void PopulationCycles::setPeriod(float seconds) noexcept
{
    if (std::isfinite(seconds) && seconds > 1.0f)
        rate = 6.2831853f / seconds;
}

void PopulationCycles::advance(float dt) noexcept
{
    if (!(dt > 0.0f))
        return;
    // with x = ln N, y = ln P: dx/dt = a - b e^y, dy/dt = d e^x - g, all
    // equal to `rate` (equilibrium at N = P = 1). Symplectic Euler: update x
    // with the old y, then y with the new x.
    logPrey += dt * rate * (1.0f - std::exp(logPredators));
    logPredators += dt * rate * (std::exp(logPrey) - 1.0f);
    logPrey = std::clamp(logPrey, -6.0f, 3.0f);
    logPredators = std::clamp(logPredators, -6.0f, 3.0f);
}

void PopulationCycles::capture(float amount) noexcept
{
    const auto a = std::clamp(amount, 0.0f, 1.0f);
    logPrey += std::log(1.0f - 0.06f * a);
    logPredators += std::log(1.0f + 0.04f * a);
}

float PopulationCycles::prey() const noexcept
{
    return std::exp(logPrey);
}

float PopulationCycles::predators() const noexcept
{
    return std::exp(logPredators);
}
}
