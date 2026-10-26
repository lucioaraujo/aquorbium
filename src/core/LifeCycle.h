// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include "core/SongForm.h"

#include <cstdint>

namespace aquorbium
{
// Life, food and descent (CONCEITOS_VERIFICACAO_E_APROFUNDAMENTO.md §2.4).
// In McCormack's Eden (2001-2005) creatures compete for limited resources,
// reproduce and evolve songs that others hear; here the idea is rewritten
// for the five voices of the reef:
//
//   - ENERGY is food: grazers eat in proportion to the prey's abundance (the
//     Lotka-Volterra cycle of PopulationCycles), a hunter eats only when it
//     captures; living costs a little, every note sung costs more (song is
//     costly). A hungry voice saves itself: it sings less.
//   - AGE darkens the body (its matter loses the highs) and ends the life;
//     hunger can end it sooner.
//   - DEATH sends the song to the reef (the sessile anchor's memory, which
//     already existed) and leaves a silence.
//   - BIRTH: the newborn sings a RECOMBINATION of its parent's song and its
//     mate's - the head of one phrase, the tail of the other, and one unit
//     mutated - so the reef's songs descend, mix and drift over generations.
//
// The bodies in the aquarium keep swimming: the life cycle is the voice's,
// a generation is a new song in the same body. Static functions over plain
// state: fixed size, no allocation, safe on the audio thread.
class LifeCycle final
{
public:
    using Phrase = SongForm::Phrase;

    struct Life
    {
        float energy = 0.7f;      // 0 starving .. 1 sated
        float age = 0.0f;         // 0 newborn .. 1 the end of its life
        float lifespan = 600.0f;  // seconds
        std::uint16_t generation = 0;
    };

    enum class Event : std::uint8_t { none, starved, oldAge };

    // the metabolism, per second of life
    static constexpr float basalCost = 0.0012f;
    static constexpr float grazingRate = 0.0045f;  // x prey abundance squared
    static constexpr float noteCost = 0.0012f;
    static constexpr float captureMeal = 0.5f;
    static constexpr float captureWound = 0.3f;

    // one stretch of life: seconds lived, food eaten (energy), notes sung
    static Event live(Life& life, float seconds, float food, int notes) noexcept;
    // what grazing yields in these seconds, given the prey's abundance; the
    // sated eat less
    [[nodiscard]] static float graze(const Life& life, float seconds, float preyAbundance) noexcept;
    // 0 fed .. 1 starving (below a third of the energy)
    [[nodiscard]] static float hunger(const Life& life) noexcept;
    // 0 young .. 1 old: the second half of life darkens the body
    [[nodiscard]] static float darkness(const Life& life) noexcept;
    static void beBorn(Life& life, float lifespan, std::uint16_t generation) noexcept;

    // the newborn's phrase: units [0, cut) from a, then b's tail, then one
    // unit mutated; all arguments in [0, 1)
    [[nodiscard]] static Phrase recombine(const Phrase& a, const Phrase& b, float cut, float mutateWhich,
                                          float mutateBy) noexcept;
};
}
