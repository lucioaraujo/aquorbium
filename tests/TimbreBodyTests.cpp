// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

// The organism bodies (2026-10-20): every body sounds, at a comparable level,
// stays finite and bounded for any MATTER and pitch, the native kind is
// silent, a change of body never clicks, MATTER changes the sound, and the
// same seed gives the same samples.
#include "core/BiomaEngine.h"
#include "core/TimbreBody.h"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <vector>

namespace
{
using Body = aquorbium::TimbreBody;
using Kind = Body::Kind;

int failures = 0;
void check(bool condition, const char* what)
{
    if (!condition)
    {
        std::cerr << "FAILED: " << what << '\n';
        ++failures;
    }
}

std::vector<float> render(Kind kind, float frequency, float matter, double seconds, std::uint32_t seed = 7u)
{
    constexpr double rate = 48000.0;
    Body body;
    body.setKind(kind);
    body.prepare(rate, seed);
    std::vector<float> out;
    const auto samples = static_cast<std::size_t>(rate * seconds);
    out.reserve(samples);
    for (std::size_t n = 0; n < samples; ++n)
    {
        if (n % 240 == 0)
            body.control(frequency, matter);
        if (n % 12000 == 0)
            body.excite(0.8f);
        out.push_back(body.render(frequency, 0.8f));
    }
    return out;
}

// a sustained note: the second half of `seconds` at constant drive
std::vector<float> steady(Kind kind, float frequency, float matter, float drive, double seconds = 1.0)
{
    constexpr double rate = 48000.0;
    Body body;
    body.setKind(kind);
    body.prepare(rate, 7u);
    std::vector<float> out;
    const auto samples = static_cast<std::size_t>(rate * seconds);
    for (std::size_t n = 0; n < samples; ++n)
    {
        if (n % 240 == 0)
            body.control(frequency, matter);
        const auto y = body.render(frequency, drive);
        if (n >= samples / 2)
            out.push_back(y);
    }
    return out;
}

// the level of a partial (Hann-windowed DFT), relative to the fundamental's
double partialDb(const std::vector<float>& x, double frequency, int harmonic)
{
    const auto at = [&](double hz) {
        double re = 0.0, im = 0.0;
        for (std::size_t i = 0; i < x.size(); ++i)
        {
            const auto w = 0.5 - 0.5 * std::cos(2.0 * 3.14159265358979 * static_cast<double>(i) / static_cast<double>(x.size()));
            re += x[i] * w * std::cos(2.0 * 3.14159265358979 * hz * static_cast<double>(i) / 48000.0);
            im -= x[i] * w * std::sin(2.0 * 3.14159265358979 * hz * static_cast<double>(i) / 48000.0);
        }
        return std::sqrt(re * re + im * im);
    };
    return 20.0 * std::log10(at(frequency * harmonic) / std::max(1.0e-12, at(frequency)));
}

// the pitch by autocorrelation around the expected period, in cents
double pitchCents(const std::vector<float>& x, double frequency)
{
    const auto period = 48000.0 / frequency;
    const auto lo = static_cast<int>(period * 0.8), hi = static_cast<int>(period * 1.2);
    std::vector<double> ac(static_cast<std::size_t>(hi + 2), 0.0);
    for (int lag = lo - 1; lag <= hi + 1; ++lag)
        for (std::size_t i = 0; i + static_cast<std::size_t>(lag) < x.size() && i < 20000; ++i)
            ac[static_cast<std::size_t>(lag)] += static_cast<double>(x[i]) * x[i + static_cast<std::size_t>(lag)];
    auto best = lo;
    for (int lag = lo; lag <= hi; ++lag)
        if (ac[static_cast<std::size_t>(lag)] > ac[static_cast<std::size_t>(best)])
            best = lag;
    const auto a = ac[static_cast<std::size_t>(best - 1)], b = ac[static_cast<std::size_t>(best)], c = ac[static_cast<std::size_t>(best + 1)];
    const auto measured = static_cast<double>(best) + 0.5 * (a - c) / (a - 2.0 * b + c);
    return 1200.0 * std::log2(period / measured);
}

double rms(const std::vector<float>& x, std::size_t from = 0)
{
    double sum = 0.0;
    for (std::size_t i = from; i < x.size(); ++i)
        sum += static_cast<double>(x[i]) * x[i];
    return std::sqrt(sum / static_cast<double>(std::max<std::size_t>(1, x.size() - from)));
}
}

int main()
{
    const bool report = std::getenv("TIMBRE_REPORT") != nullptr;
    check(rms(render(Kind::native, 220.0f, 0.5f, 0.5)) == 0.0, "the native kind adds nothing");

    for (std::size_t k = 1; k < Body::kindCount; ++k)
    {
        const auto kind = static_cast<Kind>(k);
        for (const auto matter : { 0.0f, 0.5f, 1.0f })
            for (const auto frequency : { 55.0f, 220.0f, 1200.0f })
            {
                const auto x = render(kind, frequency, matter, 2.0);
                bool finite = true;
                float peak = 0.0f;
                for (const auto s : x)
                {
                    finite = finite && std::isfinite(s);
                    peak = std::max(peak, std::abs(s));
                }
                const auto level = rms(x, 4800);
                if (report)
                    std::cout << Body::name(kind) << " matter " << matter << " f " << frequency
                              << " rms " << level << " peak " << peak << '\n';
                check(finite, "every body stays finite");
                check(peak < 3.0f, "every body stays bounded");
                check(level > 0.02 && level < 0.9, "every body sounds at a comparable level");
            }
        // MATTER is audible: the extremes differ
        const auto a = render(kind, 220.0f, 0.0f, 1.0);
        const auto b = render(kind, 220.0f, 1.0f, 1.0);
        double difference = 0.0;
        for (std::size_t i = 0; i < a.size(); ++i)
            difference += std::abs(a[i] - b[i]);
        check(difference / static_cast<double>(a.size()) > 0.005, "MATTER changes the sound");
        // determinism
        check(render(kind, 220.0f, 0.5f, 0.3, 9u) == render(kind, 220.0f, 0.5f, 0.3, 9u),
              "the same seed gives the same samples");
    }

    // a change of body fades out and in: no jump larger than the bodies' own
    {
        Body body;
        body.setKind(Kind::fold);
        body.prepare(48000.0, 3u);
        body.control(110.0f, 0.2f);
        float previous = 0.0f, maxJump = 0.0f, steady = 0.0f;
        for (int n = 0; n < 48000; ++n)
        {
            if (n == 24000)
                body.setKind(Kind::voice);
            if (n % 240 == 0)
                body.control(110.0f, 0.2f);
            const auto s = body.render(110.0f, 1.0f);
            const auto jump = std::abs(s - previous);
            // the steady jumps of both bodies, before and well after the change
            if (n < 23000 || n > 30000)
                steady = std::max(steady, jump);
            else if (n > 23990 && n < 26000)
                maxJump = std::max(maxJump, jump);
            previous = s;
        }
        check(body.kind() == Kind::voice, "the body changed");
        check(maxJump <= steady * 1.5f + 1.0e-3f, "changing body does not click");
    }

    // ---- in the engine ----
    {
        using Engine = aquorbium::BiomaEngine;
        struct Result { std::vector<float> left; std::array<float, 5> minMatter {}, maxMatter {}; };
        const auto play = [](bool touch, const Engine::TimbrePerformance& timbre, float music, int seconds = 8) {
            Engine engine;
            engine.prepare(48000.0);
            engine.setSeed(0x815ab1eu);
            engine.setEcosystemBlendImmediately(1.0f);
            engine.setAutonomousEnabled(true);
            engine.setEcologicalMusicPerformance({ music, 0.6f, 0.5f, 0.5f });
            if (touch)
                engine.setTimbrePerformance(timbre);
            Result result;
            result.minMatter.fill(1.0f);
            std::array<float, 256> left {}, right {};
            for (int block = 0; block < 48000 * seconds / 256; ++block)
            {
                engine.render(left.data(), right.data(), left.size());
                result.left.insert(result.left.end(), left.begin(), left.end());
                const auto t = engine.timbreTelemetry();
                for (std::size_t i = 0; i < 5; ++i)
                {
                    result.minMatter[i] = std::min(result.minMatter[i], t.matter[i]);
                    result.maxMatter[i] = std::max(result.maxMatter[i], t.matter[i]);
                }
            }
            return result;
        };
        const Engine::TimbrePerformance neutral {};
        check(play(false, neutral, 0.0f).left == play(true, neutral, 0.0f).left,
              "native bodies, mix and ecology at zero: the engine is untouched");

        Engine::TimbrePerformance bodies;
        bodies.body = { Kind::voice, Kind::drop, Kind::matter, Kind::breath, Kind::string };
        bodies.mix = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
        const auto on = play(true, bodies, 1.0f);
        bool finite = true;
        float peak = 0.0f;
        for (const auto s : on.left)
        {
            finite = finite && std::isfinite(s);
            peak = std::max(peak, std::abs(s));
        }
        check(finite && peak > 1.0e-3f && peak < 1.0f, "the bodies sound in the engine, finite and below full scale");
        check(on.left != play(false, neutral, 1.0f).left, "the bodies change the sound");

        bodies.ecology = 1.0f;
        // ECOLOGY is a tendency: it needs the ecosystem to live a little
        const auto living = play(true, bodies, 1.0f, 30);
        float movement = 0.0f;
        for (std::size_t i = 0; i < 5; ++i)
            movement = std::max(movement, living.maxMatter[i] - living.minMatter[i]);
        check(movement > 0.05f, "with ECOLOGY the matter moves with the ecosystem");
    }

    if (failures == 0)
        std::cout << "timbre bodies: all checks passed\n";
    // ---- the bowed string and the reed (2026-10-21): physics, measured ----
    {
        bool bowTuned = true, helmholtz = true, reedTuned = true, odd = true;
        for (const auto f : { 110.0f, 220.0f, 440.0f })
            for (const auto m : { 0.0f, 0.5f, 1.0f })
            {
                const auto bow = steady(Kind::bow, f, m, 1.0f);
                bowTuned = bowTuned && std::abs(pitchCents(bow, f)) < 2.0;
                // Helmholtz motion: a sawtooth, the 2nd partial ~6 dB under the 1st
                helmholtz = helmholtz && std::abs(partialDb(bow, f, 2) - (-6.0)) < 2.5;
                const auto reed = steady(Kind::reed, f, m, 1.0f);
                reedTuned = reedTuned && std::abs(pitchCents(reed, f)) < 1.0;
                // a tube closed at one end: odd partials, the even ones far below
                odd = odd && partialDb(reed, f, 2) < partialDb(reed, f, 3) - 30.0;
            }
        check(bowTuned, "the bowed string is in tune (2 cents), at every pitch and bow position");
        check(helmholtz, "the bow gives Helmholtz motion (a sawtooth) everywhere, never the octave");
        check(reedTuned, "the reed is in tune (1 cent)");
        check(odd, "the reed's tube sounds odd harmonics");
        check(rms(steady(Kind::bow, 220.0f, 0.5f, 0.0f)) < 1.0e-4 && rms(steady(Kind::reed, 220.0f, 0.5f, 0.0f)) < 1.0e-4,
              "without energy the bow stops and the reed is not blown");
    }

    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
