// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

// Every optional stage's bypass, in one sweep (2026-10-22; SAIDA_AUDIO_COMUM
// §5, "comparação bypass/neutralidade de cada estágio opcional"): with a
// stage's MIX at zero, its other controls - set to unusual values - must not
// change the output, bit for bit. Through AudioControlFrame, the path the
// interface uses.
#include "app/AudioControlFrame.h"
#include "core/BiomaEngine.h"

#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace
{
int failures = 0;
using Frame = aquorbium::app::AudioControlFrame;

std::vector<float> render(const Frame& frame, const std::function<void(aquorbium::BiomaEngine&)>& extra = {})
{
    auto engine = std::make_unique<aquorbium::BiomaEngine>();
    engine->prepare(48000.0);
    engine->setSeed(0xb7a55u);
    frame.apply(*engine, true);
    engine->setAutonomousEnabled(true);
    if (extra)
        extra(*engine);
    engine->noteOn(52, 0.7f);
    std::vector<float> out, l(256), r(256);
    for (int b = 0; b < 48000 * 5 / 256; ++b)
    {
        engine->render(l.data(), r.data(), l.size());
        out.insert(out.end(), l.begin(), l.end());
        out.insert(out.end(), r.begin(), r.end());
    }
    return out;
}

void expectSame(const std::string& stage, const std::vector<float>& a, const std::vector<float>& b)
{
    const auto same = a == b;
    std::cout << stage << ": " << (same ? "neutral at zero" : "CHANGES THE OUTPUT AT ZERO") << '\n';
    if (!same)
        ++failures;
}
}

bool runBypassSweepTests()
{
    Frame base;  // every optional MIX at its default zero
    const auto reference = render(base);
    struct Stage
    {
        const char* name;
        std::function<void(Frame&)> others;  // its other controls, unusual values
    };
    const std::vector<Stage> stages {
        { "GEOMETRY (influence 0)", [](Frame& f) { f.rhythmDensity = 0.95f; f.orbitRotation = 0.1f; f.intervalBias = 0.9f; f.geometryMutation = 0.8f; } },
        { "STRIKER (mix 0)", [](Frame& f) { f.strikerMatter = 0.05f; f.strikerPopulation = 0.95f; f.strikerScatter = 0.9f; } },
        { "ENVIRONMENT (mix 0)", [](Frame& f) { f.environmentFilterMix = 0.95f; f.environmentCentre = 0.1f; f.environmentSpan = 0.9f; f.environmentResonance = 0.95f; f.environmentAmbience = 0.9f; } },
        { "TAIL (mix 0)", [](Frame& f) { f.tailDecay = 0.95f; f.tailDamping = 0.05f; f.tailNightmare = 0.9f; } },
        { "FORM (influence 0)", [](Frame& f) { f.formContext = 0.95f; f.formInertia = 0.05f; f.formMutation = 0.9f; } },
        { "STRUCTURE (influence 0)", [](Frame& f) { f.structureMemory = 0.95f; f.structureHarmony = 0.05f; f.structurePhase = 0.9f; } },
        { "MUSIC (influence 0)", [](Frame& f) { f.musicReaction = 0.05f; f.musicMemory = 0.95f; f.musicWater = 0.95f; f.musicSalinity = 0.0f; } },
        { "TIMBRE (mix 0)", [](Frame& f) { f.timbreBody.fill(5.0f / 15.0f); f.timbreMatter.fill(0.95f); f.timbreEcology = 0.0f; } },
        { "CREATIVE MASTER (0)", [](Frame&) {} },
    };
    for (const auto& stage : stages)
    {
        Frame frame = base;
        stage.others(frame);
        expectSame(stage.name, reference, render(frame));
    }
    // the synthesis device at SYNTHESIS 0, with patches set
    expectSame("SYNTHESIS (mix 0)", reference, render(base, [](aquorbium::BiomaEngine& e) {
        aquorbium::SynthVoice::Patch p;
        p.sourceA = aquorbium::SynthVoice::Source::fm4;
        p.drive = 0.9f;
        for (std::size_t i = 0; i < 5; ++i)
            e.setSynthPatch(i, p);
        e.setSynthMix(0.0f);
    }));
    // the module bank at LAB MIX 0, with its families up
    expectSame("MODULE BANK (LAB MIX 0)", reference, render(base, [](aquorbium::BiomaEngine& e) {
        aquorbium::IntegratedModuleBank::Controls c {};
        c.fill(0.9f);
        c[aquorbium::IntegratedModuleBank::mix] = 0.0f;
        e.setIntegratedModuleBankControls(c);
    }));
    // the positive control: a stage that is on does change the output
    {
        Frame on = base;
        on.tailInfluence = 0.5f;
        const auto changed = render(on) != reference;
        std::cout << "control (TAIL at 0.5): " << (changed ? "changes the output" : "NO CHANGE - the sweep is blind") << '\n';
        if (!changed)
            ++failures;
    }
    if (failures == 0)
        std::cout << "bypass sweep: every optional stage neutral at zero\n";
    return failures == 0;
}
