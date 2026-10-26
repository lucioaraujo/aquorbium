// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

// The true-peak meter against the EBU's minimum-requirement signals
// (2026-10-22): EBU Tech 3341 V4 (November 2023), "'EBU Mode' metering to
// supplement loudness normalisation", Table 1, signals 15-23, read from
// https://tech.ebu.ch/docs/tech/tech3341.pdf. No EBU file is used: each
// signal is synthesised here from its description in the table. Expected:
// the stated maximum true-peak level, +0.2/-0.4 dBTP. Measured through
// TakeAnalysis (the REC's meter, the 4x TruePeakDetector from Navalha).
#include "core/TakeAnalysis.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

namespace
{
constexpr double pi = 3.14159265358979323846;
constexpr double fs = 48000.0;
int failures = 0;

double truePeakDb(const std::vector<double>& signal)
{
    aquorbium::TakeAnalysis meter;
    meter.prepare(fs);
    for (const auto x : signal)
    {
        auto l = static_cast<float>(x), r = static_cast<float>(x);  // a stereo tone, both channels
        meter.push(l, r);
    }
    return meter.result().truePeakDb;
}

void expect(const std::string& name, double measured, double expected)
{
    const auto ok = measured <= expected + 0.2 && measured >= expected - 0.4;
    std::cout << name << ": " << measured << " dBTP (expected " << expected << " +0.2/-0.4)" << (ok ? "" : "  FAIL") << '\n';
    if (!ok)
        ++failures;
}

// a tone of 1 s with 10 ms fade-in and fade-out (signals 15-19)
std::vector<double> tone(double frequency, double amplitude, double phaseDegrees)
{
    const auto n = static_cast<std::size_t>(fs);
    const auto fade = static_cast<std::size_t>(fs * 0.010);
    std::vector<double> x(n);
    for (std::size_t i = 0; i < n; ++i)
    {
        const auto gain = std::min({ 1.0, static_cast<double>(i) / fade, static_cast<double>(n - 1 - i) / fade });
        x[i] = gain * amplitude * std::sin(2.0 * pi * frequency * static_cast<double>(i) / fs + phaseDegrees * pi / 180.0);
    }
    return x;
}

// signals 20-23: synthesised at 4 fs - a sine at fs/6, amplitude 0.5, with a
// single period of fs/4 at amplitude 1.0 in the middle, continuous in phase
// on both sides - then low-pass filtered (anti-aliasing) and taken every 4th
// sample from the given offset
std::vector<double> singlePeriod(std::size_t offset)
{
    const auto high = 4.0 * fs;
    const auto n = static_cast<std::size_t>(high * 0.5);
    std::vector<double> x(n);
    // the fs/6 tone starts at a rising zero crossing; the fs/4 period is
    // placed at another rising zero crossing of it, so both sides join at 0
    const auto slowPeriod = high / (fs / 6.0);  // 24 samples at 4 fs
    const auto fastPeriod = high / (fs / 4.0);  // 16 samples at 4 fs
    const auto start = static_cast<std::size_t>(slowPeriod * 2000.0);
    const auto fade = static_cast<std::size_t>(high * 0.005);
    for (std::size_t i = 0; i < n; ++i)
    {
        double v = 0.0;
        if (i < start)
            v = 0.5 * std::sin(2.0 * pi * static_cast<double>(i) / slowPeriod);
        else if (static_cast<double>(i - start) < fastPeriod)
            v = 1.0 * std::sin(2.0 * pi * static_cast<double>(i - start) / fastPeriod);
        else
            v = 0.5 * std::sin(2.0 * pi * (static_cast<double>(i - start) - fastPeriod) / slowPeriod);
        const auto gain = std::min({ 1.0, static_cast<double>(i) / fade, static_cast<double>(n - 1 - i) / fade });
        x[i] = gain * v;
    }
    // the anti-aliasing low-pass at 4 fs: a windowed sinc (Blackman), cutoff
    // just under fs/2, 255 taps
    constexpr int taps = 255;
    const auto cutoff = 0.45 * fs / high;  // cycles per sample at 4 fs
    std::array<double, taps> h {};
    double sum = 0.0;
    for (int k = 0; k < taps; ++k)
    {
        const auto m = k - (taps - 1) / 2;
        const auto sinc = m == 0 ? 2.0 * cutoff : std::sin(2.0 * pi * cutoff * m) / (pi * m);
        const auto window = 0.42 - 0.5 * std::cos(2.0 * pi * k / (taps - 1)) + 0.08 * std::cos(4.0 * pi * k / (taps - 1));
        h[static_cast<std::size_t>(k)] = sinc * window;
        sum += h[static_cast<std::size_t>(k)];
    }
    for (auto& c : h)
        c /= sum;
    std::vector<double> out;
    for (std::size_t i = offset + taps; i + taps < n; i += 4)
    {
        double y = 0.0;
        for (int k = 0; k < taps; ++k)
            y += h[static_cast<std::size_t>(k)] * x[i - static_cast<std::size_t>(k)];
        out.push_back(y);
    }
    return out;
}
}

int main()
{
    expect("EBU 3341 #15  fs/4, 0.50 FFS, 0 deg", truePeakDb(tone(fs / 4.0, 0.5, 0.0)), -6.0);
    expect("EBU 3341 #16  fs/4, 0.50 FFS, 45 deg", truePeakDb(tone(fs / 4.0, 0.5, 45.0)), -6.0);
    expect("EBU 3341 #17  fs/6, 0.50 FFS, 60 deg", truePeakDb(tone(fs / 6.0, 0.5, 60.0)), -6.0);
    expect("EBU 3341 #18  fs/8, 0.50 FFS, 67.5 deg", truePeakDb(tone(fs / 8.0, 0.5, 67.5)), -6.0);
    expect("EBU 3341 #19  fs/4, 1.41 FFS, 45 deg", truePeakDb(tone(fs / 4.0, 1.41, 45.0)), 3.0);
    for (std::size_t offset = 0; offset < 4; ++offset)
        expect("EBU 3341 #" + std::to_string(20 + offset) + "  single fs/4 period, offset " + std::to_string(offset),
               truePeakDb(singlePeriod(offset)), 0.0);
    if (failures == 0)
        std::cout << "EBU Tech 3341 true peak (signals 15-23): all within tolerance\n";
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
