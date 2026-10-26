// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/AquariumSpatializer.h"

#include "core/Numeric.h"

#include <algorithm>
#include <cmath>

namespace aquorbium
{
namespace
{
constexpr float pi = 3.14159265358979323846f;

float retained(float candidate, float previous, float minimum, float maximum) noexcept
{
    return std::isfinite(candidate) ? std::clamp(candidate, minimum, maximum) : previous;
}
}

void AquariumSpatializer::prepare(double sampleRate) noexcept
{
    preparedSampleRate = numeric::sampleRate(sampleRate);
    gainSmoothing = static_cast<float>(
        1.0 - std::exp(-1.0 / (preparedSampleRate * 0.020)));
    toneSmoothing = static_cast<float>(
        1.0 - std::exp(-1.0 / (preparedSampleRate * 0.045)));
    updateTargets();
    reset();
}

void AquariumSpatializer::reset() noexcept
{
    currentLeftGain = 0.0f;
    currentRightGain = 0.0f;
    currentLowpassPole = targetLowpassPole;
    lowpassState = 0.0f;
}

void AquariumSpatializer::setTarget(Target value) noexcept
{
    const Target sanitized {
        retained(value.pan, target.pan, -1.0f, 1.0f),
        retained(value.gain, target.gain, 0.0f, 1.25f),
        retained(value.depth, target.depth, 0.0f, 1.0f),
        retained(value.width, target.width, 0.0f, 1.0f)
    };
    if (sanitized.pan == target.pan && sanitized.gain == target.gain
        && sanitized.depth == target.depth && sanitized.width == target.width)
        return;
    pairInitialized = false;
    target = sanitized;
    updateTargets();
}

void AquariumSpatializer::setGainPair(float left, float right, float depth, float width) noexcept
{
    if (!std::isfinite(left) || !std::isfinite(right)
        || !std::isfinite(depth) || !std::isfinite(width))
        return;
    const auto safeLeft = std::clamp(left, 0.0f, 1.25f);
    const auto safeRight = std::clamp(right, 0.0f, 1.25f);
    const auto safeDepth = std::clamp(depth, 0.0f, 1.0f);
    const auto safeWidth = std::clamp(width, 0.0f, 1.0f);
    if (pairInitialized && safeLeft == pairLeft && safeRight == pairRight
        && safeDepth == pairDepth && safeWidth == pairWidth)
        return;

    pairLeft = safeLeft;
    pairRight = safeRight;
    pairDepth = safeDepth;
    pairWidth = safeWidth;
    pairInitialized = true;
    const auto gain = std::sqrt(pairLeft * pairLeft + pairRight * pairRight);
    const auto angle = gain > 1.0e-7f ? std::atan2(pairRight, pairLeft)
                                      : pi * 0.25f;
    target = { std::clamp(angle / (pi * 0.25f) - 1.0f, -1.0f, 1.0f),
               std::min(gain, 1.25f), pairDepth, pairWidth };
    updateTargets();
}

void AquariumSpatializer::updateTargets() noexcept
{
    effectivePan = target.pan * target.width;
    const auto position = (effectivePan + 1.0f) * pi * 0.25f;
    targetLeftGain = std::cos(position) * target.gain;
    targetRightGain = std::sin(position) * target.gain;

    constexpr float minimumCutoff = 900.0f;
    constexpr float maximumCutoff = 18000.0f;
    const auto openCutoff = minimumCutoff
        * std::pow(maximumCutoff / minimumCutoff, 1.0f - target.depth);
    depthCutoffHz = std::min(openCutoff, static_cast<float>(preparedSampleRate * 0.45));
    targetLowpassPole = static_cast<float>(
        std::exp(-2.0 * static_cast<double>(pi) * depthCutoffHz / preparedSampleRate));
}

AquariumSpatializer::StereoSample AquariumSpatializer::tick(float input) noexcept
{
    const auto safeInput = std::isfinite(input) ? input : 0.0f;
    currentLeftGain += (targetLeftGain - currentLeftGain) * gainSmoothing;
    currentRightGain += (targetRightGain - currentRightGain) * gainSmoothing;
    currentLowpassPole += (targetLowpassPole - currentLowpassPole) * toneSmoothing;
    lowpassState = safeInput * (1.0f - currentLowpassPole)
        + lowpassState * currentLowpassPole;
    if (std::abs(lowpassState) < 1.0e-20f)
        lowpassState = 0.0f;
    if (targetLeftGain == 0.0f && std::abs(currentLeftGain) < 1.0e-20f)
        currentLeftGain = 0.0f;
    if (targetRightGain == 0.0f && std::abs(currentRightGain) < 1.0e-20f)
        currentRightGain = 0.0f;

    StereoSample result { lowpassState * currentLeftGain,
                          lowpassState * currentRightGain };
    if (!std::isfinite(result.left) || !std::isfinite(result.right)
        || !std::isfinite(lowpassState))
    {
        reset();
        return {};
    }
    return result;
}

AquariumSpatializer::Telemetry AquariumSpatializer::telemetry() const noexcept
{
    return { target, effectivePan, targetLeftGain, targetRightGain,
             currentLeftGain, currentRightGain, depthCutoffHz };
}
}
