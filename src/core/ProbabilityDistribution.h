// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include <cstdint>

namespace aquorbium
{
// Bounded morph between a flat distribution and a central-limit bell shape.
// Every sample consumes the same seven random values at every morph position.
class ProbabilityDistribution final
{
public:
    void setSeed(std::uint32_t value) noexcept;
    void reset() noexcept;
    [[nodiscard]] float sample(float gaussianAmount) noexcept;

private:
    [[nodiscard]] float nextSigned() noexcept;

    std::uint32_t seed = 1u;
    std::uint32_t randomState = 1u;
};
}
