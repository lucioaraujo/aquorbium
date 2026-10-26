// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include "core/AddressableStepMemory.h"
#include "core/BiomaRelabi.h"
#include "core/BiomaMarkov.h"
#include "core/CompositionalVariationDirector.h"
#include "core/EcologicalMusicDirector.h"
#include "core/Ecosystem.h"
#include "core/GeometricSequencer.h"
#include "core/HarmonicProgression.h"
#include "core/SessionGenesis.h"

#include <array>
#include <cstddef>
#include <cstdint>

namespace aquorbium
{
// Converts ecological state into a continuous, self-sustaining musical energy
// field. This is not a MIDI sequencer: gates and pitch targets emerge from
// related temporal fields and are smoothed continuously at sample rate.
class AutonomousEnergy final
{
public:
    static constexpr std::size_t organismCount = Ecosystem::organismCount;

    struct OrganismVoice
    {
        float energy = 0.0f;
        float targetEnergy = 0.0f;
        float midiPitch = 36.0f;
        float targetMidiPitch = 36.0f;
        float frequency = 65.4064f;
        float targetFrequency = 65.4064f;
        bool awake = false;
        bool geometricGate = false;
        bool geometricMutation = false;
        std::uint8_t orbitStep = 0;
        std::uint8_t orbitLength = 1;
        float geometricAccent = 0.0f;
        std::uint64_t lifeEvents = 0;
    };

    struct Telemetry
    {
        bool enabled = false;
        float enableAmount = 0.0f;
        float totalEnergy = 0.0f;
        float temporalField = 0.0f;
        std::size_t activeOrganisms = 0;
        int harmonicRootMidi = 36;
        std::uint64_t eventCount = 0;
        std::size_t formState = 3;
        std::uint64_t formTransitionCount = 0;
        float formContextTarget = 0.5f;
        float structureInfluence = 0.0f;
        float structureMemory = 0.5f;
        float structureHarmony = 0.5f;
        float structurePhase = 0.5f;
        std::size_t memoryStep = 0;
        std::size_t harmonicSection = 0;
        CompositionalVariationDirector::Plan composition {};
        EcologicalMusicDirector::Telemetry music {};
        float musicInfluence = 0.0f;
        std::array<OrganismVoice, organismCount> voices {};
        std::size_t harmonyChord = 0;  // the progression's chord sounding now (a generated world)
        PlannedChord chord {};         // its tones, above the current key
        int keyOffset = 0;             // the modulation away from the session's tonic
        std::size_t harmonyCycles = 0;
        HarmonyLanguage harmonyLanguage = HarmonyLanguage::functional;
        float tension = 0.0f;            // the form's tension now, 0..1
        bool breathing = false;          // the melody rests (after a cadence)
        std::size_t melodyLane = 0;
        std::size_t responderLane = 0;
        std::uint64_t responsesSung = 0;   // motif notes sung in answers
        std::uint64_t returnsSung = 0;     // motif notes sung when it returns
        bool motifSounding = false;        // a lane is singing the motif now
        std::size_t motifLane = 0;
    };

    struct FormMemoryPerformance
    {
        float influence = 0.0f;
        float context = 0.5f;
        float inertia = 0.35f;
        float mutation = 0.05f;
    };

    struct StructureMemoryPerformance
    {
        float influence = 0.0f;
        float memory = 0.5f;
        float harmony = 0.5f;
        float phase = 0.5f;
    };

    void prepare(double sampleRate) noexcept;
    void setSeed(std::uint32_t value) noexcept;
    void reset() noexcept;
    void setEnabled(bool value) noexcept;
    void setGeometryPerformance(
        const GeometricSequencer::PerformanceIntent& value) noexcept
    {
        geometry.setPerformanceIntent(value);
    }
    void setFormMemoryPerformance(FormMemoryPerformance value) noexcept;
    // the session's world (SessionGenesis.h): tonic path, mode, registers,
    // orbits, contour, pace, who opens; applied now and at every reset
    void setWorld(const SessionWorld& value) noexcept
    {
        world = value;
        applyWorldToGeometry();
        reset();
    }
    [[nodiscard]] const SessionWorld& sessionWorld() const noexcept { return world; }
    // the chord sounding now (a generated world), for the status and tests
    [[nodiscard]] const PlannedChord& currentChordOfPlan() const noexcept { return livePlan.at(chordIndex - cycleStart); }
    [[nodiscard]] const HarmonyPlan& currentPlan() const noexcept { return livePlan; }
    [[nodiscard]] int currentKeyOffset() const noexcept { return keyOffset; }
    [[nodiscard]] std::size_t harmonyCycles() const noexcept { return cycleCount; }
    [[nodiscard]] std::size_t planChordIndex() const noexcept { return chordIndex; }
    [[nodiscard]] std::size_t harmonyBassLane() const noexcept { return bassLane; }
    [[nodiscard]] std::size_t harmonyMelodyLane() const noexcept { return melodyLane; }
    // the Brain's correlated leanings, handed over at the control rate
    void setBrainLean(const std::array<float, organismCount>& value) noexcept { brainLean = value; }
    void setStructureMemoryPerformance(StructureMemoryPerformance value) noexcept;
    // the music that comes from the ecosystem (ECOLOGICAL_MUSIC_DESIGN.md);
    // influence zero keeps the previous route untouched
    [[nodiscard]] const EcologicalMusicDirector& ecologicalMusicDirector() const noexcept
    {
        return ecologicalMusic;
    }
    void setEcologicalMusicSpectrum(std::size_t organism, const SpectralConsonance::Spectrum& spectrum) noexcept
    {
        ecologicalMusic.setSpectrum(organism, spectrum);
    }
    void setHabitatHealth(const HabitatHealth::Reading& reading) noexcept
    {
        ecologicalMusic.setHabitat(reading);
    }
    void setRoom(const RoomCoupling::Reading& reading) noexcept
    {
        ecologicalMusic.setRoom(reading);
    }
    void chooseSoundmark() noexcept { ecologicalMusic.chooseSoundmark(); }
    void restoreSoundmark(const SongForm::Phrase& motif) noexcept { ecologicalMusic.restoreSoundmark(motif); }
    void setEcologicalMusicPerformance(EcologicalMusicDirector::Performance value) noexcept
    {
        ecologicalMusic.setPerformance(value);
    }
    [[nodiscard]] bool enabled() const noexcept { return targetEnabled; }
    [[nodiscard]] float enableAmount() const noexcept { return currentEnableAmount; }

    void tickSample(const Ecosystem& ecosystem, float outputEnergy) noexcept;
    [[nodiscard]] const OrganismVoice& voice(std::size_t index) const noexcept;
    // a directed note of the ecological music is being held (its gate)
    [[nodiscard]] bool noteHeld(std::size_t index) const noexcept
    {
        return index < organismCount && noteHoldSamples[index] > 0;
    }
    [[nodiscard]] Telemetry telemetry() const noexcept;
    [[nodiscard]] CompositionalVariationDirector::Plan compositionPlan() const noexcept
    {
        auto result = compositionDirector.plan();
        result.influence = structurePerformance.influence;
        return result;
    }

private:
    void controlTick(const Ecosystem& ecosystem, float outputEnergy) noexcept;
    void makeDecision(std::size_t index, const Ecosystem& ecosystem,
                      bool collectiveEvent) noexcept;
    void updatePitch(std::size_t index, const Ecosystem& ecosystem,
                     const GeometricSequencer::Step& step,
                     float structuralSemitones) noexcept;
    [[nodiscard]] float nextRandomUnit() noexcept;
    [[nodiscard]] int currentSection(const Ecosystem::Environment& environment) const noexcept;
    [[nodiscard]] int rootForSection(int section) const noexcept;

    double sampleRate = 48000.0;
    std::uint32_t controlIntervalSamples = 480;
    std::uint32_t samplesUntilControlTick = 0;
    float controlDt = 0.01f;
    std::uint32_t enableRampSamples = 5760;
    std::uint32_t enableSamplesRemaining = 0;
    float attackSlew = 0.0f;
    float releaseSlew = 0.0f;
    bool targetEnabled = false;
    float currentEnableAmount = 0.0f;
    float lastTemporalField = 0.0f;
    float currentTemporalField = 0.0f;
    int section = -1;
    int harmonicRootMidi = 36;
    std::uint64_t eventCount = 0;
    float formContextTarget = 0.5f;
    std::uint32_t seed = 0x4f1bbcddu;
    std::uint32_t randomState = 0x4f1bbcddu;
    BiomaRelabi temporalField;
    BiomaMarkov formMemory;
    CompositionalVariationDirector compositionDirector;
    GeometricSequencer geometry;
    FormMemoryPerformance formPerformance {};
    AddressableStepMemory stepMemory;
    HarmonicProgression harmonicProgression;
    HarmonicProgression::Chord currentChord {};
    StructureMemoryPerformance structurePerformance {};
    EcologicalMusicDirector ecologicalMusic;
    void directedNote(std::size_t index, float midi, float accent, float glideSeconds,
                      float durationSeconds, float releaseSeconds) noexcept;
    float currentStructurePhase = 0.0f;
    std::size_t currentMemoryStep = 0;
    std::array<OrganismVoice, organismCount> voices {};
    std::array<float, organismCount> lifePhase {};
    std::array<float, organismCount> lifeRate {};
    std::array<float, organismCount> ecologicalEnergy {};
    std::array<float, organismCount> pitchStep {};
    std::array<float, organismCount> frequencyMultiplier {};
    std::array<std::uint32_t, organismCount> pitchSamplesRemaining {};
    // the ecological music articulates: each note is held, then released
    std::array<std::uint32_t, organismCount> noteHoldSamples {};
    std::array<float, organismCount> noteReleaseSlew {};
    float musicMix = 0.0f;
    std::array<float, organismCount> brainLean {};
    SessionWorld world {};
    // the progression's clock and the organisms' roles in it
    float harmonyClock = 0.0f;
    std::size_t chordIndex = 0;
    // the progression lives: at each turn of the cycle it is renewed, or
    // modulates, or changes language, and every third cycle it comes home
    HarmonyPlan livePlan {};
    std::size_t cycleStart = 0;
    std::size_t cycleCount = 0;
    int keyOffset = 0;
    std::uint64_t harmonyRandomState = 1;
    [[nodiscard]] float harmonyUnit() noexcept;
    // the emotion of the form (2026-10-24): the session's arc and the
    // chord's own tension, and what they move
    float sessionSeconds = 0.0f;
    float tension = 0.0f;              // 0 repose .. 1 climax, smoothed
    float breathUntil = -1.0f;         // the melody rests after a cadence
    std::size_t lastChordSeen = 0;
    bool lastChordWasCadence = false;
    float chordStartedAt = 0.0f;
    // the motif and its answers (2026-10-24): the melody's last notes, kept
    // as semitones above the key; after a cadence another voice answers with
    // them transformed, and every second cycle the melody brings them back
    static constexpr std::size_t motifCapacity = 6;
    std::array<int, motifCapacity> recentMelody {};
    std::size_t recentCount = 0;
    std::array<int, motifCapacity> keptMotif {};
    std::size_t motifLength = 0;
    std::size_t responderLane = 1;
    // two queues, so an answer never overwrites a return: 0 = the answer
    // (the responder), 1 = the return (the melody)
    struct MotifQueue
    {
        std::array<int, motifCapacity> pitches {};  // semitones above the key
        std::size_t length = 0, next = 0, lane = 0;
        [[nodiscard]] bool sounding() const noexcept { return next < length; }
    };
    std::array<MotifQueue, 2> queues {};
    std::uint64_t responsesSung = 0, returnsSung = 0;
    void scheduleMotif(std::size_t lane, int transposition, bool answer) noexcept;
    [[nodiscard]] float inMode(float pitch) const noexcept;
    [[nodiscard]] static float foldIntoRange(float pitch, float low, float high) noexcept;
    void evolveHarmony() noexcept;
    std::size_t bassLane = 0, melodyLane = 0;
    std::array<bool, organismCount> upperLayer {};
    void assignHarmonyRoles() noexcept;
    [[nodiscard]] float voiceHarmony(std::size_t index, float target, const GeometricSequencer::Step& step) const noexcept;
    void applyWorldToGeometry() noexcept
    {
        geometry.setWorld(world.scale, world.scaleSize, world.orbitSteps, world.rotations, world.contour);
    }
    float musicAttackSlew = 1.0f;
};
}
