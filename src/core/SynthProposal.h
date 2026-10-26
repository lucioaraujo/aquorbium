// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include "core/SynthVoice.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>

namespace aquorbium
{
// PROPOSTA - the synthesis device's starting patches (SYNTH_DEVICE_DESIGN.md,
// phase 5; 2026-10-22). As in the Rasgo Modular's identity, the instrument
// does not play in the musician's place: it proposes a point of departure,
// and the musician edits from there.
//
// A proposal is a function of (organism, seed): the same seed gives the same
// patch, a new seed another one (across compilers the audition's floating
// point may move a level by a hair). What comes from where:
//   the SPECIES   which source families, the envelope's gesture (sustained,
//                 struck, plucked, resonant), the filter's register;
//   the STRATEGY  which ecological signals reach the matrix (the predator
//                 hears its hunger, the prey its fear, the migrant its
//                 distance, the sessile the habitat, the territorial the
//                 tension);
//   the SEED      every choice inside those families and ranges.
// The ranges stay inside the codec's (SynthPatchCodec), so a proposal is
// always a valid, savable patch. Nothing here is a fixed "preset": the
// species gives an influence, the seed the departure (the RASGO principle of
// influences, not a taxonomy).
//
// The organisms are fixed by index (Ecosystem.cpp):
//   0 MORPH  oscillator,       predator / territorial
//   1 GRAIN  granular striker, territorial
//   2 MODAL  resonator,        migratory
//   3 PULSAR pulsar,           prey
//   4 STRING physical string,  sessile / territorial
class SynthProposal final
{
public:
    using Patch = SynthVoice::Patch;
    using Source = SynthVoice::Source;
    using Mod = SynthVoice::ModSource;
    using Dest = SynthVoice::ModDestination;
    static constexpr std::size_t organisms = 5;

    // the loudness every proposal is calibrated to: the loudest 10 ms of a
    // test note, RMS (about -14 dBFS before the engine's synthesis level)
    static constexpr float targetLoudness = 0.2f;

    // not for the audio thread: it renders test notes (a few ms of CPU on a
    // click) and allocates one voice
    [[nodiscard]] static Patch propose(std::size_t organism, std::uint32_t seed)
    {
        // the instrument listens to what it proposes: a candidate that comes
        // out (nearly) mute - a band-pass far from its source, a quiet corner
        // of a body - gives way to the next one drawn from the same seed, and
        // the one kept is levelled to the others. A measured criterion, not
        // a taste: the musician still decides what the sound is.
        auto voice = std::make_unique<SynthVoice>();
        Patch fallback;
        float fallbackLoudness = -1.0f;
        for (std::uint32_t attempt = 0; attempt < 8; ++attempt)
        {
            auto p = candidate(organism, seed, attempt);
            const auto heard = audition(*voice, p, seed);
            if (heard.loudness > fallbackLoudness)
            {
                fallback = p;
                fallbackLoudness = heard.loudness;
            }
            // loud enough that the level (at most 1.5) can bring it within
            // 6 dB of the target: 0.2 x (0.8 / 1.5) / 2 = 0.053
            if (heard.loudness >= 0.055f)
                return level(p, heard);
        }
        return level(fallback, audition(*voice, fallback, seed));
    }

    // the patch before the audition (for the tests of the species' gesture)
    [[nodiscard]] static Patch candidate(std::size_t organism, std::uint32_t seed, std::uint32_t attempt = 0) noexcept
    {
        Random r { (static_cast<std::uint64_t>(seed) << 8) ^ (0x9e3779b97f4a7c15ull * (organism + 1))
                   ^ (0xd1b54a32d192ed03ull * attempt) };
        Patch p;
        switch (std::min(organism, organisms - 1))
        {
        case 0: morph(p, r); break;
        case 1: grain(p, r); break;
        case 2: modal(p, r); break;
        case 3: pulsar(p, r); break;
        default: string(p, r); break;
        }
        common(p, r);
        routes(p, r, organism);
        return p;
    }

    struct Heard
    {
        float loudness = 0.0f;  // the loudest 10 ms, RMS
        float peak = 0.0f;
        bool finite = true;
    };
    // a note held 0.6 s at 220 Hz with the accent 0.8, a quiet sine as the
    // NATIVE body, at the patch's own level; the ecology at rest
    [[nodiscard]] static Heard audition(SynthVoice& voice, const Patch& p, std::uint32_t seed) noexcept
    {
        constexpr double rate = 48000.0;
        constexpr int length = 28800, window = 480;
        voice.prepare(rate, seed);
        voice.setPatch(p);
        voice.noteOn(0.8f);
        Heard heard;
        double sum = 0.0;
        float phase = 0.0f;
        for (int n = 0; n < length; ++n)
        {
            phase += 220.0f / static_cast<float>(rate);
            phase -= static_cast<float>(static_cast<int>(phase));
            const auto y = voice.render(220.0f, 0.2f * std::sin(6.2831853f * phase));
            if (!std::isfinite(y))
            {
                heard.finite = false;
                heard.loudness = 0.0f;
                return heard;
            }
            heard.peak = std::max(heard.peak, std::abs(y));
            sum += static_cast<double>(y) * y;
            if (n % window == window - 1)
            {
                heard.loudness = std::max(heard.loudness, static_cast<float>(std::sqrt(sum / window)));
                sum = 0.0;
            }
        }
        return heard;
    }

private:
    // the output level that brings the audition to the target loudness,
    // inside the level's range and without a peak above ~1.5
    [[nodiscard]] static Patch level(Patch p, const Heard& heard) noexcept
    {
        if (heard.loudness <= 1.0e-6f || !heard.finite)
            return p;
        auto gain = targetLoudness / heard.loudness;
        if (heard.peak > 0.0f)
            gain = std::min(gain, 1.5f / heard.peak);
        p.level = std::clamp(p.level * gain, 0.1f, 1.5f);
        return p;
    }

    // SplitMix64 (Steele, Lea & Flood 2014): small, fast, reproducible
    struct Random
    {
        std::uint64_t state;
        std::uint64_t next() noexcept
        {
            auto z = (state += 0x9e3779b97f4a7c15ull);
            z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ull;
            z = (z ^ (z >> 27)) * 0x94d049bb133111ebull;
            return z ^ (z >> 31);
        }
        float unit() noexcept { return static_cast<float>(next() >> 40) / 16777216.0f; }
        float range(float lo, float hi) noexcept { return lo + (hi - lo) * unit(); }
        int below(int n) noexcept { return static_cast<int>(next() % static_cast<std::uint64_t>(n)); }
        template <typename T, std::size_t N>
        T pick(const std::array<T, N>& items) noexcept { return items[static_cast<std::size_t>(below(static_cast<int>(N)))]; }
        bool chance(float probability) noexcept { return unit() < probability; }
    };

    // the native body: the organism's own sound is always among the choices
    // for B, so a proposal can keep a thread to what the species was
    static void secondSource(Patch& p, Random& r, const std::array<Source, 4>& family)
    {
        if (r.chance(0.3f))
        {
            p.sourceB = Source::off;
            p.levelB = 0.0f;
            return;
        }
        p.sourceB = r.chance(0.3f) ? Source::native : r.pick(family);
        p.levelB = r.range(0.25f, 0.7f);
        p.tuneB = r.pick(std::array<float, 6> { 0.0f, 0.0f, 12.0f, -12.0f, 7.0f, 19.0f });
        p.fineB = r.range(-9.0f, 9.0f);
        p.shapeB = r.unit();
        p.widthB = r.unit();
        p.colourB = r.unit();
    }

    static void body(Patch& p, Random& r, bool a, const std::array<TimbreBody::Kind, 3>& bodies)
    {
        const auto kind = static_cast<std::uint8_t>(r.pick(bodies));
        (a ? p.bodyA : p.bodyB) = kind;
    }

    // MORPH: the foreground melodic voice - sustained, a filter that opens
    static void morph(Patch& p, Random& r)
    {
        p.sourceA = r.pick(std::array<Source, 4> { Source::oscillator, Source::wavetable, Source::fm, Source::additive });
        p.shapeA = r.range(0.2f, 0.9f);
        p.widthA = p.sourceA == Source::fm ? r.range(0.1f, 0.5f) : r.range(0.2f, 0.8f);
        p.colourA = r.range(0.0f, 0.5f);
        p.subA = p.sourceA == Source::oscillator ? r.range(0.0f, 0.4f) : 0.0f;
        secondSource(p, r, { Source::oscillator, Source::wavetable, Source::fm, Source::noise });
        p.filterMode = SynthVoice::FilterMode::lowPass;
        p.cutoff = r.range(500.0f, 3000.0f);
        p.resonance = r.range(0.1f, 0.5f);
        p.keytrack = r.range(0.4f, 0.9f);
        p.filterEnv = r.range(0.5f, 2.5f);
        p.amp = { r.range(0.005f, 0.12f), r.range(0.2f, 0.8f), r.range(0.55f, 0.9f), r.range(0.2f, 0.9f), r.range(0.3f, 0.7f) };
        p.mod = { r.range(0.005f, 0.2f), r.range(0.2f, 1.2f), r.range(0.1f, 0.5f), r.range(0.2f, 0.8f), 0.5f };
        p.vibrato = r.range(0.0f, 14.0f);
        p.lfo1 = { r.range(4.0f, 6.5f), SynthVoice::LfoShape::sine };
        // sometimes a singer: the FORMANT mode, a vowel, a tract that
        // barely follows the note (2026-10-22)
        if (r.chance(0.25f))
        {
            p.filterMode = SynthVoice::FilterMode::formant;
            p.vowel = r.unit();
            p.cutoff = r.range(700.0f, 1600.0f);
            p.keytrack = r.range(0.0f, 0.3f);
            p.filterEnv = r.range(0.0f, 0.6f);
            p.resonance = r.range(0.2f, 0.6f);
        }
    }

    // GRAIN: struck matter - no sustain, bright, short
    static void grain(Patch& p, Random& r)
    {
        p.sourceA = r.pick(std::array<Source, 4> { Source::matter, Source::body, Source::noise, Source::pulsar });
        body(p, r, true, { TimbreBody::Kind::drop, TimbreBody::Kind::glass, TimbreBody::Kind::matter });
        p.shapeA = r.range(0.0f, 0.8f);
        p.widthA = r.range(0.4f, 1.0f);
        p.colourA = r.range(0.2f, 0.8f);
        secondSource(p, r, { Source::noise, Source::matter, Source::fm, Source::oscillator });
        p.filterMode = r.pick(std::array<SynthVoice::FilterMode, 3> {
            SynthVoice::FilterMode::bandPass, SynthVoice::FilterMode::highPass, SynthVoice::FilterMode::lowPass });
        // the register by mode: a high-pass or band-pass far above the note
        // leaves a struck body almost mute (measured by the tests)
        p.cutoff = p.filterMode == SynthVoice::FilterMode::highPass ? r.range(150.0f, 700.0f)
                 : p.filterMode == SynthVoice::FilterMode::bandPass ? r.range(500.0f, 2500.0f)
                                                                    : r.range(1500.0f, 6000.0f);
        p.resonance = r.range(0.2f, 0.6f);
        p.keytrack = r.range(0.3f, 0.8f);
        p.filterEnv = r.range(0.5f, 3.0f);
        p.amp = { r.range(0.001f, 0.004f), r.range(0.06f, 0.35f), 0.0f, r.range(0.05f, 0.3f), r.range(0.0f, 0.35f) };
        p.mod = { 0.001f, r.range(0.02f, 0.15f), 0.0f, 0.1f, 0.3f };
    }

    // MODAL: a resonant body - long decay, inharmonic colours welcome
    static void modal(Patch& p, Random& r)
    {
        p.sourceA = r.pick(std::array<Source, 4> { Source::matter, Source::body, Source::additive, Source::fm4 });
        body(p, r, true, { TimbreBody::Kind::glass, TimbreBody::Kind::membrane, TimbreBody::Kind::choir });
        // FM 4-OP as a bell: parallel carriers (algorithms 5..8) and free ratios
        p.algorithmA = static_cast<std::uint8_t>(4 + r.below(4));
        p.ratioCA = r.unit();
        p.ratioDA = r.unit();
        p.shapeA = r.range(0.2f, 1.0f);          // matter structure / additive tilt / fm ratio
        p.widthA = r.range(0.2f, 0.8f);
        p.colourA = r.range(0.0f, 0.6f);         // matter damping / additive stretch / fm feedback
        if (p.sourceA == Source::additive)
            p.colourA = r.range(0.5f, 0.8f);     // a little stretch: a bell rather than a pipe
        secondSource(p, r, { Source::additive, Source::matter, Source::wavetable, Source::noise });
        p.filterMode = r.chance(0.7f) ? SynthVoice::FilterMode::lowPass : SynthVoice::FilterMode::bandPass;
        p.cutoff = r.range(800.0f, 4000.0f);
        p.resonance = r.range(0.2f, 0.7f);
        p.keytrack = r.range(0.5f, 1.0f);
        p.filterEnv = r.range(-1.0f, 1.5f);
        p.amp = { r.range(0.002f, 0.03f), r.range(1.0f, 3.5f), r.range(0.0f, 0.3f), r.range(0.8f, 3.0f), r.range(0.5f, 0.9f) };
        p.mod = { r.range(0.01f, 0.4f), r.range(0.5f, 2.5f), r.range(0.0f, 0.4f), r.range(0.5f, 2.0f), 0.5f };
        p.drift = r.range(0.0f, 0.3f);
    }

    // PULSAR: trains of grains - a formant that moves, a nervous LFO
    static void pulsar(Patch& p, Random& r)
    {
        p.sourceA = r.pick(std::array<Source, 4> { Source::pulsar, Source::pulsar, Source::fm, Source::wavetable });
        p.shapeA = r.range(0.25f, 0.8f);         // pulsar formant / fm ratio / wavetable position
        p.widthA = r.range(0.2f, 0.8f);
        p.colourA = r.range(0.2f, 0.8f);
        secondSource(p, r, { Source::pulsar, Source::noise, Source::oscillator, Source::fm });
        p.filterMode = r.chance(0.6f) ? SynthVoice::FilterMode::bandPass : SynthVoice::FilterMode::lowPass;
        p.cutoff = r.range(900.0f, 5000.0f);
        p.resonance = r.range(0.2f, 0.65f);
        p.keytrack = r.range(0.3f, 0.8f);
        p.filterEnv = r.range(0.0f, 2.0f);
        p.amp = { r.range(0.003f, 0.05f), r.range(0.15f, 0.6f), r.range(0.3f, 0.7f), r.range(0.1f, 0.5f), r.range(0.2f, 0.6f) };
        p.mod = { 0.003f, r.range(0.1f, 0.5f), r.range(0.0f, 0.3f), 0.2f, 0.5f };
        p.lfo2 = { r.range(1.5f, 7.0f), r.pick(std::array<SynthVoice::LfoShape, 3> {
            SynthVoice::LfoShape::wander, SynthVoice::LfoShape::triangle, SynthVoice::LfoShape::square }) };
        p.wah = r.range(0.2f, 1.2f);
    }

    // STRING: plucked or bowed - the string sources and bodies
    static void string(Patch& p, Random& r)
    {
        p.sourceA = r.pick(std::array<Source, 4> { Source::string, Source::string, Source::body, Source::oscillator });
        body(p, r, true, { TimbreBody::Kind::string, TimbreBody::Kind::bow, TimbreBody::Kind::reed });
        p.shapeA = r.range(0.1f, 0.6f);          // string damping
        p.widthA = r.range(0.05f, 0.4f);         // pluck position
        p.colourA = r.range(0.5f, 0.95f);        // sustain
        secondSource(p, r, { Source::string, Source::oscillator, Source::noise, Source::matter });
        p.filterMode = SynthVoice::FilterMode::lowPass;
        p.cutoff = r.range(1500.0f, 7000.0f);
        p.resonance = r.range(0.05f, 0.35f);
        p.keytrack = r.range(0.7f, 1.0f);
        p.filterEnv = r.range(0.3f, 2.0f);
        const auto bowed = p.sourceA == Source::oscillator || (p.sourceA == Source::body && p.bodyA != static_cast<std::uint8_t>(TimbreBody::Kind::string));
        p.amp = bowed ? SynthVoice::Envelope { r.range(0.04f, 0.2f), r.range(0.3f, 0.8f), r.range(0.6f, 0.9f), r.range(0.2f, 0.6f), 0.5f }
                      : SynthVoice::Envelope { 0.001f, r.range(0.6f, 2.0f), r.range(0.0f, 0.2f), r.range(0.3f, 1.2f), r.range(0.4f, 0.8f) };
        p.mod = { 0.002f, r.range(0.1f, 0.6f), r.range(0.0f, 0.3f), 0.3f, 0.5f };
        p.vibrato = bowed ? r.range(4.0f, 16.0f) : 0.0f;
        p.lfo1 = { r.range(4.5f, 6.0f), SynthVoice::LfoShape::sine };
    }

    static void common(Patch& p, Random& r)
    {
        p.levelA = 1.0f;
        p.ring = r.chance(0.15f) && p.sourceB != Source::off ? r.range(0.1f, 0.5f) : 0.0f;
        p.fm = r.chance(0.15f) && p.sourceB != Source::off ? r.range(0.05f, 0.3f) : 0.0f;
        p.drift = std::max(p.drift, r.range(0.0f, 0.15f));
        p.drive = r.chance(0.3f) ? r.range(0.05f, 0.4f) : 0.0f;
        p.accentToLevel = r.range(0.4f, 0.8f);
        p.level = 0.8f;
    }

    // the ecology: what each strategy listens to, then a few general links
    static void routes(Patch& p, Random& r, std::size_t organism)
    {
        struct Link
        {
            Mod source;
            Dest destination;
            float lo, hi;
        };
        // by strategy (Ecosystem.cpp): the first link is always made
        static const std::array<std::array<Link, 2>, organisms> strategy { {
            { { { Mod::hunger, Dest::cutoff, 0.15f, 0.4f }, { Mod::tension, Dest::drive, 0.2f, 0.5f } } },     // predator / territorial
            { { { Mod::tension, Dest::resonance, 0.2f, 0.5f }, { Mod::accent, Dest::cutoff, 0.15f, 0.35f } } }, // territorial
            { { { Mod::distance, Dest::cutoff, -0.4f, -0.15f }, { Mod::speed, Dest::lfo2Rate, 0.2f, 0.5f } } }, // migratory
            { { { Mod::prey, Dest::lfo2Rate, 0.25f, 0.6f }, { Mod::prey, Dest::pitch, 0.02f, 0.08f } } },       // prey: its swarm (the prey population)
            { { { Mod::habitat, Dest::cutoff, 0.15f, 0.35f }, { Mod::tension, Dest::shapeA, 0.1f, 0.3f } } },   // sessile
        } };
        static const std::array<Link, 8> general { {
            { Mod::energy, Dest::cutoff, 0.1f, 0.35f },
            { Mod::modEnvelope, Dest::shapeA, 0.1f, 0.4f },
            { Mod::lfo2, Dest::widthA, 0.05f, 0.25f },
            { Mod::relation, Dest::ring, 0.1f, 0.4f },
            { Mod::channel, Dest::levelB, 0.2f, 0.5f },
            { Mod::temperature, Dest::colourA, 0.1f, 0.3f },
            { Mod::age, Dest::cutoff, -0.3f, -0.1f },
            { Mod::room, Dest::level, -0.25f, -0.1f },
        } };
        const auto& own = strategy[std::min(organism, organisms - 1)];
        std::size_t slot = 0;
        const auto put = [&p, &r, &slot](const Link& link) {
            p.routes[slot++] = { link.source, link.destination, r.range(link.lo, link.hi) };
        };
        put(own[0]);
        if (r.chance(0.6f))
            put(own[1]);
        // one or two general links, never the same one twice
        const auto count = 1 + r.below(2);
        const auto first = r.below(static_cast<int>(general.size()));
        for (int i = 0; i < count; ++i)
            put(general[static_cast<std::size_t>((first + i * 3) % static_cast<int>(general.size()))]);
        // a singer's vowel follows its relation (a hunt, a courtship opens it)
        if (p.filterMode == SynthVoice::FilterMode::formant)
            put({ Mod::relation, Dest::vowel, 0.15f, 0.45f });
    }
};
}
