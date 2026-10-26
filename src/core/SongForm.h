// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace aquorbium
{
// The song as a hierarchy (CONCEITOS_VERIFICACAO_E_APROFUNDAMENTO.md §2.5).
// Humpback songs are units grouped into phrases; a phrase repeated forms a
// theme; themes in a fixed order form the song, which is repeated with
// considerable precision (Payne & McVay 1971, Science 173:585-597). The song
// evolves slowly from cycle to cycle and, rarely, is replaced whole by a new
// one (a revolution: Noad et al. 2000, Nature 408:537) - the ideas, rewritten.
//
// SongForm keeps the themes of one organism and walks through them: each
// phrase end advances the repetition, then the theme, then the song. evolve()
// changes a single unit (the slow drift). Fixed size, no allocation.
class SongForm final
{
public:
    static constexpr std::size_t maxThemes = 3;
    static constexpr std::size_t capacity = 8;

    struct Phrase
    {
        std::array<std::int8_t, capacity> degrees {};
        std::array<std::uint8_t, capacity> steps {};  // 16th steps until the next unit
        std::uint8_t length = 0;
    };

    enum class Advance : std::uint8_t
    {
        repeat,    // the same phrase again (the theme continues)
        newTheme,  // the next theme begins
        songAgain  // the last theme ended: the song begins again
    };

    void set(const std::array<Phrase, maxThemes>& themes, std::uint8_t themeCount,
             const std::array<std::uint8_t, maxThemes>& repetitions) noexcept;
    [[nodiscard]] const Phrase& current() const noexcept { return phrases[themeIndex]; }
    [[nodiscard]] Phrase& current() noexcept { return phrases[themeIndex]; }
    Advance endPhrase() noexcept;
    // the slow drift: one unit of one phrase changes (all arguments in [0, 1))
    void evolve(float whichPhrase, float whichUnit, float degreeChange, float stepChange) noexcept;

    [[nodiscard]] std::uint8_t theme() const noexcept { return themeIndex; }
    [[nodiscard]] std::uint8_t themeCount() const noexcept { return count; }
    [[nodiscard]] std::uint8_t repetition() const noexcept { return repetitionIndex; }
    [[nodiscard]] std::uint32_t cycles() const noexcept { return songCycles; }
    [[nodiscard]] const std::array<Phrase, maxThemes>& themes() const noexcept { return phrases; }

private:
    std::array<Phrase, maxThemes> phrases {};
    std::array<std::uint8_t, maxThemes> repeats { 2, 2, 2 };
    std::uint8_t count = 1;
    std::uint8_t themeIndex = 0;
    std::uint8_t repetitionIndex = 0;
    std::uint32_t songCycles = 0;
};
}
