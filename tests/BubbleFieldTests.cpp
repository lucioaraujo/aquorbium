// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

// The boiling water (2026-10-21): Minnaert's frequency, the split of coupled
// bubbles, the Deane & Stokes size distribution, the birth rate, silence at
// rate zero, bounded output and determinism.
#include "core/BubbleField.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <vector>

namespace
{
using aquorbium::BubbleField;
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
    check(std::abs(BubbleField::minnaertHz(1.0e-3f) - 3260.0f) < 1.0f, "a 1 mm bubble rings at 3.26 kHz (Minnaert)");
    const auto f0 = 3260.0f, r = 1.0e-3f, d = 2.0e-3f;
    check(std::abs(BubbleField::coupledLowHz(f0, r, d) / f0 - 1.0f / std::sqrt(1.5f)) < 1.0e-4f
              && std::abs(BubbleField::coupledHighHz(f0, r, d) / f0 - 1.0f / std::sqrt(0.5f)) < 1.0e-4f,
          "two bubbles at twice their radius split into f0/sqrt(1.5) and f0/sqrt(0.5)");
    check(std::abs(BubbleField::coupledLowHz(f0, r, 100.0f * r) / f0 - 1.0f) < 0.01f, "far apart they are independent");

    // the size distribution: log-log slopes of the density below and above 1 mm
    std::vector<int> histogram(40, 0);
    const auto logMin = std::log(0.25e-3f), logMax = std::log(6.0e-3f);
    for (int i = 0; i < 400000; ++i)
    {
        const auto u = (static_cast<float>(i) + 0.5f) / 400000.0f;
        const auto radius = BubbleField::drawRadius(u);
        const auto bin = static_cast<int>((std::log(radius) - logMin) / (logMax - logMin) * 40.0f);
        if (bin >= 0 && bin < 40)
            ++histogram[static_cast<std::size_t>(bin)];
    }
    // density per unit radius = count / bin width (bin width ~ R in log bins)
    const auto density = [&](int bin) {
        const auto centre = std::exp(logMin + (static_cast<float>(bin) + 0.5f) / 40.0f * (logMax - logMin));
        return std::log(static_cast<double>(histogram[static_cast<std::size_t>(bin)]) / centre);
    };
    const auto centreLog = [&](int bin) { return logMin + (static_cast<float>(bin) + 0.5f) / 40.0f * (logMax - logMin); };
    const auto slopeBelow = (density(14) - density(2)) / (centreLog(14) - centreLog(2));
    const auto slopeAbove = (density(36) - density(22)) / (centreLog(36) - centreLog(22));
    std::cout << "size distribution slopes: " << slopeBelow << " below 1 mm (-1.5), " << slopeAbove << " above (-3.33)\n";
    check(std::abs(slopeBelow - (-1.5)) < 0.2, "R^(-3/2) below the Hinze scale (Deane & Stokes)");
    check(std::abs(slopeAbove - (-10.0 / 3.0)) < 0.3, "R^(-10/3) above it");

    // the rate, the pairs, the bounds
    BubbleField field;
    field.prepare(48000.0, 7u);
    field.setRate(50.0f);
    float peak = 0.0f;
    for (int n = 0; n < 48000 * 10; ++n)
    {
        const auto s = field.render();
        peak = std::max({ peak, std::abs(s.left), std::abs(s.right) });
    }
    std::cout << "10 s at 50/s: " << field.born() << " bubbles, " << field.pairs() << " coupled pairs, peak " << peak << '\n';
    check(field.born() > 425 && field.born() < 575, "about fifty bubbles a second");
    check(field.pairs() > 0, "some bubbles are born together and couple");
    check(peak > 0.01f && peak < 2.0f, "audible and bounded");

    BubbleField boiling;
    boiling.prepare(48000.0, 7u);
    boiling.setRate(400.0f);
    float loudest = 0.0f;
    for (int n = 0; n < 48000 * 5; ++n)
    {
        const auto s = boiling.render();
        loudest = std::max({ loudest, std::abs(s.left), std::abs(s.right) });
    }
    check(loudest < 3.0f && std::isfinite(loudest), "a hard boil stays bounded");

    BubbleField still;
    still.prepare(48000.0, 7u);
    still.setRate(0.0f);
    bool silent = true;
    for (int n = 0; n < 48000; ++n)
    {
        const auto s = still.render();
        silent = silent && s.left == 0.0f && s.right == 0.0f;
    }
    check(silent, "still water: exact zeros");

    BubbleField a, b;
    a.prepare(48000.0, 9u);
    b.prepare(48000.0, 9u);
    a.setRate(80.0f);
    b.setRate(80.0f);
    bool same = true;
    for (int n = 0; n < 48000; ++n)
    {
        const auto x = a.render(), y = b.render();
        same = same && x.left == y.left && x.right == y.right;
    }
    check(same, "the same seed gives the same water");

    if (failures == 0)
        std::cout << "bubble field: all checks passed\n";
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
