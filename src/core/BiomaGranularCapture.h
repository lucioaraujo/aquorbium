// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace aquorbium
{
// Bounded, mono-history/stereo-output sample-grain prototype. process() may
// receive a copy of the existing mix later; no live route is connected now.
class BiomaGranularCapture final
{
public:
    static constexpr std::size_t historyCapacity = 65536;
    static constexpr std::size_t grainCapacity = 12;

    struct GrainParameters
    {
        float durationMs = 45.0f;
        float speed = 1.0f;
        float pan = 0.0f;
        float gain = 0.5f;
        bool reverse = false;
    };

    struct OnsetParameters
    {
        float ratio = 3.0f;
        float floor = 0.05f;
        float refractoryMs = 90.0f;
        bool autoCapture = false;
    };

    struct Sample
    {
        float left = 0.0f;
        float right = 0.0f;
    };

    struct Telemetry
    {
        std::uint64_t onsets = 0;
        std::uint64_t triggered = 0;
        std::uint64_t rejected = 0;
        std::size_t activeGrains = 0;
    };

    void prepare(double newSampleRate) noexcept;
    void reset() noexcept;
    void setGrainParameters(GrainParameters value) noexcept;
    void setOnsetParameters(OnsetParameters value) noexcept;
    [[nodiscard]] GrainParameters grainParameters() const noexcept { return grainDefaults; }
    [[nodiscard]] OnsetParameters onsetParameters() const noexcept { return onsetControls; }
    [[nodiscard]] bool trigger(GrainParameters value) noexcept;
    [[nodiscard]] Sample process(float input) noexcept;
    [[nodiscard]] Telemetry telemetry() const noexcept;

private:
    static constexpr std::size_t windowSize = 1024;
    static constexpr std::uint32_t maximumGrainSamples = 8192;
    static constexpr std::uint32_t safetyGap = 64;
    struct Grain
    {
        double position = 0.0;
        float increment = 1.0f;
        float leftGain = 0.0f;
        float rightGain = 0.0f;
        std::uint32_t age = 0;
        std::uint32_t length = 0;
    };

    [[nodiscard]] float readHistory(double position) const noexcept;
    [[nodiscard]] float readWindow(std::uint32_t age,
                                   std::uint32_t length) const noexcept;
    static GrainParameters sanitize(GrainParameters value,
                                    GrainParameters fallback) noexcept;
    static void count(std::uint64_t& value) noexcept;

    std::array<float, historyCapacity> history {};
    std::array<float, windowSize + 1> window {};
    std::array<Grain, grainCapacity> grains {};
    GrainParameters grainDefaults {};
    OnsetParameters onsetControls {};
    double sampleRate = 48000.0;
    std::uint64_t written = 0;
    float fastEnvelope = 0.0f;
    float slowEnvelope = 0.0f;
    float fastGain = 0.0f;
    float slowGain = 0.0f;
    std::uint32_t refractorySamples = 0;
    bool onsetArmed = true;
    std::uint64_t onsetCount = 0;
    std::uint64_t triggeredCount = 0;
    std::uint64_t rejectedCount = 0;
};
}
