// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/AddressableStepMemory.h"
#include "core/HarmonicProgression.h"

#include <array>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <utility>

namespace
{
using Memory = aquorbium::AddressableStepMemory;
using Progression = aquorbium::HarmonicProgression;

bool sequentialAndDirectAddressingAreDefined()
{
    static_assert(noexcept(std::declval<Memory&>().advance()));
    static_assert(noexcept(std::declval<Memory&>().address(0.0f)));
    Memory memory;
    memory.setLength(4);
    for (std::size_t index = 0; index < 4; ++index)
    {
        if (!memory.setStep(index, {
                static_cast<float>(index * 3),
                0.2f + static_cast<float>(index) * 0.1f,
                static_cast<float>(index) / 3.0f,
                1.0f - static_cast<float>(index) / 3.0f }))
            return false;
    }

    memory.reset(2);
    const auto two = memory.advance();
    const auto three = memory.advance();
    const auto zero = memory.advance();
    if (two.index != 2 || two.wrapped || two.event != 0
        || three.index != 3 || !three.wrapped || three.event != 1
        || zero.index != 0 || zero.event != 2)
        return false;

    const auto low = memory.address(-2.0f);
    const auto middle = memory.address(0.50f);
    const auto high = memory.address(1.0f);
    const auto invalid = memory.address(std::numeric_limits<float>::quiet_NaN());
    return low.index == 0 && middle.index == 2 && high.index == 3
        && invalid.index == 0 && middle.step.pitchSemitones == 6.0f
        && memory.cursor() == 1;
}

bool memorySanitizesAndKeepsFourControls()
{
    Memory memory;
    memory.setLength(0);
    if (memory.length() != 1 || memory.setStep(Memory::maximumSteps, {}))
        return false;
    if (!memory.setStep(0, {
            900.0f,
            -2.0f,
            std::numeric_limits<float>::infinity(),
            std::numeric_limits<float>::quiet_NaN() }))
        return false;
    const auto value = memory.advance().step;
    return value.pitchSemitones == 48.0f && value.gateDuration == 0.0f
        && value.strategyBias == 0.0f && value.desireAmount == 0.5f;
}

bool weightedSectionsAndBoundariesAreStable()
{
    static_assert(noexcept(std::declval<Progression&>().selectPhase(0.0f)));
    Progression progression;
    progression.setLength(3);
    if (!progression.setSection(0, { 0, Progression::Quality::minor, 0, 1.0f })
        || !progression.setSection(1, { 3, Progression::Quality::major7, 0, 2.0f })
        || !progression.setSection(2, { -1, Progression::Quality::dominant7, 0, 1.0f }))
        return false;

    progression.reset();
    const auto first = progression.selectPhase(0.0f);
    const auto held = progression.selectPhase(0.24f);
    const auto second = progression.selectPhase(0.25f);
    const auto stillSecond = progression.selectPhase(0.74f);
    const auto third = progression.selectPhase(0.75f);
    const auto wrapped = progression.selectPhase(1.0f);
    return first.sectionIndex == 0 && first.sectionChanged
        && held.sectionIndex == 0 && !held.sectionChanged
        && second.sectionIndex == 1 && second.sectionChanged
        && stillSecond.sectionIndex == 1 && !stillSecond.sectionChanged
        && third.sectionIndex == 2 && third.sectionChanged
        && wrapped.sectionIndex == 0 && wrapped.sectionChanged;
}

bool chordsSupportScaleDegreesQualitiesAndInversions()
{
    Progression progression;
    progression.setLength(2);
    if (!progression.setSection(0, { -1, Progression::Quality::major, 1, 1.0f })
        || !progression.setSection(1, { 2, Progression::Quality::minor7, 2, 1.0f }))
        return false;

    const auto invertedMajor = progression.selectPhase(0.0f);
    const auto invertedMinor7 = progression.selectPhase(0.5f);
    if (invertedMajor.voiceCount != 3 || invertedMajor.semitones[0] != 2.0f
        || invertedMajor.semitones[1] != 5.0f
        || invertedMajor.semitones[2] != 10.0f)
        return false;
    if (invertedMinor7.voiceCount != 4 || invertedMinor7.semitones[0] != 10.0f
        || invertedMinor7.semitones[1] != 13.0f
        || invertedMinor7.semitones[2] != 15.0f
        || invertedMinor7.semitones[3] != 18.0f)
        return false;

    std::array<float, aquorbium::IntervalQuantizer::maximumIntervals> scale {};
    scale[0] = 0.0f;
    scale[1] = 1.5f;
    scale[2] = 4.0f;
    if (!progression.setScale(scale, 3, 7.0f))
        return false;
    if (!progression.setSection(1, {
            2, Progression::Quality::minor7, 0, 1.0f }))
        return false;
    const auto microtonal = progression.selectPhase(0.5f);
    return microtonal.semitones[0] == 4.0f
        && microtonal.semitones[1] == 7.0f
        && microtonal.semitones[2] == 11.0f
        && microtonal.semitones[3] == 14.0f;
}

bool invalidProgressionInputIsBounded()
{
    Progression progression;
    progression.setLength(1000);
    if (progression.length() != Progression::maximumSections
        || progression.setSection(Progression::maximumSections, {}))
        return false;
    if (!progression.setSection(0, {
            1000,
            static_cast<Progression::Quality>(255),
            99,
            std::numeric_limits<float>::quiet_NaN() }))
        return false;
    const auto& section = progression.section(0);
    const auto chord = progression.selectPhase(
        std::numeric_limits<float>::infinity());
    return section.rootDegree == 64
        && section.quality == Progression::Quality::minor
        && section.inversion == 3 && section.weight == 1.0f
        && chord.sectionIndex == 0
        && chord.voiceCount >= 3 && chord.voiceCount <= 4;
}
}

int main()
{
    const std::pair<const char*, bool (*)()> cases[] {
        { "addressing", sequentialAndDirectAddressingAreDefined },
        { "four controls", memorySanitizesAndKeepsFourControls },
        { "weighted sections", weightedSectionsAndBoundariesAreStable },
        { "chords", chordsSupportScaleDegreesQualitiesAndInversions },
        { "invalid input", invalidProgressionInputIsBounded },
    };
    for (const auto& [name, test] : cases)
    {
        if (!test())
        {
            std::cerr << name << " failed\n";
            return EXIT_FAILURE;
        }
    }
    std::cout << "Addressable memory and harmonic progression passed\n";
}
