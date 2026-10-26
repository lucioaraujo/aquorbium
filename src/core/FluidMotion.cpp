// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/FluidMotion.h"

#include "core/MotionWaveBank.h"
#include "core/Numeric.h"

#include <algorithm>
#include <cmath>

namespace aquorbium
{
void FluidMotion::prepare(double newSampleRate) noexcept
{
    phases.prepare(newSampleRate);
    phases.setMemberCount(outputCount);
    reset();
}

void FluidMotion::setRateHz(float value) noexcept
{
    phases.setRateHz(value);
}

void FluidMotion::setFeedback(float amount, std::size_t source) noexcept
{
    feedback = numeric::clampFinite(amount, -0.95f, 0.95f, feedback);
    sourceIndex = std::min(source, outputCount - 1u);
}

void FluidMotion::reset(float phase) noexcept
{
    phases.reset(phase);
    previous.fill(0.0f);
}

FluidMotion::Frame FluidMotion::tick() noexcept
{
    const auto multiplier = std::clamp(
        1.0f + feedback * previous[sourceIndex], 0.1f, 4.0f);
    const auto phaseFrame = phases.advance(multiplier);
    Frame frame;
    frame.masterPhase = phaseFrame.masterPhase;
    frame.rateMultiplier = multiplier;
    constexpr std::array<float, outputCount> gains { 1.0f, 0.75f, 0.5f, 0.25f };
    constexpr std::array<float, outputCount> offsets { 0.0f, 0.125f, 0.25f, 0.5f };
    for (std::size_t output = 0; output < outputCount; ++output)
    {
        switch (outputMode)
        {
            case OutputMode::shapes:
                frame.value[output] = waveform(output, phaseFrame.masterPhase);
                break;
            case OutputMode::amplitudes:
                frame.value[output] = waveform(0, phaseFrame.masterPhase)
                    * gains[output];
                break;
            case OutputMode::times:
            {
                const auto shifted = phaseFrame.masterPhase + offsets[output];
                frame.value[output] = waveform(
                    0, shifted - std::floor(shifted));
                break;
            }
            case OutputMode::frequencies:
                frame.value[output] = waveform(0, phaseFrame.phase[output]);
                break;
        }
    }
    previous = frame.value;
    return frame;
}

float FluidMotion::waveform(std::size_t shape, float phase) noexcept
{
    return MotionWaveBank::evaluate(
        static_cast<MotionWaveBank::Shape>(shape % outputCount), phase);
}
}
