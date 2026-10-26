// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/MotionGestureRecorder.h"

#include "core/Numeric.h"

#include <algorithm>
#include <cmath>

namespace aquorbium
{
void MotionGestureRecorder::setSeed(std::uint32_t value) noexcept
{
    seed = value == 0u ? 1u : value;
    randomState = seed;
}

void MotionGestureRecorder::clear() noexcept
{
    frameCount = 0;
    readPosition = 0.0;
    loopCount = 0;
    currentMode = Mode::idle;
    randomState = seed;
}

void MotionGestureRecorder::beginRecording() noexcept
{
    clear();
    currentMode = Mode::recording;
}

bool MotionGestureRecorder::record(Point value) noexcept
{
    if (currentMode != Mode::recording || frameCount >= maximumFrames)
        return false;
    frames[frameCount++] = sanitized(value);
    return true;
}

bool MotionGestureRecorder::endRecording() noexcept
{
    if (currentMode != Mode::recording || frameCount < 2)
    {
        currentMode = Mode::idle;
        return false;
    }
    currentMode = Mode::playback;
    rewind();
    return true;
}

void MotionGestureRecorder::setPlaybackSpeed(float value) noexcept
{
    speed = numeric::clampFinite(value, 0.125f, 8.0f, speed);
}

void MotionGestureRecorder::setMutationAmount(float value) noexcept
{
    mutation = numeric::clampFinite(value, 0.0f, 0.05f, mutation);
}

void MotionGestureRecorder::rewind() noexcept
{
    readPosition = 0.0;
    loopCount = 0;
    randomState = seed;
}

MotionGestureRecorder::Output MotionGestureRecorder::tick() noexcept
{
    if (currentMode != Mode::playback || frameCount < 2)
        return {};
    const auto index = static_cast<std::size_t>(readPosition);
    const auto next = (index + 1u) % frameCount;
    const auto fraction = static_cast<float>(
        readPosition - static_cast<double>(index));
    Point output {
        frames[index].x + (frames[next].x - frames[index].x) * fraction,
        frames[index].y + (frames[next].y - frames[index].y) * fraction
    };
    readPosition += static_cast<double>(speed);
    auto wrapped = false;
    while (readPosition >= static_cast<double>(frameCount))
    {
        readPosition -= static_cast<double>(frameCount);
        ++loopCount;
        mutateLoop();
        wrapped = true;
    }
    return { output, index, loopCount, wrapped, true };
}

MotionGestureRecorder::Point MotionGestureRecorder::point(
    std::size_t index) const noexcept
{
    return index < frameCount ? frames[index] : Point {};
}

MotionGestureRecorder::Point MotionGestureRecorder::sanitized(Point value) noexcept
{
    value.x = std::clamp(std::isfinite(value.x) ? value.x : 0.0f, -1.0f, 1.0f);
    value.y = std::clamp(std::isfinite(value.y) ? value.y : 0.0f, -1.0f, 1.0f);
    return value;
}

float MotionGestureRecorder::nextSigned() noexcept
{
    randomState = randomState * 1664525u + 1013904223u;
    return static_cast<float>((randomState >> 8u) & 0x00ffffffu)
        / 8388607.5f - 1.0f;
}

void MotionGestureRecorder::mutateLoop() noexcept
{
    if (mutation <= 0.0f)
        return;
    for (std::size_t index = 0; index < frameCount; ++index)
    {
        frames[index].x = std::clamp(
            frames[index].x + nextSigned() * mutation, -1.0f, 1.0f);
        frames[index].y = std::clamp(
            frames[index].y + nextSigned() * mutation, -1.0f, 1.0f);
    }
}
}
