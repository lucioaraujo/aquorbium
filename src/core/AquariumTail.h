// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include <array>
#include <cstddef>

namespace aquorbium
{
// Wet-only synthetic tail based on a bounded four-line feedback delay network.
// It is a texture generator, not a model of a named physical room.
class AquariumTail final
{
public:
    static constexpr std::size_t lineCount = 4;
    static constexpr std::size_t lineCapacity = 32768;

    struct Parameters
    {
        float size = 0.55f;
        float decaySeconds = 4.0f;
        float damping = 0.55f;
        float diffusion = 0.75f;
        float nightmare = 0.0f;
        float level = 0.35f;
    };

    struct StereoSample
    {
        float left = 0.0f;
        float right = 0.0f;
    };

    struct Telemetry
    {
        std::array<std::size_t, lineCount> delaySamples {};
        std::array<float, lineCount> feedback {};
        float energy = 0.0f;
    };

    void prepare(double newSampleRate) noexcept;
    void reset() noexcept;
    void setParameters(Parameters value) noexcept;
    [[nodiscard]] Parameters parameters() const noexcept { return controls; }
    [[nodiscard]] StereoSample tick(float inputLeft, float inputRight) noexcept;
    [[nodiscard]] Telemetry telemetry() const noexcept;

private:
    void updateCoefficients() noexcept;

    std::array<std::array<float, lineCapacity>, lineCount> delay {};
    std::array<std::size_t, lineCount> writeIndex {};
    std::array<std::size_t, lineCount> delayLength {};
    std::array<float, lineCount> lowpassState {};
    std::array<float, lineCount> feedbackGain {};
    Parameters controls {};
    double sampleRate = 48000.0;
    float dampingPole = 0.5f;
    float currentEnergy = 0.0f;
};
}
