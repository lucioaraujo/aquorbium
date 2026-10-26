// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

// The loudness meter against the EBU's minimum-requirement signals
// (2026-10-22): EBU Tech 3341 V4 (November 2023), Table 1, cases 1-5 and
// 9-14, synthesised here from their descriptions (no EBU file). Not run: #6
// (5.0 channels; the meter is stereo) and #7-8 (authentic programme files,
// which the RASGO rules keep out of the repository). Tolerance: +-0.1 LU.
#include "core/LoudnessMeter.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace
{
constexpr double pi = 3.14159265358979323846;
constexpr double fs = 48000.0;
int failures = 0;
using Segment = std::pair<double, double>;  // seconds, dBFS peak per channel (<= -200: silence)
constexpr double silence = -999.0;

// a stereo 1 kHz sine, in phase on both channels, through the segments; the
// tone's phase runs on across them. `each` sees the readings after every
// 100 ms of signal (the meter's update), with the time and the segment index
std::vector<aquorbium::LoudnessMeter::Readings> play(aquorbium::LoudnessMeter& meter, const std::vector<Segment>& segments,
                                                       std::vector<double>* times = nullptr, std::vector<std::size_t>* which = nullptr)
{
    std::vector<aquorbium::LoudnessMeter::Readings> out;
    std::size_t n = 0;
    for (std::size_t s = 0; s < segments.size(); ++s)
    {
        const auto [seconds, db] = segments[s];
        const auto count = static_cast<std::size_t>(std::llround(seconds * fs));
        const auto amplitude = db <= -200.0 ? 0.0 : std::pow(10.0, db / 20.0);
        for (std::size_t i = 0; i < count; ++i, ++n)
        {
            const auto x = static_cast<float>(amplitude * std::sin(2.0 * pi * 1000.0 * static_cast<double>(n) / fs));
            meter.push(x, x);
            if ((n + 1) % 480 == 0)  // every 10 ms: finer than the meter's own update
            {
                out.push_back(meter.readings());
                if (times != nullptr) times->push_back(static_cast<double>(n + 1) / fs);
                if (which != nullptr) which->push_back(s);
            }
        }
    }
    return out;
}

void expect(const std::string& name, double measured, double expected)
{
    const auto ok = std::abs(measured - expected) <= 0.1 + 1.0e-9;
    std::cout << name << ": " << measured << " (expected " << expected << " +-0.1)" << (ok ? "" : "  FAIL") << '\n';
    if (!ok) ++failures;
}

aquorbium::LoudnessMeter fresh()
{
    aquorbium::LoudnessMeter m;
    m.prepare(fs);
    return m;
}
}

int main()
{
    // #1, #2: M, S, I of a 20 s tone
    for (const auto& [test, level] : { std::pair { 1, -23.0 }, std::pair { 2, -33.0 } })
    {
        auto meter = fresh();
        const auto r = play(meter, { { 20.0, level } }).back();
        expect("#" + std::to_string(test) + " M", r.momentaryLufs, level);
        expect("#" + std::to_string(test) + " S", r.shortTermLufs, level);
        expect("#" + std::to_string(test) + " I", r.integratedLufs, level);
    }
    // #3, #4, #5: the integrated loudness through the gates
    const std::vector<std::pair<int, std::vector<Segment>>> integrated {
        { 3, { { 10.0, -36.0 }, { 60.0, -23.0 }, { 10.0, -36.0 } } },
        { 4, { { 10.0, -72.0 }, { 10.0, -36.0 }, { 60.0, -23.0 }, { 10.0, -36.0 }, { 10.0, -72.0 } } },
        { 5, { { 20.0, -26.0 }, { 20.1, -20.0 }, { 20.0, -26.0 } } } };
    for (const auto& [test, segments] : integrated)
    {
        auto meter = fresh();
        expect("#" + std::to_string(test) + " I", play(meter, segments).back().integratedLufs, -23.0);
    }
    // #9: S constant at -23 after 3 s; #12: M constant at -23 after 1 s
    for (const auto& [test, high, low, repeats, settle, shortTerm] :
         { std::tuple { 9, 1.34, 1.66, 5, 3.0, true }, std::tuple { 12, 0.18, 0.22, 25, 1.0, false } })
    {
        std::vector<Segment> segments;
        for (int k = 0; k < repeats; ++k)
        {
            segments.push_back({ high, -20.0 });
            segments.push_back({ low, -30.0 });
        }
        auto meter = fresh();
        std::vector<double> times;
        const auto r = play(meter, segments, &times);
        double lo = 99.0, hi = -99.0;
        for (std::size_t i = 0; i < r.size(); ++i)
            if (times[i] > settle + 1.0e-9)
            {
                const auto v = shortTerm ? r[i].shortTermLufs : r[i].momentaryLufs;
                lo = std::min<double>(lo, v);
                hi = std::max<double>(hi, v);
            }
        expect("#" + std::to_string(test) + (shortTerm ? " S min after 3 s" : " M min after 1 s"), lo, -23.0);
        expect("#" + std::to_string(test) + (shortTerm ? " S max after 3 s" : " M max after 1 s"), hi, -23.0);
    }
    // #10, #13 (file-based: each segment measured on its own): max S / max M
    // #11, #14 ('live': one run): max S / max M of each tone = -38 + i
    for (const auto& [test, step, toneSeconds, tailFile, shortTerm] :
         { std::tuple { 10, 0.15, 3.0, 1.0, true }, std::tuple { 13, 0.020, 0.4, 1.0, false } })
    {
        double worst = 0.0, worstValue = -23.0;
        for (int i = 0; i < 20; ++i)
        {
            auto meter = fresh();
            const auto r = play(meter, { { i * step, silence }, { toneSeconds, -23.0 }, { tailFile, silence } });
            double best = -99.0;
            for (const auto& x : r)
                best = std::max<double>(best, shortTerm ? x.shortTermLufs : x.momentaryLufs);
            if (std::abs(best + 23.0) > worst) { worst = std::abs(best + 23.0); worstValue = best; }
        }
        expect("#" + std::to_string(test) + (shortTerm ? " max S, worst of 20 segments" : " max M, worst of 20 segments"), worstValue, -23.0);
    }
    for (const auto& [test, step, toneSeconds, shortTerm] :
         { std::tuple { 11, 0.15, 3.0, true }, std::tuple { 14, 0.020, 0.4, false } })
    {
        std::vector<Segment> segments;
        for (int i = 0; i < 20; ++i)
        {
            segments.push_back({ i * step, silence });
            segments.push_back({ toneSeconds, -38.0 + i });
            segments.push_back({ toneSeconds - i * step, silence });
        }
        auto meter = fresh();
        std::vector<std::size_t> which;
        const auto r = play(meter, segments, nullptr, &which);
        double worst = 0.0;
        int worstTone = 0;
        double worstValue = 0.0;
        for (int i = 0; i < 20; ++i)
        {
            double best = -99.0;
            for (std::size_t k = 0; k < r.size(); ++k)
                if (which[k] / 3 == static_cast<std::size_t>(i))
                    best = std::max<double>(best, shortTerm ? r[k].shortTermLufs : r[k].momentaryLufs);
            const auto error = std::abs(best - (-38.0 + i));
            if (error > worst) { worst = error; worstTone = i; worstValue = best; }
        }
        expect("#" + std::to_string(test) + (shortTerm ? " max S" : " max M") + ", worst tone (" + std::to_string(worstTone)
                   + ", expected " + std::to_string(-38 + worstTone) + ")",
               worstValue - (-38.0 + worstTone) - 23.0, -23.0);
    }
    if (failures == 0)
        std::cout << "EBU Tech 3341 loudness (cases 1-5, 9-14): all within tolerance\n";
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
