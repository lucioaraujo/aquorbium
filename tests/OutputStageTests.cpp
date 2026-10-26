// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/OutputStage.h"
#include "core/TruePeakDetector.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>

namespace
{
bool latencyAndSilence()
{
    aquorbium::OutputStage stage;
    stage.prepare(8000.0);
    if (stage.latencySamples() != 40u) return false;
    stage.prepare(768000.0);
    if (stage.latencySamples() != 3840u) return false;
    stage.prepare(48000.0);
    const auto latency = stage.latencySamples();
    if (latency != 240u) return false;
    for (std::uint32_t sample = 0; sample < latency; ++sample)
    {
        const auto output = stage.process(sample == 0 ? 0.5f : 0.0f, 0.0f);
        if (output.left != 0.0f || output.right != 0.0f) return false;
    }
    const auto delayed = stage.process(0.0f, 0.0f);
    return delayed.left > 0.45f && delayed.left < 0.51f
        && delayed.right == 0.0f;
}

bool linkedLimiterAndMeters()
{
    aquorbium::OutputStage stage;
    stage.prepare(48000.0);
    stage.setParameters({ 6.0f, -6.0f, 200.0f, false });
    const auto frames = stage.latencySamples() + 12000u;
    float peak = 0.0f;
    bool linked = false;
    for (std::uint32_t sample = 0; sample < frames; ++sample)
    {
        const auto phase = 6.283185307179586 * 997.0 * sample / 48000.0;
        const auto left = static_cast<float>(1.2 * std::sin(phase));
        const auto right = static_cast<float>(0.2 * std::sin(phase));
        const auto output = stage.process(left, right);
        peak = std::max(peak, std::max(std::abs(output.left),
                                      std::abs(output.right)));
        if (sample > stage.latencySamples() + 100u
            && std::abs(output.right) < 0.20f) linked = true;
    }
    const auto telemetry = stage.telemetry();
    return peak <= 0.505f && telemetry.estimatedTruePeak <= 0.51f
        && telemetry.inputSamplePeak > 1.0f
        && telemetry.gainReductionDb > 1.0f && telemetry.rms > 0.0f
        && !telemetry.clipLatched && linked;
}

bool dcAndInvalidProtection()
{
    aquorbium::OutputStage stage;
    stage.prepare(48000.0);
    for (int sample = 0; sample < 96000; ++sample)
        (void)stage.process(0.5f, -0.5f);
    auto output = stage.process(std::numeric_limits<float>::quiet_NaN(),
                                std::numeric_limits<float>::infinity());
    if (!std::isfinite(output.left) || !std::isfinite(output.right)) return false;
    for (int sample = 0; sample < 48000; ++sample)
        output = stage.process(0.0f, 0.0f);
    const auto telemetry = stage.telemetry();
    return std::abs(output.left) < 1.0e-5f && std::abs(output.right) < 1.0e-5f
        && telemetry.invalidSamples == 2u;
}

bool controlsAreBoundedAndMuteIsSmooth()
{
    aquorbium::OutputStage stage;
    stage.prepare(std::numeric_limits<double>::quiet_NaN());
    stage.setParameters({ std::numeric_limits<float>::infinity(), 2.0f,
                          -10.0f, false });
    const auto parameters = stage.parameters();
    if (parameters.trimDb != 0.0f || parameters.ceilingDb != -0.1f
        || parameters.releaseMs != 50.0f || parameters.muted)
        return false;

    // Fill the lookahead line first so its normal signal onset is not
    // mistaken for a discontinuity introduced by the mute ramp.
    float previous = 0.0f;
    for (int sample = 0; sample < 2400; ++sample)
        previous = stage.process(0.2f, 0.2f).left;
    stage.setParameters({ parameters.trimDb, parameters.ceilingDb,
                          parameters.releaseMs, true });
    float maximumDelta = 0.0f;
    for (int sample = 0; sample < 4800; ++sample)
    {
        const auto output = stage.process(0.2f, 0.2f);
        maximumDelta = std::max(maximumDelta, std::abs(output.left - previous));
        previous = output.left;
    }
    return maximumDelta < 0.01f && std::abs(previous) < 0.001f;
}

bool loudnessIsPostSafetyAndResettable()
{
    aquorbium::OutputStage stage;
    stage.prepare(48000.0);
    const auto amplitude = std::pow(10.0, -26.0 / 20.0) * std::sqrt(2.0);
    for (std::uint32_t sample = 0; sample < 48000u * 4u; ++sample)
    {
        const auto phase = 6.283185307179586 * 1000.0 * sample / 48000.0;
        const auto value = static_cast<float>(amplitude * std::sin(phase));
        (void)stage.process(value, value);
    }
    const auto measured = stage.telemetry();
    if (!measured.momentaryLoudnessReady
        || !measured.shortTermLoudnessReady
        || std::abs(measured.integratedLufs + 23.0f) > 0.20f
        || measured.loudnessBlocks == 0u)
        return false;
    stage.clearMeters();
    const auto cleared = stage.telemetry();
    return cleared.integratedLufs == aquorbium::LoudnessMeter::silenceLufs
        && cleared.loudnessBlocks == 0u
        && !cleared.momentaryLoudnessReady;
}
}

// 2026-10-22: the TRUE peak (an independent 4x detector) stays under the
// ceiling, intersample peaks included - the 4-sample estimate alone let
// +0.9 dBTP through a -0.1 dB ceiling
bool truePeakUnderTheCeiling()
{
    constexpr double pi = 3.14159265358979323846;
    const std::array<float (*)(int), 4> signals {
        // a sine at fs/4 shifted 45 degrees: its samples sit 3 dB under its peak
        [](int n) { return 2.0f * static_cast<float>(std::sin(pi / 2.0 * n + pi / 4.0)); },
        [](int n) { return ((n / 4800) % 2 ? 2.0f : 0.05f) * static_cast<float>(std::sin(pi / 2.0 * n + pi / 4.0)); },
        [](int n) { return ((n / 3000) % 2 ? 2.0f : 0.1f) * ((n % 2) ? 0.9f : -0.9f); },
        [](int n) { return ((n / 8) % 2) ? 2.0f : -2.0f; } };
    for (const auto ceiling : { -1.0f, -0.1f })
        for (const auto& signal : signals)
        {
            aquorbium::OutputStage stage;
            stage.prepare(48000.0);
            stage.setParameters({ 0.0f, ceiling, 150.0f, false });
            aquorbium::TruePeakDetector detector;
            detector.prepare(48000.0);
            float truePeak = 0.0f;
            for (int n = 0; n < 48000 * 2; ++n)
            {
                const auto x = signal(n);
                const auto peak = detector.processSample(stage.process(x, x).left);
                if (n > 24000)
                    truePeak = std::max(truePeak, peak);
            }
            const auto db = 20.0f * std::log10(truePeak);
            if (db > ceiling + 0.02f)
            {
                std::cerr << "true peak " << db << " dBTP over a " << ceiling << " dB ceiling\n";
                return false;
            }
            if (20.0f * std::log10(stage.telemetry().estimatedTruePeak) < db - 0.1f)
            {
                std::cerr << "the meter's true peak reads under the real one\n";
                return false;
            }
        }
    return true;
}

// 2026-10-22: after one big spike the limiter lets a steady overload come
// back to the ceiling (it used to stay 12 dB down for as long as it lasted)
bool limiterRecoversAfterASpike()
{
    constexpr double pi = 3.14159265358979323846;
    aquorbium::OutputStage stage;
    stage.prepare(48000.0);
    stage.setParameters({ 0.0f, -1.0f, 150.0f, false });
    float lastSecond = 0.0f;
    for (int n = 0; n < 48000 * 4; ++n)
    {
        auto x = 2.0f * static_cast<float>(std::sin(2.0 * pi * 1000.0 * n / 48000.0));
        if (n == 48000)
            x = 8.0f;  // +18 dBFS, once
        const auto y = stage.process(x, x).left;
        if (n >= 48000 * 3)
            lastSecond = std::max(lastSecond, std::abs(y));
    }
    const auto db = 20.0f * std::log10(lastSecond);
    if (std::abs(db - (-1.0f)) > 0.2f)
    {
        std::cerr << "after a spike the steady signal sits at " << db << " dBFS, not at the -1 dB ceiling\n";
        return false;
    }
    return true;
}

// 2026-10-22: the correlation meter: +1 for mono, -1 for opposite channels,
// ~0 for unrelated ones, 0 in silence
bool correlationMeter()
{
    const auto measure = [](float (*right)(float, int)) {
        aquorbium::OutputStage stage;
        stage.prepare(48000.0);
        stage.setParameters({ 0.0f, -1.0f, 150.0f, false });
        std::uint32_t state = 1u;
        for (int n = 0; n < 48000 * 2; ++n)
        {
            state ^= state << 13; state ^= state >> 17; state ^= state << 5;
            const auto l = 0.2f * static_cast<float>(std::sin(n * 0.031));
            const auto noise = static_cast<float>(state) / 4294967296.0f - 0.5f;
            static_cast<void>(stage.process(l, right(l, static_cast<int>(noise * 1.0e6f))));
        }
        return stage.telemetry().correlation;
    };
    const auto mono = measure([](float l, int) { return l; });
    const auto opposite = measure([](float l, int) { return -l; });
    const auto unrelated = measure([](float, int noise) { return static_cast<float>(noise) * 1.0e-7f; });
    const auto silent = measure([](float, int) { return 0.0f; });
    std::cout << "correlation: mono " << mono << ", opposite " << opposite << ", unrelated " << unrelated
              << ", one side silent " << silent << '\n';
    return mono > 0.99f && opposite < -0.99f && std::abs(unrelated) < 0.1f && silent == 0.0f;
}

int main()
{
    if (!latencyAndSilence())
    { std::cerr << "output latency failed\n"; return EXIT_FAILURE; }
    if (!linkedLimiterAndMeters())
    { std::cerr << "linked limiter failed\n"; return EXIT_FAILURE; }
    if (!dcAndInvalidProtection())
    { std::cerr << "output protection failed\n"; return EXIT_FAILURE; }
    if (!controlsAreBoundedAndMuteIsSmooth())
    { std::cerr << "output controls failed\n"; return EXIT_FAILURE; }
    if (!truePeakUnderTheCeiling())
    { std::cerr << "true peak ceiling failed\n"; return EXIT_FAILURE; }
    if (!limiterRecoversAfterASpike())
    { std::cerr << "limiter recovery failed\n"; return EXIT_FAILURE; }
    if (!correlationMeter())
    { std::cerr << "correlation meter failed\n"; return EXIT_FAILURE; }
    if (!loudnessIsPostSafetyAndResettable())
    { std::cerr << "output loudness failed\n"; return EXIT_FAILURE; }
    std::cout << "Aquorbium output safety stage passed\n";
    return EXIT_SUCCESS;
}
