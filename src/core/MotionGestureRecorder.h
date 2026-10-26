// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace aquorbium
{
// Fixed two-dimensional gesture recorder with exact or slowly mutating loop
// playback. Recording and playback are both allocation-free.
class MotionGestureRecorder final
{
public:
    static constexpr std::size_t maximumFrames = 2048;

    struct Point
    {
        float x = 0.0f;
        float y = 0.0f;
    };

    enum class Mode
    {
        idle,
        recording,
        playback
    };

    struct Output
    {
        Point point {};
        std::size_t index = 0;
        std::uint64_t loop = 0;
        bool wrapped = false;
        bool active = false;
    };

    void setSeed(std::uint32_t value) noexcept;
    void clear() noexcept;
    void beginRecording() noexcept;
    [[nodiscard]] bool record(Point value) noexcept;
    [[nodiscard]] bool endRecording() noexcept;
    void setPlaybackSpeed(float value) noexcept;
    void setMutationAmount(float value) noexcept;
    void rewind() noexcept;
    [[nodiscard]] Output tick() noexcept;

    [[nodiscard]] Mode mode() const noexcept { return currentMode; }
    [[nodiscard]] std::size_t length() const noexcept { return frameCount; }
    [[nodiscard]] float playbackSpeed() const noexcept { return speed; }
    [[nodiscard]] float mutationAmount() const noexcept { return mutation; }
    [[nodiscard]] Point point(std::size_t index) const noexcept;

private:
    [[nodiscard]] static Point sanitized(Point value) noexcept;
    [[nodiscard]] float nextSigned() noexcept;
    void mutateLoop() noexcept;

    std::array<Point, maximumFrames> frames {};
    std::size_t frameCount = 0;
    double readPosition = 0.0;
    float speed = 1.0f;
    float mutation = 0.0f;
    std::uint32_t seed = 1u;
    std::uint32_t randomState = 1u;
    std::uint64_t loopCount = 0;
    Mode currentMode = Mode::idle;
};
}
