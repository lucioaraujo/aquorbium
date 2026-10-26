// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/OrganismVoiceEngine.h"

#include <array>
#include <bit>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>

#define REQUIRE(condition) do { if (!(condition)) { \
    std::cerr << "requirement failed at line " << __LINE__ << ": " #condition "\n"; \
    return EXIT_FAILURE; } } while (false)

namespace
{
using aquorbium::Ecosystem;
using aquorbium::OrganismVoiceEngine;

std::array<float, 4096> render(Ecosystem::Species species, std::uint32_t seed,
                               bool excite = false)
{
    OrganismVoiceEngine engine;
    engine.prepare(48000.0);
    engine.setSeed(seed);
    engine.setPulseWidth(0.41f);
    if (species == Ecosystem::Species::GranularStriker)
    {
        aquorbium::BiomaGranularStriker::Parameters parameters;
        parameters.centerHz = 137.0f;
        parameters.particleDensity = 0.72f;
        engine.setGranularStrikerParameters(parameters);
        engine.exciteGranularStriker(0.8f);
    }
    if (excite)
    {
        aquorbium::BiomaModalResonator::Parameters parameters;
        parameters.fundamentalHz = 137.0f;
        parameters.decaySeconds = 0.8f;
        engine.setResonatorParameters(parameters);
        engine.exciteResonator(0.8f);
    }

    OrganismVoiceEngine::Controls controls;
    controls.species = species;
    controls.frequency = 137.0f;
    controls.shape = 0.37f;
    controls.pulsarDensity = 0.44f;
    std::array<float, 4096> result {};
    for (auto& sample : result)
        sample = engine.renderSample(controls);
    return result;
}

bool hasEnergy(const std::array<float, 4096>& samples)
{
    for (const auto sample : samples)
        if (!std::isfinite(sample) || std::abs(sample) > 1.0f)
            return false;
    for (const auto sample : samples)
        if (sample != 0.0f)
            return true;
    return false;
}

bool sameBits(const std::array<float, 4096>& left,
              const std::array<float, 4096>& right)
{
    for (std::size_t index = 0; index < left.size(); ++index)
        if (std::bit_cast<std::uint32_t>(left[index])
            != std::bit_cast<std::uint32_t>(right[index]))
            return false;
    return true;
}
}

int main()
{
    const auto oscillator = render(Ecosystem::Species::Oscillator, 7u);
    const auto pulsar = render(Ecosystem::Species::Pulsar, 7u);
    const auto resonator = render(Ecosystem::Species::Resonator, 7u, true);
    const auto granular = render(Ecosystem::Species::GranularStriker, 7u);
    REQUIRE(hasEnergy(oscillator));
    REQUIRE(hasEnergy(pulsar));
    REQUIRE(hasEnergy(resonator));
    REQUIRE(hasEnergy(granular));
    REQUIRE(!sameBits(oscillator, pulsar));
    REQUIRE(sameBits(resonator, render(Ecosystem::Species::Resonator, 7u, true)));
    REQUIRE(!sameBits(resonator, render(Ecosystem::Species::Resonator, 8u, true)));

    OrganismVoiceEngine engine;
    engine.prepare(48000.0);
    OrganismVoiceEngine::Controls controls;
    controls.species = Ecosystem::Species::GranularStriker;
    engine.exciteGranularStriker(0.8f);
    REQUIRE(std::isfinite(engine.renderSample(controls)));
    auto telemetry = engine.telemetry(controls.species,
                                      controls.modalResonatorEnabled,
                                      controls.granularStrikerEnabled);
    REQUIRE(telemetry.implemented);
    REQUIRE(telemetry.generator == OrganismVoiceEngine::Generator::GranularStriker);

    controls.granularStrikerEnabled = false;
    REQUIRE(std::isfinite(engine.renderSample(controls)));
    telemetry = engine.telemetry(controls.species,
                                 controls.modalResonatorEnabled,
                                 controls.granularStrikerEnabled);
    REQUIRE(telemetry.generator == OrganismVoiceEngine::Generator::MorphFallback);

    controls.species = Ecosystem::Species::Resonator;
    controls.granularStrikerEnabled = true;
    controls.modalResonatorEnabled = false;
    REQUIRE(std::isfinite(engine.renderSample(controls)));
    telemetry = engine.telemetry(controls.species,
                                 controls.modalResonatorEnabled,
                                 controls.granularStrikerEnabled);
    REQUIRE(telemetry.implemented);
    REQUIRE(telemetry.generator == OrganismVoiceEngine::Generator::MorphFallback);

    controls.species = Ecosystem::Species::PhysicalString;
    REQUIRE(engine.renderSample(controls) == 0.0f);
    telemetry = engine.telemetry(controls.species,
                                 controls.modalResonatorEnabled,
                                 controls.granularStrikerEnabled);
    REQUIRE(!telemetry.implemented);
    REQUIRE(telemetry.generator == OrganismVoiceEngine::Generator::PhysicalString);

    return EXIT_SUCCESS;
}
