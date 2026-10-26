// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include <cstdint>

namespace aquorbium
{
// Event-rate transposition memory. A normalized seasonal phase chooses the
// direction while each event accumulates a bounded offset in scale degrees.
class AccumulatingTransposer final
{
public:
    struct Parameters
    {
        int minimumDegree = -14;
        int maximumDegree = 14;
        int stepDegrees = 1;
    };

    struct Output
    {
        int degreeOffset = 0;
        int direction = 1;
        bool boundaryReached = false;
        std::uint64_t event = 0;
    };

    void setParameters(const Parameters& value) noexcept;
    void reset(int degreeOffset = 0) noexcept;
    [[nodiscard]] Output advance(float seasonPhase) noexcept;

    [[nodiscard]] const Parameters& parameters() const noexcept { return state; }
    [[nodiscard]] int currentDegree() const noexcept { return current; }

private:
    Parameters state;
    int current = 0;
    std::uint64_t eventCounter = 0;
};
}
