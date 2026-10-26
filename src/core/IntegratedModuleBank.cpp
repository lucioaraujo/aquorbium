// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/IntegratedModuleBank.h"

#include "core/MotionWaveBank.h"
#include "core/Numeric.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <numbers>

namespace aquorbium
{
namespace
{
float unit(float value, float fallback = 0.0f) noexcept
{
    return numeric::unit(value, fallback);
}

float bipolar(float value) noexcept
{
    return unit(value) * 2.0f - 1.0f;
}
}

void IntegratedModuleBank::prepare(double newSampleRate) noexcept
{
    sampleRate = numeric::sampleRate(newSampleRate);
    smoothing = static_cast<float>(-std::expm1(-1.0 / (sampleRate * 0.020)));
    contextSmoothing = static_cast<float>(
        -std::expm1(-1.0 / (sampleRate * 0.045)));
    eventDecay = static_cast<float>(std::exp(-1.0 / (sampleRate * 0.055)));

    fmVoice.prepare(sampleRate);
    harmonicVoice.prepare(sampleRate);
    granularCapture.prepare(sampleRate);
    percussion.prepare(sampleRate);
    fractal.prepare(sampleRate);
    vector.prepare(sampleRate);
    lorenz.prepare(sampleRate);
    doubleScroll.prepare(sampleRate);
    tripleDrift.prepare(sampleRate);
    fluidMotion.prepare(sampleRate);
    airWaterMedium.prepare(sampleRate);
    lensGestureInterval = static_cast<std::uint32_t>(
        std::max(1.0, std::round(sampleRate / 200.0)));
    logicClock.prepare(sampleRate);
    eventExtractor.prepare(sampleRate);
    integrityFade.prepare(sampleRate);
    philosophyMorph.prepare(sampleRate);
    reeseVoice.prepare(sampleRate);
    dualPeakFilter.prepare(sampleRate);
    resonantFilterSequence.prepare(sampleRate);
    frequencyIsolator.prepare(sampleRate);
    interactionProcessor.prepare(sampleRate);
    powerStarve.prepare(sampleRate);
    polygonalOscillator.prepare(sampleRate);
    biomaBass.prepare(sampleRate);
    schoolWave.prepare(sampleRate);
    harmonicRipple.prepare(sampleRate);
    recentMixWavetable.prepare(sampleRate);
    frequencyShifter.prepare(sampleRate);
    crossFeedback.prepare(sampleRate);
    hunger.prepare(sampleRate);
    selfFeedback.prepare(sampleRate);
    waterInertia.prepare(sampleRate);
    spectralTransfer.prepare(sampleRate);
    envelopeContagion.prepare(sampleRate);
    noInputFeedback.prepare(sampleRate);
    lockInFocus.prepare(sampleRate);
    behaviourScenes.prepare(sampleRate);
    binauralSpatializer.prepare(sampleRate);
    accessRouter.prepare(sampleRate);
    contactCrackle.prepare(sampleRate);
    configureFixedMaterial();
    reset();
}

void IntegratedModuleBank::setSeed(std::uint32_t value) noexcept
{
    seed = value == 0u ? 1u : value;
    lorenz.setSeed(seed ^ 0x8da6b343u);
    doubleScroll.setSeed(seed ^ 0x4f725bc1u);
    tripleDrift.setSeed(seed ^ 0x13c6ef37u);
    gesture.setSeed(seed ^ 0x5a4c19e1u);
    lensGesture.setSeed(seed ^ 0x21f37c95u);
    pulseMask.setSeed(seed ^ 0xb28f04ddu);
    shiftRegister.setSeed(seed ^ 0x728ae12bu);
    probability.setSeed(seed ^ 0x3f21c67du);
    evolution.setSeed(seed ^ 0x91e10da5u);
    reeseVoice.setSeed(seed ^ 0xc31f49a7u);
    powerStarve.setSeed(seed ^ 0x6d2b79f5u);
    biomaBass.setSeed(seed ^ 0x9a17c36du);
    hunger.setSeed(seed ^ 0x41e29b75u);
    noInputFeedback.setSeed(seed ^ 0x0b7d5e31u);
    contactCrackle.setSeed(seed ^ 0x2e91c84fu);
    cellularAutomaton.configure(30u, seed ^ 0x68e31c47u);
    reset();
}

void IntegratedModuleBank::setControls(const Controls& value) noexcept
{
    for (std::size_t index = 0; index < value.size(); ++index)
        targetControls[index] = unit(value[index], targetControls[index]);
}

void IntegratedModuleBank::configureFixedMaterial() noexcept
{
    std::array<float, HarmonicMorphOscillator::maximumHarmonics> familyA {};
    std::array<float, HarmonicMorphOscillator::maximumHarmonics> familyB {};
    familyA[0] = 1.0f;
    familyA[2] = 0.32f;
    familyA[4] = 0.16f;
    for (std::size_t index = 0; index < 8; ++index)
        familyB[index] = (index % 2 == 0 ? 1.0f : -0.45f)
            / static_cast<float>(index + 1);
    static_cast<void>(harmonicVoice.setFamilies(familyA, familyB, 8));
    harmonicVoice.setLevel(0.55f);

    std::array<float, 768> sample {};
    for (std::size_t index = 0; index < sample.size(); ++index)
    {
        const auto time = static_cast<float>(index) / static_cast<float>(sample.size());
        const auto envelope = std::exp(-time * 9.0f);
        const auto body = std::sin(2.0f * std::numbers::pi_v<float>
            * (9.0f * time + 18.0f * time * time));
        sample[index] = body * envelope;
    }
    for (std::size_t slot = 0; slot < SamplePercussionVoice::maximumSamples; ++slot)
        static_cast<void>(percussion.loadSample(slot, sample.data(), sample.size()));

    fractal.setParameters({ 73.0f, 2.0f, 0.52f, 5 });
    vector.setMode(VectorGenerator::Mode::loop);
    vector.setRateHz(41.0f);
    vector.setShape({ 0.37f, 0.25f, true });
    fluidMotion.setRateHz(0.19f);
    fluidMotion.setMode(FluidMotion::OutputMode::frequencies);
    fluidMotion.setFeedback(0.28f, 2);
    logicClock.setParameters({ 72.0f, { 5, 4 }, { 7, 4 }, 0.12f,
                               FractionalLogicClock::Logic::Xor });
    eventExtractor.setParameters({ 0.015f, 0.12f, 0.025f, 5.0f, 90.0f });
    transposer.setParameters({ -12, 12, 1 });
    thresholdSelector.setLength(8);
    branchRouter.setShape(4, 4);

    std::array<NotePermutationMatrix::Note,
               NotePermutationMatrix::maximumNotes> notes {};
    constexpr std::array<int, 8> degrees { 0, 2, 3, 5, 7, 9, 10, 12 };
    for (std::size_t index = 0; index < degrees.size(); ++index)
        notes[index] = { degrees[index], 0, 0.45f + 0.06f * static_cast<float>(index) };
    static_cast<void>(permutation.setNotes(notes, degrees.size()));
    std::array<std::uint8_t, NotePermutationMatrix::maximumNotes> reverse {};
    for (std::size_t index = 0; index < degrees.size(); ++index)
        reverse[index] = static_cast<std::uint8_t>(degrees.size() - 1u - index);
    static_cast<void>(permutation.setRow(1, reverse));

    std::array<std::uint8_t, PatternRotationMutator::maximumSteps> rhythm {};
    constexpr std::array<std::size_t, 5> hits { 0, 3, 5, 10, 13 };
    for (const auto hit : hits) rhythm[hit] = 1;
    static_cast<void>(rotation.setPattern(rhythm, 16));
    rotation.setSchedule(2, 3);

    stepMemory.setLength(8);
    for (std::size_t index = 0; index < 8; ++index)
        static_cast<void>(stepMemory.setStep(index, {
            static_cast<float>(degrees[index]),
            0.25f + 0.08f * static_cast<float>(index % 4),
            static_cast<float>(index) / 7.0f * 2.0f - 1.0f,
            0.2f + 0.1f * static_cast<float>(index % 5)
        }));
    static_cast<void>(patternBank.save(0, stepMemory));
    for (std::size_t index = 0; index < 8; ++index)
        static_cast<void>(stepMemory.setStep(index, {
            static_cast<float>(degrees[(index * 3) % degrees.size()] - 5),
            0.55f, 0.0f, 0.6f
        }));
    static_cast<void>(patternBank.save(1, stepMemory));
    std::array<std::uint8_t, StepPatternBank::maximumChain> chain {};
    chain[0] = 0;
    chain[1] = 1;
    static_cast<void>(patternBank.setChain(chain, 2));
    static_cast<void>(patternBank.restore(0, stepMemory));
    randomLoop.setLength(16);
    shiftRegister.setLength(16);
    evolution.setLength(16);
    evolution.setMode(EvolutionLoopMachine::Mode::evolving);

    for (std::size_t index = 0; index < ConditionalStageMachine::stageCount; ++index)
    {
        const auto next = (index + 1u) % ConditionalStageMachine::stageCount;
        const auto jump = (index + 3u) % ConditionalStageMachine::stageCount;
        const auto condition = index % 3 == 0
            ? ConditionalStageMachine::Condition::dangerAbove
            : index % 3 == 1 ? ConditionalStageMachine::Condition::night
                             : ConditionalStageMachine::Condition::crackAbove;
        stages.setStage(index, {
            static_cast<float>(index) / 3.5f - 1.0f,
            static_cast<std::uint32_t>(2u + index % 4u), next, jump,
            condition, 0.55f
        });
    }
    driftCurve.setParameters({ 0.72f, 0.62f, DriftCurveShaper::Family::bezierEase });
    integrityFade.setThresholds(0.2f, 0.8f);
    integrityFade.setTransitionSeconds(0.35f);

    philosophySequencer.setLength(4);
    static_cast<void>(philosophySequencer.setStep(0, { 0.0f, 0.0f, 2 }));
    static_cast<void>(philosophySequencer.setStep(1, { 1.0f, 0.0f, 3 }));
    static_cast<void>(philosophySequencer.setStep(2, { 1.0f, 1.0f, 2 }));
    static_cast<void>(philosophySequencer.setStep(3, { 0.0f, 1.0f, 4 }));
    philosophyMorph.setTransitionSeconds(0.08f);
    for (std::size_t output = 0; output < PhilosophyMatrixMixer::outputCount; ++output)
        for (std::size_t source = 0; source < PhilosophyMatrixMixer::sourceCount; ++source)
            static_cast<void>(philosophyMatrix.setCell(output, source, {
                output == source ? 1.0f : (output + source) % 3 == 0 ? 0.22f : 0.0f,
                output == source ? 0.20f : 0.0f
            }));

    gesture.beginRecording();
    for (std::size_t index = 0; index < 1024; ++index)
    {
        const auto phase = static_cast<float>(index) / 1024.0f;
        static_cast<void>(gesture.record({
            MotionWaveBank::evaluate(MotionWaveBank::Shape::doubleSine, phase),
            MotionWaveBank::evaluate(MotionWaveBank::Shape::asymmetric, phase)
        }));
    }
    static_cast<void>(gesture.endRecording());
    gesture.setPlaybackSpeed(0.125f);
    gesture.setMutationAmount(0.002f);
    lensGesture.setPlaybackSpeed(1.0f);
    lensGesture.setMutationAmount(0.001f);
    airWaterMedium.setParameters({ 0.35f, 0.18f, 0.62f, 0.18f });
    driftHistory.setWindow(0.5f, 0.5f);
    signatureLock.reset({ 0.5f, 0.5f, 0.5f });
    presenceLatch.reset();
    presenceLatch.setRelevantEvents(
        static_cast<std::uint8_t>(PresenceStateLatch::Event::danger)
        | static_cast<std::uint8_t>(PresenceStateLatch::Event::integrity));
    priorityChoke.setThreshold(0.58f);
    doubleScroll.setParameters({ 0.72f, 0.58f, 0.62f, 0.68f });
    accessRouter.clearRoutes();
    static_cast<void>(accessRouter.setRoute(0, {
        0, 0, SemanticAccessRouter::SourceKind::audio,
        SemanticAccessRouter::DestinationKind::modulation,
        0.62f, 0.18f, true
    }));
    static_cast<void>(accessRouter.setRoute(1, {
        1, 3, SemanticAccessRouter::SourceKind::audio,
        SemanticAccessRouter::DestinationKind::audio,
        0.48f, 0.22f, true
    }));
    static_cast<void>(accessRouter.setRoute(2, {
        2, 1, SemanticAccessRouter::SourceKind::control,
        SemanticAccessRouter::DestinationKind::modulation,
        0.56f, 0.26f, true
    }));
    static_cast<void>(accessRouter.setRoute(3, {
        3, 1, SemanticAccessRouter::SourceKind::control,
        SemanticAccessRouter::DestinationKind::modulation,
        0.38f, -0.18f, true
    }));
    static_cast<void>(accessRouter.setRoute(4, {
        4, 2, SemanticAccessRouter::SourceKind::event,
        SemanticAccessRouter::DestinationKind::event,
        1.0f, 0.0f, true
    }));
    static_cast<void>(openRules.compile(
        "danger>0.62:evolution=0.82; day<0.18:gesture=0.72;"
        "crack>0.48:integrity=0.90; chaos>0.66:branch=0.78"));
    contactCrackle.setParameters({ 0.52f, 0.68f, 0.54f, 0.38f });

    reeseVoice.setParameters({ 55.0f, 24.0f, 0.58f });
    dualPeakFilter.setParameters({ 0.43f, 0.41f, 0.61f, 0.27f, 0.18f });
    resonantFilterSequence.setParameters({ 82.41f, 0.64f });
    frequencyIsolator.setParameters({ 1.0f, 0.48f, 0.34f });
    interactionProcessor.setParameters({ 0.72f, 0.58f, 0.66f,
        InteractionProcessor::Mode::ring });
    powerStarve.setParameters({ 0.0f, 0.72f, 0.54f });
    polygonalOscillator.setParameters({ 110.0f, 5u, 0.18f, 0.0f });

    for (std::size_t output = 0; output < BoundedRoutingMatrix::channelCount;
         ++output)
        for (std::size_t input = 0; input < BoundedRoutingMatrix::channelCount;
             ++input)
            static_cast<void>(routingMatrix.setCell(output, input, {
                output == input ? 1.0f : (output + input) % 4u == 0u ? 0.24f : 0.0f,
                output == input ? 0.12f : (output + input) % 3u == 0u ? 0.18f : 0.0f
            }));
    for (std::size_t index = 0; index < SharedParameterPool::valueCount; ++index)
        static_cast<void>(sharedParameterPool.setValue(index,
            std::sin(2.0f * std::numbers::pi_v<float>
                * static_cast<float>(index) / SharedParameterPool::valueCount)));
    for (std::size_t consumer = 0;
         consumer < SharedParameterPool::consumerCount; ++consumer)
        static_cast<void>(sharedParameterPool.setRoute(consumer, {
            consumer, (consumer * 3u + 5u) % SharedParameterPool::valueCount,
            0.18f + 0.08f * static_cast<float>(consumer)
        }));
    sharedParameterPool.setMode(SharedParameterPool::Mode::rotate);
    granularCapture.setOnsetParameters({ 2.8f, 0.025f, 85.0f, true });
    biomaBass.setParameters({ 55.0f, 1.5f, 0.42f, 21.0f, -12.0f,
                              0.38f, 0.45f, 0.52f, 4u, 0.62f });
    schoolWave.setParameters({ 110.0f, 0.45f, 0.5f, 0.35f, 0.55f });
    harmonicRipple.setParameters({ 110.0f, 5u, 0.48f, 0.42f, 0.55f });
    recentMixWavetable.setParameters({ 82.41f, 0.42f, 0.55f });
    frequencyShifter.setParameters({ 47.0f, 0.72f });
    crossFeedback.setParameters({ 0.10f, 0.42f, 0.04f, 0.62f });
    hunger.setParameters({ 0.0f, 0.65f, 0.52f });
    selfFeedback.setParameters({ 0.35f, 0.46f, 0.42f, 0.55f });
    waterInertia.setParameters({ 0.58f, 0.66f, 0.15f });
    static_cast<void>(lSystem.configure(2u, false));
    cellularAutomaton.configure(30u, seed ^ 0x68e31c47u);
    spectralTransfer.setParameters({ 0.48f, 0.68f, 0.55f, 0.38f, 0.72f });
    noInputFeedback.setParameters({ 0.90f, 0.62f, 0.48f, 0.52f });
    reactiveStrategy.setThresholds({ 0.10f, 0.22f, 0.34f, 0.47f,
                                     0.60f, 0.74f, 0.88f }, 0.035f);
    lockInFocus.setParameters({ 110.0f, 10.0f, 0.72f, 0.78f });
    for (std::size_t scene = 0; scene < BehaviourSceneBank::sceneCount; ++scene)
        static_cast<void>(behaviourScenes.setScene(scene, {
            static_cast<float>(scene) / 7.0f,
            static_cast<float>((scene * 3u) % 8u) / 7.0f,
            static_cast<float>((scene * 5u + 1u) % 8u) / 7.0f,
            static_cast<float>(7u - scene) / 7.0f
        }));
    for (std::size_t index = 0; index < ReconfigurableResourcePool::valueCount;
         ++index)
        static_cast<void>(reconfigurablePool.setValue(index,
            std::sin(2.0f * std::numbers::pi_v<float>
                * static_cast<float>(index)
                / static_cast<float>(ReconfigurableResourcePool::valueCount))));
    reconfigurablePool.setMode(ReconfigurableResourcePool::Mode::mixer);
    binauralSpatializer.setParameters({ 0.0f, 0.0f, 0.5f, 0.68f });
}

void IntegratedModuleBank::reset() noexcept
{
    currentControls = targetControls;
    gesture.rewind();
    eventCount = 0;
    mutationCount = 0;
    pulsesInBar = 0;
    eventEnvelope = 0.0f;
    lastPitchRatio = 1.0f;
    lastPatternValue = 0.0f;
    lastEvolutionValue = 0.0f;
    lastStageValue = 0.0f;
    lastProbabilityValue = 0.0f;
    lastPhilosophyValue = 0.0f;
    lastLSystemValue = 0.0f;
    lastCellularValue = 0.0f;
    lastReactiveValue = 0.0f;
    lastResourceValue = 0.0f;
    lastFeedbackModulation = 0.0f;
    lastAccessModulation = 0.0f;
    lastRuleBranch = 0.0f;
    lastRuleEvolution = 0.0f;
    lastRuleGesture = 0.0f;
    lastRuleIntegrity = 0.0f;
    lastMediumWaterMix = 0.0f;
    lensWasHeld = false;
    lensGestureCountdown = 0u;
    lastLensGesture = {};
    lastDoubleScroll = {};
    lastQuantizedSubSemitones = -12.0f;
    lastGrainParameters = {};
    currentContext = {};
    contextInitialized = false;
    energy = 0.0f;
    fmVoice.reset();
    harmonicVoice.reset();
    granularCapture.reset();
    pulseMask.reset();
    fractal.reset();
    vector.reset();
    lorenz.reset();
    doubleScroll.reset();
    tripleDrift.reset();
    fluidMotion.reset();
    airWaterMedium.reset();
    lensGesture.clear();
    logicClock.reset();
    eventExtractor.reset();
    transposer.reset();
    thresholdSelector.reset();
    branchRouter.reset();
    permutation.reset();
    rotation.resetSchedule();
    memoryLayout.setMode(StepMemoryLayout::Mode::parallel4x8);
    patternBank.resetChain();
    randomLoop.reset();
    shiftRegister.reset(0xa55au);
    probability.reset();
    evolution.reset();
    stages.reset();
    integrityFade.reset();
    philosophySequencer.reset();
    philosophyMorph.reset();
    driftHistory.clear();
    signatureLock.setLocked(false);
    presenceLatch.reset();
    reeseVoice.reset();
    dualPeakFilter.reset();
    resonantFilterSequence.reset();
    frequencyIsolator.reset();
    interactionProcessor.reset();
    powerStarve.reset();
    polygonalOscillator.reset();
    sharedParameterPool.reset();
    biomaBass.reset();
    schoolWave.reset();
    harmonicRipple.reset();
    recentMixWavetable.reset();
    frequencyShifter.reset();
    crossFeedback.reset();
    hunger.reset();
    selfFeedback.reset();
    waterInertia.reset();
    lSystem.reset();
    cellularAutomaton.reset();
    spectralTransfer.reset();
    envelopeContagion.reset();
    noInputFeedback.reset();
    reactiveStrategy.reset();
    lockInFocus.reset();
    behaviourScenes.reset();
    reconfigurablePool.reset();
    binauralSpatializer.reset();
    accessRouter.reset();
    contactCrackle.reset();
}

void IntegratedModuleBank::advanceEvent(const Context& context, float lorenzX,
                                        float fluidValue,
                                        float probabilityValue) noexcept
{
    ++eventCount;
    eventEnvelope = 1.0f;
    const auto ruleResult = openRules.evaluate({
        context.density, context.danger, context.dayPhase,
        context.seasonPhase, context.crackStress,
        std::abs(lastDoubleScroll.x)
    });
    lastRuleBranch = ruleResult.active[0] ? ruleResult.value[0] : 0.0f;
    lastRuleEvolution = ruleResult.active[1] ? ruleResult.value[1] : 0.0f;
    lastRuleGesture = ruleResult.active[2] ? ruleResult.value[2] : 0.0f;
    lastRuleIntegrity = ruleResult.active[3] ? ruleResult.value[3] : 0.0f;
    static_cast<void>(contactCrackle.trigger({
        lorenzX, fluidValue,
        0.18f + context.crackStress * 0.72f,
        0.25f + context.agility * 0.70f,
        static_cast<std::uint32_t>(eventCount) ^ seed
    }));
    const auto formalEvent = lSystem.advance();
    const auto cellularEvent = cellularAutomaton.advance();
    lastLSystemValue = formalEvent.value;
    lastCellularValue = cellularEvent.density * 2.0f - 1.0f;
    const auto reactive = reactiveStrategy.tick(
        std::max(context.danger, context.crackStress));
    lastReactiveValue = reactive.value * 2.0f - 1.0f;
    if (reactive.changed || eventCount == 1u)
        static_cast<void>(behaviourScenes.recall(reactive.state,
            0.08f + context.density * 0.42f));
    const auto transposed = transposer.advance(context.seasonPhase);
    const auto feedbackBias = std::clamp(lastFeedbackModulation, -1.0f, 1.0f);
    const auto selected = thresholdSelector.select(unit(
        context.dayPhase + feedbackBias * 0.08f));
    const auto routed = branchRouter.select(
        unit(lorenzX * 0.5f + 0.5f + feedbackBias * 0.06f),
        unit(fluidValue * 0.5f + 0.5f - feedbackBias * 0.04f));
    static_cast<void>(permutation.selectRow(routed.index % 2u));
    const auto note = permutation.advance(transposed.degreeOffset / 7,
                                          0.55f + context.density * 0.45f);

    const auto mode = selected.index < 3 ? StepMemoryLayout::Mode::parallel4x8
        : selected.index < 6 ? StepMemoryLayout::Mode::serial2x16
                             : StepMemoryLayout::Mode::serial1x32;
    memoryLayout.setMode(mode);
    const auto address = memoryLayout.map(routed.category,
                                          eventCount + routed.variant);
    const auto step = stepMemory.address(
        static_cast<float>(address.index) / 31.0f);
    const auto semitones = static_cast<float>(note.note.degree
        + note.note.octave * 12 + transposed.degreeOffset)
        + step.step.pitchSemitones * 0.35f
        + static_cast<float>(formalEvent.turn) * 2.0f
        + lastCellularValue * 1.5f;
    lastPitchRatio = std::exp2(std::clamp(semitones, -36.0f, 36.0f) / 12.0f);

    if (++pulsesInBar >= 8u)
    {
        pulsesInBar = 0;
        const auto mutation = rotation.advanceBar();
        if (mutation.mutated)
        {
            ++mutationCount;
            const auto chain = patternBank.advanceChain();
            static_cast<void>(patternBank.restore(chain.slot, stepMemory));
        }
    }
    lastPatternValue = rotation.at(eventCount % rotation.length()) ? 1.0f : -1.0f;
    const auto looped = randomLoop.process(probabilityValue,
                                           amount(patternLoop));
    const auto shifted = shiftRegister.advance(looped.value >= 0.0f,
                                               amount(shiftProbability) * 0.45f);
    lastProbabilityValue = shifted.outputBit ? 1.0f : -1.0f;
    const auto resourceMode = static_cast<ReconfigurableResourcePool::Mode>(
        routed.variant % 4u);
    reconfigurablePool.setMode(resourceMode);
    lastResourceValue = reconfigurablePool.tick(probabilityValue,
                                                 context.dayPhase, true);

    evolution.setMode(amount(evolutionStages) < 0.25f
        ? EvolutionLoopMachine::Mode::looping
        : amount(evolutionStages) < 0.75f
            ? EvolutionLoopMachine::Mode::evolving
            : EvolutionLoopMachine::Mode::random);
    lastEvolutionValue = evolution.advance(
        amount(evolutionStages) * 0.35f).value;
    const auto stage = stages.tick({ context.danger,
        context.dayPhase < 0.25f || context.dayPhase > 0.75f,
        context.crackStress });
    lastStageValue = driftCurve.shape(stage.value * 0.5f + 0.5f) * 2.0f - 1.0f;

    const auto philosophy = philosophySequencer.tick(true);
    static_cast<void>(philosophySequencer.tick(false));
    if (philosophy.advanced)
        philosophyMorph.setTarget(philosophy.index);

    const auto masked = pulseMask.process(true,
        0.35f + amount(percussionPulses) * 0.65f);
    static_cast<void>(pulseMask.process(false, 1.0f));
    if (masked.gate && (formalEvent.gate || cellularEvent.gate))
        static_cast<void>(percussion.trigger(routed.variant % 4u,
            semitones * 0.18f, 0.035f + step.step.gateDuration * 0.20f,
            note.note.velocity));
    const auto traitDuration = 8.0f + (1.0f - context.agility) * 92.0f;
    const auto traitSpeed = std::exp2((context.intelligence - 0.5f) * 1.5f);
    const auto traitPan = std::clamp(fluidValue * (0.35f + context.agility * 0.65f)
        + (context.strength - 0.5f) * 0.35f, -1.0f, 1.0f);
    lastGrainParameters = {
        traitDuration * (0.72f + amount(granularMemory) * 0.78f),
        traitSpeed * (0.78f + amount(granularMemory) * 0.44f),
        traitPan, 0.18f + context.strength * 0.52f
            + amount(granularMemory) * 0.18f,
        (routed.variant % 2u != 0u) != (context.intelligence > 0.72f)
    };
    static_cast<void>(granularCapture.trigger(lastGrainParameters));

    const auto frequency = std::clamp(
        context.baseFrequencyHz * lastPitchRatio, 24.0f,
        static_cast<float>(sampleRate * 0.20));
    fmVoice.setParameters({ frequency, frequency * (1.0f + routed.variant * 0.25f),
        frequency * (0.15f + amount(synthBodies) * 1.5f),
        amount(synthBodies), 0.55f });
    harmonicVoice.setFrequency(frequency * 0.5f);
    harmonicVoice.setMorph(unit(looped.value * 0.5f + 0.5f));
    vector.setRateHz(frequency * 0.25f);
    vector.setShape({ 0.15f + 0.70f * context.dayPhase,
        bipolar(context.danger), true });
    reeseVoice.setParameters({ frequency * 0.5f,
        8.0f + 40.0f * amount(synthBodies),
        0.25f + 0.65f * context.danger });
    dualPeakFilter.setParameters({
        unit(std::log2(frequency / 55.0f) / 7.0f),
        0.18f + 0.68f * context.seasonPhase,
        0.40f + 0.50f * amount(synthBodies),
        0.12f + 0.42f * context.danger,
        routed.variant % 2u != 0u ? 0.65f : 0.15f
    });
    resonantFilterSequence.setParameters({ frequency * 0.5f,
        0.40f + 0.52f * amount(clockEvents) });
    frequencyIsolator.setParameters({ 1.0f,
        unit(std::log2(frequency / 45.0f) / 8.0f),
        0.16f + 0.72f * (1.0f - context.density) });
    interactionProcessor.setParameters({
        0.35f + 0.60f * amount(branchPermute),
        0.30f + 0.65f * context.danger,
        0.35f + 0.60f * context.danger,
        routed.category % 2u != 0u
            ? InteractionProcessor::Mode::xorLogic
            : InteractionProcessor::Mode::ring
    });
    polygonalOscillator.setParameters({ frequency,
        3u + routed.variant % 8u,
        0.08f + 0.78f * amount(fractalVector),
        context.seasonPhase
    });
    const auto subDegree = -7 - static_cast<int>(routed.variant % 7u)
        - static_cast<int>(std::lround(feedbackBias * 2.0f));
    lastQuantizedSubSemitones = std::clamp(
        intervalQuantizer.semitonesForDegree(subDegree), -36.0f, -5.0f);
    biomaBass.setParameters({ frequency * 0.5f,
        0.5f + 2.5f * context.dayPhase,
        0.18f + 0.78f * amount(synthBodies),
        8.0f + 42.0f * context.danger,
        lastQuantizedSubSemitones,
        0.18f + 0.72f * amount(synthBodies),
        0.28f + 0.58f * context.density,
        0.32f + 0.60f * context.danger,
        2u + routed.variant % 3u,
        0.35f + 0.60f * context.density
    });
    schoolWave.setParameters({ frequency,
        unit(looped.value * 0.5f + 0.5f),
        context.dayPhase,
        0.12f + 0.78f * amount(synthBodies),
        0.48f
    });
    harmonicRipple.setParameters({ frequency,
        2u + routed.variant % 7u,
        context.seasonPhase,
        0.18f + 0.76f * amount(percussionPulses),
        0.52f
    });
    recentMixWavetable.setParameters({ frequency * 0.75f,
        context.danger, 0.52f });
    frequencyShifter.setParameters({
        (bipolar(context.seasonPhase) * 240.0f)
            * (0.2f + amount(fractalVector) * 0.8f),
        0.35f + amount(fractalVector) * 0.60f
    });
    selfFeedback.setParameters({
        0.12f + context.danger * 0.74f,
        context.dayPhase,
        0.20f + amount(lorenzDrift) * 0.68f,
        0.28f + amount(lorenzDrift) * 0.58f
    });
    waterInertia.setParameters({
        0.22f + context.density * 0.68f,
        0.30f + amount(fluidPhase) * 0.62f,
        bipolar(context.seasonPhase) * 0.72f
    });
    spectralTransfer.setParameters({
        unit(std::log2(frequency / 55.0f) / 7.0f),
        0.28f + context.density * 0.66f,
        0.32f + context.danger * 0.58f,
        0.18f + context.seasonPhase * 0.68f,
        0.38f + amount(synthBodies) * 0.56f
    });
    noInputFeedback.setParameters({
        0.72f + context.crackStress * 0.255f,
        0.25f + context.danger * 0.70f,
        context.seasonPhase,
        0.24f + amount(integrityMemory) * 0.58f
    });
    lockInFocus.setParameters({
        frequency,
        2.0f + (1.0f - context.density) * 32.0f,
        0.35f + amount(granularMemory) * 0.60f,
        0.45f + context.danger * 0.48f
    });
    binauralSpatializer.setParameters({
        bipolar(context.dayPhase),
        bipolar(context.seasonPhase),
        0.18f + context.density * 0.78f,
        0.35f + amount(fluidPhase) * 0.60f
    });

    signatureLock.setLocked(amount(integrityMemory) > 0.66f);
    signatureLock.update({ amount(lorenzDrift), amount(gestureWaves),
                           amount(clockEvents) });
    presenceLatch.request(context.danger > 0.66f
        ? Ecosystem::Presence::Leaving : Ecosystem::Presence::Present);
    static_cast<void>(presenceLatch.trigger(context.crackStress > 0.5f
        ? PresenceStateLatch::Event::integrity
        : PresenceStateLatch::Event::danger));
}

IntegratedModuleBank::Sample IntegratedModuleBank::process(
    float inputLeft, float inputRight, const Context& rawContext) noexcept
{
    Context targetContext;
    targetContext.density = unit(rawContext.density, 0.5f);
    targetContext.danger = unit(rawContext.danger, 0.5f);
    targetContext.dayPhase = unit(rawContext.dayPhase);
    targetContext.seasonPhase = unit(rawContext.seasonPhase);
    targetContext.crackStress = unit(rawContext.crackStress);
    targetContext.baseFrequencyHz = numeric::clampFinite(
        rawContext.baseFrequencyHz, 24.0f,
        static_cast<float>(sampleRate * 0.20), 110.0f);
    targetContext.strength = unit(rawContext.strength, 0.5f);
    targetContext.agility = unit(rawContext.agility, 0.5f);
    targetContext.intelligence = unit(rawContext.intelligence, 0.5f);
    targetContext.feedbackAudio = numeric::clampFinite(
        rawContext.feedbackAudio, -1.0f, 1.0f, 0.0f);
    targetContext.temperature = unit(rawContext.temperature, 0.5f);
    targetContext.turbidity = unit(rawContext.turbidity, 0.35f);
    targetContext.verticalPosition = numeric::clampFinite(
        rawContext.verticalPosition, -1.0f, 1.0f, 0.0f);
    targetContext.lensX = numeric::clampFinite(
        rawContext.lensX, -1.0f, 1.0f, 0.0f);
    targetContext.lensY = numeric::clampFinite(
        rawContext.lensY, -1.0f, 1.0f, 0.0f);
    targetContext.compositionInfluence = unit(rawContext.compositionInfluence);
    targetContext.compositionPulse = numeric::clampFinite(
        rawContext.compositionPulse, 0.25f, 1.25f, 1.0f);
    targetContext.compositionTonal = numeric::clampFinite(
        rawContext.compositionTonal, 0.25f, 1.25f, 1.0f);
    targetContext.compositionTexture = numeric::clampFinite(
        rawContext.compositionTexture, 0.25f, 1.25f, 1.0f);
    targetContext.compositionMovement = numeric::clampFinite(
        rawContext.compositionMovement, 0.25f, 1.25f, 1.0f);
    targetContext.lensHeld = rawContext.lensHeld;
    for (std::size_t index = 0; index < currentControls.size(); ++index)
    {
        currentControls[index] += (targetControls[index] - currentControls[index])
            * smoothing;
        if (std::abs(targetControls[index] - currentControls[index]) < 1.0e-7f)
            currentControls[index] = targetControls[index];
    }

    // Zero-cost semantic bypass: the new laboratory cannot disturb the
    // established instrument or consume its real-time budget until invited.
    if (currentControls[mix] == 0.0f && targetControls[mix] == 0.0f)
        return {};

    if (!contextInitialized)
    {
        currentContext = targetContext;
        contextInitialized = true;
    }
    const auto follow = [this](float& current, float target) noexcept {
        current += (target - current) * contextSmoothing;
    };
    follow(currentContext.density, targetContext.density);
    follow(currentContext.danger, targetContext.danger);
    follow(currentContext.dayPhase, targetContext.dayPhase);
    follow(currentContext.seasonPhase, targetContext.seasonPhase);
    follow(currentContext.crackStress, targetContext.crackStress);
    follow(currentContext.baseFrequencyHz, targetContext.baseFrequencyHz);
    follow(currentContext.strength, targetContext.strength);
    follow(currentContext.agility, targetContext.agility);
    follow(currentContext.intelligence, targetContext.intelligence);
    follow(currentContext.temperature, targetContext.temperature);
    follow(currentContext.turbidity, targetContext.turbidity);
    follow(currentContext.verticalPosition, targetContext.verticalPosition);
    follow(currentContext.lensX, targetContext.lensX);
    follow(currentContext.lensY, targetContext.lensY);
    follow(currentContext.compositionInfluence, targetContext.compositionInfluence);
    follow(currentContext.compositionPulse, targetContext.compositionPulse);
    follow(currentContext.compositionTonal, targetContext.compositionTonal);
    follow(currentContext.compositionTexture, targetContext.compositionTexture);
    follow(currentContext.compositionMovement, targetContext.compositionMovement);
    currentContext.feedbackAudio = targetContext.feedbackAudio;
    currentContext.lensHeld = targetContext.lensHeld;
    const auto context = currentContext;

    if (context.lensHeld && !lensWasHeld)
    {
        lensGesture.beginRecording();
        lensGestureCountdown = 0u;
    }
    else if (!context.lensHeld && lensWasHeld)
    {
        static_cast<void>(lensGesture.endRecording());
        lensGestureCountdown = 0u;
    }
    lensWasHeld = context.lensHeld;
    if (lensGestureCountdown == 0u)
    {
        if (context.lensHeld)
            static_cast<void>(lensGesture.record({ context.lensX,
                                                   context.lensY }));
        else
            lastLensGesture = lensGesture.tick();
        lensGestureCountdown = lensGestureInterval;
    }
    --lensGestureCountdown;

    const auto input = std::clamp((inputLeft + inputRight) * 0.5f, -1.0f, 1.0f);
    const auto feedbackState = crossFeedback.tick(
        context.feedbackAudio, context.danger);
    lastFeedbackModulation = feedbackState.modulation;
    const auto detected = eventExtractor.tick(input);
    const auto clock = logicClock.tick();
    const auto lorenzFrame = lorenz.tick();
    lastDoubleScroll = doubleScroll.tick();
    const auto driftFrame = tripleDrift.tick();
    const auto fluidFrame = fluidMotion.tick();
    const auto gestureFrame = gesture.tick();
    const auto probabilityValue = probability.sample(amount(shiftProbability));
    if (detected.envelopeRise)
        static_cast<void>(recentMixWavetable.triggerCapture());
    if (clock.trigger || detected.envelopeRise)
        advanceEvent(context, lorenzFrame.x, fluidFrame.value[0], probabilityValue);

    integrityFade.setStress(context.crackStress);
    const auto integrity = integrityFade.tick();
    const auto fm = fmVoice.tick();
    const auto harmonic = harmonicVoice.tick();
    const auto granular = granularCapture.process(input);
    const auto percussionSample = percussion.tick();
    const auto fractalSample = fractal.tick();
    const auto vectorSample = vector.tick();
    const auto reeseSample = reeseVoice.tick();
    const auto filteredReese = dualPeakFilter.tick(reeseSample);
    const auto isolatedInput = frequencyIsolator.tick(input);
    const auto filterSequence = resonantFilterSequence.tick(
        input + harmonic * 0.35f,
        context.dayPhase + amount(clockEvents) * 0.25f,
        context.danger);
    const auto polygon = polygonalOscillator.tick();
    const auto interaction = interactionProcessor.tick(
        filteredReese, harmonic, eventEnvelope, context.danger);
    const auto quadrature = QuadratureStereoRotator::process(
        fm * fluidFrame.value[0], harmonic * fluidFrame.value[3],
        context.seasonPhase + fluidFrame.value[2] * 0.125f);
    const auto medium = airWaterMedium.process(
        quadrature.left, quadrature.right, context.verticalPosition,
        context.turbidity, context.temperature);
    lastMediumWaterMix = medium.waterMix;
    const auto routedEmergence = routingMatrix.process({
        filteredReese, polygon.audio, filterSequence.value,
        interaction.artifact, isolatedInput
    }, context.danger);
    const auto sharedValue = sharedParameterPool.read(
        static_cast<std::size_t>(eventCount % SharedParameterPool::consumerCount),
        context.dayPhase, clock.trigger);
    const auto bassSample = biomaBass.tick();
    const auto schoolSample = schoolWave.tick(lorenzFrame.x,
                                               context.seasonPhase);
    const auto rippleSample = harmonicRipple.tick();
    const auto resynth = recentMixWavetable.tick(input);
    const auto shiftedSample = frequencyShifter.tick(
        harmonic * 0.55f + polygon.audio * 0.45f);
    const auto feedbackSample = selfFeedback.tick(
        lorenzFrame.y * 0.45f + driftFrame.combined * 0.35f
            + filteredReese * 0.20f);
    const auto waterValue = waterInertia.tick(fluidFrame.value[1]);
    const auto spotlight = SpotlightMixer::process({
        fm, harmonic, bassSample, rippleSample, resynth.value
    }, context.dayPhase + context.seasonPhase * 0.25f, context.danger);
    hunger.setParameters({ context.density,
        0.35f + amount(integrityMemory) * 0.60f,
        0.25f + context.danger * 0.70f });
    const auto hungryBass = hunger.tick(bassSample);
    const auto contagion = envelopeContagion.process({
        fm, harmonic, bassSample, rippleSample, filteredReese, percussionSample
    }, context.density * (0.25f + amount(synthBodies) * 0.70f));
    const auto vocoded = spectralTransfer.tick(input,
        schoolSample.left * 0.5f + schoolSample.right * 0.5f);
    const auto structuralVoice = noInputFeedback.tick();
    const auto focusedInput = lockInFocus.tick(input);
    const auto behaviourScene = behaviourScenes.tick();
    const auto liveResource = reconfigurablePool.tick(driftFrame.combined,
        context.dayPhase + context.seasonPhase * 0.25f, false);
    const auto binaural = binauralSpatializer.tick(
        quadrature.left * 0.5f + quadrature.right * 0.5f);
    const auto contact = contactCrackle.tick();
    const auto gestureWave = MotionWaveBank::evaluate(
        MotionWaveBank::Shape::asymmetric,
        gestureFrame.active ? unit(gestureFrame.point.x * 0.5f + 0.5f) : 0.0f);
    const auto lensGestureWave = lastLensGesture.active
        ? MotionWaveBank::evaluate(MotionWaveBank::Shape::doubleSine,
            unit(lastLensGesture.point.x * 0.5f + 0.5f))
        : 0.0f;
    const auto loopHistory = driftHistory.tick(
        lorenzFrame.y * 0.55f + driftFrame.combined * 0.45f);
    if (!loopHistory.looping && driftHistory.captured() >= 2048
        && amount(integrityMemory) > 0.5f)
        static_cast<void>(driftHistory.beginLoop());
    else if (loopHistory.looping && amount(integrityMemory) <= 0.5f)
        driftHistory.endLoop();

    const std::array<float, 4> philosophySources {
        fm, harmonic, fractalSample, percussionSample
    };
    const auto philosophy = philosophyMorph.tick(philosophySources);
    lastPhilosophyValue = philosophy.mix;
    const auto matrix = philosophyMatrix.process(philosophySources, {
        lorenzFrame.x, driftFrame.combined, fluidFrame.value[1],
        gestureFrame.active ? gestureFrame.point.y : 0.0f
    });
    const auto prominence = std::array<float, PriorityChoke::maximumChannels> {
        std::abs(fm), std::abs(harmonic), std::abs(fractalSample),
        std::abs(percussionSample), std::abs(granular.left),
        std::abs(loopHistory.value), std::abs(lastEvolutionValue),
        std::abs(lastStageValue)
    };
    const auto choke = priorityChoke.process(prominence, prominence.size());
    const auto selected = choke.selected >= 0
        ? philosophySources[static_cast<std::size_t>(choke.selected) % 4u]
        : lastPhilosophyValue;
    powerStarve.setParameters({ context.crackStress,
        0.45f + 0.48f * amount(integrityMemory),
        0.30f + 0.64f * context.danger });
    const auto starvedMemory = powerStarve.tick(
        matrix.channel[0] * 0.6f + selected * 0.4f);
    SemanticAccessRouter::Inputs accessInput {};
    accessInput[0].audio = input;
    accessInput[1].audio = (harmonic + fm) * 0.5f;
    accessInput[2].control = lastDoubleScroll.x;
    accessInput[3].control = lastRuleBranch - lastRuleIntegrity;
    accessInput[4].event = clock.trigger || detected.envelopeRise;
    const auto access = accessRouter.process(accessInput, context.danger);
    lastAccessModulation = access[1].modulation;

    eventEnvelope *= eventDecay;
    std::array<Sample, controlCount - 1> family {};
    family[synthBodies - 1] = {
        fm * (0.14f + contagion[0] * 0.08f)
            + harmonic * (0.12f + contagion[1] * 0.07f)
            + filteredReese * 0.16f + hungryBass * 0.20f
            + schoolSample.left * 0.15f + vocoded * 0.11f,
        fm * (0.13f + contagion[0] * 0.07f)
            - harmonic * (0.11f + contagion[1] * 0.07f)
            + reeseSample * 0.16f + hungryBass * 0.19f
            + schoolSample.right * 0.18f - vocoded * 0.11f
    };
    family[granularMemory - 1] = {
        granular.left * 0.50f + isolatedInput * 0.16f
            + resynth.value * 0.19f + focusedInput * 0.15f,
        granular.right * 0.50f - isolatedInput * 0.16f
            + resynth.value * 0.19f + focusedInput * 0.15f
    };
    family[percussionPulses - 1] = {
        percussionSample * 0.68f + rippleSample * 0.32f,
        percussionSample * 0.68f - rippleSample * 0.32f
    };
    family[fractalVector - 1] = {
        fractalSample * 0.40f + vectorSample * 0.22f
            + polygon.audio * 0.16f + polygon.x * 0.14f
            + shiftedSample * 0.12f,
        fractalSample * 0.24f - vectorSample * 0.38f
            + polygon.audio * 0.16f + polygon.y * 0.14f
            - shiftedSample * 0.12f
    };
    family[lorenzDrift - 1] = {
        harmonic * (lorenzFrame.x + driftFrame.fast) * 0.34f
            + feedbackSample * 0.28f
            + harmonic * lastDoubleScroll.x * 0.14f,
        harmonic * (lorenzFrame.y + driftFrame.slow) * 0.34f
            - feedbackSample * 0.28f
            + harmonic * lastDoubleScroll.xComplement * 0.14f
    };
    family[fluidPhase - 1] = {
        medium.left * 0.52f + fm * waterValue * 0.24f
            + binaural.left * 0.24f,
        medium.right * 0.52f - harmonic * waterValue * 0.24f
            + binaural.right * 0.24f
    };
    family[gestureWaves - 1] = {
        harmonic * gestureWave
            + harmonic * lensGestureWave * 0.24f
            + contact.left * (0.24f + lastRuleGesture * 0.26f),
        harmonic * (gestureFrame.active
            ? gestureFrame.point.y : -gestureWave)
            + harmonic * (lastLensGesture.active
                ? lastLensGesture.point.y : -lensGestureWave) * 0.24f
            + contact.right * (0.24f + lastRuleGesture * 0.26f)
    };
    family[clockEvents - 1] = {
        harmonic * eventEnvelope * (0.43f + lastLSystemValue * 0.10f)
            + filterSequence.value * 0.42f,
        fm * eventEnvelope * (0.43f - lastLSystemValue * 0.10f)
            - filterSequence.value * 0.42f
    };
    family[transposeSelect - 1] = { fm * (lastPitchRatio - 0.5f) * 0.45f,
                                    harmonic * (1.5f - lastPitchRatio) * 0.45f };
    family[branchPermute - 1] = {
        harmonic * lastPitchRatio * 0.10f + interaction.mix * 0.26f
            + routedEmergence[0] * 0.22f + spotlight.left * 0.30f
            + selected * lastReactiveValue * 0.10f
            + access[3].audio * 0.12f
            + selected * (lastAccessModulation + lastRuleBranch) * 0.07f,
        fm / std::max(0.5f, lastPitchRatio) * 0.10f
            + interaction.artifact * 0.26f + routedEmergence[1] * 0.22f
            + spotlight.right * 0.30f - selected * lastReactiveValue * 0.10f
            - access[3].audio * 0.12f
            - selected * (lastAccessModulation + lastRuleBranch) * 0.07f
    };
    family[rotateLayout - 1] = {
        percussionSample * lastPatternValue * 0.58f
            + polygon.audio * sharedValue * 0.22f
            + harmonic * (lastResourceValue + liveResource) * 0.10f,
        -percussionSample * lastPatternValue * 0.58f
            + polygon.y * sharedValue * 0.22f
            - harmonic * (lastResourceValue + liveResource) * 0.10f
    };
    family[patternLoop - 1] = {
        harmonic * (lastPatternValue * 0.44f + lastCellularValue * 0.18f),
        harmonic * (loopHistory.value * 0.44f - lastCellularValue * 0.18f)
    };
    family[shiftProbability - 1] = { fm * lastProbabilityValue * 0.45f,
                                     harmonic * probabilityValue * 0.45f };
    family[evolutionStages - 1] = {
        harmonic * (lastEvolutionValue + lastStageValue) * 0.24f
            + bassSample * feedbackState.modulation * 0.18f
            + selected * (behaviourScene[0] - behaviourScene[1]) * 0.16f
            + selected * (lastRuleEvolution - lastRuleIntegrity) * 0.12f,
        fm * (lastEvolutionValue - lastStageValue) * 0.24f
            - rippleSample * feedbackState.modulation * 0.18f
            + selected * (behaviourScene[2] - behaviourScene[3]) * 0.16f
            - selected * (lastRuleEvolution - lastRuleIntegrity) * 0.12f
    };
    const auto presenceGain = presenceLatch.current() == Ecosystem::Presence::Leaving
        ? 0.45f : 1.0f;
    const auto signature = signatureLock.signature();
    family[integrityMemory - 1] = {
        (matrix.channel[0] * integrity.healthyGain
            + (starvedMemory * 0.52f + hungryBass * 0.28f
                + structuralVoice.left * 0.20f)
                * integrity.crackedGain) * presenceGain
            * (0.55f + signature.smoothing * 0.45f),
        (matrix.channel[1] * integrity.healthyGain
            + (matrix.channel[3] * 0.76f + structuralVoice.right * 0.24f)
                * integrity.crackedGain) * presenceGain
            * (0.55f + signature.curveFamily * 0.45f)
    };

    const auto phraseGain = [influence = context.compositionInfluence](float weight) {
        return 1.0f + (weight - 1.0f) * influence;
    };
    const auto applyPhraseGain = [&family, &phraseGain](Control control,
                                                       float weight) {
        auto& sample = family[static_cast<std::size_t>(control) - 1u];
        const auto gain = phraseGain(weight);
        sample.left *= gain;
        sample.right *= gain;
    };
    applyPhraseGain(synthBodies, context.compositionTonal);
    applyPhraseGain(transposeSelect, context.compositionTonal);
    applyPhraseGain(granularMemory, context.compositionTexture);
    applyPhraseGain(fractalVector, context.compositionTexture);
    applyPhraseGain(lorenzDrift, context.compositionMovement);
    applyPhraseGain(fluidPhase, context.compositionMovement);
    applyPhraseGain(gestureWaves, context.compositionMovement);
    applyPhraseGain(branchPermute, context.compositionTexture);
    applyPhraseGain(percussionPulses, context.compositionPulse);
    applyPhraseGain(clockEvents, context.compositionPulse);
    applyPhraseGain(rotateLayout, context.compositionPulse);
    applyPhraseGain(patternLoop, context.compositionPulse);
    applyPhraseGain(shiftProbability, context.compositionPulse);
    applyPhraseGain(evolutionStages, context.compositionTexture);
    applyPhraseGain(integrityMemory, context.compositionTexture);

    Sample result;
    for (std::size_t index = 1; index < controlCount; ++index)
    {
        const auto gain = currentControls[index] * (index <= 3 ? 0.12f : 0.075f);
        result.left += family[index - 1].left * gain;
        result.right += family[index - 1].right * gain;
    }
    result.left *= amount(mix);
    result.right *= amount(mix);
    if (!std::isfinite(result.left)) result.left = 0.0f;
    if (!std::isfinite(result.right)) result.right = 0.0f;
    result.left = std::clamp(result.left, -0.65f, 0.65f);
    result.right = std::clamp(result.right, -0.65f, 0.65f);
    const auto instantEnergy = std::sqrt(
        (result.left * result.left + result.right * result.right) * 0.5f);
    energy += (instantEnergy - energy) * 0.002f;
    return result;
}

IntegratedModuleBank::Telemetry IntegratedModuleBank::telemetry() const noexcept
{
    Telemetry result;
    result.target = targetControls;
    result.current = currentControls;
    result.events = eventCount;
    result.mutations = mutationCount;
    result.outputEnergy = energy;
    result.feedbackModulation = lastFeedbackModulation;
    result.doubleScrollX = lastDoubleScroll.x;
    result.accessModulation = lastAccessModulation;
    result.openRuleInfluence = std::max({
        lastRuleBranch, lastRuleEvolution,
        lastRuleGesture, lastRuleIntegrity
    });
    result.contactEnergy = contactCrackle.energy();
    result.mediumWaterMix = lastMediumWaterMix;
    result.lensGestureActive = lastLensGesture.active;
    result.quantizedSubSemitones = lastQuantizedSubSemitones;
    result.grainParameters = lastGrainParameters;
    result.smoothedContext = currentContext;
    for (std::size_t index = 1; index < currentControls.size(); ++index)
        result.activeFamilies += currentControls[index] > 0.0001f ? 1u : 0u;
    return result;
}
}
