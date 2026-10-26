// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/FluidMotion.h"
#include "core/HarmonicPhaseBank.h"

#include <array>
#include <bit>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <utility>

namespace
{
bool phaseBankUsesExactRatios()
{
    using Bank = aquorbium::HarmonicPhaseBank;
    static_assert(noexcept(std::declval<Bank&>().advance()));
    Bank bank;
    bank.prepare(100.0);
    bank.setRateHz(1.0f);
    bank.setMemberCount(4);
    bank.reset(0.2f);
    const auto frame = bank.advance();
    if (std::abs(frame.phase[0] - 0.2f) > 1.0e-6f
        || std::abs(frame.phase[1] - 0.25f) > 1.0e-6f
        || std::abs(frame.phase[2] - 0.3f) > 1.0e-6f
        || std::abs(frame.phase[3] - 0.4f) > 1.0e-6f)
        return false;
    bank.setRatio(0, { 0, 0 });
    bank.setRatio(7, { 99, 99 });
    return bank.ratio(0).numerator == 1 && bank.ratio(0).denominator == 1
        && bank.ratio(7).numerator == 16 && bank.ratio(7).denominator == 16;
}

bool phaseBankIsDeterministicAndBounded()
{
    aquorbium::HarmonicPhaseBank first, repeated;
    for (auto* bank : { &first, &repeated })
    {
        bank->prepare(200.0);
        bank->setRateHz(7.0f);
        bank->setMemberCount(8);
    }
    for (int tick = 0; tick < 10000; ++tick)
    {
        const auto a = first.advance(1.7f);
        const auto b = repeated.advance(1.7f);
        for (std::size_t member = 0; member < a.count; ++member)
            if (std::bit_cast<std::uint32_t>(a.phase[member])
                    != std::bit_cast<std::uint32_t>(b.phase[member])
                || !std::isfinite(a.phase[member]) || a.phase[member] < 0.0f
                || a.phase[member] >= 1.0f)
                return false;
    }
    return true;
}

bool fluidModesAreDistinctAndBounded()
{
    using Motion = aquorbium::FluidMotion;
    static_assert(noexcept(std::declval<Motion&>().tick()));
    Motion motion;
    motion.prepare(200.0);
    motion.setRateHz(0.5f);
    std::array<Motion::Frame, 4> frames {};
    const std::array<Motion::OutputMode, 4> modes {
        Motion::OutputMode::shapes, Motion::OutputMode::amplitudes,
        Motion::OutputMode::times, Motion::OutputMode::frequencies
    };
    for (std::size_t index = 0; index < modes.size(); ++index)
    {
        motion.setMode(modes[index]);
        motion.reset(0.23f);
        frames[index] = motion.tick();
        for (const auto value : frames[index].value)
            if (!std::isfinite(value) || std::abs(value) > 1.0f)
                return false;
    }
    auto differingPairs = 0;
    for (std::size_t first = 0; first < frames.size(); ++first)
        for (std::size_t second = first + 1; second < frames.size(); ++second)
            differingPairs += frames[first].value != frames[second].value ? 1 : 0;
    return differingPairs == 6;
}

bool autopatchIsBoundedAndOptional()
{
    aquorbium::FluidMotion plain, zero, feedback;
    for (auto* motion : { &plain, &zero, &feedback })
    {
        motion->prepare(200.0);
        motion->setRateHz(3.0f);
        motion->setMode(aquorbium::FluidMotion::OutputMode::shapes);
    }
    zero.setFeedback(0.0f, 3);
    feedback.setFeedback(0.95f, 1);
    auto differs = false;
    for (int tick = 0; tick < 4000; ++tick)
    {
        const auto a = plain.tick();
        const auto b = zero.tick();
        const auto c = feedback.tick();
        if (std::bit_cast<std::uint32_t>(a.masterPhase)
                != std::bit_cast<std::uint32_t>(b.masterPhase)
            || c.rateMultiplier < 0.1f || c.rateMultiplier > 4.0f)
            return false;
        differs = differs || std::abs(a.masterPhase - c.masterPhase) > 0.01f;
    }
    feedback.setFeedback(std::numeric_limits<float>::infinity(), 99);
    return differs && feedback.feedbackAmount() == 0.95f
        && feedback.feedbackSource() == 3;
}
}

int main()
{
    const std::pair<const char*, bool (*)()> cases[] {
        { "phase ratios", phaseBankUsesExactRatios },
        { "phase determinism", phaseBankIsDeterministicAndBounded },
        { "fluid modes", fluidModesAreDistinctAndBounded },
        { "fluid autopatch", autopatchIsBoundedAndOptional },
    };
    for (const auto& [name, test] : cases)
    {
        if (!test())
        {
            std::cerr << name << " failed\n";
            return EXIT_FAILURE;
        }
    }
    std::cout << "School motion tests passed\n";
}
