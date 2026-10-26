// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace aquorbium
{
class PhilosophyVectorMixer final
{
public:
    static constexpr std::size_t sourceCount = 4;
    using Values = std::array<float, sourceCount>;

    // Corners are ordered lower-left, lower-right, upper-left, upper-right.
    [[nodiscard]] static Values weights(float x, float y) noexcept;
    [[nodiscard]] static float mix(const Values& sources, float x, float y) noexcept;
};

class PhilosophySequencer final
{
public:
    static constexpr std::size_t maximumSteps = 16;

    struct Step
    {
        float x = 0.0f;
        float y = 0.0f;
        std::uint16_t durationPulses = 1;
    };

    struct Output
    {
        PhilosophyVectorMixer::Values weights {};
        std::size_t index = 0;
        bool advanced = false;
    };

    PhilosophySequencer() noexcept;
    void setLength(std::size_t value) noexcept;
    [[nodiscard]] bool setStep(std::size_t index, Step value) noexcept;
    void reset(std::size_t index = 0) noexcept;
    [[nodiscard]] Output tick(bool clockHigh) noexcept;
    [[nodiscard]] std::size_t length() const noexcept { return stepCount; }
    [[nodiscard]] std::size_t currentIndex() const noexcept { return currentStep; }

private:
    std::array<Step, maximumSteps> steps {};
    std::size_t stepCount = 4;
    std::size_t currentStep = 0;
    std::uint16_t pulsesAtStep = 0;
    bool previousClock = false;
};

class PhilosophyMorph final
{
public:
    struct Output
    {
        PhilosophyVectorMixer::Values weights {};
        float mix = 0.0f;
        bool active = false;
    };

    void prepare(double sampleRate) noexcept;
    void setTransitionSeconds(float value) noexcept;
    void reset(std::size_t philosophy = 0) noexcept;
    void setTarget(std::size_t philosophy) noexcept;
    [[nodiscard]] Output tick(
        const PhilosophyVectorMixer::Values& sources) noexcept;
    [[nodiscard]] std::size_t target() const noexcept { return targetIndex; }
    [[nodiscard]] float transitionSeconds() const noexcept { return transitionTime; }

private:
    void updateDuration() noexcept;

    PhilosophyVectorMixer::Values current { 1.0f, 0.0f, 0.0f, 0.0f };
    PhilosophyVectorMixer::Values start { 1.0f, 0.0f, 0.0f, 0.0f };
    PhilosophyVectorMixer::Values destination { 1.0f, 0.0f, 0.0f, 0.0f };
    double preparedSampleRate = 48000.0;
    std::size_t durationSamples = 4800;
    std::size_t elapsedSamples = 4800;
    std::size_t targetIndex = 0;
    float transitionTime = 0.1f;
};

class PhilosophyMatrixMixer final
{
public:
    static constexpr std::size_t sourceCount = 4;
    static constexpr std::size_t outputCount = 4;

    struct Cell
    {
        float gain = 0.0f;
        float modulationDepth = 0.0f;
    };

    struct Output
    {
        // Channels 0/1 are stereo bus A; 2/3 are stereo bus B.
        std::array<float, outputCount> channel {};
        std::array<float, outputCount> normalization {};
    };

    PhilosophyMatrixMixer() noexcept;
    [[nodiscard]] bool setCell(std::size_t output, std::size_t source,
                               Cell value) noexcept;
    [[nodiscard]] Cell cell(std::size_t output, std::size_t source) const noexcept;
    [[nodiscard]] Output process(
        const std::array<float, sourceCount>& sources,
        const std::array<float, sourceCount>& modulation) const noexcept;

private:
    std::array<std::array<Cell, sourceCount>, outputCount> matrix {};
};
}
