// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include "core/GeometricSequencer.h"

#include <array>
#include <cstddef>
#include <cstdint>

namespace aquorbium
{
// Fixed progression of weighted harmonic sections. The caller supplies one
// normalized ecological phase; mapping day/season into that phase remains an
// integration decision outside this event-rate core.
class HarmonicProgression final
{
public:
    static constexpr std::size_t maximumSections = 32;
    static constexpr std::size_t maximumVoices = 4;

    enum class Quality : std::uint8_t
    {
        major,
        minor,
        suspended2,
        suspended4,
        diminished,
        augmented,
        dominant7,
        major7,
        minor7
    };

    struct Section
    {
        int rootDegree = 0;
        Quality quality = Quality::minor;
        std::uint8_t inversion = 0;
        float weight = 1.0f;
    };

    struct Chord
    {
        std::array<float, maximumVoices> semitones {};
        std::size_t voiceCount = 3;
        std::size_t sectionIndex = 0;
        bool sectionChanged = false;
    };

    HarmonicProgression() noexcept;

    void setLength(std::size_t value) noexcept;
    [[nodiscard]] bool setSection(std::size_t index,
                                  const Section& value) noexcept;
    [[nodiscard]] bool setScale(
        const std::array<float, IntervalQuantizer::maximumIntervals>& values,
        std::size_t count, float periodSemitones) noexcept;
    void reset() noexcept;

    [[nodiscard]] Chord selectPhase(float normalizedPhase) noexcept;
    [[nodiscard]] const Section& section(std::size_t index) const noexcept;
    [[nodiscard]] std::size_t length() const noexcept { return activeLength; }

private:
    [[nodiscard]] std::size_t sectionForPhase(float normalizedPhase) const noexcept;
    [[nodiscard]] Chord chordFor(std::size_t index) const noexcept;
    [[nodiscard]] static Section sanitized(const Section& value) noexcept;

    std::array<Section, maximumSections> sections {};
    IntervalQuantizer quantizer;
    std::size_t activeLength = 4;
    std::size_t previousSection = maximumSections;
};
}
