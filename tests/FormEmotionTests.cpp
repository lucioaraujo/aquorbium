// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

// The emotion of the form (2026-10-24): the session's arc of tension and the
// chord's own tension move the events and the dynamics; the melody breathes
// after a cadence; inner voices hold a note into a new chord and resolve it
// by step. Measured over eight minutes of a generated world.
#include "core/BiomaEngine.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
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
}

int main()
{
    const auto seed = 1899755276u;
    const auto world = aquorbium::SessionWorld::fromSeed(seed);
    auto engine = std::make_unique<aquorbium::BiomaEngine>();
    engine->setSessionWorld(world);
    engine->prepare(48000.0);
    engine->setSeed(seed);
    engine->setAutonomousEnabled(true);
    std::array<float, 512> l {}, r {};
    struct Second { double tension = 0.0, events = 0.0, power = 0.0; int blocks = 0; };
    std::vector<Second> seconds(480);
    std::uint64_t lastEvents = 0;
    int breathing = 0, silenced = 0, held = 0, resolved = 0;
    std::uint64_t answers = 0, returns = 0;
    std::array<float, 5> last {}, holding {};
    last.fill(-1.0f);
    holding.fill(-1.0f);
    for (int block = 0; block < 48000 * 480 / 512; ++block)
    {
        engine->render(l.data(), r.data(), l.size());
        const auto t = engine->telemetry();
        const auto& a = t.autonomy;
        auto& s = seconds[static_cast<std::size_t>(block * 512 / 48000)];
        double power = 0.0;
        for (std::size_t i = 0; i < l.size(); ++i)
            power += 0.5 * (static_cast<double>(l[i]) * l[i] + static_cast<double>(r[i]) * r[i]);
        s.tension += a.tension;
        s.events += static_cast<double>(a.eventCount - lastEvents);
        s.power += power / static_cast<double>(l.size());
        ++s.blocks;
        lastEvents = a.eventCount;
        answers = a.responsesSung;
        returns = a.returnsSung;
        if (a.breathing)
        {
            ++breathing;
            silenced += a.voices[a.melodyLane].targetEnergy == 0.0f ? 1 : 0;
        }
        for (std::size_t i = 0; i < 5; ++i)
        {
            const auto p = a.voices[i].targetMidiPitch;
            const auto relative = static_cast<int>(std::lround(p)) - (36 + world.rootOffset + a.keyOffset);
            if (p != last[i])
            {
                if (holding[i] >= 0.0f)
                {
                    resolved += std::abs(p - holding[i]) <= 2.01f && a.chord.contains(relative) ? 1 : 0;
                    holding[i] = -1.0f;
                }
                last[i] = p;
            }
            else if (i != a.melodyLane && holding[i] < 0.0f && a.chord.layerOffset == 0 && !a.chord.contains(relative)
                     && !(a.motifSounding && i == a.motifLane))  // the motif's notes are the mode's, by design
            {
                holding[i] = p;
                ++held;
            }
        }
    }
    std::vector<std::pair<double, std::pair<double, double>>> rows;
    for (const auto& s : seconds)
        if (s.blocks > 0)
            rows.push_back({ s.tension / s.blocks, { s.events, s.power / s.blocks } });
    std::sort(rows.begin(), rows.end());
    const auto quarter = rows.size() / 4;
    double calmEvents = 0.0, tenseEvents = 0.0, calmPower = 0.0, tensePower = 0.0;
    for (std::size_t i = 0; i < quarter; ++i)
    {
        calmEvents += rows[i].second.first;
        calmPower += rows[i].second.second;
        tenseEvents += rows[rows.size() - 1 - i].second.first;
        tensePower += rows[rows.size() - 1 - i].second.second;
    }
    const auto levelDifference = 10.0 * std::log10(tensePower / std::max(calmPower, 1.0e-20));
    std::cout << "events, calm vs tense quarter: " << calmEvents / quarter << " / " << tenseEvents / quarter
              << " a second; level difference " << levelDifference << " dB; breathing " << breathing
              << " blocks, melody silent in " << silenced << "; held " << held << ", resolved by step " << resolved << '\n';
    check(tenseEvents > calmEvents * 1.15, "tension brings more events");
    check(levelDifference > 2.5, "tension is louder than repose");
    check(breathing > 0 && silenced == breathing, "the melody breathes after a cadence");
    check(held > 0 && resolved * 10 >= held * 7, "held notes resolve by step into the chord");
    // the motif (2026-10-24): after a cadence another voice answers with the
    // melody's phrase transformed; every second cycle the melody brings it back
    std::cout << "motif notes sung: " << answers << " in answers, " << returns << " in returns\n";
    check(answers >= 20, "another voice answers the melody's phrase");
    check(returns >= 6, "the melody brings its motif back");
    if (failures == 0)
        std::cout << "form emotion: all checks passed\n";
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
