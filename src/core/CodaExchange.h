// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include <cstddef>
#include <cstdint>

namespace aquorbium
{
// The coda alphabet and the exchange of turns
// (CONCEITOS_VERIFICACAO_E_APROFUNDAMENTO.md §2.6; 2026-10-21). Sharma et al.
// (2024, Nature Communications 15:3617) found that sperm whale codas combine
// independent features - RHYTHM (the pattern of the intervals), TEMPO (the
// overall duration), RUBATO (a slow, continuous stretching from coda to
// coda) and ORNAMENT (an extra click) - into a large combinatorial inventory;
// in exchanges, whales match each other's rubato gradually, and ornaments
// cluster where the context changes. Vocal clans are told apart by their
// identity codas (Hersh et al. 2022, PNAS 119:e2201692119). The ideas,
// rewritten:
//
//   - six rhythm types and three tempo types make the alphabet;
//   - each voice has an identity rhythm; in an exchange (courtship,
//     alliance) the pair speaks in the caller's rhythm and tempo, and an
//     ally sometimes adopts the caller's identity rhythm (a clan forming);
//   - rubato converges turn by turn towards the partner's, not at once;
//   - the ornament marks a change of subject (a new relation or partner).
//
// Static and stateless: safe on the audio thread.
class CodaExchange final
{
public:
    enum Rhythm : std::uint8_t
    {
        regular,     // even intervals
        twoShortLong,// "1+1+3": two quick, one long, repeated
        accelerando, // the intervals shrink
        ritardando,  // the intervals grow
        longShort,   // swung pairs
        shortLong    // reversed pairs
    };
    static constexpr int rhythmCount = 6;
    static constexpr int tempoCount = 3;
    static constexpr float matchRate = 0.3f;  // of the rubato gap closed per turn

    // the 16th steps of a unit in a phrase of this length, in this rhythm (>= 1)
    [[nodiscard]] static std::uint8_t steps(int rhythm, std::size_t unit, std::size_t length) noexcept;
    // the tempo type's factor on the durations
    [[nodiscard]] static float tempo(int type) noexcept;
    // one turn of rubato matching
    [[nodiscard]] static float matchRubato(float own, float partner) noexcept;
    // the chance of an ornament at the end of a coda
    [[nodiscard]] static float ornamentChance(bool contextChanged) noexcept;
    // a species' identity rhythm (its clan, before any exchange)
    [[nodiscard]] static int identityRhythm(int species) noexcept;
};
}
