// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include "core/CoreRefinementModules.h"
#include "core/SlewRateRandomWalk.h"

#include <array>
#include <cstddef>
#include <cstdint>

namespace aquorbium
{
class BiomaBassVoice final
{
public:
    struct Parameters
    {
        float frequencyHz = 55.0f;
        float fmRatio = 1.5f;
        float fmDepth = 0.45f;
        float detuneCents = 18.0f;
        float subSemitones = -12.0f;
        float fold = 0.35f;
        float cutoff = 0.45f;
        float resonance = 0.45f;
        std::size_t poles = 4;
        float energyControl = 0.55f;
    };
    struct Telemetry
    {
        float energy = 0.0f;
        float effectiveFold = 0.0f;
        float effectiveDetuneCents = 0.0f;
        float instantaneousFrequencyHz = 0.0f;
    };

    void prepare(double sampleRate) noexcept;
    void setSeed(std::uint32_t value) noexcept;
    void reset() noexcept;
    void setParameters(Parameters value) noexcept;
    [[nodiscard]] Parameters parameters() const noexcept { return controls; }
    [[nodiscard]] float tick() noexcept;
    [[nodiscard]] Telemetry telemetry() const noexcept { return state; }

private:
    [[nodiscard]] static float polyBlep(float phase, float increment) noexcept;
    [[nodiscard]] static float saw(float phase, float increment) noexcept;

    Parameters controls {};
    Telemetry state {};
    double sampleRate = 48000.0;
    std::array<float, 3> phase { 0.0f, 0.31f, 0.67f };
    float modulatorPhase = 0.0f;
    float subPhase = 0.0f;
    std::array<float, 4> filterState {};
    float energyAttack = 0.01f;
    float energyRelease = 0.001f;
    float filterCoefficient = 0.1f;
    float subIncrement = 0.0f;
    SlewRateRandomWalk drift;
    OversampledSineFolder folder;
};

class SchoolWaveVoice final
{
public:
    static constexpr std::size_t memberCount = 6;
    struct Parameters
    {
        float frequencyHz = 110.0f;
        float morph = 0.5f;
        float waveCut = 0.5f;
        float spread = 0.25f;
        float level = 0.5f;
    };
    struct Output { float left = 0.0f; float right = 0.0f; };

    void prepare(double sampleRate) noexcept;
    void reset() noexcept;
    void setParameters(Parameters value) noexcept;
    [[nodiscard]] Parameters parameters() const noexcept { return controls; }
    [[nodiscard]] Output tick(float sharedDirection,
                              float sharedPhase) noexcept;

private:
    [[nodiscard]] static float polyBlep(float phase, float increment) noexcept;
    [[nodiscard]] static float waveform(float phase, float increment,
                                        float morph, float cut) noexcept;

    Parameters controls {};
    double sampleRate = 48000.0;
    std::array<float, memberCount> phase {};
};

class HarmonicRippleVoice final
{
public:
    struct Parameters
    {
        float frequencyHz = 110.0f;
        std::size_t ripples = 4;
        float formant = 0.5f;
        float cluster = 0.35f;
        float level = 0.5f;
    };

    void prepare(double sampleRate) noexcept;
    void reset() noexcept;
    void setParameters(Parameters value) noexcept;
    [[nodiscard]] Parameters parameters() const noexcept { return controls; }
    [[nodiscard]] float tick() noexcept;

private:
    Parameters controls {};
    double sampleRate = 48000.0;
    double phase = 0.0;
};

class RecentMixWavetable final
{
public:
    static constexpr std::size_t historySize = 1024;
    static constexpr std::size_t tableSize = 512;
    struct Parameters
    {
        float frequencyHz = 82.41f;
        float colour = 0.5f;
        float level = 0.5f;
    };
    struct Output
    {
        float value = 0.0f;
        bool ready = false;
        bool capturing = false;
    };

    void prepare(double sampleRate) noexcept;
    void reset() noexcept;
    void setParameters(Parameters value) noexcept;
    [[nodiscard]] Parameters parameters() const noexcept { return controls; }
    [[nodiscard]] bool triggerCapture() noexcept;
    [[nodiscard]] Output tick(float input) noexcept;

private:
    Parameters controls {};
    double sampleRate = 48000.0;
    std::array<float, historySize> history {};
    std::array<std::array<float, tableSize>, 2> table {};
    std::uint64_t written = 0;
    std::size_t activeTable = 0;
    std::size_t captureIndex = tableSize;
    std::size_t captureStart = 0;
    float phase = 0.0f;
    bool hasTable = false;
};

class FrequencyShifter final
{
public:
    static constexpr std::size_t tapCount = 33;
    struct Parameters
    {
        float shiftHz = 0.0f;
        float mix = 1.0f;
    };

    void prepare(double sampleRate) noexcept;
    void reset() noexcept;
    void setParameters(Parameters value) noexcept;
    [[nodiscard]] Parameters parameters() const noexcept { return controls; }
    [[nodiscard]] float tick(float input) noexcept;

private:
    Parameters controls {};
    double sampleRate = 48000.0;
    std::array<float, tapCount> history {};
    std::array<float, tapCount> hilbert {};
    std::size_t writeIndex = 0;
    float carrierPhase = 0.0f;
};

class CrossFeedbackStateMachine final
{
public:
    enum class State : std::uint8_t { calm, listen, respond, overload };
    struct Parameters
    {
        float lowThreshold = 0.12f;
        float highThreshold = 0.48f;
        float hysteresis = 0.05f;
        float influence = 0.6f;
    };
    struct Output
    {
        float modulation = 0.0f;
        float envelope = 0.0f;
        State state = State::calm;
        bool changed = false;
    };

    void prepare(double sampleRate) noexcept;
    void reset() noexcept;
    void setParameters(Parameters value) noexcept;
    [[nodiscard]] Parameters parameters() const noexcept { return controls; }
    [[nodiscard]] Output tick(float audio, float context) noexcept;

private:
    Parameters controls {};
    float envelope = 0.0f;
    float attack = 0.01f;
    float release = 0.001f;
    State current = State::calm;
};

class SpotlightMixer final
{
public:
    static constexpr std::size_t sourceCount = 5;
    using Sources = std::array<float, sourceCount>;
    struct Output
    {
        float left = 0.0f;
        float right = 0.0f;
        std::size_t primary = 0;
    };

    [[nodiscard]] static Output process(const Sources& sources,
                                        float focusPhase,
                                        float duckBackground) noexcept;
};

class HungerProcessor final
{
public:
    struct Parameters
    {
        float density = 0.0f;
        float scarcity = 0.5f;
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
    float supply = 1.0f;
    float smoothing = 0.001f;
    float gate = 1.0f;
    float gateTarget = 1.0f;
    std::uint32_t countdown = 0;
    std::uint32_t seed = 1u;
    std::uint32_t randomState = 1u;
};

class BoundedSelfFeedback final
{
public:
    struct Parameters
    {
        float feedback = 0.0f;
        float tone = 0.5f;
        float drive = 0.5f;
        float mix = 0.5f;
    };

    void prepare(double sampleRate) noexcept;
    void reset() noexcept;
    void setParameters(Parameters value) noexcept;
    [[nodiscard]] Parameters parameters() const noexcept { return controls; }
    [[nodiscard]] float tick(float input) noexcept;

private:
    Parameters controls {};
    float delayed = 0.0f;
    float filtered = 0.0f;
    float filterCoefficient = 0.1f;
    double sampleRate = 48000.0;
};

class WaterInertia final
{
public:
    struct Parameters
    {
        float inertia = 0.5f;
        float viscosity = 0.5f;
        float asymmetry = 0.0f;
    };

    void prepare(double sampleRate) noexcept;
    void reset(float value = 0.0f) noexcept;
    void setParameters(Parameters value) noexcept;
    [[nodiscard]] Parameters parameters() const noexcept { return controls; }
    [[nodiscard]] float tick(float target) noexcept;

private:
    Parameters controls {};
    double sampleRate = 48000.0;
    float current = 0.0f;
    float riseCoefficient = 0.001f;
    float fallCoefficient = 0.001f;
};
}
