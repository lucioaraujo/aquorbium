// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/SongForm.h"

#include <algorithm>

namespace aquorbium
{
void SongForm::set(const std::array<Phrase, maxThemes>& themes, std::uint8_t themeCount,
                   const std::array<std::uint8_t, maxThemes>& repetitions) noexcept
{
    phrases = themes;
    count = static_cast<std::uint8_t>(std::clamp<int>(themeCount, 1, static_cast<int>(maxThemes)));
    for (std::size_t t = 0; t < maxThemes; ++t)
    {
        repeats[t] = static_cast<std::uint8_t>(std::clamp<int>(repetitions[t], 1, 8));
        phrases[t].length = static_cast<std::uint8_t>(std::clamp<int>(phrases[t].length, 1, static_cast<int>(capacity)));
    }
    themeIndex = 0;
    repetitionIndex = 0;
    songCycles = 0;
}

SongForm::Advance SongForm::endPhrase() noexcept
{
    if (++repetitionIndex < repeats[themeIndex])
        return Advance::repeat;
    repetitionIndex = 0;
    if (++themeIndex < count)
        return Advance::newTheme;
    themeIndex = 0;
    ++songCycles;
    return Advance::songAgain;
}

void SongForm::evolve(float whichPhrase, float whichUnit, float degreeChange, float stepChange) noexcept
{
    const auto t = std::min<std::size_t>(static_cast<std::size_t>(std::clamp(whichPhrase, 0.0f, 0.999f) * count), count - 1u);
    auto& phrase = phrases[t];
    const auto u = std::min<std::size_t>(static_cast<std::size_t>(std::clamp(whichUnit, 0.0f, 0.999f) * phrase.length),
                                         phrase.length - 1u);
    // a step of one scale degree up or down, and sometimes a longer or shorter unit
    const auto delta = degreeChange < 0.5f ? -1 : 1;
    phrase.degrees[u] = static_cast<std::int8_t>(std::clamp(phrase.degrees[u] + delta, -5, 7));
    if (stepChange < 0.3f)
        phrase.steps[u] = static_cast<std::uint8_t>(std::clamp<int>(phrase.steps[u] + (stepChange < 0.15f ? -1 : 1), 1, 10));
}
}
