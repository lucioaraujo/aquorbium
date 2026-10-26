// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/CoreRefinementModules.h"

#include <array>
#include <bit>
#include <cmath>
#include <complex>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>

namespace
{
bool folderHasExactBypassAndBoundedFold()
{
    aquorbium::OversampledSineFolder folder;
    for (int index = 0; index < 4096; ++index)
    {
        const auto input = std::sin(static_cast<float>(index) * 0.17f);
        const auto bypass = folder.process(input, 0.0f, 12.0f);
        if (std::bit_cast<std::uint32_t>(input)
            != std::bit_cast<std::uint32_t>(bypass)) return false;
        const auto folded = folder.process(input, 1.0f, 12.0f);
        if (!std::isfinite(folded) || std::abs(folded) > 1.0001f)
            return false;
    }
    return true;
}

double highBandEnergy(const std::array<float, 2048>& signal)
{
    auto energy = 0.0;
    for (std::size_t bin = 600; bin < signal.size() / 2; ++bin)
    {
        std::complex<double> sum {};
        for (std::size_t sample = 0; sample < signal.size(); ++sample)
        {
            const auto angle = -6.28318530717958647692
                * static_cast<double>(bin * sample)
                / static_cast<double>(signal.size());
            sum += static_cast<double>(signal[sample])
                * std::complex<double>(std::cos(angle), std::sin(angle));
        }
        energy += std::norm(sum);
    }
    return energy;
}

bool folderReducesUpperBandFoldEnergy()
{
    std::array<float, 2048> direct {}, oversampled {};
    aquorbium::OversampledSineFolder folder;
    for (std::size_t sample = 0; sample < direct.size(); ++sample)
    {
        const auto input = std::sin(6.28318530717958647692
            * 7300.0 * static_cast<double>(sample) / 48000.0);
        direct[sample] = std::sin(input * 10.0f);
        oversampled[sample] = folder.process(input, 1.0f, 10.0f);
    }
    return highBandEnergy(oversampled) < highBandEnergy(direct) * 0.85;
}

bool masterFeedbackIsLimitedAndSmooth()
{
    aquorbium::BoundedMasterFeedback feedback;
    feedback.prepare(200.0);
    feedback.setParameters({ 1.0f, 0.05f, 0.5f, 0.25f });
    auto previous = 0.0f;
    for (int tick = 0; tick < 400; ++tick)
    {
        const auto current = feedback.process({ 12.0f, 6.0f, 3.0f },
                                              4.0f, 1.0f);
        if (!std::isfinite(current) || current < previous
            || current > 0.25001f) return false;
        previous = current;
    }
    if (previous < 0.20f) return false;
    const auto released = feedback.process({}, 0.0f, 0.0f);
    if (!(released < previous && released > 0.0f)) return false;
    feedback.reset();
    return feedback.process({
        std::numeric_limits<float>::infinity(), 0.0f, 0.0f
    }, std::numeric_limits<float>::quiet_NaN(), 1.0f) == 0.0f;
}

bool mediumDistinguishesAirAndWater()
{
    aquorbium::AirWaterMedium air, water;
    air.prepare(48000.0);
    water.prepare(48000.0);
    auto airEnergy = 0.0;
    auto waterEnergy = 0.0;
    aquorbium::AirWaterMedium::Output airOutput, waterOutput;
    for (int sample = 0; sample < 48000; ++sample)
    {
        const auto input = std::sin(static_cast<float>(sample) * 1.47f) * 0.7f;
        airOutput = air.process(input, -input, 1.0f, 0.8f, 0.5f);
        waterOutput = water.process(input, -input, -1.0f, 0.8f, 0.5f);
        airEnergy += static_cast<double>(airOutput.left) * airOutput.left;
        waterEnergy += static_cast<double>(waterOutput.left) * waterOutput.left;
    }
    return airOutput.waterMix == 0.0f && waterOutput.waterMix == 1.0f
        && airOutput.brightness > waterOutput.brightness
        && airEnergy > waterEnergy * 1.5
        && std::isfinite(waterOutput.left);
}
}

int main()
{
    if (!folderHasExactBypassAndBoundedFold())
    {
        std::cerr << "folder bypass/bounds failed\n";
        return EXIT_FAILURE;
    }
    if (!folderReducesUpperBandFoldEnergy())
    {
        std::cerr << "folder spectrum failed\n";
        return EXIT_FAILURE;
    }
    if (!masterFeedbackIsLimitedAndSmooth())
    {
        std::cerr << "master feedback failed\n";
        return EXIT_FAILURE;
    }
    if (!mediumDistinguishesAirAndWater())
    {
        std::cerr << "air/water medium failed\n";
        return EXIT_FAILURE;
    }
    std::cout << "Core refinement modules passed\n";
}
