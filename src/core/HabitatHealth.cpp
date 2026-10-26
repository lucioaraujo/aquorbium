// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/HabitatHealth.h"

#include <algorithm>
#include <cmath>

namespace aquorbium
{
namespace
{
constexpr double pi = 3.14159265358979323846;
}

void HabitatHealth::prepare(double sampleRate, double windowSeconds) noexcept
{
    const auto rate = std::isfinite(sampleRate) && sampleRate >= 8000.0 ? sampleRate : 48000.0;
    for (std::size_t n = 0; n < fftSize; ++n)
        window[n] = static_cast<float>(0.5 - 0.5 * std::cos(2.0 * pi * static_cast<double>(n) / fftSize));
    for (std::size_t k = 0; k < fftSize / 2; ++k)
        twiddle[k] = std::polar(1.0f, static_cast<float>(-2.0 * pi * static_cast<double>(k) / fftSize));
    unsigned bits = 0;
    while ((1u << bits) < fftSize)
        ++bits;
    for (std::size_t n = 0; n < fftSize; ++n)
    {
        unsigned r = 0;
        for (unsigned b = 0; b < bits; ++b)
            if ((n >> b) & 1u)
                r |= 1u << (bits - 1u - b);
        bitReverse[n] = static_cast<std::uint16_t>(r);
    }
    // ten bands, 50 Hz .. 12.8 kHz, each 0.8 octave
    const auto binHz = rate / static_cast<double>(fftSize);
    for (std::size_t b = 0; b <= bandCount; ++b)
    {
        const auto hz = 50.0 * std::exp2(0.8 * static_cast<double>(b));
        bandEdge[b] = static_cast<std::uint16_t>(std::clamp<long>(std::lround(hz / binHz), 1, fftSize / 2));
    }
    for (std::size_t b = 1; b <= bandCount; ++b)
        bandEdge[b] = std::max<std::uint16_t>(bandEdge[b], static_cast<std::uint16_t>(bandEdge[b - 1] + 1));
    framesPerWindow = static_cast<std::uint32_t>(
        std::clamp(windowSeconds * rate / fftSize, 8.0, static_cast<double>(maxFramesPerWindow)));
    reset();
}

void HabitatHealth::reset() noexcept
{
    input.fill(0.0f);
    filled = 0;
    frames = 0;
    intensitySum.fill(0.0);
    changeSum.fill(0.0);
    previous.fill(0.0f);
    hasPrevious = false;
    last = {};
}

void HabitatHealth::push(float sample) noexcept
{
    input[filled++] = std::isfinite(sample) ? sample : 0.0f;
    if (filled == fftSize)
    {
        filled = 0;
        analyseFrame();
    }
}

void HabitatHealth::analyseFrame() noexcept
{
    for (std::size_t n = 0; n < fftSize; ++n)
        spectrum[bitReverse[n]] = { input[n] * window[n], 0.0f };
    for (std::size_t size = 2; size <= fftSize; size <<= 1)
    {
        const auto half = size / 2, stride = fftSize / size;
        for (std::size_t start = 0; start < fftSize; start += size)
            for (std::size_t k = 0; k < half; ++k)
            {
                const auto t = twiddle[k * stride] * spectrum[start + k + half];
                spectrum[start + k + half] = spectrum[start + k] - t;
                spectrum[start + k] += t;
            }
    }
    // band mean square, scaled so that a sine of amplitude A reads A^2 / 2
    // (Hann: peak |X| = A N / 4, equivalent noise bandwidth 1.5 bins)
    constexpr float scale = 8.0f / (1.5f * static_cast<float>(fftSize) * static_cast<float>(fftSize));
    for (std::size_t b = 0; b < bandCount; ++b)
    {
        float power = 0.0f;
        for (std::size_t k = bandEdge[b]; k < bandEdge[b + 1]; ++k)
            power += std::norm(spectrum[k]);
        power *= scale;
        bandDb[frames][b] = 10.0f * std::log10(std::max(power, 1.0e-20f));
        const auto intensity = std::sqrt(power);
        intensitySum[b] += intensity;
        if (hasPrevious)
            changeSum[b] += std::abs(intensity - previous[b]);
        previous[b] = intensity;
    }
    hasPrevious = true;
    if (++frames >= framesPerWindow)
        closeWindow();
}

void HabitatHealth::closeWindow() noexcept
{
    // the threshold: 50 dB under the loudest band of the window
    float loudest = -200.0f;
    for (std::uint32_t f = 0; f < frames; ++f)
        for (const auto db : bandDb[f])
            loudest = std::max(loudest, db);
    std::array<std::uint32_t, bandCount> active {};
    if (loudest > silenceDb)
        for (std::uint32_t f = 0; f < frames; ++f)
            for (std::size_t b = 0; b < bandCount; ++b)
                if (bandDb[f][b] > loudest + thresholdDb)
                    ++active[b];
    // ADI: Shannon entropy of the bands' occupancy, normalised by ln(bands)
    double total = 0.0;
    for (const auto a : active)
        total += a;
    double entropy = 0.0;
    if (total > 0.0)
        for (const auto a : active)
            if (a > 0)
            {
                const auto p = static_cast<double>(a) / total;
                entropy -= p * std::log(p);
            }
    Reading r;
    r.adi = static_cast<float>(entropy / std::log(static_cast<double>(bandCount)));
    // ACI: per sounding band, the changes over the intensity
    double aci = 0.0;
    int sounding = 0;
    for (std::size_t b = 0; b < bandCount; ++b)
        if (active[b] * 10u >= frames && intensitySum[b] > 1.0e-9)
        {
            aci += changeSum[b] / intensitySum[b];
            ++sounding;
        }
    r.aci = sounding > 0 ? static_cast<float>(aci / sounding) : 0.0f;
    r.health = std::clamp(0.5f * r.adi + 0.5f * std::min(1.0f, r.aci / aciReference), 0.0f, 1.0f);
    r.measured = total > 0.0;
    if (!r.measured)
        r.health = 0.5f;
    last = r;
    frames = 0;
    intensitySum.fill(0.0);
    changeSum.fill(0.0);
}
}
