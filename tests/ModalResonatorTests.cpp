// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/BiomaModalResonator.h"

#include <algorithm>
#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <vector>

namespace
{
bool approximately(float left, float right, float tolerance = 1.0e-3f)
{
    return std::abs(left - right) <= tolerance;
}

std::vector<float> render(std::uint32_t seed,
                          aquorbium::BiomaModalResonator::Parameters parameters,
                          std::size_t samples, int repeatedExcitations = 1)
{
    aquorbium::BiomaModalResonator resonator;
    resonator.prepare(48000.0);
    resonator.setSeed(seed);
    resonator.setParameters(parameters);
    std::vector<float> result(samples);
    for (std::size_t sample = 0; sample < samples; ++sample)
    {
        if (sample == 0
            || (repeatedExcitations > 1
                && sample % (samples / static_cast<std::size_t>(repeatedExcitations)) == 0))
            resonator.excite(0.84f);
        result[sample] = resonator.tick();
    }
    return result;
}

bool modeGeometryIsDefined()
{
    aquorbium::BiomaModalResonator resonator;
    resonator.prepare(48000.0);
    aquorbium::BiomaModalResonator::Parameters parameters;
    parameters.fundamentalHz = 100.0f;
    parameters.material = 0.0f;
    resonator.setParameters(parameters);
    auto telemetry = resonator.telemetry();
    const std::array<float, 4> harmonic { 100.0f, 200.0f, 300.0f, 400.0f };
    for (std::size_t index = 0; index < harmonic.size(); ++index)
        if (!approximately(telemetry.modeFrequencies[index], harmonic[index])) return false;

    parameters.material = 1.0f;
    resonator.setParameters(parameters);
    telemetry = resonator.telemetry();
    const std::array<float, 4> membrane { 100.0f, 159.33f, 213.55f, 229.54f };
    for (std::size_t index = 0; index < membrane.size(); ++index)
    {
        if (!approximately(telemetry.modeFrequencies[index], membrane[index], 0.02f)
            || !(telemetry.poleRadii[index] > 0.0f && telemetry.poleRadii[index] < 1.0f))
            return false;
    }
    return true;
}

bool silenceAndTailAreDefined()
{
    aquorbium::BiomaModalResonator resonator;
    resonator.prepare(48000.0);
    for (int sample = 0; sample < 4096; ++sample)
        if (resonator.tick() != 0.0f) return false;

    auto parameters = resonator.parameters();
    parameters.decaySeconds = 0.08f;
    parameters.hardness = 1.0f;
    resonator.setParameters(parameters);
    resonator.excite(1.0f);
    auto nonZeroSamples = 0;
    for (int sample = 0; sample < 48000 * 3; ++sample)
        if (resonator.tick() != 0.0f) ++nonZeroSamples;
    return nonZeroSamples > 100 && resonator.tick() == 0.0f
        && !resonator.telemetry().active;
}

bool deterministicAndSeeded()
{
    aquorbium::BiomaModalResonator::Parameters parameters;
    parameters.fundamentalHz = 173.0f;
    parameters.material = 0.73f;
    parameters.decaySeconds = 0.9f;
    parameters.brightness = 0.64f;
    parameters.hardness = 0.31f;
    parameters.strikePosition = 0.27f;
    const auto first = render(0x918ab42u, parameters, 48000, 5);
    const auto repeated = render(0x918ab42u, parameters, 48000, 5);
    const auto anotherSeed = render(0x41c29e7u, parameters, 48000, 5);
    if (first.size() != repeated.size()) return false;
    auto seedDiffers = false;
    for (std::size_t sample = 0; sample < first.size(); ++sample)
    {
        if (std::bit_cast<std::uint32_t>(first[sample])
            != std::bit_cast<std::uint32_t>(repeated[sample]))
            return false;
        if (std::bit_cast<std::uint32_t>(first[sample])
            != std::bit_cast<std::uint32_t>(anotherSeed[sample]))
            seedDiffers = true;
    }
    return seedDiffers;
}

bool boundedUnderRepeatedExcitation()
{
    aquorbium::BiomaModalResonator::Parameters parameters;
    parameters.fundamentalHz = 1200.0f;
    parameters.material = 0.9f;
    parameters.decaySeconds = 8.0f;
    parameters.brightness = 1.0f;
    parameters.hardness = 0.0f;
    parameters.strikePosition = 0.51f;
    const auto samples = render(77u, parameters, 48000 * 4, 200);
    auto peak = 0.0f;
    auto maximumDelta = 0.0f;
    for (std::size_t sample = 0; sample < samples.size(); ++sample)
    {
        if (!std::isfinite(samples[sample])) return false;
        peak = std::max(peak, std::abs(samples[sample]));
        if (sample > 0)
            maximumDelta = std::max(maximumDelta,
                                    std::abs(samples[sample] - samples[sample - 1]));
    }
    return peak > 1.0e-4f && peak < 0.91f && maximumDelta < 0.45f;
}

bool invalidParametersPreserveState()
{
    aquorbium::BiomaModalResonator reference;
    aquorbium::BiomaModalResonator challenged;
    aquorbium::BiomaModalResonator::Parameters parameters;
    parameters.fundamentalHz = 287.0f;
    parameters.material = 0.39f;
    parameters.decaySeconds = 2.1f;
    parameters.brightness = 0.71f;
    parameters.hardness = 0.82f;
    parameters.strikePosition = 0.63f;
    for (auto* resonator : { &reference, &challenged })
    {
        resonator->prepare(48000.0);
        resonator->setSeed(91u);
        resonator->setParameters(parameters);
    }
    auto invalid = parameters;
    invalid.fundamentalHz = std::numeric_limits<float>::quiet_NaN();
    invalid.material = std::numeric_limits<float>::infinity();
    invalid.decaySeconds = -std::numeric_limits<float>::infinity();
    invalid.brightness = std::numeric_limits<float>::quiet_NaN();
    invalid.hardness = std::numeric_limits<float>::infinity();
    invalid.strikePosition = std::numeric_limits<float>::quiet_NaN();
    challenged.setParameters(invalid);
    reference.excite(0.7f);
    challenged.excite(0.7f);
    for (int sample = 0; sample < 16000; ++sample)
        if (std::bit_cast<std::uint32_t>(reference.tick())
            != std::bit_cast<std::uint32_t>(challenged.tick()))
            return false;
    challenged.excite(std::numeric_limits<float>::quiet_NaN());
    return challenged.telemetry().excitationCount == 1;
}

bool performanceIntentIsHybrid()
{
    aquorbium::BiomaModalResonator::Parameters parameters;
    parameters.fundamentalHz = 196.0f;
    parameters.material = 0.18f;
    parameters.decaySeconds = 0.42f;
    parameters.brightness = 0.31f;
    parameters.hardness = 0.26f;
    parameters.strikePosition = 0.22f;

    aquorbium::BiomaModalResonator reference;
    aquorbium::BiomaModalResonator ignored;
    aquorbium::BiomaModalResonator performed;
    for (auto* resonator : { &reference, &ignored, &performed })
    {
        resonator->prepare(48000.0);
        resonator->setSeed(0x721u);
        resonator->setParameters(parameters);
    }
    aquorbium::BiomaModalResonator::PerformanceIntent intent;
    intent.influence = 0.0f;
    intent.material = 1.0f;
    intent.decaySeconds = 4.0f;
    intent.brightness = 1.0f;
    intent.hardness = 1.0f;
    intent.strikePosition = 0.81f;
    ignored.setPerformanceIntent(intent);
    intent.influence = 1.0f;
    performed.setPerformanceIntent(intent);
    reference.excite(0.8f);
    ignored.excite(0.8f);
    performed.excite(0.8f);

    auto performedDiffers = false;
    for (int sample = 0; sample < 48000; ++sample)
    {
        const auto referenceSample = reference.tick();
        const auto ignoredSample = ignored.tick();
        const auto performedSample = performed.tick();
        if (std::bit_cast<std::uint32_t>(referenceSample)
            != std::bit_cast<std::uint32_t>(ignoredSample))
            return false;
        if (std::bit_cast<std::uint32_t>(referenceSample)
            != std::bit_cast<std::uint32_t>(performedSample))
            performedDiffers = true;
        if (!std::isfinite(performedSample) || std::abs(performedSample) >= 0.91f)
            return false;
    }
    const auto effective = performed.telemetry().parameters;
    const auto valid = performedDiffers && approximately(effective.material, 1.0f)
        && approximately(effective.decaySeconds, 4.0f)
        && approximately(effective.brightness, 1.0f)
        && approximately(effective.hardness, 1.0f)
        && approximately(effective.strikePosition, 0.81f);
    if (!valid)
        std::cerr << "performedDiffers=" << performedDiffers
                  << " material=" << effective.material
                  << " decay=" << effective.decaySeconds
                  << " brightness=" << effective.brightness
                  << " hardness=" << effective.hardness
                  << " strike=" << effective.strikePosition << '\n';
    return valid;
}
}

int main()
{
    if (!modeGeometryIsDefined())
    {
        std::cerr << "modal ratios or pole stability failed\n";
        return EXIT_FAILURE;
    }
    if (!silenceAndTailAreDefined())
    {
        std::cerr << "resonator silence/tail contract failed\n";
        return EXIT_FAILURE;
    }
    if (!deterministicAndSeeded())
    {
        std::cerr << "resonator determinism/seed contract failed\n";
        return EXIT_FAILURE;
    }
    if (!boundedUnderRepeatedExcitation())
    {
        std::cerr << "resonator repeated excitation exceeded limits\n";
        return EXIT_FAILURE;
    }
    if (!invalidParametersPreserveState())
    {
        std::cerr << "resonator numeric safety failed\n";
        return EXIT_FAILURE;
    }
    if (!performanceIntentIsHybrid())
    {
        std::cerr << "resonator performance/autonomy blend failed\n";
        return EXIT_FAILURE;
    }
    std::cout << "aquorbium modal resonator tests passed\n";
}
