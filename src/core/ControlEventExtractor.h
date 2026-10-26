// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

namespace aquorbium
{
// Converts a bounded continuous signal into zero-crossing and energy events.
// It is control infrastructure; event meaning belongs to the ecology layer.
class ControlEventExtractor final
{
public:
    struct Parameters
    {
        float zeroHysteresis = 0.01f;
        float envelopeThreshold = 0.2f;
        float envelopeHysteresis = 0.04f;
        float attackMs = 8.0f;
        float releaseMs = 120.0f;
    };

    struct Events
    {
        bool risingZero = false;
        bool fallingZero = false;
        bool envelopeRise = false;
        bool envelopeFall = false;
        bool envelopeGate = false;
        float envelope = 0.0f;
    };

    void prepare(double newSampleRate) noexcept;
    void reset(float initialSignal = 0.0f) noexcept;
    void setParameters(Parameters value) noexcept;
    [[nodiscard]] Parameters parameters() const noexcept { return controls; }
    [[nodiscard]] Events tick(float input) noexcept;

private:
    void updateCoefficients() noexcept;

    Parameters controls {};
    double sampleRate = 48000.0;
    float attackGain = 0.0f;
    float releaseGain = 0.0f;
    float envelope = 0.0f;
    int signState = 0;
    bool gate = false;
};
}
