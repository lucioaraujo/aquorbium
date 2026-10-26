// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include "core/Ecosystem.h"

#include <array>
#include <cstddef>
#include <cstdint>

namespace aquorbium
{
class DriftHistoryLoop final
{
public:
    static constexpr std::size_t capacity = 4096;

    enum class Mode : std::uint8_t { live, looping };

    struct Output
    {
        float value = 0.0f;
        std::size_t loopIndex = 0;
        std::uint64_t loopCount = 0;
        bool wrapped = false;
        bool looping = false;
    };

    void clear() noexcept;
    void setWindow(float start, float length) noexcept;
    [[nodiscard]] bool beginLoop() noexcept;
    void endLoop() noexcept;
    [[nodiscard]] Output tick(float liveValue) noexcept;
    [[nodiscard]] Mode mode() const noexcept { return currentMode; }
    [[nodiscard]] std::size_t captured() const noexcept { return capturedCount; }
    [[nodiscard]] std::size_t loopLength() const noexcept { return playbackLength; }

private:
    void capture(float value) noexcept;
    [[nodiscard]] std::size_t chronologicalIndex(std::size_t offset) const noexcept;

    std::array<float, capacity> history {};
    std::array<float, capacity> playback {};
    std::size_t writeIndex = 0;
    std::size_t capturedCount = 0;
    std::size_t playbackLength = 0;
    std::size_t playbackIndex = 0;
    std::uint64_t loops = 0;
    float windowStart = 0.0f;
    float windowLength = 1.0f;
    Mode currentMode = Mode::live;
};

class MovementSignatureLock final
{
public:
    struct Signature
    {
        float smoothing = 0.5f;
        float curveFamily = 0.5f;
        float eventProbability = 0.5f;
    };

    void reset() noexcept;
    void reset(Signature value) noexcept;
    void setLocked(bool value) noexcept { lockedState = value; }
    void update(Signature value) noexcept;
    [[nodiscard]] Signature signature() const noexcept { return held; }
    [[nodiscard]] bool locked() const noexcept { return lockedState; }

private:
    Signature held {};
    bool lockedState = false;
};

class PresenceStateLatch final
{
public:
    enum class Event : std::uint8_t
    {
        none = 0,
        movement = 1,
        danger = 2,
        mating = 4,
        integrity = 8
    };

    void reset(Ecosystem::Presence value = Ecosystem::Presence::Present) noexcept;
    void setRelevantEvents(std::uint8_t mask) noexcept;
    void request(Ecosystem::Presence value) noexcept;
    [[nodiscard]] bool trigger(Event event) noexcept;
    [[nodiscard]] Ecosystem::Presence current() const noexcept { return currentState; }
    [[nodiscard]] Ecosystem::Presence pending() const noexcept { return pendingState; }
    [[nodiscard]] bool hasPending() const noexcept { return pendingChange; }

private:
    Ecosystem::Presence currentState = Ecosystem::Presence::Present;
    Ecosystem::Presence pendingState = Ecosystem::Presence::Present;
    std::uint8_t relevantMask = 0x0fu;
    bool pendingChange = false;
};

class PriorityChoke final
{
public:
    static constexpr std::size_t maximumChannels = 8;

    struct Output
    {
        std::array<float, maximumChannels> gain {};
        int selected = -1;
    };

    void setThreshold(float value) noexcept;
    [[nodiscard]] Output process(
        const std::array<float, maximumChannels>& prominence,
        std::size_t channels) const noexcept;
    [[nodiscard]] float threshold() const noexcept { return prominenceThreshold; }

private:
    float prominenceThreshold = 0.5f;
};
}
