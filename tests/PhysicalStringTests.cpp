// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/BiomaPhysicalString.h"

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
using PhysicalString = aquorbium::BiomaPhysicalString;

bool approximately(float left, float right, float tolerance = 1.0e-4f)
{
    return std::abs(left - right) <= tolerance;
}

std::vector<float> render(double sampleRate, std::uint32_t seed,
                          PhysicalString::Parameters parameters,
                          std::size_t samples)
{
    PhysicalString string;
    string.prepare(sampleRate);
    string.setSeed(seed);
    string.setParameters(parameters);
    string.excite(0.82f);
    std::vector<float> result(samples);
    for (auto& sample : result)
        sample = string.tick();
    return result;
}

bool silenceAndTailAreDefined()
{
    PhysicalString string;
    string.prepare(48000.0);
    for (int sample = 0; sample < 8192; ++sample)
        if (string.tick() != 0.0f) return false;

    auto parameters = string.parameters();
    parameters.fundamentalHz = 311.3f;
    parameters.decaySeconds = 0.04f;
    parameters.brightness = 0.0f;
    string.setParameters(parameters);
    string.excite(1.0f);
    auto hadSignal = false;
    for (int sample = 0; sample < 48000 * 4; ++sample)
    {
        const auto value = string.tick();
        if (!std::isfinite(value) || std::abs(value) >= 0.91f) return false;
        hadSignal = hadSignal || value != 0.0f;
    }
    for (int sample = 0; sample < 8192; ++sample)
        if (string.tick() != 0.0f) return false;
    return hadSignal && !string.telemetry().active;
}

bool deterministicAndSeeded()
{
    PhysicalString::Parameters parameters;
    parameters.fundamentalHz = 173.37f;
    parameters.decaySeconds = 2.7f;
    parameters.brightness = 0.71f;
    parameters.hardness = 0.63f;
    parameters.pluckPosition = 0.29f;
    const auto first = render(48000.0, 0x917ac3u, parameters, 48000);
    const auto repeated = render(48000.0, 0x917ac3u, parameters, 48000);
    const auto another = render(48000.0, 0x517ac3u, parameters, 48000);
    auto seedDiffers = false;
    for (std::size_t sample = 0; sample < first.size(); ++sample)
    {
        if (std::bit_cast<std::uint32_t>(first[sample])
            != std::bit_cast<std::uint32_t>(repeated[sample]))
            return false;
        if (std::bit_cast<std::uint32_t>(first[sample])
            != std::bit_cast<std::uint32_t>(another[sample]))
            seedDiffers = true;
    }
    return seedDiffers;
}

float estimatePeriod(const std::vector<float>& samples, float expectedPeriod,
                     std::size_t begin, std::size_t count)
{
    const auto center = static_cast<int>(std::round(expectedPeriod));
    auto bestLag = center;
    auto bestCorrelation = -1.0;
    for (int lag = std::max(2, center - 4); lag <= center + 4; ++lag)
    {
        double product = 0.0;
        double leftEnergy = 0.0;
        double rightEnergy = 0.0;
        for (std::size_t index = begin + static_cast<std::size_t>(lag);
             index < begin + count; ++index)
        {
            const auto left = static_cast<double>(samples[index]);
            const auto right = static_cast<double>(
                samples[index - static_cast<std::size_t>(lag)]);
            product += left * right;
            leftEnergy += left * left;
            rightEnergy += right * right;
        }
        const auto denominator = std::sqrt(leftEnergy * rightEnergy);
        const auto correlation = denominator > 0.0 ? product / denominator : -1.0;
        if (correlation > bestCorrelation)
        {
            bestCorrelation = correlation;
            bestLag = lag;
        }
    }
    return static_cast<float>(bestLag);
}

bool fractionalTuningIsAccurate()
{
    for (const auto rate : { 48000.0, 96000.0 })
    {
        for (const auto frequency : { 82.41f, 173.37f, 441.17f })
        {
            PhysicalString::Parameters parameters;
            parameters.fundamentalHz = frequency;
            parameters.decaySeconds = 8.0f;
            parameters.brightness = 0.84f;
            parameters.hardness = 0.75f;
            parameters.pluckPosition = 0.19f;
            const auto sampleCount = static_cast<std::size_t>(rate * 1.4);
            const auto samples = render(rate, 0x48a2u, parameters, sampleCount);
            const auto expectedPeriod = static_cast<float>(rate) / frequency;
            const auto period = estimatePeriod(
                samples, expectedPeriod, static_cast<std::size_t>(rate * 0.35),
                static_cast<std::size_t>(rate * 0.8));
            const auto measuredFrequency = static_cast<float>(rate) / period;
            const auto relativeError = std::abs(measuredFrequency - frequency) / frequency;
            if (relativeError > 0.006f)
            {
                std::cerr << "pitch rate=" << rate << " target=" << frequency
                          << " measured=" << measuredFrequency
                          << " error=" << relativeError << '\n';
                return false;
            }

            PhysicalString string;
            string.prepare(rate);
            string.setParameters(parameters);
            const auto telemetry = string.telemetry();
            if (std::abs(telemetry.effectiveDelaySamples - expectedPeriod) > 0.002f
                || !(telemetry.loopGain >= 0.0f && telemetry.loopGain < 1.0f)
                || !(telemetry.fractionalDelay >= 0.0f
                     && telemetry.fractionalDelay < 1.0f))
                return false;
        }
    }
    return true;
}

double highFrequencyRatio(const std::vector<float>& samples,
                          std::size_t begin, std::size_t end)
{
    double signal = 0.0;
    double difference = 0.0;
    for (std::size_t sample = begin + 1; sample < end; ++sample)
    {
        const auto value = static_cast<double>(samples[sample]);
        const auto delta = value - static_cast<double>(samples[sample - 1]);
        signal += value * value;
        difference += delta * delta;
    }
    return signal > 0.0 ? difference / signal : 0.0;
}

double rms(const std::vector<float>& samples, std::size_t begin,
           std::size_t end)
{
    double squared = 0.0;
    for (auto sample = begin; sample < end; ++sample)
    {
        const auto value = static_cast<double>(samples[sample]);
        squared += value * value;
    }
    return std::sqrt(squared / static_cast<double>(end - begin));
}

bool materialAxesAreAudibleAndDistinct()
{
    PhysicalString::Parameters base;
    base.fundamentalHz = 146.83f;
    base.decaySeconds = 3.0f;
    base.brightness = 0.12f;
    base.hardness = 0.18f;
    base.pluckPosition = 0.12f;

    auto bright = base;
    bright.brightness = 0.96f;
    const auto darkAudio = render(48000.0, 0x2219u, base, 48000);
    const auto brightAudio = render(48000.0, 0x2219u, bright, 48000);
    if (highFrequencyRatio(brightAudio, 4096, 24000)
        <= highFrequencyRatio(darkAudio, 4096, 24000) * 1.08)
        return false;

    auto hard = base;
    hard.hardness = 0.94f;
    const auto softAttack = render(48000.0, 0x2219u, base, 4096);
    const auto hardAttack = render(48000.0, 0x2219u, hard, 4096);
    if (highFrequencyRatio(hardAttack, 0, 2048)
        <= highFrequencyRatio(softAttack, 0, 2048) * 1.08)
        return false;

    auto displaced = base;
    displaced.pluckPosition = 0.44f;
    const auto displacedAudio = render(48000.0, 0x2219u, displaced, 48000);
    auto positionDifferences = 0u;
    for (std::size_t sample = 0; sample < darkAudio.size(); ++sample)
        if (std::bit_cast<std::uint32_t>(darkAudio[sample])
            != std::bit_cast<std::uint32_t>(displacedAudio[sample]))
            ++positionDifferences;
    if (positionDifferences < darkAudio.size() / 2)
        return false;

    auto shortDecay = base;
    shortDecay.decaySeconds = 0.12f;
    auto longDecay = base;
    longDecay.decaySeconds = 5.0f;
    const auto shortAudio = render(48000.0, 0x2219u, shortDecay, 48000);
    const auto longAudio = render(48000.0, 0x2219u, longDecay, 48000);
    return rms(longAudio, 24000, 48000) > rms(shortAudio, 24000, 48000) * 2.0;
}

bool performanceIntentIsFrozenAndHybrid()
{
    PhysicalString::Parameters parameters;
    parameters.fundamentalHz = 196.31f;
    parameters.decaySeconds = 1.1f;
    parameters.brightness = 0.24f;
    parameters.hardness = 0.37f;
    parameters.pluckPosition = 0.16f;
    PhysicalString reference;
    PhysicalString ignored;
    PhysicalString performed;
    for (auto* string : { &reference, &ignored, &performed })
    {
        string->prepare(48000.0);
        string->setSeed(0x82f1u);
        string->setParameters(parameters);
    }
    PhysicalString::PerformanceIntent intent;
    intent.influence = 0.0f;
    intent.decaySeconds = 7.0f;
    intent.brightness = 0.94f;
    intent.hardness = 0.87f;
    intent.pluckPosition = 0.42f;
    ignored.setPerformanceIntent(intent);
    intent.influence = 1.0f;
    performed.setPerformanceIntent(intent);
    for (auto* string : { &reference, &ignored, &performed })
        string->excite(0.75f);

    auto performedDiffers = false;
    for (int sample = 0; sample < 24000; ++sample)
    {
        const auto a = reference.tick();
        const auto b = ignored.tick();
        const auto c = performed.tick();
        if (std::bit_cast<std::uint32_t>(a) != std::bit_cast<std::uint32_t>(b))
            return false;
        if (std::bit_cast<std::uint32_t>(a) != std::bit_cast<std::uint32_t>(c))
            performedDiffers = true;
    }
    const auto effective = performed.telemetry().parameters;
    if (!performedDiffers
        || !approximately(effective.decaySeconds, 7.0f)
        || !approximately(effective.brightness, 0.94f)
        || !approximately(effective.hardness, 0.87f)
        || !approximately(effective.pluckPosition, 0.42f))
        return false;

    // Changing a snapshot after the event cannot mutate the ringing body.
    PhysicalString stable;
    PhysicalString challenged;
    for (auto* string : { &stable, &challenged })
    {
        string->prepare(48000.0);
        string->setSeed(0x91a7u);
        string->setParameters(parameters);
        string->excite(0.8f);
    }
    for (int sample = 0; sample < 4096; ++sample)
        if (std::bit_cast<std::uint32_t>(stable.tick())
            != std::bit_cast<std::uint32_t>(challenged.tick()))
            return false;
    intent.influence = 0.73f;
    challenged.setPerformanceIntent(intent);
    for (int sample = 0; sample < 48000; ++sample)
        if (std::bit_cast<std::uint32_t>(stable.tick())
            != std::bit_cast<std::uint32_t>(challenged.tick()))
            return false;
    return true;
}

bool invalidParametersPreserveState()
{
    PhysicalString reference;
    PhysicalString challenged;
    PhysicalString::Parameters parameters;
    parameters.fundamentalHz = 267.19f;
    parameters.decaySeconds = 2.3f;
    parameters.brightness = 0.62f;
    parameters.hardness = 0.48f;
    parameters.pluckPosition = 0.31f;
    for (auto* string : { &reference, &challenged })
    {
        string->prepare(48000.0);
        string->setSeed(0x719u);
        string->setParameters(parameters);
    }
    auto invalid = parameters;
    invalid.fundamentalHz = std::numeric_limits<float>::quiet_NaN();
    invalid.decaySeconds = std::numeric_limits<float>::infinity();
    invalid.brightness = -std::numeric_limits<float>::infinity();
    invalid.hardness = std::numeric_limits<float>::quiet_NaN();
    invalid.pluckPosition = std::numeric_limits<float>::infinity();
    challenged.setParameters(invalid);
    reference.excite(0.8f);
    challenged.excite(0.8f);
    for (int sample = 0; sample < 48000; ++sample)
        if (std::bit_cast<std::uint32_t>(reference.tick())
            != std::bit_cast<std::uint32_t>(challenged.tick()))
            return false;
    challenged.excite(std::numeric_limits<float>::quiet_NaN());
    return challenged.telemetry().excitationCount == 1;
}

bool repeatedExcitationIsBounded()
{
    PhysicalString string;
    string.prepare(96000.0);
    PhysicalString::Parameters parameters;
    parameters.fundamentalHz = 39.0f;
    parameters.decaySeconds = 12.0f;
    parameters.brightness = 1.0f;
    parameters.hardness = 1.0f;
    parameters.pluckPosition = 0.5f;
    string.setParameters(parameters);
    auto peak = 0.0f;
    auto maximumDelta = 0.0f;
    auto previous = 0.0f;
    for (int sample = 0; sample < 96000 * 4; ++sample)
    {
        if (sample % 960 == 0)
            string.excite(1.0f);
        const auto value = string.tick();
        if (!std::isfinite(value)) return false;
        peak = std::max(peak, std::abs(value));
        maximumDelta = std::max(maximumDelta, std::abs(value - previous));
        previous = value;
    }
    return peak > 1.0e-4f && peak < 0.91f && maximumDelta < 0.55f;
}

bool excitationIsWindowedAtTheContactBoundary()
{
    PhysicalString::Parameters parameters;
    parameters.fundamentalHz = 82.41f;
    parameters.decaySeconds = 2.4f;
    parameters.brightness = 0.66f;
    parameters.hardness = 0.58f;
    parameters.pluckPosition = 0.27f;
    const auto samples = render(48000.0, 0x7b412u, parameters, 48000);
    if (std::abs(samples.front()) > 1.0e-5f)
        return false;
    auto maximumDelta = 0.0f;
    for (std::size_t sample = 1; sample < samples.size(); ++sample)
        maximumDelta = std::max(
            maximumDelta, std::abs(samples[sample] - samples[sample - 1]));
    return maximumDelta < 0.02f;
}
}

int main()
{
    if (!silenceAndTailAreDefined())
    {
        std::cerr << "physical string silence/tail contract failed\n";
        return EXIT_FAILURE;
    }
    if (!deterministicAndSeeded())
    {
        std::cerr << "physical string determinism/seed contract failed\n";
        return EXIT_FAILURE;
    }
    if (!fractionalTuningIsAccurate())
    {
        std::cerr << "physical string fractional tuning failed\n";
        return EXIT_FAILURE;
    }
    if (!materialAxesAreAudibleAndDistinct())
    {
        std::cerr << "physical string material contrast failed\n";
        return EXIT_FAILURE;
    }
    if (!performanceIntentIsFrozenAndHybrid())
    {
        std::cerr << "physical string performance contract failed\n";
        return EXIT_FAILURE;
    }
    if (!invalidParametersPreserveState())
    {
        std::cerr << "physical string numeric safety failed\n";
        return EXIT_FAILURE;
    }
    if (!repeatedExcitationIsBounded())
    {
        std::cerr << "physical string energy bound failed\n";
        return EXIT_FAILURE;
    }
    if (!excitationIsWindowedAtTheContactBoundary())
    {
        std::cerr << "physical string excitation window failed\n";
        return EXIT_FAILURE;
    }
    std::cout << "aquorbium physical string tests passed\n";
}
