// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

// The habitat's health, heard (2026-10-21): ADI and ACI on known signals.
#include "core/HabitatHealth.h"

#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <iostream>

namespace
{
using aquorbium::HabitatHealth;
int failures = 0;
void check(bool condition, const char* what)
{
    if (!condition)
    {
        std::cerr << "FAILED: " << what << '\n';
        ++failures;
    }
}
constexpr double pi = 3.14159265358979323846;
HabitatHealth::Reading measure(const std::function<float(int)>& signal)
{
    HabitatHealth h;
    h.prepare(48000.0, 5.0);
    for (int n = 0; n < 48000 * 6; ++n)
        h.push(signal(n));
    return h.reading();
}
}

int main()
{
    const auto silence = measure([](int) { return 0.0f; });
    check(!silence.measured && silence.adi == 0.0f && silence.health == 0.5f, "silence: nothing measured, neutral health");

    // a tone at the centre of its band (264-459 Hz)
    const auto drone = measure([](int n) { return 0.3f * static_cast<float>(std::sin(2.0 * pi * 348.0 * n / 48000.0)); });
    check(drone.measured && drone.adi < 0.15f, "one steady tone occupies one band");
    check(drone.aci < 0.05f, "a steady tone has no acoustic complexity");

    // three tones in three distant bands (each at its band's centre)
    const auto three = measure([](int n) {
        const auto t = n / 48000.0;
        return static_cast<float>(0.1 * (std::sin(2.0 * pi * 115.0 * t) + std::sin(2.0 * pi * 1056.0 * t)
                                          + std::sin(2.0 * pi * 5572.0 * t)));
    });
    check(std::abs(three.adi - std::log(3.0f) / std::log(10.0f)) < 0.08f, "three bands used evenly: ADI = ln 3 / ln 10");

    // the same tone pulsing 100 ms on, 100 ms off: articulated
    const auto pulse = measure([](int n) {
        const auto on = (n / 4800) % 2 == 0;
        return on ? 0.3f * static_cast<float>(std::sin(2.0 * pi * 348.0 * n / 48000.0)) : 0.0f;
    });
    check(pulse.aci > 5.0f * std::max(drone.aci, 0.01f), "a pulsing tone is more complex than a drone");

    // a varied, articulated soundscape: tones in several bands, each pulsing
    // at its own rate - the healthy reef
    const auto reef = measure([](int n) {
        const auto t = n / 48000.0;
        double s = 0.0;
        const double hz[] { 90.0, 300.0, 900.0, 2500.0, 7000.0 };
        const int period[] { 7000, 5100, 3900, 2900, 2300 };
        for (int k = 0; k < 5; ++k)
            if ((n / period[k]) % 2 == 0)
                s += 0.06 * std::sin(2.0 * pi * hz[k] * t);
        return static_cast<float>(s);
    });
    std::cout << "drone: ADI " << drone.adi << " ACI " << drone.aci << " health " << drone.health
              << " | pulse: ACI " << pulse.aci << " | three tones: ADI " << three.adi << " | reef: ADI " << reef.adi
              << " ACI " << reef.aci << " health " << reef.health << '\n';
    check(reef.health > drone.health + 0.4f, "a varied, articulated reef is healthier than a drone");

    if (failures == 0)
        std::cout << "habitat health: all checks passed\n";
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
