// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

// The engine soak (2026-10-22; aquorbium-tarefas.md phase 10): 30 minutes of
// audio with everything on - the ecological music (whose water walks through
// its states), the synthesis device, the module bank, the headphones, MIDI
// notes coming and going - checked minute by minute: finite, under the
// ceiling, neither fading out nor running away, no block over the real-time
// budget, and no memory growth (no allocation on the way). The device's own
// soak (xruns, lost frames) needs the real device: it stays open.
#include "core/BiomaEngine.h"
#include "core/SynthProposal.h"
#include "core/TruePeakDetector.h"

#include <sys/resource.h>
#include <time.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <vector>

namespace
{
// the thread's own CPU time: a wall clock counts the moments the system took
// the thread away (a 27 ms "block" on a loaded machine was that)
double threadMicroseconds()
{
    timespec t {};
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &t);
    return static_cast<double>(t.tv_sec) * 1.0e6 + static_cast<double>(t.tv_nsec) / 1.0e3;
}

long residentKilobytes()
{
    rusage usage {};
    getrusage(RUSAGE_SELF, &usage);
    return usage.ru_maxrss;
}
}

int main()
{
    constexpr double rate = 48000.0;
    constexpr std::size_t block = 256;
    constexpr int minutes = 30;
    auto engine = std::make_unique<aquorbium::BiomaEngine>();
    engine->prepare(rate);
    engine->setSeed(0x50a6u);
    engine->setEcosystemBlendImmediately(1.0f);
    engine->setAutonomousEnabled(true);
    engine->setDensity(0.8f);
    engine->setEcologicalMusicPerformance({ 0.8f, 0.7f, 0.6f, 0.5f, 1.0f });
    for (std::size_t i = 0; i < aquorbium::SynthProposal::organisms; ++i)
        engine->setSynthPatch(i, aquorbium::SynthProposal::propose(i, 0x50a6u));
    engine->setSynthMix(0.6f);
    aquorbium::IntegratedModuleBank::Controls bank {};
    bank.fill(0.4f);
    bank[aquorbium::IntegratedModuleBank::mix] = 0.5f;
    engine->setIntegratedModuleBankControls(bank);
    engine->setHeadphones(true);
    engine->setOutputStageParameters({ 0.0f, -1.0f, 150.0f, false });
    aquorbium::TruePeakDetector left, right;
    left.prepare(rate);
    right.prepare(rate);
    std::vector<float> l(block), r(block);
    const auto budgetMicroseconds = 1.0e6 * static_cast<double>(block) / rate;
    double worstBlock = 0.0, quietestMinute = 0.0, loudestMinute = -200.0, totalTime = 0.0;
    long totalBlocks = 0;
    float peak = 0.0f;
    bool finite = true;
    long memoryAfterFirstMinute = 0;
    int failures = 0;
    constexpr std::array<int, 6> notes { 36, 43, 50, 55, 62, 67 };
    std::size_t blocksPerMinute = static_cast<std::size_t>(rate * 60.0) / block;
    for (int minute = 0; minute < minutes; ++minute)
    {
        double sum = 0.0;
        for (std::size_t b = 0; b < blocksPerMinute; ++b)
        {
            // MIDI notes come and go every 2.5 s
            if (b % 469 == 0)
            {
                const auto k = (minute * 24 + static_cast<int>(b / 469)) % static_cast<int>(notes.size());
                engine->noteOn(notes[static_cast<std::size_t>(k)], 0.6f);
                engine->noteOff(notes[static_cast<std::size_t>((k + 3) % static_cast<int>(notes.size()))]);
            }
            const auto t0 = threadMicroseconds();
            engine->render(l.data(), r.data(), block);
            const auto spent = threadMicroseconds() - t0;
            worstBlock = std::max(worstBlock, spent);
            totalTime += spent;
            ++totalBlocks;
            for (std::size_t i = 0; i < block; ++i)
            {
                finite = finite && std::isfinite(l[i]) && std::isfinite(r[i]);
                peak = std::max({ peak, left.processSample(l[i]), right.processSample(r[i]) });
                sum += 0.5 * (static_cast<double>(l[i]) * l[i] + static_cast<double>(r[i]) * r[i]);
            }
        }
        const auto minuteDb = 10.0 * std::log10(std::max(sum / static_cast<double>(blocksPerMinute * block), 1.0e-20));
        if (minute == 0)
        {
            memoryAfterFirstMinute = residentKilobytes();
            quietestMinute = minuteDb;
        }
        quietestMinute = std::min(quietestMinute, minuteDb);
        loudestMinute = std::max(loudestMinute, minuteDb);
        if (minute % 5 == 4)
            std::cout << "minute " << minute + 1 << ": " << minuteDb << " dBFS RMS, true peak so far "
                      << 20.0 * std::log10(std::max(peak, 1.0e-9f)) << " dBTP, worst block " << worstBlock << " us\n";
    }
    const auto growth = residentKilobytes() - memoryAfterFirstMinute;
    std::cout << "soak: " << minutes << " min; RMS per minute " << quietestMinute << " .. " << loudestMinute
              << " dBFS; true peak " << 20.0 * std::log10(std::max(peak, 1.0e-9f)) << " dBTP; worst block " << worstBlock
              << " us of " << budgetMicroseconds << " (thread CPU), mean " << totalTime / static_cast<double>(totalBlocks)
              << " us; memory growth after the first minute " << growth << " kB\n";
    const auto check = [&failures](bool ok, const char* what) {
        if (!ok)
        {
            std::cerr << "FAILED: " << what << '\n';
            ++failures;
        }
    };
    check(finite, "finite for 30 minutes");
    check(20.0 * std::log10(std::max(peak, 1.0e-9f)) <= -1.0 + 0.05, "under the -1 dB ceiling (true peak)");
    check(quietestMinute > -60.0, "no minute fades out (RMS above -60 dBFS)");
    check(loudestMinute - quietestMinute < 20.0, "no runaway: the minutes within 20 dB of each other");
    // with everything on (music, synthesis, bank, headphones, MIDI): the
    // mean inside the budget, and NO block over it. Measured: 2026-10-22,
    // before the engine flushed denormals itself, mean 48 % and worst block
    // 122 %; 2026-10-23, after, mean 31 % and worst block 52 % (thread CPU)
    check(totalTime / static_cast<double>(totalBlocks) < 0.75 * budgetMicroseconds, "the mean block under 75 % of the budget");
    check(worstBlock < budgetMicroseconds, "no block over the real-time budget (thread CPU)");
    check(growth < 1024, "no memory growth after the first minute (< 1 MB)");
    if (failures == 0)
        std::cout << "engine soak: all checks passed\n";
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
