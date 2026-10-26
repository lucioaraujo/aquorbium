// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/GeometricSequencer.h"

#include "core/Numeric.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace aquorbium
{
namespace
{
constexpr std::array<float, IntervalQuantizer::maximumIntervals> defaultField {
    0.0f, 2.0f, 3.0f, 5.0f, 7.0f, 9.0f, 10.0f
};

float finiteOr(float value, float fallback) noexcept
{
    return std::isfinite(value) ? value : fallback;
}

int normalizedRotation(int rotation, int length) noexcept
{
    auto result = rotation % length;
    if (result < 0)
        result += length;
    return result;
}
}

void EuclideanPattern::generate(std::size_t stepCount, std::size_t pulseCount,
                                int rotation) noexcept
{
    activeSteps = std::clamp<std::size_t>(stepCount, 1, maximumSteps);
    activePulses = std::min(pulseCount, activeSteps);
    pattern.fill(0);
    if (activePulses == 0)
        return;

    const auto length = static_cast<int>(activeSteps);
    const auto normalized = normalizedRotation(rotation, length);
    for (std::size_t step = 0; step < activeSteps; ++step)
    {
        // This circular residue construction is the event-distribution form
        // of the Euclidean process. It keeps the implementation bounded and
        // makes rotation independent from the pattern itself.
        const auto residue = (step * activePulses) % activeSteps;
        if (residue < activePulses)
        {
            const auto destination = (step + static_cast<std::size_t>(normalized))
                % activeSteps;
            pattern[destination] = 1;
        }
    }
}

bool EuclideanPattern::at(std::size_t index) const noexcept
{
    return pattern[index % activeSteps] != 0;
}

IntervalQuantizer::IntervalQuantizer() noexcept
{
    intervals = defaultField;
}

bool IntervalQuantizer::setField(
    const std::array<float, maximumIntervals>& values, std::size_t count,
    float periodSemitones) noexcept
{
    if (count == 0 || count > maximumIntervals || !std::isfinite(periodSemitones)
        || periodSemitones <= 0.0f)
        return false;

    auto previous = -std::numeric_limits<float>::infinity();
    for (std::size_t index = 0; index < count; ++index)
    {
        const auto value = values[index];
        if (!std::isfinite(value) || value < 0.0f || value >= periodSemitones
            || value <= previous)
            return false;
        previous = value;
    }

    intervals = values;
    intervalCount = count;
    periodSize = periodSemitones;
    return true;
}

float IntervalQuantizer::semitonesForDegree(int degree) const noexcept
{
    const auto size = static_cast<int>(intervalCount);
    auto periodIndex = degree / size;
    auto localDegree = degree % size;
    if (localDegree < 0)
    {
        localDegree += size;
        --periodIndex;
    }
    return intervals[static_cast<std::size_t>(localDegree)]
        + static_cast<float>(periodIndex) * periodSize;
}

float IntervalQuantizer::midiPitch(float rootMidi, int degree,
                                   int registerSemitones) const noexcept
{
    const auto root = finiteOr(rootMidi, 36.0f);
    return std::clamp(root + static_cast<float>(registerSemitones)
                          + semitonesForDegree(degree),
                      0.0f, 127.0f);
}

GeometricSequencer::GeometricSequencer() noexcept
{
    reset();
}

void GeometricSequencer::setSeed(std::uint32_t value) noexcept
{
    seed = value == 0 ? 1u : value;
    reset();
}

void GeometricSequencer::reset() noexcept
{
    cursors.fill(0);
    eventCounts.fill(0);
    for (std::size_t lane = 0; lane < laneCount; ++lane)
    {
        auto state = seed ^ (0x9e3779b9u * static_cast<std::uint32_t>(lane + 1));
        state ^= state >> 16u;
        state *= 0x7feb352du;
        state ^= state >> 15u;
        randomStates[lane] = state == 0 ? static_cast<std::uint32_t>(lane + 1) : state;
    }
}

void GeometricSequencer::setOrbitSteps(std::size_t value) noexcept
{
    const auto next = std::clamp<std::size_t>(value, 1, EuclideanPattern::maximumSteps);
    if (next == steps)
        return;
    steps = next;
    for (auto& cursor : cursors)
        cursor = static_cast<std::uint8_t>(static_cast<std::size_t>(cursor) % steps);
}

void GeometricSequencer::setPerformanceIntent(const PerformanceIntent& value) noexcept
{
    if (std::isfinite(value.influence))
        performance.influence = std::clamp(value.influence, 0.0f, 1.0f);
    if (std::isfinite(value.rhythmDensityBias))
        performance.rhythmDensityBias = std::clamp(value.rhythmDensityBias, -1.0f, 1.0f);
    if (std::isfinite(value.orbitRotation))
        performance.orbitRotation = std::clamp(value.orbitRotation, -1.0f, 1.0f);
    if (std::isfinite(value.intervalBias))
        performance.intervalBias = std::clamp(value.intervalBias, -1.0f, 1.0f);
    if (std::isfinite(value.mutation))
        performance.mutation = std::clamp(value.mutation, 0.0f, 1.0f);
}

GeometricSequencer::Step GeometricSequencer::advance(
    std::size_t requestedLane, const GenerativeInput& input) noexcept
{
    const auto lane = std::min(requestedLane, laneCount - 1);
    const auto density = std::clamp(finiteOr(input.density, 0.5f), 0.0f, 1.0f);
    const auto cohesion = std::clamp(finiteOr(input.cohesion, 0.5f), 0.0f, 1.0f);
    const auto danger = std::clamp(finiteOr(input.danger, 0.0f), 0.0f, 1.0f);
    const auto influence = performance.influence;
    const auto effectiveDensity = std::clamp(
        density + performance.rhythmDensityBias * influence * 0.5f, 0.0f, 1.0f);
    const auto pulseCount = static_cast<std::size_t>(
        std::round(effectiveDensity * static_cast<float>(steps)));

    const auto performanceRotation = static_cast<int>(std::round(
        performance.orbitRotation * influence * static_cast<float>(steps)));
    auto rotation = input.section + laneRotations[lane] + performanceRotation;

    const auto mutationChance = std::clamp(
        danger * 0.12f + performance.mutation * influence * 0.38f, 0.0f, 0.5f);
    auto mutated = false;
    auto pitchMutation = 0;
    if (mutationChance > 0.0f && nextRandomUnit(lane) < mutationChance)
    {
        mutated = true;
        // the coin, tilted by the Brain as far as GEOMETRY reaches; at zero
        // influence the threshold is 0.5, the former fair coin exactly
        const auto lean = std::clamp(finiteOr(input.brainLean, 0.0f), -1.0f, 1.0f);
        const auto downThreshold = 0.5f - 0.5f * lean * influence;
        const auto direction = nextRandomUnit(lane) < downThreshold ? -1 : 1;
        rotation += direction;
        pitchMutation = direction;
    }

    EuclideanPattern pattern;
    pattern.generate(steps, pulseCount, rotation);
    const auto orbitStep = static_cast<std::size_t>(cursors[lane]) % steps;
    const auto gate = pattern.at(orbitStep);
    auto gateProbability = gate ? 1.0f : 0.0f;
    const auto rhythmRotation = std::clamp(
        finiteOr(input.rhythmRotation, 0.0f), -1.0f, 1.0f);
    if (std::abs(rhythmRotation) > 0.0f)
    {
        EuclideanPattern rotatedPattern;
        rotatedPattern.generate(steps, pulseCount,
            rotation + (rhythmRotation > 0.0f ? 1 : -1));
        const auto rotatedGate = rotatedPattern.at(orbitStep) ? 1.0f : 0.0f;
        gateProbability += (rotatedGate - gateProbability)
            * std::abs(rhythmRotation);
    }

    const auto stride = cohesion > 0.70f ? 1 : danger > 0.66f ? 3 : 2;
    const auto contourIndex = normalizedRotation(
        static_cast<int>(orbitStep) * stride + input.section + laneRotations[lane],
        static_cast<int>(circularContour.size()));
    const auto spread = 1 + static_cast<int>(std::round(danger * 2.0f));
    const auto autonomousOffset = circularContour[static_cast<std::size_t>(contourIndex)]
        * spread;
    const auto cohesiveOffset = static_cast<int>(
        std::round(static_cast<float>(autonomousOffset) * (1.0f - cohesion * 0.72f)));
    const auto performanceOffset = static_cast<int>(
        std::round(performance.intervalBias * influence * 4.0f));
    const auto degree = input.anchorDegree + cohesiveOffset + performanceOffset
        + pitchMutation;
    const auto pitch = intervalQuantizer.midiPitch(
        input.rootMidi, degree, input.registerSemitones);

    Step result;
    result.gate = gate;
    result.mutated = mutated;
    result.lane = static_cast<std::uint8_t>(lane);
    result.orbitStep = static_cast<std::uint8_t>(orbitStep);
    result.orbitLength = static_cast<std::uint8_t>(steps);
    result.pulseCount = static_cast<std::uint8_t>(pulseCount);
    result.gateProbability = gateProbability;
    result.degree = degree;
    result.midiPitch = pitch;
    result.accent = gateProbability * std::clamp(
        (orbitStep == 0 ? 0.92f : 0.58f) + cohesion * 0.18f
            + (mutated ? 0.08f : 0.0f),
        0.0f, 1.0f);
    result.cv = {
        pitch / 127.0f,
        gate ? 1.0f : 0.0f,
        result.accent,
        steps > 1 ? static_cast<float>(orbitStep) / static_cast<float>(steps - 1) : 0.0f
    };
    result.event = eventCounts[lane]++;

    cursors[lane] = static_cast<std::uint8_t>((orbitStep + 1) % steps);
    return result;
}

float GeometricSequencer::nextRandomUnit(std::size_t lane) noexcept
{
    auto& state = randomStates[lane];
    state = state * 1664525u + 1013904223u;
    return static_cast<float>((state >> 8u) & 0x00ffffffu) / 16777215.0f;
}
}
