// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/BiomaEngine.h"
#include "core/BiomaMorphOscillator.h"
#include "core/BiomaPulsar.h"
#include "core/BiomaRelabi.h"

#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <memory>
#include <vector>

namespace
{
std::array<float, 48000> render(std::uint32_t seed, std::size_t blockSize,
                                aquorbium::BiomaEngine::EcosystemRenderMode mode
                                    = aquorbium::BiomaEngine::EcosystemRenderMode::Active,
                                bool outputFeedback = true)
{
    aquorbium::BiomaEngine engine;
    engine.prepare(48000.0);
    engine.setSeed(seed);
    engine.setDensity(0.6f);
    engine.setDrift(0.3f);
    engine.setEcosystemBlendImmediately(
        mode == aquorbium::BiomaEngine::EcosystemRenderMode::Active ? 1.0f : 0.0f);
    engine.setEcosystemOutputFeedbackEnabled(outputFeedback);
    engine.noteOn(48);
    std::array<float, 48000> left {};
    std::array<float, 48000> right {};
    for (std::size_t offset = 0; offset < left.size(); offset += blockSize)
    {
        const auto count = std::min(blockSize, left.size() - offset);
        engine.render(left.data() + offset, right.data() + offset, count);
    }
    return left;
}

std::array<float, 48000> renderMorph(std::size_t blockSize)
{
    aquorbium::BiomaEngine engine;
    engine.prepare(48000.0);
    engine.setSeed(0xB10A5EEDu);
    engine.setDensity(0.6f);
    engine.setDrift(0.3f);
    engine.setEcosystemOutputFeedbackEnabled(false);
    engine.setEcosystemBlendImmediately(0.0f);
    engine.noteOn(48, 0.8f);

    constexpr std::size_t morphSample = 12000;
    std::array<float, 48000> left {};
    std::array<float, 48000> right {};
    std::size_t offset = 0;
    while (offset < left.size())
    {
        if (offset == morphSample)
            engine.setEcosystemBlend(1.0f);
        const auto nextEvent = offset < morphSample ? morphSample : left.size();
        const auto count = std::min({ blockSize, left.size() - offset, nextEvent - offset });
        engine.render(left.data() + offset, right.data() + offset, count);
        offset += count;
    }
    return left;
}

bool sameControlState(const aquorbium::BiomaEngine::Telemetry& left,
                      const aquorbium::BiomaEngine::Telemetry& right)
{
    if (std::bit_cast<std::uint32_t>(left.environment.dayPhase)
            != std::bit_cast<std::uint32_t>(right.environment.dayPhase)
        || std::bit_cast<std::uint32_t>(left.environment.seasonPhase)
            != std::bit_cast<std::uint32_t>(right.environment.seasonPhase)
        || std::bit_cast<std::uint32_t>(left.integrity.crackStress)
            != std::bit_cast<std::uint32_t>(right.integrity.crackStress))
        return false;

    for (std::size_t index = 0; index < aquorbium::Ecosystem::organismCount; ++index)
    {
        const auto& a = left.organisms[index];
        const auto& b = right.organisms[index];
        const auto& aMix = left.mix[index];
        const auto& bMix = right.mix[index];
        if (std::bit_cast<std::uint32_t>(a.position.x) != std::bit_cast<std::uint32_t>(b.position.x)
            || std::bit_cast<std::uint32_t>(a.position.y) != std::bit_cast<std::uint32_t>(b.position.y)
            || std::bit_cast<std::uint32_t>(a.position.z) != std::bit_cast<std::uint32_t>(b.position.z)
            || std::bit_cast<std::uint32_t>(aMix.left) != std::bit_cast<std::uint32_t>(bMix.left)
            || std::bit_cast<std::uint32_t>(aMix.right) != std::bit_cast<std::uint32_t>(bMix.right)
            || std::bit_cast<std::uint32_t>(aMix.detail) != std::bit_cast<std::uint32_t>(bMix.detail))
            return false;
    }
    return true;
}
}

int main()
{
    static_assert(aquorbium::BiomaEngine::physicalStringBodyCount
                  == aquorbium::BiomaEngine::voiceCount + 1);
    static_assert(aquorbium::BiomaEngine::outputCalibrationGain == 5.0f);
    aquorbium::BiomaEngine authoredBalance;
    if (authoredBalance.speciesGain(aquorbium::Ecosystem::Species::Oscillator) != 0.55f
        || authoredBalance.speciesGain(aquorbium::Ecosystem::Species::Pulsar) != 1.20f
        || authoredBalance.speciesGain(aquorbium::Ecosystem::Species::Resonator) != 1.50f
        || authoredBalance.speciesGain(aquorbium::Ecosystem::Species::GranularStriker) != 1.25f
        || authoredBalance.speciesGain(aquorbium::Ecosystem::Species::PhysicalString) != 1.50f)
        return EXIT_FAILURE;
    authoredBalance.setSpeciesGain(aquorbium::Ecosystem::Species::Pulsar, 4.0f);
    if (authoredBalance.speciesGain(aquorbium::Ecosystem::Species::Pulsar) != 2.0f)
        return EXIT_FAILURE;
    authoredBalance.prepare(48000.0);
    if (authoredBalance.outputLatencySamples() != 240u
        || authoredBalance.telemetry().output.latencySamples != 240u)
        return EXIT_FAILURE;
    authoredBalance.setSeed(42u);
    const auto initialLensY = authoredBalance.telemetry().lens.surfacePosition.y;
    authoredBalance.setLensGesture(true, { 0.0f, 1.0f, 0.0f });
    if (authoredBalance.telemetry().lens.heldByObserver != 0
        || authoredBalance.telemetry().lens.targetPosition.y < 0.99f)
        return EXIT_FAILURE;
    std::array<float, 480> lensLeft {};
    std::array<float, 480> lensRight {};
    authoredBalance.render(lensLeft.data(), lensRight.data(), lensLeft.size());
    if (authoredBalance.telemetry().lens.surfacePosition.y <= initialLensY)
        return EXIT_FAILURE;
    authoredBalance.setLensGesture(false, { 0.0f, 1.0f, 0.0f });
    if (authoredBalance.telemetry().lens.heldByObserver != -1)
        return EXIT_FAILURE;

    auto equalListening = std::make_unique<aquorbium::BiomaEngine>();
    auto focusedListening = std::make_unique<aquorbium::BiomaEngine>();
    for (auto* engine : { equalListening.get(), focusedListening.get() })
    {
        engine->prepare(48000.0);
        engine->setSeed(73u);
    }
    focusedListening->setListeningBlend({ 1.0f, 0.0f, 0.0f, 0.0f });
    std::array<float, 4800> listeningLeft {};
    std::array<float, 4800> listeningRight {};
    equalListening->render(listeningLeft.data(), listeningRight.data(), listeningLeft.size());
    focusedListening->render(listeningLeft.data(), listeningRight.data(), listeningLeft.size());
    const auto equalMix = equalListening->telemetry().mix;
    const auto focusedMix = focusedListening->telemetry().mix;
    float listeningDifference = 0.0f;
    for (std::size_t i = 0; i < equalMix.size(); ++i)
        listeningDifference += std::abs(equalMix[i].left - focusedMix[i].left)
            + std::abs(equalMix[i].right - focusedMix[i].right)
            + std::abs(equalMix[i].detail - focusedMix[i].detail);
    if (listeningDifference <= 0.01f)
        return EXIT_FAILURE;
    focusedListening->setCohesion(0.81f);
    focusedListening->setDanger(0.92f);
    const auto desireTarget = focusedListening->ecosystemState().desireTarget();
    if (desireTarget.cohesion != 0.81f || desireTarget.danger != 0.92f)
        return EXIT_FAILURE;
    aquorbium::BiomaRelabi relabi;
    relabi.prepare(48000.0);
    float relabiMin = 1.0f, relabiMax = -1.0f;
    for (int sample = 0; sample < 48000 * 8; ++sample)
    {
        const auto value = relabi.tick();
        if (!std::isfinite(value) || value < -1.01f || value > 1.01f) return EXIT_FAILURE;
        relabiMin = std::min(relabiMin, value);
        relabiMax = std::max(relabiMax, value);
    }
    if (relabiMax - relabiMin < 0.1f) return EXIT_FAILURE;

    aquorbium::BiomaMorphOscillator oscillator;
    oscillator.prepare(48000.0);
    oscillator.setFrequency(13000.0f);
    oscillator.setShape(1.0f);
    float oscillatorPeak = 0.0f;
    for (int sample = 0; sample < 48000; ++sample)
    {
        const auto value = oscillator.tick();
        if (!std::isfinite(value)) return EXIT_FAILURE;
        oscillatorPeak = std::max(oscillatorPeak, std::abs(value));
    }
    if (oscillatorPeak <= 0.1f || oscillatorPeak > 1.1f) return EXIT_FAILURE;

    aquorbium::BiomaPulsar pulsar;
    pulsar.prepare(48000.0);
    pulsar.setFrequency(220.0f);
    pulsar.setDensity(0.05f);
    double sparseEnergy = 0.0;
    for (int sample = 0; sample < 48000; ++sample)
    {
        const auto value = pulsar.tick();
        sparseEnergy += static_cast<double>(value) * value;
    }
    pulsar.reset();
    pulsar.setDensity(0.95f);
    double denseEnergy = 0.0;
    for (int sample = 0; sample < 48000; ++sample)
    {
        const auto value = pulsar.tick();
        if (!std::isfinite(value)) return EXIT_FAILURE;
        denseEnergy += static_cast<double>(value) * value;
    }
    if (sparseEnergy <= 0.0 || denseEnergy <= sparseEnergy * 2.0) return EXIT_FAILURE;

    aquorbium::BiomaPulsar stressedA;
    aquorbium::BiomaPulsar stressedB;
    for (auto* stressed : { &stressedA, &stressedB })
    {
        stressed->prepare(48000.0);
        stressed->setSeed(0x7a31u);
        stressed->setFrequency(220.0f);
        stressed->setDensity(0.8f);
        stressed->setEcology(0.92f, 0.7f, 0.85f, 0.25f);
        stressed->reset();
    }
    double stressedEnergy = 0.0;
    for (int sample = 0; sample < 48000; ++sample)
    {
        const auto a = stressedA.tick();
        const auto b = stressedB.tick();
        if (!std::isfinite(a)
            || std::bit_cast<std::uint32_t>(a)
                != std::bit_cast<std::uint32_t>(b))
            return EXIT_FAILURE;
        stressedEnergy += static_cast<double>(a) * a;
    }
    if (!(stressedEnergy > 0.0 && stressedEnergy < denseEnergy * 0.45))
        return EXIT_FAILURE;

    const auto left = render(42, 127);
    const auto matching = render(42, 509);
    const auto different = render(84, 127);
    double energy = 0.0;
    for (const auto sample : left)
    {
        if (!std::isfinite(sample)) return EXIT_FAILURE;
        energy += static_cast<double>(sample) * sample;
    }
    if (energy <= 1.0) return EXIT_FAILURE;
    for (std::size_t index = 0; index < left.size(); ++index)
    {
        if (std::bit_cast<std::uint32_t>(left[index]) != std::bit_cast<std::uint32_t>(matching[index]))
            return EXIT_FAILURE;
    }
    bool differsWithSeed = false;
    for (std::size_t index = 0; index < left.size(); ++index)
        differsWithSeed = differsWithSeed || left[index] != different[index];
    if (!differsWithSeed) return EXIT_FAILURE;

    const auto neutral = render(42, 127, aquorbium::BiomaEngine::EcosystemRenderMode::Neutral, false);
    const auto neutralMatching = render(42, 509, aquorbium::BiomaEngine::EcosystemRenderMode::Neutral, false);
    for (std::size_t index = 0; index < neutral.size(); ++index)
    {
        if (std::bit_cast<std::uint32_t>(neutral[index])
            != std::bit_cast<std::uint32_t>(neutralMatching[index]))
            return EXIT_FAILURE;
    }

    aquorbium::BiomaEngine activeComparison;
    aquorbium::BiomaEngine neutralComparison;
    for (auto* engine : { &activeComparison, &neutralComparison })
    {
        engine->prepare(48000.0);
        engine->setSeed(0xA0B0B1u);
        engine->setDesire({ 0.32f, 0.72f, 0.66f, 0.58f });
        engine->setEcosystemOutputFeedbackEnabled(false);
        engine->noteOn(48, 0.75f);
    }
    activeComparison.setEcosystemBlendImmediately(1.0f);
    neutralComparison.setEcosystemBlendImmediately(0.0f);
    std::array<float, 4800> activeLeft {}, activeRight {}, neutralLeft {}, neutralRight {};
    activeComparison.render(activeLeft.data(), activeRight.data(), activeLeft.size());
    neutralComparison.render(neutralLeft.data(), neutralRight.data(), neutralLeft.size());
    const auto activeTelemetry = activeComparison.telemetry();
    const auto neutralTelemetry = neutralComparison.telemetry();
    if (!sameControlState(activeTelemetry, neutralTelemetry)
        || activeTelemetry.outputFeedbackEnabled || neutralTelemetry.outputFeedbackEnabled
        || activeTelemetry.renderMode != aquorbium::BiomaEngine::EcosystemRenderMode::Active
        || neutralTelemetry.renderMode != aquorbium::BiomaEngine::EcosystemRenderMode::Neutral)
        return EXIT_FAILURE;
    bool differsWithMode = false;
    for (std::size_t index = 0; index < activeLeft.size(); ++index)
        differsWithMode = differsWithMode || activeLeft[index] != neutralLeft[index]
            || activeRight[index] != neutralRight[index];
    if (!differsWithMode || !std::isfinite(activeTelemetry.outputEnergy)
        || !std::isfinite(neutralTelemetry.outputEnergy))
        return EXIT_FAILURE;

    // One shared reflection body runs after the stereo sum. The technical
    // output switch is acoustically different but ecologically inert because
    // both instances keep identical clocks and the homeostasis still observes
    // the direct pre-space signal in this integration milestone.
    auto spaceOn = std::make_unique<aquorbium::BiomaEngine>();
    auto spaceOff = std::make_unique<aquorbium::BiomaEngine>();
    for (auto* engine : { spaceOn.get(), spaceOff.get() })
    {
        engine->prepare(48000.0);
        engine->setSeed(0x54a91u);
        engine->setDesire({ 0.63f, 0.71f, 0.42f, 0.57f });
        engine->setEcosystemBlendImmediately(1.0f);
        engine->setEcosystemOutputFeedbackEnabled(false);
        engine->setAutonomousEnabled(true);
    }
    spaceOff->setMultiTapSpaceEnabled(false);
    std::vector<float> spaceOnLeft(48000), spaceOnRight(48000);
    std::vector<float> spaceOffLeft(48000), spaceOffRight(48000);
    spaceOn->render(spaceOnLeft.data(), spaceOnRight.data(), spaceOnLeft.size());
    spaceOff->render(spaceOffLeft.data(), spaceOffRight.data(), spaceOffLeft.size());
    const auto spaceOnTelemetry = spaceOn->telemetry();
    const auto spaceOffTelemetry = spaceOff->telemetry();
    if (!sameControlState(spaceOnTelemetry, spaceOffTelemetry)
        || !spaceOnTelemetry.multiTapSpaceEnabled
        || spaceOffTelemetry.multiTapSpaceEnabled
        || spaceOnTelemetry.multiTapSpace.activeTaps
            != aquorbium::MultiTapSpace::tapCount
        || spaceOnTelemetry.multiTapSpaceReturnTarget != 0.36f)
    {
        std::cerr << "shared space telemetry/control mismatch\n";
        return EXIT_FAILURE;
    }
    bool sharedSpaceDiffers = false;
    for (std::size_t index = 0; index < spaceOnLeft.size(); ++index)
        sharedSpaceDiffers = sharedSpaceDiffers
            || spaceOnLeft[index] != spaceOffLeft[index]
            || spaceOnRight[index] != spaceOffRight[index];
    if (!sharedSpaceDiffers || spaceOnTelemetry.multiTapSpace.wetEnergy <= 0.0f
        || spaceOffTelemetry.multiTapSpace.wetEnergy <= 0.0f)
    {
        std::cerr << "shared space did not produce a controlled difference\n";
        return EXIT_FAILURE;
    }

    // Killing every source after charging the shared line separates a genuine
    // space tail from the minute residual of the autonomous DC blockers.
    constexpr std::array species {
        aquorbium::Ecosystem::Species::Oscillator,
        aquorbium::Ecosystem::Species::GranularStriker,
        aquorbium::Ecosystem::Species::Resonator,
        aquorbium::Ecosystem::Species::Pulsar,
        aquorbium::Ecosystem::Species::PhysicalString
    };
    for (auto* engine : { spaceOn.get(), spaceOff.get() })
        for (const auto item : species)
            engine->setSpeciesGain(item, 0.0f);
    std::vector<float> tailOnLeft(12000), tailOnRight(12000);
    std::vector<float> tailOffLeft(12000), tailOffRight(12000);
    spaceOn->render(tailOnLeft.data(), tailOnRight.data(), tailOnLeft.size());
    spaceOff->render(tailOffLeft.data(), tailOffRight.data(), tailOffLeft.size());
    double spaceTailEnergy = 0.0;
    for (std::size_t index = 0; index < tailOnLeft.size(); ++index)
    {
        spaceTailEnergy += static_cast<double>(tailOnLeft[index]) * tailOnLeft[index]
            + static_cast<double>(tailOnRight[index]) * tailOnRight[index];
        // The direct route may retain a minute DC-blocker transient after its
        // gain is removed; compare energy instead of claiming exact silence.
    }
    double directResidualEnergy = 0.0;
    for (std::size_t index = 0; index < tailOffLeft.size(); ++index)
        directResidualEnergy += static_cast<double>(tailOffLeft[index]) * tailOffLeft[index]
            + static_cast<double>(tailOffRight[index]) * tailOffRight[index];
    if (spaceTailEnergy <= directResidualEnergy * 1.01 + 1.0e-12)
    {
        std::cerr << "shared space tail not separated: wet=" << spaceTailEnergy
                  << " direct=" << directResidualEnergy << '\n';
        return EXIT_FAILURE;
    }

    // The performance return rejects invalid values and fades to zero instead
    // of bypassing a live delay line abruptly.
    const auto validSpaceReturn = spaceOn->multiTapSpaceReturnTarget();
    spaceOn->setMultiTapSpaceReturn(std::numeric_limits<float>::quiet_NaN());
    if (spaceOn->multiTapSpaceReturnTarget() != validSpaceReturn)
    {
        std::cerr << "invalid shared space return was not retained\n";
        return EXIT_FAILURE;
    }
    spaceOn->setMultiTapSpaceReturn(0.0f);
    std::vector<float> spaceFadeLeft(48000), spaceFadeRight(48000);
    spaceOn->render(spaceFadeLeft.data(), spaceFadeRight.data(), spaceFadeLeft.size());
    if (!(spaceOn->multiTapSpaceReturnCurrent() < 1.0e-5f))
    {
        std::cerr << "shared space return fade did not converge: "
                  << spaceOn->multiTapSpaceReturnCurrent() << '\n';
        return EXIT_FAILURE;
    }

    const auto autonomousSpace = spaceOn->telemetry().multiTapSpace;
    spaceOn->setDesire({ 1.0f, 1.0f, 0.0f, 1.0f });
    spaceOn->render(spaceFadeLeft.data(), spaceFadeRight.data(), spaceFadeLeft.size());
    const auto remappedSpace = spaceOn->telemetry().multiTapSpace;
    if (!(remappedSpace.targetFeedback > autonomousSpace.targetFeedback))
    {
        std::cerr << "ecosystem did not remap shared space memory\n";
        return EXIT_FAILURE;
    }
    const auto autonomousDelay = remappedSpace.taps[0].targetDelayMilliseconds;
    spaceOn->setMultiTapSpacePerformance({ 1.0f, 1.0f, 0.0f, 0.0f, 1.0f });
    spaceOn->render(spaceFadeLeft.data(), spaceFadeRight.data(), 1);
    const auto performedSpace = spaceOn->telemetry().multiTapSpace;
    if (!(performedSpace.taps[0].targetDelayMilliseconds > autonomousDelay)
        || !(performedSpace.targetFeedback > remappedSpace.targetFeedback))
    {
        std::cerr << "performance intent did not take over shared space\n";
        return EXIT_FAILURE;
    }

    aquorbium::BiomaEngine morph;
    morph.prepare(48000.0);
    morph.setSeed(1776u);
    morph.setEcosystemOutputFeedbackEnabled(false);
    morph.setEcosystemBlendImmediately(0.0f);
    morph.noteOn(48, 0.75f);
    std::array<float, 1> morphLeft {}, morphRight {};
    morph.setEcosystemRenderMode(aquorbium::BiomaEngine::EcosystemRenderMode::Active);
    if (morph.ecosystemBlendTarget() != 1.0f || morph.ecosystemBlendCurrent() != 0.0f)
    {
        std::cerr << "morph target/current did not retain endpoint\n";
        return EXIT_FAILURE;
    }
    morph.render(morphLeft.data(), morphRight.data(), 1);
    const auto firstMorphStep = morph.telemetry();
    if (!(firstMorphStep.ecosystemBlendCurrent > 0.0f
          && firstMorphStep.ecosystemBlendCurrent < 0.001f)
        || firstMorphStep.ecosystemBlendTarget != 1.0f
        || !std::isfinite(morphLeft[0]) || !std::isfinite(morphRight[0]))
    {
        std::cerr << "first morph step invalid: " << firstMorphStep.ecosystemBlendCurrent << '\n';
        return EXIT_FAILURE;
    }
    std::array<float, 3839> morphRampLeft {}, morphRampRight {};
    morph.render(morphRampLeft.data(), morphRampRight.data(), morphRampLeft.size());
    if (morph.ecosystemBlendCurrent() != 1.0f)
    {
        std::cerr << "morph did not finish: " << morph.ecosystemBlendCurrent() << '\n';
        return EXIT_FAILURE;
    }
    morph.setEcosystemBlend(-4.0f);
    if (morph.ecosystemBlendTarget() != 0.0f || morph.ecosystemBlendCurrent() != 1.0f)
    {
        std::cerr << "bounded morph target invalid\n";
        return EXIT_FAILURE;
    }

    const auto morphSmallBlocks = renderMorph(127);
    const auto morphLargeBlocks = renderMorph(509);
    for (std::size_t index = 0; index < morphSmallBlocks.size(); ++index)
    {
        if (std::bit_cast<std::uint32_t>(morphSmallBlocks[index])
            != std::bit_cast<std::uint32_t>(morphLargeBlocks[index]))
        {
            std::cerr << "morph block invariance failed at " << index << '\n';
            return EXIT_FAILURE;
        }
    }

    aquorbium::BiomaEngine polyphonic;
    polyphonic.prepare(48000.0);
    polyphonic.setSeed(99);
    polyphonic.noteOn(48, 0.8f);
    polyphonic.noteOn(52, 0.7f);
    polyphonic.noteOn(55, 0.6f);
    polyphonic.noteOn(60, 0.5f);
    if (polyphonic.activeVoiceCount() != aquorbium::BiomaEngine::voiceCount) return EXIT_FAILURE;
    std::array<float, 24000> polyLeft {}, polyRight {};
    polyphonic.render(polyLeft.data(), polyRight.data(), polyLeft.size());
    polyphonic.noteOff(52);
    polyphonic.render(polyLeft.data(), polyRight.data(), polyLeft.size());
    if (polyphonic.activeVoiceCount() >= aquorbium::BiomaEngine::voiceCount) return EXIT_FAILURE;
    polyphonic.noteOn(64, 0.9f);
    if (polyphonic.activeVoiceCount() != aquorbium::BiomaEngine::voiceCount) return EXIT_FAILURE;

    aquorbium::BiomaEngine levelStress;
    levelStress.prepare(48000.0);
    levelStress.setSeed(0x7a91c53du);
    levelStress.setDesire({ 0.15f, 1.0f, 0.35f, 0.85f });
    levelStress.setDensity(1.0f);
    levelStress.setDrift(1.0f);
    levelStress.setCreativeMasterInfluence(1.0f);
    levelStress.setAutonomousEnabled(true);
    for (const auto note : std::array { 36, 48, 55, 67 })
        levelStress.noteOn(note, 1.0f);
    std::array<float, 257> stressLeft {}, stressRight {};
    double stressSquared = 0.0;
    float stressPeak = 0.0f;
    float stressDelta = 0.0f;
    float previousLeft = 0.0f;
    float previousRight = 0.0f;
    constexpr std::size_t stressSamples = 48000 * 8;
    for (std::size_t rendered = 0; rendered < stressSamples; rendered += stressLeft.size())
    {
        const auto count = std::min(stressLeft.size(), stressSamples - rendered);
        levelStress.render(stressLeft.data(), stressRight.data(), count);
        for (std::size_t sample = 0; sample < count; ++sample)
        {
            const auto leftSample = stressLeft[sample];
            const auto rightSample = stressRight[sample];
            if (!std::isfinite(leftSample) || !std::isfinite(rightSample))
                return EXIT_FAILURE;
            stressPeak = std::max({ stressPeak, std::abs(leftSample),
                                    std::abs(rightSample) });
            stressDelta = std::max({ stressDelta,
                                     std::abs(leftSample - previousLeft),
                                     std::abs(rightSample - previousRight) });
            stressSquared += static_cast<double>(leftSample) * leftSample
                + static_cast<double>(rightSample) * rightSample;
            previousLeft = leftSample;
            previousRight = rightSample;
        }
    }
    const auto stressRms = std::sqrt(stressSquared
        / static_cast<double>(stressSamples * 2));
    const auto stressTelemetry = levelStress.telemetry();
    if (stressRms < 0.01 || stressPeak >= 0.90f || stressDelta >= 0.50f
        || !(stressTelemetry.masterFeedback > 0.0f
             && stressTelemetry.masterFeedback <= 0.32001f))
    {
        std::cerr << "output calibration outside safe range: rms=" << stressRms
                  << " peak=" << stressPeak << " max_delta=" << stressDelta
                  << " master_feedback=" << stressTelemetry.masterFeedback << '\n';
        return EXIT_FAILURE;
    }
    std::cout << "output calibration stress: rms=" << stressRms
              << " peak=" << stressPeak << " max_delta=" << stressDelta << '\n';

    // the Brain reaches the Space (2026-10-23): with the SPACE gesture at 1
    // the reflections' spread follows the Brain's mood, within +-10 %, against
    // the same engine with the gesture at 0 (whose spread the Brain leaves
    // alone); the space's own SPREAD at 1 keeps the two pan fields comparable
    {
        auto stillEngine = std::make_unique<aquorbium::BiomaEngine>();
        auto movedEngine = std::make_unique<aquorbium::BiomaEngine>();
        auto& still = *stillEngine;
        auto& moved = *movedEngine;
        for (auto* engine : { &still, &moved })
        {
            engine->prepare(48000.0);
            engine->setSeed(0xb7a1u);
            engine->setAutonomousEnabled(true);
        }
        moved.setMultiTapSpacePerformance({ 1.0f, 0.5f, 1.0f, 0.5f, 0.35f });
        std::array<float, 480> l {}, r {};
        float lowest = 10.0f, highest = 0.0f;
        for (int block = 0; block < 100 * 60; ++block)  // 60 s
        {
            still.render(l.data(), r.data(), l.size());
            moved.render(l.data(), r.data(), l.size());
            if (block < 100 * 2 || block % 50 != 0)
                continue;
            const auto& a = still.telemetry().multiTapSpace.taps;
            const auto& b = moved.telemetry().multiTapSpace.taps;
            for (std::size_t t = 0; t < a.size(); ++t)
                if (std::abs(a[t].currentPan) > 0.2f && std::abs(a[t].currentPan) < 0.85f)
                {
                    const auto ratio = b[t].currentPan / a[t].currentPan;
                    lowest = std::min(lowest, ratio);
                    highest = std::max(highest, ratio);
                }
        }
        std::cout << "brain -> space: spread ratio " << lowest << " .. " << highest << '\n';
        if (!(lowest >= 0.89f && highest <= 1.11f && highest - lowest > 0.02f))
        {
            std::cerr << "the Brain does not reach the Space within its bounds\n";
            return EXIT_FAILURE;
        }
    }
    std::cout << "aquorbium core smoke test passed\n";
}
