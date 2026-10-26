// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

// The engine under stress (aquorbium-tarefas.md, phase 10, 2026-10-22): the
// worst case - every control a musician can reach at its maximum, the
// synthesis device at full, four voices, headphones - must stay under the
// ceiling measured as TRUE peak by an independent detector (4x oversampling,
// TruePeakDetector), and the engine must survive the stimuli of the list:
// impulse, DC, sub-bass, silence, NaN/Inf, a sine near Nyquist, full-scale
// noise (through the room input, its only audio input), irregular blocks and
// sample-rate changes - finite, bounded, and still alive afterwards.
#include "app/PerformanceSceneControls.h"
#include "core/BiomaEngine.h"
#include "core/SynthProposal.h"
#include "core/TruePeakDetector.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <limits>
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
        std::cerr << "engine stress requirement failed: " << what << '\n';
        ++failures;
    }
}

// what came out: true peak (dBTP, 4x), sample peak, RMS of the last part,
// finiteness
struct Measure
{
    aquorbium::TruePeakDetector left, right;
    float truePeak = 0.0f, samplePeak = 0.0f;
    double tailSum = 0.0;
    std::size_t tailCount = 0;
    bool finite = true;
    void prepare(double rate)
    {
        left.prepare(rate);
        right.prepare(rate);
    }
    void push(float l, float r, bool tail)
    {
        finite = finite && std::isfinite(l) && std::isfinite(r);
        if (!std::isfinite(l) || !std::isfinite(r))
            return;
        samplePeak = std::max({ samplePeak, std::abs(l), std::abs(r) });
        truePeak = std::max({ truePeak, left.processSample(l), right.processSample(r) });
        if (tail)
        {
            tailSum += 0.5 * (static_cast<double>(l) * l + static_cast<double>(r) * r);
            ++tailCount;
        }
    }
    [[nodiscard]] double tailDb() const
    {
        return tailCount == 0 ? -200.0 : 10.0 * std::log10(std::max(tailSum / static_cast<double>(tailCount), 1.0e-20));
    }
};
double db(float value) { return 20.0 * std::log10(std::max(static_cast<double>(value), 1.0e-12)); }

// every parameter of the interface at its maximum, the trim at +6 dB, the
// ceiling given; the ecology dense, agitated, in danger
aquorbium::app::AudioControlFrame everythingAtMaximum(double ceilingControl)
{
    using namespace aquorbium::app;
    PerformanceParameterState state;
    for (const auto& parameter : PerformanceParameterState::parameters)
        static_cast<void>(state.set(parameter.id, 1.0));
    static_cast<void>(state.set("masterCeiling", ceilingControl));
    EcologyParameterState ecology;
    using Key = EcologyParameterState::Key;
    static_cast<void>(ecology.set(Key::density, 1.0));
    static_cast<void>(ecology.set(Key::calm, 0.0));
    static_cast<void>(ecology.set(Key::cohesion, 1.0));
    static_cast<void>(ecology.set(Key::danger, 1.0));
    static_cast<void>(state.setEcology(ecology.snapshot()));
    return sceneControls(state);
}

std::unique_ptr<aquorbium::BiomaEngine> loudEngine(double rate, double ceilingControl, std::uint32_t seed)
{
    auto engine = std::make_unique<aquorbium::BiomaEngine>();
    engine->prepare(rate);
    engine->setSeed(seed);
    everythingAtMaximum(ceilingControl).apply(*engine, true);
    engine->setAutonomousEnabled(true);
    engine->setSynthMix(1.0f);
    for (std::size_t i = 0; i < aquorbium::SynthProposal::organisms; ++i)
    {
        auto patch = aquorbium::SynthProposal::propose(i, seed);
        patch.level = 1.5f;
        patch.drive = 1.0f;
        engine->setSynthPatch(i, patch);
    }
    for (const auto note : { 36, 43, 48, 55 })
        engine->noteOn(note, 1.0f);
    return engine;
}

// the frame's ceiling control (0..1) in dB, as AudioControlFrame maps it
double ceilingDb(double control) { return -6.0 + control * 5.9; }
}

bool runEngineStressTests()
{
    constexpr double rate = 48000.0;
    constexpr std::size_t block = 256;
    std::array<float, 8192> left {}, right {};

    // 1. the worst case, at three ceilings: the true peak stays under it
    for (const auto control : { 0.0, 5.0 / 5.9, 1.0 })
    {
        for (const auto headphones : { false, true })
        {
            auto engine = loudEngine(rate, control, 0x5eedu + static_cast<std::uint32_t>(control * 10.0));
            engine->setHeadphones(headphones);
            Measure m;
            m.prepare(rate);
            const auto seconds = 30.0;
            for (std::size_t done = 0; done < static_cast<std::size_t>(rate * seconds); done += block)
            {
                engine->render(left.data(), right.data(), block);
                for (std::size_t i = 0; i < block; ++i)
                    m.push(left[i], right[i], done > static_cast<std::size_t>(rate * 5.0));
            }
            const auto ceiling = ceilingDb(control);
            std::cout << "worst case, ceiling " << ceiling << " dB" << (headphones ? ", headphones" : ", speakers")
                      << ": true peak " << db(m.truePeak) << " dBTP, sample peak " << db(m.samplePeak)
                      << " dBFS, RMS " << m.tailDb() << " dBFS\n";
            expect(m.finite, "worst case stays finite");
            expect(db(m.samplePeak) <= ceiling + 0.05, "worst case: the sample peak under the ceiling");
            expect(db(m.truePeak) <= ceiling + 0.5, "worst case: the true peak (4x) under the ceiling (+0.5 dB)");
            expect(m.tailDb() > -40.0, "worst case: it really is loud (RMS above -40 dBFS)");
        }
    }

    // 2. the stimuli, through the room input (ROOM on)
    struct Stimulus
    {
        const char* name;
        float (*sample)(std::size_t n);
    };
    const std::array<Stimulus, 9> stimuli {{
        { "silence", [](std::size_t) { return 0.0f; } },
        { "impulse", [](std::size_t n) { return n % 48000 == 0 ? 1.0f : 0.0f; } },
        { "DC +1", [](std::size_t) { return 1.0f; } },
        { "sub-bass 20 Hz full scale", [](std::size_t n) { return std::sin(6.2831853f * 20.0f * static_cast<float>(n) / 48000.0f); } },
        { "sine near Nyquist (23.5 kHz)", [](std::size_t n) { return std::sin(6.2831853f * 23500.0f * static_cast<float>(n) / 48000.0f); } },
        { "full-scale noise", [](std::size_t n) {
              auto x = static_cast<std::uint32_t>(n * 2654435761u) ^ 0x9e3779b9u;
              x ^= x << 13; x ^= x >> 17; x ^= x << 5;
              return static_cast<float>(x & 0xffffffu) / 8388608.0f - 1.0f; } },
        { "NaN", [](std::size_t) { return std::numeric_limits<float>::quiet_NaN(); } },
        { "+Inf", [](std::size_t) { return std::numeric_limits<float>::infinity(); } },
        { "-Inf and huge values", [](std::size_t n) {
              return n % 2 == 0 ? -std::numeric_limits<float>::infinity() : 3.0e38f; } },
    }};
    std::array<float, block> microphone {};
    for (const auto& stimulus : stimuli)
    {
        auto engine = loudEngine(rate, 5.0 / 5.9, 0xabcu);
        engine->setRoomCouplingEnabled(true);
        Measure m;
        m.prepare(rate);
        std::size_t n = 0;
        // 6 s of the stimulus, then 4 s of silence: the engine must still sing
        for (std::size_t done = 0; done < static_cast<std::size_t>(rate * 10.0); done += block)
        {
            const auto stimulusOn = done < static_cast<std::size_t>(rate * 6.0);
            for (auto& x : microphone)
                x = stimulusOn ? stimulus.sample(n++) : 0.0f;
            engine->setRoomInput(microphone.data(), microphone.size());
            engine->render(left.data(), right.data(), block);
            for (std::size_t i = 0; i < block; ++i)
                m.push(left[i], right[i], done > static_cast<std::size_t>(rate * 7.0));
        }
        std::cout << "stimulus " << stimulus.name << ": true peak " << db(m.truePeak) << " dBTP, after it RMS "
                  << m.tailDb() << " dBFS\n";
        expect(m.finite, std::string("finite under ") + stimulus.name);
        expect(db(m.truePeak) <= ceilingDb(5.0 / 5.9) + 0.5, std::string("bounded under ") + stimulus.name);
        expect(m.tailDb() > -50.0, std::string("still sounding after ") + stimulus.name);
    }

    // 3. irregular blocks: 1, 3, 7, 64, 255, 1000, 4096 in turn
    {
        auto engine = loudEngine(rate, 5.0 / 5.9, 0x77u);
        Measure m;
        m.prepare(rate);
        constexpr std::array<std::size_t, 7> sizes { 1, 3, 7, 64, 255, 1000, 4096 };
        std::size_t done = 0, k = 0;
        while (done < static_cast<std::size_t>(rate * 20.0))
        {
            const auto size = sizes[k++ % sizes.size()];
            engine->render(left.data(), right.data(), size);
            for (std::size_t i = 0; i < size; ++i)
                m.push(left[i], right[i], done > static_cast<std::size_t>(rate * 5.0));
            done += size;
        }
        std::cout << "irregular blocks: true peak " << db(m.truePeak) << " dBTP, RMS " << m.tailDb() << " dBFS\n";
        expect(m.finite && db(m.truePeak) <= ceilingDb(5.0 / 5.9) + 0.5 && m.tailDb() > -40.0,
               "irregular blocks: finite, bounded, sounding");
    }

    // 4. sample-rate changes while playing: 48 > 44.1 > 96 > 192 > 22.05 > 48 kHz
    {
        auto engine = loudEngine(rate, 5.0 / 5.9, 0x99u);
        for (const auto r : { 48000.0, 44100.0, 96000.0, 192000.0, 22050.0, 48000.0 })
        {
            engine->prepare(r);
            Measure m;
            m.prepare(r);
            for (std::size_t done = 0; done < static_cast<std::size_t>(r * 4.0); done += block)
            {
                engine->render(left.data(), right.data(), block);
                for (std::size_t i = 0; i < block; ++i)
                    m.push(left[i], right[i], done > static_cast<std::size_t>(r * 1.0));
            }
            std::cout << "rate " << r << ": true peak " << db(m.truePeak) << " dBTP, RMS " << m.tailDb() << " dBFS\n";
            expect(m.finite && db(m.truePeak) <= ceilingDb(5.0 / 5.9) + 0.5,
                   "rate change to " + std::to_string(static_cast<int>(r)) + ": finite and bounded");
            expect(m.tailDb() > -60.0, "rate change to " + std::to_string(static_cast<int>(r)) + ": still sounding");
        }
    }

    if (failures == 0)
        std::cout << "engine stress: all checks passed\n";
    return failures == 0;
}
