// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

// Golden renders (2026-10-22; SAIDA_AUDIO_COMUM §5, "testes dourados"): five
// fixed-seed renders - the defaults, the ecological music, the synthesis
// device, the module bank, the headphones - measured (integrated LUFS, true
// peak, the RMS of four windows) against values recorded on 2026-10-22. An
// unintended change of the sound shows here; an intended one updates the
// values (run with --print and paste them, saying why in the commit).
// The engine is chaotic: a floating-point difference between compilers can
// become another music, so the values bind only where they were recorded
// (Linux, GCC, x86-64); elsewhere the test prints and passes.
#include "core/BiomaEngine.h"
#include "core/LoudnessMeter.h"
#include "core/SynthProposal.h"
#include "core/TruePeakDetector.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <memory>
#include <vector>

namespace
{
struct Metrics
{
    double lufs = 0.0, truePeak = 0.0;
    std::array<double, 4> rms {};
};

Metrics render(const std::function<void(aquorbium::BiomaEngine&)>& setup)
{
    auto engine = std::make_unique<aquorbium::BiomaEngine>();
    engine->prepare(48000.0);
    engine->setSeed(0x601d3u);
    engine->setEcosystemBlendImmediately(1.0f);
    engine->setAutonomousEnabled(true);
    setup(*engine);
    engine->noteOn(55, 0.7f);
    aquorbium::LoudnessMeter meter;
    meter.prepare(48000.0);
    aquorbium::TruePeakDetector left, right;
    left.prepare(48000.0);
    right.prepare(48000.0);
    Metrics m;
    float peak = 0.0f;
    std::array<double, 4> sums {};
    std::vector<float> l(256), r(256);
    const std::size_t start = 48000, window = 60000;  // 1 s in, then 4 x 1.25 s
    std::size_t n = 0;
    while (n < start + 4 * window)
    {
        engine->render(l.data(), r.data(), l.size());
        for (std::size_t i = 0; i < l.size(); ++i, ++n)
        {
            if (n < start || n >= start + 4 * window)
                continue;
            meter.push(l[i], r[i]);
            peak = std::max({ peak, left.processSample(l[i]), right.processSample(r[i]) });
            sums[(n - start) / window] += 0.5 * (static_cast<double>(l[i]) * l[i] + static_cast<double>(r[i]) * r[i]);
        }
    }
    m.lufs = meter.readings().integratedLufs;
    m.truePeak = 20.0 * std::log10(std::max(peak, 1.0e-9f));
    for (std::size_t w = 0; w < 4; ++w)
        m.rms[w] = 10.0 * std::log10(std::max(sums[w] / static_cast<double>(window), 1.0e-20));
    return m;
}
}

int main(int argc, char** argv)
{
    const auto print = argc > 1 && std::strcmp(argv[1], "--print") == 0;
    const std::array<std::pair<const char*, std::function<void(aquorbium::BiomaEngine&)>>, 5> cases {{
        { "defaults", [](aquorbium::BiomaEngine&) {} },
        { "ecological music", [](aquorbium::BiomaEngine& e) { e.setEcologicalMusicPerformance({ 0.7f, 0.6f, 0.5f, 0.5f, 1.0f }); } },
        { "synthesis device", [](aquorbium::BiomaEngine& e) {
              for (std::size_t i = 0; i < aquorbium::SynthProposal::organisms; ++i)
                  e.setSynthPatch(i, aquorbium::SynthProposal::propose(i, 0x601d3u));
              e.setSynthMix(0.6f); } },
        { "module bank", [](aquorbium::BiomaEngine& e) {
              aquorbium::IntegratedModuleBank::Controls c {};
              c.fill(0.5f);
              c[aquorbium::IntegratedModuleBank::mix] = 0.7f;
              e.setIntegratedModuleBankControls(c); } },
        { "headphones + music", [](aquorbium::BiomaEngine& e) {
              e.setHeadphones(true);
              e.setEcologicalMusicPerformance({ 0.7f, 0.6f, 0.5f, 0.5f, 1.0f }); } },
    }};
    // recorded 2026-10-22 (Linux, GCC, x86-64): LUFS, dBTP, RMS x 4
    constexpr std::array<std::array<double, 6>, 5> golden {{
        {{ -28.7787, -16.2004, -33.0782, -28.4016, -30.7356, -34.2527 }},  // defaults
        {{ -31.0700, -14.6721, -33.5119, -35.7089, -37.1020, -31.1447 }},  // ecological music
        {{ -33.0661, -20.5022, -36.5223, -33.9226, -35.6385, -37.3153 }},  // synthesis device
        {{ -17.1868, -7.1485, -20.5329, -19.4853, -20.5871, -18.3839 }},  // module bank
        {{ -31.0327, -15.4462, -33.5332, -35.9699, -37.2032, -31.2719 }},  // headphones + music
    }};
#if defined(__linux__) && defined(__GNUC__) && !defined(__clang__) && defined(__x86_64__)
    constexpr bool binding = true;
#else
    constexpr bool binding = false;
#endif
    int failures = 0;
    for (std::size_t c = 0; c < cases.size(); ++c)
    {
        const auto m = render(cases[c].second);
        if (print)
        {
            std::printf("        {{ %.4f, %.4f, %.4f, %.4f, %.4f, %.4f }},  // %s\n", m.lufs, m.truePeak, m.rms[0], m.rms[1],
                        m.rms[2], m.rms[3], cases[c].first);
            continue;
        }
        const std::array<double, 6> got { m.lufs, m.truePeak, m.rms[0], m.rms[1], m.rms[2], m.rms[3] };
        double worst = 0.0;
        for (std::size_t k = 0; k < got.size(); ++k)
            worst = std::max(worst, std::abs(got[k] - golden[c][k]));
        const auto ok = worst <= 0.01;  // dB: the same build gives the same numbers
        std::printf("golden %-20s %.2f LUFS %.2f dBTP; worst difference %.4f dB%s\n", cases[c].first, m.lufs, m.truePeak, worst,
                    ok ? "" : binding ? "  FAIL" : "  (not binding on this platform)");
        if (!ok && binding)
            ++failures;
    }
    if (!print && failures == 0)
        std::printf("golden renders: all within 0.01 dB\n");
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
