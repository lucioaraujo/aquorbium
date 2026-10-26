// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/SpectralConsonance.h"

#include <algorithm>
#include <cmath>

namespace aquorbium
{
namespace
{
constexpr float b1 = 3.5f;
constexpr float b2 = 5.75f;
constexpr float xStar = 0.24f;
constexpr float s1 = 0.0207f;
constexpr float s2 = 18.96f;
// the roughest value of exp(-b1 x) - exp(-b2 x), at x = ln(b2/b1)/(b2-b1)
const float peakRoughness = [] {
    const auto x = std::log(b2 / b1) / (b2 - b1);
    return std::exp(-b1 * x) - std::exp(-b2 * x);
}();

float midiToHz(float midi) noexcept
{
    return 440.0f * std::exp2((midi - 69.0f) / 12.0f);
}

float pairRoughness(float f1, float a1, float f2, float a2) noexcept
{
    const auto fmin = std::min(f1, f2);
    const auto s = xStar / (s1 * fmin + s2);
    const auto x = s * std::abs(f2 - f1);
    return std::min(a1, a2) * (std::exp(-b1 * x) - std::exp(-b2 * x));
}
} // namespace

SpectralConsonance::Spectrum SpectralConsonance::harmonic(std::uint8_t count, float tilt) noexcept
{
    Spectrum s;
    s.count = static_cast<std::uint8_t>(std::clamp<int>(count, 1, static_cast<int>(maxPartials)));
    for (std::size_t n = 0; n < s.count; ++n)
    {
        s.ratio[n] = static_cast<float>(n + 1);
        s.amplitude[n] = 1.0f / std::pow(static_cast<float>(n + 1), tilt);
    }
    return s;
}

SpectralConsonance::Spectrum SpectralConsonance::odd(std::uint8_t count) noexcept
{
    Spectrum s;
    s.count = static_cast<std::uint8_t>(std::clamp<int>(count, 1, static_cast<int>(maxPartials)));
    for (std::size_t n = 0; n < s.count; ++n)
    {
        const auto k = static_cast<float>(2 * n + 1);
        s.ratio[n] = k;
        s.amplitude[n] = 1.0f / k;
    }
    return s;
}

SpectralConsonance::Spectrum SpectralConsonance::membrane() noexcept
{
    Spectrum s;
    constexpr std::array<float, 6> r { 1.0f, 1.593f, 2.135f, 2.295f, 2.653f, 2.917f };
    constexpr std::array<float, 6> a { 1.0f, 0.75f, 0.55f, 0.45f, 0.30f, 0.22f };
    s.count = 6;
    for (std::size_t n = 0; n < 6; ++n)
    {
        s.ratio[n] = r[n];
        s.amplitude[n] = a[n];
    }
    return s;
}

SpectralConsonance::Spectrum SpectralConsonance::glass() noexcept
{
    Spectrum s;
    constexpr std::array<float, 4> r { 1.0f, 2.32f, 4.25f, 6.63f };
    s.count = 4;
    for (std::size_t n = 0; n < 4; ++n)
    {
        s.ratio[n] = r[n];
        s.amplitude[n] = std::pow(0.5f, static_cast<float>(n));
    }
    return s;
}

SpectralConsonance::Spectrum SpectralConsonance::sine() noexcept
{
    Spectrum s;
    s.count = 1;
    s.ratio[0] = 1.0f;
    s.amplitude[0] = 1.0f;
    return s;
}

SpectralConsonance::Spectrum SpectralConsonance::blend(const Spectrum& a, float weightA, const Spectrum& b,
                                                       float weightB) noexcept
{
    // gather both, merge nearly coincident ratios, keep the strongest
    std::array<float, maxPartials * 2> ratio {};
    std::array<float, maxPartials * 2> amplitude {};
    std::size_t n = 0;
    const auto add = [&](float r, float amp) {
        if (amp <= 1.0e-4f)
            return;
        for (std::size_t k = 0; k < n; ++k)
            if (std::abs(ratio[k] - r) < 0.01f * r)
            {
                amplitude[k] += amp;
                return;
            }
        ratio[n] = r;
        amplitude[n] = amp;
        ++n;
    };
    for (std::size_t k = 0; k < a.count; ++k)
        add(a.ratio[k], a.amplitude[k] * std::max(0.0f, weightA));
    for (std::size_t k = 0; k < b.count; ++k)
        add(b.ratio[k], b.amplitude[k] * std::max(0.0f, weightB));
    Spectrum out;
    for (std::size_t pick = 0; pick < maxPartials && pick < n; ++pick)
    {
        std::size_t best = pick;
        for (std::size_t k = pick + 1; k < n; ++k)
            if (amplitude[k] > amplitude[best])
                best = k;
        std::swap(ratio[pick], ratio[best]);
        std::swap(amplitude[pick], amplitude[best]);
        out.ratio[pick] = ratio[pick];
        out.amplitude[pick] = amplitude[pick];
        out.count = static_cast<std::uint8_t>(pick + 1);
    }
    if (out.count == 0)
        out = sine();
    return out;
}

float SpectralConsonance::dissonance(float midiA, const Spectrum& a, float midiB, const Spectrum& b) noexcept
{
    const auto fa = midiToHz(midiA);
    const auto fb = midiToHz(midiB);
    float total = 0.0f;
    for (std::size_t i = 0; i < a.count; ++i)
        for (std::size_t j = 0; j < b.count; ++j)
            total += pairRoughness(fa * a.ratio[i], a.amplitude[i], fb * b.ratio[j], b.amplitude[j]);
    return total;
}

float SpectralConsonance::consonance(float midiA, const Spectrum& a, float midiB, const Spectrum& b) noexcept
{
    // the roughness the interval ADDS to the two tones' own roughness (an
    // inharmonic tone is already rough by itself; its unison must stay
    // smooth), normalised by the roughest small interval (1..5 semitones)
    // between the same two spectra at the same register
    const auto low = std::min(midiA, midiB);
    const auto& lowSpectrum = midiA <= midiB ? a : b;
    const auto& highSpectrum = midiA <= midiB ? b : a;
    const auto own = [&](float lowMidi, float highMidi) {
        return 0.5f * (dissonance(lowMidi, lowSpectrum, lowMidi, lowSpectrum)
                       + dissonance(highMidi, highSpectrum, highMidi, highSpectrum));
    };
    const auto excess = [&](float highMidi) {
        return dissonance(low, lowSpectrum, highMidi, highSpectrum) - own(low, highMidi);
    };
    float roughest = 1.0e-6f;
    for (int k = 1; k <= 5; ++k)
        roughest = std::max(roughest, excess(low + static_cast<float>(k)));
    const auto high = std::max(midiA, midiB);
    return std::clamp(1.0f - std::max(0.0f, excess(high)) / roughest, 0.0f, 1.0f);
}
}
