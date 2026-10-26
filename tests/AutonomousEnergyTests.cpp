// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/BiomaEngine.h"

#include <algorithm>
#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <vector>

namespace
{
constexpr double sampleRate = 48000.0;
constexpr std::size_t durationSamples = static_cast<std::size_t>(sampleRate * 8.0);

struct Rendered
{
    std::vector<float> left;
    std::vector<float> right;
    aquorbium::BiomaEngine::Telemetry telemetry;
};

Rendered renderAutonomous(
    std::uint32_t seed, std::size_t blockSize, bool enabled,
    aquorbium::GeometricSequencer::PerformanceIntent geometryPerformance = {},
    aquorbium::AutonomousEnergy::FormMemoryPerformance formPerformance = {},
    aquorbium::AutonomousEnergy::StructureMemoryPerformance structurePerformance = {})
{
    aquorbium::BiomaEngine engine;
    engine.prepare(sampleRate);
    engine.setSeed(seed);
    engine.setDesire({ 0.62f, 0.56f, 0.68f, 0.34f });
    engine.setEcosystemOutputFeedbackEnabled(false);
    engine.setGeometryPerformance(geometryPerformance);
    engine.setFormMemoryPerformance(formPerformance);
    engine.setStructureMemoryPerformance(structurePerformance);
    engine.setAutonomousEnabled(enabled);

    Rendered result;
    result.left.resize(durationSamples);
    result.right.resize(durationSamples);
    for (std::size_t offset = 0; offset < durationSamples; offset += blockSize)
    {
        const auto count = std::min(blockSize, durationSamples - offset);
        engine.render(result.left.data() + offset, result.right.data() + offset, count);
    }
    result.telemetry = engine.telemetry();
    return result;
}

bool sameSamples(const Rendered& left, const Rendered& right)
{
    if (left.left.size() != right.left.size()) return false;
    for (std::size_t sample = 0; sample < left.left.size(); ++sample)
    {
        if (std::bit_cast<std::uint32_t>(left.left[sample])
                != std::bit_cast<std::uint32_t>(right.left[sample])
            || std::bit_cast<std::uint32_t>(left.right[sample])
                != std::bit_cast<std::uint32_t>(right.right[sample]))
            return false;
    }
    return true;
}

bool isSilent(const Rendered& render)
{
    return std::all_of(render.left.begin(), render.left.end(), [](float value) { return value == 0.0f; })
        && std::all_of(render.right.begin(), render.right.end(), [](float value) { return value == 0.0f; });
}

bool validateEnabledRender(const Rendered& render)
{
    double energy = 0.0;
    float peak = 0.0f;
    float maximumDelta = 0.0f;
    for (std::size_t sample = 0; sample < render.left.size(); ++sample)
    {
        if (!std::isfinite(render.left[sample]) || !std::isfinite(render.right[sample]))
            return false;
        energy += static_cast<double>(render.left[sample]) * render.left[sample]
            + static_cast<double>(render.right[sample]) * render.right[sample];
        peak = std::max({ peak, std::abs(render.left[sample]), std::abs(render.right[sample]) });
        if (sample > 0)
        {
            maximumDelta = std::max(maximumDelta,
                                    std::abs(render.left[sample] - render.left[sample - 1]));
            maximumDelta = std::max(maximumDelta,
                                    std::abs(render.right[sample] - render.right[sample - 1]));
        }
    }

    const auto& autonomy = render.telemetry.autonomy;
    if (energy <= 0.01 || peak <= 0.001f || peak >= 0.9f || maximumDelta >= 0.25f
        || !render.telemetry.autonomousEnabled || autonomy.enableAmount != 1.0f
        || autonomy.activeOrganisms == 0 || autonomy.eventCount < 4
        || !std::isfinite(autonomy.totalEnergy) || autonomy.totalEnergy <= 0.0f
        || !std::isfinite(render.telemetry.resonator.modalEnergy))
        return false;
    for (const auto& voice : autonomy.voices)
    {
        if (!std::isfinite(voice.energy) || !std::isfinite(voice.frequency)
            || !std::isfinite(voice.midiPitch) || voice.frequency <= 0.0f
            || voice.frequency >= sampleRate * 0.45)
            return false;
    }
    constexpr std::array<aquorbium::OrganismVoiceEngine::Generator,
                         aquorbium::Ecosystem::organismCount> expectedGenerators {
        aquorbium::OrganismVoiceEngine::Generator::MorphOscillator,
        aquorbium::OrganismVoiceEngine::Generator::GranularStriker,
        aquorbium::OrganismVoiceEngine::Generator::ModalResonator,
        aquorbium::OrganismVoiceEngine::Generator::Pulsar,
        aquorbium::OrganismVoiceEngine::Generator::PhysicalString
    };
    for (std::size_t index = 0; index < expectedGenerators.size(); ++index)
    {
        if (!render.telemetry.sources[index].implemented
            || render.telemetry.sources[index].species
                != render.telemetry.organisms[index].species
            || render.telemetry.sources[index].generator != expectedGenerators[index])
            return false;
    }
    if (render.telemetry.organisms[4].species
            != aquorbium::Ecosystem::Species::PhysicalString)
        return false;
    return true;
}

bool evolvesWhileSilent()
{
    aquorbium::BiomaEngine engine;
    engine.prepare(sampleRate);
    engine.setSeed(0x815ab1eu);
    engine.setAutonomousEnabled(false);
    engine.setEcosystemOutputFeedbackEnabled(false);
    std::vector<float> left(static_cast<std::size_t>(sampleRate * 6.0));
    std::vector<float> right(left.size());
    engine.render(left.data(), right.data(), left.size());
    const auto silentState = engine.telemetry().autonomy;
    if (silentState.enableAmount != 0.0f || silentState.eventCount == 0)
        return false;
    if (!std::all_of(left.begin(), left.end(), [](float value) { return value == 0.0f; }))
        return false;

    engine.setAutonomousEnabled(true);
    left.assign(static_cast<std::size_t>(sampleRate * 2.0), 0.0f);
    right.assign(left.size(), 0.0f);
    engine.render(left.data(), right.data(), left.size());
    const auto awakened = engine.telemetry().autonomy;
    return awakened.eventCount > silentState.eventCount
        && awakened.totalEnergy > 0.0f
        && std::any_of(left.begin(), left.end(), [](float value) { return value != 0.0f; });
}

bool enableRampArrivesExactly()
{
    aquorbium::BiomaEngine engine;
    engine.prepare(sampleRate);
    engine.setSeed(0x42u);
    engine.setAutonomousEnabled(true);
    float left = 0.0f;
    float right = 0.0f;
    engine.render(&left, &right, 1);
    const auto firstStep = engine.telemetry().autonomy.enableAmount;
    if (!(firstStep > 0.0f && firstStep < 0.001f))
        return false;

    std::vector<float> rampLeft(5759);
    std::vector<float> rampRight(rampLeft.size());
    engine.render(rampLeft.data(), rampRight.data(), rampLeft.size());
    if (engine.telemetry().autonomy.enableAmount != 1.0f)
        return false;

    engine.setAutonomousEnabled(false);
    rampLeft.assign(5760, 0.0f);
    rampRight.assign(rampLeft.size(), 0.0f);
    engine.render(rampLeft.data(), rampRight.data(), rampLeft.size());
    return engine.telemetry().autonomy.enableAmount == 0.0f;
}
}

int main()
{
    const auto disabled = renderAutonomous(0x815ab1eu, 257, false);
    if (!isSilent(disabled) || disabled.telemetry.autonomy.eventCount == 0)
    {
        std::cerr << "disabled autonomous path is not silent/evolving\n";
        return EXIT_FAILURE;
    }

    const auto smallBlocks = renderAutonomous(0x815ab1eu, 127, true);
    const auto largeBlocks = renderAutonomous(0x815ab1eu, 509, true);
    if (!validateEnabledRender(smallBlocks))
    {
        std::cerr << "autonomous render failed validity limits: events="
                  << smallBlocks.telemetry.autonomy.eventCount << " active="
                  << smallBlocks.telemetry.autonomy.activeOrganisms << " energy="
                  << smallBlocks.telemetry.autonomy.totalEnergy << " enable="
                  << smallBlocks.telemetry.autonomy.enableAmount << '\n';
        return EXIT_FAILURE;
    }
    if (smallBlocks.telemetry.resonator.excitationCount == 0)
    {
        std::cerr << "default autonomous path never excited the modal organism\n";
        return EXIT_FAILURE;
    }
    if (smallBlocks.telemetry.granularStriker.excitationCount == 0)
    {
        std::cerr << "default autonomous path never excited the granular organism\n";
        return EXIT_FAILURE;
    }
    if (smallBlocks.telemetry.physicalString.excitationCount == 0)
    {
        std::cerr << "default autonomous path never excited the physical-string organism\n";
        return EXIT_FAILURE;
    }
    if (!sameSamples(smallBlocks, largeBlocks))
    {
        std::cerr << "autonomous render is block-dependent\n";
        return EXIT_FAILURE;
    }

    const auto anotherSeed = renderAutonomous(0x31f4a927u, 127, true);
    if (sameSamples(smallBlocks, anotherSeed))
    {
        std::cerr << "autonomous seeds produced identical paths\n";
        return EXIT_FAILURE;
    }

    aquorbium::GeometricSequencer::PerformanceIntent ignoredPerformance;
    ignoredPerformance.influence = 0.0f;
    ignoredPerformance.rhythmDensityBias = -1.0f;
    ignoredPerformance.orbitRotation = 1.0f;
    ignoredPerformance.intervalBias = -1.0f;
    ignoredPerformance.mutation = 1.0f;
    const auto ignoredPerformanceRender = renderAutonomous(
        0x815ab1eu, 127, true, ignoredPerformance);
    if (!sameSamples(smallBlocks, ignoredPerformanceRender))
    {
        std::cerr << "zero-influence geometry altered autonomous audio\n";
        return EXIT_FAILURE;
    }

    ignoredPerformance.influence = 1.0f;
    const auto performedGeometry = renderAutonomous(
        0x815ab1eu, 127, true, ignoredPerformance);
    if (!validateEnabledRender(performedGeometry)
        || sameSamples(smallBlocks, performedGeometry))
    {
        std::cerr << "geometry performance did not create a safe audible change\n";
        return EXIT_FAILURE;
    }

    aquorbium::AutonomousEnergy::FormMemoryPerformance hiddenForm;
    hiddenForm.influence = 0.0f;
    hiddenForm.context = 1.0f;
    hiddenForm.inertia = 1.0f;
    hiddenForm.mutation = 1.0f;
    const auto hiddenFormRender = renderAutonomous(
        0x815ab1eu, 127, true, {}, hiddenForm);
    if (!sameSamples(smallBlocks, hiddenFormRender)
        || hiddenFormRender.telemetry.autonomy.formTransitionCount == 0)
    {
        std::cerr << "zero-influence form memory was not transparent/evolving\n";
        return EXIT_FAILURE;
    }

    hiddenForm.influence = 1.0f;
    const auto performedForm = renderAutonomous(
        0x815ab1eu, 127, true, {}, hiddenForm);
    if (!validateEnabledRender(performedForm)
        || sameSamples(smallBlocks, performedForm)
        || performedForm.telemetry.autonomy.formState
            >= aquorbium::BiomaMarkov::stateCount)
    {
        std::cerr << "form memory did not create a bounded structural change\n";
        return EXIT_FAILURE;
    }

    aquorbium::AutonomousEnergy::StructureMemoryPerformance hiddenStructure;
    hiddenStructure.influence = 0.0f;
    hiddenStructure.memory = 1.0f;
    hiddenStructure.harmony = 1.0f;
    hiddenStructure.phase = 1.0f;
    const auto hiddenStructureRender = renderAutonomous(
        0x815ab1eu, 127, true, {}, {}, hiddenStructure);
    if (!sameSamples(smallBlocks, hiddenStructureRender))
    {
        std::cerr << "zero-influence structure memory altered autonomous audio\n";
        return EXIT_FAILURE;
    }

    hiddenStructure.influence = 1.0f;
    const auto performedStructure = renderAutonomous(
        0x815ab1eu, 127, true, {}, {}, hiddenStructure);
    if (!validateEnabledRender(performedStructure)
        || sameSamples(smallBlocks, performedStructure)
        || performedStructure.telemetry.autonomy.memoryStep >= 8
        || performedStructure.telemetry.autonomy.harmonicSection >= 4)
    {
        std::cerr << "step memory and harmony did not create a bounded structural change\n";
        return EXIT_FAILURE;
    }

    if (!evolvesWhileSilent())
    {
        std::cerr << "autonomous clock stopped while inaudible\n";
        return EXIT_FAILURE;
    }

    if (!enableRampArrivesExactly())
    {
        std::cerr << "autonomous enable ramp did not reach exact endpoints\n";
        return EXIT_FAILURE;
    }

    aquorbium::BiomaEngine hybrid;
    hybrid.prepare(sampleRate);
    hybrid.setSeed(77u);
    hybrid.setAutonomousEnabled(true);
    hybrid.noteOn(60, 0.75f);
    std::vector<float> hybridLeft(48000);
    std::vector<float> hybridRight(48000);
    hybrid.render(hybridLeft.data(), hybridRight.data(), hybridLeft.size());
    if (hybrid.activeVoiceCount() != 1 || hybrid.telemetry().autonomy.activeOrganisms == 0)
    {
        std::cerr << "autonomous and external voice banks did not coexist\n";
        return EXIT_FAILURE;
    }

    std::cout << "aquorbium autonomous energy tests passed\n";
}
