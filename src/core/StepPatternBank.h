// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include "core/AddressableStepMemory.h"

#include <array>
#include <cstddef>
#include <cstdint>

namespace aquorbium
{
// Fixed snapshots and chain order for AddressableStepMemory. It deliberately
// contains no filesystem persistence; serialization belongs outside the
// event-rate core.
class StepPatternBank final
{
public:
    static constexpr std::size_t maximumSlots = 8;
    static constexpr std::size_t maximumChain = 16;

    struct ChainOutput
    {
        std::size_t slot = 0;
        std::size_t chainIndex = 0;
        bool wrapped = false;
        std::uint64_t event = 0;
    };

    [[nodiscard]] bool save(std::size_t slot,
                            const AddressableStepMemory& source) noexcept;
    [[nodiscard]] bool restore(std::size_t slot,
                               AddressableStepMemory& destination,
                               std::size_t startIndex = 0) const noexcept;
    [[nodiscard]] bool setChain(
        const std::array<std::uint8_t, maximumChain>& slots,
        std::size_t count) noexcept;
    void resetChain(std::size_t startIndex = 0) noexcept;
    [[nodiscard]] ChainOutput advanceChain() noexcept;

    [[nodiscard]] bool hasPattern(std::size_t slot) const noexcept;
    [[nodiscard]] std::size_t chainLength() const noexcept { return chainSize; }

private:
    struct Pattern
    {
        std::array<AddressableStepMemory::Step,
                   AddressableStepMemory::maximumSteps> steps {};
        std::size_t length = 1;
        bool valid = false;
    };

    std::array<Pattern, maximumSlots> patterns {};
    std::array<std::uint8_t, maximumChain> chain {};
    std::size_t chainSize = 0;
    std::size_t chainCursor = 0;
    std::uint64_t eventCounter = 0;
};
}
