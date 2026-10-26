// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/RoomCoupling.h"

#include <algorithm>
#include <cmath>

namespace aquorbium
{
namespace
{
constexpr double pi = 3.14159265358979323846;
float onePole(double hz, double rate) noexcept
{
    return static_cast<float>(1.0 - std::exp(-2.0 * pi * hz / rate));
}
}

void RoomCoupling::prepare(double sampleRate) noexcept
{
    rate = std::isfinite(sampleRate) && sampleRate >= 8000.0 ? sampleRate : 48000.0;
    lowCoefficient = onePole(300.0, rate);
    midCoefficient = onePole(2000.0, rate);
    powerCoefficient = static_cast<float>(1.0 - std::exp(-1.0 / (0.3 * rate)));
    interval = static_cast<std::size_t>(rate / 100.0);
    momentCoefficient = 1.0 - std::exp(-1.0 / (10.0 * 100.0));
    reset();
}

void RoomCoupling::reset() noexcept
{
    mic = {};
    out = {};
    meanX = meanY = meanXX = meanXY = 0.0;
    weight = 0.0;
    coupling2 = 0.0f;
    learnedUpdates = 0;
    settledUpdates = 0;
    countdown = interval;
    last = {};
}

void RoomCoupling::split(Bands& bands, float x) noexcept
{
    if (!std::isfinite(x))
        x = 0.0f;
    bands.lowState += (x - bands.lowState) * lowCoefficient;
    bands.midState += (x - bands.midState) * midCoefficient;
    const auto low = bands.lowState;
    const auto middle = bands.midState - bands.lowState;
    const auto high = x - bands.midState;
    bands.power[0] += (low * low - bands.power[0]) * powerCoefficient;
    bands.power[1] += (middle * middle - bands.power[1]) * powerCoefficient;
    bands.power[2] += (high * high - bands.power[2]) * powerCoefficient;
}

void RoomCoupling::push(float microphone, float output) noexcept
{
    split(mic, microphone);
    split(out, output);
    if (--countdown == 0)
    {
        countdown = interval;
        update();
    }
}

void RoomCoupling::update() noexcept
{
    const auto micPower = mic.power[0] + mic.power[1] + mic.power[2];
    const auto outPower = out.power[0] + out.power[1] + out.power[2];
    // the regression of microphone power on output power: the slope is the
    // speaker-to-microphone coupling k^2
    const auto a = momentCoefficient;
    meanX += (outPower - meanX) * a;
    meanY += (micPower - meanY) * a;
    meanXX += (static_cast<double>(outPower) * outPower - meanXX) * a;
    meanXY += (static_cast<double>(outPower) * micPower - meanXY) * a;
    // moments that started at zero are all scaled by the same weight; divide
    // it out, or a steady room sound would bias the slope upwards
    weight += (1.0 - weight) * a;
    const auto ex = meanX / weight, ey = meanY / weight, exx = meanXX / weight, exy = meanXY / weight;
    const auto variance = exx - ex * ex;
    if (variance > 1.0e-12 && variance > 0.01 * ex * ex)
    {
        coupling2 = static_cast<float>(std::max(0.0, (exy - ex * ey) / variance));
        ++learnedUpdates;
    }
    const auto k2 = coupling2;
    const auto coupling = std::sqrt(k2);
    std::array<float, 3> excess {};
    float total = 0.0f;
    for (std::size_t b = 0; b < 3; ++b)
    {
        // a little margin over the learned coupling, so that the aquarium's
        // own sound is not taken for the room
        excess[b] = std::max(0.0f, mic.power[b] - 1.2f * k2 * out.power[b]);
        total += excess[b];
    }
    Reading r;
    r.coupling = coupling;
    r.levelDb = 10.0f * std::log10(std::max(total, 1.0e-12f));
    // ~3 s of learning while the aquarium sounds. While it is silent (paused,
    // muted) there is nothing to learn and nothing to subtract: everything
    // the microphone hears is the room, as soon as its powers settle (~0.5 s).
    // Before 2026-10-22 a silent aquarium never heard a loud room (found by
    // the engine stress tests).
    constexpr float silentOutput = 1.0e-8f;  // -80 dBFS
    settledUpdates = outPower < silentOutput ? settledUpdates + 1 : 0;
    r.active = (learnedUpdates >= 300 || settledUpdates >= 50) && r.levelDb > floorDb;
    if (!r.active)
        r.levelDb = std::min(r.levelDb, floorDb);
    r.strength = r.active ? std::clamp((r.levelDb - floorDb) / 40.0f, 0.0f, 1.0f) : 0.0f;
    if (total > 0.0f)
    {
        // the centroid of the bands' centres (150 Hz, 775 Hz, 4 kHz), in MIDI
        constexpr std::array<float, 3> centreMidi { 50.2f, 78.6f, 107.3f };
        float m = 0.0f;
        for (std::size_t b = 0; b < 3; ++b)
            m += centreMidi[b] * excess[b];
        r.midi = std::clamp(m / total, 36.0f, 96.0f);
    }
    last = r;
}
}
