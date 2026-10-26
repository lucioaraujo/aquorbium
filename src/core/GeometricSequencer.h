// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace aquorbium
{
class EuclideanPattern final
{
public:
    static constexpr std::size_t maximumSteps = 32;

    void generate(std::size_t stepCount, std::size_t pulseCount,
                  int rotation = 0) noexcept;
    [[nodiscard]] bool at(std::size_t index) const noexcept;
    [[nodiscard]] std::size_t steps() const noexcept { return activeSteps; }
    [[nodiscard]] std::size_t pulses() const noexcept { return activePulses; }

private:
    std::array<std::uint8_t, maximumSteps> pattern {};
    std::size_t activeSteps = 1;
    std::size_t activePulses = 0;
};

class IntervalQuantizer final
{
public:
    static constexpr std::size_t maximumIntervals = 16;

    IntervalQuantizer() noexcept;
    [[nodiscard]] bool setField(const std::array<float, maximumIntervals>& values,
                                std::size_t count, float periodSemitones) noexcept;
    [[nodiscard]] float semitonesForDegree(int degree) const noexcept;
    [[nodiscard]] float midiPitch(float rootMidi, int degree,
                                  int registerSemitones = 0) const noexcept;
    [[nodiscard]] std::size_t size() const noexcept { return intervalCount; }
    [[nodiscard]] float period() const noexcept { return periodSize; }

private:
    std::array<float, maximumIntervals> intervals {};
    std::size_t intervalCount = 7;
    float periodSize = 12.0f;
};

// Event-rate geometry shared by the autonomous and performative paths. It
// deliberately owns no clock and renders no audio: callers decide when an
// ecological or external event advances a lane.
class GeometricSequencer final
{
public:
    static constexpr std::size_t laneCount = 5;

    GeometricSequencer() noexcept;

    struct GenerativeInput
    {
        float density = 0.5f;
        float rhythmRotation = 0.0f;
        float cohesion = 0.5f;
        float danger = 0.0f;
        float rootMidi = 36.0f;
        int anchorDegree = 0;
        int registerSemitones = 0;
        int section = 0;
        // the Brain's leaning for this lane, -1..1 (2026-10-23): with
        // GEOMETRY above zero it tilts which way a mutation goes, so
        // organisms correlated in the Brain's register mutate together
        float brainLean = 0.0f;
    };

    struct PerformanceIntent
    {
        float influence = 0.0f;
        float rhythmDensityBias = 0.0f;
        float orbitRotation = 0.0f;
        float intervalBias = 0.0f;
        float mutation = 0.0f;
    };

    struct Step
    {
        bool gate = false;
        bool mutated = false;
        std::uint8_t lane = 0;
        std::uint8_t orbitStep = 0;
        std::uint8_t orbitLength = 1;
        std::uint8_t pulseCount = 0;
        // Continuous event probability; equals gate as 0/1 without a morph.
        float gateProbability = 0.0f;
        int degree = 0;
        float midiPitch = 36.0f;
        float accent = 0.0f;
        std::array<float, 4> cv {};
        std::uint64_t event = 0;
    };

    void setSeed(std::uint32_t value) noexcept;
    // the session's world: mode, orbit length, rotations, contour
    void setWorld(const std::array<float, IntervalQuantizer::maximumIntervals>& scale, std::size_t scaleSize,
                  std::size_t orbitSteps, const std::array<int, laneCount>& rotations,
                  const std::array<int, 7>& contour) noexcept
    {
        static_cast<void>(intervalQuantizer.setField(scale, scaleSize, 12.0f));
        setOrbitSteps(orbitSteps);
        laneRotations = rotations;
        circularContour = contour;
    }
    void reset() noexcept;
    void setOrbitSteps(std::size_t value) noexcept;
    void setPerformanceIntent(const PerformanceIntent& value) noexcept;
    [[nodiscard]] Step advance(std::size_t lane,
                               const GenerativeInput& input) noexcept;
    [[nodiscard]] const PerformanceIntent& performanceIntent() const noexcept
    {
        return performance;
    }
    [[nodiscard]] std::size_t orbitSteps() const noexcept { return steps; }
    [[nodiscard]] const IntervalQuantizer& quantizer() const noexcept
    {
        return intervalQuantizer;
    }

private:
    [[nodiscard]] float nextRandomUnit(std::size_t lane) noexcept;

    std::uint32_t seed = 0x2cb76f91u;
    std::size_t steps = 8;
    // the session's world (SessionGenesis.h): the lanes' rotations and the
    // melody's contour; these defaults are the classic instrument
    std::array<int, laneCount> laneRotations { 0, 2, 5, 7, 3 };
    std::array<int, 7> circularContour { 0, 1, -1, 2, -2, 3, -3 };
    PerformanceIntent performance;
    IntervalQuantizer intervalQuantizer;
    std::array<std::uint8_t, laneCount> cursors {};
    std::array<std::uint32_t, laneCount> randomStates {};
    std::array<std::uint64_t, laneCount> eventCounts {};
};
}
