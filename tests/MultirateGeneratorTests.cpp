// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/FractalCore.h"
#include "core/VectorGenerator.h"

#include <algorithm>
#include <bit>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <utility>

namespace
{
bool fractalIsDeterministicAndBounded()
{
    static_assert(noexcept(std::declval<aquorbium::FractalCore&>().tick()));
    aquorbium::FractalCore first, repeated;
    const aquorbium::FractalCore::Parameters parameters {
        713.0f, 2.37f, 0.78f, 7
    };
    for (auto* core : { &first, &repeated })
    {
        core->prepare(48000.0);
        core->setParameters(parameters);
        core->reset(0.37f);
    }
    for (int sample = 0; sample < 48000; ++sample)
    {
        const auto a = first.tick();
        const auto b = repeated.tick();
        if (std::bit_cast<std::uint32_t>(a)
                != std::bit_cast<std::uint32_t>(b)
            || !std::isfinite(a) || std::abs(a) > 1.0f)
            return false;
    }
    return true;
}

bool fractalUsesOneClockModelAtBothRates()
{
    aquorbium::FractalCore control, audio;
    const aquorbium::FractalCore::Parameters parameters {
        1.0f, 2.3f, 0.65f, 6
    };
    control.prepare(200.0);
    audio.prepare(2000.0);
    control.setParameters(parameters);
    audio.setParameters(parameters);
    auto accumulatedDifference = 0.0;
    auto divergentSamples = 0;
    for (int controlSample = 0; controlSample < 200; ++controlSample)
    {
        const auto slow = control.tick();
        const auto fast = audio.tick();
        const auto difference = std::abs(slow - fast);
        accumulatedDifference += difference;
        divergentSamples += difference > 0.01f ? 1 : 0;
        for (int subSample = 1; subSample < 10; ++subSample)
            (void)audio.tick();
    }
    const auto directPhaseDifference = std::abs(
        control.currentPhase() - audio.currentPhase());
    const auto phaseDifference = std::min(
        directPhaseDifference, 1.0f - directPhaseDifference);
    if (phaseDifference >= 2.0e-4f
        || accumulatedDifference / 200.0 >= 0.02 || divergentSamples > 8)
    {
        std::cerr << "multirate mean=" << accumulatedDifference / 200.0
                  << " divergent=" << divergentSamples
                  << " phase=" << phaseDifference << '\n';
        return false;
    }
    return true;
}

bool fractalIterationsChangeStructure()
{
    aquorbium::FractalCore simple, recursive;
    simple.prepare(48000.0);
    recursive.prepare(48000.0);
    simple.setParameters({ 190.0f, 2.0f, 0.8f, 1 });
    recursive.setParameters({ 190.0f, 2.0f, 0.8f, 8 });
    double difference = 0.0;
    for (int sample = 0; sample < 4096; ++sample)
        difference += std::abs(simple.tick() - recursive.tick());

    auto invalid = recursive.parameters();
    invalid.frequencyHz = std::numeric_limits<float>::quiet_NaN();
    invalid.replicationRatio = std::numeric_limits<float>::infinity();
    invalid.roughness = -4.0f;
    invalid.iterations = 999;
    recursive.setParameters(invalid);
    const auto sanitized = recursive.parameters();
    return difference > 10.0 && sanitized.frequencyHz == 190.0f
        && sanitized.replicationRatio == 2.0f
        && sanitized.roughness == 0.0f
        && sanitized.iterations == aquorbium::FractalCore::maximumIterations;
}

bool vectorOneShotCompletes()
{
    static_assert(noexcept(std::declval<aquorbium::VectorGenerator&>().tick()));
    aquorbium::VectorGenerator generator;
    generator.prepare(100.0);
    generator.setMode(aquorbium::VectorGenerator::Mode::oneShot);
    generator.setRateHz(10.0f);
    generator.setShape({ 0.5f, 0.0f, false });
    generator.trigger();
    auto peak = 0.0f;
    for (int sample = 0; sample < 10; ++sample)
        peak = std::max(peak, generator.tick());
    return peak > 0.99f && !generator.active() && generator.tick() == 0.0f;
}

bool vectorLoopsAtControlAndAudioRates()
{
    aquorbium::VectorGenerator control, audio;
    control.prepare(200.0);
    control.setRateHz(0.5f);
    control.setShape({ 0.3f, -0.6f, false });
    audio.prepare(48000.0);
    audio.setRateHz(440.0f);
    audio.setShape({ 0.63f, 0.75f, true });
    auto controlMovement = 0.0f;
    auto audioMovement = 0.0f;
    auto previousControl = control.tick();
    auto previousAudio = audio.tick();
    for (int sample = 0; sample < 48000; ++sample)
    {
        const auto controlValue = control.tick();
        const auto audioValue = audio.tick();
        if (!std::isfinite(controlValue) || controlValue < 0.0f
            || controlValue > 1.0f || !std::isfinite(audioValue)
            || std::abs(audioValue) > 1.0f)
            return false;
        controlMovement += std::abs(controlValue - previousControl);
        audioMovement += std::abs(audioValue - previousAudio);
        previousControl = controlValue;
        previousAudio = audioValue;
    }
    return controlMovement > 0.1f && audioMovement > 100.0f;
}

bool vectorSanitizesParameters()
{
    aquorbium::VectorGenerator generator;
    generator.prepare(std::numeric_limits<double>::quiet_NaN());
    generator.setRateHz(27.0f);
    generator.setRateHz(std::numeric_limits<float>::quiet_NaN());
    generator.setShape({ 0.0f, 8.0f, true });
    const auto shape = generator.shape();
    for (int sample = 0; sample < 1000; ++sample)
        if (!std::isfinite(generator.tick()))
            return false;
    return shape.skew == 0.01f && shape.curve == 1.0f && shape.bipolar;
}
}

int main()
{
    const std::pair<const char*, bool (*)()> cases[] {
        { "fractal deterministic", fractalIsDeterministicAndBounded },
        { "fractal multirate", fractalUsesOneClockModelAtBothRates },
        { "fractal iterations", fractalIterationsChangeStructure },
        { "vector one shot", vectorOneShotCompletes },
        { "vector roles", vectorLoopsAtControlAndAudioRates },
        { "vector limits", vectorSanitizesParameters },
    };
    for (const auto& [name, test] : cases)
    {
        if (!test())
        {
            std::cerr << name << " failed\n";
            return EXIT_FAILURE;
        }
    }
    std::cout << "FractalCore and VectorGenerator tests passed\n";
}
