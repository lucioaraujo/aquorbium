// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/EcologicalNoiseBed.h"
#include "core/SchoolSpectralBank.h"

#include <algorithm>
#include <bit>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <utility>

namespace
{
bool bankMapsCentreAndSpan()
{
    aquorbium::SchoolSpectralBank bank;
    bank.prepare(48000.0);
    bank.setParameters({ 0.35f, 0.1f, 0.4f });
    const auto narrow = bank.telemetry();
    bank.setParameters({ 0.65f, 0.9f, 0.4f });
    const auto wide = bank.telemetry();
    for (std::size_t index = 1; index < bank.bandCount; ++index)
        if (!(narrow.frequencyHz[index] > narrow.frequencyHz[index - 1])
            || !(wide.frequencyHz[index] > wide.frequencyHz[index - 1]))
            return false;
    const auto narrowRatio = narrow.frequencyHz.back() / narrow.frequencyHz.front();
    const auto wideRatio = wide.frequencyHz.back() / wide.frequencyHz.front();
    return wide.frequencyHz[bank.bandCount / 2]
            > narrow.frequencyHz[bank.bandCount / 2]
        && wideRatio > narrowRatio * 4.0f;
}

bool bankIdentityIsStableAndFinite()
{
    aquorbium::SchoolSpectralBank first, repeated, other;
    for (auto* bank : { &first, &repeated, &other })
        bank->prepare(48000.0);
    first.setIdentity(7u, 91u);
    repeated.setIdentity(7u, 91u);
    other.setIdentity(8u, 91u);
    const auto a = first.telemetry();
    const auto b = repeated.telemetry();
    const auto c = other.telemetry();
    auto different = false;
    for (std::size_t index = 0; index < first.bandCount; ++index)
    {
        if (std::bit_cast<std::uint32_t>(a.gain[index])
            != std::bit_cast<std::uint32_t>(b.gain[index]))
            return false;
        different = different || a.gain[index] != c.gain[index];
    }
    auto peak = 0.0f;
    for (int sample = 0; sample < 48000; ++sample)
    {
        const auto output = first.tick(sample == 0 ? 1.0f : 0.0f);
        if (!std::isfinite(output.mix))
            return false;
        peak = std::max(peak, std::abs(output.mix));
    }
    return different && peak > 0.0f && peak < 2.0f;
}

bool noiseIsDeterministicAndBounded()
{
    aquorbium::EcologicalNoiseBed first, repeated;
    for (auto* noise : { &first, &repeated })
    {
        noise->prepare(48000.0);
        noise->setSeed(0x7129au);
        noise->setParameters({ 0.35f, 0.72f, 0.4f, 0.65f, 0.0f, 0.5f });
    }
    for (int sample = 0; sample < 20000; ++sample)
    {
        const auto a = first.tick();
        const auto b = repeated.tick();
        if (std::bit_cast<std::uint32_t>(a.left)
                != std::bit_cast<std::uint32_t>(b.left)
            || std::bit_cast<std::uint32_t>(a.right)
                != std::bit_cast<std::uint32_t>(b.right)
            || !std::isfinite(a.left) || !std::isfinite(a.right)
            || std::abs(a.left) > 0.5f || std::abs(a.right) > 0.5f)
            return false;
    }
    return true;
}

bool ecologyChangesMorphColourAndResolution()
{
    aquorbium::EcologicalNoiseBed noise;
    noise.prepare(48000.0);
    noise.setParameters({ 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.25f });
    const auto dark = noise.telemetry();
    noise.setParameters({ 0.5f, 0.5f, 0.0f, 1.0f, 1.0f, 0.25f });
    const auto brightBroken = noise.telemetry();
    if (!(brightBroken.colourCutoffHz > dark.colourCutoffHz * 20.0f)
        || brightBroken.morphPosition == dark.morphPosition
        || dark.holdSamples != 1u || brightBroken.holdSamples != 64u)
        return false;

    noise.reset();
    auto previous = noise.tick();
    for (std::size_t sample = 1; sample < brightBroken.holdSamples; ++sample)
    {
        const auto current = noise.tick();
        if (current.left != previous.left || current.right != previous.right)
            return false;
    }
    const auto changed = noise.tick();
    return changed.left != previous.left || changed.right != previous.right;
}

bool invalidParametersAreRetained()
{
    aquorbium::SchoolSpectralBank bank;
    bank.prepare(48000.0);
    bank.setParameters({ 0.25f, 0.75f, 0.6f });
    bank.setParameters({ std::numeric_limits<float>::quiet_NaN(), 4.0f,
                         -4.0f });
    const auto bankParameters = bank.parameters();

    aquorbium::EcologicalNoiseBed noise;
    noise.prepare(48000.0);
    noise.setParameters({ 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.3f });
    noise.setParameters({ std::numeric_limits<float>::infinity(),
                          std::numeric_limits<float>::quiet_NaN(), -2.0f,
                          4.0f, 9.0f, 2.0f });
    const auto noiseParameters = noise.parameters();
    return bankParameters.centre == 0.25f && bankParameters.span == 1.0f
        && bankParameters.resonance == 0.0f
        && noiseParameters.seasonPhase == 0.2f
        && noiseParameters.dayPhase == 0.3f
        && noiseParameters.turbidity == 0.0f
        && noiseParameters.temperature == 1.0f
        && noiseParameters.crackStress == 1.0f
        && noiseParameters.level == 0.5f;
}
}

int main()
{
    const std::pair<const char*, bool (*)()> cases[] {
        { "school mapping", bankMapsCentreAndSpan },
        { "school identity", bankIdentityIsStableAndFinite },
        { "noise determinism", noiseIsDeterministicAndBounded },
        { "ecological mapping", ecologyChangesMorphColourAndResolution },
        { "parameter limits", invalidParametersAreRetained },
    };
    for (const auto& [name, test] : cases)
    {
        if (!test())
        {
            std::cerr << name << " failed\n";
            return EXIT_FAILURE;
        }
    }
    std::cout << "Spectral environment tests passed\n";
}
