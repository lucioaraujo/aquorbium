// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include <cstddef>
#include <cstdint>

namespace aquorbium
{
// Stateless control-rate waveform bank. Identity mapping is deterministic and
// does not consume the Brain's random stream.
class MotionWaveBank final
{
public:
    enum class Shape : std::uint8_t
    {
        sine,
        triangle,
        smoothPulse,
        doubleSine,
        rampUp,
        rampDown,
        square,
        asymmetric,
        count
    };

    static constexpr std::size_t shapeCount =
        static_cast<std::size_t>(Shape::count);

    [[nodiscard]] static float evaluate(Shape shape, float phase) noexcept;
    [[nodiscard]] static Shape shapeForIdentity(std::uint32_t species,
                                                std::uint32_t strategy,
                                                std::uint32_t seed) noexcept;
};
}
