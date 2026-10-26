// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

// The gain budget (2026-10-22; OUTPUT_STAGE_DESIGN.md): each species alone
// and the sum, measured before the creative MASTER (the pre-master tap), with
// the ecological music at 0.6 and the density at its maximum. It pins what
// was measured, so that a change of balance is seen, not discovered:
//   - the summing point keeps at least 10 dB of headroom (true peak);
//   - the two foreground species sit within 3 LU of each other, and so do
//     the three support species;
//   - the gap foreground - support is reported (22-26 LU when measured; whether
//     it should be smaller is the author's decision, see the tasks).
#include "core/BiomaEngine.h"
#include "core/LoudnessMeter.h"
#include "core/TruePeakDetector.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <vector>

namespace
{
using Species = aquorbium::Ecosystem::Species;
constexpr std::array<Species, 5> species { Species::Oscillator, Species::Pulsar, Species::GranularStriker,
                                           Species::Resonator, Species::PhysicalString };
int failures = 0;
void check(bool condition, const char* what)
{
    if (!condition)
    {
        std::cerr << "FAILED: " << what << '\n';
        ++failures;
    }
}

struct Level
{
    double lufs = -70.0, truePeakDb = -200.0;
    double energyDb = -200.0;  // the organisms' stem, mean square: linear in a gain
};

Level measure(int solo, float support = 1.0f)
{
    auto engine = std::make_unique<aquorbium::BiomaEngine>();
    engine->prepare(48000.0);
    engine->setSeed(0x5eedu);
    engine->setEcosystemBlendImmediately(1.0f);
    engine->setAutonomousEnabled(true);
    engine->setDensity(1.0f);
    engine->setEcologicalMusicPerformance({ 0.6f, 0.6f, 0.5f, 0.5f, 1.0f });
    engine->setSupportPresence(support);
    for (std::size_t k = 0; k < species.size(); ++k)
        engine->setSpeciesGain(species[k], solo < 0 || solo == static_cast<int>(k) ? 1.0f : 0.0f);
    aquorbium::LoudnessMeter meter;
    meter.prepare(48000.0);
    aquorbium::TruePeakDetector left, right;
    left.prepare(48000.0);
    right.prepare(48000.0);
    float peak = 0.0f;
    double energy = 0.0;
    long count = 0;
    std::vector<float> l(256), r(256), tapL(256), tapR(256);
    std::array<std::vector<float>, aquorbium::BiomaEngine::stemCount> stemL, stemR;
    for (auto& v : stemL) v.assign(256, 0.0f);
    for (auto& v : stemR) v.assign(256, 0.0f);
    for (int block = 0; block < 48000 * 35 / 256; ++block)
    {
        engine->setRecordTap(aquorbium::BiomaEngine::RecordTap::preMasterCreative, tapL.data(), tapR.data(), tapL.size());
        aquorbium::BiomaEngine::StemBuffers stems;
        for (std::size_t s = 0; s < stems.left.size(); ++s)
        {
            stems.left[s] = stemL[s].data();
            stems.right[s] = stemR[s].data();
        }
        stems.capacity = 256;
        engine->setStems(stems);
        engine->render(l.data(), r.data(), l.size());
        if (block > 48000 * 5 / 256)
            for (std::size_t i = 0; i < tapL.size(); ++i)
            {
                meter.push(tapL[i], tapR[i]);
                energy += 0.5 * (static_cast<double>(stemL[0][i]) * stemL[0][i] + static_cast<double>(stemR[0][i]) * stemR[0][i]);
                ++count;
                peak = std::max({ peak, left.processSample(tapL[i]), right.processSample(tapR[i]) });
            }
    }
    return { meter.readings().integratedLufs, 20.0 * std::log10(std::max(peak, 1.0e-9f)),
             10.0 * std::log10(std::max(energy / static_cast<double>(std::max(count, 1L)), 1.0e-30)) };
}
}

int main()
{
    const auto sum = measure(-1);
    std::array<Level, 5> alone {};
    for (int k = 0; k < 5; ++k)
        alone[static_cast<std::size_t>(k)] = measure(k);
    const char* names[] { "MORPH (oscillator)", "PULSAR", "GRAIN (striker)", "MODAL (resonator)", "STRING" };
    std::cout << "the sum: " << sum.lufs << " LUFS, " << sum.truePeakDb << " dBTP before the creative MASTER\n";
    for (std::size_t k = 0; k < alone.size(); ++k)
        std::cout << "  " << names[k] << " alone: " << alone[k].lufs << " LUFS, " << alone[k].truePeakDb << " dBTP\n";
    const auto foreground = (alone[0].lufs + alone[1].lufs) / 2.0;
    const auto support = (alone[2].lufs + alone[3].lufs + alone[4].lufs) / 3.0;
    std::cout << "foreground - support: " << foreground - support << " LU\n";
    check(sum.truePeakDb <= -10.0, "the summing point keeps 10 dB of headroom before the MASTER");
    check(std::abs(alone[0].lufs - alone[1].lufs) <= 3.0, "the foreground species within 3 LU of each other");
    const auto supportSpread = std::max({ alone[2].lufs, alone[3].lufs, alone[4].lufs })
        - std::min({ alone[2].lufs, alone[3].lufs, alone[4].lufs });
    check(supportSpread <= 3.0, "the support species within 3 LU of each other");
    check(foreground - support > 0.0 && foreground - support < 30.0,
          "the foreground above the support, by less than 30 LU (22-26 LU when measured)");
    // SUPPORT (2026-10-23): +20 dB on the support species, on top of their
    // authored gains; the foreground untouched
    {
        const auto strike = measure(2, 10.0f), morph = measure(0, 10.0f);
        // measured on the organisms' stem: the gain is exact there. The mix
        // grows less (+16-17 dB for GRAIN): the SPACE's feedback saturates
        // softly by design, and a sparse species is much of its own echo;
        // its gated LUFS also lets more near-silent blocks in
        std::cout << "SUPPORT +20 dB: GRAIN organisms' stem " << strike.energyDb - alone[2].energyDb << " dB (mix LUFS "
                  << strike.lufs - alone[2].lufs << "), MORPH " << morph.energyDb - alone[0].energyDb << " dB\n";
        check(std::abs((strike.energyDb - alone[2].energyDb) - 20.0) < 0.5, "SUPPORT raises a support species by its gain (+20 dB of energy)");
        check(std::abs(morph.energyDb - alone[0].energyDb) < 0.01, "SUPPORT leaves the foreground untouched");
    }
    if (failures == 0)
        std::cout << "gain budget: all checks passed\n";
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
