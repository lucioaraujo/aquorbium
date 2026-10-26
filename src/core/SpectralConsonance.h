// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace aquorbium
{
// Consonance that depends on the timbre and the register
// (CONCEITOS_VERIFICACAO_E_APROFUNDAMENTO.md §2.12). Plomp & Levelt (1965,
// JASA 38:548-560) showed that sensory dissonance comes from the beating of
// partials closer than a critical band, the roughest at about a quarter of
// it; Sethares (1993, JASA 94:1218-1228) parameterised their curve and summed
// it over the partials of two tones, so that the consonant intervals follow
// from the spectrum. Rewritten here from the published formula:
//
//   d(f1, f2, a1, a2) = min(a1, a2) * (exp(-b1 s df) - exp(-b2 s df)),
//   s = x* / (s1 * min(f1, f2) + s2),
//   b1 = 3.5, b2 = 5.75, x* = 0.24, s1 = 0.0207, s2 = 18.96.
//
// A harmonic spectrum gives the familiar ranking (octave, fifth, fourth,
// thirds...), worse in the low register; an inharmonic membrane or a glass
// favours other intervals. Fixed size, no allocation: safe on the audio thread.
class SpectralConsonance final
{
public:
    static constexpr std::size_t maxPartials = 8;

    struct Spectrum
    {
        std::array<float, maxPartials> ratio {};
        std::array<float, maxPartials> amplitude {};
        std::uint8_t count = 0;
    };

    // a harmonic series with amplitudes 1/n^tilt (n = 1..count)
    [[nodiscard]] static Spectrum harmonic(std::uint8_t count, float tilt = 1.0f) noexcept;
    // odd harmonics only (a folded or clarinet-like tone)
    [[nodiscard]] static Spectrum odd(std::uint8_t count) noexcept;
    // the circular membrane's first modes (zeros of the Bessel functions)
    [[nodiscard]] static Spectrum membrane() noexcept;
    // a rubbed glass's bending modes (one plausible glass)
    [[nodiscard]] static Spectrum glass() noexcept;
    // a pure tone
    [[nodiscard]] static Spectrum sine() noexcept;
    // a weighted union of two spectra (a blend of two bodies), the strongest kept
    [[nodiscard]] static Spectrum blend(const Spectrum& a, float weightA, const Spectrum& b, float weightB) noexcept;

    // the summed roughness of two tones (MIDI pitches) with these spectra
    [[nodiscard]] static float dissonance(float midiA, const Spectrum& a, float midiB, const Spectrum& b) noexcept;
    // 0 (rough) .. 1 (smooth): the roughness the interval adds to the two
    // tones' own, relative to the roughest small interval (1..5 semitones)
    // between the same spectra in the same register
    [[nodiscard]] static float consonance(float midiA, const Spectrum& a, float midiB, const Spectrum& b) noexcept;
};
}
