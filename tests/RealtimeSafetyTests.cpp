// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/BiomaEngine.h"
#include "core/BiomaPhysicalString.h"
#include "core/FixedMpscQueue.h"
#include "core/GranularPerformance.h"
#include "app/AudioControlFrame.h"
#include "core/FixedLatestSnapshot.h"
#include "core/SlewRateRandomWalk.h"
#include "core/SynthFMVoice.h"
#include "core/LorenzCV.h"
#include "core/BiomaGranularCapture.h"
#include "core/BiomaMarkov.h"
#include "core/AquariumTail.h"
#include "core/ControlEventExtractor.h"
#include "core/FractionalLogicClock.h"
#include "core/AddressableStepMemory.h"
#include "core/HarmonicProgression.h"
#include "core/AccumulatingTransposer.h"
#include "core/ThresholdStepSelector.h"
#include "core/StepPatternBank.h"
#include "core/RandomToLoop.h"
#include "core/BranchingStepRouter.h"
#include "core/NotePermutationMatrix.h"
#include "core/BiomaRelabi.h"
#include "core/PatternRotationMutator.h"
#include "core/StepMemoryLayout.h"
#include "core/DriftCurveShaper.h"
#include "core/SamplePercussionVoice.h"
#include "core/HarmonicMorphOscillator.h"
#include "core/StochasticPulseMask.h"
#include "core/FractalCore.h"
#include "core/VectorGenerator.h"
#include "core/ShiftRegisterMemory.h"
#include "core/ProbabilityDistribution.h"
#include "core/EvolutionLoopMachine.h"
#include "core/ConditionalStageMachine.h"
#include "core/TripleSlowDrift.h"
#include "core/IntegrityCrossfade.h"
#include "core/HarmonicPhaseBank.h"
#include "core/FluidMotion.h"
#include "core/MotionWaveBank.h"
#include "core/MotionGestureRecorder.h"
#include "core/SchoolSpectralBank.h"
#include "core/EcologicalNoiseBed.h"
#include "core/PhilosophyMixEngine.h"
#include "core/BehaviourMemory.h"

#include <array>
#include <atomic>
#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <new>

namespace
{
std::atomic<bool> trackAllocations { false };
std::atomic<std::size_t> allocationCount { 0 };

void countAllocation() noexcept
{
    if (trackAllocations.load(std::memory_order_relaxed))
        allocationCount.fetch_add(1, std::memory_order_relaxed);
}
}

void* operator new(std::size_t size)
{
    countAllocation();
    if (auto* memory = std::malloc(size == 0 ? 1 : size))
        return memory;
    throw std::bad_alloc {};
}

void* operator new[](std::size_t size)
{
    return ::operator new(size);
}

void operator delete(void* memory) noexcept { std::free(memory); }
void operator delete[](void* memory) noexcept { std::free(memory); }
void operator delete(void* memory, std::size_t) noexcept { std::free(memory); }
void operator delete[](void* memory, std::size_t) noexcept { std::free(memory); }

int main()
{
    struct ControlEvent
    {
        int note = 0;
        float velocity = 0.0f;
    };

    aquorbium::BiomaEngine engine;
    engine.prepare(48000.0);
    engine.setSeed(0xA91B42C3u);
    engine.setDesire({ 0.32f, 0.74f, 0.68f, 0.57f });
    engine.setAutonomousEnabled(true);
    engine.noteOn(36, 0.8f);
    engine.noteOn(43, 0.7f);
    engine.noteOn(48, 0.6f);
    engine.noteOn(55, 0.5f);

    aquorbium::app::AudioControlFrame outputBoundary;
    outputBoundary.masterTrim = outputBoundary.masterCeiling
        = outputBoundary.masterRelease = 0.0f;
    outputBoundary.apply(engine, true);
    const auto minimumOutput = engine.outputStageParameters();
    if (minimumOutput.trimDb != -18.0f || minimumOutput.ceilingDb != -6.0f
        || minimumOutput.releaseMs != 50.0f)
        return EXIT_FAILURE;
    outputBoundary.masterTrim = outputBoundary.masterCeiling
        = outputBoundary.masterRelease = 1.0f;
    outputBoundary.apply(engine, true);
    const auto maximumOutput = engine.outputStageParameters();
    if (maximumOutput.trimDb != 6.0f
        || std::abs(maximumOutput.ceilingDb + 0.1f) > 1.0e-6f
        || maximumOutput.releaseMs != 500.0f)
        return EXIT_FAILURE;
    aquorbium::app::AudioControlFrame {}.apply(engine, true);

    aquorbium::BiomaPhysicalString physicalString;
    physicalString.prepare(48000.0);
    physicalString.setSeed(0x61A9204u);
    physicalString.setParameters({ 73.42f, 8.0f, 0.82f, 0.74f, 0.31f });

    aquorbium::SynthFMVoice fmVoice;
    fmVoice.prepare(48000.0);
    fmVoice.setParameters({ 90.0f, 175.0f, 220.0f, 0.4f, 0.4f });
    aquorbium::SlewRateRandomWalk randomWalk;
    randomWalk.prepare(48000.0);
    randomWalk.setSeed(0x92ac51u);
    aquorbium::LorenzCV lorenz;
    lorenz.prepare(48000.0);
    aquorbium::BiomaGranularCapture capture;
    capture.prepare(48000.0);
    aquorbium::BiomaMarkov markov;
    markov.setSeed(0x57ae91u);
    auto syntheticTail = std::make_unique<aquorbium::AquariumTail>();
    syntheticTail->prepare(48000.0);
    aquorbium::ControlEventExtractor eventExtractor;
    eventExtractor.prepare(48000.0);
    aquorbium::FractionalLogicClock fractionalClock;
    fractionalClock.prepare(48000.0);
    aquorbium::AddressableStepMemory stepMemory;
    stepMemory.setLength(13);
    aquorbium::HarmonicProgression harmonicProgression;
    harmonicProgression.setLength(7);
    aquorbium::AccumulatingTransposer accumulatingTransposer;
    aquorbium::ThresholdStepSelector thresholdSelector;
    thresholdSelector.setLength(13);
    aquorbium::StepPatternBank patternBank;
    (void)patternBank.save(0, stepMemory);
    std::array<std::uint8_t, aquorbium::StepPatternBank::maximumChain> patternChain {};
    patternChain[0] = 0;
    (void)patternBank.setChain(patternChain, 1);
    aquorbium::RandomToLoop randomToLoop;
    randomToLoop.setLength(29);
    aquorbium::BranchingStepRouter branchingRouter;
    branchingRouter.setShape(3, 4);
    aquorbium::NotePermutationMatrix permutationMatrix;
    aquorbium::BiomaRelabi relabi;
    relabi.prepare(200.0);
    aquorbium::PatternRotationMutator rotationMutator;
    rotationMutator.setSchedule(4, 1);
    aquorbium::StepMemoryLayout memoryLayout;
    aquorbium::DriftCurveShaper driftCurve;
    auto sampleVoice = std::make_unique<aquorbium::SamplePercussionVoice>();
    sampleVoice->prepare(48000.0);
    std::array<float, 64> percussionSample {};
    for (std::size_t index = 0; index < percussionSample.size(); ++index)
        percussionSample[index] = 1.0f
            - static_cast<float>(index) / static_cast<float>(percussionSample.size());
    (void)sampleVoice->loadSample(0, percussionSample.data(), percussionSample.size());
    aquorbium::HarmonicMorphOscillator harmonicMorph;
    harmonicMorph.prepare(48000.0);
    aquorbium::StochasticPulseMask pulseMask;
    pulseMask.setSeed(0x38a91u);
    aquorbium::FractalCore fractalCore;
    fractalCore.prepare(48000.0);
    fractalCore.setParameters({ 73.0f, 2.4f, 0.71f, 7 });
    aquorbium::VectorGenerator vectorGenerator;
    vectorGenerator.prepare(48000.0);
    vectorGenerator.setRateHz(31.0f);
    vectorGenerator.setShape({ 0.42f, 0.37f, true });
    aquorbium::ShiftRegisterMemory shiftMemory;
    shiftMemory.setLength(23);
    shiftMemory.setSeed(0x729a1u);
    shiftMemory.reset(0x52a91u);
    aquorbium::ProbabilityDistribution probabilityDistribution;
    probabilityDistribution.setSeed(0x92ab3u);
    aquorbium::EvolutionLoopMachine evolutionMachine;
    evolutionMachine.setLength(31);
    evolutionMachine.setSeed(0x389a2u);
    evolutionMachine.setMode(aquorbium::EvolutionLoopMachine::Mode::evolving);
    aquorbium::ConditionalStageMachine stageMachine;
    stageMachine.setStage(0, { 0.5f, 7, 1, 3,
        aquorbium::ConditionalStageMachine::Condition::dangerAbove, 0.6f });
    aquorbium::TripleSlowDrift tripleDrift;
    tripleDrift.prepare(48000.0);
    tripleDrift.setSeed(0x912a7u);
    aquorbium::IntegrityCrossfade integrityCrossfade;
    integrityCrossfade.prepare(48000.0);
    aquorbium::HarmonicPhaseBank phaseBank;
    phaseBank.prepare(48000.0);
    phaseBank.setRateHz(11.0f);
    aquorbium::FluidMotion fluidMotion;
    fluidMotion.prepare(48000.0);
    fluidMotion.setRateHz(7.0f);
    fluidMotion.setFeedback(0.7f, 1);
    aquorbium::MotionGestureRecorder gestureRecorder;
    gestureRecorder.setSeed(0x271a9u);
    gestureRecorder.beginRecording();
    for (int point = 0; point < 64; ++point)
    {
        const auto phase = static_cast<float>(point) / 64.0f;
        (void)gestureRecorder.record({
            aquorbium::MotionWaveBank::evaluate(
                aquorbium::MotionWaveBank::Shape::sine, phase),
            aquorbium::MotionWaveBank::evaluate(
                aquorbium::MotionWaveBank::Shape::triangle, phase)
        });
    }
    (void)gestureRecorder.endRecording();
    gestureRecorder.setMutationAmount(0.01f);
    aquorbium::SchoolSpectralBank spectralBank;
    spectralBank.prepare(48000.0);
    spectralBank.setIdentity(3u, 0x71a92u);
    spectralBank.setParameters({ 0.45f, 0.6f, 0.5f });
    aquorbium::EcologicalNoiseBed noiseBed;
    noiseBed.prepare(48000.0);
    noiseBed.setSeed(0x813a2u);
    noiseBed.setParameters({ 0.2f, 0.7f, 0.4f, 0.6f, 0.35f, 0.2f });
    aquorbium::PhilosophySequencer philosophySequence;
    philosophySequence.setLength(4);
    aquorbium::PhilosophyMorph philosophyMorph;
    philosophyMorph.prepare(48000.0);
    philosophyMorph.reset(0);
    philosophyMorph.setTarget(3);
    aquorbium::PhilosophyMatrixMixer philosophyMatrix;
    (void)philosophyMatrix.setCell(0, 1, { 0.5f, 0.25f });
    aquorbium::DriftHistoryLoop driftHistory;
    for (int point = 0; point < 64; ++point)
        (void)driftHistory.tick(static_cast<float>(point) / 63.0f);
    driftHistory.setWindow(0.25f, 0.5f);
    (void)driftHistory.beginLoop();
    aquorbium::MovementSignatureLock movementLock;
    movementLock.reset({ 0.4f, 0.6f, 0.2f });
    movementLock.setLocked(true);
    aquorbium::PresenceStateLatch presenceLatch;
    presenceLatch.request(aquorbium::Ecosystem::Presence::Leaving);
    aquorbium::PriorityChoke priorityChoke;
    float moduleProbe = 0.0f;

    std::array<float, 257> left {};
    std::array<float, 257> right {};
    aquorbium::FixedMpscQueue<ControlEvent, 256> controlEvents;
    aquorbium::FixedLatestSnapshot<aquorbium::app::AudioControlFrame> frames;
    aquorbium::app::AudioControlFrame applied;
    allocationCount.store(0, std::memory_order_relaxed);
    trackAllocations.store(true, std::memory_order_release);
    for (int block = 0; block < 500; ++block)
    {
        if (!controlEvents.tryPush({ 36 + (block % 24), 0.75f }))
            return EXIT_FAILURE;
        ControlEvent event;
        if (!controlEvents.tryPop(event) || event.velocity != 0.75f)
            return EXIT_FAILURE;
        const auto gesture = static_cast<float>(block % 101) / 100.0f;
        if (block == 100) engine.setEcosystemBlend(0.0f);
        if (block == 100)
            (void)capture.trigger({ 40.0f, 1.3f, -0.3f, 0.8f, false });
        if (block % 32 == 0)
            moduleProbe += static_cast<float>(markov.advance({ gesture, 0.5f })) * 0.001f;
        if (block % 64 == 0)
        {
            const auto chain = patternBank.advanceChain();
            moduleProbe += static_cast<float>(chain.slot) * 0.0001f;
            if (!patternBank.restore(chain.slot, stepMemory))
                return EXIT_FAILURE;
        }
        if (block % 16 == 0)
        {
            const auto rotation = rotationMutator.advanceBar();
            moduleProbe += rotation.mutated ? 0.0001f : 0.0f;
        }
        if (block % 11 == 0)
            (void)sampleVoice->trigger(0, gesture * 24.0f - 12.0f,
                                       0.02f + gesture * 0.2f, 0.8f);
        if (block == 300) engine.setEcosystemBlend(1.0f);
        aquorbium::app::AudioControlFrame frame;
        frame.density = gesture;
        frame.cohesion = gesture;
        frame.danger = 1.0f - gesture;
        frame.ecosystemMorph = gesture;
        frame.geometryInfluence = gesture;
        frame.strikerInfluence = gesture;
        frame.spaceReturn = gesture;
        frame.environmentInfluence = gesture;
        frame.environmentFilterMix = 1.0f - gesture;
        frame.environmentCentre = gesture;
        frame.environmentSpan = 1.0f - gesture;
        frame.environmentResonance = gesture;
        frame.environmentAmbience = gesture * 0.5f;
        frame.tailInfluence = gesture;
        frame.tailDecay = gesture;
        frame.tailDamping = 1.0f - gesture;
        frame.tailNightmare = gesture;
        frame.creativeMaster = gesture;
        frame.formInfluence = gesture;
        frame.formContext = 1.0f - gesture;
        frame.formInertia = gesture;
        frame.formMutation = 1.0f - gesture;
        frame.structureInfluence = gesture;
        frame.structureMemory = 1.0f - gesture;
        frame.structureHarmony = gesture;
        frame.structurePhase = 1.0f - gesture;
        frame.moduleBank[0] = 0.65f;
        for (std::size_t index = 1; index < frame.moduleBank.size(); ++index)
            frame.moduleBank[index] = std::fmod(
                gesture + static_cast<float>(index) * 0.071f, 1.0f);
        frame.lensHeld = block % 50 < 25;
        frame.lensX = gesture * 2.0f - 1.0f;
        frames.publish(frame);
        if (!frames.consumeLatest(applied))
            return EXIT_FAILURE;
        applied.apply(engine);
        engine.setCohesion(gesture);
        engine.setDanger(1.0f - gesture);
        engine.setListeningBlend({ gesture, 1.0f - gesture, 0.25f, 0.75f });
        engine.setLensGesture(block % 50 < 25,
                              { gesture * 2.0f - 1.0f, 0.0f, 1.0f });
        engine.setGranularStrikerPerformance(
            aquorbium::GranularPerformance::map(
                { gesture, 1.0f - gesture, gesture, gesture * 0.75f }));
        engine.setMultiTapSpaceReturn(gesture * 0.75f);
        engine.setMultiTapSpacePerformance(
            { gesture, gesture, 1.0f - gesture, gesture * 0.8f,
              0.2f + gesture * 0.8f });
        engine.render(left.data(), right.data(), left.size());
        if (block % 9 == 0)
            physicalString.excite(0.9f);
        physicalString.setPerformanceIntent(
            { gesture, 0.2f + gesture * 8.0f, gesture,
              1.0f - gesture, 0.04f + gesture * 0.46f });
        for (std::size_t sample = 0; sample < left.size(); ++sample)
        {
            left[sample] += physicalString.tick() * 0.1f;
            moduleProbe += fmVoice.tick() + randomWalk.tick();
            const auto lorenzValue = lorenz.tick();
            moduleProbe += lorenzValue.x * 0.001f;
            const auto captured = capture.process(left[sample]);
            moduleProbe += captured.left + captured.right;
            const auto tail = syntheticTail->tick(left[sample], right[sample]);
            moduleProbe += tail.left + tail.right;
            const auto events = eventExtractor.tick(lorenzValue.y);
            const auto clock = fractionalClock.tick();
            const auto memoryStep = sample % 97 == 0
                ? stepMemory.advance() : stepMemory.address(gesture);
            const auto chord = harmonicProgression.selectPhase(
                static_cast<float>((block * 257 + static_cast<int>(sample)) % 48000)
                    / 48000.0f);
            const auto transposition = sample % 97 == 0
                ? accumulatingTransposer.advance(gesture) :
                  aquorbium::AccumulatingTransposer::Output {};
            const auto threshold = thresholdSelector.select(gesture);
            const auto looped = randomToLoop.process(lorenzValue.z, gesture);
            const auto branch = branchingRouter.select(gesture, lorenzValue.x);
            const auto permuted = sample % 97 == 0
                ? permutationMatrix.advance(0, 1.0f)
                : aquorbium::NotePermutationMatrix::Output {};
            const auto temporal = relabi.tick();
            const auto layoutAddress = memoryLayout.map(
                static_cast<std::size_t>(block % 4), sample);
            const auto curved = driftCurve.shape(gesture);
            harmonicMorph.setMorph(gesture);
            const auto harmonic = harmonicMorph.tick();
            const auto sampleHit = sampleVoice->tick();
            const auto maskedPulse = pulseMask.process(sample % 32 < 8, gesture);
            const auto fractal = fractalCore.tick();
            const auto vector = vectorGenerator.tick();
            const auto distribution = probabilityDistribution.sample(gesture);
            const auto evolution = evolutionMachine.advance(gesture);
            const auto shift = shiftMemory.advance(distribution > 0.0f, gesture);
            const auto stage = stageMachine.tick({ gesture, block % 2 == 0,
                                                    1.0f - gesture });
            const auto slowDrift = tripleDrift.tick();
            integrityCrossfade.setStress(gesture);
            const auto integrityMix = integrityCrossfade.tick();
            const auto schoolPhases = phaseBank.advance(0.5f + gesture);
            const auto motion = fluidMotion.tick();
            const auto gestureLoop = gestureRecorder.tick();
            const auto spectral = spectralBank.tick(lorenzValue.x);
            const auto ambience = noiseBed.tick();
            const auto philosophyStep = philosophySequence.tick(sample % 97 == 0);
            const aquorbium::PhilosophyVectorMixer::Values philosophySources {
                lorenzValue.x, lorenzValue.y, lorenzValue.z, gesture
            };
            const auto philosophyBlend = philosophyMorph.tick(philosophySources);
            const auto philosophyOutputs = philosophyMatrix.process(
                philosophySources, philosophyStep.weights);
            const auto rememberedDrift = driftHistory.tick(lorenzValue.x);
            movementLock.update({ gesture, 1.0f - gesture, gesture * 0.5f });
            if (sample % 4096 == 0)
                (void)presenceLatch.trigger(
                    aquorbium::PresenceStateLatch::Event::danger);
            const std::array<float, aquorbium::PriorityChoke::maximumChannels>
                prominence { gesture, 1.0f - gesture, 0.75f, 0.2f };
            const auto choke = priorityChoke.process(prominence, 4);
            moduleProbe += events.envelope * 0.0001f
                + (clock.trigger ? 0.0001f : 0.0f)
                + memoryStep.step.desireAmount * 0.00001f
                + chord.semitones[0] * 0.000001f
                + static_cast<float>(transposition.degreeOffset) * 0.000001f
                + static_cast<float>(threshold.index) * 0.000001f
                + looped.value * 0.000001f
                + static_cast<float>(branch.index) * 0.000001f
                + static_cast<float>(permuted.note.degree) * 0.000001f
                + temporal * 0.000001f
                + static_cast<float>(layoutAddress.index) * 0.000001f
                + curved * 0.000001f
                + harmonic * 0.000001f + sampleHit * 0.000001f
                + (maskedPulse.gate ? 0.000001f : 0.0f)
                + fractal * 0.000001f + vector * 0.000001f
                + distribution * 0.000001f + evolution.value * 0.000001f
                + (shift.outputBit ? 0.000001f : 0.0f)
                + stage.value * 0.000001f + slowDrift.combined * 0.000001f
                + integrityMix.mix * 0.000001f
                + schoolPhases.phase[0] * 0.000001f
                + motion.value[0] * 0.000001f
                + gestureLoop.point.x * 0.000001f
                + spectral.mix * 0.000001f
                + (ambience.left + ambience.right) * 0.000001f
                + philosophyBlend.mix * 0.000001f
                + philosophyOutputs.channel[0] * 0.000001f
                + rememberedDrift.value * 0.000001f
                + movementLock.signature().smoothing * 0.000001f
                + static_cast<float>(choke.selected) * 0.000001f;
        }
    }
    trackAllocations.store(false, std::memory_order_release);

    if (allocationCount.load(std::memory_order_relaxed) != 0
        || !std::isfinite(moduleProbe))
    {
        std::cerr << "audio render allocated " << allocationCount.load() << " times\n";
        return EXIT_FAILURE;
    }

    // Applying unchanged controls only once must equal applying every block.
    auto once = std::make_unique<aquorbium::BiomaEngine>();
    auto repeated = std::make_unique<aquorbium::BiomaEngine>();
    aquorbium::app::AudioControlFrame stable;
    stable.density = 0.68f;
    stable.strikerInfluence = 0.4f;
    stable.geometryInfluence = 0.3f;
    stable.formInfluence = 0.4f;
    stable.structureInfluence = 0.6f;
    stable.structureMemory = 0.7f;
    stable.structureHarmony = 0.8f;
    stable.moduleBank.fill(0.6f);
    stable.spaceReturn = 0.3f;
    stable.lensHeld = true;
    stable.lensZ = 0.8f;
    std::array<float, 257> onceLeft {}, onceRight {}, repeatedLeft {}, repeatedRight {};
    for (auto* source : { once.get(), repeated.get() })
    {
        source->prepare(48000.0);
        source->setSeed(42);
        stable.apply(*source, true);
        source->noteOn(48, 0.7f);
    }
    for (int block = 0; block < 24; ++block)
    {
        stable.apply(*repeated);
        for (auto* source : { once.get(), repeated.get() })
        {
            if (block == 6) source->noteOn(55, 0.6f);
            if (block == 12) source->noteOff(48);
            if (block == 18) source->noteOn(36, 0.8f);
        }
        once->render(onceLeft.data(), onceRight.data(), onceLeft.size());
        repeated->render(repeatedLeft.data(), repeatedRight.data(), repeatedLeft.size());
        for (std::size_t sample = 0; sample < onceLeft.size(); ++sample)
            if (std::bit_cast<std::uint32_t>(onceLeft[sample])
                    != std::bit_cast<std::uint32_t>(repeatedLeft[sample])
                || std::bit_cast<std::uint32_t>(onceRight[sample])
                    != std::bit_cast<std::uint32_t>(repeatedRight[sample]))
            {
                std::cerr << "unchanged control frames changed the DSP trajectory\n";
                return EXIT_FAILURE;
            }
    }
    std::cout << "aquorbium realtime allocation test passed; frame="
              << sizeof(aquorbium::app::AudioControlFrame) << " bytes; bridge="
              << sizeof(aquorbium::FixedLatestSnapshot<aquorbium::app::AudioControlFrame>)
              << " bytes\n";
}
