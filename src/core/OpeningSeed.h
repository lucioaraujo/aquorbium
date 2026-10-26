// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace aquorbium
{
// The opening seed (2026-10-24). The author: "preciso que desenvolva o seed de
// abertura, para que a cada vez que o instrumento for aberto soe diferente".
// It did not: the session's genesis (SessionGenesis.h) drew another piece -
// key, mode, harmony, orbits, pace - but the app then restored the 108 saved
// controls and the five synthesis patches, so every opening had the same
// bodies, the same space, the same density, the same modules on: the same
// instrument playing other notes.
//
// As in the Rasgo Modular's seed (dossies/ESTUDO_seed_composicao_generativa.md
// §1.1 and §2.1: the seed as a HYPOTHESIS, drawn from genes, not a
// randomize), the seed now proposes the sound too. But the Modular's grammar
// is a patch's (source -> transform -> output); the Aquorbium's is a BIOME's
// (the author: "levando em consideração os conceitos e particularidades de
// cada instrumento"). Rewritten here, no code shared. The genes are the
// habitat's own quantities, the ones the ecosystem already lives by
// (Ecosystem.h: temperature, daylight, turbidity, the school's cohesion, the
// danger, the salinity):
//   TEMPERATURE  the metabolism: the brain's clock, how fast the water's
//                music reacts, how strongly the open modules sound; it leans
//                to the season the session wakes in;
//   LIGHT        how deep the light reaches: bright water or the dark of the
//                deep - the tone of the space, the environment's filter, the
//                bodies' matter; it leans to the hour the session wakes in;
//   TURBIDITY    clear or murky water: diffusion, damping, ambience;
//   CURRENT      still water or a current: mutation, scatter, memory, calm;
//   POPULATION   how much lives here: density, the striker's population, the
//                rhythm's density;
//   PREDATION    the danger in the water: danger, the nightmare in the tail,
//                how many modules wake (the boldness of the place);
//   HABITAT      a narrow pool or the open sea: the space's size and return,
//                the tail's decay; the sea leans to the open.
// What it proposes:
//   the GATES    which modules open (a card's first control is its amount in
//                the mix): one to four of them, another constellation each
//                time;
//   the CONTROLS every other control around its classic value, pulled by
//                the genes, with a little chance of its own;
//   the BODIES   each organism's body, leaning to the bodies its species has
//                an affinity with (an influence, not a rule: any body can
//                come);
//   the ECOLOGY  density, calm, cohesion, danger and the four behaviours.
// The synthesis device's patches come from SynthProposal (the PROPOSTA) with
// the same seed: there the species and strategies already shape each voice.
// The output stage stays the musician's - the MASTER, the creative MASTER,
// the SUPPORT balance, the LUPA and the SALA - as the Rasgo Modular leaves
// its MIXER and MASTER to the musician's hand.
//
// It is a point of departure: the musician plays and changes everything from
// there, and the VARIA (ControlMotion.h) can move it over time.
struct OpeningHypothesis
{
    struct Genes
    {
        float temperature = 0.5f, light = 0.5f, turbidity = 0.35f, current = 0.5f, population = 0.5f,
              predation = 0.5f, habitat = 0.5f;
    };
    struct Setting
    {
        std::string_view id;
        float value = 0.0f;
    };
    static constexpr std::size_t capacity = 96;

    Genes genes {};
    std::array<Setting, capacity> settings {};
    std::size_t count = 0;
    // the ecology (EcologyParameterState keys)
    float density = 0.45f, calm = 0.8f, cohesion = 0.5f, danger = 0.5f;
    std::array<float, 4> behaviours { 0.25f, 0.25f, 0.25f, 0.25f };  // sparse, energy memory, cycle, drift
    float synthMix = 0.0f;
    std::size_t gatesOpened = 0;

    [[nodiscard]] const Setting* find(std::string_view id) const noexcept
    {
        for (std::size_t i = 0; i < count; ++i)
            if (settings[i].id == id)
                return &settings[i];
        return nullptr;
    }

    // controls the opening never touches: the output stage and the musician's
    // own surfaces
    [[nodiscard]] static bool musiciansOwn(std::string_view id) noexcept
    {
        return id.starts_with("master") || id == "creativeMaster" || id == "supportPresence" || id.starts_with("lens")
            || id.starts_with("room") || id.starts_with("timbreBody");
    }

    // the gates, in the order the cards show them (spaceGesture is the
    // SPACE card's door; moduleBankMix the LAB's)
    static constexpr std::array<std::string_view, 8> gates { "strikerInfluence", "spaceGesture", "geometryInfluence",
                                                             "environmentInfluence", "tailInfluence", "formInfluence",
                                                             "structureInfluence", "musicInfluence" };
    static constexpr std::array<std::string_view, 15> labModules {
        "moduleSynthBodies", "moduleGranularMemory", "modulePercussionPulses", "moduleFractalVector",
        "moduleLorenzDrift", "moduleFluidPhase", "moduleGestureWaves", "moduleClockEvents", "moduleTransposeSelect",
        "moduleBranchPermute", "moduleRotateLayout", "modulePatternLoop", "moduleShiftProbability",
        "moduleEvolutionStages", "moduleIntegrityMemory" };

    // dayPhase and seasonPhase: the hour and season the session wakes in
    // (SessionWorld), so the light leans to the hour and the temperature to
    // the season, as the ecosystem computes them (Ecosystem.cpp)
    [[nodiscard]] static OpeningHypothesis fromSeed(std::uint32_t seed, float dayPhase = 0.25f,
                                                    float seasonPhase = 0.25f) noexcept
    {
        OpeningHypothesis h;
        // its own stream, so the genesis's draws (SessionGenesis.h) stay as
        // they were for the same seed
        std::uint64_t state = 0x6a09e667f3bcc909ull ^ (static_cast<std::uint64_t>(seed) * 0x9e3779b97f4a7c15ull);
        const auto next = [&state]() {
            state += 0x9e3779b97f4a7c15ull;
            auto z = state;
            z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ull;
            z = (z ^ (z >> 27)) * 0x94d049bb133111ebull;
            return z ^ (z >> 31);
        };
        const auto unit = [&next]() { return static_cast<float>(next() >> 40) / 16777216.0f; };
        const auto pick = [&next](std::size_t n) { return static_cast<std::size_t>(next() % n); };
        // a gene: the mean of two draws leans to the middle, the extremes stay possible
        const auto gene = [&unit]() { return 0.5f * (unit() + unit()); };
        constexpr float twoPi = 6.2831853f;
        auto& g = h.genes;
        g.temperature = 0.55f * (0.5f + 0.5f * std::sin(seasonPhase * twoPi)) + 0.45f * gene();
        g.light = 0.55f * (0.5f + 0.5f * std::sin(dayPhase * twoPi)) + 0.45f * gene();
        g.turbidity = gene();
        g.current = gene();
        g.population = gene();
        g.predation = unit();
        const auto sea = unit() < 0.65f;  // the sea, or fresh water
        g.habitat = std::clamp(gene() + (sea ? 0.15f : -0.15f), 0.0f, 1.0f);

        const auto set = [&h](std::string_view id, float value) {
            if (h.count < capacity && !musiciansOwn(id))
                h.settings[h.count++] = { id, std::clamp(value, 0.0f, 1.0f) };
        };
        // a control around a centre, pulled by a gene and with a little
        // chance of its own; kept off the extremes
        const auto around = [&](float centre, float pull, float spread = 0.12f) {
            return std::clamp(centre + pull + (unit() - 0.5f) * 2.0f * spread, 0.05f, 0.95f);
        };
        const auto c = [](float v) { return v - 0.5f; };  // a gene around its middle
        const auto dark = 1.0f - g.light;

        // the gates: one to four modules wake, more where the water is dangerous
        std::array<std::size_t, gates.size()> order {};
        for (std::size_t i = 0; i < order.size(); ++i)
            order[i] = i;
        for (std::size_t i = order.size() - 1; i > 0; --i)
            std::swap(order[i], order[pick(i + 1)]);
        h.gatesOpened = 1 + static_cast<std::size_t>(std::min(3.0f, g.predation * 2.2f + unit() * 1.4f));
        std::array<bool, gates.size()> open {};
        for (std::size_t k = 0; k < h.gatesOpened; ++k)
            open[order[k]] = true;
        for (std::size_t i = 0; i < gates.size(); ++i)
            set(gates[i], open[i] ? 0.22f + 0.5f * g.temperature * (0.6f + 0.4f * unit()) : 0.0f);
        // the LAB: sometimes, with two to four of its modules
        const auto lab = unit() < 0.15f + 0.3f * g.predation;
        set("moduleBankMix", lab ? 0.2f + 0.4f * unit() : 0.0f);
        std::array<bool, labModules.size()> labOn {};
        if (lab)
            for (std::size_t k = 0, n = 2 + pick(3); k < n; ++k)
                labOn[pick(labOn.size())] = true;
        for (std::size_t i = 0; i < labModules.size(); ++i)
            set(labModules[i], labOn[i] ? 0.3f + 0.5f * unit() : 0.0f);

        // the cards' controls, read from the habitat
        set("ecosystemMorph", 0.45f + 0.55f * unit());
        set("rhythmDensity", around(0.5f, 0.6f * c(g.population)));
        set("orbitRotation", around(0.5f, 0.0f, 0.3f));
        set("intervalBias", around(0.5f, 0.25f * c(g.predation), 0.2f));
        set("geometryMutation", around(0.15f, 0.4f * c(g.current), 0.1f));
        set("strikerMatter", around(0.5f, -0.4f * c(dark), 0.2f));
        set("strikerPopulation", around(0.5f, 0.6f * c(g.population)));
        set("strikerScatter", around(0.5f, 0.5f * c(g.current)));
        set("spaceReturn", around(0.48f, 0.35f * c(g.habitat), 0.08f));
        set("spaceSize", around(0.5f, 0.8f * c(g.habitat)));
        set("spaceSpread", around(0.8f, 0.3f * c(g.turbidity), 0.1f));
        set("spaceTone", around(0.5f, -0.5f * c(dark) - 0.2f * c(g.turbidity)));
        set("spaceMemory", around(0.35f, 0.4f * c(g.current)));
        set("environmentFilterMix", around(0.5f, 0.3f * c(g.turbidity), 0.15f));
        set("environmentCentre", around(0.5f, -0.7f * c(dark)));
        set("environmentSpan", around(0.5f, 0.0f, 0.2f));
        set("environmentResonance", around(0.45f, 0.15f * c(g.predation), 0.08f));  // hot: it breathes, never sweeps
        set("environmentAmbience", around(0.2f, 0.4f * c(g.turbidity) + 0.2f * c(g.habitat), 0.1f));
        set("tailDecay", around(0.45f, 0.6f * c(g.habitat)));
        set("tailDamping", around(0.55f, 0.35f * c(dark) + 0.25f * c(g.turbidity)));
        set("tailNightmare", std::clamp(0.9f * g.predation * g.predation * dark, 0.0f, 0.45f));
        set("formContext", around(0.5f, 0.0f, 0.25f));
        set("formInertia", around(0.35f, -0.5f * c(g.current)));
        set("formMutation", around(0.08f, 0.2f * c(g.current), 0.05f));
        set("structureMemory", around(0.5f, 0.0f, 0.2f));
        set("structureHarmony", around(0.5f, 0.0f, 0.2f));
        set("structurePhase", around(0.5f, 0.0f, 0.3f));
        set("musicReaction", around(0.6f, 0.4f * c(g.temperature)));
        set("musicMemory", around(0.5f, -0.3f * c(g.current)));
        set("musicWater", around(0.5f, 0.3f * c(g.turbidity), 0.2f));
        set("musicSalinity", sea ? 1.0f : 0.0f);
        set("brainClock", around(0.5f, 0.5f * c(g.temperature)));
        set("brainCoordination", std::clamp(1.2f * unit() * g.current, 0.0f, 0.6f));

        // the bodies: each organism its native voice or a body, leaning to
        // the bodies its species has an affinity with (TimbreBody::Kind:
        // 1 voice, 2 matter, 3 breath, 4 string, 5 drop, 6 fold, 7 membrane,
        // 8 glass, 9 choir, 10 bow, 11 reed)
        constexpr int kinds = 12;      // TimbreBody::kindCount
        constexpr float scale = 15.0f; // TimbreBody::kindScale
        static constexpr std::array<std::array<int, 3>, 5> affinity {{
            {{ 6, 9, 11 }},  // MORPH, an oscillator: fold, choir, reed
            {{ 2, 5, 3 }},   // GRAIN, a granular striker: matter, drop, breath
            {{ 8, 7, 5 }},   // MODAL, a resonator: glass, membrane, drop
            {{ 7, 5, 2 }},   // PULSAR: membrane, drop, matter
            {{ 4, 10, 8 }},  // STRING: string, bow, glass
        }};
        static constexpr std::array<std::string_view, 5> kindIds { "timbreKind0", "timbreKind1", "timbreKind2",
                                                                   "timbreKind3", "timbreKind4" };
        static constexpr std::array<std::string_view, 5> matterIds { "timbreMatter0", "timbreMatter1", "timbreMatter2",
                                                                     "timbreMatter3", "timbreMatter4" };
        static constexpr std::array<std::string_view, 5> mixIds { "timbreMix0", "timbreMix1", "timbreMix2",
                                                                  "timbreMix3", "timbreMix4" };
        const auto bodied = 0.3f + 0.4f * g.population;
        for (std::size_t i = 0; i < 5; ++i)
        {
            auto body = 0;
            if (unit() < bodied)
                body = unit() < 0.6f ? affinity[i][pick(3)] : 1 + static_cast<int>(pick(kinds - 1));
            set(kindIds[i], static_cast<float>(body) / scale);
            set(matterIds[i], around(0.5f, -0.3f * c(dark), 0.25f));
            set(mixIds[i], body > 0 ? 0.25f + 0.55f * unit() : 0.0f);
        }
        set("timbreEcology", unit() < 0.5f ? 0.0f : 0.2f + 0.6f * g.current);

        // the ecology
        h.density = std::clamp(0.18f + 0.62f * g.population, 0.1f, 0.85f);
        h.calm = std::clamp(0.95f - 0.6f * g.current + (unit() - 0.5f) * 0.1f, 0.3f, 0.95f);
        h.cohesion = around(0.5f, 0.0f, 0.3f);
        h.danger = around(0.4f, 0.45f * c(g.predation), 0.12f);
        auto total = 0.0f;
        for (auto& b : h.behaviours)
        {
            b = 0.05f + unit();
            total += b;
        }
        for (auto& b : h.behaviours)
            b /= total;
        // the synthesis device: in most sessions, at a level the temperature sets
        h.synthMix = unit() < 0.6f ? 0.15f + 0.45f * g.temperature : 0.0f;
        return h;
    }
};
}
