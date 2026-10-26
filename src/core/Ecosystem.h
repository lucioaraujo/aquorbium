// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include "core/AquariumPose.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>

namespace aquorbium
{
class BiomaBrain;

// Control physics for the spherical Aquarium. This is intentionally separate
// from JUCE's application and buffer adapters: the same deterministic model
// must be testable without opening a device or constructing a GUI.
class Ecosystem
{
public:
    static constexpr std::size_t organismCount = 5;
    static constexpr std::size_t observerCount = 4;

    using Vec3 = SpatialVector3;

    enum class Species : std::uint8_t
    {
        Oscillator,
        Pulsar,
        GranularStriker,
        Resonator,
        PhysicalString
    };

    enum class SonicRole : std::uint8_t { Foreground, Support };
    enum class Presence : std::uint8_t { Present, Leaving, Absent, Entering, Trapped };
    enum class Strategy : std::uint8_t { None, Territorial, Sessile, Migratory, Predator, Prey };

    // Authorial behavioural names. Artistic references remain in the research
    // documents rather than being presented as algorithms by those artists.
    enum class ListeningMode : std::uint8_t
    {
        SparseFocus,
        EnergyMemory,
        GeometricCycle,
        CollectiveDrift
    };

    struct Desire
    {
        float calm = 0.5f;
        float density = 0.5f;
        float cohesion = 0.5f;
        float danger = 0.5f;
    };

    struct Environment
    {
        float dayPhase = 0.0f;
        float seasonPhase = 0.0f;
        float temperature = 0.5f;
        float daylight = 0.5f;
        float matingPulse = 0.0f;
    };

    struct Integrity
    {
        float health = 1.0f;
        float crackStress = 0.0f;
        bool cracked = false;
        bool shattered = false;
    };

    struct OrganismState
    {
        Species species = Species::Oscillator;
        SonicRole role = SonicRole::Support;
        Presence presence = Presence::Present;
        std::array<Strategy, 2> strategies { Strategy::None, Strategy::None };
        Vec3 position {};
        Vec3 velocity {};
        Vec3 facing { 1.0f, 0.0f, 0.0f };
        Vec3 anchor {};
        float energy = 0.5f;
        float presenceAmount = 1.0f;
        float strength = 0.5f;
        float agility = 0.5f;
        float intelligence = 0.5f;
        float perceptionCosine = 0.25f;
        float turbidity = 0.35f;
        float schoolCohesion = 0.5f;
        int schoolId = -1;
    };

    struct MixState
    {
        float left = 0.0f;
        float right = 0.0f;
        float observation = 0.0f;
        float detail = 0.0f;
    };

    struct Lens
    {
        Vec3 surfacePosition { 1.0f, 0.0f, 0.0f };
        Vec3 targetPosition { 1.0f, 0.0f, 0.0f };
        float focusRadius = 0.38f;
        float glassReach = 0.75f;
        float magnification = 2.5f;
        float peripheralDetail = 0.35f;
        int heldByObserver = -1;
    };

    void prepare(double sampleRate) noexcept;
    void reset(BiomaBrain& brain) noexcept;

    // Called once per rendered sample. Heavy ecological work only runs on an
    // internal 200 Hz clock, independent of host callback boundaries.
    void tickSample(BiomaBrain& brain) noexcept;

    void setDesire(Desire value) noexcept;
    [[nodiscard]] Desire desireTarget() const noexcept { return targetDesire; }
    [[nodiscard]] Desire desireCurrent() const noexcept { return currentDesire; }
    void setOutputEnergy(float value) noexcept;
    void setListeningBlend(std::array<float, observerCount> value) noexcept;
    // Audio-thread control contract. Visual camera motion lives outside the
    // Ecosystem; only an explicit physical pose changes external perception.
    void setAquariumPose(AquariumPose value) noexcept { physicalPose = value; }
    void setAquariumMotionAudioEnabled(bool value) noexcept { motionAffectsAudio = value; }
    [[nodiscard]] bool aquariumMotionAudioEnabled() const noexcept { return motionAffectsAudio; }

    bool holdLens(std::size_t observer) noexcept;
    void releaseLens(std::size_t observer) noexcept;
    void setLensTarget(Vec3 target) noexcept;
    void requestPresence(std::size_t organism, Presence state) noexcept;

    // Explicit stress represents a documented ecological event. Sustained
    // normal operation alone is not allowed to shatter the Aquarium.
    void applyStress(float amount) noexcept;

    [[nodiscard]] const OrganismState& organism(std::size_t index) const noexcept;
    [[nodiscard]] const MixState& mix(std::size_t index) const noexcept;
    // Sample-rate smoothing for sound only. mix() remains the ecological
    // control target used by diagnostics, without changing the 200 Hz model.
    [[nodiscard]] const MixState& audioMix(std::size_t index) const noexcept;
    [[nodiscard]] const Environment& environmentState() const noexcept { return environment; }
    [[nodiscard]] const Integrity& integrityState() const noexcept { return integrity; }
    [[nodiscard]] const Lens& lensState() const noexcept { return lens; }
    [[nodiscard]] std::size_t lensFocusedOrganisms() const noexcept;
    [[nodiscard]] const AquariumPose& aquariumPoseState() const noexcept { return physicalPose; }
    [[nodiscard]] Vec3 aquariumToWorld(Vec3 local) const noexcept;
    [[nodiscard]] Vec3 worldToAquarium(Vec3 world) const noexcept;
    [[nodiscard]] Vec3 organismWorldPosition(std::size_t index) const noexcept;
    [[nodiscard]] float aquariumRadius() const noexcept { return boundaryRadius; }
    [[nodiscard]] float observerDistance(std::size_t index) const noexcept;
    // What `viewer` perceives of `target`, by the same underwater perception
    // the strategies use (field of view, distance, turbidity, presence): the
    // music may only react to what an organism observed (invariants 1 and 2).
    [[nodiscard]] float perceivedBy(std::size_t viewer, std::size_t target) const noexcept;
    [[nodiscard]] float distanceBetween(std::size_t first, std::size_t second) const noexcept;
    // The length of the path a sound travels INSIDE the water from an organism
    // to the main listener (observer 0): the straight line to the listener
    // minus the part in air outside the glass. 0 .. ~2 radii.
    [[nodiscard]] float waterPathToListener(std::size_t organism) const noexcept;
    // Where an organism is, heard from the main listener (observer 0), who
    // faces the aquarium with the head upright: azimuth (radians, + to the
    // right), elevation (radians, + up) and distance. For the binaural mode.
    struct Direction
    {
        float azimuth = 0.0f;
        float elevation = 0.0f;
        float distance = 1.0f;
    };
    [[nodiscard]] Direction directionFromListener(std::size_t organism) const noexcept;
    // the hour and season a session wakes in (SessionGenesis.h), applied at
    // the next reset; 0 and 0 are the classic start
    // the LUPA card (2026-10-24): the lens's radius on the glass, its reach
    // into the water, its magnification and the periphery's detail (the
    // blur); the defaults are the classic lens
    void setLensShape(float focusRadius, float glassReach, float magnification, float peripheralDetail) noexcept
    {
        const auto finite = [](float v, float fallback) { return std::isfinite(v) ? v : fallback; };
        lensShape.focusRadius = std::clamp(finite(focusRadius, 0.38f), 0.05f, 1.5f);
        lensShape.glassReach = std::clamp(finite(glassReach, 0.75f), 0.05f, 2.0f);
        lensShape.magnification = std::clamp(finite(magnification, 2.5f), 1.0f, 8.0f);
        lensShape.peripheralDetail = std::clamp(finite(peripheralDetail, 0.35f), 0.02f, 1.0f);
        lens.focusRadius = lensShape.focusRadius;
        lens.glassReach = lensShape.glassReach;
        lens.magnification = lensShape.magnification;
        lens.peripheralDetail = lensShape.peripheralDetail;
    }
    void setStartPhases(float day, float season) noexcept
    {
        startDayPhase = std::isfinite(day) ? day - std::floor(day) : 0.0f;
        startSeasonPhase = std::isfinite(season) ? season - std::floor(season) : 0.0f;
    }

private:
    struct Motion
    {
        float phase = 0.0f;
        float secondaryPhase = 0.0f;
        float baseSpeed = 0.25f;
        float eventChancePerSecond = 0.04f;
        float pauseChancePerSecond = 0.01f;
        float pauseRemaining = 0.0f;
    };

    struct Observer
    {
        Vec3 position {};
        Vec3 facing {};
        ListeningMode mode = ListeningMode::CollectiveDrift;
        Motion motion {};
        float depthFalloff = 0.035f;
        float occlusionStrength = 0.65f;
    };

    void controlTick(BiomaBrain& brain) noexcept;
    void updateEnvironment(float dt) noexcept;
    void updateDesire(float dt) noexcept;
    void updateIntegrity(float dt, BiomaBrain& brain) noexcept;
    void updateLens(float dt) noexcept;
    void updateObservers(float dt, BiomaBrain& brain) noexcept;
    void updateOrganisms(float dt, BiomaBrain& brain) noexcept;
    void updateMix() noexcept;

    [[nodiscard]] Vec3 motionStep(Motion& motion, float dt, BiomaBrain& brain) const noexcept;
    [[nodiscard]] Vec3 strategyForce(std::size_t index, Strategy strategy, float dt, BiomaBrain& brain) noexcept;
    [[nodiscard]] float underwaterVisibility(const OrganismState& viewer,
                                              const OrganismState& target) const noexcept;
    [[nodiscard]] float observerVisibility(std::size_t observer, std::size_t target) const noexcept;
    [[nodiscard]] float listeningWeight(std::size_t observer, std::size_t target) const noexcept;
    [[nodiscard]] float lensWeight(std::size_t observer, std::size_t target) const noexcept;
    [[nodiscard]] Vec3 acousticWorldPosition(std::size_t organism) const noexcept;

    static constexpr float boundaryRadius = 4.0f;
    static constexpr float observerRadius = 6.0f;
    double preparedSampleRate = 48000.0;
    std::uint32_t controlIntervalSamples = 240;
    std::uint32_t samplesUntilControlTick = 0;
    std::uint64_t elapsedControlTicks = 0;
    float controlDt = 0.005f;
    float audioMixCoefficient = 0.0013879248f; // 15 ms at 48 kHz
    float observedOutputEnergy = 0.0f;
    float requestedOutputEnergy = 0.0f;
    float pendingStress = 0.0f;

    Desire targetDesire {};
    Desire currentDesire {};
    float startDayPhase = 0.0f;
    Lens lensShape {};  // the shape kept across resets (setLensShape)
    float startSeasonPhase = 0.0f;
    Environment environment {};
    Integrity integrity {};
    Lens lens {};
    AquariumPose physicalPose {};
    bool motionAffectsAudio = false;
    float aquariumRotation = 0.0f;
    float aquariumRotationPhase = 0.0f;

    std::array<OrganismState, organismCount> organisms {};
    std::array<Motion, organismCount> organismMotion {};
    std::array<Vec3, organismCount> crackOffset {};
    std::array<Observer, observerCount> observers {};
    std::array<MixState, organismCount> mixState {};
    std::array<MixState, organismCount> audioMixState {};
    std::array<float, observerCount> targetBlend { 0.25f, 0.25f, 0.25f, 0.25f };
    std::array<float, observerCount> currentBlend { 0.25f, 0.25f, 0.25f, 0.25f };
};
}
