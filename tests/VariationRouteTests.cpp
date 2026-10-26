// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

// The compositional variation director in the whole engine (2026-10-22;
// COMPOSITIONAL_VARIATION_DESIGN.md): its bypass is exact (STRUCTURE MIX 0
// leaves the previous route, whatever its other controls say), it really
// acts when on, and the output does not depend on the audio block size
// (64, 256, irregular 7/1000: bit for bit). The director's own determinism,
// order and limits are in CompositionalVariationDirectorTests.
#include "core/BiomaEngine.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <vector>

namespace
{
int failures = 0;
void check(bool condition, const char* what)
{
    if (!condition)
    {
        std::cerr << "FAILED: " << what << '\n';
        ++failures;
    }
}

std::vector<float> render(std::size_t blockA, std::size_t blockB, float influence, float memory, float harmony)
{
    auto engine = std::make_unique<aquorbium::BiomaEngine>();
    engine->prepare(48000.0);
    engine->setSeed(77u);
    engine->setEcosystemBlendImmediately(1.0f);
    engine->setAutonomousEnabled(true);
    engine->setStructureMemoryPerformance({ influence, memory, harmony, 0.3f });
    std::vector<float> out, left(1024), right(1024);
    const std::size_t total = 48000 * 12;
    std::size_t k = 0;
    while (out.size() < 2 * total)
    {
        const auto block = (k++ % 2) ? blockA : blockB;
        engine->render(left.data(), right.data(), block);
        for (std::size_t i = 0; i < block; ++i)
        {
            out.push_back(left[i]);
            out.push_back(right[i]);
        }
    }
    out.resize(2 * total);
    return out;
}

double maximumDifference(const std::vector<float>& a, const std::vector<float>& b)
{
    double m = 0.0;
    for (std::size_t i = 0; i < a.size(); ++i)
        m = std::max(m, static_cast<double>(std::abs(a[i] - b[i])));
    return m;
}
}

int main()
{
    const auto on = render(256, 256, 1.0f, 0.5f, 0.7f);
    check(maximumDifference(on, render(64, 64, 1.0f, 0.5f, 0.7f)) == 0.0, "director on: blocks of 64 = blocks of 256, bit for bit");
    check(maximumDifference(on, render(7, 1000, 1.0f, 0.5f, 0.7f)) == 0.0, "director on: irregular blocks 7/1000 = 256, bit for bit");
    check(maximumDifference(on, render(256, 256, 1.0f, 0.5f, 0.7f)) == 0.0, "director on: the same seed, the same output");

    const auto bypass = render(256, 256, 0.0f, 0.5f, 0.7f);
    check(maximumDifference(bypass, render(256, 256, 0.0f, 0.9f, 0.1f)) == 0.0,
          "bypass: at STRUCTURE MIX 0 the other structure controls change nothing");
    const auto acting = maximumDifference(bypass, on);
    std::cout << "director on vs bypass: max difference " << acting << '\n';
    check(acting > 0.01, "the director really acts when on");

    if (failures == 0)
        std::cout << "variation route: all checks passed\n";
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
