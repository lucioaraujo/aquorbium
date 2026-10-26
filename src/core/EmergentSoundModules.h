// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include "core/SlewRateRandomWalk.h"

#include <array>
#include <cstddef>
#include <cstdint>

namespace aquorbium
{
class ReeseVoice final
{
public:
    struct Parameters
    {
        float frequencyHz = 55.0f;
        float detuneCents = 18.0f;
        float driftAmount = 0.35f;
    };

    void prepare(double sampleRate) noexcept;
    void setSeed(std::uint32_t value) noexcept;
    void reset() noexcept;
    void setParameters(Parameters value) noexcept;
    [[nodiscard]] Parameters parameters() const noexcept { return controls; }
    [[nodiscard]] float tick() noexcept;

private:
    [[nodiscard]] static float polyBlep(float phase, float increment) noexcept;
    [[nodiscard]] static float saw(float phase, float increment) noexcept;

    Parameters controls {};
    double preparedSampleRate = 48000.0;
    std::array<float, 3> phase { 0.0f, 0.31f, 0.67f };
    SlewRateRandomWalk drift;
};

class DualPeakFilter final
{
public:
    struct Parameters
    {
        float centre = 0.45f;
        float separation = 0.35f;
        float resonance = 0.55f;
        float crossFeedback = 0.25f;
        float series = 0.0f;
    };

    void prepare(double sampleRate) noexcept;
    void reset() noexcept;
    void setParameters(Parameters value) noexcept;
    [[nodiscard]] Parameters parameters() const noexcept { return controls; }
    [[nodiscard]] float tick(float input) noexcept;

private:
    struct Biquad
    {
        float b0 = 0.0f, b1 = 0.0f, b2 = 0.0f, a1 = 0.0f, a2 = 0.0f;
        float z1 = 0.0f, z2 = 0.0f;
        [[nodiscard]] float tick(float input) noexcept;
        void reset() noexcept { z1 = z2 = 0.0f; }
    };
    void updateCoefficients() noexcept;

    Parameters controls {};
    double preparedSampleRate = 48000.0;
    std::array<Biquad, 2> peak {};
    float previousA = 0.0f;
    float previousB = 0.0f;
};

class ResonantFilterSequencer final
{
public:
    static constexpr std::size_t bandCount = 8;
    struct Parameters
    {
        float baseFrequencyHz = 110.0f;
        float resonance = 0.62f;
    };
    struct Output
    {
        float value = 0.0f;
        std::size_t primary = 0;
        std::size_t secondary = 0;
    };

    void prepare(double sampleRate) noexcept;
    void reset() noexcept;
    void setParameters(Parameters value) noexcept;
    [[nodiscard]] Parameters parameters() const noexcept { return controls; }
    [[nodiscard]] Output tick(float input, float rotationPhase,
                              float encounter) noexcept;

private:
    struct Biquad
    {
        float b0 = 0.0f, b1 = 0.0f, b2 = 0.0f, a1 = 0.0f, a2 = 0.0f;
        float z1 = 0.0f, z2 = 0.0f;
        [[nodiscard]] float tick(float input) noexcept;
        void reset() noexcept { z1 = z2 = 0.0f; }
    };
    void updateCoefficients() noexcept;

    Parameters controls {};
    double preparedSampleRate = 48000.0;
    std::array<Biquad, bandCount> filters {};
};

class FrequencyIsolator final
{
public:
    struct Parameters
    {
        float focus = 0.0f;
        float centre = 0.5f;
        float width = 0.5f;
    };

    void prepare(double sampleRate) noexcept;
    void reset() noexcept;
    void setParameters(Parameters value) noexcept;
    [[nodiscard]] Parameters parameters() const noexcept { return controls; }
    [[nodiscard]] float tick(float input) noexcept;

private:
    struct Biquad
    {
        float b0 = 0.0f, b1 = 0.0f, b2 = 0.0f, a1 = 0.0f, a2 = 0.0f;
        float z1 = 0.0f, z2 = 0.0f;
        [[nodiscard]] float tick(float input) noexcept;
        void reset() noexcept { z1 = z2 = 0.0f; }
    };
    void updateCoefficients() noexcept;

    Parameters controls {};
    double preparedSampleRate = 48000.0;
    std::array<Biquad, 2> filters {};
};

class InteractionProcessor final
{
public:
    enum class Mode : std::uint8_t { ring, xorLogic };
    struct Parameters
    {
        float amount = 0.5f;
        float harmonicRichness = 0.5f;
        float pursuitDucking = 0.5f;
        Mode mode = Mode::ring;
    };
    struct Output
    {
        float artifact = 0.0f;
        float prey = 0.0f;
        float mix = 0.0f;
    };

    void prepare(double sampleRate) noexcept;
    void reset() noexcept;
    void setParameters(Parameters value) noexcept;
    [[nodiscard]] Parameters parameters() const noexcept { return controls; }
    [[nodiscard]] Output tick(float predator, float prey,
                              float contact, float pursuit) noexcept;

private:
    Parameters controls {};
    float smoothedContact = 0.0f;
    float smoothedPursuit = 0.0f;
    float smoothing = 0.001f;
};

class PowerStarve final
{
public:
    struct Parameters
    {
        float stress = 0.0f;
        float depth = 0.6f;
        float instability = 0.5f;
    };

    void prepare(double sampleRate) noexcept;
    void setSeed(std::uint32_t value) noexcept;
    void reset() noexcept;
    void setParameters(Parameters value) noexcept;
    [[nodiscard]] Parameters parameters() const noexcept { return controls; }
    [[nodiscard]] float tick(float input) noexcept;

private:
    Parameters controls {};
    float currentStress = 0.0f;
    float smoothing = 0.001f;
    float held = 0.0f;
    std::uint32_t counter = 0;
    std::uint32_t seed = 1u;
    std::uint32_t randomState = 1u;
};

class QuadratureStereoRotator final
{
public:
    struct Output { float left = 0.0f; float right = 0.0f; };
    [[nodiscard]] static Output process(float left, float right,
                                        float phase) noexcept;
};

class PolygonalOscillator final
{
public:
    struct Parameters
    {
        float frequencyHz = 110.0f;
        std::size_t vertices = 5;
        float roundness = 0.2f;
        float rotation = 0.0f;
    };
    struct Output
    {
        float audio = 0.0f;
        float x = 0.0f;
        float y = 0.0f;
    };

    void prepare(double sampleRate) noexcept;
    void reset(float phase = 0.0f) noexcept;
    void setParameters(Parameters value) noexcept;
    [[nodiscard]] Parameters parameters() const noexcept { return controls; }
    [[nodiscard]] Output tick() noexcept;

private:
    Parameters controls {};
    double preparedSampleRate = 48000.0;
    double phase = 0.0;
};
}
