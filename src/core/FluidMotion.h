// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include "core/HarmonicPhaseBank.h"

#include <array>
#include <cstddef>

namespace aquorbium
{
// One school-level motion core exposing four interpretations of the same
// master phase. Optional bounded autopatch feeds one output back to speed.
class FluidMotion final
{
public:
    static constexpr std::size_t outputCount = 4;

    enum class OutputMode
    {
        shapes,
        amplitudes,
        times,
        frequencies
    };

    struct Frame
    {
        std::array<float, outputCount> value {};
        float masterPhase = 0.0f;
        float rateMultiplier = 1.0f;
    };

    void prepare(double newSampleRate) noexcept;
    void setRateHz(float value) noexcept;
    void setMode(OutputMode value) noexcept { outputMode = value; }
    void setFeedback(float amount, std::size_t source) noexcept;
    void reset(float phase = 0.0f) noexcept;
    [[nodiscard]] Frame tick() noexcept;

    [[nodiscard]] OutputMode mode() const noexcept { return outputMode; }
    [[nodiscard]] float feedbackAmount() const noexcept { return feedback; }
    [[nodiscard]] std::size_t feedbackSource() const noexcept { return sourceIndex; }
    [[nodiscard]] const HarmonicPhaseBank& phaseBank() const noexcept
    {
        return phases;
    }

private:
    [[nodiscard]] static float waveform(std::size_t shape, float phase) noexcept;

    HarmonicPhaseBank phases {};
    OutputMode outputMode = OutputMode::shapes;
    std::array<float, outputCount> previous {};
    float feedback = 0.0f;
    std::size_t sourceIndex = 0;
};
}
