// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/GenerativeInteractionModules.h"

#include <array>
#include <bit>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>

namespace
{
constexpr float pi = 3.14159265358979323846f;

bool lSystemIsBoundedAndRepeatable()
{
    aquorbium::FixedLSystemGenerator first, second;
    if (!first.configure(2u, false) || !second.configure(2u, false)
        || first.length() != 49u || second.configure(4u, false)) return false;
    for (int event = 0; event < 200; ++event)
    {
        const auto a = first.advance();
        const auto b = second.advance();
        if (a.gate != b.gate || a.turn != b.turn || a.value != b.value
            || a.position != b.position) return false;
    }
    return true;
}

bool cellularAutomatonEvolvesDeterministically()
{
    aquorbium::CellularAutomatonSequencer first, second;
    first.configure(30u, 1u << 15u);
    second.configure(30u, 1u << 15u);
    auto changed = false;
    auto previous = std::uint32_t { 0 };
    for (int generation = 0; generation < 64; ++generation)
    {
        const auto a = first.advance();
        const auto b = second.advance();
        if (a.state != b.state || a.gate != b.gate || a.density != b.density
            || !std::isfinite(a.density) || a.density < 0.0f
            || a.density > 1.0f) return false;
        changed = changed || (generation > 0 && a.state != previous);
        previous = a.state;
    }
    return changed;
}

bool vocoderTransfersEnvelopeAndStaysBounded()
{
    aquorbium::SpectralTransferVocoder dry, wet;
    dry.prepare(48000.0);
    wet.prepare(48000.0);
    dry.setParameters({ 0.45f, 0.75f, 0.65f, 0.20f, 0.0f });
    wet.setParameters({ 0.45f, 0.75f, 0.65f, 0.20f, 1.0f });
    auto difference = 0.0;
    auto wetEnergy = 0.0;
    for (int sample = 0; sample < 48000; ++sample)
    {
        const auto envelope = sample % 6000 < 3000 ? 1.0f : 0.08f;
        const auto modulator = std::sin(2.0f * pi * 310.0f * sample / 48000.0f)
            * envelope;
        const auto carrier = std::sin(2.0f * pi * 137.0f * sample / 48000.0f)
            * 0.7f;
        const auto a = dry.tick(modulator, carrier);
        const auto b = wet.tick(modulator, carrier);
        if (std::bit_cast<std::uint32_t>(a)
                != std::bit_cast<std::uint32_t>(carrier)
            || !std::isfinite(b) || std::abs(b) > 1.00001f) return false;
        difference += std::abs(static_cast<double>(a - b));
        wetEnergy += static_cast<double>(b) * b;
    }
    return difference > 1.0 && wetEnergy > 0.001;
}

bool envelopeContagionReachesNeighbours()
{
    aquorbium::EnvelopeContagionBus bus;
    bus.prepare(48000.0);
    aquorbium::EnvelopeContagionBus::Members input {};
    input[0] = 1.0f;
    aquorbium::EnvelopeContagionBus::Members output {};
    for (int sample = 0; sample < 1000; ++sample)
        output = bus.process(input, 1.0f);
    return output[1] > 0.1f && output[5] > 0.1f
        && output[3] < output[1];
}

bool noInputBusIsDeterministicAndAlive()
{
    aquorbium::NoInputFeedbackBus first, second;
    for (auto* bus : { &first, &second })
    {
        bus->prepare(48000.0);
        bus->setSeed(7721u);
        bus->setParameters({ 0.96f, 0.78f, 0.58f, 0.8f });
    }
    auto energy = 0.0;
    for (int sample = 0; sample < 48000; ++sample)
    {
        const auto a = first.tick();
        const auto b = second.tick();
        if (std::bit_cast<std::uint32_t>(a.left)
                != std::bit_cast<std::uint32_t>(b.left)
            || std::bit_cast<std::uint32_t>(a.right)
                != std::bit_cast<std::uint32_t>(b.right)
            || !std::isfinite(a.left) || !std::isfinite(a.right)
            || std::abs(a.left) > 1.00001f || std::abs(a.right) > 1.00001f)
            return false;
        energy += static_cast<double>(a.left) * a.left
            + static_cast<double>(a.right) * a.right;
    }
    return energy > 0.01;
}

bool reactiveStrategyUsesThresholdsAndHysteresis()
{
    aquorbium::ReactiveThresholdStrategy strategy;
    strategy.setThresholds({ 0.1f, 0.2f, 0.3f, 0.4f,
                             0.5f, 0.6f, 0.7f }, 0.04f);
    strategy.reset(0.0f);
    const auto high = strategy.tick(0.76f);
    const auto held = strategy.tick(0.69f);
    const auto low = strategy.tick(0.01f);
    return high.state == 7u && high.changed && held.state == 7u
        && !held.changed && low.state == 0u && low.changed;
}

bool lockInFocusFavoursReference()
{
    aquorbium::LockInFocusExtractor reference, distant;
    reference.prepare(48000.0);
    distant.prepare(48000.0);
    reference.setParameters({ 211.0f, 4.0f, 1.0f, 1.0f });
    distant.setParameters({ 211.0f, 4.0f, 1.0f, 1.0f });
    for (int sample = 0; sample < 96000; ++sample)
    {
        const auto atReference = std::sin(2.0f * pi * 211.0f
            * sample / 48000.0f) * 0.7f;
        const auto offReference = std::sin(2.0f * pi * 731.0f
            * sample / 48000.0f) * 0.7f;
        const auto a = reference.tick(atReference);
        const auto b = distant.tick(offReference);
        if (!std::isfinite(a) || !std::isfinite(b)
            || std::abs(a) > 1.00001f || std::abs(b) > 1.00001f) return false;
    }
    return reference.magnitude() > distant.magnitude() * 8.0f;
}

bool behaviourScenesRecallSmoothly()
{
    aquorbium::BehaviourSceneBank bank;
    bank.prepare(48000.0);
    if (!bank.setScene(0, { 0.0f, 0.0f, 0.0f, 0.0f })
        || !bank.setScene(3, { 1.0f, 0.8f, 0.6f, 0.4f })) return false;
    bank.reset();
    if (!bank.recall(3, 0.02f) || bank.recall(99, 0.1f)) return false;
    const auto first = bank.tick();
    aquorbium::BehaviourSceneBank::Scene last {};
    for (int sample = 0; sample < 9600; ++sample) last = bank.tick();
    return first[0] > 0.0f && first[0] < 1.0f && last[0] > 0.99f
        && last[3] > 0.39f && bank.currentScene() == 3u;
}

bool resourcePoolSupportsFourModes()
{
    aquorbium::ReconfigurableResourcePool pool;
    for (std::size_t index = 0;
         index < aquorbium::ReconfigurableResourcePool::valueCount; ++index)
        if (!pool.setValue(index, static_cast<float>(index) / 15.0f * 2.0f - 1.0f))
            return false;
    pool.setMode(aquorbium::ReconfigurableResourcePool::Mode::mixer);
    const auto mixed = pool.tick(0.0f, 0.5f, false);
    pool.setMode(aquorbium::ReconfigurableResourcePool::Mode::switcher);
    const auto switched = pool.tick(0.0f, 0.5f, false);
    pool.setMode(aquorbium::ReconfigurableResourcePool::Mode::router);
    const auto routed = pool.tick(0.0f, 0.5f, false);
    pool.setMode(aquorbium::ReconfigurableResourcePool::Mode::recorder);
    const auto recorded = pool.tick(0.73f, 0.5f, true);
    return mixed != switched && routed != switched
        && std::abs(recorded - 0.73f) < 1.0e-6f;
}

bool binauralStageHasExactBypassAndSpatialDifference()
{
    aquorbium::LightweightBinauralSpatializer dry, wet;
    dry.prepare(48000.0);
    wet.prepare(48000.0);
    dry.setParameters({ 0.8f, 0.4f, 0.3f, 0.0f });
    wet.setParameters({ 0.8f, 0.4f, 0.3f, 1.0f });
    auto difference = 0.0;
    for (int sample = 0; sample < 24000; ++sample)
    {
        const auto input = std::sin(2.0f * pi * 1900.0f
            * sample / 48000.0f) * 0.6f;
        const auto a = dry.tick(input);
        const auto b = wet.tick(input);
        if (std::bit_cast<std::uint32_t>(a.left)
                != std::bit_cast<std::uint32_t>(input)
            || std::bit_cast<std::uint32_t>(a.right)
                != std::bit_cast<std::uint32_t>(input)
            || !std::isfinite(b.left) || !std::isfinite(b.right)
            || std::abs(b.left) > 1.00001f || std::abs(b.right) > 1.00001f)
            return false;
        difference += std::abs(static_cast<double>(b.left - b.right));
    }
    return difference > 1.0;
}

bool invalidParametersStayFinite()
{
    const auto nan = std::numeric_limits<float>::quiet_NaN();
    aquorbium::SpectralTransferVocoder vocoder;
    aquorbium::LockInFocusExtractor lockIn;
    aquorbium::LightweightBinauralSpatializer binaural;
    vocoder.prepare(nan);
    lockIn.prepare(nan);
    binaural.prepare(nan);
    vocoder.setParameters({ nan, nan, nan, nan, nan });
    lockIn.setParameters({ nan, nan, nan, nan });
    binaural.setParameters({ nan, nan, nan, nan });
    const auto a = vocoder.tick(nan, nan);
    const auto b = lockIn.tick(nan);
    const auto c = binaural.tick(nan);
    return std::isfinite(a) && std::isfinite(b)
        && std::isfinite(c.left) && std::isfinite(c.right);
}
}

int main()
{
    struct Test { const char* name; bool (*run)(); };
    const std::array tests {
        Test { "l-system", lSystemIsBoundedAndRepeatable },
        Test { "cellular automaton", cellularAutomatonEvolvesDeterministically },
        Test { "spectral transfer", vocoderTransfersEnvelopeAndStaysBounded },
        Test { "envelope contagion", envelopeContagionReachesNeighbours },
        Test { "no-input bus", noInputBusIsDeterministicAndAlive },
        Test { "reactive thresholds", reactiveStrategyUsesThresholdsAndHysteresis },
        Test { "lock-in focus", lockInFocusFavoursReference },
        Test { "behaviour scenes", behaviourScenesRecallSmoothly },
        Test { "resource modes", resourcePoolSupportsFourModes },
        Test { "binaural", binauralStageHasExactBypassAndSpatialDifference },
        Test { "invalid values", invalidParametersStayFinite }
    };
    for (const auto test : tests)
        if (!test.run())
        {
            std::cerr << "Failed: " << test.name << '\n';
            return EXIT_FAILURE;
        }
    std::cout << "Generative interaction cohort tests passed\n";
    return EXIT_SUCCESS;
}
