// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <utility>

#include "core/HarmonyPlan.h"

namespace aquorbium
{
// The world a session starts in (2026-10-24). The author: "essa sequência de
// notas aleatórias é chata"; "sempre se repete, cada vez que eu abro". It
// did: a fresh seed only reordered the same melody - the root, the mode, the
// registers, the orbits, the contour and the organisms' pace were constants
// (measured: two seeds, the same pitch path per organism).
//
// The concept comes from the Rasgo Modular's seed as a HYPOTHESIS, not a
// randomize (dossies/ESTUDO_seed_composicao_generativa.md, §1.1): the seed
// chooses a coherent piece - a tonic and a path of tonics, a mode, who sings
// in which register, the length and rotation of the orbits, the shape of the
// melody, the pace of life, who opens, each body's colour, and the hour and
// season the session wakes in. Rewritten for the Aquorbium, no code shared.
//
// It never touches the musician's controls or saved patches. classic() is
// the instrument as it was (bit-identical), so tests and golden renders keep
// their meaning; fromSeed() is what a session opens with.
struct SessionWorld
{
    static constexpr std::size_t organisms = 5;
    static constexpr std::size_t sections = 8;
    static constexpr std::size_t maximumScale = 16;

    int rootOffset = 0;  // semitones above C2 (36)
    std::array<int, sections> rootPath { 0, 5, 3, 7, 2, 8, 5, 0 };
    std::array<float, maximumScale> scale { 0.0f, 2.0f, 3.0f, 5.0f, 7.0f, 9.0f, 10.0f };
    std::size_t scaleSize = 7;
    std::array<int, organisms> octaves { 0, 12, 12, 19, 7 };
    std::array<int, organisms> degrees { 0, 2, 4, 1, 6 };
    std::size_t orbitSteps = 8;
    std::array<int, organisms> rotations { 0, 2, 5, 7, 3 };
    std::array<int, 7> contour { 0, 1, -1, 2, -2, 3, -3 };
    std::array<float, organisms> lifeRates { 0.17f, 0.23f, 0.31f, 0.41f, 0.29f };
    std::size_t leader = 0;
    std::array<float, organisms> pulseWidths { 0.36f, 0.44f, 0.52f, 0.60f, 0.68f };
    std::array<float, organisms> shapeOffsets {};
    float dayPhase = 0.0f;
    float seasonPhase = 0.0f;
    // harmony inside one key (the generated worlds): the tonic stays, and the
    // day's sections move the chord through degrees of the mode, each chord
    // held for sectionsPerChord sections (~9 s each). The classic world
    // transposed the whole mode at every section instead (rootPath above),
    // eight keys a minute - heard as a chromatic soup (measured 2026-10-24:
    // all twelve pitch classes in 60 s)
    bool keyCentred = false;
    std::array<int, sections> chordPath {};
    int sectionsPerChord = 1;
    // the chord progression the organisms voice (HarmonyPlan.h); a generated
    // world has one, the classic world none
    bool harmonic = false;
    HarmonyPlan harmony {};
    // the session's arc of tension (2026-10-24): a shape over a period of
    // four to eight minutes, then again - rise to a climax and fall, waves,
    // a slow build, or a calm centre between two agitations
    enum class Arc : std::uint8_t { riseFall, waves, slowBuild, calmCentre };
    Arc arc = Arc::riseFall;
    float arcSeconds = 360.0f;
    [[nodiscard]] float arcAt(float seconds) const noexcept
    {
        constexpr float twoPi = 6.2831853f;
        const auto x = seconds / std::max(30.0f, arcSeconds);
        const auto f = x - std::floor(x);  // 0..1 within the period
        switch (arc)
        {
            case Arc::riseFall: return f < 0.7f ? f / 0.7f * (f / 0.7f) : (1.0f - f) / 0.3f;  // slow rise, quicker fall
            case Arc::waves: return 0.5f - 0.5f * std::cos(twoPi * 3.0f * f) * (0.6f + 0.4f * f);
            case Arc::slowBuild: return std::min(1.0f, f * 1.15f) * (f < 0.95f ? 1.0f : (1.0f - f) / 0.05f);
            case Arc::calmCentre: return 0.5f + 0.5f * std::cos(twoPi * f);
        }
        return 0.5f;
    }
    // names, for the status and the take's provenance
    const char* modeName = "dorian";
    bool classic = true;

    [[nodiscard]] static SessionWorld classicWorld() noexcept { return {}; }

    [[nodiscard]] static SessionWorld fromSeed(std::uint32_t seed) noexcept
    {
        SessionWorld w;
        w.classic = false;
        std::uint64_t state = 0x9e3779b97f4a7c15ull ^ (static_cast<std::uint64_t>(seed) * 0xbf58476d1ce4e5b9ull);
        const auto next = [&state]() {
            state += 0x9e3779b97f4a7c15ull;
            auto z = state;
            z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ull;
            z = (z ^ (z >> 27)) * 0x94d049bb133111ebull;
            return z ^ (z >> 31);
        };
        const auto unit = [&next]() { return static_cast<float>(next() >> 40) / 16777216.0f; };
        const auto pick = [&next](std::size_t count) { return static_cast<std::size_t>(next() % count); };

        // the mode: a field of intervals in the octave
        struct Mode { const char* name; std::array<float, 8> steps; std::size_t size; };
        static constexpr std::array<Mode, 10> modes {{
            { "dorian", { 0, 2, 3, 5, 7, 9, 10 }, 7 },
            { "aeolian", { 0, 2, 3, 5, 7, 8, 10 }, 7 },
            { "mixolydian", { 0, 2, 4, 5, 7, 9, 10 }, 7 },
            { "lydian", { 0, 2, 4, 6, 7, 9, 11 }, 7 },
            { "ionian", { 0, 2, 4, 5, 7, 9, 11 }, 7 },
            { "phrygian", { 0, 1, 3, 5, 7, 8, 10 }, 7 },
            { "minor pentatonic", { 0, 3, 5, 7, 10 }, 5 },
            { "major pentatonic", { 0, 2, 4, 7, 9 }, 5 },
            { "harmonic minor", { 0, 2, 3, 5, 7, 8, 11 }, 7 },
            { "hirajoshi", { 0, 2, 3, 7, 8 }, 5 },
        }};
        const auto& mode = modes[pick(modes.size())];
        w.modeName = mode.name;
        w.scale.fill(0.0f);
        for (std::size_t i = 0; i < mode.size; ++i)
            w.scale[i] = mode.steps[i];
        w.scaleSize = mode.size;

        // the tonic, and a path of tonics through the day's eight sections:
        // fourths and fifths most, thirds and steps sometimes, kept in range
        w.rootOffset = static_cast<int>(pick(12)) - 3;  // A1 .. G#2
        static constexpr std::array<int, 12> moves { 5, -5, 7, -7, 5, -5, 3, -3, 4, -4, 2, -2 };
        auto at = 0;
        w.rootPath[0] = 0;
        for (std::size_t k = 1; k < sections; ++k)
        {
            auto candidate = at + moves[pick(moves.size())];
            for (int tries = 0; tries < 6 && (candidate < -5 || candidate > 9); ++tries)
                candidate = at + moves[pick(moves.size())];
            at = std::clamp(candidate, -5, 9);
            w.rootPath[k] = at;
        }
        if (unit() < 0.6f)
            w.rootPath[sections - 1] = 0;  // the day often comes home
        // ...used only by the classic world: a generated world stays in its key
        w.rootPath.fill(0);
        w.keyCentred = true;
        // the progressions, in degrees of the mode (0 = I, 3 = IV, 4 = V,
        // 5 = vi, -1 = the step below, as i - bVII in a modal vamp)
        static constexpr std::array<std::array<int, sections>, 8> progressions {{
            {{ 0, 3, 4, 0, 5, 3, 4, 0 }},    // I IV V I vi IV V I
            {{ 0, 5, 3, 4, 0, 5, 3, 4 }},    // I vi IV V
            {{ 0, -1, 0, -1, 0, -1, -2, -1 }}, // modal vamp
            {{ 0, 2, 0, 2, 3, 2, 0, 0 }},    // I iii I iii IV iii I
            {{ 0, 3, 0, 3, 0, 4, 3, 0 }},    // plagal
            {{ 0, 0, 0, 0, 0, 0, 0, 0 }},    // a drone
            {{ 0, 5, 1, 4, 0, 5, 1, 4 }},    // I vi ii V
            {{ 0, -2, -1, 0, 0, -2, -1, 0 }}, // descending, returning
        }};
        w.chordPath = progressions[pick(progressions.size())];
        static constexpr std::array<int, 4> rates { 1, 2, 2, 4 };
        w.sectionsPerChord = rates[pick(rates.size())];

        // who sings where: a register template, shuffled, the bass anchored
        static constexpr std::array<std::array<int, organisms>, 6> registers {{
            {{ 0, 12, 12, 19, 7 }}, {{ 0, 7, 12, 19, 24 }}, {{ 0, 12, 19, 24, 12 }},
            {{ 0, 0, 12, 12, 24 }}, {{ 0, 7, 12, 12, 19 }}, {{ 0, 12, 7, 24, 19 }},
        }};
        w.octaves = registers[pick(registers.size())];
        for (std::size_t i = organisms - 1; i > 1; --i)
            std::swap(w.octaves[i], w.octaves[1 + pick(i)]);
        for (std::size_t i = 0; i < organisms; ++i)
            w.degrees[i] = static_cast<int>(pick(w.scaleSize));
        if (unit() < 0.7f)
            w.degrees[0] = 0;  // the bass usually holds the tonic
        // in one key a register is a whole number of octaves: a voice set a
        // fifth up (7 or 19 semitones) takes the mode's fifth as a degree
        // instead - 7 chromatic semitones over a mode leave the key (an A
        // major world played D#, measured 2026-10-24)
        std::size_t fifth = 0;
        for (std::size_t i = 0; i < w.scaleSize; ++i)
            if (std::abs(w.scale[i] - 7.0f) < 1.0e-4f)
                fifth = i;
        for (std::size_t i = 0; i < organisms; ++i)
            if (w.octaves[i] % 12 != 0)
            {
                w.octaves[i] -= 7;
                w.degrees[i] += static_cast<int>(fifth);
            }

        // the orbits: their length, each lane's rotation, the melody's shape
        static constexpr std::array<std::size_t, 10> orbits { 5, 6, 7, 8, 9, 10, 11, 12, 13, 16 };
        w.orbitSteps = orbits[pick(orbits.size())];
        for (auto& r : w.rotations)
            r = static_cast<int>(pick(w.orbitSteps));
        static constexpr std::array<std::array<int, 7>, 6> contours {{
            {{ 0, 1, -1, 2, -2, 3, -3 }},   // widening wave
            {{ 0, 1, 2, 1, 0, -1, -2 }},    // arch
            {{ 0, 2, -1, 1, -2, 3, 0 }},    // zigzag
            {{ 0, -1, -2, -1, 0, 1, 2 }},   // valley
            {{ 0, 3, -2, 4, -1, 2, -3 }},   // leaps
            {{ 0, 0, 1, 0, -1, 0, 2 }},     // near stillness
        }};
        w.contour = contours[pick(contours.size())];
        if (unit() < 0.5f)
            for (auto& c : w.contour)
                c = -c;

        // the pace of life: one tempo for the session, each organism around it
        const auto tempo = std::exp2(unit() * 1.4f - 0.7f);  // 0.62x .. 1.62x
        static constexpr std::array<float, organisms> base { 0.17f, 0.23f, 0.31f, 0.41f, 0.29f };
        for (std::size_t i = 0; i < organisms; ++i)
            w.lifeRates[i] = base[pick(organisms)] * tempo * (0.75f + 0.5f * unit());
        w.leader = pick(organisms);

        // each body's colour, and the hour and season the session wakes in
        for (std::size_t i = 0; i < organisms; ++i)
        {
            w.pulseWidths[i] = 0.14f + 0.72f * unit();
            w.shapeOffsets[i] = (unit() - 0.5f) * 0.5f;
        }
        w.dayPhase = unit();
        w.seasonPhase = unit();
        // the progression, in one of the languages, at the session's pace
        auto random = [&unit]() { return unit(); };
        w.harmony = HarmonyPlan::generate(w.scale, w.scaleSize, tempo, random);
        w.harmonic = true;
        static constexpr std::array<Arc, 4> arcs { Arc::riseFall, Arc::waves, Arc::slowBuild, Arc::calmCentre };
        w.arc = arcs[pick(arcs.size())];
        w.arcSeconds = 240.0f + 240.0f * unit();
        return w;
    }
};
}
