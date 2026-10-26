// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include "core/CoreRefinementModules.h"

#include <array>

namespace aquorbium
{
// Isolated sine-wavetable FM body. No routing into the live app yet.
class SynthFMVoice final
{
public:
    struct Parameters
    {
        float carrierHz = 110.0f;
        float modulatorHz = 110.0f;
        float depthHz = 0.0f;
        float fold = 0.0f;
        float level = 0.5f;
    };

    void prepare(double newSampleRate) noexcept;
    void reset() noexcept;
    void setParameters(Parameters value) noexcept;
    [[nodiscard]] Parameters parameters() const noexcept { return controls; }
    [[nodiscard]] float tick() noexcept;
    [[nodiscard]] float lastCarrierFrequencyHz() const noexcept { return lastFrequencyHz; }

private:
    static constexpr int tableSize = 2048;
    [[nodiscard]] float readSine(double phase) const noexcept;
    static void advance(double& phase, double frequency, double sampleRate) noexcept;

    std::array<float, tableSize + 1> sineTable {};
    Parameters controls {};
    double sampleRate = 48000.0;
    double carrierPhase = 0.0;
    double modulatorPhase = 0.0;
    float lastFrequencyHz = 110.0f;
    OversampledSineFolder folder;
};
}
