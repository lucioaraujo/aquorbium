// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/BranchingStepRouter.h"

#include <algorithm>

namespace aquorbium
{
BranchingStepRouter::BranchingStepRouter() noexcept
{
    setShape(categoryCount, variantCount);
}

void BranchingStepRouter::setShape(std::size_t categories,
                                   std::size_t variants) noexcept
{
    categoryCount = std::clamp<std::size_t>(categories, 1, maximumCategories);
    variantCount = std::clamp<std::size_t>(variants, 1, maximumVariants);
    categorySelector.setLength(categoryCount);
    variantSelector.setLength(variantCount);
    reset();
}

bool BranchingStepRouter::setCategoryThresholds(
    const std::array<float, ThresholdStepSelector::maximumSteps>& values,
    std::size_t count) noexcept
{
    if (count > maximumCategories
        || !categorySelector.setThresholds(values, count))
        return false;
    categoryCount = count;
    reset();
    return true;
}

bool BranchingStepRouter::setVariantThresholds(
    const std::array<float, ThresholdStepSelector::maximumSteps>& values,
    std::size_t count) noexcept
{
    if (count > maximumVariants
        || !variantSelector.setThresholds(values, count))
        return false;
    variantCount = count;
    reset();
    return true;
}

void BranchingStepRouter::reset() noexcept
{
    categorySelector.reset();
    variantSelector.reset();
    previousIndex = maximumLeaves;
    transitions = 0;
}

BranchingStepRouter::Output BranchingStepRouter::select(
    float categorySignal, float variantSignal) noexcept
{
    const auto category = categorySelector.select(categorySignal).index;
    const auto variant = variantSelector.select(variantSignal).index;
    const auto index = category * variantCount + variant;

    Output result;
    result.category = category;
    result.variant = variant;
    result.index = index;
    result.changed = index != previousIndex;
    if (result.changed)
    {
        previousIndex = index;
        ++transitions;
    }
    result.transitionCount = transitions;
    return result;
}
}
