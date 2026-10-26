// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace aquorbium
{
// The water itself, boiling (CONCEITOS_VERIFICACAO_E_APROFUNDAMENTO.md §3;
// 2026-10-21). Until now the boiling only made the organisms sing Minnaert
// pitches; the water did not bubble. This is the bubbling, rewritten from
// the acoustics of bubbles (no code copied):
//
//   - each bubble rings at its Minnaert frequency, f = 3.26 / R (Hz, R in
//     metres, near the surface: Minnaert 1933), and dies with the damping
//     of a millimetre bubble, a dimensionless delta of ~0.1 (radiation,
//     thermal and viscous losses: Leighton, The Acoustic Bubble, 1994);
//   - its pitch rises a little as it lifts towards the surface (van den
//     Doel 2005, "Physically based models for liquid sounds");
//   - the sizes follow the distribution measured under breaking waves,
//     R^(-3/2) below ~1 mm and R^(-10/3) above (Deane & Stokes 2002,
//     Nature 418:839);
//   - bubbles born together COUPLE: two equal bubbles at a distance d split
//     their resonance into an in-phase mode, lower, and an antiphase mode,
//     higher, f = f0 / sqrt(1 +- R/d) (Feuillade 1995, JASA 98:1178) -
//     the collective modes that make real water richer than independent
//     bubbles (Xue et al. 2023, "Improved water sound synthesis using
//     coupled bubbles", ACM TOG 42:4).
//
// The rate follows the water (turbulence a little, boiling a lot); each
// bubble has its place in the stereo field. Fixed capacity, no allocation:
// safe on the audio thread. At rate zero, with no bubble alive, it returns
// exact zeros.
class BubbleField final
{
public:
    static constexpr std::size_t capacity = 32;

    struct Sample
    {
        float left = 0.0f;
        float right = 0.0f;
    };

    void prepare(double sampleRate, std::uint32_t seed) noexcept;
    void reset() noexcept;
    // bubbles per second
    void setRate(float bubblesPerSecond) noexcept;
    [[nodiscard]] Sample render() noexcept;

    // the physics, exposed for the tests
    [[nodiscard]] static float minnaertHz(float radiusMetres) noexcept;
    // the in-phase (lower) and antiphase (higher) modes of two equal bubbles
    // of resonance f0 and radius R at a distance d (d > R)
    [[nodiscard]] static float coupledLowHz(float f0, float radius, float distance) noexcept;
    [[nodiscard]] static float coupledHighHz(float f0, float radius, float distance) noexcept;
    // a radius drawn from the Deane & Stokes distribution, for u in (0, 1)
    [[nodiscard]] static float drawRadius(float u) noexcept;

    // counters, for the tests
    [[nodiscard]] std::uint32_t born() const noexcept { return bornCount; }
    [[nodiscard]] std::uint32_t pairs() const noexcept { return pairCount; }

private:
    struct Bubble
    {
        float phase = 0.0f;
        float increment = 0.0f;  // cycles per sample
        float rise = 1.0f;       // per-sample multiplier of the increment
        float amplitude = 0.0f;
        float decay = 0.0f;
        float left = 0.0f, right = 0.0f;
    };

    [[nodiscard]] float nextUnit() noexcept;
    void spawn(float radius, float frequency, float amplitude, float pan) noexcept;

    double rate = 48000.0;
    float sampleRateF = 48000.0f;
    std::uint32_t seed = 0xb0bb1e5u;
    std::uint32_t state = 0xb0bb1e5u;
    float perSample = 0.0f;   // birth probability per sample
    std::array<Bubble, capacity> bubbles {};
    std::uint32_t sinceBirth = 0xffffffu;
    float lastRadius = 0.0f;
    float lastFrequency = 0.0f;
    float lastPan = 0.5f;
    std::uint32_t bornCount = 0;
    std::uint32_t pairCount = 0;
};
}
