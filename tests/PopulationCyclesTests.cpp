// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

// Lotka-Volterra cycles (2026-10-20): the populations stay positive and
// bounded, cycle with about the set period, the predators' peak follows the
// prey's by about a quarter cycle, the orbit does not drift, and a capture
// takes prey and feeds the predators.
#include "core/PopulationCycles.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <vector>

namespace
{
using aquorbium::PopulationCycles;
int failures = 0;
void check(bool condition, const char* what)
{
    if (!condition)
    {
        std::cerr << "FAILED: " << what << '\n';
        ++failures;
    }
}
}

int main()
{
    PopulationCycles lv;
    lv.reset(1.2f, 0.9f);  // a small orbit, so the period is close to 2 pi / rate
    lv.setPeriod(120.0f);
    const float dt = 0.01f;
    std::vector<float> preyPeaks, predatorPeaks;
    float pN = lv.prey(), ppN = pN, pP = lv.predators(), ppP = pP;
    float minimum = 1.0e9f, maximum = 0.0f;
    float firstPrey = lv.prey(), firstPredators = lv.predators();
    for (int k = 1; k <= 60000; ++k)  // 600 s
    {
        lv.advance(dt);
        const auto n = lv.prey(), p = lv.predators();
        minimum = std::min({ minimum, n, p });
        maximum = std::max({ maximum, n, p });
        // a peak, at least 10 s after the previous one (a flat top repeats)
        const auto t = static_cast<float>(k - 1) * dt;
        if (pN > ppN && pN >= n && (preyPeaks.empty() || t - preyPeaks.back() > 10.0f))
            preyPeaks.push_back(t);
        if (pP > ppP && pP >= p && (predatorPeaks.empty() || t - predatorPeaks.back() > 10.0f))
            predatorPeaks.push_back(t);
        ppN = pN; pN = n; ppP = pP; pP = p;
    }
    check(minimum > 0.0f && maximum < 10.0f, "the populations stay positive and bounded");
    check(preyPeaks.size() >= 4, "the prey cycle several times in ten minutes");
    if (preyPeaks.size() >= 3)
    {
        const auto period = (preyPeaks.back() - preyPeaks.front()) / static_cast<float>(preyPeaks.size() - 1);
        check(std::abs(period - 120.0f) < 12.0f, "the cycle lasts about the set period");
        // the predators peak a quarter cycle (or so) after the prey
        float lag = 1.0e9f;
        for (const auto t : predatorPeaks)
            if (t > preyPeaks[1])
            {
                lag = t - preyPeaks[1];
                break;
            }
        check(lag > 0.1f * period && lag < 0.45f * period, "the predators' peak follows the prey's");
    }
    // the orbit does not drift: back near the start after whole cycles
    PopulationCycles again;
    again.reset(firstPrey, firstPredators);
    again.setPeriod(120.0f);
    float maxPrey = 0.0f, lastMax = 0.0f;
    for (int cycle = 0; cycle < 10; ++cycle)
    {
        maxPrey = 0.0f;
        for (int k = 0; k < 12000; ++k)
        {
            again.advance(dt);
            maxPrey = std::max(maxPrey, again.prey());
        }
        if (cycle == 0)
            lastMax = maxPrey;
    }
    check(std::abs(maxPrey - lastMax) < 0.05f * lastMax, "the cycles keep their size (no numerical drift)");
    // a capture
    PopulationCycles c;
    c.reset(1.0f, 1.0f);
    const auto n0 = c.prey(), p0 = c.predators();
    c.capture();
    check(c.prey() < n0 && c.predators() > p0, "a capture takes prey and feeds the predators");
    if (failures == 0)
        std::cout << "population cycles: all checks passed\n";
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
