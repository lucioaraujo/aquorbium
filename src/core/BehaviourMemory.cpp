// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/BehaviourMemory.h"

#include "core/Numeric.h"

#include <algorithm>
#include <cmath>

namespace aquorbium
{
void DriftHistoryLoop::clear() noexcept
{
    history.fill(0.0f);
    playback.fill(0.0f);
    writeIndex = 0;
    capturedCount = 0;
    playbackLength = 0;
    playbackIndex = 0;
    loops = 0;
    currentMode = Mode::live;
}

void DriftHistoryLoop::setWindow(float start, float length) noexcept
{
    windowStart = numeric::unit(start, windowStart);
    windowLength = numeric::unit(length, windowLength);
    if (currentMode == Mode::looping)
        (void)beginLoop();
}

bool DriftHistoryLoop::beginLoop() noexcept
{
    if (capturedCount < 2)
        return false;
    const auto requestedLength = 2u + static_cast<std::size_t>(std::lround(
        windowLength * static_cast<float>(capturedCount - 2u)));
    playbackLength = std::clamp<std::size_t>(requestedLength, 2, capturedCount);
    const auto maximumStart = capturedCount - playbackLength;
    const auto startOffset = static_cast<std::size_t>(std::lround(
        windowStart * static_cast<float>(maximumStart)));
    for (std::size_t index = 0; index < playbackLength; ++index)
        playback[index] = history[chronologicalIndex(startOffset + index)];
    playbackIndex = 0;
    loops = 0;
    currentMode = Mode::looping;
    return true;
}

void DriftHistoryLoop::endLoop() noexcept
{
    currentMode = Mode::live;
    playbackIndex = 0;
    loops = 0;
}

DriftHistoryLoop::Output DriftHistoryLoop::tick(float liveValue) noexcept
{
    const auto safeValue = numeric::clampFinite(liveValue, -1.0f, 1.0f, 0.0f);
    capture(safeValue);
    if (currentMode != Mode::looping || playbackLength < 2)
        return { safeValue, 0, 0, false, false };

    const auto index = playbackIndex;
    const auto value = playback[index];
    playbackIndex = (playbackIndex + 1u) % playbackLength;
    const auto wrapped = playbackIndex == 0;
    if (wrapped)
        ++loops;
    return { value, index, loops, wrapped, true };
}

void DriftHistoryLoop::capture(float value) noexcept
{
    history[writeIndex] = value;
    writeIndex = (writeIndex + 1u) % capacity;
    capturedCount = std::min(capturedCount + 1u, capacity);
}

std::size_t DriftHistoryLoop::chronologicalIndex(std::size_t offset) const noexcept
{
    const auto oldest = capturedCount < capacity ? 0u : writeIndex;
    return (oldest + offset) % capacity;
}

void MovementSignatureLock::reset() noexcept
{
    reset(Signature {});
}

void MovementSignatureLock::reset(Signature value) noexcept
{
    lockedState = false;
    update(value);
}

void MovementSignatureLock::update(Signature value) noexcept
{
    if (lockedState)
        return;
    held.smoothing = numeric::unit(value.smoothing, held.smoothing);
    held.curveFamily = numeric::unit(value.curveFamily, held.curveFamily);
    held.eventProbability = numeric::unit(
        value.eventProbability, held.eventProbability);
}

void PresenceStateLatch::reset(Ecosystem::Presence value) noexcept
{
    currentState = value;
    pendingState = value;
    pendingChange = false;
}

void PresenceStateLatch::setRelevantEvents(std::uint8_t mask) noexcept
{
    relevantMask = mask & 0x0fu;
}

void PresenceStateLatch::request(Ecosystem::Presence value) noexcept
{
    pendingState = value;
    pendingChange = pendingState != currentState;
}

bool PresenceStateLatch::trigger(Event event) noexcept
{
    const auto bit = static_cast<std::uint8_t>(event);
    if (!pendingChange || bit == 0u || (relevantMask & bit) == 0u)
        return false;
    currentState = pendingState;
    pendingChange = false;
    return true;
}

void PriorityChoke::setThreshold(float value) noexcept
{
    prominenceThreshold = numeric::unit(value, prominenceThreshold);
}

PriorityChoke::Output PriorityChoke::process(
    const std::array<float, maximumChannels>& prominence,
    std::size_t channels) const noexcept
{
    Output output;
    const auto count = std::min(channels, maximumChannels);
    for (std::size_t index = 0; index < count; ++index)
    {
        const auto value = numeric::unit(prominence[index], 0.0f);
        if (value >= prominenceThreshold)
        {
            output.selected = static_cast<int>(index);
            output.gain[index] = 1.0f;
            break;
        }
    }
    return output;
}
}
