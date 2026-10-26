// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include <cstddef>
#include <cstdint>

namespace aquorbium
{
// Deterministic environmental noise source. Cycles choose the texture,
// water properties choose colour, and crack stress lowers temporal resolution.
class EcologicalNoiseBed final
{
public:
    struct Parameters
    {
        float seasonPhase = 0.0f;
        float dayPhase = 0.0f;
        float turbidity = 0.0f;
        float temperature = 0.5f;
        float crackStress = 0.0f;
        float level = 0.25f;
    };

    struct StereoSample
    {
        float left = 0.0f;
        float right = 0.0f;
    };

    struct Telemetry
    {
        float morphPosition = 0.0f;
        float colourCutoffHz = 0.0f;
        std::size_t holdSamples = 1;
    };

    void prepare(double sampleRate) noexcept;
    void reset() noexcept;
    void setSeed(std::uint32_t value) noexcept;
    void setParameters(Parameters value) noexcept;
    [[nodiscard]] StereoSample tick() noexcept;
    [[nodiscard]] Parameters parameters() const noexcept { return controls; }
    [[nodiscard]] Telemetry telemetry() const noexcept;

private:
    struct ChannelState
    {
        float colour = 0.0f;
        float deep = 0.0f;
        float crackle = 0.0f;
        float held = 0.0f;
    };

    [[nodiscard]] float nextSigned(std::uint32_t& state) noexcept;
    [[nodiscard]] float makeTexture(ChannelState& state,
                                    std::uint32_t& randomState) noexcept;
    void updateDerived() noexcept;

    Parameters controls {};
    double preparedSampleRate = 48000.0;
    std::uint32_t seed = 1u;
    std::uint32_t leftRandom = 1u;
    std::uint32_t rightRandom = 2u;
    ChannelState left {};
    ChannelState right {};
    float colourCoefficient = 0.1f;
    float deepCoefficient = 0.01f;
    float morphPosition = 0.0f;
    float colourCutoffHz = 1000.0f;
    std::size_t holdSamples = 1;
    std::size_t holdCounter = 0;
};
}
