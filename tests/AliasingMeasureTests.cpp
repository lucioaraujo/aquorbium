// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

// Aliasing of the non-linear stages (aquorbium-tarefas.md, phase 7b,
// 2026-10-23). The method: a periodic signal whose fundamental grid sits on
// a PRIME number of FFT bins q (N = 65536, a power of two, is not divisible
// by q). Every component a stage really makes falls on a multiple of q; a
// component folded back from above Nyquist lands at N - k*q, which is not a
// multiple of q. So the energy OFF the grid, against the total, is the
// aliasing (plus any noise of the stage). A pure sine and a naive sawtooth
// are the negative and positive controls of the metric.
#include "core/AdvancedVoiceEcologyModules.h"
#include "core/BiomaPhysicalString.h"

#include <algorithm>
#include <cmath>
#include <complex>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

namespace
{
int failures = 0;
void check(bool condition, const std::string& what)
{
    if (!condition)
    {
        std::cerr << "FAILED: " << what << '\n';
        ++failures;
    }
}

constexpr double rate = 48000.0;
constexpr std::size_t fftSize = 65536;
constexpr double pi = 3.14159265358979323846;

void fft(std::vector<std::complex<double>>& a)
{
    const auto n = a.size();
    for (std::size_t i = 1, j = 0; i < n; ++i)
    {
        auto bit = n >> 1;
        for (; j & bit; bit >>= 1)
            j ^= bit;
        j ^= bit;
        if (i < j)
            std::swap(a[i], a[j]);
    }
    for (std::size_t length = 2; length <= n; length <<= 1)
    {
        const auto angle = -2.0 * pi / static_cast<double>(length);
        const std::complex<double> step(std::cos(angle), std::sin(angle));
        for (std::size_t i = 0; i < n; i += length)
        {
            std::complex<double> w(1.0, 0.0);
            for (std::size_t k = 0; k < length / 2; ++k)
            {
                const auto u = a[i + k];
                const auto v = a[i + k + length / 2] * w;
                a[i + k] = u + v;
                a[i + k + length / 2] = u - v;
                w *= step;
            }
        }
    }
}

bool isPrime(std::size_t n)
{
    if (n < 2)
        return false;
    for (std::size_t d = 2; d * d <= n; ++d)
        if (n % d == 0)
            return false;
    return true;
}

// the prime grid (in bins) nearest a frequency
std::size_t primeGrid(double hz)
{
    auto q = static_cast<std::size_t>(std::lround(hz * static_cast<double>(fftSize) / rate));
    while (!isPrime(q))
        ++q;
    return q;
}
double gridHz(std::size_t q) { return static_cast<double>(q) * rate / static_cast<double>(fftSize); }

// off-grid energy against the total, dB; the signal is warmed up first.
// lowBandDb gets the same ratio for the off-grid energy under 5 kHz, where a
// folded component is heard as an inharmonic tone rather than hiss near
// Nyquist.
double lowBandDb = 0.0;
double aliasingDb(std::size_t q, const std::function<float()>& next, double warmSeconds = 1.0)
{
    for (std::size_t i = 0; i < static_cast<std::size_t>(rate * warmSeconds); ++i)
        static_cast<void>(next());
    std::vector<std::complex<double>> buffer(fftSize);
    for (std::size_t i = 0; i < fftSize; ++i)
    {
        const auto window = 0.5 - 0.5 * std::cos(2.0 * pi * static_cast<double>(i) / static_cast<double>(fftSize));
        buffer[i] = { static_cast<double>(next()) * window, 0.0 };
    }
    fft(buffer);
    double on = 0.0, off = 0.0, offLow = 0.0;
    const auto lowBins = static_cast<std::size_t>(5000.0 * static_cast<double>(fftSize) / rate);
    for (std::size_t k = 1; k < fftSize / 2; ++k)
    {
        const auto energy = std::norm(buffer[k]);
        const auto r = k % q;
        const auto distance = std::min(r, q - r);
        if (distance <= 3)  // the Hann main lobe and a little more
            on += energy;
        else
        {
            off += energy;
            if (k < lowBins)
                offLow += energy;
        }
    }
    lowBandDb = 10.0 * std::log10(std::max(offLow, 1.0e-30) / std::max(on + off, 1.0e-30));
    return 10.0 * std::log10(std::max(off, 1.0e-30) / std::max(on + off, 1.0e-30));
}

void report(const std::string& name, double db)
{
    std::cout << "  " << name << ": " << db << " dB off the harmonic grid (" << lowBandDb << " dB under 5 kHz)\n";
}
}

int main()
{
    // ---- the metric's controls ----------------------------------------------
    {
        const auto q = primeGrid(2000.0);
        double phase = 0.0;
        const auto sine = aliasingDb(q, [&] {
            phase += gridHz(q) / rate;
            phase -= std::floor(phase);
            return static_cast<float>(std::sin(2.0 * pi * phase));
        });
        std::cout << "controls at " << gridHz(q) << " Hz:\n";
        report("pure sine (negative control)", sine);
        phase = 0.0;
        const auto naiveSaw = aliasingDb(q, [&] {
            phase += gridHz(q) / rate;
            phase -= std::floor(phase);
            return static_cast<float>(2.0 * phase - 1.0);
        });
        report("naive sawtooth (positive control)", naiveSaw);
        check(sine < -100.0, "metric: a pure sine is clean");
        check(naiveSaw > -40.0, "metric: a naive sawtooth shows its aliasing");
    }

    // ---- BiomaBassVoice: polyBLEP saws, sine FM, the oversampled folder, the
    // tanh ladder and the tanh output. Detune 0 keeps it periodic; the grid
    // is f0/2 (sub an octave down, FM at 1.5).
    std::cout << "BiomaBassVoice:\n";
    double worstBass = -300.0;
    for (const auto hz : { 55.0, 220.0, 880.0 })
        for (const auto fold : { 0.0f, 0.35f, 1.0f })
        {
            const auto q = primeGrid(hz / 2.0);
            aquorbium::BiomaBassVoice voice;
            voice.prepare(rate);
            aquorbium::BiomaBassVoice::Parameters p;
            p.frequencyHz = static_cast<float>(gridHz(q) * 2.0);
            p.detuneCents = 0.0f;
            p.fold = fold;
            p.cutoff = 1.0f;  // open: the filter must not hide what the folder makes
            voice.setParameters(p);
            const auto db = aliasingDb(q, [&] { return voice.tick(); }, 2.0);
            report(std::to_string(static_cast<int>(hz)) + " Hz, fold " + std::to_string(fold).substr(0, 4), db);
            worstBass = std::max(worstBass, db);
        }

    // ---- SchoolWaveVoice: sine/triangle/polyBLEP saw, then the wave cut ----
    std::cout << "SchoolWaveVoice (spread 0):\n";
    double schoolCutCentre = -300.0, schoolCutEdge = -300.0;
    for (const auto hz : { 220.0, 1760.0 })
        for (const auto morph : { 0.0f, 0.5f, 1.0f })
            for (const auto cut : { 0.5f, 0.0f, 1.0f })
            {
                const auto q = primeGrid(hz);
                aquorbium::SchoolWaveVoice voice;
                voice.prepare(rate);
                aquorbium::SchoolWaveVoice::Parameters p;
                p.frequencyHz = static_cast<float>(gridHz(q));
                p.morph = morph;
                p.waveCut = cut;
                p.spread = 0.0f;
                p.level = 1.0f;
                voice.setParameters(p);
                const auto db = aliasingDb(q, [&] { return voice.tick(0.0f, 0.0f).left; });
                report(std::to_string(static_cast<int>(hz)) + " Hz, morph " + std::to_string(morph).substr(0, 3)
                           + ", cut " + std::to_string(cut).substr(0, 3), db);
                if (cut == 0.5f)
                    schoolCutCentre = std::max(schoolCutCentre, db);
                else
                    schoolCutEdge = std::max(schoolCutEdge, db);
            }

    // ---- HarmonicRippleVoice: additive harmonics 2..9 under a pulse --------
    std::cout << "HarmonicRippleVoice (8 ripples, formant 1):\n";
    double rippleWorst = -300.0;
    for (const auto hz : { 220.0, 1000.0, 3000.0 })
    {
        const auto q = primeGrid(hz);
        aquorbium::HarmonicRippleVoice voice;
        voice.prepare(rate);
        aquorbium::HarmonicRippleVoice::Parameters p;
        p.frequencyHz = static_cast<float>(gridHz(q));
        p.ripples = 8;
        p.formant = 1.0f;
        p.cluster = 0.35f;
        p.level = 1.0f;
        voice.setParameters(p);
        const auto db = aliasingDb(q, [&] { return voice.tick(); });
        report(std::to_string(static_cast<int>(hz)) + " Hz", db);
        rippleWorst = std::max(rippleWorst, db);
    }

    // ---- the string's energy barrier, a sine through it --------------------
    std::cout << "BiomaPhysicalString::energyBarrier (sine in):\n";
    double barrierWorst = -300.0;
    for (const auto hz : { 440.0, 2000.0 })
        for (const auto amplitude : { 0.85, 1.2, 2.0 })
        {
            const auto q = primeGrid(hz);
            double phase = 0.0;
            const auto db = aliasingDb(q, [&] {
                phase += gridHz(q) / rate;
                phase -= std::floor(phase);
                return aquorbium::BiomaPhysicalString::energyBarrier(
                    static_cast<float>(amplitude * std::sin(2.0 * pi * phase)));
            }, 0.0);
            report(std::to_string(static_cast<int>(hz)) + " Hz, peak " + std::to_string(amplitude).substr(0, 4), db);
            barrierWorst = std::max(barrierWorst, db);
        }

    // regression guards at the values measured on 2026-10-23 (+3 dB margin);
    // they are not quality judgements - those are in OUTPUT_STAGE_DESIGN.md
    std::cout << "worst: bass " << worstBass << ", school (cut centred) " << schoolCutCentre
              << ", school (cut at the edges) " << schoolCutEdge << ", ripple " << rippleWorst
              << ", string barrier " << barrierWorst << " dB\n";
    check(worstBass <= -42.0, "BiomaBassVoice aliasing (measured -45 dB at 880 Hz)");
    check(schoolCutCentre <= -26.7, "SchoolWaveVoice aliasing, cut centred (measured -29.8 dB)");
    check(schoolCutEdge <= -26.3, "SchoolWaveVoice aliasing, cut at the edges (measured -29.3 dB; -10 dB before the BLEP)");
    check(rippleWorst <= -61.5, "HarmonicRippleVoice aliasing (measured -64.5 dB; -29 dB at 3 kHz before the band limit)");
    check(barrierWorst <= -37.8, "string energy barrier aliasing, 2 kHz at twice its limit (measured -40.9 dB)");

    if (failures == 0)
        std::cout << "aliasing measurement: all checks passed\n";
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
