// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace aquorbium
{
// Eight-band resonant signature for a school. Centre/span describe the shared
// spectral territory while identity changes only the fixed gain profile.
class SchoolSpectralBank final
{
public:
    static constexpr std::size_t bandCount = 8;

    struct Parameters
    {
        float centre = 0.5f;
        float span = 0.5f;
        float resonance = 0.45f;
    };

    struct Output
    {
        std::array<float, bandCount> band {};
        float mix = 0.0f;
    };

    struct Telemetry
    {
        std::array<float, bandCount> frequencyHz {};
        std::array<float, bandCount> gain {};
    };

    void prepare(double sampleRate) noexcept;
    void reset() noexcept;
    void setParameters(Parameters value) noexcept;
    void setIdentity(std::uint32_t schoolId, std::uint32_t seed) noexcept;
    [[nodiscard]] Output tick(float input) noexcept;
    [[nodiscard]] Parameters parameters() const noexcept { return controls; }
    [[nodiscard]] Telemetry telemetry() const noexcept;

private:
    struct Biquad
    {
        float b0 = 0.0f;
        float b1 = 0.0f;
        float b2 = 0.0f;
        float a1 = 0.0f;
        float a2 = 0.0f;
        float z1 = 0.0f;
        float z2 = 0.0f;

        [[nodiscard]] float tick(float input) noexcept;
        void reset() noexcept { z1 = z2 = 0.0f; }
    };

    void updateCoefficients() noexcept;

    std::array<Biquad, bandCount> filters {};
    std::array<float, bandCount> frequencies {};
    std::array<float, bandCount> gains { 1.0f, 1.0f, 1.0f, 1.0f,
                                         1.0f, 1.0f, 1.0f, 1.0f };
    Parameters controls {};
    double preparedSampleRate = 48000.0;
    float gainNormalization = 0.125f;
};
}
