// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace aquorbium
{
class MasterChain final
{
public:
    static constexpr std::size_t bandCount = 3;

    struct Parameters
    {
        float influence = 0.0f;
        float multiband = 0.5f;
        float glue = 0.35f;
        float tilt = 0.0f;
        float width = 1.0f;
        float crackStress = 0.0f;
        bool eqAfterDynamics = false;
    };

    struct Sample
    {
        float left = 0.0f;
        float right = 0.0f;
    };

    struct Telemetry
    {
        std::array<float, bandCount> bandGainReductionDb {};
        float glueGainReductionDb = 0.0f;
        float currentInfluence = 0.0f;
        float effectiveWidth = 1.0f;
        float inputPeak = 0.0f;
        float outputPeak = 0.0f;
        std::uint64_t invalidSamples = 0;
    };

    void prepare(double sampleRate) noexcept;
    void reset() noexcept;
    void setParameters(Parameters value) noexcept;
    [[nodiscard]] Parameters parameters() const noexcept { return controls; }
    [[nodiscard]] Sample process(float left, float right) noexcept;
    [[nodiscard]] Telemetry telemetry() const noexcept;

private:
    struct Biquad
    {
        float b0 = 1.0f, b1 = 0.0f, b2 = 0.0f;
        float a1 = 0.0f, a2 = 0.0f;
        float z1 = 0.0f, z2 = 0.0f;
        [[nodiscard]] float tick(float input) noexcept;
        void reset() noexcept { z1 = z2 = 0.0f; }
    };

    struct BandState
    {
        float envelope = 0.0f;
        float gain = 1.0f;
        float attack = 0.01f;
        float release = 0.001f;
    };

    struct Bands
    {
        std::array<float, bandCount> value {};
    };

    void updateFilters() noexcept;
    void setFilter(Biquad& filter, float frequency, bool highpass) noexcept;
    [[nodiscard]] Bands split(std::size_t channel, float input) noexcept;
    [[nodiscard]] float processBand(std::size_t band, float left, float right,
                                    float& outputLeft, float& outputRight) noexcept;
    [[nodiscard]] float processGlue(float detector) noexcept;
    static void count(std::uint64_t& value) noexcept;

    Parameters controls {};
    double sampleRate = 48000.0;
    float targetInfluence = 0.0f;
    float currentInfluence = 0.0f;
    float influenceSmoothing = 0.001f;
    float lowCrossoverHz = 180.0f;
    float highCrossoverHz = 2600.0f;
    // channel, split (low/high), response (LP/HP), LR stage
    std::array<std::array<std::array<std::array<Biquad, 2>, 2>, 2>, 2> filters {};
    std::array<BandState, bandCount> bandState {};
    float glueEnvelope = 0.0f;
    float glueGain = 1.0f;
    float glueAttack = 0.001f;
    float glueRelease = 0.0001f;
    float inputPeak = 0.0f;
    float outputPeak = 0.0f;
    float effectiveWidth = 1.0f;
    std::uint64_t invalidSampleCount = 0;
};
}
