// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

// Fast automation without clicks, and mono compatibility (aquorbium-tarefas.md,
// phase 10; 2026-10-22).
//
// Clicks: two identical engines (same seed, same controls) play together and
// only one gets an abrupt change. Their difference D is exactly what the
// change did. A smoothed change makes D grow; a step makes D jump from one
// sample to the next at the instant of the change. The test looks at the
// high band (second difference) of D in the first 2 ms after it appears (the
// output stage's 5 ms lookahead delays it) against 30..32 ms later: a change
// smoothed over ~15 ms starts as a fraction of what it becomes; a step is
// whole from its first sample, up or down. Positive controls (a true x1.5
// gain step, a hard cut to silence) prove the measure sees real ones.
//
// Mono: the stereo correlation and the loss of the mono fold-down (L+R)/2,
// with the speakers and the headphones, wide and narrow.
#include "app/AudioControlFrame.h"
#include "core/BiomaEngine.h"
#include "core/SynthProposal.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace
{
int failures = 0;
void expect(bool condition, const std::string& what)
{
    if (!condition)
    {
        std::cerr << "automation requirement failed: " << what << '\n';
        ++failures;
    }
}

constexpr double rate = 48000.0;
constexpr std::size_t block = 128;

using Change = std::function<void(aquorbium::BiomaEngine&, aquorbium::app::AudioControlFrame&)>;

struct Setup
{
    bool synth = false;  // the synthesis device's voices playing (PROPOSTA, SYNTHESIS 0.6)
    bool creativeMaster = false;  // CREATIVE MASTER at 1 (for the EQ order)
    // the module bank open (LAB MIX 1): loud enough for the creative master's
    // dynamics to act - at the instrument's usual level they do not compress,
    // and the EQ order changes nothing (x*eq*1 = x*1*eq)
    bool moduleBank = false;
};

std::unique_ptr<aquorbium::BiomaEngine> engineFor(const aquorbium::app::AudioControlFrame& frame, const Setup& setup)
{
    auto engine = std::make_unique<aquorbium::BiomaEngine>();
    engine->prepare(rate);
    engine->setSeed(0xc11cu);
    frame.apply(*engine, true);
    engine->setAutonomousEnabled(true);
    if (setup.synth)
    {
        for (std::size_t i = 0; i < aquorbium::SynthProposal::organisms; ++i)
            engine->setSynthPatch(i, aquorbium::SynthProposal::propose(i, 0xc11cu));
        engine->setSynthMix(0.6f);
    }
    if (setup.moduleBank)
    {
        aquorbium::IntegratedModuleBank::Controls c {};
        c.fill(0.6f);
        c[aquorbium::IntegratedModuleBank::mix] = 1.0f;
        engine->setIntegratedModuleBankControls(c);
    }
    engine->noteOn(48, 0.8f);
    engine->noteOn(55, 0.7f);
    return engine;
}

struct ClickResult
{
    double jump = 0.0, ratio = 0.0, delayMs = -1.0;  // jump: D's early high band, RMS; ratio: early / 30 ms later
};

// the abrupt change at 3 s; returns the largest sample-to-sample step of the
// difference just after it, and in a settled stretch later
// syntheticStep > 0: no engine change; instead the recorded output of B is
// multiplied by it from 5 ms on - a true, instantaneous gain step (the
// positive control of the measure)
ClickResult measureChange(const Change& change, const Setup& setup = {}, double syntheticStep = 0.0)
{
    aquorbium::app::AudioControlFrame frame;
    frame.musicInfluence = 0.6f;  // the ecological music playing
    frame.spaceGesture = 0.5f;    // SPACE PERFORMANCE on: SIZE and SPREAD act
    if (setup.creativeMaster)
        frame.creativeMaster = 1.0f;
    auto a = engineFor(frame, setup);
    auto b = engineFor(frame, setup);
    std::array<float, block> la {}, ra {}, lb {}, rb {};
    const auto changeAt = static_cast<std::size_t>(rate * 3.0) / block * block;
    const auto end = changeAt + static_cast<std::size_t>(rate * 2.0);
    // both outputs from just before the change on
    std::vector<std::array<double, 4>> kept;
    kept.reserve(end - changeAt + block);
    for (std::size_t done = 0; done < end; done += block)
    {
        if (done == changeAt)
            change(*b, frame);
        a->render(la.data(), ra.data(), block);
        b->render(lb.data(), rb.data(), block);
        if (done + block >= changeAt)
            for (std::size_t i = 0; i < block; ++i)
                if (done + i + 2 >= changeAt)
                    kept.push_back({ la[i], ra[i], lb[i], rb[i] });
    }
    if (syntheticStep > 0.0)
        for (std::size_t k = 2 + static_cast<std::size_t>(rate * 0.005); k < kept.size(); ++k)
        {
            kept[k][2] *= syntheticStep;
            kept[k][3] *= syntheticStep;
        }
    ClickResult result;
    // the first sample where the change shows (the output stage's lookahead
    // delays it ~5 ms)
    std::size_t onset = 0;
    for (std::size_t k = 2; k < kept.size() && onset == 0; ++k)
        if (kept[k][2] != kept[k][0] || kept[k][3] != kept[k][1])
            onset = k;
    if (onset == 0)
        return result;
    result.delayMs = 1000.0 * static_cast<double>(onset - 2) / rate;
    // the high band (second difference) of the difference D = B - A: in its
    // first 2 ms, and 30..32 ms later. A change smoothed over ~15 ms starts
    // small (D early is a fraction of D later); a step is whole from its
    // first sample, up or down (a hard mute too), plus its own spike
    const auto window = static_cast<std::size_t>(rate * 0.002);
    const auto later = static_cast<std::size_t>(rate * 0.030);
    const auto highBand = [&kept, window](std::size_t from) {
        double energy = 0.0;
        for (std::size_t k = from; k < std::min(kept.size(), from + window); ++k)
            for (std::size_t c = 0; c < 2; ++c)
            {
                const auto d0 = kept[k][c + 2] - kept[k][c];
                const auto d1 = kept[k - 1][c + 2] - kept[k - 1][c];
                const auto d2 = kept[k - 2][c + 2] - kept[k - 2][c];
                const auto hf = d0 - 2.0 * d1 + d2;
                energy += hf * hf;
            }
        return energy;
    };
    const auto early = highBand(onset), settled = highBand(onset + later);
    result.ratio = std::sqrt(early / std::max(settled, 1.0e-24));
    result.jump = std::sqrt(early / static_cast<double>(2 * window));  // D's early high band, RMS
    return result;
}

struct MonoResult
{
    double correlation = 0.0, monoLossDb = 0.0;
    // the same below ~150 Hz: the bass foundation (2026-10-22)
    double lowCorrelation = 0.0, lowMonoLossDb = 0.0;
};

MonoResult measureMono(bool headphones, float spread)
{
    aquorbium::app::AudioControlFrame frame;
    frame.musicInfluence = 0.6f;
    frame.spaceGesture = 0.5f;
    frame.spaceSpread = spread;
    auto engine = engineFor(frame, {});
    engine->setHeadphones(headphones);
    std::array<float, block> l {}, r {};
    double ll = 0.0, rr = 0.0, lr = 0.0;
    double lowL = 0.0, lowR = 0.0, lll = 0.0, lrr = 0.0, llr = 0.0;
    // a 4th-order low-pass at ~150 Hz (four one-poles), the bass band
    std::array<double, 4> stateL {}, stateR {};
    const auto lowCoefficient = 1.0 - std::exp(-2.0 * 3.14159265358979 * 150.0 / rate);
    for (std::size_t done = 0; done < static_cast<std::size_t>(rate * 20.0); done += block)
    {
        engine->render(l.data(), r.data(), block);
        if (done < static_cast<std::size_t>(rate * 4.0))
            continue;
        for (std::size_t i = 0; i < block; ++i)
        {
            lowL = l[i];
            lowR = r[i];
            for (std::size_t p = 0; p < 4; ++p)
            {
                stateL[p] += (lowL - stateL[p]) * lowCoefficient;
                stateR[p] += (lowR - stateR[p]) * lowCoefficient;
                lowL = stateL[p];
                lowR = stateR[p];
            }
            lll += lowL * lowL;
            lrr += lowR * lowR;
            llr += lowL * lowR;
            ll += static_cast<double>(l[i]) * l[i];
            rr += static_cast<double>(r[i]) * r[i];
            lr += static_cast<double>(l[i]) * r[i];
        }
    }
    MonoResult m;
    m.correlation = lr / std::sqrt(std::max(ll * rr, 1.0e-30));
    // the mono sum's power against the stereo pair's mean power: 0 dB when
    // L = R, -3 dB when they are unrelated, -inf when they cancel
    const auto mono = (ll + rr + 2.0 * lr) / 4.0;
    m.monoLossDb = 10.0 * std::log10(std::max(mono, 1.0e-30) / std::max((ll + rr) / 2.0, 1.0e-30));
    m.lowCorrelation = llr / std::sqrt(std::max(lll * lrr, 1.0e-30));
    const auto lowMono = (lll + lrr + 2.0 * llr) / 4.0;
    m.lowMonoLossDb = 10.0 * std::log10(std::max(lowMono, 1.0e-30) / std::max((lll + lrr) / 2.0, 1.0e-30));
    return m;
}
}

bool runAutomationClickTests()
{
    struct Case
    {
        const char* name;
        Change change;
        Setup setup;
    };
    using Frame = aquorbium::app::AudioControlFrame;
    using Engine = aquorbium::BiomaEngine;
    const auto frameChange = [](auto edit) {
        return Change([edit](Engine& e, Frame& f) { edit(f); f.apply(e); });
    };
    const std::vector<Case> cases {
        { "OUTPUT TRIM 0 dB -> +6 dB", frameChange([](Frame& f) { f.masterTrim = 1.0f; }), {} },
        { "OUTPUT TRIM 0 dB -> -18 dB", frameChange([](Frame& f) { f.masterTrim = 0.0f; }), {} },
        { "MUTE", Change([](Engine& e, Frame&) {
              auto p = e.outputStageParameters(); p.muted = true; e.setOutputStageParameters(p); }), {} },
        { "SYNTHESIS 0.6 -> 0", Change([](Engine& e, Frame&) { e.setSynthMix(0.0f); }), { true } },
        { "SYNTHESIS 0.6 -> 1", Change([](Engine& e, Frame&) { e.setSynthMix(1.0f); }), { true } },
        { "HEADPHONES on", Change([](Engine& e, Frame&) { e.setHeadphones(true); }), {} },
        { "SPACE MIX -> 1", frameChange([](Frame& f) { f.spaceReturn = 1.0f; }), {} },
        { "SPACE SPREAD (width) 1 -> 0", frameChange([](Frame& f) { f.spaceSpread = 0.0f; }), {} },
        { "CREATIVE MASTER 0 -> 1 (bypass off)", frameChange([](Frame& f) { f.creativeMaster = 1.0f; }), {} },
        { "TAIL MIX 0 -> 1", frameChange([](Frame& f) { f.tailInfluence = 1.0f; }), {} },
        { "ENVIRONMENT MIX 0 -> 1", frameChange([](Frame& f) { f.environmentInfluence = 1.0f; }), {} },
        { "STRIKER MIX 0 -> 1", frameChange([](Frame& f) { f.strikerInfluence = 1.0f; }), {} },
        { "MORPH 1 -> 0", frameChange([](Frame& f) { f.ecosystemMorph = 0.0f; }), {} },
        { "TIMBRE MIX 0 -> 1 (voice body)", frameChange([](Frame& f) {
              f.timbreBody.fill(1.0f / 15.0f); f.timbreMix.fill(1.0f); }), {} },
        { "MODULE BANK (LAB MIX) 0 -> 1", Change([](Engine& e, Frame&) {
              aquorbium::IntegratedModuleBank::Controls c {};
              c.fill(0.6f);
              c[aquorbium::IntegratedModuleBank::mix] = 1.0f;
              e.setIntegratedModuleBankControls(c); }), {} },
        // soft take-over (phase 5b, 2026-10-23): the paths of Desire and of
        // the Brain, slammed as a future AccessPort could slam them
        { "DENSITY 0.45 -> 1 (Desire)", frameChange([](Frame& f) { f.density = 1.0f; }), {} },
        { "DRIFT 0.2 -> 1 (Desire calm)", frameChange([](Frame& f) { f.drift = 1.0f; }), {} },
        { "COHESION 0.5 -> 0 (Desire)", frameChange([](Frame& f) { f.cohesion = 0.0f; }), {} },
        { "DANGER 0.5 -> 1 (Desire)", frameChange([](Frame& f) { f.danger = 1.0f; }), {} },
        { "LISTENING -> all sparse focus", frameChange([](Frame& f) {
              f.sparseFocus = 1.0f; f.energyMemory = 0.0f; f.geometricCycle = 0.0f; f.collectiveDrift = 0.0f; }), {} },
        { "LENS held on the glass", frameChange([](Frame& f) {
              f.lensHeld = true; f.lensX = 0.0f; f.lensY = 1.0f; f.lensZ = 0.0f; }), {} },
        { "AUTONOMOUS off", frameChange([](Frame& f) { f.autonomous = false; }), {} },
        // GEOMETRY scales its offsets: with them centred it changes nothing
        { "GEOMETRY 0 -> 1 (Brain), rhythm and mutation up", frameChange([](Frame& f) {
              f.geometryInfluence = 1.0f; f.rhythmDensity = 1.0f; f.geometryMutation = 0.6f; }), {} },
        { "FORM 0 -> 1 (Brain)", frameChange([](Frame& f) { f.formInfluence = 1.0f; }), {} },
        { "STRUCTURE 0 -> 1 (Brain)", frameChange([](Frame& f) { f.structureInfluence = 1.0f; }), {} },
        { "MUSIC 0.6 -> 0", frameChange([](Frame& f) { f.musicInfluence = 0.0f; }), {} },
        { "MUSIC 0.6 -> 1", frameChange([](Frame& f) { f.musicInfluence = 1.0f; }), {} },
        { "SUPPORT 0 dB -> +24 dB", frameChange([](Frame& f) { f.supportPresence = 1.0f; }), {} },
        { "EQ ORDER before -> after (CREATIVE MASTER 1)",
              frameChange([](Frame& f) { f.masterEqAfterDynamics = true; }), { false, true, true } },
    };
    for (const auto& c : cases)
    {
        const auto r = measureChange(c.change, c.setup);
        std::cout << "click " << c.name << ": shows after " << r.delayMs << " ms, early/later x" << r.ratio
                  << ", early high band " << 20.0 * std::log10(std::max(r.jump, 1.0e-9)) << " dBFS\n";
        expect(r.delayMs >= 0.0, std::string("the change is heard: ") + c.name);
        // a click: D whole from the start (early >= half of 30 ms later) AND
        // audible (its high band above -80 dBFS). STRIKER MIX starts new
        // impacts, whose attacks are sounds, not clicks: reported only
        if (std::string(c.name).rfind("STRIKER", 0) != 0)
            expect(!(r.ratio > 0.5 && r.jump > 1.0e-4), std::string("no click on ") + c.name);
    }

    // positive controls: a true instantaneous gain step (x1.5) and a hard cut
    // to silence (x0) on the recorded output; both must read as clicks
    for (const auto step : { 1.5, 1.0e-9 })
    {
        const auto r = measureChange(Change([](Engine&, Frame&) {}), {}, step);
        std::cout << "click control (a true x" << (step < 1.0 ? 0.0 : step) << " step): early/later x" << r.ratio
                  << ", early high band " << 20.0 * std::log10(std::max(r.jump, 1.0e-9)) << " dBFS\n";
        expect(r.ratio > 0.5 && r.jump > 1.0e-4, "the measure detects a real step (positive control)");
    }

    for (const auto headphones : { false, true })
        for (const auto spread : { 1.0f, 0.0f })
        {
            const auto m = measureMono(headphones, spread);
            std::cout << "mono " << (headphones ? "headphones" : "speakers") << ", spread " << spread
                      << ": correlation " << m.correlation << ", mono fold-down " << m.monoLossDb << " dB; below 150 Hz "
                      << m.lowCorrelation << ", " << m.lowMonoLossDb << " dB\n";
            // the bass foundation stays mono-safe with the head and the width on
            expect(m.lowCorrelation > 0.8 && m.lowMonoLossDb > -1.0, "the bass below 150 Hz survives the mono fold-down");
            expect(m.correlation > -0.2, "no strongly anti-phase stereo");
            expect(m.monoLossDb > -6.0, "the mono fold-down loses less than 6 dB");
        }

    if (failures == 0)
        std::cout << "automation: all checks passed\n";
    return failures == 0;
}
