// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/BiomaBrain.h"
#include "core/BiomaEngine.h"
#include "core/BiomaMorphOscillator.h"
#include "core/BiomaPulsar.h"
#include "core/BiomaRelabi.h"
#include "core/Ecosystem.h"

#include <array>
#include <bit>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>

namespace
{
const auto floatNaN = std::numeric_limits<float>::quiet_NaN();
const auto floatInfinity = std::numeric_limits<float>::infinity();
const auto doubleInfinity = std::numeric_limits<double>::infinity();

bool sameBits(float left, float right)
{
    return std::bit_cast<std::uint32_t>(left) == std::bit_cast<std::uint32_t>(right);
}

bool oscillatorsRejectNonFiniteParameters()
{
    aquorbium::BiomaMorphOscillator reference;
    aquorbium::BiomaMorphOscillator challenged;
    for (auto* oscillator : { &reference, &challenged })
    {
        oscillator->prepare(48000.0);
        oscillator->reset(0.37f);
        oscillator->setFrequency(733.0f);
        oscillator->setShape(0.63f);
        oscillator->setPulseWidth(0.41f);
    }
    challenged.setFrequency(floatNaN);
    challenged.setShape(floatInfinity);
    challenged.setPulseWidth(-floatInfinity);
    for (int sample = 0; sample < 4096; ++sample)
        if (!sameBits(reference.tick(), challenged.tick())) return false;

    aquorbium::BiomaPulsar pulsarReference;
    aquorbium::BiomaPulsar pulsarChallenged;
    for (auto* pulsar : { &pulsarReference, &pulsarChallenged })
    {
        pulsar->prepare(48000.0);
        pulsar->reset(0.21f);
        pulsar->setFrequency(311.0f);
        pulsar->setDensity(0.72f);
    }
    pulsarChallenged.setFrequency(floatInfinity);
    pulsarChallenged.setDensity(floatNaN);
    for (int sample = 0; sample < 4096; ++sample)
        if (!sameBits(pulsarReference.tick(), pulsarChallenged.tick())) return false;
    return true;
}

bool controlSourcesRecoverFromInvalidSampleRates()
{
    aquorbium::BiomaBrain brain;
    brain.prepare(doubleInfinity);
    const auto drift = brain.nextDrift(floatNaN);
    for (const auto value : drift)
        if (!std::isfinite(value) || value != 0.0f) return false;

    aquorbium::BiomaRelabi relabi;
    relabi.prepare(std::numeric_limits<double>::quiet_NaN());
    for (int sample = 0; sample < 48000; ++sample)
        if (!std::isfinite(relabi.tick())) return false;
    return true;
}

bool ecosystemRetainsValidFields()
{
    aquorbium::BiomaBrain brain;
    brain.prepare(48000.0);
    aquorbium::Ecosystem ecosystem;
    ecosystem.prepare(doubleInfinity);
    ecosystem.reset(brain);
    ecosystem.setDesire({ 0.31f, 0.42f, 0.53f, 0.64f });
    ecosystem.setDesire({ floatNaN, 0.75f, floatInfinity, -floatInfinity });
    const auto desire = ecosystem.desireTarget();
    if (desire.calm != 0.31f || desire.density != 0.75f
        || desire.cohesion != 0.53f || desire.danger != 0.64f)
        return false;

    for (int sample = 0; sample < 48000; ++sample)
        ecosystem.tickSample(brain);
    for (std::size_t index = 0; index < aquorbium::Ecosystem::organismCount; ++index)
    {
        const auto& organism = ecosystem.organism(index);
        const auto& mix = ecosystem.mix(index);
        if (!std::isfinite(organism.position.x) || !std::isfinite(organism.position.y)
            || !std::isfinite(organism.position.z) || !std::isfinite(mix.left)
            || !std::isfinite(mix.right) || !std::isfinite(mix.detail))
            return false;
    }
    return true;
}

bool engineRejectsInvalidControlData()
{
    aquorbium::BiomaEngine engine;
    engine.prepare(doubleInfinity);
    engine.setAutonomousEnabled(true);
    engine.setDesire({ 0.28f, 0.66f, 0.57f, 0.49f });
    engine.setEcosystemBlendImmediately(0.37f);
    engine.setDensity(floatNaN);
    engine.setDrift(floatInfinity);
    engine.setCohesion(-floatInfinity);
    engine.setDanger(floatNaN);
    engine.setEcosystemBlend(floatNaN);
    const auto validSpaceReturn = engine.multiTapSpaceReturnTarget();
    engine.setMultiTapSpaceReturn(floatNaN);
    engine.setMultiTapSpacePerformance(
        { floatNaN, floatInfinity, -floatInfinity, floatNaN, floatInfinity });
    const auto oscillatorGain = engine.speciesGain(
        aquorbium::Ecosystem::Species::Oscillator);
    engine.setSpeciesGain(aquorbium::Ecosystem::Species::Oscillator, floatNaN);

    const auto target = engine.ecosystemState().desireTarget();
    if (target.calm != 0.28f || target.density != 0.66f
        || target.cohesion != 0.57f || target.danger != 0.49f
        || engine.ecosystemBlendTarget() != 0.37f
        || engine.multiTapSpaceReturnTarget() != validSpaceReturn
        || engine.speciesGain(aquorbium::Ecosystem::Species::Oscillator)
            != oscillatorGain)
        return false;

    engine.noteOn(std::numeric_limits<int>::max(), 0.8f);
    const auto voicesBeforeInvalidVelocity = engine.activeVoiceCount();
    engine.noteOn(60, floatNaN);
    if (engine.activeVoiceCount() != voicesBeforeInvalidVelocity)
        return false;

    std::array<float, 257> left {};
    std::array<float, 257> right {};
    for (int block = 0; block < 500; ++block)
    {
        engine.render(left.data(), right.data(), left.size());
        for (std::size_t sample = 0; sample < left.size(); ++sample)
            if (!std::isfinite(left[sample]) || !std::isfinite(right[sample]))
                return false;
    }

    const auto telemetry = engine.telemetry();
    return std::isfinite(telemetry.outputEnergy)
        && std::isfinite(telemetry.environment.dayPhase)
        && std::isfinite(telemetry.integrity.health)
        && std::isfinite(telemetry.multiTapSpace.wetEnergy)
        && std::isfinite(telemetry.multiTapSpace.currentFeedback);
}
}

int main()
{
    if (!oscillatorsRejectNonFiniteParameters())
    {
        std::cerr << "oscillator numeric safety failed\n";
        return EXIT_FAILURE;
    }
    if (!controlSourcesRecoverFromInvalidSampleRates())
    {
        std::cerr << "control source numeric safety failed\n";
        return EXIT_FAILURE;
    }
    if (!ecosystemRetainsValidFields())
    {
        std::cerr << "ecosystem numeric safety failed\n";
        return EXIT_FAILURE;
    }
    if (!engineRejectsInvalidControlData())
    {
        std::cerr << "engine numeric safety failed\n";
        return EXIT_FAILURE;
    }
    std::cout << "aquorbium numeric safety tests passed\n";
}
