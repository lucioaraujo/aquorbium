// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include <cstdint>

namespace aquorbium
{
// Makes one pass/mask decision at each pulse onset and holds that decision for
// the complete pulse, avoiding clicks from per-sample probability changes.
class StochasticPulseMask final
{
public:
    struct Output
    {
        bool gate = false;
        bool trigger = false;
        bool masked = false;
        std::uint64_t accepted = 0;
        std::uint64_t rejected = 0;
    };

    void setSeed(std::uint32_t value) noexcept;
    void reset() noexcept;
    [[nodiscard]] Output process(bool pulse, float passProbability) noexcept;

private:
    [[nodiscard]] float nextUnit() noexcept;

    std::uint32_t seed = 1u;
    std::uint32_t randomState = 1u;
    bool previousPulse = false;
    bool passCurrentPulse = false;
    std::uint64_t acceptedCount = 0;
    std::uint64_t rejectedCount = 0;
};
}
