// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include "core/SlewRateRandomWalk.h"

#include <array>
#include <cstddef>

namespace aquorbium
{
// A wet-only early-reflection body. Ecological and UI layers author a small
// parameter snapshot; this class owns only bounded, sample-rate DSP state.
class MultiTapSpace final
{
public:
    static constexpr std::size_t tapCount = 8;
    static constexpr std::size_t delayCapacity = 262144;
    static_assert((delayCapacity & (delayCapacity - 1)) == 0,
                  "MultiTapSpace ring capacity must remain a power of two");

    struct Tap
    {
        float delayMilliseconds = 40.0f;
        float gain = 0.0f;
        float pan = 0.0f;
        float cutoffHz = 12000.0f;
        bool enabled = false;
    };

    struct Parameters
    {
        std::array<Tap, tapCount> taps {};
        float feedback = 0.0f;
    };

    // Influence 0 is the autonomous snapshot, 1 is the complete performance
    // intent, and intermediate values form the hybrid mode.
    struct PerformanceIntent
    {
        float influence = 0.0f;
        float size = 0.5f;
        float spread = 1.0f;
        float tone = 0.5f;
        float memory = 0.35f;
    };

    struct DriftIntent
    {
        float amount = 0.0f;
        float rateHz = 0.25f;
        float depthMilliseconds = 0.8f;
    };

    struct StereoSample
    {
        float left = 0.0f;
        float right = 0.0f;
    };

    struct TapTelemetry
    {
        float targetDelayMilliseconds = 0.0f;
        float currentDelayMilliseconds = 0.0f;
        float currentGain = 0.0f;
        float currentPan = 0.0f;
        float currentCutoffHz = 0.0f;
        bool enabled = false;
    };

    struct Telemetry
    {
        std::array<TapTelemetry, tapCount> taps {};
        float targetFeedback = 0.0f;
        float currentFeedback = 0.0f;
        float wetEnergy = 0.0f;
        float outputNormalization = 1.0f;
        float driftValue = 0.0f;
        float driftDepthMilliseconds = 0.0f;
        std::size_t activeTaps = 0;
    };

    [[nodiscard]] static Parameters aquariumEarlyReflections() noexcept;

    void prepare(double sampleRate) noexcept;
    void reset() noexcept;
    void setParameters(const Parameters& value) noexcept;
    void setPerformanceIntent(const PerformanceIntent& value) noexcept;
    void setDriftIntent(const DriftIntent& value) noexcept;
    void setSeed(std::uint32_t value) noexcept;
    [[nodiscard]] StereoSample tick(float inputLeft, float inputRight) noexcept;
    [[nodiscard]] Telemetry telemetry() const noexcept;
    [[nodiscard]] const Parameters& parameters() const noexcept { return generative; }
    [[nodiscard]] const PerformanceIntent& performanceIntent() const noexcept
    {
        return performance;
    }

private:
    struct TapState
    {
        float targetDelaySamples = 2.0f;
        float currentDelaySamples = 2.0f;
        float targetLeftGain = 0.0f;
        float currentLeftGain = 0.0f;
        float targetRightGain = 0.0f;
        float currentRightGain = 0.0f;
        float targetLowpassPole = 0.0f;
        float currentLowpassPole = 0.0f;
        float targetSourceRightWeight = 0.5f;
        float currentSourceRightWeight = 0.5f;
        float filterState = 0.0f;
        bool enabled = false;
    };

    void updateTargets() noexcept;
    void snapCurrentStateToTargets() noexcept;
    [[nodiscard]] float readInterpolated(
        const std::array<float, delayCapacity>& line, float delaySamples) const noexcept;
    [[nodiscard]] static float retained(float candidate, float previous,
                                        float minimum, float maximum) noexcept;

    double preparedSampleRate = 48000.0;
    Parameters generative {};
    PerformanceIntent performance {};
    DriftIntent driftIntent {};
    std::array<TapState, tapCount> tapStates {};
    std::array<float, delayCapacity> leftDelay {};
    std::array<float, delayCapacity> rightDelay {};
    std::size_t writeIndex = 0;
    float targetFeedback = 0.0f;
    float currentFeedback = 0.0f;
    float previousFeedbackLeft = 0.0f;
    float previousFeedbackRight = 0.0f;
    float currentWetEnergy = 0.0f;
    float lastOutputNormalization = 1.0f;
    float parameterSmoothing = 0.001f;
    float delaySmoothing = 0.0002f;
    float feedbackSmoothing = 0.0004f;
    float currentDriftSamples = 0.0f;
    float targetDriftDepthSamples = 0.0f;
    SlewRateRandomWalk delayDrift;
    bool hasProcessed = false;
};
}
