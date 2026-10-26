// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include "core/AmbisonicEncoder.h"
#include "core/AutonomousEnergy.h"
#include "core/BinauralSource.h"
#include "core/BubbleField.h"
#include "core/SynthVoice.h"
#include "core/HabitatHealth.h"
#include "core/RoomCoupling.h"
#include "core/AquariumTail.h"
#include "core/AquariumSpatializer.h"
#include "core/BiomaBrain.h"
#include "core/CoreRefinementModules.h"
#include "core/BiomaModalResonator.h"
#include "core/BiomaPhysicalString.h"
#include "core/Ecosystem.h"
#include "core/MultiTapSpace.h"
#include "core/OrganismVoiceEngine.h"
#include "core/TimbreBody.h"
#include "core/OutputStage.h"
#include "core/SchoolSpectralBank.h"
#include "core/EcologicalNoiseBed.h"
#include "core/IntegratedModuleBank.h"
#include "core/MasterChain.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>

namespace aquorbium
{
static_assert(BiomaBrain::organismCount == Ecosystem::organismCount,
              "Brain and Ecosystem must describe the same organism field");

class BiomaEngine
{
public:
    static constexpr std::size_t voiceCount = 4;
    // Fixed acoustic calibration after the ecological feedback measurement.
    // It raises listening level without changing the ecosystem's behaviour.
    static constexpr float outputCalibrationGain = 5.0f;
    // One large delay-line body for every external voice, plus the autonomous
    // ecological bank. PhysicalString is deliberately not duplicated inside
    // each compact organism dispatcher.
    static constexpr std::size_t physicalStringBodyCount = voiceCount + 1;
    static_assert(physicalStringBodyCount == 5);

    // Neutral retains the direct render path while the
    // ecological clock keeps advancing. This makes A/B renders share seed,
    // note events and stochastic state instead of comparing two performances.
    enum class EcosystemRenderMode : std::uint8_t { Neutral, Active };

    struct SpectralEnvironmentPerformance
    {
        float influence = 0.0f;
        float filterMix = 0.5f;
        float centre = 0.5f;
        float span = 0.5f;
        float resonance = 0.45f;
        float ambience = 0.0f;
    };

    struct SyntheticTailPerformance
    {
        float influence = 0.0f;
        float decay = 0.45f;
        float damping = 0.55f;
        float nightmare = 0.0f;
    };

    BiomaEngine();
    ~BiomaEngine();
    BiomaEngine(const BiomaEngine&) = delete;
    BiomaEngine& operator=(const BiomaEngine&) = delete;
    BiomaEngine(BiomaEngine&&) = delete;
    BiomaEngine& operator=(BiomaEngine&&) = delete;

    // Value snapshot for an offline renderer, visual adapter or diagnostic.
    // It is allocation-free, but is not a cross-thread GUI contract yet.
    // the output's feedback taps (2026-10-23, aquorbium-arquitetura.md: the
    // energy that feeds the Brain is published in separate taps, before the
    // creative MASTER, before the safety limiter, as the limiter's reduction
    // and after it - read only after the limiter, the ecosystem's dynamics
    // would be erased). Each is an envelope, 10 ms attack and 150 ms release,
    // bounded to 0..1; the reduction is the limiter's, 0 = none, 1 = 12 dB.
    struct FeedbackTaps
    {
        float preMaster = 0.0f;
        float preSafety = 0.0f;
        float safetyReduction = 0.0f;
        float postSafety = 0.0f;
    };
    [[nodiscard]] const FeedbackTaps& feedbackTaps() const noexcept { return taps; }

    struct Telemetry
    {
        EcosystemRenderMode renderMode = EcosystemRenderMode::Active;
        float ecosystemBlendTarget = 1.0f;
        float ecosystemBlendCurrent = 1.0f;
        bool outputFeedbackEnabled = true;
        bool autonomousEnabled = false;
        bool granularStrikerEnabled = true;
        bool modalResonatorEnabled = true;
        bool spatializerEnabled = true;
        bool multiTapSpaceEnabled = true;
        bool aquariumMotionAudioEnabled = false;
        std::size_t activeVoices = 0;
        // Pre-calibration energy used by ecological feedback, not a dB meter.
        float outputEnergy = 0.0f;
        float masterFeedback = 0.0f;
        float densityTarget = 0.45f;
        float densityCurrent = 0.45f;
        float driftTarget = 0.20f;
        float driftCurrent = 0.20f;
        float multiTapSpaceReturnTarget = 0.36f;
        float multiTapSpaceReturnCurrent = 0.36f;
        SpectralEnvironmentPerformance spectralEnvironmentTarget {};
        SpectralEnvironmentPerformance spectralEnvironmentCurrent {};
        SyntheticTailPerformance syntheticTailTarget {};
        SyntheticTailPerformance syntheticTailCurrent {};
        IntegratedModuleBank::Telemetry moduleBank {};
        AutonomousEnergy::Telemetry autonomy {};
        BiomaGranularStriker::Telemetry granularStriker {};
        BiomaModalResonator::Telemetry resonator {};
        BiomaPhysicalString::Telemetry physicalString {};
        Ecosystem::Desire desire {};
        Ecosystem::Environment environment {};
        Ecosystem::Integrity integrity {};
        Ecosystem::Lens lens {};
        std::size_t lensFocusedOrganisms = 0;
        std::array<Ecosystem::OrganismState, Ecosystem::organismCount> organisms {};
        std::array<Ecosystem::MixState, Ecosystem::organismCount> mix {};
        std::array<OrganismVoiceEngine::Telemetry, Ecosystem::organismCount> sources {};
        std::array<AquariumSpatializer::Telemetry, Ecosystem::organismCount> spatial {};
        MultiTapSpace::Telemetry multiTapSpace {};
        AquariumTail::Telemetry syntheticTail {};
        MasterChain::Telemetry creativeMaster {};
        OutputStage::Telemetry output {};
        FeedbackTaps feedbackTaps {};
        // the CÉREBRO and SALA cards (2026-10-24)
        std::uint8_t brainRegister = 0;
        float brainMood = 0.0f;           // the mean of its leanings, -1..1
        RoomCoupling::Reading room {};
        bool roomEnabled = false;
    };

    void prepare(double sampleRate) noexcept;
    void setSeed(std::uint32_t value) noexcept;
    // the world a session opens in (SessionGenesis.h): set it before prepare;
    // without it the engine is the classic instrument, bit for bit
    void setSessionWorld(const SessionWorld& value) noexcept
    {
        sessionWorld = value;
        autonomousEnergy.setWorld(value);
        ecosystem.setStartPhases(value.dayPhase, value.seasonPhase);
    }
    [[nodiscard]] const SessionWorld& currentSessionWorld() const noexcept { return sessionWorld; }
    void setDensity(float value) noexcept;
    void setDrift(float value) noexcept;
    void setCohesion(float value) noexcept;
    void setDanger(float value) noexcept;
    void setDesire(Ecosystem::Desire value) noexcept;
    void setEcosystemRenderMode(EcosystemRenderMode value) noexcept;
    [[nodiscard]] EcosystemRenderMode ecosystemRenderMode() const noexcept;
    void setEcosystemBlend(float value) noexcept;
    // Intended for deterministic offline renders and tests. Performance
    // controls should use setEcosystemBlend(), which always smooths.
    void setEcosystemBlendImmediately(float value) noexcept;
    [[nodiscard]] float ecosystemBlendTarget() const noexcept { return targetEcosystemBlend; }
    [[nodiscard]] float ecosystemBlendCurrent() const noexcept { return currentEcosystemBlend; }
    void setEcosystemOutputFeedbackEnabled(bool value) noexcept;
    [[nodiscard]] bool ecosystemOutputFeedbackEnabled() const noexcept { return outputFeedbackEnabled; }
    void setAutonomousEnabled(bool value) noexcept { autonomousEnergy.setEnabled(value); }
    [[nodiscard]] bool autonomousEnabled() const noexcept { return autonomousEnergy.enabled(); }
    void setGranularStrikerEnabled(bool value) noexcept { granularStrikerEnabled = value; }
    [[nodiscard]] bool isGranularStrikerEnabled() const noexcept
    {
        return granularStrikerEnabled;
    }
    void setGranularStrikerPerformance(
        const BiomaGranularStriker::PerformanceIntent& value) noexcept;
    void setGeometryPerformance(
        const GeometricSequencer::PerformanceIntent& value) noexcept
    {
        autonomousEnergy.setGeometryPerformance(value);
    }
    void setFormMemoryPerformance(
        const AutonomousEnergy::FormMemoryPerformance& value) noexcept
    {
        autonomousEnergy.setFormMemoryPerformance(value);
    }
    void setEcologicalMusicPerformance(EcologicalMusicDirector::Performance value) noexcept
    {
        autonomousEnergy.setEcologicalMusicPerformance(value);
    }

    // The organisms' bodies (2026-10-20): per organism a second body, its
    // MATTER and how much of it is mixed with the native body; ECOLOGY lets
    // the ecosystem transform MATTER and presence (water, hunt, flight,
    // courtship, vigour, health). All mixes and ECOLOGY at zero: the previous
    // sound, bit for bit.
    struct TimbrePerformance
    {
        std::array<TimbreBody::Kind, Ecosystem::organismCount> body {};
        std::array<float, Ecosystem::organismCount> matter { 0.5f, 0.5f, 0.5f, 0.5f, 0.5f };
        std::array<float, Ecosystem::organismCount> mix {};
        float ecology = 0.0f;
    };
    void setTimbrePerformance(const TimbrePerformance& value) noexcept;
    struct TimbreTelemetry
    {
        std::array<float, Ecosystem::organismCount> matter {};
        std::array<float, Ecosystem::organismCount> mix {};
    };
    [[nodiscard]] TimbreTelemetry timbreTelemetry() const noexcept { return timbreState; }
    void setStructureMemoryPerformance(
        const AutonomousEnergy::StructureMemoryPerformance& value) noexcept
    {
        autonomousEnergy.setStructureMemoryPerformance(value);
    }
    void setModalResonatorEnabled(bool value) noexcept { modalResonatorEnabled = value; }
    [[nodiscard]] bool isModalResonatorEnabled() const noexcept { return modalResonatorEnabled; }
    void setResonatorPerformance(
        const BiomaModalResonator::PerformanceIntent& value) noexcept;
    // Per-species balance for calibration, stems and future performance
    // adapters. Unity is the authored default; zero keeps DSP advancing but
    // removes that species from the acoustic/ecological mix.
    void setSpeciesGain(Ecosystem::Species species, float value) noexcept;
    // SUPPORT (2026-10-23): a level for the three support species - GRAIN,
    // MODAL, STRING - on top of their authored gains, x0.5 .. x16 (-6 ..
    // +24 dB); 1 = the authored balance. Glides per sample
    void setSupportPresence(float gain) noexcept;
    [[nodiscard]] float speciesGain(Ecosystem::Species species) const noexcept;
    // Deterministic technical comparison only; change while audio is stopped.
    // Both spatial routes keep advancing so switching does not alter ecology.
    void setSpatializerEnabled(bool value) noexcept { spatializerEnabled = value; }
    [[nodiscard]] bool isSpatializerEnabled() const noexcept { return spatializerEnabled; }
    // Technical A/B route. The delay body advances in both positions, so this
    // switch does not change ecological clocks or erase its state.
    void setMultiTapSpaceEnabled(bool value) noexcept { multiTapSpaceEnabled = value; }
    [[nodiscard]] bool isMultiTapSpaceEnabled() const noexcept
    {
        return multiTapSpaceEnabled;
    }
    // Public performance route: zero is the click-free bypass. The technical
    // switch above is intended only for deterministic offline comparisons.
    void setMultiTapSpaceReturn(float value) noexcept;
    [[nodiscard]] float multiTapSpaceReturnTarget() const noexcept
    {
        return targetMultiTapSpaceReturn;
    }
    [[nodiscard]] float multiTapSpaceReturnCurrent() const noexcept
    {
        return currentMultiTapSpaceReturn;
    }
    void setMultiTapSpacePerformance(
        const MultiTapSpace::PerformanceIntent& value) noexcept;
    void setSpectralEnvironmentPerformance(
        SpectralEnvironmentPerformance value) noexcept;
    void setSyntheticTailPerformance(SyntheticTailPerformance value) noexcept;
    void setIntegratedModuleBankControls(
        const IntegratedModuleBank::Controls& value) noexcept
    {
        moduleBank->setControls(value);
    }
    void setCreativeMasterInfluence(float value) noexcept;
    // the creative MASTER's tilt EQ before (false) or after (true) its
    // multiband dynamics: a choice of the musician (principle 4ms LEQA)
    void setCreativeMasterEqAfterDynamics(bool after) noexcept { creativeMasterEqAfterDynamics = after; }
    // the creative MASTER's insides (2026-10-24): offsets on top of the
    // ecological values (0 = the ecology alone, as before)
    struct CreativeMasterShape
    {
        float multiband = 0.0f;  // -0.5 .. +0.5
        float glue = 0.0f;       // -0.5 .. +0.5
        float tilt = 0.0f;       // -1 .. +1
        float width = 0.0f;      // -0.75 .. +0.75
    };
    void setCreativeMasterShape(const CreativeMasterShape& value) noexcept { creativeMasterShape = value; }
    // the CÉREBRO card: the register's clock (x classic) and coordination
    void setBrainControls(float clockScale, float coordination) noexcept { brain.setControls(clockScale, coordination); }
    [[nodiscard]] std::uint8_t brainRegister() const noexcept { return brain.registerBits(); }
    [[nodiscard]] const std::array<float, Ecosystem::organismCount>& brainLeaning() const noexcept { return brain.correlation(); }
    // the LUPA card
    void setLensShape(float focusRadius, float glassReach, float magnification, float peripheralDetail) noexcept
    {
        ecosystem.setLensShape(focusRadius, glassReach, magnification, peripheralDetail);
    }
    // the SALA card: the room's level read +-dB, and how strongly the music
    // reacts to it (x; 1 = classic)
    void setRoomControls(float sensitivityDb, float reaction) noexcept
    {
        roomSensitivityDb = std::isfinite(sensitivityDb) ? std::clamp(sensitivityDb, -24.0f, 24.0f) : 0.0f;
        roomReaction = std::isfinite(reaction) ? std::clamp(reaction, 0.0f, 4.0f) : 1.0f;
    }
    [[nodiscard]] RoomCoupling::Reading roomReading() const noexcept { return room.reading(); }
    // Technical safety stage after every creative process. These parameters
    // are never modulated by the ecosystem.
    void setOutputStageParameters(OutputStage::Parameters value) noexcept
    {
        outputStage->setParameters(value);
    }
    [[nodiscard]] std::uint32_t outputLatencySamples() const noexcept
    {
        return outputStage->latencySamples();
    }
    [[nodiscard]] OutputStage::Parameters outputStageParameters() const noexcept
    {
        return outputStage->parameters();
    }
    void setAquariumPose(AquariumPose value) noexcept { ecosystem.setAquariumPose(value); }
    void setAquariumMotionAudioEnabled(bool value) noexcept
    {
        ecosystem.setAquariumMotionAudioEnabled(value);
    }
    // The app publishes a pointer gesture to the audio thread; only that
    // thread may change lens ownership or its spherical target.
    void setLensGesture(bool held, Ecosystem::Vec3 target) noexcept;
    void setListeningBlend(
        std::array<float, Ecosystem::observerCount> weights) noexcept
    {
        ecosystem.setListeningBlend(weights);
    }
    void noteOn(int midiNote, float velocity = 1.0f) noexcept;
    void noteOff(int midiNote) noexcept;
    void noteOff() noexcept;
    [[nodiscard]] std::size_t activeVoiceCount() const noexcept;
    [[nodiscard]] const Ecosystem& ecosystemState() const noexcept { return ecosystem; }
    [[nodiscard]] Telemetry telemetry() const noexcept;
    void render(float* left, float* right, std::size_t samples) noexcept;
    // RoomCoupling (optional, off by default): the microphone samples for the
    // next render() only - descriptors are taken, nothing is kept or played
    void setRoomInput(const float* microphone, std::size_t count) noexcept
    {
        roomInput = microphone;
        roomInputCount = count;
    }
    void setRoomCouplingEnabled(bool enabled) noexcept;
    // recording taps (2026-10-22, OUTPUT_STAGE_DESIGN.md): where the REC may
    // listen besides the final output - "pre-master-criativo" (the mix before
    // the creative MASTER), "pre-safety" (after it, before the safety
    // limiter). The buffers are written by the next render() only, then let
    // go, like the room input; the final output ("post-safety") is render's
    enum class RecordTap : std::uint8_t { postSafety, preSafety, preMasterCreative };
    // stems (2026-10-22): five stereo parts whose sum is the pre-master mix,
    // exactly - the organisms (autonomous, MIDI, synthesis), the LAB (the
    // module bank), the water (what the ENVIRONMENT's spectral filter does to
    // them, its ambience and the bubbles), the space's reflections and the
    // tail. For the next render() only, like the taps
    static constexpr std::size_t stemCount = 5;
    struct StemBuffers
    {
        std::array<float*, stemCount> left {};
        std::array<float*, stemCount> right {};
        std::size_t capacity = 0;
    };
    void setStems(const StemBuffers& value) noexcept { stems = value; }
    // first-order Ambisonics of the mix before the creative MASTER
    // (2026-10-23), AmbiX: ACN W, Y, Z, X with SN3D. Each organism from its
    // direction to the listener, the rest (MIDI voices, bank, water, space,
    // tail) as the stereo pair at +-30 degrees. Like the stems, set before
    // each render and cleared by it (the REC), so a session without it is
    // bit-identical. Its W is (L + R) / sqrt(2) of that mix on the speakers.
    struct AmbisonicBuffers
    {
        std::array<float*, 4> channel {};  // W, Y, Z, X
        std::size_t capacity = 0;
    };
    void setAmbisonics(const AmbisonicBuffers& value) noexcept { ambisonics = value; }
    void setRecordTap(RecordTap tap, float* left, float* right, std::size_t capacity) noexcept
    {
        recordTap = tap;
        recordTapLeft = left;
        recordTapRight = right;
        recordTapCapacity = capacity;
    }
    // headphones: each organism heard from its direction through a head
    // (BinauralSource, Brown & Duda); loudspeakers keep the stereo mix
    void setHeadphones(bool headphones) noexcept { binauralTarget = headphones ? 1.0f : 0.0f; }
    // the synthesis device (SYNTH_DEVICE_DESIGN.md): one voice per organism,
    // played by the ecosystem's notes; SYNTHESIS mixes it with the organism's
    // own sound (0 = the previous sound, bit for bit)
    void setSynthMix(float amount) noexcept;
    void setSynthPatch(std::size_t organism, const SynthVoice::Patch& patch) noexcept;
    [[nodiscard]] const SynthVoice::Patch& synthPatch(std::size_t organism) const noexcept;
    [[nodiscard]] const SynthVoice::Ecology& synthEcology(std::size_t organism) const noexcept
    {
        return (*synthVoiceBank)[std::min(organism, synthVoiceBank->size() - 1)].currentEcology();
    }
    // the musician's soundmark (MARCA): choose the phrase sounding now, or
    // restore one kept from an earlier session
    void chooseSoundmark() noexcept { autonomousEnergy.chooseSoundmark(); }
    void restoreSoundmark(const SongForm::Phrase& motif) noexcept { autonomousEnergy.restoreSoundmark(motif); }

private:
    struct Voice
    {
        int midiNote = -1;
        float velocity = 0.0f;
        bool gate = false;
        float envelope = 0.0f;
        std::array<OrganismVoiceEngine, Ecosystem::organismCount> organism {};
        std::array<AquariumSpatializer, Ecosystem::organismCount> spatializer {};
        std::array<float, Ecosystem::organismCount> frequency {
            110.0f, 165.0f, 220.0f, 330.0f, 247.5f
        };
        BiomaPhysicalString physicalString;
        // the same 10 Hz DC blocker as the autonomous route, per organism
        // (2026-10-22: the Pulsar's train left +0.0055 of DC in the mix)
        std::array<float, Ecosystem::organismCount> dcInput {}, dcOutput {};
    };

    [[nodiscard]] Voice& findVoiceFor(int midiNote) noexcept;
    void resetAutonomousVoice() noexcept;
    [[nodiscard]] float advanceEcosystemBlend() noexcept;
    [[nodiscard]] float advanceMultiTapSpaceReturn() noexcept;
    void updateMultiTapSpaceParameters() noexcept;
    void updateSpectralEnvironmentParameters() noexcept;
    void updateSyntheticTailParameters() noexcept;
    void updateCreativeMasterParameters() noexcept;
    double sampleRate = 48000.0;
    float density = 0.45f;
    float drift = 0.20f;
    float audioDensity = 0.45f;
    float audioDrift = 0.20f;
    float directControlSmoothing = 0.001f;
    float strikerMixTarget = 1.0f;
    float strikerMixCurrent = 1.0f;
    float targetEcosystemBlend = 1.0f;
    float currentEcosystemBlend = 1.0f;
    std::uint32_t ecosystemBlendRampSamples = 3840;
    std::uint32_t ecosystemBlendSamplesRemaining = 0;
    bool outputFeedbackEnabled = true;
    bool granularStrikerEnabled = true;
    bool modalResonatorEnabled = true;
    bool spatializerEnabled = true;
    bool multiTapSpaceEnabled = true;
    float lastOutputEnergy = 0.0f;
    float finalOutputFeedbackSample = 0.0f;
    BoundedMasterFeedback masterFeedback;
    float targetMultiTapSpaceReturn = 0.36f;
    float currentMultiTapSpaceReturn = 0.36f;
    float multiTapSpaceReturnSmoothing = 0.00025f;
    std::uint32_t multiTapSpaceControlInterval = 240;
    std::uint32_t samplesUntilMultiTapSpaceControl = 0;
    SpectralEnvironmentPerformance targetSpectralEnvironment {};
    SpectralEnvironmentPerformance currentSpectralEnvironment {};
    SyntheticTailPerformance targetSyntheticTail {};
    SyntheticTailPerformance currentSyntheticTail {};
    float targetCreativeMasterInfluence = 0.0f;
    bool creativeMasterEqAfterDynamics = false;
    CreativeMasterShape creativeMasterShape {};
    float roomSensitivityDb = 0.0f;
    float roomReaction = 1.0f;
    std::array<float, Ecosystem::organismCount> speciesGains {
        0.55f, 1.25f, 1.50f, 1.20f, 1.50f
    };
    std::uint32_t engineSeed = 0x815ab1eu;
    std::array<std::uint64_t, Ecosystem::organismCount> lastAutonomousLifeEvent {};
    std::array<Voice, voiceCount> voices {};
    Voice autonomousVoice;
    float autonomousDcCoefficient = 0.9987f;
    std::array<float, Ecosystem::organismCount> autonomousDcInput {};
    std::array<float, Ecosystem::organismCount> autonomousDcOutput {};
    // liquid absorption (ecological music): a one-pole low-pass per organism
    std::array<float, Ecosystem::organismCount> liquidCoefficient {};
    std::array<float, Ecosystem::organismCount> liquidState {};
    // the ecological music's influence as heard, smoothed per organism: it
    // mixes the liquid filter and lowers the event bodies' floor, and a step
    // of it clicked (AutomationClickTests, MUSIC 0.6 -> 0, 2026-10-23)
    std::array<float, Ecosystem::organismCount> liquidCurrent {};
    // the organisms' second bodies
    std::array<TimbreBody, Ecosystem::organismCount> timbreBodies {};
    TimbrePerformance timbre {};
    TimbreTelemetry timbreState {};
    std::array<float, Ecosystem::organismCount> timbreMixTarget {};
    std::array<float, Ecosystem::organismCount> timbreMixCurrent {};
    float timbreMixSmoothing = 0.001f;
    float timbreWobblePhase = 0.0f;
    void updateTimbreTargets() noexcept;
    BiomaBrain brain;
    Ecosystem ecosystem;
    AutonomousEnergy autonomousEnergy;
    SchoolSpectralBank schoolSpectralLeft;
    SchoolSpectralBank schoolSpectralRight;
    EcologicalNoiseBed ecologicalNoise;
    // One fixed-capacity shared body. Heap ownership keeps the already-large
    // engine usable on hosts with a small thread stack; construction happens
    // before prepare/render and never inside the audio callback.
    std::unique_ptr<MultiTapSpace> multiTapSpace;
    // The fixed-capacity FDN is also heap-owned: construction and allocation
    // stay outside the callback, while every audio sample advances its state.
    std::unique_ptr<AquariumTail> syntheticTail;
    // The promoted prototype bank owns several large fixed histories. Heap
    // ownership preserves the engine's stack footprint; process() is bounded.
    std::unique_ptr<IntegratedModuleBank> moduleBank;
    // Creative mastering remains independent from the final technical
    // limiter. Zero influence is a sample-exact bypass.
    std::unique_ptr<MasterChain> masterChain;
    // Fixed lookahead storage is heap-owned for the same stack-safety reason.
    std::unique_ptr<OutputStage> outputStage;
    // ecoacoustic indices of what the aquarium sounds like (HabitatHealth)
    std::unique_ptr<HabitatHealth> habitat;
    RoomCoupling room {};
    // the water bubbling (turbulence a little, boiling a lot): BubbleField
    BubbleField bubbleField {};
    std::array<BinauralSource, Ecosystem::organismCount> binaural {};
    // the MIDI voices' heads, one per organism (the head is linear: the
    // organism's voices share it), in the organism's direction (2026-10-22)
    std::array<BinauralSource, Ecosystem::organismCount> midiBinaural {};
    std::array<float, Ecosystem::organismCount> midiBinauralInput {};
    // ~100 kB each with every source: on the heap, not on a (Windows) stack
    std::unique_ptr<std::array<SynthVoice, Ecosystem::organismCount>> synthVoiceBank;
    std::array<SynthVoice, Ecosystem::organismCount>& synthVoices = *synthVoiceBank;
    std::array<std::uint64_t, Ecosystem::organismCount> synthEvents {};
    std::array<float, Ecosystem::organismCount> synthLiquid {};
    // SYNTHESIS: the mix heard (smoothed per sample, 15 ms) and the one asked
    // for. Applied at once before 2026-10-22, which clicked (AutomationClickTests)
    float synthMix = 0.0f;
    float synthMixTarget = 0.0f;
    float supportTarget = 1.0f, supportCurrent = 1.0f;
    [[nodiscard]] float supportFactor(Ecosystem::Species species) const noexcept
    {
        return species == Ecosystem::Species::GranularStriker || species == Ecosystem::Species::Resonator
                || species == Ecosystem::Species::PhysicalString
            ? supportCurrent : 1.0f;
    }
    RecordTap recordTap = RecordTap::postSafety;
    float* recordTapLeft = nullptr;
    float* recordTapRight = nullptr;
    std::size_t recordTapCapacity = 0;
    StemBuffers stems {};
    SessionWorld sessionWorld {};
    [[nodiscard]] float sessionPulseWidth(std::size_t organism) const noexcept
    {
        return sessionWorld.classic ? 0.36f + 0.08f * static_cast<float>(organism)
                                    : sessionWorld.pulseWidths[std::min(organism, sessionWorld.pulseWidths.size() - 1)];
    }
    FeedbackTaps taps {};
    float tapAttack = 0.002f, tapRelease = 0.0001f;
    AmbisonicBuffers ambisonics {};
    std::array<FoaPanner, Ecosystem::organismCount> ambisonicPanners {};
    float binauralTarget = 0.0f;
    float binauralMix = 0.0f;     // crossfaded over ~50 ms
    float binauralStep = 1.0f / 2400.0f;
    bool bubblesActive = false;
    bool roomEnabled = false;
    const float* roomInput = nullptr;
    std::size_t roomInputCount = 0;
};
}
