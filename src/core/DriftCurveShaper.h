// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include <cstdint>

namespace aquorbium
{
// Stateless normalized curve family with an independent stepped-to-continuous
// axis. It shapes control trajectories; it does not generate random values.
class DriftCurveShaper final
{
public:
    enum class Family : std::uint8_t
    {
        linear,
        exponential,
        inverseExponential,
        bezierEase,
        arch,
        peak
    };

    struct Parameters
    {
        float smoothness = 1.0f;
        float curvature = 0.5f;
        Family family = Family::bezierEase;
    };

    void setParameters(const Parameters& value) noexcept;
    [[nodiscard]] const Parameters& parameters() const noexcept { return state; }
    [[nodiscard]] float shape(float normalizedPhase) const noexcept;

private:
    [[nodiscard]] float continuousCurve(float phase) const noexcept;

    Parameters state;
};
}
