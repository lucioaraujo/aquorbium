// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

// The synthesis device's voice (2026-10-21): the tool is measured, not
// judged by taste - tuning, band-limiting, the pulse's symmetry, the noise
// colours, the filter's slopes and modes, the envelope times, the native
// passthrough, bounds and determinism.
#include "core/BiomaEngine.h"
#include "core/SynthPatchCodec.h"
#include "core/SynthVoice.h"

#include <algorithm>
#include <cmath>
#include <memory>
#include <cstdlib>
#include <iostream>
#include <vector>

namespace
{
using aquorbium::SynthVoice;
int failures = 0;
void check(bool condition, const char* what)
{
    if (!condition)
    {
        std::cerr << "FAILED: " << what << '\n';
        ++failures;
    }
}
constexpr double pi = 3.14159265358979323846;
constexpr double rate = 48000.0;

// a sustained note through an open voice: the second half
std::vector<float> play(SynthVoice::Patch patch, float frequency, double seconds = 1.0, float native = 0.0f)
{
    SynthVoice v;
    v.prepare(rate, 7u);
    patch.amp = { 0.001f, 0.001f, 1.0f, 0.05f, 0.5f };
    patch.accentToLevel = 0.0f;
    v.setPatch(patch);
    v.noteOn(1.0f);
    std::vector<float> out;
    const auto n = static_cast<int>(rate * seconds);
    for (int i = 0; i < n; ++i)
    {
        const auto y = v.render(frequency, native);
        if (i >= n / 2)
            out.push_back(y);
    }
    return out;
}
double partial(const std::vector<float>& x, double hz)
{
    double re = 0.0, im = 0.0;
    for (std::size_t i = 0; i < x.size(); ++i)
    {
        const auto w = 0.5 - 0.5 * std::cos(2.0 * pi * static_cast<double>(i) / static_cast<double>(x.size()));
        re += x[i] * w * std::cos(2.0 * pi * hz * static_cast<double>(i) / rate);
        im -= x[i] * w * std::sin(2.0 * pi * hz * static_cast<double>(i) / rate);
    }
    return std::sqrt(re * re + im * im);
}
double cents(const std::vector<float>& x, double f)
{
    const auto period = rate / f;
    const auto lo = static_cast<int>(period * 0.8), hi = static_cast<int>(period * 1.2);
    std::vector<double> ac(static_cast<std::size_t>(hi + 2), 0.0);
    for (int lag = lo - 1; lag <= hi + 1; ++lag)
        for (std::size_t i = 0; i + static_cast<std::size_t>(lag) < x.size(); ++i)
            ac[static_cast<std::size_t>(lag)] += static_cast<double>(x[i]) * x[i + static_cast<std::size_t>(lag)];
    auto best = lo;
    for (int lag = lo; lag <= hi; ++lag)
        if (ac[static_cast<std::size_t>(lag)] > ac[static_cast<std::size_t>(best)])
            best = lag;
    const auto a = ac[static_cast<std::size_t>(best - 1)], b = ac[static_cast<std::size_t>(best)],
               c = ac[static_cast<std::size_t>(best + 1)];
    return 1200.0 * std::log2(period / (best + 0.5 * (a - c) / (a - 2.0 * b + c)));
}
SynthVoice::Patch openOscillator(float shape)
{
    SynthVoice::Patch p;
    p.sourceA = SynthVoice::Source::oscillator;
    p.shapeA = shape;
    p.cutoff = 20000.0f;
    p.keytrack = 0.0f;
    p.filterEnv = 0.0f;
    p.resonance = 0.0f;
    p.level = 1.0f;
    return p;
}
}

int main()
{
    // ---- tuning, within a cent (measured on the sine - the phase is the
    // same for every shape, and the autocorrelation is biased on sharp
    // edges at short periods) ----
    bool tuned = true;
    for (const auto f : { 110.0f, 440.0f, 1760.0f })
    {
        const auto c = cents(play(openOscillator(0.0f), f), f);
        std::cout << "sine at " << f << " Hz: " << c << " cents; saw: " << cents(play(openOscillator(2.0f / 3.0f), f), f) << '\n';
        tuned = tuned && std::abs(c) < 1.0;
    }
    check(tuned, "the oscillator is in tune (1 cent)");

    // ---- band-limiting: a 2489 Hz saw (aliases would land between partials) ----
    {
        const auto f = 2489.0;
        const auto x = play(openOscillator(2.0f / 3.0f), static_cast<float>(f));
        const auto fundamental = partial(x, f);
        double worstAlias = 0.0;
        // aliases of partials above Nyquist fold to |k f - n rate|; check a band of them
        for (int k = 10; k <= 30; ++k)
        {
            const auto folded = std::abs(k * f - rate * std::round(k * f / rate));
            if (folded > 100.0 && std::abs(std::fmod(folded, f)) > 100.0 && std::abs(std::fmod(folded, f) - f) > 100.0)
                worstAlias = std::max(worstAlias, partial(x, folded));
        }
        const auto aliasDb = 20.0 * std::log10(worstAlias / fundamental + 1.0e-12);
        std::cout << "saw at 2489 Hz: worst alias " << aliasDb << " dB under the fundamental\n";
        check(aliasDb < -30.0, "the saw is band-limited (PolyBLEP): aliases 30 dB down");
    }

    // ---- the pulse at 50 %: odd partials only; the saw has both ----
    {
        auto p = openOscillator(1.0f);
        p.widthA = 0.5f;
        const auto x = play(p, 220.0f);
        const auto h1 = partial(x, 220.0), h2 = partial(x, 440.0), h3 = partial(x, 660.0);
        check(20.0 * std::log10(h2 / h1) < -40.0 && std::abs(20.0 * std::log10(h3 / h1) + 9.5) < 1.5,
              "a square: no even partials, the third at 1/3");
        const auto saw = play(openOscillator(2.0f / 3.0f), 220.0f);
        check(std::abs(20.0 * std::log10(partial(saw, 440.0) / partial(saw, 220.0)) + 6.0) < 1.0,
              "a saw: the second partial at 1/2");
        const auto sine = play(openOscillator(0.0f), 220.0f);
        check(20.0 * std::log10(partial(sine, 660.0) / partial(sine, 220.0)) < -60.0, "a sine is pure");
    }

    // ---- noise colours: slope per octave between 500 Hz and 4 kHz ----
    {
        const auto slope = [](float colour) {
            SynthVoice::Patch p;
            p.sourceA = SynthVoice::Source::noise;
            p.colourA = colour;
            p.cutoff = 20000.0f;
            p.keytrack = 0.0f;
            p.filterEnv = 0.0f;
            p.resonance = 0.0f;
            const auto x = play(p, 100.0f, 8.0);
            // average power in narrow bands around 500 and 4000 Hz
            double low = 0.0, high = 0.0;
            for (int k = -20; k <= 20; ++k)
            {
                low += std::pow(partial(x, 500.0 + k * 2.0), 2.0);
                high += std::pow(partial(x, 4000.0 + k * 16.0), 2.0);
            }
            return 10.0 * std::log10(high / low) / 3.0;  // dB per octave (3 octaves)
        };
        const auto white = slope(0.0f), pink = slope(0.5f), brown = slope(1.0f);
        std::cout << "noise slopes: white " << white << ", pink " << pink << ", brown " << brown << " dB/octave\n";
        check(std::abs(white) < 1.0, "white noise is flat");
        check(std::abs(pink + 3.0) < 1.0, "pink noise falls 3 dB an octave (Kellet)");
        check(brown < -5.0, "brown noise falls about 6 dB an octave");
    }

    // ---- the filter: slopes and modes on a sine sweep ----
    {
        const auto response = [](SynthVoice::FilterMode mode, float hz) {
            SynthVoice::Patch p = openOscillator(0.0f);
            p.filterMode = mode;
            p.cutoff = 1000.0f;
            p.resonance = 0.0f;
            const auto x = play(p, hz, 0.5);
            double power = 0.0;
            for (const auto v : x)
                power += static_cast<double>(v) * v;
            return 10.0 * std::log10(power / static_cast<double>(x.size()) / 0.5);  // a sine's power is 1/2
        };
        using M = SynthVoice::FilterMode;
        const auto lp = response(M::lowPass, 4000.0f), lpPass = response(M::lowPass, 100.0f);
        const auto hp = response(M::highPass, 250.0f), hpPass = response(M::highPass, 10000.0f);
        const auto bpCentre = response(M::bandPass, 1000.0f), bpOff = response(M::bandPass, 8000.0f);
        const auto notch = response(M::notch, 1000.0f);
        std::cout << "filter: LP 4 kHz " << lp << " dB, HP 250 Hz " << hp << " dB, BP centre " << bpCentre
                  << " dB, notch at cutoff " << notch << " dB\n";
        check(std::abs(lpPass) < 0.5 && lp < -20.0 && lp > -28.0, "low-pass: flat below, -24 dB two octaves above (12 dB/oct)");
        check(std::abs(hpPass) < 0.5 && hp < -20.0 && hp > -28.0, "high-pass: the mirror");
        check(std::abs(bpCentre) < 0.5 && bpOff < -10.0, "band-pass: unity at the centre, falling away");
        check(notch < -30.0, "notch: a hole at the cutoff");
    }

    // ---- envelopes: the times are what they say ----
    {
        SynthVoice v;
        v.prepare(rate, 1u);
        SynthVoice::Patch p = openOscillator(0.0f);
        p.amp = { 0.1f, 0.2f, 0.5f, 0.3f, 0.5f };
        p.accentToLevel = 0.0f;
        v.setPatch(p);
        v.noteOn(1.0f);
        int peakAt = -1, releaseEnd = -1;
        for (int i = 0; i < static_cast<int>(rate * 2.0); ++i)
        {
            static_cast<void>(v.render(440.0f, 0.0f));
            if (peakAt < 0 && v.ampLevel() >= 0.999f)
                peakAt = i;
            if (i == static_cast<int>(rate * 1.0))
            {
                check(std::abs(v.ampLevel() - 0.5f) < 1.0e-4f, "the sustain holds its level");
                v.noteOff();
            }
            if (i > static_cast<int>(rate * 1.0) && releaseEnd < 0 && v.ampLevel() <= 0.0f)
                releaseEnd = i - static_cast<int>(rate * 1.0);
        }
        std::cout << "envelope: peak at " << peakAt / 48.0 << " ms (100), release in " << releaseEnd / 48.0 << " ms (300)\n";
        check(std::abs(peakAt / rate - 0.1) < 0.002, "the attack lasts what it says");
        check(std::abs(releaseEnd / rate - 0.3) < 0.002, "the release lasts what it says");
    }

    // ---- the native source passes the organism's own body through ----
    {
        SynthVoice v;
        v.prepare(rate, 1u);
        SynthVoice::Patch p;
        p.sourceA = SynthVoice::Source::native;
        p.cutoff = 20000.0f;
        p.keytrack = 0.0f;
        p.filterEnv = 0.0f;
        p.resonance = 0.0f;
        p.level = 1.0f;
        p.accentToLevel = 0.0f;
        p.amp = { 0.001f, 0.001f, 1.0f, 0.05f, 0.5f };
        v.setPatch(p);
        v.noteOn(1.0f);
        double dot = 0.0, a2 = 0.0, b2 = 0.0;
        for (int i = 0; i < 48000; ++i)
        {
            const auto in = static_cast<float>(0.5 * std::sin(2.0 * pi * 300.0 * i / rate));
            const auto out = v.render(220.0f, in);
            if (i > 1000)
            {
                dot += static_cast<double>(in) * out;
                a2 += static_cast<double>(in) * in;
                b2 += static_cast<double>(out) * out;
            }
        }
        check(dot / std::sqrt(a2 * b2) > 0.99 && std::abs(std::sqrt(b2 / a2) - 1.0) < 0.05,
              "the NATIVE source is the organism's own sound, unchanged");
    }

    // ---- bounds and determinism: everything at its extreme ----
    {
        SynthVoice::Patch p;
        p.sourceA = SynthVoice::Source::oscillator;
        p.sourceB = SynthVoice::Source::noise;
        p.levelB = 1.0f;
        p.ring = 1.0f;
        p.fm = 1.0f;
        p.subA = 1.0f;
        p.resonance = 1.0f;
        p.drive = 1.0f;
        p.filterEnv = 4.0f;
        p.wah = 3.0f;
        p.vibrato = 100.0f;
        p.drift = 1.0f;
        SynthVoice a, b;
        a.prepare(rate, 3u);
        b.prepare(rate, 3u);
        a.setPatch(p);
        b.setPatch(p);
        a.noteOn(1.0f);
        b.noteOn(1.0f);
        bool bounded = true, same = true;
        for (int i = 0; i < 96000; ++i)
        {
            const auto x = a.render(330.0f, 0.0f), y = b.render(330.0f, 0.0f);
            bounded = bounded && std::isfinite(x) && std::abs(x) < 20.0f;
            same = same && x == y;
        }
        check(bounded, "everything at its extreme stays finite and bounded");
        check(same, "the same seed gives the same sound");
    }

    // ---- phase 3: the Rasgo Modular's sources and the bodies ----
    {
        using Src = SynthVoice::Source;
        const auto with = [](Src source, float shape, float width, float colour) {
            auto p = openOscillator(0.0f);
            p.sourceA = source;
            p.shapeA = shape;
            p.widthA = width;
            p.colourA = colour;
            return p;
        };
        const auto db = [](const std::vector<float>& x, double hz, double ref) {
            return 20.0 * std::log10(partial(x, hz) / partial(x, ref) + 1.0e-12);
        };
        // FM: no index, a sine; ratio 1 makes harmonic sidebands, ratio 1.5 half-harmonic ones
        {
            const auto pure = play(with(Src::fm, 1.0f / 9.0f, 0.0f, 0.0f), 220.0f);
            const auto harmonic = play(with(Src::fm, 1.0f / 9.0f, 0.6f, 0.0f), 220.0f);
            const auto half = play(with(Src::fm, 2.0f / 9.0f, 0.6f, 0.0f), 220.0f);
            // ratio 1.5: the sidebands fall at 220 +- 330 k - at 110 Hz, never at 330
            std::cout << "FM: H2 " << db(pure, 440, 220) << " dB at index 0, " << db(harmonic, 440, 220)
                      << " dB at index 0.6; ratio 1.5: 110 Hz " << db(half, 110, 220) << " dB\n";
            check(db(pure, 440, 220) < -60.0, "FM at index 0 is a sine");
            check(db(harmonic, 440, 220) > -20.0, "FM with index makes sidebands");
            check(db(half, 110, 220) > -25.0 && db(harmonic, 110, 220) < -50.0, "the ratio decides where the sidebands fall");
            check(std::abs(cents(play(with(Src::fm, 1.0f / 9.0f, 0.4f, 0.3f), 220.0f), 220.0)) < 1.0, "FM is in tune");
        }
        // WAVETABLE: the saw end, the sine end, band-limited
        {
            const auto saw = play(with(Src::wavetable, 0.0f, 0.0f, 0.0f), 220.0f);
            const auto sineEnd = play(with(Src::wavetable, 1.0f, 0.0f, 0.0f), 220.0f);
            check(std::abs(db(saw, 440, 220) + 6.0) < 1.0, "wavetable position 0: a saw");
            check(db(sineEnd, 440, 220) < -40.0, "wavetable position 1: a sine");
            const auto f = 2489.0;
            const auto high = play(with(Src::wavetable, 0.0f, 0.0f, 0.0f), static_cast<float>(f));
            double worst = 0.0;
            for (int k = 10; k <= 30; ++k)
            {
                const auto folded = std::abs(k * f - rate * std::round(k * f / rate));
                if (folded > 100.0 && std::abs(std::fmod(folded, f)) > 100.0 && std::abs(std::fmod(folded, f) - f) > 100.0)
                    worst = std::max(worst, partial(high, folded));
            }
            const auto alias = 20.0 * std::log10(worst / partial(high, f) + 1.0e-12);
            std::cout << "wavetable: saw H2 " << db(saw, 440, 220) << " dB; worst alias at 2489 Hz " << alias << " dB\n";
            check(alias < -40.0, "the wavetable's mip-maps keep it band-limited");
        }
        // ADDITIVE: tilt brightens; odd only removes the even partials; stretch detunes the upper partials
        {
            const auto dark = play(with(Src::additive, 0.1f, 0.5f, 0.5f), 220.0f);
            const auto bright = play(with(Src::additive, 0.9f, 0.5f, 0.5f), 220.0f);
            const auto odd = play(with(Src::additive, 0.6f, 1.0f, 0.5f), 220.0f);
            const auto stretched = play(with(Src::additive, 0.8f, 0.5f, 1.0f), 220.0f);
            check(db(bright, 2200, 220) > db(dark, 2200, 220) + 20.0, "additive tilt: brighter");
            check(db(odd, 440, 220) < -40.0 && db(odd, 660, 220) > -20.0, "additive odd: only the odd partials");
            // partial 10 moves from 10 f to 10 f (1 + 0.004 * 9) - 2280 Hz
            check(partial(stretched, 2200.0 * 1.036) > 10.0 * partial(stretched, 2200.0), "additive stretch: the partials go inharmonic");
        }
        // PULSAR: the pitch stays, the formant moves the spectrum
        {
            const auto centroid = [](const std::vector<float>& x) {
                double num = 0.0, den = 0.0;
                for (int k = 1; k <= 40; ++k)
                {
                    const auto a = partial(x, 110.0 * k);
                    num += a * k;
                    den += a;
                }
                return num / den;
            };
            const auto low = play(with(Src::pulsar, 0.45f, 0.0f, 0.5f), 110.0f);
            const auto high = play(with(Src::pulsar, 0.85f, 0.0f, 0.5f), 110.0f);
            std::cout << "pulsar: spectral centroid harmonic " << centroid(low) << " -> " << centroid(high) << '\n';
            check(centroid(high) > centroid(low) + 3.0, "pulsar: a higher formant, a brighter sound");
            check(std::abs(cents(high, 110.0)) < 2.0, "pulsar: the pitch does not move with the formant");
        }
        // STRING: plucked in tune, it decays; more sustain, a longer decay
        {
            const auto decayRatio = [](float sustain) {
                SynthVoice v;
                v.prepare(rate, 3u);
                auto p = openOscillator(0.0f);
                p.sourceA = SynthVoice::Source::string;
                p.shapeA = 0.4f;
                p.widthA = 0.3f;
                p.colourA = sustain;
                p.amp = { 0.001f, 0.001f, 1.0f, 0.05f, 0.5f };
                v.setPatch(p);
                v.noteOn(1.0f);
                std::vector<float> x;
                for (int i = 0; i < 96000; ++i)
                    x.push_back(v.render(196.0f, 0.0f));
                const auto energy = [&x](int from) {
                    double e = 0.0;
                    for (int i = from; i < from + 4800; ++i)
                        e += static_cast<double>(x[static_cast<std::size_t>(i)]) * x[static_cast<std::size_t>(i)];
                    return e;
                };
                return std::make_pair(10.0 * std::log10(energy(48000) / energy(2400)),
                                      cents(std::vector<float>(x.begin() + 2400, x.begin() + 26400), 196.0));
            };
            const auto [shortDecay, shortCents] = decayRatio(0.2f);
            const auto [longDecay, longCents] = decayRatio(0.95f);
            std::cout << "string: after 1 s " << shortDecay << " dB (sustain 0.2), " << longDecay << " dB (0.95); pitch "
                      << longCents << " cents\n";
            check(shortDecay < -20.0 && longDecay > shortDecay + 10.0, "string: it decays, longer with more sustain");
            check(std::abs(longCents) < 3.0, "string: plucked in tune (Jaffe & Smith's all-pass)");
        }
        // MATTER: harmonic at structure 0, stretched at 1
        {
            const auto peakNear = [](const std::vector<float>& x, double hz) {
                double best = 0.0, at = hz;
                for (double f = hz * 0.97; f <= hz * 1.25; f += 2.0)
                    if (partial(x, f) > best)
                    {
                        best = partial(x, f);
                        at = f;
                    }
                return at;
            };
            const auto harmonic = play(with(Src::matter, 0.0f, 0.7f, 0.2f), 200.0f, 0.4);
            const auto stretched = play(with(Src::matter, 1.0f, 0.7f, 0.2f), 200.0f, 0.4);
            const auto second0 = peakNear(harmonic, 400.0), second1 = peakNear(stretched, 400.0);
            std::cout << "matter: the second mode at " << second0 << " Hz (structure 0) and " << second1 << " Hz (1)\n";
            check(std::abs(second0 - 400.0) < 6.0 && second1 > 415.0, "matter: structure stretches the modes");
        }
        // BODY: the organisms' bodies as a source - the reed's odd partials
        {
            auto p = openOscillator(0.0f);
            p.sourceA = Src::body;
            p.bodyA = static_cast<std::uint8_t>(aquorbium::TimbreBody::Kind::reed);
            p.shapeA = 0.5f;
            const auto reed = play(p, 220.0f, 2.0);
            check(db(reed, 660, 220) > db(reed, 440, 220) + 20.0, "a body as a source: the reed sounds its odd partials");
        }
        // every source at its extremes: bounded
        {
            bool bounded = true;
            for (int k = 4; k < static_cast<int>(SynthVoice::sourceCount); ++k)
                for (const auto m : { 0.0f, 1.0f })
                {
                    auto p = with(static_cast<Src>(k), m, m, m);
                    p.bodyA = 11;
                    for (const auto f : { 30.0f, 4000.0f })
                    {
                        const auto x = play(p, f, 0.3);
                        for (const auto v : x)
                            bounded = bounded && std::isfinite(v) && std::abs(v) < 8.0f;
                    }
                }
            check(bounded, "every new source, at its extremes and at both ends of the range, stays bounded");
        }
    }

    // ---- the modulation matrix: the ecosystem plays the parameters ----
    {
        using D = SynthVoice::ModDestination;
        using S = SynthVoice::ModSource;
        const auto brightness = [](float energy, bool routed) {
            SynthVoice v;
            v.prepare(rate, 5u);
            auto p = openOscillator(2.0f / 3.0f);
            p.cutoff = 300.0f;
            p.amp = { 0.001f, 0.001f, 1.0f, 0.05f, 0.5f };
            p.accentToLevel = 0.0f;
            if (routed)
                p.routes[0] = { S::energy, D::cutoff, 1.0f };  // +4 octaves at full energy
            v.setPatch(p);
            SynthVoice::Ecology e;
            e.energy = energy;
            v.setEcology(e);
            v.noteOn(1.0f);
            std::vector<float> x;
            for (int i = 0; i < 48000; ++i)
            {
                const auto y = v.render(110.0f, 0.0f);
                if (i >= 24000)
                    x.push_back(y);
            }
            return 20.0 * std::log10(partial(x, 2200.0) / partial(x, 110.0));  // the 20th partial
        };
        const auto still = brightness(0.0f, true), alive = brightness(1.0f, true), unrouted = brightness(1.0f, false);
        std::cout << "matrix: energy -> cutoff: 20th partial " << still << " dB at rest, " << alive
                  << " dB at full energy (unrouted " << unrouted << ")\n";
        check(alive > still + 20.0, "the organism's energy opens the filter");
        check(std::abs(unrouted - still) < 0.5, "without a route, the energy changes nothing");

        // LFO 1 (a slow square) -> pitch, one semitone: two pitches
        SynthVoice v;
        v.prepare(rate, 5u);
        auto p = openOscillator(0.0f);
        p.lfo1 = { 0.5f, SynthVoice::LfoShape::square };
        p.routes[1] = { S::lfo1, D::pitch, 1.0f / 12.0f };
        p.amp = { 0.001f, 0.001f, 1.0f, 0.05f, 0.5f };
        v.setPatch(p);
        v.noteOn(1.0f);
        std::vector<float> high, low;
        for (int i = 0; i < 96000; ++i)
        {
            const auto y = v.render(440.0f, 0.0f);
            if (i > 2400 && i < 45600)
                high.push_back(y);  // the square's first half: +1
            if (i > 50400 && i < 93600)
                low.push_back(y);   // the second half: -1
        }
        const auto up = cents(high, 440.0), down = cents(low, 440.0);
        std::cout << "matrix: LFO -> pitch: " << up << " and " << down << " cents (+-100)\n";
        check(std::abs(up - 100.0) < 2.0 && std::abs(down + 100.0) < 2.0, "the LFO moves the pitch by what the route says");

        // every route at its extreme: bounded
        SynthVoice wild;
        wild.prepare(rate, 9u);
        auto w = openOscillator(1.0f);
        w.sourceB = SynthVoice::Source::noise;
        for (std::size_t r = 0; r < SynthVoice::routeCount; ++r)
            w.routes[r] = { static_cast<S>(1 + r), static_cast<D>(1 + (r * 2) % (SynthVoice::modDestinationCount - 1)), r % 2 ? 1.0f : -1.0f };
        wild.setPatch(w);
        SynthVoice::Ecology all;
        all.energy = all.speed = all.relation = all.tension = all.hunger = 1.0f;
        wild.setEcology(all);
        wild.noteOn(1.0f);
        bool bounded = true;
        for (int i = 0; i < 48000; ++i)
        {
            const auto y = wild.render(220.0f, 0.0f);
            bounded = bounded && std::isfinite(y) && std::abs(y) < 20.0f;
        }
        check(bounded, "every route at its extreme stays bounded");
    }

    // ---- in the engine: SYNTHESIS 0 is the previous sound, bit for bit;
    // at 1 the voices sound, follow the notes and stay finite ----
    {
        const auto render = [](bool touch, float mix, double seconds) {
            aquorbium::BiomaEngine e;
            e.prepare(48000.0);
            e.setSeed(0x815ab1eu);
            e.setEcosystemBlendImmediately(1.0f);
            e.setAutonomousEnabled(true);
            e.setEcologicalMusicPerformance({ 1.0f, 0.6f, 0.5f });
            if (touch)
                e.setSynthMix(mix);
            std::vector<float> out;
            std::vector<float> l(256), r(256);
            for (int b = 0; b < static_cast<int>(48000.0 * seconds / 256.0); ++b)
            {
                e.render(l.data(), r.data(), 256);
                out.insert(out.end(), l.begin(), l.end());
            }
            return out;
        };
        const auto untouched = render(false, 0.0f, 6.0), zero = render(true, 0.0f, 6.0);
        check(untouched == zero, "SYNTHESIS 0: the previous sound, bit for bit");
        const auto synth = render(true, 1.0f, 20.0);
        double power = 0.0;
        bool finite = true;
        for (const auto v : synth)
        {
            finite = finite && std::isfinite(v);
            power += static_cast<double>(v) * v;
        }
        check(finite && power > 0.0, "SYNTHESIS 1: the voices sound and stay finite");
        check(synth != untouched, "SYNTHESIS 1 changes the sound");
    }

    // regressions found by the PROPOSTA tests (2026-10-22)
    {
        using Src = SynthVoice::Source;
        const auto loudest = [](SynthVoice::Patch p, float& peak) {
            auto v = std::make_unique<SynthVoice>();
            v->prepare(48000.0, 77u);
            v->setPatch(p);
            v->noteOn(0.8f);
            double window = 0.0;
            float best = 0.0f;
            peak = 0.0f;
            for (int n = 0; n < 24000; ++n)
            {
                const auto y = v->render(220.0f, 0.0f);
                peak = std::max(peak, std::isfinite(y) ? std::abs(y) : 1.0e30f);
                window += static_cast<double>(y) * y;
                if (n % 480 == 479)
                {
                    best = std::max(best, static_cast<float>(std::sqrt(window / 480.0)));
                    window = 0.0;
                }
            }
            return best;
        };
        // a struck body is heard on its very first note (it was excited
        // before its kind was set, and stayed mute)
        for (const auto kind : { aquorbium::TimbreBody::Kind::matter, aquorbium::TimbreBody::Kind::membrane, aquorbium::TimbreBody::Kind::drop })
        {
            SynthVoice::Patch p;
            p.sourceA = Src::body;
            p.bodyA = static_cast<std::uint8_t>(kind);
            p.cutoff = 20000.0f;
            p.amp = { 0.001f, 0.3f, 0.0f, 0.2f, 0.5f };
            float peak = 0.0f;
            check(loudest(p, peak) > 0.01f, "a struck body sounds on its first note");
        }
        // FM B -> A on a resonant source stays bounded: it keeps its tuning.
        // The patch is the one that blew up to 1e37 (a membrane under an
        // additive's FM), as the codec saved it
        {
            const auto p = aquorbium::SynthPatchCodec::decode(
                "sourceA=10;sourceB=6;bodyA=7;shapeA=0.377217;shapeB=0.67337;widthA=0.67361;widthB=0.387611;"
                "fineB=-1.76156;colourA=0.025503;colourB=0.248843;levelA=1;levelB=0.497039;fm=0.26356;drift=0.276714;"
                "filterMode=0;cutoff=1709.58;resonance=0.666934;keytrack=0.620119;filterEnv=1.38158;"
                "ampA=0.028792;ampD=3.00477;ampS=0.167665;ampR=2.05238;ampC=0.614288;"
                "modA=0.376559;modD=0.857048;modS=0.127877;modR=1.0645;accent=0.414344;level=0.8;");
            float peak = 0.0f;
            (void) loudest(p, peak);
            check(peak < 4.0f, "FM into a resonant source (a membrane) stays bounded");
        }
    }

    // FORMANT (2026-10-22): the vowels' formants where the table puts them,
    // the tract moving them together, the matrix walking the vowel, a level
    // near the low-pass's, bounded at the extremes
    {
        using FM = SynthVoice::FilterMode;
        const auto vowelPatch = [](float vowel, float cutoff) {
            auto p = openOscillator(0.67f);  // a saw
            p.filterMode = FM::formant;
            p.vowel = vowel;
            p.cutoff = cutoff;               // the tract: 1000 Hz = the table's voice
            p.resonance = 0.5f;
            return p;
        };
        // the strongest harmonic of a 55 Hz saw inside a band
        const auto peakIn = [](const std::vector<float>& x, double lo, double hi) {
            double best = 0.0, at = 0.0;
            for (int h = 1; h * 55.0 <= hi; ++h)
                if (h * 55.0 >= lo)
                {
                    const auto a = partial(x, h * 55.0);
                    if (a > best) { best = a; at = h * 55.0; }
                }
            return at;
        };
        const auto a = play(vowelPatch(0.0f, 1000.0f), 55.0f, 2.0);
        const auto ii = play(vowelPatch(0.5f, 1000.0f), 55.0f, 2.0);
        const auto aF1 = peakIn(a, 200.0, 800.0), aF2 = peakIn(a, 850.0, 2000.0);
        // I's F2 is -30 dB in the table (weaker than F1's skirt): look for
        // its local resonance, a peak above its surroundings
        const auto iF1 = peakIn(ii, 200.0, 800.0), iF2 = peakIn(ii, 1500.0, 2000.0);
        const auto iF2Rise = partial(ii, iF2) / std::max(partial(ii, 1540.0), partial(ii, 1980.0));
        std::cout << "formant: A F1 " << aF1 << " F2 " << aF2 << " Hz; I F1 " << iF1 << " F2 " << iF2 << " Hz (x" << iF2Rise << " over its surroundings)\n";
        check(std::abs(aF1 - 600.0) <= 60.0 && std::abs(aF2 - 1040.0) <= 60.0, "FORMANT: A's F1 ~600 and F2 ~1040 Hz");
        check(std::abs(iF1 - 250.0) <= 60.0 && std::abs(iF2 - 1750.0) <= 60.0 && iF2Rise > 1.2,
              "FORMANT: I's F1 ~250 and a resonance at F2 ~1750 Hz");
        const auto tract = play(vowelPatch(0.0f, 2000.0f), 55.0f, 2.0);
        const auto tF1 = peakIn(tract, 800.0, 1600.0);
        check(std::abs(tF1 - 1200.0) <= 60.0, "FORMANT: CUTOFF 2000 (the tract x2) moves A's F1 to ~1200 Hz");
        // the matrix walks the vowel: the accent (1) x 0.5 turns A into I
        auto walked = vowelPatch(0.0f, 1000.0f);
        walked.routes[0] = { SynthVoice::ModSource::accent, SynthVoice::ModDestination::vowel, 0.5f };
        const auto w = play(walked, 55.0f, 2.0);
        check(std::abs(peakIn(w, 200.0, 800.0) - 250.0) <= 60.0, "FORMANT: a route to VOWEL walks A to I");
        // level: a 110 Hz saw through every vowel near the low-pass at 2 kHz
        auto lowPass = openOscillator(0.67f);
        lowPass.cutoff = 2000.0f;
        lowPass.resonance = 0.2f;
        const auto rmsOf = [](const std::vector<float>& x) {
            double sum = 0.0;
            for (const auto v : x) sum += static_cast<double>(v) * v;
            return std::sqrt(sum / static_cast<double>(x.size()));
        };
        const auto reference = rmsOf(play(lowPass, 110.0f));
        double quietest = 99.0, loudest = -99.0;
        for (const auto vowel : { 0.0f, 0.25f, 0.5f, 0.75f, 1.0f })
        {
            auto p = vowelPatch(vowel, 1000.0f);
            p.resonance = 0.2f;
            const auto db = 20.0 * std::log10(rmsOf(play(p, 110.0f)) / reference);
            quietest = std::min(quietest, db);
            loudest = std::max(loudest, db);
        }
        std::cout << "formant: level re the low-pass " << quietest << " .. " << loudest << " dB\n";
        check(quietest > -10.0 && loudest < 4.0, "FORMANT: a saw through the vowels within -10..+4 dB of the low-pass");
        // bounded at the extremes
        auto wild = vowelPatch(1.0f, 20000.0f);
        wild.resonance = 1.0f;
        wild.drive = 1.0f;
        float peak = 0.0f;
        bool finite = true;
        for (const auto y : play(wild, 2000.0f))
        {
            finite = finite && std::isfinite(y);
            peak = std::max(peak, std::abs(y));
        }
        check(finite && peak < 4.0f, "FORMANT: extremes stay finite and bounded");
        // the codec keeps it
        auto saved = vowelPatch(0.37f, 1500.0f);
        saved.routes[2] = { SynthVoice::ModSource::relation, SynthVoice::ModDestination::vowel, -0.4f };
        const auto back = aquorbium::SynthPatchCodec::decode(aquorbium::SynthPatchCodec::encode(saved));
        check(back.filterMode == FM::formant && std::abs(back.vowel - 0.37f) < 1.0e-4f
                  && back.routes[2].destination == SynthVoice::ModDestination::vowel,
              "FORMANT: the codec keeps the mode, the vowel and a route to it");
    }

    // FM 4-OP (2026-10-22): the algorithms route as the Modular's table says
    {
        const auto fm4 = [](int algorithm, float ratioB, float ratioC, float ratioD, float index, float feedback) {
            auto p = openOscillator(0.0f);
            p.sourceA = SynthVoice::Source::fm4;
            p.algorithmA = static_cast<std::uint8_t>(algorithm);
            p.shapeA = ratioB;      // B's ratio over the table
            p.ratioCA = ratioC;
            p.ratioDA = ratioD;
            p.widthA = index;
            p.colourA = feedback;
            return p;
        };
        // the table's macros: 1x = 1/9, 2x = 3/9, 3x = 5/9, 5x = 7/9
        const auto r1 = 1.0f / 9.0f, r2 = 3.0f / 9.0f, r3 = 5.0f / 9.0f, r5 = 7.0f / 9.0f;
        // 7 (additive): four sines at 1x, B, C, D, each a quarter
        const auto additive = play(fm4(7, r2, r3, r5, 0.8f, 0.0f), 110.0f);
        const auto p1 = partial(additive, 110.0), p2 = partial(additive, 220.0), p3 = partial(additive, 330.0),
                   p5 = partial(additive, 550.0), p4 = partial(additive, 440.0);
        check(std::abs(p2 / p1 - 1.0) < 0.05 && std::abs(p3 / p1 - 1.0) < 0.05 && std::abs(p5 / p1 - 1.0) < 0.05
                  && p4 < p1 * 0.01,
              "FM 4-OP algorithm 7: four equal sines at 1x, 2x, 3x, 5x and nothing between");
        // 0 (A>B>C>D, D the carrier), index 0: a pure sine at D's ratio
        const auto chain = play(fm4(0, r2, r3, r5, 0.0f, 0.0f), 110.0f);
        check(partial(chain, 550.0) > 100.0 * partial(chain, 110.0) && partial(chain, 550.0) > 100.0 * partial(chain, 1100.0),
              "FM 4-OP algorithm 0, index 0: only the carrier D sounds, at 5x");
        // with an index, the chain makes sidebands around D at multiples of C's 3x
        const auto modulated = play(fm4(0, r2, r3, r5, 0.5f, 0.0f), 110.0f);
        check(partial(modulated, 550.0 + 330.0) > 0.05 * partial(modulated, 550.0),
              "FM 4-OP algorithm 0 with an index: sidebands at D +- C");
        // 4 (A>D, B>D, C>D): three modulators into one carrier, at 1x
        const auto stack = play(fm4(4, r1, r1, r1, 0.0f, 0.0f), 220.0f);
        check(partial(stack, 220.0) > 100.0 * partial(stack, 440.0), "FM 4-OP algorithm 4, index 0: one sine");
        // every algorithm at the extremes: finite and inside the soft clip
        bool finite = true;
        float peak = 0.0f;
        for (int a = 0; a < 8; ++a)
            for (const auto y : play(fm4(a, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f), 1500.0f, 0.5))
            {
                finite = finite && std::isfinite(y);
                peak = std::max(peak, std::abs(y));
            }
        check(finite && peak <= 1.5f, "FM 4-OP: the eight algorithms at the extremes stay finite and <= 1.5");
        // the codec keeps the algorithm and the ratios
        auto saved = fm4(6, r2, r3, r5, 0.4f, 0.2f);
        saved.sourceB = SynthVoice::Source::fm4;
        saved.algorithmB = 3;
        saved.ratioDB = 1.0f;
        const auto back = aquorbium::SynthPatchCodec::decode(aquorbium::SynthPatchCodec::encode(saved));
        check(back.sourceA == SynthVoice::Source::fm4 && back.algorithmA == 6 && back.algorithmB == 3
                  && std::abs(back.ratioCA - r3) < 1.0e-4f && std::abs(back.ratioDB - 1.0f) < 1.0e-4f,
              "FM 4-OP: the codec keeps the algorithms and the ratios");
    }

    // the DRIVE's aliasing (2026-10-22): first-order antiderivative
    // anti-aliasing (Parker et al., DAFx 2016). The worst folded partial of
    // a sine, against its fundamental. Plain, a 7 kHz note at full drive
    // folded -18.6 dB; with ADAA -30.2; in the usual register far less
    {
        const auto worstAlias = [](float f0, float drive) {
            auto p = openOscillator(0.0f);  // a sine
            p.drive = drive;
            const auto x = play(p, f0, 1.25);
            const auto fundamental = partial(x, f0);
            double worst = 0.0;
            for (int h = 3; h <= 31; h += 2)
            {
                auto f = std::fmod(h * static_cast<double>(f0), rate);
                if (f > rate / 2.0) f = rate - f;
                const auto r = std::fmod(f, static_cast<double>(f0));
                if (f < 20.0 || r < 20.0 || f0 - r < 20.0)
                    continue;  // on a harmonic: not an alias we can tell apart
                worst = std::max(worst, partial(x, f));
            }
            return 20.0 * std::log10(std::max(worst, 1.0e-12) / fundamental);
        };
        const auto a1100 = worstAlias(1100.0f, 1.0f), a1700 = worstAlias(1700.0f, 1.0f);
        const auto a5000 = worstAlias(5000.0f, 0.5f), a7000 = worstAlias(7000.0f, 1.0f);
        std::cout << "drive aliasing (worst folded partial re the fundamental): 1.1 kHz full " << a1100
                  << " dB, 1.7 kHz full " << a1700 << ", 5 kHz half " << a5000 << ", 7 kHz full " << a7000 << '\n';
        check(a1100 < -70.0 && a1700 < -50.0, "DRIVE: the usual register stays clean (-70 / -50 dB)");
        check(a5000 < -50.0 && a7000 < -27.0, "DRIVE: high notes improved by ADAA (were -41 and -18.6 dB)");
    }

    // CHAOS and FEEDBACK (2026-10-23): the two families the Rasgo Modular's
    // CHAOS and a no-input loop brought to the synthesis device
    {
        const auto correlation = [](const std::vector<float>& y, std::size_t lag) {
            double ab = 0.0, aa = 0.0, bb = 0.0;
            for (std::size_t i = 0; i + lag < y.size(); ++i)
            {
                ab += static_cast<double>(y[i]) * y[i + lag];
                aa += static_cast<double>(y[i]) * y[i];
                bb += static_cast<double>(y[i + lag]) * y[i + lag];
            }
            return ab / std::sqrt(aa * bb + 1.0e-30);
        };
        const auto rmsOf = [](const std::vector<float>& y) {
            double sum = 0.0;
            for (const auto v : y) sum += static_cast<double>(v) * v;
            return std::sqrt(sum / static_cast<double>(std::max<std::size_t>(1, y.size())));
        };
        // CHAOS: one well (periodic), chaos, the large orbit (periodic), all
        // at one loudness (the level follower)
        const auto chaos = [](float forcing, float damping) {
            aquorbium::ChaosSource source;
            source.prepare(48000.0f);
            for (int i = 0; i < 48000; ++i) static_cast<void>(source.render(200.0f, forcing, damping, 0.0f));
            std::vector<float> y(48000);
            for (auto& v : y) v = source.render(200.0f, forcing, damping, 0.0f);
            return y;
        };
        const auto oneWell = chaos(0.0f, 0.4f), chaotic = chaos(0.4f, 0.4f), largeOrbit = chaos(1.0f, 0.4f);
        std::cout << "chaos: period correlation " << correlation(oneWell, 240) << " / " << correlation(chaotic, 240)
                  << " / " << correlation(largeOrbit, 240) << "; RMS " << rmsOf(oneWell) << " / " << rmsOf(chaotic)
                  << " / " << rmsOf(largeOrbit) << '\n';
        check(correlation(oneWell, 240) > 0.999 && correlation(largeOrbit, 240) > 0.999,
              "CHAOS: the low and the high FORCING are periodic at the note");
        check(correlation(chaotic, 240) < 0.5 && correlation(chaotic, 480) < 0.5,
              "CHAOS: the middle FORCING is chaotic (no period of 1 or 2 notes)");
        for (const auto& y : { oneWell, chaotic, largeOrbit })
            check(std::abs(20.0 * std::log10(rmsOf(y) / 0.3)) < 1.5, "CHAOS: one loudness in every regime (+-1.5 dB)");
        auto bounded = true;
        for (const auto f : { 30.0f, 440.0f, 3000.0f, 9000.0f })
            for (const auto forcing : { 0.0f, 0.5f, 1.0f })
                for (const auto damping : { 0.0f, 1.0f })
                {
                    aquorbium::ChaosSource source;
                    source.prepare(48000.0f);
                    for (int i = 0; i < 24000; ++i)
                    {
                        const auto y = source.render(f, forcing, damping, 1.0f);
                        bounded = bounded && std::isfinite(y) && std::abs(y) <= 1.5f;
                    }
                }
        check(bounded, "CHAOS: finite and bounded over the register and the macros");

        // FEEDBACK: excited, it sings by itself, in tune, bounded
        const auto feedback = [](float frequency, float gain, float detune) {
            aquorbium::FeedbackSource source;
            source.prepare(48000.0f, 7u);
            source.excite(1.0f, frequency);
            std::vector<float> y(96000);
            for (auto& v : y) v = source.render(frequency, gain, detune, 0.3f);
            return std::vector<float>(y.begin() + 48000, y.end());
        };
        // the period by autocorrelation, refined by a parabola
        const auto periodOf = [&correlation](const std::vector<float>& y, std::size_t from, std::size_t to) {
            std::size_t best = from;
            double top = -2.0;
            for (auto lag = from; lag <= to; ++lag)
                if (const auto c = correlation(y, lag); c > top) { top = c; best = lag; }
            const auto a = correlation(y, best - 1), b = top, c = correlation(y, best + 1);
            const auto denominator = a - 2.0 * b + c;
            return static_cast<double>(best) + (std::abs(denominator) > 1.0e-12 ? 0.5 * (a - c) / denominator : 0.0);
        };
        for (const auto f : { 110.0f, 220.0f, 440.0f })
        {
            const auto y = feedback(f, 0.0f, 0.0f);
            const auto expected = 48000.0 / f;
            const auto measured = periodOf(y, static_cast<std::size_t>(expected * 0.8), static_cast<std::size_t>(expected * 1.2));
            const auto cents = 1200.0 * std::log2(measured / expected);
            std::cout << "feedback " << f << " Hz at the lowest GAIN: RMS " << rmsOf(y) << ", " << cents << " cents\n";
            check(rmsOf(y) > 0.05, "FEEDBACK: at the lowest GAIN it still sings by itself");
            check(std::abs(cents) < 8.0, "FEEDBACK: in tune within 8 cents");
        }
        auto singing = true, limited = true;
        for (const auto gain : { 0.0f, 0.5f, 1.0f })
            for (const auto detune : { 0.0f, 0.5f, 1.0f })
            {
                const auto y = feedback(200.0f, gain, detune);
                singing = singing && rmsOf(y) > 0.05;
                for (const auto v : y)
                    limited = limited && std::isfinite(v) && std::abs(v) <= 0.5f;
            }
        check(singing, "FEEDBACK: every GAIN and DETUNE sings");
        check(limited, "FEEDBACK: finite and within 0.5");

        // through the whole voice, and kept by the codec
        for (const auto source : { SynthVoice::Source::chaos, SynthVoice::Source::feedback })
        {
            auto p = openOscillator(0.0f);
            p.sourceA = source;
            p.shapeA = 0.5f;
            p.widthA = 0.5f;
            p.colourA = 0.3f;
            const auto y = play(p, 220.0f);
            auto ok = true;
            double sum = 0.0;
            for (const auto v : y) { ok = ok && std::isfinite(v); sum += static_cast<double>(v) * v; }
            check(ok && sum / static_cast<double>(y.size()) > 1.0e-5, "CHAOS/FEEDBACK: audible and finite in the voice");
            const auto back = aquorbium::SynthPatchCodec::decode(aquorbium::SynthPatchCodec::encode(p));
            check(back.sourceA == source, "CHAOS/FEEDBACK: the codec keeps the source");
        }
    }

    if (failures == 0)
        std::cout << "synth voice: all checks passed\n";
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
