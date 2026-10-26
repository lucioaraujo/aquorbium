// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/Ecosystem.h"

#include "core/BiomaBrain.h"
#include "core/Numeric.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace aquorbium
{
namespace
{
using Vec3 = Ecosystem::Vec3;

constexpr float pi = 3.14159265358979323846f;
constexpr float twoPi = 2.0f * pi;

Vec3 add(Vec3 a, Vec3 b) noexcept { return { a.x + b.x, a.y + b.y, a.z + b.z }; }
Vec3 subtract(Vec3 a, Vec3 b) noexcept { return { a.x - b.x, a.y - b.y, a.z - b.z }; }
Vec3 multiply(Vec3 value, float amount) noexcept { return { value.x * amount, value.y * amount, value.z * amount }; }
float dot(Vec3 a, Vec3 b) noexcept { return a.x * b.x + a.y * b.y + a.z * b.z; }
float lengthSquared(Vec3 value) noexcept { return dot(value, value); }
float length(Vec3 value) noexcept { return std::sqrt(lengthSquared(value)); }

Vec3 normalised(Vec3 value, Vec3 fallback = { 1.0f, 0.0f, 0.0f }) noexcept
{
    const auto magnitude = length(value);
    return magnitude > 1.0e-7f ? multiply(value, 1.0f / magnitude) : fallback;
}

Vec3 rotateY(Vec3 value, float angle) noexcept
{
    const auto cosine = std::cos(angle);
    const auto sine = std::sin(angle);
    return { value.x * cosine - value.z * sine,
             value.y,
             value.x * sine + value.z * cosine };
}

float clampUnit(float value) noexcept { return numeric::unit(value, 0.0f); }
float lerp(float from, float to, float amount) noexcept { return from + (to - from) * amount; }

float randomRange(BiomaBrain& brain, float minimum, float maximum) noexcept
{
    return lerp(minimum, maximum, brain.nextRandomUnit());
}

bool hasStrategy(const Ecosystem::OrganismState& organism, Ecosystem::Strategy strategy) noexcept
{
    return organism.strategies[0] == strategy || organism.strategies[1] == strategy;
}
}

void Ecosystem::prepare(double sampleRate) noexcept
{
    preparedSampleRate = numeric::sampleRate(sampleRate);
    controlIntervalSamples = static_cast<std::uint32_t>(
        std::max(1.0, std::round(preparedSampleRate / 200.0)));
    controlDt = static_cast<float>(static_cast<double>(controlIntervalSamples) / preparedSampleRate);
    audioMixCoefficient = static_cast<float>(-std::expm1(-1.0 / (preparedSampleRate * 0.015)));
    samplesUntilControlTick = 0;
}

void Ecosystem::reset(BiomaBrain& brain) noexcept
{
    elapsedControlTicks = 0;
    samplesUntilControlTick = 0;
    environment = {};
    environment.dayPhase = startDayPhase;
    environment.seasonPhase = startSeasonPhase;
    integrity = {};
    lens = {};
    lens.focusRadius = lensShape.focusRadius;
    lens.glassReach = lensShape.glassReach;
    lens.magnification = lensShape.magnification;
    lens.peripheralDetail = lensShape.peripheralDetail;
    aquariumRotation = 0.0f;
    aquariumRotationPhase = randomRange(brain, 0.0f, twoPi);
    observedOutputEnergy = 0.0f;
    requestedOutputEnergy = 0.0f;
    pendingStress = 0.0f;
    currentDesire = targetDesire;
    currentBlend = targetBlend;
    crackOffset.fill({});
    mixState.fill({});

    organisms = {{
        { Species::Oscillator, SonicRole::Foreground, Presence::Present,
          { Strategy::Predator, Strategy::Territorial },
          { 3.25f, 0.20f, 0.10f }, {}, { -1.0f, 0.0f, 0.0f },
          { 3.25f, 0.20f, 0.10f }, 0.78f, 1.0f, 0.72f, 0.58f, 0.68f,
          0.20f, 0.22f, 0.58f, 0 },
        { Species::GranularStriker, SonicRole::Support, Presence::Present,
          { Strategy::Territorial, Strategy::None },
          { -1.45f, 1.10f, 0.65f }, {}, { 1.0f, 0.0f, 0.0f },
          { -1.45f, 1.10f, 0.65f }, 0.48f, 1.0f, 0.46f, 0.62f, 0.52f,
          0.30f, 0.38f, 0.72f, 0 },
        { Species::Resonator, SonicRole::Support, Presence::Present,
          { Strategy::Migratory, Strategy::None },
          { 0.70f, -1.35f, -1.20f }, {}, { 0.0f, 0.0f, 1.0f },
          { 0.70f, -1.35f, -1.20f }, 0.42f, 1.0f, 0.38f, 0.48f, 0.76f,
          0.15f, 0.30f, 0.66f, 0 },
        { Species::Pulsar, SonicRole::Foreground, Presence::Present,
          { Strategy::Prey, Strategy::None },
          { -3.10f, -0.15f, 0.20f }, {}, { 1.0f, 0.0f, 0.0f },
          { -3.10f, -0.15f, 0.20f }, 0.68f, 1.0f, 0.40f, 0.86f, 0.56f,
          0.05f, 0.28f, 0.35f, -1 },
        { Species::PhysicalString, SonicRole::Support, Presence::Present,
          { Strategy::Sessile, Strategy::Territorial },
          { 0.20f, 1.80f, 1.70f }, {}, { 0.0f, -0.6f, -0.8f },
          { 0.20f, 1.80f, 1.70f }, 0.52f, 1.0f, 0.58f, 0.34f, 0.72f,
          0.18f, 0.24f, 0.45f, -1 }
    }};

    for (std::size_t index = 0; index < organismCount; ++index)
    {
        auto& organismState = organisms[index];
        organismState.strength = clampUnit(organismState.strength + randomRange(brain, -0.08f, 0.08f));
        organismState.agility = clampUnit(organismState.agility + randomRange(brain, -0.08f, 0.08f));
        organismState.intelligence = clampUnit(organismState.intelligence + randomRange(brain, -0.08f, 0.08f));

        auto& motion = organismMotion[index];
        motion.phase = randomRange(brain, 0.0f, twoPi);
        motion.secondaryPhase = randomRange(brain, 0.0f, twoPi);
        motion.baseSpeed = randomRange(brain, 0.16f, 0.36f);
        motion.eventChancePerSecond = randomRange(brain, 0.025f, 0.075f);
        motion.pauseChancePerSecond = randomRange(brain, 0.004f, 0.018f);
        motion.pauseRemaining = 0.0f;
    }

    static constexpr std::array<Vec3, observerCount> observerPositions {{
        { observerRadius, 0.0f, 0.0f }, { -observerRadius, 0.0f, 0.0f },
        { 0.0f, observerRadius, 0.0f }, { 0.0f, -observerRadius, 0.0f }
    }};
    static constexpr std::array<ListeningMode, observerCount> modes {{
        ListeningMode::SparseFocus, ListeningMode::EnergyMemory,
        ListeningMode::GeometricCycle, ListeningMode::CollectiveDrift
    }};

    for (std::size_t index = 0; index < observerCount; ++index)
    {
        observers[index] = {};
        observers[index].position = observerPositions[index];
        observers[index].facing = normalised(multiply(observerPositions[index], -1.0f));
        observers[index].mode = modes[index];
        observers[index].motion.phase = randomRange(brain, 0.0f, twoPi);
        observers[index].motion.secondaryPhase = randomRange(brain, 0.0f, twoPi);
        observers[index].motion.baseSpeed = randomRange(brain, 0.025f, 0.065f);
        observers[index].motion.eventChancePerSecond = 0.01f;
        observers[index].motion.pauseChancePerSecond = 0.006f;
    }

    updateEnvironment(0.0f);
    updateMix();
    audioMixState = mixState;
}

void Ecosystem::tickSample(BiomaBrain& brain) noexcept
{
    if (samplesUntilControlTick == 0)
    {
        controlTick(brain);
        samplesUntilControlTick = controlIntervalSamples;
    }
    --samplesUntilControlTick;
    for (std::size_t index = 0; index < organismCount; ++index)
    {
        const auto& target = mixState[index];
        auto& current = audioMixState[index];
        current.left = lerp(current.left, target.left, audioMixCoefficient);
        current.right = lerp(current.right, target.right, audioMixCoefficient);
        current.observation = lerp(current.observation, target.observation, audioMixCoefficient);
        current.detail = lerp(current.detail, target.detail, audioMixCoefficient);
    }
}

void Ecosystem::setDesire(Desire value) noexcept
{
    targetDesire = {
        numeric::unit(value.calm, targetDesire.calm),
        numeric::unit(value.density, targetDesire.density),
        numeric::unit(value.cohesion, targetDesire.cohesion),
        numeric::unit(value.danger, targetDesire.danger)
    };
}

void Ecosystem::setOutputEnergy(float value) noexcept
{
    requestedOutputEnergy = std::isfinite(value) ? clampUnit(value) : 0.0f;
}

void Ecosystem::setListeningBlend(std::array<float, observerCount> value) noexcept
{
    float total = 0.0f;
    for (auto& item : value)
    {
        item = std::isfinite(item) ? std::max(0.0f, item) : 0.0f;
        total += item;
    }
    if (total <= 1.0e-6f)
        targetBlend.fill(0.25f);
    else
        for (std::size_t index = 0; index < observerCount; ++index)
            targetBlend[index] = value[index] / total;
}

bool Ecosystem::holdLens(std::size_t observer) noexcept
{
    if (observer >= observerCount)
        return false;
    if (lens.heldByObserver < 0 || lens.heldByObserver == static_cast<int>(observer))
    {
        lens.heldByObserver = static_cast<int>(observer);
        return true;
    }
    return false;
}

void Ecosystem::releaseLens(std::size_t observer) noexcept
{
    if (lens.heldByObserver == static_cast<int>(observer))
        lens.heldByObserver = -1;
}

void Ecosystem::setLensTarget(Vec3 target) noexcept
{
    if (std::isfinite(target.x) && std::isfinite(target.y) && std::isfinite(target.z))
        lens.targetPosition = normalised(target, lens.targetPosition);
}

void Ecosystem::requestPresence(std::size_t organismIndex, Presence state) noexcept
{
    if (organismIndex < organismCount)
        organisms[organismIndex].presence = state;
}

void Ecosystem::applyStress(float amount) noexcept
{
    if (std::isfinite(amount) && amount > 0.0f)
        pendingStress = std::clamp(pendingStress + amount, 0.0f, 1.0f);
}

const Ecosystem::OrganismState& Ecosystem::organism(std::size_t index) const noexcept
{
    return organisms[std::min(index, organismCount - 1)];
}

const Ecosystem::MixState& Ecosystem::mix(std::size_t index) const noexcept
{
    return mixState[std::min(index, organismCount - 1)];
}

const Ecosystem::MixState& Ecosystem::audioMix(std::size_t index) const noexcept
{
    return audioMixState[std::min(index, organismCount - 1)];
}

std::size_t Ecosystem::lensFocusedOrganisms() const noexcept
{
    if (lens.heldByObserver < 0) return 0;
    std::size_t count = 0;
    for (std::size_t i = 0; i < organismCount; ++i)
        if (organisms[i].presenceAmount > 0.0f
            && lensWeight(static_cast<std::size_t>(lens.heldByObserver), i) > lens.peripheralDetail)
            ++count;
    return count;
}

float Ecosystem::observerDistance(std::size_t index) const noexcept
{
    return length(subtract(observers[std::min(index, observerCount - 1)].position,
                           physicalPose.translation()));
}

Ecosystem::Vec3 Ecosystem::aquariumToWorld(Vec3 local) const noexcept
{
    return physicalPose.localToWorld(rotateY(local, aquariumRotation));
}

Ecosystem::Vec3 Ecosystem::worldToAquarium(Vec3 world) const noexcept
{
    return rotateY(physicalPose.worldToLocal(world), -aquariumRotation);
}

Ecosystem::Vec3 Ecosystem::organismWorldPosition(std::size_t index) const noexcept
{
    const auto bounded = std::min(index, organismCount - 1);
    return aquariumToWorld(add(organisms[bounded].position, crackOffset[bounded]));
}

Ecosystem::Vec3 Ecosystem::acousticWorldPosition(std::size_t index) const noexcept
{
    const auto bounded = std::min(index, organismCount - 1);
    const auto internallyRotated = rotateY(
        add(organisms[bounded].position, crackOffset[bounded]), aquariumRotation);
    return motionAffectsAudio ? physicalPose.localToWorld(internallyRotated)
                              : internallyRotated;
}

void Ecosystem::controlTick(BiomaBrain& brain) noexcept
{
    ++elapsedControlTicks;
    updateDesire(controlDt);
    updateEnvironment(controlDt);
    updateIntegrity(controlDt, brain);
    updateLens(controlDt);
    updateOrganisms(controlDt, brain);
    updateObservers(controlDt, brain);

    aquariumRotationPhase += controlDt * 0.07f;
    aquariumRotation += controlDt * (0.018f + std::sin(aquariumRotationPhase) * 0.006f);
    updateMix();
}

void Ecosystem::updateEnvironment(float dt) noexcept
{
    constexpr float dayLengthSeconds = 75.0f;
    constexpr float seasonLengthSeconds = 300.0f;
    environment.dayPhase = std::fmod(environment.dayPhase + dt / dayLengthSeconds, 1.0f);
    environment.seasonPhase = std::fmod(environment.seasonPhase + dt / seasonLengthSeconds, 1.0f);
    environment.temperature = std::sin(environment.seasonPhase * twoPi) * 0.5f + 0.5f;
    environment.daylight = std::sin(environment.dayPhase * twoPi) * 0.5f + 0.5f;
    const auto seasonalWindow = std::sin(environment.seasonPhase * twoPi) * 0.5f + 0.5f;
    environment.matingPulse = seasonalWindow * seasonalWindow * seasonalWindow * seasonalWindow;
}

void Ecosystem::updateDesire(float dt) noexcept
{
    const auto amount = 1.0f - std::exp(-dt / 0.25f);
    currentDesire.calm = lerp(currentDesire.calm, targetDesire.calm, amount);
    currentDesire.density = lerp(currentDesire.density, targetDesire.density, amount);
    currentDesire.cohesion = lerp(currentDesire.cohesion, targetDesire.cohesion, amount);
    currentDesire.danger = lerp(currentDesire.danger, targetDesire.danger, amount);
    for (std::size_t index = 0; index < observerCount; ++index)
        currentBlend[index] = lerp(currentBlend[index], targetBlend[index], amount);
    observedOutputEnergy = lerp(observedOutputEnergy, requestedOutputEnergy, 1.0f - std::exp(-dt / 0.08f));
}

void Ecosystem::updateIntegrity(float dt, BiomaBrain& brain) noexcept
{
    if (!integrity.shattered)
    {
        const auto explicitStress = pendingStress;
        pendingStress = 0.0f;
        const auto dangerExcess = std::max(0.0f, currentDesire.danger - 0.85f) / 0.15f;
        const auto energyExcess = std::max(0.0f, observedOutputEnergy - 0.55f) / 0.45f;
        const auto sustainedStress = dangerExcess * energyExcess * 0.018f * dt;
        integrity.crackStress = clampUnit(integrity.crackStress + sustainedStress + explicitStress);

        if (integrity.crackStress >= 1.0f)
        {
            integrity.crackStress = 1.0f;
            integrity.cracked = true;
            integrity.shattered = true;
        }
        else if (explicitStress <= 0.0f && dangerExcess <= 0.0f && integrity.crackStress > 0.0f)
        {
            const auto recoveryRate = 0.010f + currentDesire.calm * 0.025f;
            integrity.crackStress = std::max(0.0f, integrity.crackStress - recoveryRate * dt);
        }

        if (!integrity.shattered)
        {
            integrity.cracked = integrity.cracked
                ? integrity.crackStress >= 0.64f
                : integrity.crackStress >= 0.72f;
        }
        integrity.health = 1.0f - integrity.crackStress;
    }

    const auto leak = integrity.crackStress * integrity.crackStress * 0.18f;
    for (auto& offset : crackOffset)
    {
        const Vec3 target { randomRange(brain, -leak, leak), randomRange(brain, -leak, leak),
                            randomRange(brain, -leak, leak) };
        offset = add(multiply(offset, 0.94f), multiply(target, 0.06f));
    }
}

void Ecosystem::updateLens(float dt) noexcept
{
    lens.surfacePosition = normalised(lens.surfacePosition);
    const auto target = normalised(lens.targetPosition, lens.surfacePosition);
    const auto cosine = std::clamp(dot(target, lens.surfacePosition), -1.0f, 1.0f);
    const auto angle = std::acos(cosine);
    const auto step = dt * 0.55f;
    if (angle <= step)
    {
        lens.surfacePosition = target;
        return;
    }
    auto direction = subtract(target, multiply(lens.surfacePosition, cosine));
    // Antipodal targets have no unique tangent. Choose a stable orthogonal
    // direction instead of stalling at the opposite point of the sphere.
    if (lengthSquared(direction) < 1.0e-8f)
    {
        const Vec3 axis = std::abs(lens.surfacePosition.y) < 0.9f
            ? Vec3 { 0.0f, 1.0f, 0.0f } : Vec3 { 1.0f, 0.0f, 0.0f };
        direction = subtract(axis, multiply(lens.surfacePosition, dot(axis, lens.surfacePosition)));
    }
    lens.surfacePosition = normalised(add(multiply(lens.surfacePosition, std::cos(step)),
                                         multiply(normalised(direction), std::sin(step))));
}

Ecosystem::Vec3 Ecosystem::motionStep(Motion& motion, float dt, BiomaBrain& brain) const noexcept
{
    // Fixed random consumption makes changes of pause state local instead of
    // silently shifting every later random decision in the ecosystem.
    const auto pauseRoll = brain.nextRandomUnit();
    const auto eventRoll = brain.nextRandomUnit();
    const Vec3 eventDirection { brain.nextRandomUnit() * 2.0f - 1.0f,
                                brain.nextRandomUnit() * 2.0f - 1.0f,
                                brain.nextRandomUnit() * 2.0f - 1.0f };

    if (motion.pauseRemaining > 0.0f)
    {
        motion.pauseRemaining = std::max(0.0f, motion.pauseRemaining - dt);
        return {};
    }

    const auto pauseChance = motion.pauseChancePerSecond
        * lerp(0.35f, 2.0f, currentDesire.calm) * dt;
    if (pauseRoll < pauseChance)
    {
        motion.pauseRemaining = lerp(0.35f, 2.4f, eventRoll);
        return {};
    }

    motion.secondaryPhase += dt * 0.11f;
    motion.phase += dt * motion.baseSpeed * lerp(0.55f, 1.55f, currentDesire.density)
        * (0.78f + std::sin(motion.secondaryPhase) * 0.22f);
    Vec3 result { std::sin(motion.phase), std::cos(motion.phase * 0.79f),
                  std::sin(motion.phase * 0.31f) };
    result = multiply(result, 0.42f);

    const auto eventChance = motion.eventChancePerSecond
        * lerp(1.7f, 0.18f, currentDesire.calm) * dt;
    if (eventRoll < eventChance)
        result = add(result, multiply(normalised(eventDirection), 0.32f));
    return result;
}

void Ecosystem::updateObservers(float dt, BiomaBrain& brain) noexcept
{
    for (auto& observer : observers)
    {
        const auto drift = motionStep(observer.motion, dt, brain);
        const auto radial = normalised(observer.position);
        const auto tangent = subtract(drift, multiply(radial, dot(drift, radial)));
        observer.position = multiply(normalised(add(observer.position, multiply(tangent, dt))), observerRadius);
        observer.facing = normalised(multiply(observer.position, -1.0f));
    }
}

void Ecosystem::updateOrganisms(float dt, BiomaBrain& brain) noexcept
{
    std::array<Vec3, organismCount> schoolDirection {};
    std::array<int, organismCount> schoolMembers {};
    for (const auto& item : organisms)
    {
        if (item.schoolId >= 0 && item.schoolId < static_cast<int>(organismCount))
        {
            const auto school = static_cast<std::size_t>(item.schoolId);
            schoolDirection[school] = add(schoolDirection[school], item.facing);
            ++schoolMembers[school];
        }
    }
    for (std::size_t school = 0; school < organismCount; ++school)
        if (schoolMembers[school] > 0)
            schoolDirection[school] = normalised(schoolDirection[school]);

    for (std::size_t index = 0; index < organismCount; ++index)
    {
        auto& item = organisms[index];
        auto force = motionStep(organismMotion[index], dt, brain);
        const auto temperatureFactor = lerp(0.65f, 1.35f, environment.temperature);
        force = multiply(force, temperatureFactor);

        if (item.schoolId >= 0 && item.schoolId < static_cast<int>(organismCount))
        {
            const auto school = static_cast<std::size_t>(item.schoolId);
            const auto matingBoost = environment.matingPulse * 0.28f;
            const auto cohesion = clampUnit((item.schoolCohesion + matingBoost)
                                             * lerp(0.35f, 1.35f, currentDesire.cohesion));
            force = add(multiply(force, 1.0f - cohesion), multiply(schoolDirection[school], cohesion));
        }

        for (const auto strategy : item.strategies)
            force = add(force, strategyForce(index, strategy, dt, brain));

        float presenceTarget = 1.0f;
        if (item.presence == Presence::Leaving || item.presence == Presence::Absent)
            presenceTarget = 0.0f;
        const auto presenceAmount = 1.0f - std::exp(-dt / 0.65f);
        item.presenceAmount = lerp(item.presenceAmount, presenceTarget, presenceAmount);
        if (item.presence == Presence::Leaving && item.presenceAmount < 0.01f)
            item.presence = Presence::Absent;
        if (item.presence == Presence::Entering && item.presenceAmount > 0.99f)
            item.presence = Presence::Present;

        const auto response = 1.0f - std::exp(-dt * lerp(1.5f, 5.0f, item.agility));
        item.velocity = add(multiply(item.velocity, 1.0f - response), multiply(force, response));
        item.position = add(item.position, multiply(item.velocity, dt * std::max(0.05f, item.presenceAmount)));

        const auto radius = length(item.position);
        const auto maximumRadius = boundaryRadius * 0.96f;
        if (radius > maximumRadius)
        {
            const auto normal = normalised(item.position);
            item.position = multiply(normal, maximumRadius);
            const auto outwardSpeed = dot(item.velocity, normal);
            if (outwardSpeed > 0.0f)
                item.velocity = subtract(item.velocity, multiply(normal, outwardSpeed * 1.65f));
        }
        if (lengthSquared(item.velocity) > 1.0e-8f)
            item.facing = normalised(item.velocity, item.facing);
    }
}

Ecosystem::Vec3 Ecosystem::strategyForce(std::size_t index, Strategy strategy, float dt,
                                          BiomaBrain& brain) noexcept
{
    (void) dt;
    auto& self = organisms[index];
    switch (strategy)
    {
        case Strategy::Territorial:
        {
            Vec3 repulsion {};
            constexpr float territoryRadius = 1.45f;
            for (std::size_t otherIndex = 0; otherIndex < organismCount; ++otherIndex)
            {
                const auto& other = organisms[otherIndex];
                if (otherIndex == index || other.species != self.species)
                    continue;
                const auto visibility = underwaterVisibility(self, other);
                const auto difference = subtract(self.position, other.position);
                const auto distance = length(difference);
                if (distance > 1.0e-5f && distance < territoryRadius)
                    repulsion = add(repulsion, multiply(normalised(difference),
                        (territoryRadius - distance) * visibility * self.strength * 0.55f));
            }
            return repulsion;
        }
        case Strategy::Sessile:
            return multiply(subtract(self.anchor, self.position), 0.42f);
        case Strategy::Migratory:
        {
            const auto time = static_cast<float>(elapsedControlTicks) * controlDt;
            const auto phase = time * 0.065f + static_cast<float>(index) * 1.7f;
            const Vec3 target { std::cos(phase) * 2.8f, self.anchor.y, std::sin(phase) * 2.8f };
            return multiply(subtract(target, self.position), 0.16f);
        }
        case Strategy::Predator:
        {
            const Vec3 jitter { brain.nextRandomUnit() * 2.0f - 1.0f,
                                brain.nextRandomUnit() * 2.0f - 1.0f,
                                brain.nextRandomUnit() * 2.0f - 1.0f };
            const OrganismState* target = nullptr;
            float targetScore = -std::numeric_limits<float>::infinity();
            for (std::size_t otherIndex = 0; otherIndex < organismCount; ++otherIndex)
            {
                const auto& other = organisms[otherIndex];
                if (otherIndex == index || !hasStrategy(other, Strategy::Prey))
                    continue;
                const auto visibility = underwaterVisibility(self, other);
                const auto distance = length(subtract(other.position, self.position));
                const auto score = visibility - distance * 0.08f;
                if (visibility > 0.0f && distance < 3.8f && score > targetScore)
                {
                    target = &other;
                    targetScore = score;
                }
            }
            if (target == nullptr)
                return {};
            auto direction = normalised(subtract(target->position, self.position));
            direction = normalised(add(direction, multiply(jitter, (1.0f - self.intelligence) * 0.32f)), direction);
            const auto night = 1.0f + (1.0f - environment.daylight) * 0.25f;
            return multiply(direction, self.strength * self.agility * night
                                       * lerp(0.35f, 1.45f, currentDesire.danger));
        }
        case Strategy::Prey:
        {
            Vec3 escape {};
            for (std::size_t otherIndex = 0; otherIndex < organismCount; ++otherIndex)
            {
                const auto& other = organisms[otherIndex];
                if (otherIndex == index || !hasStrategy(other, Strategy::Predator))
                    continue;
                const auto visibility = underwaterVisibility(self, other);
                const auto difference = subtract(other.position, self.position);
                const auto distance = length(difference);
                const auto fearRadius = 2.2f + self.intelligence * 1.5f;
                if (visibility > 0.0f && distance > 1.0e-5f && distance < fearRadius)
                    escape = subtract(escape, multiply(normalised(difference),
                        (fearRadius - distance) * visibility));
            }
            return multiply(escape, self.agility * lerp(0.45f, 1.35f, currentDesire.danger));
        }
        case Strategy::None:
            return {};
    }
    return {};
}

float Ecosystem::perceivedBy(std::size_t viewer, std::size_t target) const noexcept
{
    if (viewer >= organismCount || target >= organismCount || viewer == target)
        return 0.0f;
    return underwaterVisibility(organisms[viewer], organisms[target]);
}

float Ecosystem::distanceBetween(std::size_t first, std::size_t second) const noexcept
{
    if (first >= organismCount || second >= organismCount)
        return 0.0f;
    return length(subtract(organisms[first].position, organisms[second].position));
}

float Ecosystem::waterPathToListener(std::size_t organism) const noexcept
{
    if (organism >= organismCount)
        return 0.0f;
    const auto total = length(subtract(organismWorldPosition(organism), observers[0].position));
    const auto airPath = std::max(0.0f, observerDistance(0) - boundaryRadius);
    return std::clamp(total - airPath, 0.0f, boundaryRadius * 2.0f);
}

Ecosystem::Direction Ecosystem::directionFromListener(std::size_t organism) const noexcept
{
    Direction result;
    if (organism >= organismCount)
        return result;
    const auto& listener = observers[0];
    const auto toSource = subtract(acousticWorldPosition(organism), listener.position);
    result.distance = std::max(1.0e-4f, length(toSource));
    // the head's frame: front where the listener faces, up the world's y,
    // right = up x front
    const auto front = normalised(listener.facing, { 0.0f, 0.0f, 1.0f });
    Vec3 up { 0.0f, 1.0f, 0.0f };
    up = normalised(subtract(up, multiply(front, dot(up, front))), { 0.0f, 1.0f, 0.0f });
    const Vec3 right { up.y * front.z - up.z * front.y, up.z * front.x - up.x * front.z, up.x * front.y - up.y * front.x };
    const auto x = dot(toSource, right), y = dot(toSource, up), z = dot(toSource, front);
    result.azimuth = std::atan2(x, z);
    result.elevation = std::atan2(y, std::sqrt(x * x + z * z));
    return result;
}

float Ecosystem::underwaterVisibility(const OrganismState& viewer,
                                       const OrganismState& target) const noexcept
{
    if (target.presenceAmount <= 0.0f)
        return 0.0f;
    const auto difference = subtract(target.position, viewer.position);
    const auto distance = length(difference);
    if (distance <= 1.0e-5f)
        return 1.0f;
    const auto alignment = dot(normalised(difference), normalised(viewer.facing));
    if (alignment < viewer.perceptionCosine)
        return 0.0f;
    const auto extinction = 0.28f + clampUnit(viewer.turbidity) * 1.9f;
    return clampUnit(std::exp(-distance * extinction) * target.presenceAmount);
}

float Ecosystem::observerVisibility(std::size_t observerIndex, std::size_t targetIndex) const noexcept
{
    const auto& observer = observers[observerIndex];
    const auto targetPosition = acousticWorldPosition(targetIndex);
    const auto toTarget = subtract(targetPosition, observer.position);
    const auto distance = length(toTarget);
    if (distance <= 1.0e-5f)
        return 0.0f;
    const auto targetDirection = normalised(toTarget);
    const auto alignment = dot(targetDirection, observer.facing);
    if (alignment <= 0.0f)
        return 0.0f;

    auto occlusion = 1.0f;
    for (std::size_t otherIndex = 0; otherIndex < organismCount; ++otherIndex)
    {
        if (otherIndex == targetIndex)
            continue;
        const auto toOther = subtract(acousticWorldPosition(otherIndex), observer.position);
        const auto otherDistance = length(toOther);
        if (otherDistance <= 1.0e-5f || otherDistance >= distance)
            continue;
        const auto sameLine = dot(normalised(toOther), targetDirection);
        if (sameLine > 0.985f)
            occlusion *= clampUnit(1.0f - organisms[otherIndex].energy
                * organisms[otherIndex].presenceAmount * observer.occlusionStrength);
    }
    const auto depth = 1.0f / (1.0f + distance * distance * observer.depthFalloff);
    return clampUnit(alignment * depth * occlusion);
}

float Ecosystem::listeningWeight(std::size_t observerIndex, std::size_t targetIndex) const noexcept
{
    const auto& item = organisms[targetIndex];
    const auto time = static_cast<float>(elapsedControlTicks) * controlDt;
    switch (observers[observerIndex].mode)
    {
        case ListeningMode::SparseFocus:
        {
            const auto pulse = std::sin(time * 3.7f + item.position.x * 2.9f + static_cast<float>(targetIndex));
            return pulse > 0.35f ? 1.0f : 0.08f;
        }
        case ListeningMode::EnergyMemory:
            return std::clamp(0.38f + item.energy * 0.48f - observedOutputEnergy * 0.24f, 0.08f, 1.0f);
        case ListeningMode::GeometricCycle:
        {
            const auto rotation = static_cast<std::size_t>(time * 1.6f) % organismCount;
            return ((targetIndex + rotation) % organismCount == 0) ? 1.0f : 0.28f;
        }
        case ListeningMode::CollectiveDrift:
            return std::clamp(0.66f + std::sin(time * 0.17f + item.position.y) * 0.26f, 0.2f, 1.0f);
    }
    return 0.0f;
}

float Ecosystem::lensWeight(std::size_t observerIndex, std::size_t targetIndex) const noexcept
{
    if (lens.heldByObserver != static_cast<int>(observerIndex))
        return 1.0f;
    // The Lens is attached to the glass, so both operands stay in Aquarium
    // coordinates. Moving the physical tank cannot detach or displace it.
    const auto position = add(organisms[targetIndex].position, crackOffset[targetIndex]);
    const auto radius = length(position);
    if (radius > 1.0e-5f && std::abs(radius - boundaryRadius) <= lens.glassReach)
    {
        const auto pointOnGlass = multiply(position, 1.0f / radius);
        const auto distance = length(subtract(pointOnGlass, lens.surfacePosition));
        const auto focus = std::clamp(1.0f - distance / lens.focusRadius, 0.0f, 1.0f);
        const auto smoothFocus = focus * focus * (3.0f - 2.0f * focus);
        return lerp(lens.peripheralDetail, lens.magnification, smoothFocus);
    }
    return lens.peripheralDetail;
}

void Ecosystem::updateMix() noexcept
{
    static constexpr std::array<float, observerCount> observerPan { -0.78f, 0.78f, -0.26f, 0.26f };
    float leftTotal = 0.0f;
    float rightTotal = 0.0f;

    for (std::size_t target = 0; target < organismCount; ++target)
    {
        auto& result = mixState[target];
        result = {};
        float detailTotal = 0.0f;
        float blendTotal = 0.0f;
        for (std::size_t observer = 0; observer < observerCount; ++observer)
        {
            const auto visibility = observerVisibility(observer, target);
            const auto listening = listeningWeight(observer, target);
            const auto lensEffect = lensWeight(observer, target);
            const auto lensGain = lensEffect >= 1.0f ? 1.0f + (lensEffect - 1.0f) * 0.22f : lensEffect;
            const auto weight = visibility * listening * currentBlend[observer] * lensGain;
            const auto angle = (observerPan[observer] + 1.0f) * pi * 0.25f;
            result.left += weight * std::cos(angle);
            result.right += weight * std::sin(angle);
            result.observation += visibility * currentBlend[observer];
            detailTotal += lensEffect * visibility * currentBlend[observer];
            blendTotal += visibility * currentBlend[observer];
        }

        const auto& item = organisms[target];
        const auto roleGain = item.role == SonicRole::Foreground ? 1.0f : 0.56f;
        const auto energyGain = lerp(0.45f, 1.0f, clampUnit(item.energy));
        const auto presenceGain = clampUnit(item.presenceAmount);
        result.left *= roleGain * energyGain * presenceGain;
        result.right *= roleGain * energyGain * presenceGain;
        result.observation = clampUnit(result.observation);
        result.detail = blendTotal > 1.0e-6f ? clampUnit(detailTotal / blendTotal / lens.magnification) : 0.0f;
        leftTotal += result.left;
        rightTotal += result.right;
    }

    const auto busiestChannel = std::max(leftTotal, rightTotal);
    const auto normalisation = busiestChannel > 0.92f ? 0.92f / busiestChannel : 1.0f;
    for (auto& result : mixState)
    {
        result.left *= normalisation;
        result.right *= normalisation;
    }
}
}
