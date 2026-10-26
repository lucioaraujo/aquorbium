// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

// The session's world (2026-10-24): the author heard the same melody at every
// opening. Each seed now draws another piece - and it stays in its key.
#include "core/BiomaEngine.h"
#include "core/SessionGenesis.h"

#include <array>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <set>
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

// the pitches the organisms are sent to, in the first 60 s
std::vector<int> notes(const aquorbium::SessionWorld* world, std::uint32_t seed)
{
    auto engine = std::make_unique<aquorbium::BiomaEngine>();
    if (world != nullptr)
        engine->setSessionWorld(*world);
    engine->prepare(48000.0);
    engine->setSeed(seed);
    engine->setAutonomousEnabled(true);
    std::array<float, 512> l {}, r {};
    std::array<float, 5> last {};
    last.fill(-1.0f);
    std::vector<int> result;
    for (int block = 0; block < 48000 * 60 / 512; ++block)
    {
        engine->render(l.data(), r.data(), l.size());
        const auto t = engine->telemetry();
        for (std::size_t i = 0; i < 5; ++i)
            if (t.autonomy.voices[i].targetMidiPitch != last[i])
            {
                last[i] = t.autonomy.voices[i].targetMidiPitch;
                result.push_back(static_cast<int>(std::lround(last[i])));
            }
    }
    return result;
}
}

int main()
{
    using aquorbium::SessionWorld;
    // the same seed, the same world; different seeds, different worlds
    const auto a = SessionWorld::fromSeed(1u), again = SessionWorld::fromSeed(1u);
    check(a.rootOffset == again.rootOffset && a.orbitSteps == again.orbitSteps && a.degrees == again.degrees
              && a.chordPath == again.chordPath && std::string(a.modeName) == again.modeName,
          "a world is reproducible from its seed");
    std::set<std::string> distinct;
    for (std::uint32_t seed = 1; seed <= 200; ++seed)
    {
        const auto w = SessionWorld::fromSeed(seed);
        distinct.insert(std::to_string(w.rootOffset) + w.modeName + std::to_string(w.orbitSteps)
                        + std::to_string(w.leader) + std::to_string(w.chordPath[1]));
        auto octavesOnly = true;
        for (const auto o : w.octaves)
            octavesOnly = octavesOnly && o % 12 == 0;
        check(octavesOnly && w.keyCentred && !w.classic, "a generated world: registers in octaves, one key");
    }
    std::cout << "200 seeds: " << distinct.size() << " distinct worlds\n";
    check(distinct.size() > 150, "the seeds draw many different worlds");

    // in the engine: other seeds, other melodies; every note in the key
    const auto first = SessionWorld::fromSeed(1899755276u), second = SessionWorld::fromSeed(12345u);
    const auto x = notes(&first, 1899755276u), y = notes(&second, 12345u);
    std::size_t same = 0;
    for (std::size_t i = 0; i < std::min(x.size(), y.size()); ++i)
        same += x[i] == y[i] ? 1u : 0u;
    std::cout << "melodies of two seeds: " << same << " of " << std::min(x.size(), y.size()) << " notes alike\n";
    check(same * 10 < std::min(x.size(), y.size()), "two seeds sing different melodies");
    // the progression (HarmonyPlan.h): every note is in the current key's
    // mode or in the sounding chord (a secondary dominant, a chromatic
    // mediant, a modulation bring their own notes); most are chord tones;
    // in five minutes the harmony turns its cycle and moves
    // the last case with the ecological music playing (MUSIC 0.7): its
    // notes are voiced by the same harmony
    for (const auto seed : { 1899755276u, 12345u, 777u, 4242u, 31337u })
    {
        const auto world = SessionWorld::fromSeed(seed);
        auto engine = std::make_unique<aquorbium::BiomaEngine>();
        engine->setSessionWorld(world);
        engine->prepare(48000.0);
        engine->setSeed(seed);
        engine->setAutonomousEnabled(true);
        if (seed == 31337u)
            engine->setEcologicalMusicPerformance({ 0.7f, 0.6f, 0.5f, 0.5f, 1.0f });
        std::array<float, 512> l {}, r {};
        std::array<float, 5> last {};
        last.fill(-1.0f);
        std::size_t total = 0, inChord = 0, stray = 0, cycles = 0;
        std::set<int> keys;
        std::set<int> languages;
        for (int block = 0; block < 48000 * 300 / 512; ++block)
        {
            engine->render(l.data(), r.data(), l.size());
            const auto t = engine->telemetry();
            const auto& a = t.autonomy;
            cycles = a.harmonyCycles;
            keys.insert(a.keyOffset);
            languages.insert(static_cast<int>(a.harmonyLanguage));
            for (std::size_t i = 0; i < 5; ++i)
            {
                const auto p = a.voices[i].targetMidiPitch;
                if (p == last[i])
                    continue;
                last[i] = p;
                if (a.chord.layerOffset != 0)
                    continue;  // a polytonal episode: the upper layer is in another key
                const auto relative = static_cast<int>(std::lround(p)) - (36 + world.rootOffset + a.keyOffset);
                const auto pc = ((relative % 12) + 12) % 12;
                auto inMode = false;
                for (std::size_t k = 0; k < world.scaleSize; ++k)
                    inMode = inMode || static_cast<int>(std::lround(world.scale[k])) == pc;
                ++total;
                inChord += a.chord.contains(pc) ? 1u : 0u;
                stray += !inMode && !a.chord.contains(pc) ? 1u : 0u;
            }
        }
        std::cout << "seed " << seed << ": " << inChord << " of " << total << " notes in the chord, " << stray
                  << " in neither the mode nor the chord; " << cycles << " cycles, " << keys.size() << " keys, "
                  << languages.size() << " languages\n";
        check(stray * 50 <= total, "every note in the mode or the chord (2 % tolerance for glides at a change)");
        check(inChord * 10 >= total * 8, "most notes are chord tones");
        check(cycles >= 3, "the harmony turns its cycle");
    }

    // the languages keep their grammar, over many seeds
    std::size_t functionalCadences = 0, functionalPlans = 0, impressionistVI = 0, chromaticSteps = 0, chromaticShared = 0;
    for (std::uint32_t seed = 1; seed <= 400; ++seed)
    {
        const auto w = SessionWorld::fromSeed(seed);
        const auto& plan = w.harmony;
        auto sane = plan.length >= 4;
        for (std::size_t i = 0; i < plan.length; ++i)
        {
            sane = sane && plan.chords[i].size >= 3;
            for (std::size_t k = 0; k < plan.chords[i].size; ++k)
                sane = sane && plan.chords[i].tones[k] >= 0 && plan.chords[i].tones[k] < 12;
        }
        check(sane, "a plan's chords are whole (no stale tones)");
        const auto& endChord = plan.chords[plan.length - 1];
        if (plan.language == aquorbium::HarmonyLanguage::functional)
        {
            ++functionalPlans;
            functionalCadences += endChord.cadence && endChord.tones[0] == 0 ? 1u : 0u;
        }
        if (plan.language == aquorbium::HarmonyLanguage::impressionist)
            for (std::size_t i = 1; i < plan.length; ++i)
                impressionistVI += plan.chords[i - 1].tones[0] == 7 && plan.chords[i].tones[0] == 0 ? 1u : 0u;
        if (plan.language == aquorbium::HarmonyLanguage::chromatic)
            for (std::size_t i = 1; i + 1 < plan.length; ++i)
            {
                std::size_t shared = 0;
                for (std::size_t k = 0; k < 3; ++k)
                    shared += plan.chords[i - 1].contains(plan.chords[i].tones[k]) ? 1u : 0u;
                ++chromaticSteps;
                chromaticShared += shared == 2 ? 1u : 0u;
            }
    }
    std::cout << "functional plans ending on I: " << functionalCadences << " of " << functionalPlans
              << "; impressionist V-I: " << impressionistVI << "; Cohn steps sharing two tones: " << chromaticShared
              << " of " << chromaticSteps << '\n';
    check(functionalPlans > 0 && functionalCadences == functionalPlans, "functional: the cycle closes on the tonic");
    check(impressionistVI == 0, "impressionist: never V - I");
    check(chromaticSteps > 0 && chromaticShared == chromaticSteps, "chromatic: each P/L/R step keeps two tones");

    // the classic world is the engine as it was: the same notes without it
    const auto classic = SessionWorld::classicWorld();
    check(notes(nullptr, 7u) == notes(&classic, 7u), "classic() is the engine as it was");

    if (failures == 0)
        std::cout << "session genesis: all checks passed\n";
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
