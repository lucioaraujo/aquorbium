// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

// The opening seed and the VARIA (2026-10-24): every opening proposes another
// biome - gates, controls, bodies, ecology - and the controls move over the
// music's time, around what the seed or the musician left.
#include "core/ControlMotion.h"
#include "core/OpeningSeed.h"

#include <array>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
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

// the parameter ids the app persists, read from its own table, so a
// proposal can never name a control the app does not know
std::set<std::string> knownIds()
{
    std::ifstream in(AQUORBIUM_SOURCE_DIR "/src/app/PerformanceParameterState.h");
    std::stringstream text;
    text << in.rdbuf();
    const auto s = text.str();
    std::set<std::string> ids;
    const auto start = s.find("parameters {{");
    const auto end = s.find("}};", start);
    for (auto at = s.find("{ \"", start); at != std::string::npos && at < end; at = s.find("{ \"", at + 1))
    {
        const auto close = s.find('"', at + 3);
        ids.insert(s.substr(at + 3, close - at - 3));
    }
    return ids;
}
}

int main()
{
    using aquorbium::ControlMotion;
    using aquorbium::OpeningHypothesis;

    // ---- the opening seed ----------------------------------------------------
    const auto ids = knownIds();
    check(ids.size() == 94, "the app's table read: " + std::to_string(ids.size()) + " ids");
    {
        const auto a = OpeningHypothesis::fromSeed(1234u, 0.3f, 0.7f);
        const auto b = OpeningHypothesis::fromSeed(1234u, 0.3f, 0.7f);
        auto same = a.count == b.count;
        for (std::size_t i = 0; same && i < a.count; ++i)
            same = a.settings[i].id == b.settings[i].id && a.settings[i].value == b.settings[i].value;
        check(same, "the same seed proposes the same biome");
    }
    std::set<std::string> worlds, constellations;
    std::array<std::array<int, 12>, 5> bodies {};
    auto lightAtNoon = 0.0f, lightAtMidnight = 0.0f;
    for (std::uint32_t seed = 1; seed <= 400; ++seed)
    {
        const auto day = static_cast<float>(seed % 4) * 0.25f;  // 0.25: noon (full daylight), 0.75: midnight
        const auto h = OpeningHypothesis::fromSeed(seed, day, 0.4f);
        std::string world, gates;
        for (std::size_t i = 0; i < h.count; ++i)
        {
            const auto& s = h.settings[i];
            const auto id = std::string(s.id);
            check(ids.count(id) == 1, "a known control: " + id);
            check(!OpeningHypothesis::musiciansOwn(s.id), "the musician's own control untouched: " + id);
            check(s.value >= 0.0f && s.value <= 1.0f, "a value in range: " + id);
            world += id + "=" + std::to_string(std::lround(s.value * 20.0f)) + ";";
        }
        std::size_t open = 0;
        for (const auto gate : OpeningHypothesis::gates)
        {
            const auto* s = h.find(gate);
            check(s != nullptr, "every gate proposed: " + std::string(gate));
            if (s != nullptr && s->value > 0.0f)
            {
                ++open;
                gates += std::string(gate) + " ";
            }
            check(s == nullptr || s->value == 0.0f || s->value >= 0.2f, "a gate closed or clearly open");
        }
        check(open == h.gatesOpened && open >= 1 && open <= 4,
              "one to four modules wake: " + std::to_string(open));
        for (const auto* id : { "masterTrim", "masterCeiling", "creativeMaster", "supportPresence", "lensRadius",
                                "roomSensitivity", "masterWidth" })
            check(h.find(id) == nullptr, std::string("the output stage stays the musician's: ") + id);
        for (std::size_t i = 0; i < 5; ++i)
        {
            const auto* kind = h.find("timbreKind" + std::to_string(i));
            if (kind != nullptr)
                ++bodies[i][static_cast<std::size_t>(std::lround(kind->value * 15.0f))];
        }
        if (day == 0.25f)
            lightAtNoon += h.genes.light;
        if (day == 0.75f)
            lightAtMidnight += h.genes.light;
        worlds.insert(world);
        constellations.insert(gates);
        check(h.density >= 0.1f && h.density <= 0.85f && h.calm >= 0.3f && h.calm <= 0.95f, "the ecology in range");
        auto sum = 0.0f;
        for (const auto b : h.behaviours)
            sum += b;
        check(std::abs(sum - 1.0f) < 1.0e-4f, "the behaviours share the whole");
    }
    std::cout << "400 seeds: " << worlds.size() << " biomes, " << constellations.size() << " constellations of gates\n";
    check(worlds.size() == 400, "every seed another biome");
    check(constellations.size() >= 60, "many constellations of modules");
    // the light leans to the hour the session wakes in
    std::cout << "mean light: noon " << lightAtNoon / 100.0f << ", midnight " << lightAtMidnight / 100.0f << '\n';
    check(lightAtNoon / 100.0f > lightAtMidnight / 100.0f + 0.3f, "brighter water at noon than at midnight");
    // the species lean to their bodies: the STRING to string, bow and glass,
    // the MODAL to glass and membrane - and other bodies still come
    const auto share = [&](std::size_t organism, std::initializer_list<int> kinds) {
        auto in = 0, all = 0;
        for (int k = 1; k < 12; ++k)
        {
            all += bodies[organism][static_cast<std::size_t>(k)];
            for (const auto x : kinds)
                in += k == x ? bodies[organism][static_cast<std::size_t>(k)] : 0;
        }
        return all > 0 ? static_cast<float>(in) / static_cast<float>(all) : 0.0f;
    };
    std::cout << "STRING's affinity share " << share(4, { 4, 10, 8 }) << ", MODAL's " << share(2, { 8, 7, 5 }) << '\n';
    check(share(4, { 4, 10, 8 }) > 0.5f && share(4, { 4, 10, 8 }) < 0.95f, "the STRING leans to its bodies, not only");
    check(share(2, { 8, 7, 5 }) > 0.5f && share(2, { 8, 7, 5 }) < 0.95f, "the MODAL leans to its bodies, not only");

    // ---- the VARIA -------------------------------------------------------------
    const std::array<ControlMotion::Control, 8> controls {{
        { "spaceTone", 0.5f }, { "spaceSize", 0.6f }, { "tailInfluence", 0.0f }, { "strikerInfluence", 0.4f },
        { "timbreKind0", 4.0f / 15.0f }, { "masterTrim", 0.75f }, { "orbitRotation", 0.5f }, { "environmentResonance", 0.45f } }};
    const auto run = [&](float intensity, float tension, float seconds, float ceiling, std::uint32_t seed,
                         std::vector<std::array<float, 8>>* trace) {
        ControlMotion motion;
        motion.inhabit(controls, seed);
        motion.setIntensity(intensity);
        std::array<float, 8> v {};
        for (std::size_t i = 0; i < v.size(); ++i)
            v[i] = controls[i].value;
        for (int frame = 0; frame < static_cast<int>(seconds * 30.0f); ++frame)
        {
            motion.tick(v, 1.0f / 30.0f, 0.5f, tension, ceiling);
            if (trace != nullptr)
                trace->push_back(v);
        }
        return v;
    };
    {
        std::vector<std::array<float, 8>> trace;
        run(0.0f, 0.5f, 60.0f, 0.0f, 9u, &trace);
        auto still = true;
        for (const auto& v : trace)
            for (std::size_t i = 0; i < v.size(); ++i)
                still = still && v[i] == controls[i].value;
        check(still, "VARIA at zero writes nothing");
    }
    {
        std::vector<std::array<float, 8>> trace;
        run(1.0f, 0.5f, 600.0f, 0.0f, 9u, &trace);
        float low = 1.0f, high = 0.0f, orbitLow = 1.0f, orbitHigh = 0.0f, gateLow = 1.0f, gateHigh = 0.0f;
        auto fixedStill = true, closedStill = true;
        std::set<int> visited;
        for (const auto& v : trace)
        {
            low = std::min(low, v[0]);
            high = std::max(high, v[0]);
            orbitLow = std::min(orbitLow, v[6]);
            orbitHigh = std::max(orbitHigh, v[6]);
            gateLow = std::min(gateLow, v[3]);
            gateHigh = std::max(gateHigh, v[3]);
            fixedStill = fixedStill && v[4] == controls[4].value && v[5] == controls[5].value;
            closedStill = closedStill && v[2] == 0.0f;
            visited.insert(static_cast<int>(std::lround(v[0] * 200.0f)));
        }
        std::cout << "10 min of VARIA: tone " << low << ".." << high << ", orbit " << orbitLow << ".." << orbitHigh
                  << ", striker gate " << gateLow << ".." << gateHigh << '\n';
        check(high - low > 0.08f && high - low < 0.5f, "a free control moves, around its value");
        check(orbitHigh - orbitLow < 0.05f, "the structure barely moves");
        check(gateHigh - gateLow < 0.12f && gateLow > 0.0f, "an open gate breathes and stays open");
        check(fixedStill, "the bodies' choice and the master never move");
        check(closedStill, "a closed gate stays exactly closed");
        check(visited.size() > 20, "the free control visits many values");
    }
    {
        // the musician's hand: the control re-anchors where it was put
        ControlMotion motion;
        motion.inhabit(controls, 3u);
        std::array<float, 8> v {};
        for (std::size_t i = 0; i < v.size(); ++i)
            v[i] = controls[i].value;
        for (int frame = 0; frame < 300; ++frame)
            motion.tick(v, 1.0f / 30.0f, 0.5f, 0.5f);
        v[1] = 0.15f;  // the musician moves SPACE SIZE down
        auto far = 0.0f;
        for (int frame = 0; frame < 3000; ++frame)
        {
            motion.tick(v, 1.0f / 30.0f, 0.5f, 0.5f);
            far = std::max(far, std::abs(v[1] - 0.15f));
        }
        check(std::abs(motion.centreOf(1) - 0.15f) < 1.0e-6f && far < 0.25f, "the musician's value becomes the centre");
    }
    {
        // the form: at the climax brighter, at rest darker; near the ceiling home
        const auto calm = run(1.0f, 0.0f, 120.0f, 0.0f, 5u, nullptr);
        const auto climax = run(1.0f, 1.0f, 120.0f, 0.0f, 5u, nullptr);
        std::cout << "tone at rest " << calm[0] << ", at the climax " << climax[0] << '\n';
        check(climax[0] > calm[0], "the climax brightens the tone");
        check(climax[1] < calm[1], "the climax shrinks the space");
        const auto loud = run(1.0f, 0.5f, 120.0f, 1.0f, 5u, nullptr);
        check(std::abs(loud[7] - controls[7].value) < 0.005f, "near the ceiling the resonance goes home");
        const auto again = run(1.0f, 0.5f, 120.0f, 0.0f, 5u, nullptr);
        const auto twice = run(1.0f, 0.5f, 120.0f, 0.0f, 5u, nullptr);
        check(again == twice, "the VARIA is deterministic for a seed");
    }

    if (failures == 0)
        std::cout << "opening seed tests passed\n";
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
