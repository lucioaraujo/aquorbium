// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/EmergentSoundModules.h"
#include "core/RoutingResourceModules.h"

#include <array>
#include <bit>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <type_traits>

namespace
{
constexpr float pi = 3.14159265358979323846f;

bool reeseIsBoundedAndDeterministic()
{
    aquorbium::ReeseVoice first, second;
    for (auto* voice : { &first, &second })
    {
        voice->prepare(48000.0);
        voice->setSeed(9173u);
        voice->setParameters({ 73.42f, 31.0f, 0.72f });
    }
    auto energy = 0.0;
    for (int sample = 0; sample < 96000; ++sample)
    {
        const auto a = first.tick();
        const auto b = second.tick();
        if (!std::isfinite(a) || std::abs(a) > 1.00001f
            || std::bit_cast<std::uint32_t>(a)
                != std::bit_cast<std::uint32_t>(b))
            return false;
        energy += static_cast<double>(a) * a;
    }
    return energy > 1.0;
}

bool dualPeakHasTwoTopologies()
{
    aquorbium::DualPeakFilter parallel, serial;
    parallel.prepare(48000.0);
    serial.prepare(48000.0);
    parallel.setParameters({ 0.51f, 0.62f, 0.71f, 0.26f, 0.0f });
    serial.setParameters({ 0.51f, 0.62f, 0.71f, 0.26f, 1.0f });
    auto difference = 0.0;
    auto energy = 0.0;
    for (int sample = 0; sample < 24000; ++sample)
    {
        const auto input = std::sin(2.0f * pi * 311.0f
            * static_cast<float>(sample) / 48000.0f);
        const auto a = parallel.tick(input);
        const auto b = serial.tick(input);
        if (!std::isfinite(a) || !std::isfinite(b)
            || std::abs(a) > 1.00001f || std::abs(b) > 1.00001f)
            return false;
        difference += std::abs(static_cast<double>(a - b));
        energy += static_cast<double>(a) * a + static_cast<double>(b) * b;
    }
    return difference > 1.0 && energy > 1.0;
}

bool filterSequenceSelectsBands()
{
    aquorbium::ResonantFilterSequencer sequence;
    sequence.prepare(48000.0);
    sequence.setParameters({ 65.41f, 0.74f });
    aquorbium::ResonantFilterSequencer::Output first, fifth;
    auto energy = 0.0;
    for (int sample = 0; sample < 24000; ++sample)
    {
        const auto impulse = sample % 211 == 0 ? 1.0f : 0.0f;
        first = sequence.tick(impulse, 0.01f, 0.0f);
        fifth = sequence.tick(impulse, 0.51f, 1.0f);
        if (!std::isfinite(first.value) || !std::isfinite(fifth.value))
            return false;
        energy += static_cast<double>(first.value) * first.value
            + static_cast<double>(fifth.value) * fifth.value;
    }
    return first.primary == 0u && first.secondary == 4u
        && fifth.primary == 4u && fifth.secondary == 0u && energy > 0.01;
}

bool frequencyFocusHasNeutralBypass()
{
    aquorbium::FrequencyIsolator isolator;
    isolator.prepare(48000.0);
    isolator.setParameters({ 0.0f, 0.42f, 0.38f });
    for (int sample = 0; sample < 2048; ++sample)
    {
        const auto input = std::sin(2.0f * pi * 127.0f
            * static_cast<float>(sample) / 48000.0f) * 0.7f;
        if (std::bit_cast<std::uint32_t>(isolator.tick(input))
            != std::bit_cast<std::uint32_t>(input))
            return false;
    }
    isolator.reset();
    isolator.setParameters({ 1.0f, 0.42f, 0.18f });
    auto difference = 0.0;
    for (int sample = 0; sample < 4096; ++sample)
    {
        const auto input = std::sin(2.0f * pi * 127.0f
            * static_cast<float>(sample) / 48000.0f) * 0.7f;
        difference += std::abs(static_cast<double>(isolator.tick(input) - input));
    }
    return difference > 1.0;
}

bool interactionChangesWithModeAndDucksPrey()
{
    aquorbium::InteractionProcessor ring, logic;
    ring.prepare(48000.0);
    logic.prepare(48000.0);
    ring.setParameters({ 0.9f, 0.7f, 0.9f,
                         aquorbium::InteractionProcessor::Mode::ring });
    logic.setParameters({ 0.9f, 0.7f, 0.9f,
                          aquorbium::InteractionProcessor::Mode::xorLogic });
    auto difference = 0.0;
    auto preyEnergy = 0.0;
    auto dryEnergy = 0.0;
    for (int sample = 0; sample < 12000; ++sample)
    {
        const auto a = std::sin(2.0f * pi * 131.0f * sample / 48000.0f);
        const auto b = std::sin(2.0f * pi * 197.0f * sample / 48000.0f);
        const auto r = ring.tick(a, b, 1.0f, 1.0f);
        const auto x = logic.tick(a, b, 1.0f, 1.0f);
        difference += std::abs(static_cast<double>(r.artifact - x.artifact));
        if (sample > 6000)
        {
            preyEnergy += std::abs(static_cast<double>(r.prey));
            dryEnergy += std::abs(static_cast<double>(b));
        }
        if (!std::isfinite(r.mix) || !std::isfinite(x.mix)) return false;
    }
    return difference > 1.0 && preyEnergy < dryEnergy * 0.45;
}

bool powerStarveIsDeterministicAndResponsive()
{
    aquorbium::PowerStarve first, second;
    for (auto* starve : { &first, &second })
    {
        starve->prepare(48000.0);
        starve->setSeed(7129u);
        starve->setParameters({ 1.0f, 0.9f, 0.8f });
    }
    auto difference = 0.0;
    for (int sample = 0; sample < 24000; ++sample)
    {
        const auto input = std::sin(2.0f * pi * 239.0f * sample / 48000.0f)
            * 0.8f;
        const auto a = first.tick(input);
        const auto b = second.tick(input);
        if (std::bit_cast<std::uint32_t>(a)
                != std::bit_cast<std::uint32_t>(b)
            || !std::isfinite(a) || std::abs(a) > 1.00001f)
            return false;
        difference += std::abs(static_cast<double>(a - input));
    }
    return difference > 1.0;
}

bool quadraturePreservesPower()
{
    const auto output = aquorbium::QuadratureStereoRotator::process(
        0.3f, -0.4f, 0.173f);
    const auto inputPower = 0.3f * 0.3f + 0.4f * 0.4f;
    const auto outputPower = output.left * output.left
        + output.right * output.right;
    const auto quarter = aquorbium::QuadratureStereoRotator::process(
        1.0f, 0.0f, 0.25f);
    return std::abs(inputPower - outputPower) < 1.0e-5f
        && std::abs(quarter.left) < 1.0e-5f
        && std::abs(quarter.right - 1.0f) < 1.0e-5f;
}

bool polygonPublishesLiteralCoordinates()
{
    aquorbium::PolygonalOscillator polygon;
    polygon.prepare(48000.0);
    polygon.setParameters({ 173.0f, 5u, 0.0f, 0.13f });
    auto energy = 0.0;
    auto coordinateChange = 0.0;
    for (int sample = 0; sample < 12000; ++sample)
    {
        const auto point = polygon.tick();
        if (!std::isfinite(point.audio) || std::abs(point.x) > 1.00001f
            || std::abs(point.y) > 1.00001f)
            return false;
        energy += static_cast<double>(point.audio) * point.audio;
        coordinateChange += std::abs(point.x) + std::abs(point.y);
    }
    return energy > 1.0 && coordinateChange > 1.0;
}

bool routingMatrixIsBounded()
{
    aquorbium::BoundedRoutingMatrix matrix;
    for (std::size_t output = 0;
         output < aquorbium::BoundedRoutingMatrix::channelCount; ++output)
        for (std::size_t input = 0;
             input < aquorbium::BoundedRoutingMatrix::channelCount; ++input)
            if (!matrix.setCell(output, input,
                { output == input ? 1.0f : 0.75f,
                  (output + input) % 2u == 0u ? 0.8f : -0.8f }))
                return false;
    const auto result = matrix.process({ 4.0f, -4.0f, 2.0f, -2.0f,
                                         std::numeric_limits<float>::infinity() },
                                       1.0f);
    for (const auto value : result)
        if (!std::isfinite(value) || std::abs(value) > 1.00001f) return false;
    return !matrix.setCell(99u, 0u, {});
}

bool sharedPoolRoutesRotatesAndHolds()
{
    aquorbium::SharedParameterPool pool;
    for (std::size_t index = 0; index < aquorbium::SharedParameterPool::valueCount;
         ++index)
        if (!pool.setValue(index, static_cast<float>(index) / 15.0f))
            return false;
    if (!pool.setRoute(0, { 0u, 15u, 0.5f })) return false;
    pool.setMode(aquorbium::SharedParameterPool::Mode::direct);
    const auto direct = pool.read(0, 0.0f);
    pool.setMode(aquorbium::SharedParameterPool::Mode::rotate);
    const auto rotated = pool.read(0, 0.25f);
    pool.setMode(aquorbium::SharedParameterPool::Mode::hold);
    pool.reset();
    const auto held = pool.read(0, 0.0f, true);
    static_cast<void>(pool.setValue(0, -1.0f));
    const auto stillHeld = pool.read(0, 0.0f, false);
    return std::abs(direct - 0.5f) < 1.0e-6f
        && std::abs(rotated - direct) > 0.01f
        && held == stillHeld && !pool.setRoute(99u, {});
}

bool invalidInputsRemainContained()
{
    const auto nan = std::numeric_limits<float>::quiet_NaN();
    const auto infinity = std::numeric_limits<float>::infinity();

    aquorbium::ReeseVoice reese;
    reese.prepare(infinity);
    reese.setParameters({ nan, infinity, nan });
    if (!std::isfinite(reese.tick())) return false;

    aquorbium::DualPeakFilter peaks;
    peaks.prepare(infinity);
    peaks.setParameters({ nan, infinity, nan, infinity, nan });
    if (!std::isfinite(peaks.tick(infinity))) return false;

    aquorbium::ResonantFilterSequencer sequence;
    sequence.prepare(infinity);
    sequence.setParameters({ nan, infinity });
    if (!std::isfinite(sequence.tick(infinity, nan, infinity).value)) return false;

    aquorbium::FrequencyIsolator isolator;
    isolator.prepare(infinity);
    isolator.setParameters({ nan, infinity, nan });
    if (!std::isfinite(isolator.tick(infinity))) return false;

    aquorbium::InteractionProcessor interaction;
    interaction.prepare(infinity);
    interaction.setParameters({ nan, infinity, nan,
        aquorbium::InteractionProcessor::Mode::xorLogic });
    if (!std::isfinite(interaction.tick(infinity, nan, infinity, nan).mix))
        return false;

    aquorbium::PowerStarve starve;
    starve.prepare(infinity);
    starve.setParameters({ nan, infinity, nan });
    if (!std::isfinite(starve.tick(infinity))) return false;

    const auto rotated = aquorbium::QuadratureStereoRotator::process(
        infinity, nan, infinity);
    if (!std::isfinite(rotated.left) || !std::isfinite(rotated.right))
        return false;

    aquorbium::PolygonalOscillator polygon;
    polygon.prepare(infinity);
    polygon.setParameters({ nan, 999u, infinity, nan });
    const auto point = polygon.tick();
    if (!std::isfinite(point.audio) || !std::isfinite(point.x)
        || !std::isfinite(point.y) || polygon.parameters().vertices != 16u)
        return false;

    aquorbium::BoundedRoutingMatrix matrix;
    static_cast<void>(matrix.setCell(0, 0, { nan, infinity }));
    const auto routed = matrix.process({ infinity, nan, 0.0f, 0.0f, 0.0f }, nan);
    for (const auto value : routed)
        if (!std::isfinite(value)) return false;

    aquorbium::SharedParameterPool pool;
    static_cast<void>(pool.setValue(0, nan));
    static_cast<void>(pool.setRoute(0, { 0u, 0u, infinity }));
    return std::isfinite(pool.read(0, nan));
}
}

int main()
{
    static_assert(noexcept(aquorbium::QuadratureStereoRotator::process(
        0.0f, 0.0f, 0.0f)));
    const std::array tests {
        reeseIsBoundedAndDeterministic,
        dualPeakHasTwoTopologies,
        filterSequenceSelectsBands,
        frequencyFocusHasNeutralBypass,
        interactionChangesWithModeAndDucksPrey,
        powerStarveIsDeterministicAndResponsive,
        quadraturePreservesPower,
        polygonPublishesLiteralCoordinates,
        routingMatrixIsBounded,
        sharedPoolRoutesRotatesAndHolds,
        invalidInputsRemainContained
    };
    for (std::size_t index = 0; index < tests.size(); ++index)
    {
        if (!tests[index]())
        {
            std::cerr << "emergent module cohort test " << index + 1
                      << " failed\n";
            return EXIT_FAILURE;
        }
    }
    std::cout << "Emergent module cohort: 10 components passed\n";
}
