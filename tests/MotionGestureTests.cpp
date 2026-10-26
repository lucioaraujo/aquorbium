// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/MotionGestureRecorder.h"
#include "core/MotionWaveBank.h"

#include <array>
#include <bit>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <utility>

namespace
{
bool waveBankIsBoundedAndDistinct()
{
    using Bank = aquorbium::MotionWaveBank;
    std::array<float, Bank::shapeCount> signatures {};
    for (std::size_t shape = 0; shape < Bank::shapeCount; ++shape)
    {
        auto signature = 0.0f;
        for (int sample = 0; sample < 257; ++sample)
        {
            const auto value = Bank::evaluate(static_cast<Bank::Shape>(shape),
                static_cast<float>(sample) / 257.0f);
            if (!std::isfinite(value) || std::abs(value) > 1.0f)
                return false;
            signature += value * static_cast<float>(sample + 1);
        }
        signatures[shape] = signature;
    }
    for (std::size_t first = 0; first < signatures.size(); ++first)
        for (std::size_t second = first + 1; second < signatures.size(); ++second)
            if (std::abs(signatures[first] - signatures[second]) < 0.01f)
                return false;
    return Bank::evaluate(Bank::Shape::sine,
        std::numeric_limits<float>::quiet_NaN()) == 0.0f;
}

bool identityMappingIsStable()
{
    using Bank = aquorbium::MotionWaveBank;
    const auto reference = Bank::shapeForIdentity(3, 7, 91u);
    if (reference != Bank::shapeForIdentity(3, 7, 91u))
        return false;
    auto alternatives = 0;
    for (std::uint32_t identity = 0; identity < 32; ++identity)
        alternatives += Bank::shapeForIdentity(identity, 7, 91u) != reference
            ? 1 : 0;
    return alternatives > 16;
}

bool gestureRecordsAndLoopsExactly()
{
    using Recorder = aquorbium::MotionGestureRecorder;
    static_assert(noexcept(std::declval<Recorder&>().tick()));
    Recorder recorder;
    recorder.beginRecording();
    const std::array<Recorder::Point, 4> points {
        Recorder::Point { -1.0f, 0.0f }, Recorder::Point { 0.0f, 1.0f },
        Recorder::Point { 1.0f, 0.0f }, Recorder::Point { 0.0f, -1.0f }
    };
    for (const auto point : points)
        if (!recorder.record(point))
            return false;
    if (!recorder.endRecording())
        return false;
    recorder.setMutationAmount(0.0f);
    for (int loop = 0; loop < 3; ++loop)
        for (const auto expected : points)
        {
            const auto output = recorder.tick();
            if (!output.active || output.point.x != expected.x
                || output.point.y != expected.y)
                return false;
        }
    return recorder.length() == points.size();
}

bool fractionalPlaybackInterpolates()
{
    aquorbium::MotionGestureRecorder recorder;
    recorder.beginRecording();
    (void)recorder.record({ -1.0f, -1.0f });
    (void)recorder.record({ 1.0f, 1.0f });
    if (!recorder.endRecording())
        return false;
    recorder.setPlaybackSpeed(0.5f);
    const auto first = recorder.tick().point;
    const auto middle = recorder.tick().point;
    recorder.setPlaybackSpeed(std::numeric_limits<float>::infinity());
    return first.x == -1.0f && middle.x == 0.0f
        && recorder.playbackSpeed() == 0.5f;
}

bool mutationIsSeededAndOptional()
{
    using Recorder = aquorbium::MotionGestureRecorder;
    Recorder first, repeated, exact;
    for (auto* recorder : { &first, &repeated, &exact })
    {
        recorder->setSeed(7712u);
        recorder->beginRecording();
        (void)recorder->record({ -0.5f, 0.25f });
        (void)recorder->record({ 0.5f, -0.25f });
        if (!recorder->endRecording())
            return false;
    }
    first.setMutationAmount(0.02f);
    repeated.setMutationAmount(0.02f);
    for (int tick = 0; tick < 20; ++tick)
    {
        const auto a = first.tick().point;
        const auto b = repeated.tick().point;
        (void)exact.tick();
        if (std::bit_cast<std::uint32_t>(a.x)
                != std::bit_cast<std::uint32_t>(b.x)
            || std::bit_cast<std::uint32_t>(a.y)
                != std::bit_cast<std::uint32_t>(b.y))
            return false;
    }
    return first.point(0).x != exact.point(0).x
        && exact.point(0).x == -0.5f && exact.point(0).y == 0.25f;
}

bool gestureSanitizesRecording()
{
    aquorbium::MotionGestureRecorder recorder;
    recorder.beginRecording();
    if (!recorder.record({ std::numeric_limits<float>::quiet_NaN(), 4.0f })
        || recorder.endRecording())
        return false;
    const auto point = recorder.point(0);
    recorder.setMutationAmount(4.0f);
    return point.x == 0.0f && point.y == 1.0f
        && recorder.mutationAmount() == 0.05f;
}
}

int main()
{
    const std::pair<const char*, bool (*)()> cases[] {
        { "wave bank", waveBankIsBoundedAndDistinct },
        { "identity map", identityMappingIsStable },
        { "gesture loop", gestureRecordsAndLoopsExactly },
        { "gesture interpolation", fractionalPlaybackInterpolates },
        { "gesture mutation", mutationIsSeededAndOptional },
        { "gesture limits", gestureSanitizesRecording },
    };
    for (const auto& [name, test] : cases)
    {
        if (!test())
        {
            std::cerr << name << " failed\n";
            return EXIT_FAILURE;
        }
    }
    std::cout << "Motion gesture tests passed\n";
}
