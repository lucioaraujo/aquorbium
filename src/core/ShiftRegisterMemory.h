// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include <cstddef>
#include <cstdint>

namespace aquorbium
{
// Fixed binary memory. Mutation is supplied as an explicit control so ecology
// can later map calm or danger without coupling this primitive to Ecosystem.
class ShiftRegisterMemory final
{
public:
    static constexpr std::size_t maximumLength = 32;

    struct Output
    {
        bool outputBit = false;
        bool insertedBit = false;
        bool mutated = false;
        std::uint32_t pattern = 0;
    };

    void setLength(std::size_t value) noexcept;
    void setSeed(std::uint32_t value) noexcept;
    void reset(std::uint32_t pattern = 1u) noexcept;
    [[nodiscard]] Output advance(bool sourceBit,
                                 float mutationProbability) noexcept;

    [[nodiscard]] std::size_t length() const noexcept { return activeLength; }
    [[nodiscard]] std::uint32_t pattern() const noexcept { return bits; }

private:
    [[nodiscard]] std::uint32_t mask() const noexcept;
    [[nodiscard]] float nextUnit() noexcept;

    std::uint32_t bits = 1u;
    std::uint32_t seed = 1u;
    std::uint32_t randomState = 1u;
    std::size_t activeLength = 16;
};
}
