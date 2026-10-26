// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/PhilosophyMixEngine.h"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <utility>

namespace
{
bool vectorBlendMapsCornersAndCentre()
{
    using Mixer = aquorbium::PhilosophyVectorMixer;
    for (std::size_t corner = 0; corner < Mixer::sourceCount; ++corner)
    {
        const auto x = corner == 1 || corner == 3 ? 1.0f : 0.0f;
        const auto y = corner >= 2 ? 1.0f : 0.0f;
        const auto weights = Mixer::weights(x, y);
        for (std::size_t index = 0; index < weights.size(); ++index)
            if (weights[index] != (index == corner ? 1.0f : 0.0f))
                return false;
    }
    const auto centre = Mixer::weights(0.5f, 0.5f);
    auto sum = 0.0f;
    for (const auto weight : centre)
    {
        if (weight != 0.25f)
            return false;
        sum += weight;
    }
    const Mixer::Values values { 1.0f, 2.0f, 3.0f, 4.0f };
    return sum == 1.0f && Mixer::mix(values, 0.5f, 0.5f) == 2.5f;
}

bool sequencerUsesEdgesAndDurations()
{
    aquorbium::PhilosophySequencer sequencer;
    sequencer.setLength(3);
    (void)sequencer.setStep(0, { 0.0f, 0.0f, 2 });
    (void)sequencer.setStep(1, { 1.0f, 0.0f, 1 });
    (void)sequencer.setStep(2, { 0.0f, 1.0f, 1 });
    sequencer.reset();
    if (sequencer.tick(true).index != 0 || sequencer.tick(true).advanced)
        return false;
    if (sequencer.tick(false).index != 0)
        return false;
    const auto secondEdge = sequencer.tick(true);
    if (!secondEdge.advanced || secondEdge.index != 1
        || secondEdge.weights[1] != 1.0f)
        return false;
    (void)sequencer.tick(false);
    const auto thirdEdge = sequencer.tick(true);
    return thirdEdge.advanced && thirdEdge.index == 2
        && thirdEdge.weights[2] == 1.0f;
}

bool discreteMorphIsContinuousAndExact()
{
    using Values = aquorbium::PhilosophyVectorMixer::Values;
    aquorbium::PhilosophyMorph morph;
    morph.prepare(48000.0);
    morph.setTransitionSeconds(0.001f);
    morph.reset(0);
    morph.setTarget(3);
    const Values sources { 0.0f, 1.0f, 2.0f, 4.0f };
    auto previous = -1.0f;
    aquorbium::PhilosophyMorph::Output output;
    for (int sample = 0; sample < 48; ++sample)
    {
        output = morph.tick(sources);
        auto sum = 0.0f;
        for (const auto weight : output.weights)
            sum += weight;
        if (std::abs(sum - 1.0f) > 1.0e-6f || output.mix < previous)
            return false;
        previous = output.mix;
    }
    return !output.active && output.weights[3] == 1.0f
        && output.mix == 4.0f;
}

bool matrixCreatesIndependentStereoBuses()
{
    using Matrix = aquorbium::PhilosophyMatrixMixer;
    Matrix matrix;
    const std::array<float, 4> sources { 1.0f, 2.0f, 3.0f, 4.0f };
    const std::array<float, 4> zeroCv {};
    auto output = matrix.process(sources, zeroCv);
    if (output.channel != sources)
        return false;

    for (std::size_t source = 0; source < 4; ++source)
    {
        (void)matrix.setCell(0, source, { 1.0f, 0.0f });
        (void)matrix.setCell(2, source, { 0.0f, source == 3 ? 1.0f : 0.0f });
    }
    output = matrix.process(sources, { 0.0f, 0.0f, 0.0f, 1.0f });
    return output.channel[0] == 2.5f && output.normalization[0] == 0.25f
        && output.channel[1] == 2.0f && output.channel[2] == 4.0f
        && output.channel[3] == 4.0f;
}

bool invalidControlsAreBounded()
{
    aquorbium::PhilosophySequencer sequencer;
    sequencer.setLength(0);
    const auto invalidStep = sequencer.setStep(40, {});
    const auto validStep = sequencer.setStep(
        0, { std::numeric_limits<float>::quiet_NaN(), 4.0f, 0 });
    const auto output = sequencer.tick(false);

    aquorbium::PhilosophyMorph morph;
    morph.prepare(std::numeric_limits<double>::infinity());
    morph.setTransitionSeconds(std::numeric_limits<float>::quiet_NaN());
    morph.reset(99);

    aquorbium::PhilosophyMatrixMixer matrix;
    const auto rejected = matrix.setCell(9, 0, {});
    const auto accepted = matrix.setCell(
        0, 0, { 9.0f, std::numeric_limits<float>::quiet_NaN() });
    const auto cell = matrix.cell(0, 0);
    return sequencer.length() == 1 && !invalidStep && validStep
        && output.weights[2] == 1.0f && morph.target() == 3
        && morph.transitionSeconds() == 0.1f && !rejected && accepted
        && cell.gain == 1.0f && cell.modulationDepth == 0.0f;
}
}

int main()
{
    const std::pair<const char*, bool (*)()> cases[] {
        { "vector blend", vectorBlendMapsCornersAndCentre },
        { "sequencer", sequencerUsesEdgesAndDurations },
        { "morph", discreteMorphIsContinuousAndExact },
        { "matrix", matrixCreatesIndependentStereoBuses },
        { "limits", invalidControlsAreBounded },
    };
    for (const auto& [name, test] : cases)
    {
        if (!test())
        {
            std::cerr << name << " failed\n";
            return EXIT_FAILURE;
        }
    }
    std::cout << "Philosophy mix tests passed\n";
}
