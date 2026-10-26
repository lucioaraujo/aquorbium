// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/LoudnessMeter.h"

#include <cmath>
#include <cstdlib>
#include <iostream>

namespace
{
constexpr double pi = 3.14159265358979323846;

float measureSine(double sampleRate, double rmsDb, bool stereo,
                  double seconds = 5.0)
{
    aquorbium::LoudnessMeter meter;
    meter.prepare(sampleRate);
    const auto amplitude = std::pow(10.0, rmsDb / 20.0) * std::sqrt(2.0);
    const auto increment = 2.0 * pi * 1000.0 / sampleRate;
    double phase = 0.0;
    const auto samples = static_cast<std::size_t>(sampleRate * seconds);
    for (std::size_t index = 0; index < samples; ++index)
    {
        const auto value = static_cast<float>(amplitude * std::sin(phase));
        meter.push(value, stereo ? value : 0.0f);
        phase += increment;
    }
    return meter.readings().integratedLufs;
}

bool near(float value, float expected, float tolerance)
{
    return std::abs(value - expected) <= tolerance;
}

bool ebuReferenceLevel()
{
    // EBU Tech 3341 reference: identical stereo channels at -26 dBFS RMS
    // produce -23 LUFS after K-weighting and channel summation.
    return near(measureSine(48000.0, -26.0, true), -23.0f, 0.12f)
        && near(measureSine(48000.0, -23.0, false), -23.0f, 0.12f);
}

bool rateAndChannelRelations()
{
    for (const auto rate : { 44100.0, 48000.0, 96000.0 })
        if (!near(measureSine(rate, -26.0, true), -23.0f, 0.16f))
            return false;
    const auto mono = measureSine(48000.0, -23.0, false);
    const auto stereo = measureSine(48000.0, -23.0, true);
    const auto louder = measureSine(48000.0, -17.0, false);
    return near(stereo - mono, 3.0103f, 0.10f)
        && near(louder - mono, 6.0f, 0.10f);
}

bool windowsAndGating()
{
    aquorbium::LoudnessMeter meter;
    meter.prepare(48000.0);
    if (meter.readings().momentaryReady) return false;
    const auto amplitude = std::pow(10.0, -26.0 / 20.0) * std::sqrt(2.0);
    const auto increment = 2.0 * pi * 1000.0 / 48000.0;
    double phase = 0.0;
    for (std::size_t index = 0; index < 48000u * 5u; ++index)
    {
        const auto value = static_cast<float>(amplitude * std::sin(phase));
        meter.push(value, value);
        phase += increment;
    }
    const auto active = meter.readings();
    if (!active.momentaryReady || !active.shortTermReady
        || !near(active.integratedLufs, -23.0f, 0.15f))
        return false;
    for (std::size_t index = 0; index < 48000u * 10u; ++index)
        meter.push(0.0f, 0.0f);
    const auto silentTail = meter.readings();
    return silentTail.momentaryLufs <= aquorbium::LoudnessMeter::silenceLufs
        && near(silentTail.integratedLufs, -23.0f, 0.35f)
        && silentTail.integratedBlocks > active.integratedBlocks;
}

bool resetAndNonFiniteSafety()
{
    aquorbium::LoudnessMeter meter;
    meter.prepare(48000.0);
    for (std::size_t index = 0; index < 48000u; ++index)
        meter.push(index == 0 ? NAN : 0.0f,
                   index == 1 ? INFINITY : 0.0f);
    auto readings = meter.readings();
    if (!std::isfinite(readings.integratedLufs)
        || readings.integratedLufs != aquorbium::LoudnessMeter::silenceLufs)
        return false;
    meter.reset();
    readings = meter.readings();
    return readings.integratedBlocks == 0u && !readings.momentaryReady
        && readings.integratedLufs == aquorbium::LoudnessMeter::silenceLufs;
}
}

int main()
{
    if (!ebuReferenceLevel())
    { std::cerr << "EBU loudness reference failed\n"; return EXIT_FAILURE; }
    if (!rateAndChannelRelations())
    { std::cerr << "loudness rate/channel relation failed\n"; return EXIT_FAILURE; }
    if (!windowsAndGating())
    { std::cerr << "loudness windows/gating failed\n"; return EXIT_FAILURE; }
    if (!resetAndNonFiniteSafety())
    { std::cerr << "loudness reset/safety failed\n"; return EXIT_FAILURE; }
    std::cout << "Aquorbium BS.1770 loudness meter passed\n";
    return EXIT_SUCCESS;
}
