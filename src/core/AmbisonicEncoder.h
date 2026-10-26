// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include <cmath>

namespace aquorbium
{
// First-order Ambisonics in the AmbiX convention (2026-10-23): channels in
// ACN order (W, Y, Z, X) with SN3D normalisation, so W is the source itself
// and X, Y, Z its first-order directional parts. Azimuth counter-clockwise
// from the front (left positive), elevation up. References: C. Nachbar,
// F. Zotter, E. Deleflie and A. Sontacchi, "AmbiX - a suggested Ambisonics
// format", Ambisonics Symposium 2011; M. A. Gerzon, "Periphony: with-height
// sound reproduction", JAES 21(1), 1973.
struct FoaSample
{
    float w = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float x = 0.0f;

    FoaSample& operator+=(const FoaSample& other) noexcept
    {
        w += other.w;
        y += other.y;
        z += other.z;
        x += other.x;
        return *this;
    }
};

// One source panned into first order. The direction glides (~20 ms), so a
// moving organism does not step its directional gains.
class FoaPanner final
{
public:
    void prepare(double sampleRate) noexcept
    {
        smoothing = static_cast<float>(1.0 - std::exp(-1.0 / (sampleRate * 0.020)));
        reset();
    }
    void reset() noexcept
    {
        initialised = false;
        currentY = currentZ = currentX = 0.0f;
        targetY = targetZ = targetX = 0.0f;
    }
    // azimuthRight: radians, positive to the RIGHT (the Ecosystem's
    // listener convention); AmbiX turns it to left-positive
    void setDirection(float azimuthRight, float elevation) noexcept
    {
        if (!std::isfinite(azimuthRight) || !std::isfinite(elevation))
            return;
        const auto horizontal = std::cos(elevation);
        targetY = -std::sin(azimuthRight) * horizontal;
        targetZ = std::sin(elevation);
        targetX = std::cos(azimuthRight) * horizontal;
        if (!initialised)
        {
            currentY = targetY;
            currentZ = targetZ;
            currentX = targetX;
            initialised = true;
        }
    }
    [[nodiscard]] FoaSample tick(float mono) noexcept
    {
        currentY += (targetY - currentY) * smoothing;
        currentZ += (targetZ - currentZ) * smoothing;
        currentX += (targetX - currentX) * smoothing;
        return { mono, mono * currentY, mono * currentZ, mono * currentX };
    }

    // a stereo pair as two sources at +-30 degrees (the stereo speakers),
    // each at 1/sqrt(2), so W is (L + R) / sqrt(2) like a mono source's
    [[nodiscard]] static FoaSample encodeStereo(float left, float right) noexcept
    {
        constexpr float half = 0.70710678f;
        constexpr float sin30 = 0.5f, cos30 = 0.86602540f;
        const auto sum = (left + right) * half;
        const auto difference = (left - right) * half;
        return { sum, difference * sin30, 0.0f, sum * cos30 };
    }

private:
    float smoothing = 0.001f;
    float currentY = 0.0f, currentZ = 0.0f, currentX = 0.0f;
    float targetY = 0.0f, targetZ = 0.0f, targetX = 0.0f;
    bool initialised = false;
};
}
