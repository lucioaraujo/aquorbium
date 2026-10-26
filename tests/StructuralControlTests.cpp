// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/ConditionalStageMachine.h"
#include "core/IntegrityCrossfade.h"
#include "core/TripleSlowDrift.h"

#include <bit>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <utility>

namespace
{
bool conditionalStagesFollowAndJump()
{
    using Machine = aquorbium::ConditionalStageMachine;
    static_assert(noexcept(std::declval<Machine&>().tick({})));
    Machine machine;
    machine.setStage(0, { -0.5f, 2, 1, 6,
                          Machine::Condition::dangerAbove, 0.7f });
    machine.reset();
    const auto first = machine.tick({ 0.9f, false, 0.0f });
    const auto jump = machine.tick({ 0.9f, false, 0.0f });
    if (!first.entered || first.advanced || !jump.advanced || !jump.jumped
        || jump.nextStage != 6 || machine.currentStage() != 6)
        return false;
    machine.reset();
    (void)machine.tick({ 0.2f, false, 0.0f });
    const auto normal = machine.tick({ 0.2f, false, 0.0f });
    return normal.advanced && !normal.jumped && normal.nextStage == 1;
}

bool conditionalContextsAreIndependent()
{
    using Machine = aquorbium::ConditionalStageMachine;
    Machine machine;
    machine.setStage(2, { 0.2f, 1, 3, 5, Machine::Condition::night, 0.5f });
    machine.setStage(3, { 0.3f, 1, 4, 7,
                          Machine::Condition::crackAbove, 0.6f });
    machine.reset(2);
    const auto night = machine.tick({ 0.0f, true, 0.0f });
    machine.reset(2);
    const auto day = machine.tick({ 0.0f, false, 1.0f });
    machine.reset(3);
    const auto crack = machine.tick({
        std::numeric_limits<float>::quiet_NaN(), false, 0.8f
    });
    return night.jumped && night.nextStage == 5 && !day.jumped
        && day.nextStage == 3 && crack.jumped && crack.nextStage == 7;
}

bool tripleDriftIsDeterministicAndBounded()
{
    static_assert(noexcept(std::declval<aquorbium::TripleSlowDrift&>().tick()));
    aquorbium::TripleSlowDrift first, repeated;
    for (auto* drift : { &first, &repeated })
    {
        drift->prepare(200.0);
        drift->setPeriodsSeconds({ 0.1f, 0.2f, 0.4f });
        drift->setWeights({ 0.5f, 0.3f, 0.2f });
        drift->setSeed(7123u);
    }
    auto moved = false;
    for (int sample = 0; sample < 4000; ++sample)
    {
        const auto a = first.tick();
        const auto b = repeated.tick();
        if (std::bit_cast<std::uint32_t>(a.combined)
                != std::bit_cast<std::uint32_t>(b.combined)
            || !std::isfinite(a.combined) || std::abs(a.combined) > 1.0f)
            return false;
        moved = moved || std::abs(a.combined) > 0.001f;
    }
    return moved;
}

bool tripleDriftSupportsLongPeriods()
{
    aquorbium::TripleSlowDrift drift;
    drift.prepare(200.0);
    drift.setPeriodsSeconds({ -2.0f, 60.0f,
        std::numeric_limits<float>::infinity() });
    drift.setWeights({ 0.0f, 0.0f, 0.0f });
    const auto periods = drift.periodsSeconds();
    const auto weights = drift.weights();
    return periods[0] == 0.1f && periods[1] == 60.0f
        && periods[2] == 1200.0f && weights[0] == 0.5f
        && weights[1] == 0.3f && weights[2] == 0.2f;
}

bool integrityCrossfadeIsSmoothAndEqualPower()
{
    static_assert(noexcept(std::declval<aquorbium::IntegrityCrossfade&>().tick()));
    aquorbium::IntegrityCrossfade fade;
    fade.prepare(8000.0);
    fade.setTransitionSeconds(0.1f);
    fade.setStress(1.0f);
    auto previous = 0.0f;
    for (int sample = 0; sample < 800; ++sample)
    {
        const auto output = fade.tick();
        const auto power = output.healthyGain * output.healthyGain
            + output.crackedGain * output.crackedGain;
        if (!std::isfinite(output.mix) || output.mix < previous
            || output.mix - previous > 0.08f || std::abs(power - 1.0f) > 1.0e-5f)
            return false;
        previous = output.mix;
    }
    return previous > 0.998f;
}

bool integrityCrossfadeMapsThresholds()
{
    aquorbium::IntegrityCrossfade fade;
    fade.prepare(48000.0);
    fade.setThresholds(0.25f, 0.75f);
    fade.reset(0.25f);
    if (fade.currentMix() != 0.0f)
        return false;
    fade.reset(0.5f);
    if (std::abs(fade.currentMix() - 0.5f) > 1.0e-6f)
        return false;
    fade.reset(0.75f);
    fade.setStress(std::numeric_limits<float>::quiet_NaN());
    return fade.currentMix() == 1.0f && fade.targetMix() == 1.0f;
}
}

int main()
{
    const std::pair<const char*, bool (*)()> cases[] {
        { "stage jump", conditionalStagesFollowAndJump },
        { "stage contexts", conditionalContextsAreIndependent },
        { "triple drift", tripleDriftIsDeterministicAndBounded },
        { "long drift", tripleDriftSupportsLongPeriods },
        { "integrity smooth", integrityCrossfadeIsSmoothAndEqualPower },
        { "integrity map", integrityCrossfadeMapsThresholds },
    };
    for (const auto& [name, test] : cases)
    {
        if (!test())
        {
            std::cerr << name << " failed\n";
            return EXIT_FAILURE;
        }
    }
    std::cout << "Structural control tests passed\n";
}
