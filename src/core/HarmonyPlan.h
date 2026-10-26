// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>

namespace aquorbium
{
// A session's chord progression (2026-10-24). The author: "pode haver
// encadeamentos harmônicos também, multitonalismos, e muitos outros recursos
// composicionais e que geram emoção a quem escuta".
//
// The languages come from the Trioio's harmony machine
// (TRIOIO/docs/MAQUINA_DE_HARMONIA.md, §1 and §5), rewritten here for an
// instrument that plays live: a cycle of chords drawn once per session, in
// one language, which the organisms then voice in real time. Sources, as
// there: Riemann (functions, 1893), Piston (cadences, *Harmony*, 1941),
// the Brazilian 6/9 tonic, II-V and passing diminished (Trioio §1.3),
// Debussy's planing and whole tones (§1.6), Cohn's neo-Riemannian P/L/R
// (*Audacious Euphony*, 2012) and Milhaud's polytonality (§1.7).
//
// Pitch classes are semitones above the session's tonic, 0..11. A chord may
// leave the mode (a secondary dominant's leading tone, a chromatic mediant):
// that is the colour the progression brings.
struct PlannedChord
{
    std::array<int, 6> tones {};  // pitch classes, the root first
    std::size_t size = 0;
    int bass = 0;                 // the bass's pitch class (a pedal, an inversion)
    float tension = 0.0f;         // 0 rest .. 1 most unstable
    int layerOffset = 0;          // polytonal: the upper layer's transposition (0 = none)
    bool cadence = false;         // the end of a phrase

    [[nodiscard]] bool contains(int pitchClass) const noexcept
    {
        const auto pc = ((pitchClass % 12) + 12) % 12;
        for (std::size_t i = 0; i < size; ++i)
            if (tones[i] == pc)
                return true;
        return false;
    }
};

enum class HarmonyLanguage : std::uint8_t { functional, modal, brazilian, impressionist, chromatic };
inline constexpr std::array<const char*, 5> harmonyLanguageNames { "functional", "modal", "brazilian",
                                                                   "impressionist", "chromatic" };

struct HarmonyPlan
{
    static constexpr std::size_t capacity = 16;
    HarmonyLanguage language = HarmonyLanguage::functional;
    std::array<PlannedChord, capacity> chords {};
    std::size_t length = 0;
    float chordSeconds = 4.0f;   // the harmonic rhythm
    bool polytonal = false;      // the session has polytonal episodes

    [[nodiscard]] const PlannedChord& at(std::size_t index) const noexcept
    {
        return chords[length == 0 ? 0 : index % length];
    }

    // scale: semitones of the mode (0 first), as SessionWorld holds it
    template <typename Random>
    [[nodiscard]] static HarmonyPlan generate(const std::array<float, 16>& scale, std::size_t scaleSize,
                                              float tempo, Random& random, int forcedLanguage = -1) noexcept
    {
        HarmonyPlan plan;
        const auto unit = [&random]() { return random(); };
        const auto pick = [&random](std::size_t count) {
            return std::min(count - 1, static_cast<std::size_t>(random() * static_cast<float>(count)));
        };
        const auto pc = [](int v) { return ((v % 12) + 12) % 12; };
        const auto degreeSemitones = [&](int degree) {
            const auto n = static_cast<int>(scaleSize);
            auto octave = degree / n;
            auto local = degree % n;
            if (local < 0) { local += n; --octave; }
            return static_cast<int>(std::lround(scale[static_cast<std::size_t>(local)])) + 12 * octave;
        };
        // a chord of stacked thirds of the mode on a degree (seventh optional)
        const auto diatonic = [&](int degree, bool seventh, float tension) {
            PlannedChord c;
            for (int k = 0; k < (seventh ? 4 : 3); ++k)
                c.tones[c.size++] = pc(degreeSemitones(degree + 2 * k));
            c.bass = c.tones[0];
            c.tension = tension;
            return c;
        };
        // intervals above a root; a negative value ends the list (the
        // arrays below are values, never views of temporaries)
        using Shape = std::array<int, 6>;
        const auto chromaticChord = [&](int root, const Shape& intervals, float tension) {
            PlannedChord c;
            for (const auto i : intervals)
            {
                if (i < 0)
                    break;
                if (c.size < c.tones.size())
                    c.tones[c.size++] = pc(root + i);
            }
            c.bass = c.tones[0];
            c.tension = tension;
            return c;
        };
        const auto push = [&plan](PlannedChord c) {
            if (plan.length < capacity)
                plan.chords[plan.length++] = c;
        };
        // major when the mode holds a major third above the tonic
        auto major = false;
        for (std::size_t i = 0; i < scaleSize; ++i)
            major = major || std::lround(scale[i]) == 4;
        const auto sevenNotes = scaleSize == 7;

        // the language: functional and Brazilian want a seven-note mode
        static constexpr std::array<HarmonyLanguage, 5> all { HarmonyLanguage::functional, HarmonyLanguage::modal,
                                                              HarmonyLanguage::brazilian, HarmonyLanguage::impressionist,
                                                              HarmonyLanguage::chromatic };
        plan.language = all[pick(all.size())];
        if (forcedLanguage >= 0 && forcedLanguage < static_cast<int>(all.size()))
            plan.language = all[static_cast<std::size_t>(forcedLanguage)];
        if (!sevenNotes && (plan.language == HarmonyLanguage::functional || plan.language == HarmonyLanguage::brazilian))
            plan.language = unit() < 0.5f ? HarmonyLanguage::modal : HarmonyLanguage::impressionist;
        // the harmonic rhythm follows the session's pace: 2.5 .. 8 s a chord
        plan.chordSeconds = std::clamp(5.0f / std::max(0.3f, tempo) * (0.7f + 0.6f * unit()), 2.5f, 8.0f);

        switch (plan.language)
        {
            case HarmonyLanguage::functional:
            {
                // two phrases of T .. S .. D, the first open (a half cadence),
                // the second closed (authentic, or deceptive, or plagal);
                // secondary dominants lead into ii, IV, V or vi
                static constexpr std::array<std::array<int, 3>, 5> openings {{
                    {{ 0, 5, 3 }}, {{ 0, 3, 1 }}, {{ 0, 2, 3 }}, {{ 0, 5, 1 }}, {{ 0, 3, 3 }} }};
                for (int phrase = 0; phrase < 2; ++phrase)
                {
                    const auto& open = openings[pick(openings.size())];
                    for (const auto degree : open)
                    {
                        if (degree != 0 && unit() < 0.35f)
                            // V7 of the next chord: a major triad and minor seventh a fifth above it
                            push(chromaticChord(degreeSemitones(degree) + 7, Shape { 0, 4, 7, 10, -1, -1 }, 0.7f));
                        push(diatonic(degree, unit() < 0.3f, degree == 0 ? 0.1f : degree == 5 ? 0.3f : 0.45f));
                    }
                    auto dominant = diatonic(4, true, 0.85f);
                    if (!major)  // the harmonic minor's dominant: a raised leading tone
                        dominant = chromaticChord(degreeSemitones(4), Shape { 0, 4, 7, 10, -1, -1 }, 0.85f);
                    push(dominant);
                    if (phrase == 0)
                        plan.chords[plan.length - 1].cadence = true;  // a half cadence
                    else
                    {
                        const auto ending = unit();
                        if (ending < 0.2f)
                        {
                            push(diatonic(5, false, 0.35f));  // deceptive: V - vi
                            push(diatonic(3, false, 0.4f));
                            push(dominant);
                        }
                        else if (ending < 0.35f)
                            push(diatonic(3, false, 0.35f));  // plagal colour before the close
                        auto home = diatonic(0, false, 0.0f);
                        if (!major && unit() < 0.3f)
                            home = chromaticChord(0, Shape { 0, 4, 7, -1, -1, -1 }, 0.0f);  // a Picardy third
                        home.cadence = true;
                        push(home);
                    }
                }
                break;
            }
            case HarmonyLanguage::modal:
            {
                // vamps over a tonic pedal: i - bVII, i - IV, i - bII, I - bVII - IV
                static constexpr std::array<std::array<int, 4>, 4> vamps {{
                    {{ 0, -1, 0, -1 }}, {{ 0, 3, 0, 3 }}, {{ 0, 1, 0, 1 }}, {{ 0, -1, 3, 0 }} }};
                const auto& vamp = vamps[pick(vamps.size())];
                const auto pedal = unit() < 0.6f;
                for (int repeat = 0; repeat < 2; ++repeat)
                    for (const auto degree : vamp)
                    {
                        auto c = diatonic(degree, unit() < 0.25f, degree == 0 ? 0.1f : 0.4f);
                        if (pedal)
                            c.bass = 0;
                        push(c);
                    }
                plan.chords[plan.length - 1].cadence = true;
                break;
            }
            case HarmonyLanguage::brazilian:
            {
                // I6/9 - #I dim - IIm7 - V7(9) - I6/9, with VI7(b9) leading to II
                const auto tonic = major ? Shape { 0, 4, 7, 9, 2, -1 } : Shape { 0, 3, 7, 9, 2, -1 };
                push(chromaticChord(0, tonic, 0.05f));
                if (unit() < 0.6f)
                    push(chromaticChord(1, Shape { 0, 3, 6, 9, -1, -1 }, 0.6f));  // the passing diminished
                else
                    push(chromaticChord(9, Shape { 0, 4, 7, 10, 1, -1 }, 0.7f));  // VI7(b9) -> II
                push(chromaticChord(2, Shape { 0, 3, 7, 10, -1, -1 }, 0.4f));    // IIm7
                push(chromaticChord(7, Shape { 0, 4, 7, 10, 2, -1 }, 0.8f)); // V7(9)
                push(chromaticChord(0, tonic, 0.05f));
                push(chromaticChord(5, Shape { 0, 4, 7, 11, -1, -1 }, 0.35f));    // IV7M
                push(chromaticChord(5, Shape { 0, 3, 7, 9, -1, -1 }, 0.5f));      // ivm6, the bossa's plagal
                auto home = chromaticChord(0, tonic, 0.0f);
                home.cadence = true;
                push(home);
                break;
            }
            case HarmonyLanguage::impressionist:
            {
                // planing: one colour (a major ninth) sliding by whole steps
                // and thirds; a whole-tone sonority; never V - I: the end is
                // bVII(add9) - I6/9
                static constexpr std::array<int, 6> slides { 2, -2, 4, -3, 2, 3 };
                auto root = 0;
                for (int k = 0; k < 6; ++k)
                {
                    if (k == 3 && unit() < 0.5f)
                        push(chromaticChord(root, Shape { 0, 4, 8, 2, 6, -1 }, 0.55f));  // whole tones
                    else
                        push(chromaticChord(root, Shape { 0, 4, 7, 11, 2, -1 }, 0.3f + 0.05f * static_cast<float>(k)));
                    root += slides[pick(slides.size())];
                }
                push(chromaticChord(10, Shape { 0, 4, 7, 2, -1, -1 }, 0.35f));  // bVII(add9)
                auto home = chromaticChord(0, Shape { 0, 4, 7, 9, 2, -1 }, 0.0f);
                home.cadence = true;
                push(home);
                break;
            }
            case HarmonyLanguage::chromatic:
            {
                // Cohn's P, L and R: each step keeps two tones of the triad
                // and moves one by a semitone or a tone - chromatic mediants,
                // far keys reached smoothly; home at the end
                auto root = 0;
                auto isMajor = major;
                for (int k = 0; k < 7; ++k)
                {
                    push(chromaticChord(root, isMajor ? Shape { 0, 4, 7, -1, -1, -1 } : Shape { 0, 3, 7, -1, -1, -1 },
                                        k == 0 ? 0.1f : 0.6f));
                    switch (pick(3))
                    {
                        case 0: isMajor = !isMajor; break;                                   // P: parallel
                        case 1: root = isMajor ? root + 4 : root - 4; isMajor = !isMajor; break;  // L: leading-tone exchange
                        default: root = isMajor ? root + 9 : root + 3; isMajor = !isMajor; break; // R: relative
                    }
                    root = pc(root);
                }
                auto home = chromaticChord(0, major ? Shape { 0, 4, 7, -1, -1, -1 } : Shape { 0, 3, 7, -1, -1, -1 }, 0.0f);
                home.cadence = true;
                push(home);
                break;
            }
        }
        // polytonal episodes (Milhaud): in about a third of the sessions, some
        // chords put the upper layer in another key - a tritone, a fifth, a
        // minor third or a major sixth away
        plan.polytonal = unit() < 0.33f;
        if (plan.polytonal)
        {
            static constexpr std::array<int, 4> offsets { 6, 7, 3, 9 };
            const auto offset = offsets[pick(offsets.size())];
            for (std::size_t i = 0; i < plan.length; ++i)
                if (!plan.chords[i].cadence && unit() < 0.4f)
                    plan.chords[i].layerOffset = offset;
        }
        return plan;
    }
};
}
