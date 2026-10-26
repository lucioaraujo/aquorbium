// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

namespace aquorbium
{
// Smooth equal-power transition between healthy and cracked states. It maps
// stress but does not own or modify AquariumIntegrity.
class IntegrityCrossfade final
{
public:
    struct Output
    {
        float mix = 0.0f;
        float healthyGain = 1.0f;
        float crackedGain = 0.0f;
    };

    void prepare(double newSampleRate) noexcept;
    void setThresholds(float healthy, float cracked) noexcept;
    void setTransitionSeconds(float value) noexcept;
    void setStress(float value) noexcept;
    void reset(float stress = 0.0f) noexcept;
    [[nodiscard]] Output tick() noexcept;

    [[nodiscard]] float targetMix() const noexcept { return target; }
    [[nodiscard]] float currentMix() const noexcept { return current; }

private:
    void updateCoefficient() noexcept;
    [[nodiscard]] float mapStress(float value) const noexcept;

    double sampleRate = 48000.0;
    float healthyThreshold = 0.25f;
    float crackedThreshold = 0.75f;
    float transitionSeconds = 0.25f;
    float coefficient = 0.0f;
    float target = 0.0f;
    float current = 0.0f;
};
}
