// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include <algorithm>
#include <cmath>
#include <array>
#include <cstdint>

namespace aquorbium
{
// Control-rate source shared by the organisms. It deliberately owns every
// stochastic state: a seed always recreates the same biome, independently of
// the order in which a host calls the audio callback.
//
// Its correlated register readings and related slow rates were adapted from
// original Rasgo Synth modules (BuchlaRandomSource and OchdBank), but the
// implementation and mapping here belong to Aquorbium's performance engine.
class BiomaBrain
{
public:
    static constexpr std::size_t organismCount = 5;

    void prepare(double sampleRate) noexcept;
    void setSeed(std::uint32_t value) noexcept;
    void reset() noexcept;
    void setEcologicalFeedback(float value) noexcept;
    [[nodiscard]] float ecologicalFeedbackAmount() const noexcept
    {
        return ecologicalFeedback;
    }
    [[nodiscard]] std::array<float, organismCount> nextDrift(float amount) noexcept;
    // the CÉREBRO card (2026-10-24): the register's clock (x the classic
    // 0.23 Hz; 1 = classic) and how much the organisms share one leaning
    // (0 = each its own weights, the classic; 1 = all the same)
    void setControls(float clockScale, float coordination) noexcept
    {
        clockScaleValue = std::isfinite(clockScale) ? std::clamp(clockScale, 0.05f, 8.0f) : 1.0f;
        coordinationValue = std::isfinite(coordination) ? std::clamp(coordination, 0.0f, 1.0f) : 0.0f;
    }
    [[nodiscard]] std::uint8_t registerBits() const noexcept { return registerState; }
    // the shift register read as five correlated leanings, -1..1, held
    // between its clock ticks (~0.23 Hz): what the Brain hands to the
    // Sequencer and to the Space (2026-10-23)
    [[nodiscard]] const std::array<float, organismCount>& correlation() const noexcept
    {
        return heldCorrelation;
    }
    [[nodiscard]] float nextRandomUnit() noexcept;

private:
    void configureMotion() noexcept;
    void clockCorrelatedRegister() noexcept;
    [[nodiscard]] std::array<float, organismCount> correlatedValues() const noexcept;

    double sampleRate = 48000.0;
    std::uint32_t randomState = 0xA91B42C3u;
    std::uint8_t registerState = 0xB5u;
    double clockPhase = 0.0;
    std::array<double, organismCount> phase {};
    std::array<double, organismCount> ratio {};
    std::array<float, organismCount> heldCorrelation {};
    float ecologicalFeedback = 0.0f;
    float clockScaleValue = 1.0f;
    float coordinationValue = 0.0f;
};
}
