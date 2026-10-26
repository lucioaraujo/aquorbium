// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

// The MIDI voices through the heads (2026-10-22): with HEADPHONES, a note
// played from outside is heard from its organisms' directions as the
// autonomous route is - the stereo decorrelates as a head does, at about the
// same level; with SPEAKERS nothing changes.
#include "core/BiomaEngine.h"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <vector>

namespace
{
int failures = 0;
void check(bool condition, const char* what)
{
    if (!condition)
    {
        std::cerr << "FAILED: " << what << '\n';
        ++failures;
    }
}

struct Result
{
    double correlation = 0.0, levelDb = -200.0;
    std::vector<float> samples;
};

Result play(bool headphones)
{
    auto engine = std::make_unique<aquorbium::BiomaEngine>();
    engine->prepare(48000.0);
    engine->setSeed(21u);
    engine->setEcosystemBlendImmediately(1.0f);
    engine->setAutonomousEnabled(false);  // only the MIDI voices
    engine->setHeadphones(headphones);
    engine->noteOn(60, 0.9f);
    engine->noteOn(67, 0.8f);
    std::vector<float> l(256), r(256);
    double ll = 0.0, rr = 0.0, lr = 0.0;
    long count = 0;
    Result result;
    for (int block = 0; block < 48000 * 8 / 256; ++block)
    {
        engine->render(l.data(), r.data(), l.size());
        for (std::size_t i = 0; i < l.size(); ++i)
        {
            result.samples.push_back(l[i]);
            if (block > 48000 / 256)
            {
                ll += static_cast<double>(l[i]) * l[i];
                rr += static_cast<double>(r[i]) * r[i];
                lr += static_cast<double>(l[i]) * r[i];
                ++count;
            }
        }
    }
    result.correlation = lr / std::sqrt(std::max(ll * rr, 1.0e-30));
    result.levelDb = 10.0 * std::log10(std::max((ll + rr) / (2.0 * static_cast<double>(count)), 1.0e-30));
    return result;
}
}

int main()
{
    const auto speakers = play(false), headphones = play(true);
    std::cout << "MIDI voices: speakers correlation " << speakers.correlation << " at " << speakers.levelDb
              << " dB; headphones correlation " << headphones.correlation << " at " << headphones.levelDb << " dB\n";
    check(headphones.correlation < speakers.correlation - 0.05, "with HEADPHONES the MIDI voices go through the heads");
    check(std::abs(headphones.levelDb - speakers.levelDb) < 3.0, "at about the same level (within 3 dB)");
    check(play(false).samples == speakers.samples, "deterministic");
    if (failures == 0)
        std::cout << "MIDI binaural: all checks passed\n";
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
