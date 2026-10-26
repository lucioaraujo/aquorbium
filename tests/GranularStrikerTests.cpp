// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/BiomaGranularStriker.h"
#include "core/GranularPerformance.h"

#include <array>
#include <bit>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>

#define REQUIRE(condition) do { if (!(condition)) { \
    std::cerr << "requirement failed at line " << __LINE__ << ": " #condition "\n"; \
    return EXIT_FAILURE; } } while (false)

namespace
{
using aquorbium::BiomaGranularStriker;
constexpr std::size_t renderSamples = 24000;

struct Render
{
    std::array<float, renderSamples> samples {};
    std::size_t nonZeroSamples = 0;
    float peak = 0.0f;
    double mean = 0.0;
};

Render render(std::uint32_t seed, const BiomaGranularStriker::Parameters& parameters,
              const BiomaGranularStriker::PerformanceIntent& performance = {})
{
    BiomaGranularStriker striker;
    striker.prepare(48000.0);
    striker.setSeed(seed);
    striker.setParameters(parameters);
    striker.setPerformanceIntent(performance);
    striker.excite(0.82f);
    Render result;
    for (auto& sample : result.samples)
    {
        sample = striker.tick();
        if (sample != 0.0f)
            ++result.nonZeroSamples;
        result.peak = std::max(result.peak, std::abs(sample));
        result.mean += sample;
    }
    result.mean /= static_cast<double>(result.samples.size());
    return result;
}

bool sameBits(const Render& left, const Render& right)
{
    for (std::size_t index = 0; index < left.samples.size(); ++index)
        if (std::bit_cast<std::uint32_t>(left.samples[index])
            != std::bit_cast<std::uint32_t>(right.samples[index]))
            return false;
    return true;
}

bool valid(const Render& value)
{
    if (value.nonZeroSamples == 0 || value.peak <= 0.0f || value.peak >= 1.0f
        || std::abs(value.mean) > 0.02)
        return false;
    for (const auto sample : value.samples)
        if (!std::isfinite(sample) || std::abs(sample) >= 1.0f)
            return false;
    return true;
}

double spectralActivity(const Render& value)
{
    double signal = 0.0;
    double difference = 0.0;
    for (std::size_t index = 1; index < value.samples.size(); ++index)
    {
        signal += std::abs(static_cast<double>(value.samples[index]));
        difference += std::abs(static_cast<double>(value.samples[index])
                               - static_cast<double>(value.samples[index - 1]));
    }
    return signal > 0.0 ? difference / signal : 0.0;
}
}

int main()
{
    const auto dust = aquorbium::GranularPerformance::map({ 1.0f, 0.0f, 0.0f, 0.0f });
    const auto metal = aquorbium::GranularPerformance::map({ 1.0f, 1.0f, 1.0f, 1.0f });
    REQUIRE(dust.influence == 1.0f);
    REQUIRE(dust.grainDurationMs == 3.0f);
    REQUIRE(dust.resonance == 0.15f);
    REQUIRE(dust.particleDensity == 0.0f);
    REQUIRE(dust.spectralSpread == 0.0f);
    REQUIRE(dust.irregularity == 0.10f);
    REQUIRE(metal.grainDurationMs == 32.0f);
    REQUIRE(metal.resonance == 1.0f);
    REQUIRE(metal.particleDensity == 1.0f);
    REQUIRE(metal.spectralSpread == 1.0f);
    REQUIRE(metal.irregularity == 1.0f);

    BiomaGranularStriker::Parameters base;
    base.centerHz = 830.0f;
    base.grainDurationMs = 11.0f;
    base.particleDensity = 0.58f;
    base.spectralSpread = 0.42f;
    base.resonance = 0.64f;
    base.irregularity = 0.48f;
    const auto first = render(0x1234u, base);
    REQUIRE(valid(first));
    REQUIRE(sameBits(first, render(0x1234u, base)));
    REQUIRE(!sameBits(first, render(0x1235u, base)));

    auto shortGrains = base;
    shortGrains.grainDurationMs = 2.0f;
    shortGrains.irregularity = 0.0f;
    auto longGrains = shortGrains;
    longGrains.grainDurationMs = 40.0f;
    const auto shortRender = render(99u, shortGrains);
    const auto longRender = render(99u, longGrains);
    REQUIRE(valid(shortRender));
    REQUIRE(valid(longRender));
    REQUIRE(longRender.nonZeroSamples > shortRender.nonZeroSamples * 8);

    auto narrow = base;
    narrow.spectralSpread = 0.0f;
    auto wide = narrow;
    wide.spectralSpread = 1.0f;
    const auto narrowRender = render(71u, narrow);
    const auto wideRender = render(71u, wide);
    REQUIRE(!sameBits(narrowRender, wideRender));
    REQUIRE(std::abs(spectralActivity(narrowRender) - spectralActivity(wideRender))
        > 0.02);

    BiomaGranularStriker striker;
    striker.prepare(48000.0);
    striker.setSeed(77u);
    REQUIRE(striker.tick() == 0.0f);
    auto lowDensity = base;
    lowDensity.particleDensity = 0.0f;
    striker.setParameters(lowDensity);
    striker.excite(0.7f);
    REQUIRE(striker.telemetry().particlesInLastStrike
        == BiomaGranularStriker::minimumParticlesPerStrike);
    striker.reset();
    auto highDensity = base;
    highDensity.particleDensity = 1.0f;
    striker.setParameters(highDensity);
    striker.excite(0.7f);
    REQUIRE(striker.telemetry().particlesInLastStrike
        == BiomaGranularStriker::maximumParticlesPerStrike);

    const auto retained = striker.parameters();
    BiomaGranularStriker::Parameters invalid;
    invalid.centerHz = std::numeric_limits<float>::quiet_NaN();
    invalid.grainDurationMs = std::numeric_limits<float>::infinity();
    invalid.particleDensity = -std::numeric_limits<float>::infinity();
    invalid.spectralSpread = std::numeric_limits<float>::quiet_NaN();
    invalid.resonance = std::numeric_limits<float>::infinity();
    invalid.irregularity = std::numeric_limits<float>::quiet_NaN();
    striker.setParameters(invalid);
    const auto afterInvalid = striker.parameters();
    REQUIRE(std::bit_cast<std::uint32_t>(retained.centerHz)
        == std::bit_cast<std::uint32_t>(afterInvalid.centerHz));
    REQUIRE(std::bit_cast<std::uint32_t>(retained.grainDurationMs)
        == std::bit_cast<std::uint32_t>(afterInvalid.grainDurationMs));
    REQUIRE(std::bit_cast<std::uint32_t>(retained.particleDensity)
        == std::bit_cast<std::uint32_t>(afterInvalid.particleDensity));
    REQUIRE(std::bit_cast<std::uint32_t>(retained.spectralSpread)
        == std::bit_cast<std::uint32_t>(afterInvalid.spectralSpread));
    REQUIRE(std::bit_cast<std::uint32_t>(retained.resonance)
        == std::bit_cast<std::uint32_t>(afterInvalid.resonance));
    REQUIRE(std::bit_cast<std::uint32_t>(retained.irregularity)
        == std::bit_cast<std::uint32_t>(afterInvalid.irregularity));
    const auto excitationsBeforeInvalid = striker.telemetry().excitationCount;
    striker.excite(std::numeric_limits<float>::quiet_NaN());
    REQUIRE(striker.telemetry().excitationCount == excitationsBeforeInvalid);

    striker.reset();
    striker.setParameters(highDensity);
    for (int strike = 0; strike < 200; ++strike)
    {
        striker.excite(1.0f);
        for (int sample = 0; sample < 9; ++sample)
            REQUIRE(std::isfinite(striker.tick()));
    }
    REQUIRE(striker.telemetry().stolenParticleCount > 0);
    for (int sample = 0; sample < 48000; ++sample)
        REQUIRE(std::isfinite(striker.tick()));
    REQUIRE(!striker.telemetry().active);
    REQUIRE(striker.tick() == 0.0f);

    BiomaGranularStriker::PerformanceIntent zeroInfluence;
    zeroInfluence.influence = 0.0f;
    zeroInfluence.grainDurationMs = 40.0f;
    zeroInfluence.particleDensity = 1.0f;
    zeroInfluence.spectralSpread = 1.0f;
    zeroInfluence.resonance = 1.0f;
    zeroInfluence.irregularity = 1.0f;
    REQUIRE(sameBits(render(515u, base), render(515u, base, zeroInfluence)));
    zeroInfluence.influence = 1.0f;
    REQUIRE(!sameBits(render(515u, base), render(515u, base, zeroInfluence)));

    // A gesture affects only particles born after it. Existing tails must not
    // be retuned or have filter coefficients changed under the listener.
    BiomaGranularStriker unchangedTail;
    BiomaGranularStriker performedTail;
    unchangedTail.prepare(48000.0);
    performedTail.prepare(48000.0);
    unchangedTail.setSeed(404u);
    performedTail.setSeed(404u);
    unchangedTail.setParameters(base);
    performedTail.setParameters(base);
    unchangedTail.excite(0.9f);
    performedTail.excite(0.9f);
    for (int sample = 0; sample < 137; ++sample)
        REQUIRE(std::bit_cast<std::uint32_t>(unchangedTail.tick())
            == std::bit_cast<std::uint32_t>(performedTail.tick()));
    performedTail.setPerformanceIntent(metal);
    for (int sample = 0; sample < 4096; ++sample)
        REQUIRE(std::bit_cast<std::uint32_t>(unchangedTail.tick())
            == std::bit_cast<std::uint32_t>(performedTail.tick()));

    for (const auto sampleRate : { 8000.0, 44100.0, 96000.0, 768000.0 })
    {
        BiomaGranularStriker extreme;
        extreme.prepare(sampleRate);
        extreme.setSeed(19u);
        extreme.setParameters({ static_cast<float>(sampleRate), 40.0f, 1.0f,
                                1.0f, 1.0f, 1.0f });
        extreme.excite(1.0f);
        for (int sample = 0; sample < 4096; ++sample)
        {
            const auto value = extreme.tick();
            REQUIRE(std::isfinite(value));
            REQUIRE(std::abs(value) < 1.0f);
        }
    }

    return EXIT_SUCCESS;
}
