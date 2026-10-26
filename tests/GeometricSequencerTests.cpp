// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/GeometricSequencer.h"

#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <vector>

namespace
{
bool patternIsBalanced()
{
    for (std::size_t steps = 1; steps <= aquorbium::EuclideanPattern::maximumSteps; ++steps)
    {
        for (std::size_t pulses = 0; pulses <= steps; ++pulses)
        {
            aquorbium::EuclideanPattern pattern;
            pattern.generate(steps, pulses);
            std::vector<std::size_t> attacks;
            for (std::size_t step = 0; step < steps; ++step)
                if (pattern.at(step)) attacks.push_back(step);
            if (attacks.size() != pulses || pattern.steps() != steps
                || pattern.pulses() != pulses)
                return false;

            if (pulses > 1 && pulses < steps)
            {
                std::size_t minimumGap = steps;
                std::size_t maximumGap = 0;
                for (std::size_t index = 0; index < attacks.size(); ++index)
                {
                    const auto next = attacks[(index + 1) % attacks.size()]
                        + (index + 1 == attacks.size() ? steps : 0);
                    const auto gap = next - attacks[index];
                    minimumGap = std::min(minimumGap, gap);
                    maximumGap = std::max(maximumGap, gap);
                }
                if (maximumGap - minimumGap > 1)
                    return false;
            }
        }
    }
    return true;
}

bool rotationIsCircular()
{
    aquorbium::EuclideanPattern base;
    aquorbium::EuclideanPattern rotated;
    base.generate(13, 5, 0);
    rotated.generate(13, 5, 3);
    for (std::size_t step = 0; step < 13; ++step)
        if (base.at(step) != rotated.at((step + 3) % 13)) return false;
    return true;
}

bool quantizerIsDefined()
{
    aquorbium::IntervalQuantizer quantizer;
    if (quantizer.semitonesForDegree(0) != 0.0f
        || quantizer.semitonesForDegree(6) != 10.0f
        || quantizer.semitonesForDegree(7) != 12.0f
        || quantizer.semitonesForDegree(-1) != -2.0f
        || quantizer.semitonesForDegree(-7) != -12.0f)
        return false;

    std::array<float, aquorbium::IntervalQuantizer::maximumIntervals> micro {};
    micro[0] = 0.0f;
    micro[1] = 1.5f;
    micro[2] = 4.25f;
    if (!quantizer.setField(micro, 3, 7.0f)
        || quantizer.semitonesForDegree(3) != 7.0f
        || quantizer.semitonesForDegree(-1) != -2.75f)
        return false;

    const auto previousSize = quantizer.size();
    micro[1] = std::nanf("");
    return !quantizer.setField(micro, 3, 7.0f) && quantizer.size() == previousSize;
}

using Step = aquorbium::GeometricSequencer::Step;

bool sameStep(const Step& left, const Step& right)
{
    return left.gate == right.gate && left.mutated == right.mutated
        && left.lane == right.lane && left.orbitStep == right.orbitStep
        && left.orbitLength == right.orbitLength && left.pulseCount == right.pulseCount
        && left.degree == right.degree
        && std::bit_cast<std::uint32_t>(left.midiPitch)
            == std::bit_cast<std::uint32_t>(right.midiPitch)
        && std::bit_cast<std::uint32_t>(left.accent)
            == std::bit_cast<std::uint32_t>(right.accent)
        && left.cv == right.cv && left.event == right.event;
}

std::vector<Step> sequence(std::uint32_t seed,
                           aquorbium::GeometricSequencer::PerformanceIntent intent)
{
    aquorbium::GeometricSequencer sequencer;
    sequencer.setSeed(seed);
    sequencer.setOrbitSteps(13);
    sequencer.setPerformanceIntent(intent);
    aquorbium::GeometricSequencer::GenerativeInput input;
    input.density = 0.47f;
    input.cohesion = 0.61f;
    input.danger = 0.73f;
    input.rootMidi = 38.0f;
    input.anchorDegree = 2;
    input.registerSemitones = 12;

    std::vector<Step> result;
    for (int event = 0; event < 160; ++event)
    {
        input.section = event / 19;
        result.push_back(sequencer.advance(
            static_cast<std::size_t>(event)
                % aquorbium::GeometricSequencer::laneCount,
            input));
    }
    return result;
}

bool deterministicAndPerformable()
{
    const aquorbium::GeometricSequencer::PerformanceIntent neutral {};
    const auto first = sequence(0x712a4e31u, neutral);
    const auto repeat = sequence(0x712a4e31u, neutral);
    if (!std::equal(first.begin(), first.end(), repeat.begin(), sameStep))
        return false;

    aquorbium::GeometricSequencer::PerformanceIntent ignored;
    ignored.influence = 0.0f;
    ignored.rhythmDensityBias = 1.0f;
    ignored.orbitRotation = -1.0f;
    ignored.intervalBias = 1.0f;
    ignored.mutation = 1.0f;
    const auto zeroInfluence = sequence(0x712a4e31u, ignored);
    if (!std::equal(first.begin(), first.end(), zeroInfluence.begin(), sameStep))
        return false;

    auto performedIntent = ignored;
    performedIntent.influence = 1.0f;
    const auto performed = sequence(0x712a4e31u, performedIntent);
    if (std::equal(first.begin(), first.end(), performed.begin(), sameStep))
        return false;

    const auto anotherSeed = sequence(0x183f09adu, neutral);
    if (std::equal(first.begin(), first.end(), anotherSeed.begin(), sameStep))
        return false;

    for (const auto& step : performed)
    {
        if (!std::isfinite(step.midiPitch) || !std::isfinite(step.accent)
            || step.midiPitch < 0.0f || step.midiPitch > 127.0f
            || step.accent < 0.0f || step.accent > 1.0f
            || !std::all_of(step.cv.begin(), step.cv.end(), [](float value) {
                return std::isfinite(value) && value >= 0.0f && value <= 1.0f;
            }))
            return false;
    }
    return true;
}

bool lanesOwnTheirRandomState()
{
    aquorbium::GeometricSequencer isolated;
    aquorbium::GeometricSequencer interleaved;
    isolated.setSeed(0x99182b5u);
    interleaved.setSeed(0x99182b5u);
    aquorbium::GeometricSequencer::GenerativeInput input;
    input.danger = 1.0f;
    std::vector<Step> laneZero;
    std::vector<Step> laneZeroInterleaved;
    for (int event = 0; event < 100; ++event)
    {
        laneZero.push_back(isolated.advance(0, input));
        laneZeroInterleaved.push_back(interleaved.advance(0, input));
        (void) interleaved.advance(1, input);
        (void) interleaved.advance(2, input);
        (void) interleaved.advance(3, input);
        (void) interleaved.advance(4, input);
    }
    return std::equal(laneZero.begin(), laneZero.end(), laneZeroInterleaved.begin(), sameStep);
}
}

// the Brain's leaning (2026-10-23): at GEOMETRY zero it changes nothing, bit
// for bit; at GEOMETRY one, a full leaning decides every mutation's way
bool brainLeansTheMutations()
{
    const auto run = [](float influence, float lean, std::vector<int>& degrees) {
        aquorbium::GeometricSequencer sequencer;
        sequencer.setSeed(0x5eed1u);
        sequencer.setPerformanceIntent({ influence, 0.0f, 0.0f, 0.0f, 1.0f });
        aquorbium::GeometricSequencer::GenerativeInput input;
        input.danger = 1.0f;  // mutations happen even at influence zero
        input.brainLean = lean;
        for (int event = 0; event < 4000; ++event)
        {
            degrees.push_back(sequencer.advance(static_cast<std::size_t>(event) % 4u, input).degree);
        }
    };
    std::vector<int> plain, leaned;
    run(0.0f, 0.0f, plain);
    run(0.0f, 1.0f, leaned);
    if (plain != leaned)
        return false;  // influence zero: the leaning must change nothing
    // full influence: compare the degrees of a +1 and a -1 leaning; every
    // mutated step differs by exactly 2 degrees (one up, one down)
    std::vector<int> upward, downward;
    run(1.0f, 1.0f, upward);
    run(1.0f, -1.0f, downward);
    std::size_t differing = 0;
    for (std::size_t i = 0; i < upward.size(); ++i)
    {
        if (upward[i] == downward[i])
            continue;
        if (upward[i] - downward[i] != 2)
            return false;
        ++differing;
    }
    std::cout << "brain leaning: " << differing << " of " << upward.size()
              << " steps mutated the opposite way under opposite leanings\n";
    return differing > 100;
}

int main()
{
    if (!patternIsBalanced())
    {
        std::cerr << "Euclidean patterns are not balanced\n";
        return EXIT_FAILURE;
    }
    if (!rotationIsCircular())
    {
        std::cerr << "pattern rotation is not circular\n";
        return EXIT_FAILURE;
    }
    if (!quantizerIsDefined())
    {
        std::cerr << "interval quantizer failed\n";
        return EXIT_FAILURE;
    }
    if (!deterministicAndPerformable())
    {
        std::cerr << "sequencer determinism/performance contract failed\n";
        return EXIT_FAILURE;
    }
    if (!lanesOwnTheirRandomState())
    {
        std::cerr << "lane event traffic changed another lane's random path\n";
        return EXIT_FAILURE;
    }

    if (!brainLeansTheMutations())
    {
        std::cerr << "the Brain's leaning contract failed\n";
        return EXIT_FAILURE;
    }
    std::cout << "aquorbium geometric sequencer tests passed\n";
}
