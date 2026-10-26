// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/HarmonicProgression.h"

#include <algorithm>
#include <array>
#include <cmath>

namespace aquorbium
{
namespace
{
struct ChordShape
{
    std::array<float, HarmonicProgression::maximumVoices> intervals {};
    std::size_t voices = 3;
};

ChordShape shapeFor(HarmonicProgression::Quality quality) noexcept
{
    using Quality = HarmonicProgression::Quality;
    switch (quality)
    {
        case Quality::major:      return { { 0.0f, 4.0f, 7.0f, 0.0f }, 3 };
        case Quality::minor:      return { { 0.0f, 3.0f, 7.0f, 0.0f }, 3 };
        case Quality::suspended2: return { { 0.0f, 2.0f, 7.0f, 0.0f }, 3 };
        case Quality::suspended4: return { { 0.0f, 5.0f, 7.0f, 0.0f }, 3 };
        case Quality::diminished: return { { 0.0f, 3.0f, 6.0f, 0.0f }, 3 };
        case Quality::augmented:  return { { 0.0f, 4.0f, 8.0f, 0.0f }, 3 };
        case Quality::dominant7:  return { { 0.0f, 4.0f, 7.0f, 10.0f }, 4 };
        case Quality::major7:     return { { 0.0f, 4.0f, 7.0f, 11.0f }, 4 };
        case Quality::minor7:     return { { 0.0f, 3.0f, 7.0f, 10.0f }, 4 };
    }
    return { { 0.0f, 3.0f, 7.0f, 0.0f }, 3 };
}

bool validQuality(HarmonicProgression::Quality quality) noexcept
{
    return static_cast<std::uint8_t>(quality)
        <= static_cast<std::uint8_t>(HarmonicProgression::Quality::minor7);
}
}

HarmonicProgression::HarmonicProgression() noexcept
{
    sections[0] = { 0, Quality::minor, 0, 1.0f };
    sections[1] = { 3, Quality::major, 0, 1.0f };
    sections[2] = { 4, Quality::minor7, 1, 1.0f };
    sections[3] = { -1, Quality::major, 0, 1.0f };
}

void HarmonicProgression::setLength(std::size_t value) noexcept
{
    activeLength = std::clamp<std::size_t>(value, 1, maximumSections);
    if (previousSection >= activeLength)
        previousSection = maximumSections;
}

bool HarmonicProgression::setSection(std::size_t index,
                                     const Section& value) noexcept
{
    if (index >= maximumSections)
        return false;
    sections[index] = sanitized(value);
    return true;
}

bool HarmonicProgression::setScale(
    const std::array<float, IntervalQuantizer::maximumIntervals>& values,
    std::size_t count, float periodSemitones) noexcept
{
    return quantizer.setField(values, count, periodSemitones);
}

void HarmonicProgression::reset() noexcept
{
    previousSection = maximumSections;
}

HarmonicProgression::Chord HarmonicProgression::selectPhase(
    float normalizedPhase) noexcept
{
    const auto index = sectionForPhase(normalizedPhase);
    auto result = chordFor(index);
    result.sectionChanged = index != previousSection;
    previousSection = index;
    return result;
}

const HarmonicProgression::Section& HarmonicProgression::section(
    std::size_t index) const noexcept
{
    return sections[index % activeLength];
}

std::size_t HarmonicProgression::sectionForPhase(
    float normalizedPhase) const noexcept
{
    auto phase = std::isfinite(normalizedPhase) ? normalizedPhase : 0.0f;
    phase -= std::floor(phase);

    float totalWeight = 0.0f;
    for (std::size_t index = 0; index < activeLength; ++index)
        totalWeight += sections[index].weight;

    const auto position = phase * totalWeight;
    float boundary = 0.0f;
    for (std::size_t index = 0; index < activeLength; ++index)
    {
        boundary += sections[index].weight;
        if (position < boundary)
            return index;
    }
    return activeLength - 1;
}

HarmonicProgression::Chord HarmonicProgression::chordFor(
    std::size_t index) const noexcept
{
    const auto& selected = sections[index];
    const auto shape = shapeFor(selected.quality);
    const auto root = quantizer.semitonesForDegree(selected.rootDegree);

    Chord result;
    result.voiceCount = shape.voices;
    result.sectionIndex = index;
    for (std::size_t voice = 0; voice < shape.voices; ++voice)
        result.semitones[voice] = root + shape.intervals[voice];

    const auto inversion = static_cast<std::size_t>(selected.inversion) % shape.voices;
    for (std::size_t turn = 0; turn < inversion; ++turn)
    {
        const auto lowest = result.semitones[0] + quantizer.period();
        for (std::size_t voice = 1; voice < shape.voices; ++voice)
            result.semitones[voice - 1] = result.semitones[voice];
        result.semitones[shape.voices - 1] = lowest;
    }
    return result;
}

HarmonicProgression::Section HarmonicProgression::sanitized(
    const Section& value) noexcept
{
    Section result;
    result.rootDegree = std::clamp(value.rootDegree, -64, 64);
    result.quality = validQuality(value.quality) ? value.quality : Quality::minor;
    result.inversion = std::min<std::uint8_t>(value.inversion, 3);
    result.weight = std::clamp(
        std::isfinite(value.weight) ? value.weight : 1.0f, 0.01f, 100.0f);
    return result;
}
}
