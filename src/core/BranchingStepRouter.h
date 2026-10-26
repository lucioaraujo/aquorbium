// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include "core/ThresholdStepSelector.h"

#include <array>
#include <cstddef>
#include <cstdint>

namespace aquorbium
{
// Two-layer decision tree: the first signal selects a broad category and the
// second selects one variant inside it. At most 8 x 4 leaves remain bounded.
class BranchingStepRouter final
{
public:
    static constexpr std::size_t maximumCategories = 8;
    static constexpr std::size_t maximumVariants = 4;
    static constexpr std::size_t maximumLeaves = 32;

    struct Output
    {
        std::size_t category = 0;
        std::size_t variant = 0;
        std::size_t index = 0;
        bool changed = false;
        std::uint64_t transitionCount = 0;
    };

    BranchingStepRouter() noexcept;

    void setShape(std::size_t categories, std::size_t variants) noexcept;
    [[nodiscard]] bool setCategoryThresholds(
        const std::array<float, ThresholdStepSelector::maximumSteps>& values,
        std::size_t count) noexcept;
    [[nodiscard]] bool setVariantThresholds(
        const std::array<float, ThresholdStepSelector::maximumSteps>& values,
        std::size_t count) noexcept;
    void reset() noexcept;
    [[nodiscard]] Output select(float categorySignal,
                                float variantSignal) noexcept;

    [[nodiscard]] std::size_t categories() const noexcept { return categoryCount; }
    [[nodiscard]] std::size_t variants() const noexcept { return variantCount; }
    [[nodiscard]] std::size_t leaves() const noexcept
    {
        return categoryCount * variantCount;
    }

private:
    ThresholdStepSelector categorySelector;
    ThresholdStepSelector variantSelector;
    std::size_t categoryCount = 3;
    std::size_t variantCount = 3;
    std::size_t previousIndex = maximumLeaves;
    std::uint64_t transitions = 0;
};
}
