// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

namespace aquorbium
{
// A lightweight mono-to-stereo acoustic body for one organism. Ecological
// control remains outside this class; it receives bounded perceptual targets
// and performs only sample-rate smoothing, equal-power panning and depth tone.
class AquariumSpatializer final
{
public:
    struct Target
    {
        float pan = 0.0f;    // -1 left, +1 right
        float gain = 1.0f;   // bounded to the per-source headroom contract
        float depth = 0.0f;  // 0 open/near, 1 submerged/distant
        float width = 1.0f;  // 0 collapses to centre, 1 retains full pan
    };

    struct StereoSample
    {
        float left = 0.0f;
        float right = 0.0f;
    };

    struct Telemetry
    {
        Target target {};
        float effectivePan = 0.0f;
        float targetLeftGain = 0.0f;
        float targetRightGain = 0.0f;
        float currentLeftGain = 0.0f;
        float currentRightGain = 0.0f;
        float depthCutoffHz = 0.0f;
    };

    void prepare(double sampleRate) noexcept;
    void reset() noexcept;
    void setTarget(Target value) noexcept;
    // Accepts an already-budgeted L/R perspective. The expensive polar
    // conversion runs only when a control-rate value actually changes.
    void setGainPair(float left, float right, float depth, float width) noexcept;
    [[nodiscard]] StereoSample tick(float input) noexcept;
    [[nodiscard]] Telemetry telemetry() const noexcept;

private:
    void updateTargets() noexcept;

    double preparedSampleRate = 48000.0;
    Target target {};
    float targetLeftGain = 0.70710678f;
    float targetRightGain = 0.70710678f;
    float currentLeftGain = 0.0f;
    float currentRightGain = 0.0f;
    float targetLowpassPole = 0.0f;
    float currentLowpassPole = 0.0f;
    float lowpassState = 0.0f;
    float gainSmoothing = 0.001f;
    float toneSmoothing = 0.0005f;
    float effectivePan = 0.0f;
    float depthCutoffHz = 18000.0f;
    float pairLeft = 0.0f;
    float pairRight = 0.0f;
    float pairDepth = 0.0f;
    float pairWidth = 1.0f;
    bool pairInitialized = false;
};
}
