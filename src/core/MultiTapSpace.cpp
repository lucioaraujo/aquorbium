// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/MultiTapSpace.h"

#include "core/Numeric.h"

#include <algorithm>
#include <cmath>

namespace aquorbium
{
namespace
{
constexpr float pi = 3.14159265358979323846f;
constexpr float interpolationGainBound = 1.25f;
constexpr std::size_t delayMask = MultiTapSpace::delayCapacity - 1;

float interpolate(float left, float right, float amount) noexcept
{
    return left + (right - left) * amount;
}

float cleanDenormal(float value) noexcept
{
    return std::abs(value) < 1.0e-20f ? 0.0f : value;
}
}

MultiTapSpace::Parameters MultiTapSpace::aquariumEarlyReflections() noexcept
{
    Parameters result;
    result.taps = {{
        { 17.3f, 0.46f, -0.82f, 14800.0f, true },
        { 29.7f, 0.38f,  0.68f, 12600.0f, true },
        { 47.1f, 0.31f, -0.34f, 10100.0f, true },
        { 73.9f, 0.25f,  0.86f,  8100.0f, true },
        { 109.4f, 0.21f, -0.69f,  6500.0f, true },
        { 163.7f, 0.17f,  0.42f,  5100.0f, true },
        { 241.3f, 0.13f, -0.18f,  3900.0f, true },
        { 367.9f, 0.10f,  0.73f,  2900.0f, true }
    }};
    result.feedback = 0.31f;
    return result;
}

void MultiTapSpace::prepare(double sampleRate) noexcept
{
    preparedSampleRate = numeric::sampleRate(sampleRate);
    delayDrift.prepare(preparedSampleRate);
    parameterSmoothing = static_cast<float>(
        1.0 - std::exp(-1.0 / (preparedSampleRate * 0.035)));
    delaySmoothing = static_cast<float>(
        1.0 - std::exp(-1.0 / (preparedSampleRate * 0.140)));
    feedbackSmoothing = static_cast<float>(
        1.0 - std::exp(-1.0 / (preparedSampleRate * 0.090)));
    setDriftIntent(driftIntent);
    updateTargets();
    reset();
}

void MultiTapSpace::reset() noexcept
{
    leftDelay.fill(0.0f);
    rightDelay.fill(0.0f);
    writeIndex = 0;
    previousFeedbackLeft = 0.0f;
    previousFeedbackRight = 0.0f;
    currentWetEnergy = 0.0f;
    currentDriftSamples = 0.0f;
    delayDrift.reset();
    lastOutputNormalization = 1.0f;
    for (auto& state : tapStates)
        state.filterState = 0.0f;
    snapCurrentStateToTargets();
    hasProcessed = false;
}

void MultiTapSpace::setParameters(const Parameters& value) noexcept
{
    const auto maximumCutoff = std::max(
        80.0f, std::min(22000.0f, static_cast<float>(preparedSampleRate * 0.45)));
    const auto maximumDelayMilliseconds = static_cast<float>(
        static_cast<double>(delayCapacity - 3) * 1000.0 / preparedSampleRate);
    const auto minimumDelayMilliseconds = static_cast<float>(
        2.0 * 1000.0 / preparedSampleRate);

    for (std::size_t index = 0; index < tapCount; ++index)
    {
        auto& destination = generative.taps[index];
        const auto& source = value.taps[index];
        destination.delayMilliseconds = retained(
            source.delayMilliseconds, destination.delayMilliseconds,
            minimumDelayMilliseconds, maximumDelayMilliseconds);
        destination.gain = retained(source.gain, destination.gain, 0.0f, 1.0f);
        destination.pan = retained(source.pan, destination.pan, -1.0f, 1.0f);
        destination.cutoffHz = retained(
            source.cutoffHz, destination.cutoffHz, 80.0f, maximumCutoff);
        destination.enabled = source.enabled;
    }
    generative.feedback = retained(value.feedback, generative.feedback, 0.0f, 0.88f);
    updateTargets();
    if (!hasProcessed)
        snapCurrentStateToTargets();
}

void MultiTapSpace::setPerformanceIntent(const PerformanceIntent& value) noexcept
{
    performance.influence = numeric::unit(value.influence, performance.influence);
    performance.size = numeric::unit(value.size, performance.size);
    performance.spread = numeric::unit(value.spread, performance.spread);
    performance.tone = numeric::unit(value.tone, performance.tone);
    performance.memory = numeric::unit(value.memory, performance.memory);
    updateTargets();
    if (!hasProcessed)
        snapCurrentStateToTargets();
}

void MultiTapSpace::setDriftIntent(const DriftIntent& value) noexcept
{
    driftIntent.amount = numeric::unit(value.amount, driftIntent.amount);
    driftIntent.rateHz = numeric::clampFinite(
        value.rateHz, 0.01f, 2.0f, driftIntent.rateHz);
    driftIntent.depthMilliseconds = numeric::clampFinite(
        value.depthMilliseconds, 0.0f, 4.0f,
        driftIntent.depthMilliseconds);
    targetDriftDepthSamples = driftIntent.amount
        * driftIntent.depthMilliseconds
        * static_cast<float>(preparedSampleRate / 1000.0);
    delayDrift.setParameters({ driftIntent.rateHz, 0.28f,
                               std::min(1.8f, driftIntent.rateHz * 0.65f + 0.08f) });
}

void MultiTapSpace::setSeed(std::uint32_t value) noexcept
{
    delayDrift.setSeed((value == 0u ? 1u : value) ^ 0x7d4a21c3u);
}

MultiTapSpace::StereoSample MultiTapSpace::tick(
    float inputLeft, float inputRight) noexcept
{
    hasProcessed = true;
    const auto safeLeft = std::isfinite(inputLeft) ? inputLeft : 0.0f;
    const auto safeRight = std::isfinite(inputRight) ? inputRight : 0.0f;
    currentDriftSamples += (targetDriftDepthSamples - currentDriftSamples)
        * parameterSmoothing;
    const auto driftValue = delayDrift.tick();

    // Cross-coupled feedback distributes returning energy without changing
    // the row-sum bound of the matrix.
    const auto feedbackLeft = previousFeedbackLeft * 0.30f
        + previousFeedbackRight * 0.70f;
    const auto feedbackRight = previousFeedbackRight * 0.30f
        + previousFeedbackLeft * 0.70f;
    leftDelay[writeIndex] = safeLeft + feedbackLeft * currentFeedback;
    rightDelay[writeIndex] = safeRight + feedbackRight * currentFeedback;

    float wetLeft = 0.0f;
    float wetRight = 0.0f;
    float gainBudget = 0.0f;
    for (std::size_t index = 0; index < tapStates.size(); ++index)
    {
        auto& state = tapStates[index];
        const auto driftPolarity = index % 2u == 0u ? 1.0f : -1.0f;
        const auto driftSpread = 0.42f
            + 0.58f * static_cast<float>(index + 1u)
                / static_cast<float>(tapStates.size());
        const auto modulatedTarget = std::clamp(
            state.targetDelaySamples
                + driftValue * currentDriftSamples * driftPolarity * driftSpread,
            2.0f, static_cast<float>(delayCapacity - 3u));
        const auto delayStep = std::clamp(
            (modulatedTarget - state.currentDelaySamples) * delaySmoothing,
            -0.20f, 0.20f);
        state.currentDelaySamples += delayStep;
        state.currentLeftGain += (state.targetLeftGain - state.currentLeftGain)
            * parameterSmoothing;
        state.currentRightGain += (state.targetRightGain - state.currentRightGain)
            * parameterSmoothing;
        state.currentLowpassPole +=
            (state.targetLowpassPole - state.currentLowpassPole) * parameterSmoothing;
        state.currentSourceRightWeight +=
            (state.targetSourceRightWeight - state.currentSourceRightWeight)
            * parameterSmoothing;

        const auto delayedLeft = readInterpolated(leftDelay, state.currentDelaySamples);
        const auto delayedRight = readInterpolated(rightDelay, state.currentDelaySamples);
        const auto reflected = delayedLeft * (1.0f - state.currentSourceRightWeight)
            + delayedRight * state.currentSourceRightWeight;
        state.filterState = reflected * (1.0f - state.currentLowpassPole)
            + state.filterState * state.currentLowpassPole;
        state.filterState = cleanDenormal(state.filterState);

        wetLeft += state.filterState * state.currentLeftGain;
        wetRight += state.filterState * state.currentRightGain;
        gainBudget += std::max(std::abs(state.currentLeftGain),
                              std::abs(state.currentRightGain));
    }

    // Cubic Lagrange interpolation can have an L1 gain of 1.25. Including
    // that bound here makes coincident taps and the recursive path safe by
    // construction, independent of the authored tap budget.
    lastOutputNormalization = 1.0f
        / std::max(1.0f, interpolationGainBound * gainBudget);
    wetLeft *= lastOutputNormalization;
    wetRight *= lastOutputNormalization;

    constexpr float feedbackPathCeiling = 0.78f;
    const auto feedbackNormalization = feedbackPathCeiling
        / std::max(feedbackPathCeiling, interpolationGainBound * gainBudget);
    previousFeedbackLeft = cleanDenormal(wetLeft * feedbackNormalization
                                         / lastOutputNormalization);
    previousFeedbackRight = cleanDenormal(wetRight * feedbackNormalization
                                          / lastOutputNormalization);
    currentFeedback += (targetFeedback - currentFeedback) * feedbackSmoothing;

    currentWetEnergy += ((std::abs(wetLeft) + std::abs(wetRight)) * 0.5f
                         - currentWetEnergy) * 0.001f;
    currentWetEnergy = cleanDenormal(currentWetEnergy);
    writeIndex = (writeIndex + 1) & delayMask;

    if (!std::isfinite(wetLeft) || !std::isfinite(wetRight)
        || !std::isfinite(currentWetEnergy))
    {
        reset();
        return {};
    }
    return { wetLeft, wetRight };
}

MultiTapSpace::Telemetry MultiTapSpace::telemetry() const noexcept
{
    Telemetry result;
    result.targetFeedback = targetFeedback;
    result.currentFeedback = currentFeedback;
    result.wetEnergy = currentWetEnergy;
    result.outputNormalization = lastOutputNormalization;
    result.driftValue = delayDrift.value();
    result.driftDepthMilliseconds = currentDriftSamples * 1000.0f
        / static_cast<float>(preparedSampleRate);
    for (std::size_t index = 0; index < tapCount; ++index)
    {
        const auto& state = tapStates[index];
        auto& tap = result.taps[index];
        tap.targetDelayMilliseconds = static_cast<float>(
            static_cast<double>(state.targetDelaySamples) * 1000.0
            / preparedSampleRate);
        tap.currentDelayMilliseconds = static_cast<float>(
            static_cast<double>(state.currentDelaySamples) * 1000.0
            / preparedSampleRate);
        tap.currentGain = std::sqrt(state.currentLeftGain * state.currentLeftGain
                                    + state.currentRightGain * state.currentRightGain);
        const auto angle = tap.currentGain > 1.0e-9f
            ? std::atan2(state.currentRightGain, state.currentLeftGain)
            : pi * 0.25f;
        tap.currentPan = std::clamp(angle / (pi * 0.25f) - 1.0f, -1.0f, 1.0f);
        tap.currentCutoffHz = static_cast<float>(
            -std::log(std::clamp(state.currentLowpassPole, 1.0e-9f, 0.999999f))
            * preparedSampleRate / (2.0 * static_cast<double>(pi)));
        tap.enabled = state.enabled;
        if (state.enabled)
            ++result.activeTaps;
    }
    return result;
}

void MultiTapSpace::updateTargets() noexcept
{
    const auto amount = performance.influence;
    const auto performanceSize = std::exp2((performance.size - 0.5f) * 1.7f);
    const auto sizeScale = interpolate(1.0f, performanceSize, amount);
    const auto performanceSpread = 0.15f + performance.spread * 0.85f;
    const auto spreadScale = interpolate(1.0f, performanceSpread, amount);
    const auto toneScale = std::exp2((performance.tone - 0.5f) * 4.0f * amount);
    const auto maximumDelay = static_cast<float>(delayCapacity - 3);
    const auto maximumCutoff = std::max(
        80.0f, std::min(22000.0f, static_cast<float>(preparedSampleRate * 0.45)));

    for (std::size_t index = 0; index < tapCount; ++index)
    {
        const auto& tap = generative.taps[index];
        auto& state = tapStates[index];
        state.enabled = tap.enabled;
        state.targetDelaySamples = std::clamp(
            static_cast<float>(static_cast<double>(tap.delayMilliseconds)
                               * preparedSampleRate / 1000.0)
                * sizeScale,
            2.0f, maximumDelay);
        const auto effectivePan = std::clamp(tap.pan * spreadScale, -1.0f, 1.0f);
        const auto position = (effectivePan + 1.0f) * pi * 0.25f;
        const auto gain = tap.enabled ? tap.gain : 0.0f;
        state.targetLeftGain = std::cos(position) * gain;
        state.targetRightGain = std::sin(position) * gain;
        state.targetSourceRightWeight = 0.5f + effectivePan * 0.35f;
        const auto cutoff = std::clamp(tap.cutoffHz * toneScale, 80.0f, maximumCutoff);
        state.targetLowpassPole = static_cast<float>(
            std::exp(-2.0 * static_cast<double>(pi) * cutoff / preparedSampleRate));
    }
    targetFeedback = interpolate(
        generative.feedback, performance.memory * 0.88f, amount);
}

void MultiTapSpace::snapCurrentStateToTargets() noexcept
{
    for (auto& state : tapStates)
    {
        state.currentDelaySamples = state.targetDelaySamples;
        state.currentLeftGain = state.targetLeftGain;
        state.currentRightGain = state.targetRightGain;
        state.currentLowpassPole = state.targetLowpassPole;
        state.currentSourceRightWeight = state.targetSourceRightWeight;
    }
    currentFeedback = targetFeedback;
}

float MultiTapSpace::readInterpolated(
    const std::array<float, delayCapacity>& line, float delaySamples) const noexcept
{
    auto readPosition = static_cast<float>(writeIndex) - delaySamples;
    if (readPosition < 0.0f)
        readPosition += static_cast<float>(delayCapacity);
    const auto centre = static_cast<std::size_t>(std::floor(readPosition));
    const auto fraction = readPosition - static_cast<float>(centre);

    const auto previous = line[(centre + delayCapacity - 1) & delayMask];
    const auto current = line[centre & delayMask];
    const auto next = line[(centre + 1) & delayMask];
    const auto following = line[(centre + 2) & delayMask];

    const auto c0 = -fraction * (fraction - 1.0f) * (fraction - 2.0f) / 6.0f;
    const auto c1 = (fraction + 1.0f) * (fraction - 1.0f)
        * (fraction - 2.0f) * 0.5f;
    const auto c2 = -(fraction + 1.0f) * fraction * (fraction - 2.0f) * 0.5f;
    const auto c3 = (fraction + 1.0f) * fraction * (fraction - 1.0f) / 6.0f;
    return previous * c0 + current * c1 + next * c2 + following * c3;
}

float MultiTapSpace::retained(float candidate, float previous,
                              float minimum, float maximum) noexcept
{
    return std::isfinite(candidate)
        ? std::clamp(candidate, minimum, maximum) : previous;
}
}
