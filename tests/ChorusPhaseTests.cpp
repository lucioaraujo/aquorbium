// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

// The chorus of coupled phase oscillators (2026-10-20): listening makes the
// timing emerge - anti-phase between rivals, synchrony between allies,
// three voices in tri-phase, a quarter-cycle answer for a courting pair - and
// without hearing each other the voices keep their own clocks.
#include "core/ChorusPhase.h"

#include <cmath>
#include <cstdlib>
#include <iostream>

namespace
{
using aquorbium::ChorusPhase;

int failures = 0;
void check(bool condition, const char* what)
{
    if (!condition)
    {
        std::cerr << "FAILED: " << what << '\n';
        ++failures;
    }
}

// run n voices with the same rate; every pair hears each other each step
ChorusPhase run(std::size_t voices, float offset, int steps, std::uint32_t seed = 7u, float rateSpread = 0.0f)
{
    ChorusPhase chorus;
    chorus.reset(seed);
    for (std::size_t i = 0; i < voices; ++i)
        chorus.setNaturalRate(i, 1.0f / 16.0f * (1.0f + rateSpread * static_cast<float>(i)));
    for (int s = 0; s < steps; ++s)
    {
        for (std::size_t i = 0; i < voices; ++i)
            for (std::size_t j = 0; j < voices; ++j)
                if (i != j)
                    chorus.hear(i, j, 1.0f, offset, 0.15f);
        static_cast<void>(chorus.step());
    }
    return chorus;
}
}

int main()
{
    // two rivals: anti-phase (they alternate)
    {
        const auto c = run(2, 0.5f, 4000);
        check(std::abs(std::abs(c.difference(0, 1)) - 0.5f) < 0.03f, "two rivals settle in anti-phase");
    }
    // two allies: in phase
    {
        const auto c = run(2, 0.0f, 4000, 11u);
        check(std::abs(c.difference(0, 1)) < 0.03f, "two allies settle in phase");
    }
    // a courting pair: a quarter cycle apart (call and answer)
    {
        ChorusPhase chorus;
        chorus.reset(5u);
        for (int s = 0; s < 4000; ++s)
        {
            chorus.hear(1, 0, 1.0f, 0.25f, 0.15f);  // the answerer follows a quarter behind... ahead
            static_cast<void>(chorus.step());
        }
        check(std::abs(chorus.difference(0, 1) - 0.25f) < 0.03f, "the answer settles a quarter cycle after the call");
    }
    // three rivals: they spread around the cycle (no two together)
    {
        const auto c = run(3, 0.5f, 6000, 3u);
        float closest = 1.0f;
        for (std::size_t a = 0; a < 3; ++a)
            for (std::size_t b = a + 1; b < 3; ++b)
                closest = std::min(closest, std::abs(c.difference(a, b)));
        check(closest > 0.2f, "three rivals spread apart (tri-phase-like)");
    }
    // slightly different natural rates still lock when they hear each other
    {
        const auto c = run(2, 0.0f, 6000, 9u, 0.04f);
        check(std::abs(c.difference(0, 1)) < 0.08f, "allies with different clocks still lock");
    }
    // deaf voices keep their own clocks: the firing count follows the rate
    {
        ChorusPhase chorus;
        chorus.reset(1u);
        chorus.setNaturalRate(0, 1.0f / 8.0f);
        chorus.setNaturalRate(1, 1.0f / 32.0f);
        int fired0 = 0, fired1 = 0;
        for (int s = 0; s < 3200; ++s)
        {
            const auto f = chorus.step();
            fired0 += f[0] ? 1 : 0;
            fired1 += f[1] ? 1 : 0;
        }
        check(std::abs(fired0 - 400) <= 1 && std::abs(fired1 - 100) <= 1, "without listening, each keeps its natural rate");
    }
    if (failures == 0)
        std::cout << "chorus phase: all checks passed\n";
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
