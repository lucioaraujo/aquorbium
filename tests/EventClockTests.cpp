// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/ControlEventExtractor.h"
#include "core/FractionalLogicClock.h"

#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <utility>

namespace
{
using Clock = aquorbium::FractionalLogicClock;
using Extractor = aquorbium::ControlEventExtractor;

bool zeroCrossingsAreDiscreteAndStable()
{
    static_assert(noexcept(std::declval<Extractor&>().tick(0.0f)));
    Extractor extractor;
    extractor.prepare(48000.0);
    extractor.setParameters({ 0.02f, 0.2f, 0.04f, 5.0f, 80.0f });
    extractor.reset(-0.5f);
    int rising = 0;
    int falling = 0;
    for (int sample = 0; sample < 48000; ++sample)
    {
        const auto signal = static_cast<float>(0.8 * std::sin(
            6.283185307179586 * 2.0 * sample / 48000.0));
        const auto events = extractor.tick(signal);
        rising += events.risingZero ? 1 : 0;
        falling += events.fallingZero ? 1 : 0;
        if (!std::isfinite(events.envelope) || events.envelope < 0.0f
            || events.envelope > 1.0f) return false;
    }
    return rising == 2 && falling == 2;
}

bool envelopeHasHysteresisAndRejectsInvalidInput()
{
    Extractor extractor;
    extractor.prepare(48000.0);
    extractor.setParameters({ 0.01f, 0.25f, 0.08f, 1.0f, 12.0f });
    int rises = 0;
    int falls = 0;
    for (int sample = 0; sample < 8000; ++sample)
    {
        auto input = sample < 1000 ? 0.0f : (sample < 2500 ? 0.8f : 0.0f);
        if (sample == 4000) input = std::numeric_limits<float>::quiet_NaN();
        const auto events = extractor.tick(input);
        rises += events.envelopeRise ? 1 : 0;
        falls += events.envelopeFall ? 1 : 0;
        if (!std::isfinite(events.envelope)) return false;
    }
    extractor.setParameters({ -1.0f, 2.0f,
        std::numeric_limits<float>::infinity(), 0.0f, 9000.0f });
    const auto parameters = extractor.parameters();
    return rises == 1 && falls == 1 && parameters.zeroHysteresis == 0.0f
        && parameters.envelopeThreshold == 1.0f
        && parameters.envelopeHysteresis == 0.08f
        && parameters.attackMs == 0.1f && parameters.releaseMs == 5000.0f;
}

bool fractionalRatiosAndPartitionAreExact()
{
    static_assert(noexcept(std::declval<Clock&>().tick()));
    Clock direct, partitioned;
    const Clock::Parameters parameters {
        60.0f, { 7, 4 }, { 5, 4 }, 0.2f, Clock::Logic::Or
    };
    direct.prepare(48000.0);
    partitioned.prepare(48000.0);
    direct.setParameters(parameters);
    partitioned.setParameters(parameters);
    direct.reset();
    partitioned.reset();
    Clock::Output final;
    for (int sample = 0; sample < 48000 * 8; ++sample)
    {
        final = direct.tick();
        const auto repeated = partitioned.tick();
        if (final.branchA != repeated.branchA || final.branchB != repeated.branchB
            || final.gate != repeated.gate || final.trigger != repeated.trigger
            || final.cyclesA != repeated.cyclesA || final.cyclesB != repeated.cyclesB)
            return false;
    }
    return final.cyclesA == 14 && final.cyclesB == 10;
}

bool logicModesAndInvalidParametersAreDistinct()
{
    Clock andClock, orClock, xorClock;
    for (auto* clock : { &andClock, &orClock, &xorClock })
        clock->prepare(8000.0);
    const Clock::Parameters base {
        120.0f, { 1, 1 }, { 3, 2 }, 0.3f, Clock::Logic::And
    };
    andClock.setParameters(base);
    auto changed = base;
    changed.logic = Clock::Logic::Or;
    orClock.setParameters(changed);
    changed.logic = Clock::Logic::Xor;
    xorClock.setParameters(changed);
    andClock.reset();
    orClock.reset();
    xorClock.reset();
    int andGates = 0;
    int orGates = 0;
    int xorGates = 0;
    for (int sample = 0; sample < 8000 * 4; ++sample)
    {
        andGates += andClock.tick().gate ? 1 : 0;
        orGates += orClock.tick().gate ? 1 : 0;
        xorGates += xorClock.tick().gate ? 1 : 0;
    }
    Clock invalid;
    invalid.prepare(std::numeric_limits<double>::infinity());
    invalid.setParameters({ -1.0f, { 0, 0 }, { 255, 255 }, 1.0f,
                            static_cast<Clock::Logic>(99) });
    const auto sanitized = invalid.parameters();
    return andGates < orGates && xorGates > 0 && xorGates < orGates
        && sanitized.beatsPerMinute == 20.0f
        && sanitized.branchA.numerator == 1
        && sanitized.branchA.denominator == 1
        && sanitized.branchB.numerator == 32
        && sanitized.branchB.denominator == 32
        && sanitized.duty == 0.49f && sanitized.logic == Clock::Logic::Or;
}
}

int main()
{
    const std::pair<const char*, bool (*)()> cases[] {
        { "zero crossing", zeroCrossingsAreDiscreteAndStable },
        { "envelope hysteresis", envelopeHasHysteresisAndRejectsInvalidInput },
        { "fractional ratios", fractionalRatiosAndPartitionAreExact },
        { "clock logic", logicModesAndInvalidParametersAreDistinct },
    };
    for (const auto& [name, test] : cases)
    {
        if (!test())
        {
            std::cerr << name << " failed\n";
            return 1;
        }
    }
    std::cout << "Control events and fractional logic clock passed\n";
}
