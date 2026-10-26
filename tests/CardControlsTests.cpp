// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

// The LUPA, MASTER CRIATIVO and CÉREBRO controls (2026-10-24): at their
// centre the instrument is the classic one, bit for bit; away from it each
// group changes the sound.
#include "app/AudioControlFrame.h"
#include "core/BiomaEngine.h"

#include <array>
#include <bit>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
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

std::vector<float> render(const std::function<void(aquorbium::app::AudioControlFrame&)>& edit, bool applyFrame = true)
{
    auto engine = std::make_unique<aquorbium::BiomaEngine>();
    engine->prepare(48000.0);
    engine->setSeed(0xca2du);
    aquorbium::app::AudioControlFrame frame;
    frame.lensHeld = true;  // the lens on the glass, so its shape matters
    frame.creativeMaster = 1.0f;
    if (edit)
        edit(frame);
    if (applyFrame)
        frame.apply(*engine, true);
    engine->setAutonomousEnabled(true);
    engine->noteOn(48, 0.8f);
    std::array<float, 512> l {}, r {};
    std::vector<float> out;
    for (int block = 0; block < 48000 * 20 / 512; ++block)
    {
        engine->render(l.data(), r.data(), l.size());
        out.insert(out.end(), l.begin(), l.end());
    }
    return out;
}

bool identical(const std::vector<float>& a, const std::vector<float>& b)
{
    if (a.size() != b.size())
        return false;
    for (std::size_t i = 0; i < a.size(); ++i)
        if (std::bit_cast<std::uint32_t>(a[i]) != std::bit_cast<std::uint32_t>(b[i]))
            return false;
    return true;
}
}

int main()
{
    const auto base = render({});
    // the centre is classic: the same frame with the cards' values set
    // explicitly to their defaults gives the very same samples
    const auto explicitDefaults = render([](auto& f) {
        f.cards = { 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.0f, 0.5f, 0.5f };
    });
    check(identical(base, explicitDefaults), "the cards at their centre change nothing");

    struct Case { const char* name; std::size_t first, count; float value; };
    const std::array<Case, 4> cases {{
        { "LENS (radius, reach, zoom, blur)", 0, 4, 0.9f },
        { "CREATIVE MASTER (multiband, glue, tilt, width)", 4, 4, 0.95f },
        { "BRAIN clock", 8, 1, 1.0f },
        { "BRAIN coordination", 9, 1, 1.0f },
    }};
    for (const auto& c : cases)
    {
        const auto moved = render([&c](auto& f) {
            for (std::size_t i = c.first; i < c.first + c.count; ++i)
                f.cards[i] = c.value;
        });
        double difference = 0.0, energy = 0.0;
        for (std::size_t i = 0; i < base.size(); ++i)
        {
            difference += static_cast<double>(moved[i] - base[i]) * (moved[i] - base[i]);
            energy += static_cast<double>(base[i]) * base[i];
        }
        const auto db = 10.0 * std::log10(std::max(difference, 1.0e-30) / std::max(energy, 1.0e-30));
        std::cout << c.name << ": the difference is " << db << " dB re the sound\n";
        check(db > -60.0, std::string(c.name) + " changes the sound");
    }
    if (failures == 0)
        std::cout << "card controls: all checks passed\n";
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
