// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/ConditionalStageMachine.h"

#include <algorithm>
#include <cmath>

namespace aquorbium
{
ConditionalStageMachine::ConditionalStageMachine() noexcept
{
    for (std::size_t index = 0; index < stageCount; ++index)
    {
        stages[index].value = static_cast<float>(index) /
            static_cast<float>(stageCount - 1u);
        stages[index].next = (index + 1u) % stageCount;
        stages[index].jump = stages[index].next;
    }
}

void ConditionalStageMachine::setStage(std::size_t index, Stage value) noexcept
{
    if (index >= stageCount)
        return;
    value.value = std::clamp(
        std::isfinite(value.value) ? value.value : stages[index].value,
        -1.0f, 1.0f);
    value.durationTicks = std::clamp<std::uint32_t>(
        value.durationTicks, 1u, 1000000u);
    value.next %= stageCount;
    value.jump %= stageCount;
    value.threshold = std::clamp(
        std::isfinite(value.threshold) ? value.threshold
                                       : stages[index].threshold,
        0.0f, 1.0f);
    stages[index] = value;
}

void ConditionalStageMachine::reset(std::size_t initialStage) noexcept
{
    current = initialStage % stageCount;
    elapsed = 0;
}

ConditionalStageMachine::Output ConditionalStageMachine::tick(
    Context context) noexcept
{
    context = sanitized(context);
    const auto index = current;
    const auto& currentDefinition = stages[index];
    const auto entered = elapsed == 0;
    auto advanced = false;
    auto jumped = false;
    ++elapsed;
    if (elapsed >= currentDefinition.durationTicks)
    {
        jumped = conditionMet(currentDefinition, context);
        current = jumped ? currentDefinition.jump : currentDefinition.next;
        elapsed = 0;
        advanced = true;
    }
    return { currentDefinition.value, index, current, entered, advanced, jumped };
}

const ConditionalStageMachine::Stage& ConditionalStageMachine::stage(
    std::size_t index) const noexcept
{
    return stages[index % stageCount];
}

ConditionalStageMachine::Context ConditionalStageMachine::sanitized(
    Context value) noexcept
{
    value.danger = std::clamp(
        std::isfinite(value.danger) ? value.danger : 0.0f, 0.0f, 1.0f);
    value.crackStress = std::clamp(
        std::isfinite(value.crackStress) ? value.crackStress : 0.0f,
        0.0f, 1.0f);
    return value;
}

bool ConditionalStageMachine::conditionMet(
    const Stage& value, const Context& context) noexcept
{
    switch (value.condition)
    {
        case Condition::dangerAbove: return context.danger >= value.threshold;
        case Condition::night: return context.isNight;
        case Condition::crackAbove: return context.crackStress >= value.threshold;
        case Condition::none: break;
    }
    return false;
}
}
