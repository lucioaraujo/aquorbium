// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include <array>

namespace aquorbium
{
// Fixed 4x interpolated evaluation of a sine waveshaper. The short box
// decimator is deliberately modest: it reduces fold aliases without adding a
// large oversampling filter or changing the exact amount=0 path.
class OversampledSineFolder final
{
public:
    void reset() noexcept;
    [[nodiscard]] float process(float input, float amount,
                                float drive) noexcept;

private:
    float previousInput = 0.0f;
    bool initialized = false;
};

class BoundedMasterFeedback final
{
public:
    struct Parameters
    {
        float amount = 0.35f;
        float attackSeconds = 0.08f;
        float releaseSeconds = 0.80f;
        float maximum = 0.32f;
    };

    void prepare(double updateRateHz) noexcept;
    void reset() noexcept;
    void setParameters(Parameters value) noexcept;
    [[nodiscard]] Parameters parameters() const noexcept { return controls; }
    [[nodiscard]] float process(
        const std::array<float, 3>& bandReductionDb,
        float glueReductionDb, float masterInfluence) noexcept;
    [[nodiscard]] float value() const noexcept { return current; }

private:
    void updateCoefficients() noexcept;

    Parameters controls {};
    double updateRate = 200.0;
    float attack = 0.05f;
    float release = 0.005f;
    float current = 0.0f;
};

class AirWaterMedium final
{
public:
    struct Parameters
    {
        float waterline = 0.35f;
        float transitionWidth = 0.18f;
        float underwaterDamping = 0.62f;
        float underwaterLoss = 0.18f;
    };

    struct Output
    {
        float left = 0.0f;
        float right = 0.0f;
        float waterMix = 0.0f;
        float density = 0.0f;
        float brightness = 1.0f;
    };

    void prepare(double sampleRate) noexcept;
    void reset() noexcept;
    void setParameters(Parameters value) noexcept;
    [[nodiscard]] Parameters parameters() const noexcept { return controls; }
    [[nodiscard]] Output process(float left, float right,
                                 float normalizedHeight,
                                 float turbidity,
                                 float temperature) noexcept;

private:
    Parameters controls {};
    double preparedRate = 48000.0;
    float stateLeft = 0.0f;
    float stateRight = 0.0f;
};
}
