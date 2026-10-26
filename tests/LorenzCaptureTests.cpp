// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/BiomaGranularCapture.h"
#include "core/LorenzCV.h"

#include <algorithm>
#include <bit>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <utility>

namespace
{
using Capture = aquorbium::BiomaGranularCapture;
using Lorenz = aquorbium::LorenzCV;

bool lorenzIsBoundedAndRepeatable()
{
    static_assert(noexcept(std::declval<Lorenz&>().tick()));
    Lorenz first, repeat, other;
    for (auto* control : { &first, &repeat, &other })
        control->prepare(48000.0);
    first.setSeed(0x12345678u);
    repeat.setSeed(0x12345678u);
    other.setSeed(0x9abcdef0u);
    bool diverged = false;
    Lorenz::Output previous;
    for (int sample = 0; sample < 48000 * 8; ++sample)
    {
        const auto value = first.tick();
        const auto twin = repeat.tick();
        const auto independent = other.tick();
        if (!std::isfinite(value.x) || !std::isfinite(value.y)
            || !std::isfinite(value.z) || std::abs(value.x) > 1.0f
            || std::abs(value.y) > 1.0f || std::abs(value.z) > 1.0f
            || std::abs(value.x - previous.x) > 0.002f
            || std::abs(value.y - previous.y) > 0.002f
            || std::abs(value.z - previous.z) > 0.002f)
            return false;
        if (std::bit_cast<std::uint32_t>(value.x)
                != std::bit_cast<std::uint32_t>(twin.x)
            || std::bit_cast<std::uint32_t>(value.y)
                != std::bit_cast<std::uint32_t>(twin.y)
            || std::bit_cast<std::uint32_t>(value.z)
                != std::bit_cast<std::uint32_t>(twin.z)) return false;
        diverged = diverged || std::abs(value.x - independent.x) > 0.0001f;
        previous = value;
    }
    return diverged && first.controlSteps() >= 1599
        && first.controlSteps() <= 1601 && first.recoveries() == 0;
}

bool lorenzRejectsInvalidControls()
{
    Lorenz control;
    control.prepare(std::numeric_limits<double>::quiet_NaN());
    control.setParameters({ -1.0f, 200.0f,
        std::numeric_limits<float>::infinity(), 10.0f });
    const auto parameters = control.parameters();
    if (parameters.sigma != 0.1f || parameters.rho != 60.0f
        || parameters.beta != 2.6666667f || parameters.dT != 0.02f)
        return false;
    for (int sample = 0; sample < 48000 * 3; ++sample)
    {
        const auto value = control.tick();
        if (!std::isfinite(value.x) || !std::isfinite(value.y)
            || !std::isfinite(value.z)) return false;
    }
    return control.recoveries() == 0;
}

void fillTone(Capture& capture, int length = 10000)
{
    for (int sample = 0; sample < length; ++sample)
    {
        const auto input = static_cast<float>(0.4 * std::sin(
            6.283185307179586 * 221.0 * sample / 48000.0));
        (void)capture.process(input);
    }
}

bool granularBoundsAndVoiceLimit()
{
    static_assert(noexcept(std::declval<Capture&>().process(0.0f)));
    Capture capture;
    capture.prepare(48000.0);
    for (int sample = 0; sample < 10000; ++sample)
    {
        const auto value = capture.process(0.0f);
        if (value.left != 0.0f || value.right != 0.0f) return false;
    }
    Capture::GrainParameters grain { 20.0f, 2.0f, 0.0f, 1.0f, false };
    for (std::size_t index = 0; index < Capture::grainCapacity; ++index)
        if (!capture.trigger(grain)) return false;
    if (capture.trigger(grain)) return false;
    if (capture.telemetry().activeGrains != Capture::grainCapacity
        || capture.telemetry().rejected != 1) return false;
    for (int sample = 0; sample < 2000; ++sample)
    {
        const auto value = capture.process(0.0f);
        if (value.left != 0.0f || value.right != 0.0f) return false;
    }
    return capture.telemetry().activeGrains == 0;
}

bool granularSnapshotsAndDirection()
{
    Capture first, repeat, reversed, slower;
    for (auto* capture : { &first, &repeat, &reversed, &slower })
    {
        capture->prepare(48000.0);
        fillTone(*capture);
    }
    Capture::GrainParameters forward { 30.0f, 1.25f, -1.0f, 1.0f, false };
    auto back = forward;
    back.reverse = true;
    back.pan = 1.0f;
    auto slow = forward;
    slow.speed = 0.5f;
    if (!first.trigger(forward) || !repeat.trigger(forward)
        || !reversed.trigger(back) || !slower.trigger(slow)) return false;
    first.setGrainParameters({ 80.0f, 0.5f, 1.0f, 0.1f, true });
    bool directionDiffers = false;
    bool speedDiffers = false;
    bool audible = false;
    for (int sample = 0; sample < 1700; ++sample)
    {
        const auto left = first.process(0.0f);
        const auto twin = repeat.process(0.0f);
        const auto right = reversed.process(0.0f);
        const auto slowOutput = slower.process(0.0f);
        if (std::bit_cast<std::uint32_t>(left.left)
                != std::bit_cast<std::uint32_t>(twin.left)
            || std::abs(left.left) > 0.067f
            || std::abs(right.right) > 0.067f
            || !std::isfinite(left.left) || !std::isfinite(right.right))
            return false;
        audible = audible || std::abs(left.left) > 0.001f;
        directionDiffers = directionDiffers
            || std::abs(left.left - right.right) > 0.001f;
        speedDiffers = speedDiffers
            || std::abs(left.left - slowOutput.left) > 0.001f;
    }
    return audible && directionDiffers && speedDiffers
        && first.telemetry().activeGrains == 0
        && reversed.telemetry().activeGrains == 0;
}

bool granularHistoryWrapsSafely()
{
    Capture capture;
    capture.prepare(96000.0);
    for (int sample = 0; sample < 150000; ++sample)
        (void)capture.process(sample % 100 < 50 ? 0.2f : -0.2f);
    if (!capture.trigger({ 150.0f, 2.0f, 0.0f, 1.0f, true }))
        return false;
    for (int sample = 0; sample < 9000; ++sample)
    {
        const auto value = capture.process(0.0f);
        if (!std::isfinite(value.left) || !std::isfinite(value.right)
            || std::abs(value.left) > 0.067f
            || std::abs(value.right) > 0.067f) return false;
    }
    return capture.telemetry().activeGrains == 0;
}

bool granularOnsetAndInvalidInput()
{
    Capture capture;
    capture.prepare(48000.0);
    fillTone(capture, 12000);
    const auto previous = capture.telemetry().onsets;
    capture.setOnsetParameters({ 2.0f, 0.04f, 80.0f, true });
    capture.setGrainParameters({ 20.0f, 1.0f, 0.0f, 0.8f, false });
    for (int sample = 0; sample < 400; ++sample)
        (void)capture.process(1.0f);
    const auto telemetry = capture.telemetry();
    if (telemetry.onsets <= previous || telemetry.triggered == 0)
        return false;
    capture.setGrainParameters({ std::numeric_limits<float>::infinity(),
        -1.0f, 3.0f, std::numeric_limits<float>::quiet_NaN(), true });
    const auto grain = capture.grainParameters();
    if (grain.durationMs != 20.0f || grain.speed != 0.5f
        || grain.pan != 1.0f || grain.gain != 0.8f) return false;
    for (int sample = 0; sample < 3000; ++sample)
    {
        const auto value = capture.process(std::numeric_limits<float>::quiet_NaN());
        if (!std::isfinite(value.left) || !std::isfinite(value.right)
            || std::abs(value.left) > 0.8f || std::abs(value.right) > 0.8f)
            return false;
    }
    return true;
}
}

int main()
{
    const std::pair<const char*, bool (*)()> cases[] {
        { "lorenz repeatability", lorenzIsBoundedAndRepeatable },
        { "lorenz invalid controls", lorenzRejectsInvalidControls },
        { "granular bounds", granularBoundsAndVoiceLimit },
        { "granular snapshot/direction", granularSnapshotsAndDirection },
        { "granular history wrap", granularHistoryWrapsSafely },
        { "granular onset/input", granularOnsetAndInvalidInput },
    };
    for (const auto& [name, test] : cases)
    {
        if (!test())
        {
            std::cerr << name << " failed\n";
            return 1;
        }
    }
    std::cout << "Lorenz CV and bounded granular capture passed\n";
}
