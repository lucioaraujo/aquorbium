// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/AdvancedVoiceEcologyModules.h"

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

bool biomaBassIsDeterministicAndSelfRegulating()
{
    aquorbium::BiomaBassVoice first, second;
    for (auto* voice : { &first, &second })
    {
        voice->prepare(48000.0);
        voice->setSeed(8831u);
        voice->setParameters({ 58.27f, 1.7f, 1.0f, 42.0f, -19.0f,
                               0.85f, 0.52f, 0.78f, 4u, 0.9f });
    }
    auto energy = 0.0;
    auto sawNegativeFrequency = false;
    for (int sample = 0; sample < 96000; ++sample)
    {
        const auto a = first.tick();
        const auto b = second.tick();
        if (!std::isfinite(a) || std::abs(a) > 1.00001f
            || std::bit_cast<std::uint32_t>(a)
                != std::bit_cast<std::uint32_t>(b))
            return false;
        energy += static_cast<double>(a) * a;
        sawNegativeFrequency = sawNegativeFrequency
            || first.telemetry().instantaneousFrequencyHz < 0.0f;
    }
    const auto telemetry = first.telemetry();
    return energy > 1.0 && sawNegativeFrequency && telemetry.energy > 0.0f
        && telemetry.effectiveFold < first.parameters().fold
        && telemetry.effectiveDetuneCents < first.parameters().detuneCents;
}

bool schoolWaveRespondsToCut()
{
    aquorbium::SchoolWaveVoice neutral, cut;
    neutral.prepare(48000.0);
    cut.prepare(48000.0);
    neutral.setParameters({ 143.0f, 0.72f, 0.5f, 0.7f, 0.8f });
    cut.setParameters({ 143.0f, 0.72f, 0.92f, 0.7f, 0.8f });
    auto difference = 0.0;
    for (int sample = 0; sample < 24000; ++sample)
    {
        const auto a = neutral.tick(0.4f, 0.2f);
        const auto b = cut.tick(0.4f, 0.2f);
        if (!std::isfinite(a.left) || !std::isfinite(a.right)
            || std::abs(a.left) > 1.00001f || std::abs(a.right) > 1.00001f)
            return false;
        difference += std::abs(static_cast<double>(a.left - b.left))
            + std::abs(static_cast<double>(a.right - b.right));
    }
    return difference > 1.0;
}

bool rippleIsBoundedAndClustered()
{
    aquorbium::HarmonicRippleVoice voice;
    voice.prepare(48000.0);
    voice.setParameters({ 96.0f, 8u, 0.72f, 0.9f, 0.8f });
    auto energy = 0.0;
    for (int sample = 0; sample < 24000; ++sample)
    {
        const auto value = voice.tick();
        if (!std::isfinite(value) || std::abs(value) > 1.00001f) return false;
        energy += static_cast<double>(value) * value;
    }
    return energy > 1.0;
}

bool recentMixCapturesIncrementally()
{
    aquorbium::RecentMixWavetable wavetable;
    wavetable.prepare(48000.0);
    if (wavetable.triggerCapture()) return false;
    for (std::size_t sample = 0;
         sample < aquorbium::RecentMixWavetable::tableSize + 32u; ++sample)
        static_cast<void>(wavetable.tick(std::sin(2.0f * pi * 7.0f
            * static_cast<float>(sample)
            / static_cast<float>(aquorbium::RecentMixWavetable::tableSize))));
    if (!wavetable.triggerCapture()) return false;
    auto energy = 0.0;
    aquorbium::RecentMixWavetable::Output output;
    for (int sample = 0; sample < 4096; ++sample)
    {
        output = wavetable.tick(0.0f);
        energy += static_cast<double>(output.value) * output.value;
    }
    return output.ready && !output.capturing && energy > 0.01;
}

bool frequencyShiftHasNeutralBypass()
{
    aquorbium::FrequencyShifter bypass, shifted;
    bypass.prepare(48000.0);
    shifted.prepare(48000.0);
    bypass.setParameters({ 317.0f, 0.0f });
    shifted.setParameters({ 317.0f, 1.0f });
    auto difference = 0.0;
    for (int sample = 0; sample < 12000; ++sample)
    {
        const auto input = std::sin(2.0f * pi * 211.0f * sample / 48000.0f)
            * 0.7f;
        const auto dry = bypass.tick(input);
        const auto wet = shifted.tick(input);
        if (std::bit_cast<std::uint32_t>(dry)
                != std::bit_cast<std::uint32_t>(input)
            || !std::isfinite(wet) || std::abs(wet) > 1.00001f)
            return false;
        difference += std::abs(static_cast<double>(wet - input));
    }
    return difference > 1.0;
}

bool feedbackStateHasHysteresis()
{
    aquorbium::CrossFeedbackStateMachine machine;
    machine.prepare(48000.0);
    machine.setParameters({ 0.08f, 0.28f, 0.03f, 0.8f });
    auto changed = false;
    aquorbium::CrossFeedbackStateMachine::Output output;
    for (int sample = 0; sample < 48000; ++sample)
    {
        output = machine.tick(sample < 12000 ? 0.0f : 1.0f, 1.0f);
        changed = changed || output.changed;
    }
    return changed && output.state
        == aquorbium::CrossFeedbackStateMachine::State::overload
        && output.modulation > 0.0f;
}

bool spotlightPreservesBackground()
{
    const auto output = aquorbium::SpotlightMixer::process(
        { 1.0f, 0.6f, -0.4f, 0.3f, -0.2f }, 0.41f, 0.7f);
    return output.primary == 2u && std::isfinite(output.left)
        && std::isfinite(output.right) && output.left != output.right
        && std::abs(output.left) <= 1.0f && std::abs(output.right) <= 1.0f;
}

bool hungerIsDeterministicAndDensityDependent()
{
    aquorbium::HungerProcessor first, second, fed;
    for (auto* processor : { &first, &second })
    {
        processor->prepare(48000.0);
        processor->setSeed(5519u);
        processor->setParameters({ 1.0f, 0.95f, 0.9f });
    }
    fed.prepare(48000.0);
    fed.setParameters({ 0.0f, 0.95f, 0.9f });
    auto starvedEnergy = 0.0;
    auto fedEnergy = 0.0;
    for (int sample = 0; sample < 48000; ++sample)
    {
        const auto input = std::sin(2.0f * pi * 131.0f * sample / 48000.0f)
            * 0.8f;
        const auto a = first.tick(input);
        const auto b = second.tick(input);
        const auto c = fed.tick(input);
        if (std::bit_cast<std::uint32_t>(a)
            != std::bit_cast<std::uint32_t>(b)) return false;
        starvedEnergy += static_cast<double>(a) * a;
        fedEnergy += static_cast<double>(c) * c;
    }
    return starvedEnergy < fedEnergy * 0.75;
}

bool selfFeedbackIsNeutralAtZeroMix()
{
    aquorbium::BoundedSelfFeedback dry, wet;
    dry.prepare(48000.0);
    wet.prepare(48000.0);
    dry.setParameters({ 0.9f, 0.7f, 0.8f, 0.0f });
    wet.setParameters({ 0.9f, 0.7f, 0.8f, 1.0f });
    auto difference = 0.0;
    for (int sample = 0; sample < 16000; ++sample)
    {
        const auto input = sample % 257 == 0 ? 0.8f : 0.0f;
        const auto a = dry.tick(input);
        const auto b = wet.tick(input);
        if (std::bit_cast<std::uint32_t>(a)
                != std::bit_cast<std::uint32_t>(input)
            || !std::isfinite(b) || std::abs(b) > 1.00001f)
            return false;
        difference += std::abs(static_cast<double>(a - b));
    }
    return difference > 1.0;
}

bool waterInertiaIsBoundedAndAsymmetric()
{
    aquorbium::WaterInertia water;
    water.prepare(48000.0);
    water.setParameters({ 0.72f, 0.85f, 0.8f });
    auto rise = 0.0f;
    for (int sample = 0; sample < 4800; ++sample) rise = water.tick(1.0f);
    auto fall = rise;
    for (int sample = 0; sample < 4800; ++sample) fall = water.tick(-1.0f);
    return std::isfinite(rise) && std::isfinite(fall)
        && rise > 0.0f && fall > -1.0f && std::abs(rise + fall) > 0.02f;
}

bool invalidValuesRemainFinite()
{
    const auto nan = std::numeric_limits<float>::quiet_NaN();
    const auto infinity = std::numeric_limits<float>::infinity();
    aquorbium::BiomaBassVoice bass;
    bass.prepare(infinity);
    bass.setParameters({ nan, infinity, nan, infinity, nan, infinity,
                         nan, infinity, 999u, nan });
    aquorbium::SchoolWaveVoice school;
    school.prepare(infinity);
    school.setParameters({ nan, infinity, nan, infinity, nan });
    aquorbium::HarmonicRippleVoice ripple;
    ripple.prepare(infinity);
    ripple.setParameters({ nan, 999u, infinity, nan, infinity });
    aquorbium::FrequencyShifter shift;
    shift.prepare(infinity);
    shift.setParameters({ infinity, nan });
    aquorbium::BoundedSelfFeedback feedback;
    feedback.prepare(infinity);
    feedback.setParameters({ nan, infinity, nan, infinity });
    aquorbium::WaterInertia water;
    water.prepare(infinity);
    water.setParameters({ nan, infinity, nan });
    return std::isfinite(bass.tick())
        && std::isfinite(school.tick(nan, infinity).left)
        && std::isfinite(ripple.tick())
        && std::isfinite(shift.tick(infinity))
        && std::isfinite(feedback.tick(infinity))
        && std::isfinite(water.tick(infinity));
}
}

int main()
{
    const std::array tests {
        biomaBassIsDeterministicAndSelfRegulating,
        schoolWaveRespondsToCut,
        rippleIsBoundedAndClustered,
        recentMixCapturesIncrementally,
        frequencyShiftHasNeutralBypass,
        feedbackStateHasHysteresis,
        spotlightPreservesBackground,
        hungerIsDeterministicAndDensityDependent,
        selfFeedbackIsNeutralAtZeroMix,
        waterInertiaIsBoundedAndAsymmetric,
        invalidValuesRemainFinite
    };
    for (std::size_t index = 0; index < tests.size(); ++index)
    {
        if (!tests[index]())
        {
            std::cerr << "advanced voice/ecology test " << index + 1
                      << " failed\n";
            return EXIT_FAILURE;
        }
    }
    std::cout << "Advanced voice/ecology cohort: 10 components passed\n";
}
