// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include "core/AccumulatingTransposer.h"
#include "core/AdvancedVoiceEcologyModules.h"
#include "core/AddressableStepMemory.h"
#include "core/BehaviourMemory.h"
#include "core/BiomaGranularCapture.h"
#include "core/BranchingStepRouter.h"
#include "core/ConditionalStageMachine.h"
#include "core/ControlEventExtractor.h"
#include "core/CompositionalVariationDirector.h"
#include "core/CoreRefinementModules.h"
#include "core/DriftCurveShaper.h"
#include "core/EmergentSoundModules.h"
#include "core/EvolutionLoopMachine.h"
#include "core/FluidMotion.h"
#include "core/FractalCore.h"
#include "core/FractionalLogicClock.h"
#include "core/GenerativeInteractionModules.h"
#include "core/GeometricSequencer.h"
#include "core/HarmonicMorphOscillator.h"
#include "core/IntegrityCrossfade.h"
#include "core/LorenzCV.h"
#include "core/MotionGestureRecorder.h"
#include "core/NotePermutationMatrix.h"
#include "core/OpenInteractionModules.h"
#include "core/PatternRotationMutator.h"
#include "core/PhilosophyMixEngine.h"
#include "core/ProbabilityDistribution.h"
#include "core/RandomToLoop.h"
#include "core/RoutingResourceModules.h"
#include "core/SamplePercussionVoice.h"
#include "core/ShiftRegisterMemory.h"
#include "core/StepMemoryLayout.h"
#include "core/StepPatternBank.h"
#include "core/StochasticPulseMask.h"
#include "core/SynthFMVoice.h"
#include "core/ThresholdStepSelector.h"
#include "core/TripleSlowDrift.h"
#include "core/VectorGenerator.h"

#include <array>
#include <cstddef>
#include <cstdint>

namespace aquorbium
{
// One bounded adapter promotes the previously isolated prototypes without
// pretending that they are fifteen unrelated output voices. Every family has
// one visible macro, an explicit consequence and a neutral master bypass.
class IntegratedModuleBank final
{
public:
    enum Control : std::size_t
    {
        mix = 0,
        synthBodies,
        granularMemory,
        percussionPulses,
        fractalVector,
        lorenzDrift,
        fluidPhase,
        gestureWaves,
        clockEvents,
        transposeSelect,
        branchPermute,
        rotateLayout,
        patternLoop,
        shiftProbability,
        evolutionStages,
        integrityMemory,
        controlCount
    };

    using Controls = std::array<float, controlCount>;

    struct Context
    {
        float density = 0.5f;
        float danger = 0.5f;
        float dayPhase = 0.0f;
        float seasonPhase = 0.0f;
        float crackStress = 0.0f;
        float baseFrequencyHz = 110.0f;
        float strength = 0.5f;
        float agility = 0.5f;
        float intelligence = 0.5f;
        float feedbackAudio = 0.0f;
        float temperature = 0.5f;
        float turbidity = 0.35f;
        float verticalPosition = 0.0f;
        float lensX = 0.0f;
        float lensY = 0.0f;
        float compositionInfluence = 0.0f;
        float compositionPulse = 1.0f;
        float compositionTonal = 1.0f;
        float compositionTexture = 1.0f;
        float compositionMovement = 1.0f;
        bool lensHeld = false;
    };

    struct Sample
    {
        float left = 0.0f;
        float right = 0.0f;
    };

    struct Telemetry
    {
        Controls target {};
        Controls current {};
        std::uint64_t events = 0;
        std::uint64_t mutations = 0;
        std::size_t activeFamilies = 0;
        float outputEnergy = 0.0f;
        float feedbackModulation = 0.0f;
        float doubleScrollX = 0.0f;
        float accessModulation = 0.0f;
        float openRuleInfluence = 0.0f;
        float contactEnergy = 0.0f;
        float mediumWaterMix = 0.0f;
        bool lensGestureActive = false;
        float quantizedSubSemitones = -12.0f;
        BiomaGranularCapture::GrainParameters grainParameters {};
        Context smoothedContext {};
    };

    void prepare(double sampleRate) noexcept;
    void setSeed(std::uint32_t value) noexcept;
    void setControls(const Controls& value) noexcept;
    void reset() noexcept;
    [[nodiscard]] Sample process(float inputLeft, float inputRight,
                                 const Context& context) noexcept;
    [[nodiscard]] Telemetry telemetry() const noexcept;

private:
    void configureFixedMaterial() noexcept;
    void advanceEvent(const Context& context, float lorenzX,
                      float fluidValue, float probabilityValue) noexcept;
    [[nodiscard]] float amount(Control control) const noexcept
    {
        return currentControls[static_cast<std::size_t>(control)];
    }

    double sampleRate = 48000.0;
    float smoothing = 0.001f;
    float contextSmoothing = 0.001f;
    Controls targetControls {};
    Controls currentControls {};
    Context currentContext {};
    bool contextInitialized = false;
    std::uint32_t seed = 1u;
    std::uint64_t eventCount = 0;
    std::uint64_t mutationCount = 0;
    std::uint32_t pulsesInBar = 0;
    float eventEnvelope = 0.0f;
    float eventDecay = 0.999f;
    float lastPitchRatio = 1.0f;
    float lastPatternValue = 0.0f;
    float lastEvolutionValue = 0.0f;
    float lastStageValue = 0.0f;
    float lastProbabilityValue = 0.0f;
    float lastPhilosophyValue = 0.0f;
    float lastLSystemValue = 0.0f;
    float lastCellularValue = 0.0f;
    float lastReactiveValue = 0.0f;
    float lastResourceValue = 0.0f;
    float lastFeedbackModulation = 0.0f;
    float lastAccessModulation = 0.0f;
    float lastRuleBranch = 0.0f;
    float lastRuleEvolution = 0.0f;
    float lastRuleGesture = 0.0f;
    float lastRuleIntegrity = 0.0f;
    float lastMediumWaterMix = 0.0f;
    bool lensWasHeld = false;
    std::uint32_t lensGestureInterval = 240u;
    std::uint32_t lensGestureCountdown = 0u;
    MotionGestureRecorder::Output lastLensGesture {};
    DoubleScrollAttractor::Output lastDoubleScroll {};
    float lastQuantizedSubSemitones = -12.0f;
    BiomaGranularCapture::GrainParameters lastGrainParameters {};
    float energy = 0.0f;

    SynthFMVoice fmVoice;
    HarmonicMorphOscillator harmonicVoice;
    BiomaGranularCapture granularCapture;
    SamplePercussionVoice percussion;
    StochasticPulseMask pulseMask;
    FractalCore fractal;
    VectorGenerator vector;
    LorenzCV lorenz;
    DoubleScrollAttractor doubleScroll;
    TripleSlowDrift tripleDrift;
    FluidMotion fluidMotion;
    MotionGestureRecorder gesture;
    MotionGestureRecorder lensGesture;
    FractionalLogicClock logicClock;
    ControlEventExtractor eventExtractor;
    AccumulatingTransposer transposer;
    IntervalQuantizer intervalQuantizer;
    ThresholdStepSelector thresholdSelector;
    BranchingStepRouter branchRouter;
    NotePermutationMatrix permutation;
    PatternRotationMutator rotation;
    StepMemoryLayout memoryLayout;
    AddressableStepMemory stepMemory;
    StepPatternBank patternBank;
    RandomToLoop randomLoop;
    ShiftRegisterMemory shiftRegister;
    ProbabilityDistribution probability;
    EvolutionLoopMachine evolution;
    ConditionalStageMachine stages;
    DriftCurveShaper driftCurve;
    IntegrityCrossfade integrityFade;
    PhilosophySequencer philosophySequencer;
    PhilosophyMorph philosophyMorph;
    PhilosophyMatrixMixer philosophyMatrix;
    DriftHistoryLoop driftHistory;
    MovementSignatureLock signatureLock;
    PresenceStateLatch presenceLatch;
    PriorityChoke priorityChoke;
    SemanticAccessRouter accessRouter;
    OpenRuleProgram openRules;
    ContactCrackleExciter contactCrackle;
    AirWaterMedium airWaterMedium;
    ReeseVoice reeseVoice;
    DualPeakFilter dualPeakFilter;
    ResonantFilterSequencer resonantFilterSequence;
    FrequencyIsolator frequencyIsolator;
    InteractionProcessor interactionProcessor;
    PowerStarve powerStarve;
    PolygonalOscillator polygonalOscillator;
    BoundedRoutingMatrix routingMatrix;
    SharedParameterPool sharedParameterPool;
    BiomaBassVoice biomaBass;
    SchoolWaveVoice schoolWave;
    HarmonicRippleVoice harmonicRipple;
    RecentMixWavetable recentMixWavetable;
    FrequencyShifter frequencyShifter;
    CrossFeedbackStateMachine crossFeedback;
    HungerProcessor hunger;
    BoundedSelfFeedback selfFeedback;
    WaterInertia waterInertia;
    FixedLSystemGenerator lSystem;
    CellularAutomatonSequencer cellularAutomaton;
    SpectralTransferVocoder spectralTransfer;
    EnvelopeContagionBus envelopeContagion;
    NoInputFeedbackBus noInputFeedback;
    ReactiveThresholdStrategy reactiveStrategy;
    LockInFocusExtractor lockInFocus;
    BehaviourSceneBank behaviourScenes;
    ReconfigurableResourcePool reconfigurablePool;
    LightweightBinauralSpatializer binauralSpatializer;
};
}
