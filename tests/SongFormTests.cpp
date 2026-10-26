// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

// The song as a hierarchy (2026-10-20): themes made of repeated phrases, in
// a fixed order, the song beginning again; slow evolution changes exactly one
// unit; without evolution the song repeats with precision.
#include "core/SongForm.h"

#include <cstdlib>
#include <iostream>
#include <vector>

namespace
{
using aquorbium::SongForm;
int failures = 0;
void check(bool condition, const char* what)
{
    if (!condition)
    {
        std::cerr << "FAILED: " << what << '\n';
        ++failures;
    }
}

SongForm::Phrase phrase(std::int8_t base, std::uint8_t length)
{
    SongForm::Phrase p;
    p.length = length;
    for (std::uint8_t i = 0; i < length; ++i)
    {
        p.degrees[i] = static_cast<std::int8_t>(base + i);
        p.steps[i] = 2;
    }
    return p;
}

int differingUnits(const SongForm& a, const SongForm& b)
{
    int n = 0;
    for (std::size_t t = 0; t < SongForm::maxThemes; ++t)
        for (std::size_t u = 0; u < SongForm::capacity; ++u)
            n += (a.themes()[t].degrees[u] != b.themes()[t].degrees[u]
                  || a.themes()[t].steps[u] != b.themes()[t].steps[u]) ? 1 : 0;
    return n;
}
}

int main()
{
    SongForm song;
    song.set({ phrase(0, 4), phrase(3, 5), phrase(-2, 3) }, 3, { 2, 3, 1 });

    // the theme order across two cycles: A A B B B C | A A B B B C
    std::vector<int> order;
    std::vector<SongForm::Advance> advances;
    for (int i = 0; i < 12; ++i)
    {
        order.push_back(song.theme());
        advances.push_back(song.endPhrase());
    }
    const std::vector<int> expected { 0, 0, 1, 1, 1, 2, 0, 0, 1, 1, 1, 2 };
    check(order == expected, "themes follow the order, each repeated its number of times");
    check(advances[5] == SongForm::Advance::songAgain && advances[11] == SongForm::Advance::songAgain,
          "the song begins again after the last theme");
    check(advances[1] == SongForm::Advance::newTheme && advances[0] == SongForm::Advance::repeat,
          "a theme repeats, then the next begins");
    check(song.cycles() == 2, "two song cycles counted");

    // slow evolution: exactly one unit changes per evolve()
    const auto before = song;
    song.evolve(0.4f, 0.5f, 0.7f, 0.9f);
    check(differingUnits(before, song) == 1, "evolution changes one unit");
    song.evolve(0.9f, 0.1f, 0.2f, 0.1f);
    check(differingUnits(before, song) <= 2, "two evolutions change at most two units");

    // a single theme, a single repetition: every phrase end is a new song
    SongForm one;
    one.set({ phrase(0, 3), {}, {} }, 1, { 1, 1, 1 });
    check(one.endPhrase() == SongForm::Advance::songAgain, "a one-phrase song begins again at each end");

    if (failures == 0)
        std::cout << "song form: all checks passed\n";
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
