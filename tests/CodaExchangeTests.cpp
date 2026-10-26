// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

// The coda alphabet and the exchange (2026-10-21): the rhythm types have
// their shapes, the tempo types their order, rubato converges turn by turn,
// and ornaments mark changes of subject.
#include "core/CodaExchange.h"

#include <cmath>
#include <cstdlib>
#include <iostream>

namespace
{
using aquorbium::CodaExchange;
int failures = 0;
void check(bool condition, const char* what)
{
    if (!condition)
    {
        std::cerr << "FAILED: " << what << '\n';
        ++failures;
    }
}
}

int main()
{
    bool valid = true;
    for (int r = 0; r < CodaExchange::rhythmCount; ++r)
        for (std::size_t n = 1; n <= 8; ++n)
            for (std::size_t k = 0; k < n; ++k)
                valid = valid && CodaExchange::steps(r, k, n) >= 1 && CodaExchange::steps(r, k, n) <= 4;
    check(valid, "every rhythm gives 1 to 4 steps per unit");
    bool even = true;
    for (std::size_t k = 0; k < 6; ++k)
        even = even && CodaExchange::steps(CodaExchange::regular, k, 6) == 2;
    check(even, "the regular coda is even");
    check(CodaExchange::steps(CodaExchange::twoShortLong, 0, 6) == 1 && CodaExchange::steps(CodaExchange::twoShortLong, 1, 6) == 1
              && CodaExchange::steps(CodaExchange::twoShortLong, 2, 6) == 4,
          "1+1+3: two quick, one long");
    bool shrinking = true, growing = true;
    for (std::size_t k = 1; k < 6; ++k)
    {
        shrinking = shrinking && CodaExchange::steps(CodaExchange::accelerando, k, 6) <= CodaExchange::steps(CodaExchange::accelerando, k - 1, 6);
        growing = growing && CodaExchange::steps(CodaExchange::ritardando, k, 6) >= CodaExchange::steps(CodaExchange::ritardando, k - 1, 6);
    }
    check(shrinking && CodaExchange::steps(CodaExchange::accelerando, 0, 6) > CodaExchange::steps(CodaExchange::accelerando, 5, 6),
          "accelerando: the intervals shrink");
    check(growing && CodaExchange::steps(CodaExchange::ritardando, 5, 6) > CodaExchange::steps(CodaExchange::ritardando, 0, 6),
          "ritardando: the intervals grow");
    check(CodaExchange::tempo(0) < CodaExchange::tempo(1) && CodaExchange::tempo(1) < CodaExchange::tempo(2), "three tempo types");

    // rubato converges turn by turn, not at once
    float a = 0.8f, b = 1.3f;
    const auto first = std::abs(a - b);
    const auto a1 = CodaExchange::matchRubato(a, b), b1 = CodaExchange::matchRubato(b, a);
    check(std::abs(a1 - b1) < first && std::abs(a1 - b1) > 0.2f * first, "one turn closes part of the gap, not all");
    for (int turn = 0; turn < 8; ++turn)
    {
        const auto na = CodaExchange::matchRubato(a, b), nb = CodaExchange::matchRubato(b, a);
        a = na;
        b = nb;
    }
    check(std::abs(a - b) < 0.01f * first, "after eight turns the partners share their rubato");

    check(CodaExchange::ornamentChance(true) > 5.0f * CodaExchange::ornamentChance(false),
          "ornaments mark changes of subject");
    bool distinct = true;
    for (int s = 0; s < 5; ++s)
        for (int t = s + 1; t < 5; ++t)
            distinct = distinct && CodaExchange::identityRhythm(s) != CodaExchange::identityRhythm(t);
    check(distinct, "each species starts in its own clan");

    if (failures == 0)
        std::cout << "coda exchange: all checks passed\n";
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
