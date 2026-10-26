// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

// The head (2026-10-21): Woodworth's interaural delay, the head's shadow
// growing with frequency, symmetry in front, the pinna's notch moving with
// the elevation, bounded output.
#include "core/BinauralSource.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <vector>

namespace
{
using aquorbium::BinauralSource;
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
std::uint32_t state = 0x2468aceu;
float noise()
{
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return static_cast<float>(state) / 4294967296.0f * 2.0f - 1.0f;
}
struct Ears
{
    std::vector<float> left, right;
};
Ears listen(float azimuth, float elevation, int samples = 48000)
{
    BinauralSource head;
    head.prepare(48000.0);
    head.setDirection(azimuth, elevation);
    head.reset();  // start settled at the direction
    head.setDirection(azimuth, elevation);
    Ears e;
    for (int n = 0; n < samples; ++n)
    {
        const auto s = head.tick(noise() * 0.5f);
        e.left.push_back(s.left);
        e.right.push_back(s.right);
    }
    return e;
}
// the level of a band (Hann-windowed DFT averaged over a few bins)
double bandDb(const std::vector<float>& x, double hz)
{
    double total = 0.0;
    for (int k = -3; k <= 3; ++k)
    {
        const auto f = hz + k * 20.0;
        double re = 0.0, im = 0.0;
        for (std::size_t i = 0; i < x.size(); ++i)
        {
            const auto w = 0.5 - 0.5 * std::cos(2.0 * pi * static_cast<double>(i) / static_cast<double>(x.size()));
            re += x[i] * w * std::cos(2.0 * pi * f * static_cast<double>(i) / 48000.0);
            im -= x[i] * w * std::sin(2.0 * pi * f * static_cast<double>(i) / 48000.0);
        }
        total += re * re + im * im;
    }
    return 10.0 * std::log10(total + 1.0e-20);
}
}

int main()
{
    // Woodworth's maximum: a/c (pi/2 + 1) = 0.656 ms for an 8.75 cm head
    const auto itd = BinauralSource::interauralDelaySeconds(static_cast<float>(pi))
        - BinauralSource::interauralDelaySeconds(0.0f);
    check(std::abs(itd - 0.0875 / 343.0 * (pi / 2.0 + 1.0)) < 1.0e-6, "Woodworth's maximum delay, ~0.66 ms");
    check(std::abs(BinauralSource::shadowAlpha(0.0f) - 2.0f) < 1.0e-5f, "the ear facing the source: highs doubled (+6 dB)");

    // a source at the right: the right ear leads by ~0.66 ms
    const auto right = listen(static_cast<float>(pi / 2.0), 0.0f);
    int bestLag = 0;
    double best = -1.0e30;
    for (int lag = -60; lag <= 60; ++lag)
    {
        double c = 0.0;
        for (std::size_t i = 100; i + 100 < right.left.size(); ++i)
            c += static_cast<double>(right.right[i]) * right.left[static_cast<std::size_t>(static_cast<int>(i) + lag)];
        if (c > best)
        {
            best = c;
            bestLag = lag;
        }
    }
    const auto lagMs = bestLag / 48.0;
    const auto ildLow = bandDb(right.right, 200.0) - bandDb(right.left, 200.0);
    const auto ildHigh = bandDb(right.right, 6000.0) - bandDb(right.left, 6000.0);
    std::cout << "source at the right: the left ear lags by " << lagMs << " ms; level difference " << ildLow
              << " dB at 200 Hz, " << ildHigh << " dB at 6 kHz\n";
    check(lagMs > 0.55 && lagMs < 0.75, "the far ear hears it ~0.66 ms later");
    check(ildHigh > 12.0, "the head shadows the highs at the far ear");
    check(std::abs(ildLow) < 4.0 && ildHigh > ildLow + 8.0, "the shadow grows with frequency (the lows bend around)");

    // in front: the two ears are the same
    const auto front = listen(0.0f, 0.0f, 24000);
    double difference = 0.0;
    for (std::size_t i = 0; i < front.left.size(); ++i)
        difference = std::max(difference, static_cast<double>(std::abs(front.left[i] - front.right[i])));
    check(difference < 1.0e-6, "straight ahead the ears hear the same");

    // the pinna: the spectrum changes with the elevation (its notch moves)
    const auto level = listen(0.0f, 0.0f), above = listen(0.0f, static_cast<float>(pi / 4.0));
    double maxChange = 0.0;
    for (double hz = 2000.0; hz <= 14000.0; hz += 500.0)
        maxChange = std::max(maxChange, std::abs(bandDb(level.left, hz) - bandDb(above.left, hz)));
    std::cout << "elevation 0 vs 45 degrees: up to " << maxChange << " dB of spectral change (2-14 kHz)\n";
    check(maxChange > 6.0, "the pinna colours the sound by its height");

    bool bounded = true;
    for (const auto v : right.left)
        bounded = bounded && std::isfinite(v) && std::abs(v) < 4.0f;
    check(bounded, "bounded and finite");

    if (failures == 0)
        std::cout << "binaural source: all checks passed\n";
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
