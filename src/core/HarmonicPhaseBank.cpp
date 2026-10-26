// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/HarmonicPhaseBank.h"

#include "core/Numeric.h"

#include <algorithm>
#include <cmath>

namespace aquorbium
{
HarmonicPhaseBank::HarmonicPhaseBank() noexcept
{
    ratios[0] = { 1, 1 };
    ratios[1] = { 5, 4 };
    ratios[2] = { 3, 2 };
    ratios[3] = { 2, 1 };
}

void HarmonicPhaseBank::prepare(double newSampleRate) noexcept
{
    sampleRate = std::isfinite(newSampleRate)
        ? std::clamp(newSampleRate, 1.0, numeric::maximumSampleRate)
        : 200.0;
    setRateHz(rateHz);
    reset();
}

void HarmonicPhaseBank::setRateHz(float value) noexcept
{
    rateHz = numeric::clampFinite(
        value, 0.0001f, static_cast<float>(sampleRate * 0.45), rateHz);
}

void HarmonicPhaseBank::setMemberCount(std::size_t value) noexcept
{
    activeMembers = std::clamp(value, std::size_t { 1 }, maximumMembers);
}

void HarmonicPhaseBank::setRatio(std::size_t member, Ratio value) noexcept
{
    if (member >= maximumMembers)
        return;
    value.numerator = std::clamp<std::uint8_t>(value.numerator, 1, 16);
    value.denominator = std::clamp<std::uint8_t>(value.denominator, 1, 16);
    ratios[member] = value;
}

void HarmonicPhaseBank::reset(float phase) noexcept
{
    masterCycles = static_cast<double>(numeric::wrapUnit(phase));
}

HarmonicPhaseBank::Frame HarmonicPhaseBank::advance(float rateMultiplier) noexcept
{
    Frame frame;
    frame.count = activeMembers;
    frame.masterPhase = wrapped(masterCycles);
    for (std::size_t member = 0; member < activeMembers; ++member)
    {
        frame.phase[member] = wrapped(masterCycles
            * static_cast<double>(ratios[member].numerator)
            / static_cast<double>(ratios[member].denominator));
    }
    const auto multiplier = numeric::clampFinite(
        rateMultiplier, 0.1f, 4.0f, 1.0f);
    masterCycles += static_cast<double>(rateHz * multiplier) / sampleRate;
    // 720720 is divisible by every denominator from 1 to 16, so reducing the
    // long-running clock here preserves every configured rational phase.
    constexpr auto commonPeriod = 720720.0;
    if (masterCycles >= commonPeriod)
        masterCycles -= std::floor(masterCycles / commonPeriod) * commonPeriod;
    return frame;
}

HarmonicPhaseBank::Ratio HarmonicPhaseBank::ratio(std::size_t member) const noexcept
{
    return member < maximumMembers ? ratios[member] : Ratio {};
}

float HarmonicPhaseBank::wrapped(double value) noexcept
{
    return static_cast<float>(value - std::floor(value));
}
}
