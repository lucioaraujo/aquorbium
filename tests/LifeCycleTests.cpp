// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

// Life, food and descent (2026-10-21): hunger kills sooner than age, food
// keeps a life to its end, singing costs, age darkens only the second half,
// and a newborn's phrase carries its parent's head and its mate's tail.
#include "core/LifeCycle.h"

#include <cmath>
#include <cstdlib>
#include <iostream>

namespace
{
using aquorbium::LifeCycle;
int failures = 0;
void check(bool condition, const char* what)
{
    if (!condition)
    {
        std::cerr << "FAILED: " << what << '\n';
        ++failures;
    }
}
// seconds until the life ends, one bar (2.4 s) at a time
float lifetime(float prey, int notesPerBar, bool hunter, LifeCycle::Event& how)
{
    LifeCycle::Life life;
    LifeCycle::beBorn(life, 600.0f, 1);
    float t = 0.0f;
    for (int bar = 0; bar < 10000; ++bar)
    {
        const auto food = hunter ? 0.0f : LifeCycle::graze(life, 2.4f, prey);
        how = LifeCycle::live(life, 2.4f, food, notesPerBar);
        t += 2.4f;
        if (how != LifeCycle::Event::none)
            return t;
    }
    return t;
}
}

int main()
{
    LifeCycle::Event how {};
    const auto fed = lifetime(1.0f, 2, false, how);
    check(how == LifeCycle::Event::oldAge && std::abs(fed - 600.0f) < 3.0f, "a fed grazer lives to its end");
    const auto hunter = lifetime(1.0f, 2, true, how);
    check(how == LifeCycle::Event::starved && hunter < 600.0f, "a hunter that never captures starves");
    const auto scarce = lifetime(0.4f, 2, false, how);
    check(how == LifeCycle::Event::starved, "scarce prey starve the grazers");
    const auto quiet = lifetime(0.4f, 0, false, how);
    check(quiet > scarce, "singing costs: a silent grazer lasts longer in scarcity");
    std::cout << "lifetimes: fed " << fed << " s, hunter without prey " << hunter << " s, scarce " << scarce
              << " s, scarce and silent " << quiet << " s\n";

    // a capture is a meal
    {
        LifeCycle::Life life;
        LifeCycle::beBorn(life, 600.0f, 0);
        life.energy = 0.2f;
        check(LifeCycle::hunger(life) > 0.3f, "low energy is hunger");
        LifeCycle::live(life, 2.4f, LifeCycle::captureMeal, 1);
        check(life.energy > 0.65f && LifeCycle::hunger(life) == 0.0f, "a capture feeds");
    }
    // age darkens the second half only
    {
        LifeCycle::Life life;
        life.age = 0.4f;
        check(LifeCycle::darkness(life) == 0.0f, "the young are bright");
        life.age = 0.75f;
        check(std::abs(LifeCycle::darkness(life) - 0.5f) < 0.01f, "half dark at three quarters of life");
        life.age = 1.0f;
        check(LifeCycle::darkness(life) == 1.0f, "the old are dark");
    }
    // descent: head of a, tail of b (moved to continue a's contour), one mutation
    {
        LifeCycle::Phrase a, b;
        a.length = 6;
        b.length = 6;
        for (std::size_t u = 0; u < 6; ++u)
        {
            a.degrees[u] = static_cast<std::int8_t>(u);          // 0 1 2 3 4 5, steps 2
            a.steps[u] = 2;
            b.degrees[u] = static_cast<std::int8_t>(-static_cast<int>(u) / 2);  // 0 0 -1 -1 -2 -2, steps 4
            b.steps[u] = 4;
        }
        const auto child = LifeCycle::recombine(a, b, 0.5f, 0.99f, 0.9f);
        check(child.length == 6, "the child's length lies between the parents'");
        check(child.degrees[0] == 0 && child.degrees[1] == 1 && child.steps[0] == 2, "the head is the parent's");
        check(child.steps[5] == 4 && child.steps[4] == 4, "the tail keeps the mate's rhythm");
        // head = 1 + 0.5*5 = 3 units (0 1 2); b's tail from index 3 (-1 -2 -2),
        // joined at b[2] = -1 -> shift +3: 2 1 1; the last unit mutated +1
        check(child.degrees[3] == 2 && child.degrees[4] == 1 && child.degrees[5] == 2,
              "the mate's contour continues from the head, one unit mutated");
        bool inRange = true;
        for (int seed = 0; seed < 200; ++seed)
        {
            const auto f = [seed](int k) { return std::fmod(0.6180339f * static_cast<float>(seed * 7 + k), 1.0f); };
            LifeCycle::Phrase x = a, y = b;
            x.length = static_cast<std::uint8_t>(3 + seed % 6);
            y.length = static_cast<std::uint8_t>(3 + (seed / 6) % 6);
            const auto c = LifeCycle::recombine(x, y, f(1), f(2), f(3));
            inRange = inRange && c.length >= 3 && c.length <= 8;
            for (std::size_t u = 0; u < c.length; ++u)
                inRange = inRange && c.degrees[u] >= -5 && c.degrees[u] <= 7 && c.steps[u] >= 1;
        }
        check(inRange, "every child is a valid phrase");
    }
    if (failures == 0)
        std::cout << "life cycle: all checks passed\n";
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
