// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace aquorbium
{
class FixedLSystemGenerator final
{
public:
    static constexpr std::size_t capacity = 128;
    enum class Symbol : std::uint8_t { forward, left, right };
    struct Event
    {
        bool gate = false;
        int turn = 0;
        float value = 0.0f;
        std::size_t position = 0;
    };

    [[nodiscard]] bool configure(std::size_t iterations,
                                 bool alternateRule) noexcept;
    void reset() noexcept;
    [[nodiscard]] Event advance() noexcept;
    [[nodiscard]] std::size_t length() const noexcept { return symbolCount; }

private:
    std::array<Symbol, capacity> symbols { Symbol::forward };
    std::array<Symbol, capacity> scratch {};
    std::size_t symbolCount = 1;
    std::size_t cursor = 0;
};

class CellularAutomatonSequencer final
{
public:
    struct Output
    {
        bool gate = false;
        float density = 0.0f;
        std::uint32_t state = 0;
    };

    void configure(std::uint8_t rule, std::uint32_t initialState) noexcept;
    void reset() noexcept;
    [[nodiscard]] Output advance() noexcept;

private:
    std::uint8_t rule = 30u;
    std::uint32_t initialState = 1u << 15u;
    std::uint32_t state = initialState;
};

class SpectralTransferVocoder final
{
public:
    static constexpr std::size_t bandCount = 6;
    struct Parameters
    {
        float centre = 0.5f;
        float span = 0.6f;
        float resonance = 0.5f;
        float response = 0.5f;
        float mix = 0.7f;
    };

    void prepare(double sampleRate) noexcept;
    void reset() noexcept;
    void setParameters(Parameters value) noexcept;
    [[nodiscard]] float tick(float modulator, float carrier) noexcept;

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
    double sampleRate = 48000.0;
    float envelopeCoefficient = 0.01f;
    std::array<Biquad, bandCount> modulatorFilters {};
    std::array<Biquad, bandCount> carrierFilters {};
    std::array<float, bandCount> envelope {};
};

class EnvelopeContagionBus final
{
public:
    static constexpr std::size_t memberCount = 6;
    using Members = std::array<float, memberCount>;

    void prepare(double sampleRate) noexcept;
    void reset() noexcept;
    [[nodiscard]] Members process(const Members& input,
                                  float contagion) noexcept;

private:
    Members envelope {};
    float attack = 0.01f;
    float release = 0.001f;
};

class NoInputFeedbackBus final
{
public:
    struct Parameters
    {
        float feedback = 0.75f;
        float drive = 0.55f;
        float colour = 0.5f;
        float level = 0.5f;
    };
    struct Output { float left = 0.0f; float right = 0.0f; };

    void prepare(double sampleRate) noexcept;
    void setSeed(std::uint32_t value) noexcept;
    void reset() noexcept;
    void setParameters(Parameters value) noexcept;
    [[nodiscard]] Output tick() noexcept;

private:
    [[nodiscard]] float noise() noexcept;
    Parameters controls {};
    double sampleRate = 48000.0;
    std::array<float, 3> coefficient { 0.01f, 0.03f, 0.09f };
    std::array<float, 3> leftState {};
    std::array<float, 3> rightState {};
    float feedbackLeft = 0.0001f;
    float feedbackRight = -0.0001f;
    std::uint32_t seed = 1u;
    std::uint32_t randomState = 1u;
};

class ReactiveThresholdStrategy final
{
public:
    static constexpr std::size_t stateCount = 8;
    using Thresholds = std::array<float, stateCount - 1>;
    struct Output
    {
        std::size_t state = 0;
        float value = 0.0f;
        bool changed = false;
    };

    void setThresholds(const Thresholds& value, float hysteresis) noexcept;
    void reset(float stimulus = 0.0f) noexcept;
    [[nodiscard]] Output tick(float stimulus) noexcept;

private:
    [[nodiscard]] std::size_t select(float stimulus) const noexcept;
    Thresholds thresholds { 0.12f, 0.25f, 0.38f, 0.50f,
                            0.62f, 0.75f, 0.88f };
    float hysteresis = 0.03f;
    std::size_t current = 0;
};

class LockInFocusExtractor final
{
public:
    struct Parameters
    {
        float referenceHz = 110.0f;
        float bandwidthHz = 12.0f;
        float mix = 0.7f;
        float gain = 0.7f;
    };

    void prepare(double sampleRate) noexcept;
    void reset() noexcept;
    void setParameters(Parameters value) noexcept;
    [[nodiscard]] float tick(float input) noexcept;
    [[nodiscard]] float magnitude() const noexcept;

private:
    Parameters controls {};
    double sampleRate = 48000.0;
    float phase = 0.0f;
    float phaseIncrement = 0.0f;
    float lowpassCoefficient = 0.01f;
    float inPhase = 0.0f;
    float quadrature = 0.0f;
};

class BehaviourSceneBank final
{
public:
    static constexpr std::size_t sceneCount = 8;
    static constexpr std::size_t dimensionCount = 4;
    using Scene = std::array<float, dimensionCount>;

    void prepare(double sampleRate) noexcept;
    void reset() noexcept;
    [[nodiscard]] bool setScene(std::size_t index, const Scene& scene) noexcept;
    [[nodiscard]] bool recall(std::size_t index,
                              float transitionSeconds) noexcept;
    [[nodiscard]] Scene tick() noexcept;
    [[nodiscard]] std::size_t currentScene() const noexcept { return selected; }

private:
    std::array<Scene, sceneCount> scenes {};
    Scene current {};
    Scene target {};
    double sampleRate = 48000.0;
    float coefficient = 1.0f;
    std::size_t selected = 0;
};

class ReconfigurableResourcePool final
{
public:
    static constexpr std::size_t valueCount = 16;
    enum class Mode : std::uint8_t { mixer, switcher, recorder, router };

    void reset() noexcept;
    void setMode(Mode value) noexcept { mode = value; }
    [[nodiscard]] bool setValue(std::size_t index, float value) noexcept;
    [[nodiscard]] float tick(float input, float address,
                             bool trigger) noexcept;

private:
    std::array<float, valueCount> values {};
    Mode mode = Mode::mixer;
    std::size_t writeIndex = 0;
    float held = 0.0f;
};

class LightweightBinauralSpatializer final
{
public:
    static constexpr std::size_t delayCapacity = 1024;
    struct Parameters
    {
        float azimuth = 0.0f;
        float elevation = 0.0f;
        float distance = 0.5f;
        float amount = 0.7f;
    };
    struct Output { float left = 0.0f; float right = 0.0f; };

    void prepare(double sampleRate) noexcept;
    void reset() noexcept;
    void setParameters(Parameters value) noexcept;
    [[nodiscard]] Output tick(float input) noexcept;

private:
    [[nodiscard]] float readDelay(float delaySamples) const noexcept;
    Parameters controls {};
    double sampleRate = 48000.0;
    std::array<float, delayCapacity> delay {};
    std::size_t writeIndex = 0;
    float low = 0.0f;
    float lowCoefficient = 0.05f;
};
}
