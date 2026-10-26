// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/BiomaBrain.h"
#include "core/Ecosystem.h"

#include <array>
#include <bit>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>

#define REQUIRE(condition) do { if (!(condition)) { \
    std::cerr << "requirement failed at line " << __LINE__ << ": " #condition "\n"; \
    return EXIT_FAILURE; } } while (false)

namespace
{
bool sameBits(float left, float right)
{
    return std::bit_cast<std::uint32_t>(left) == std::bit_cast<std::uint32_t>(right);
}

bool finite(const aquorbium::Ecosystem::Vec3& value)
{
    return std::isfinite(value.x) && std::isfinite(value.y) && std::isfinite(value.z);
}

float radius(const aquorbium::Ecosystem::Vec3& value)
{
    return std::sqrt(value.x * value.x + value.y * value.y + value.z * value.z);
}

struct Snapshot
{
    std::array<aquorbium::Ecosystem::OrganismState, aquorbium::Ecosystem::organismCount> organisms {};
    std::array<aquorbium::Ecosystem::MixState, aquorbium::Ecosystem::organismCount> mix {};
    aquorbium::Ecosystem::Environment environment {};
    aquorbium::Ecosystem::Integrity integrity {};
};

Snapshot simulate(std::uint32_t seed)
{
    aquorbium::BiomaBrain brain;
    brain.prepare(48000.0);
    brain.setSeed(seed);
    aquorbium::Ecosystem ecosystem;
    ecosystem.prepare(48000.0);
    ecosystem.setDesire({ 0.38f, 0.71f, 0.66f, 0.62f });
    ecosystem.reset(brain);
    for (int sample = 0; sample < 48000 * 12; ++sample)
    {
        ecosystem.setOutputEnergy(0.24f + std::sin(static_cast<float>(sample) * 0.00031f) * 0.08f);
        ecosystem.tickSample(brain);
    }

    Snapshot result;
    for (std::size_t index = 0; index < aquorbium::Ecosystem::organismCount; ++index)
    {
        result.organisms[index] = ecosystem.organism(index);
        result.mix[index] = ecosystem.mix(index);
    }
    result.environment = ecosystem.environmentState();
    result.integrity = ecosystem.integrityState();
    return result;
}

bool snapshotsEqual(const Snapshot& left, const Snapshot& right)
{
    for (std::size_t index = 0; index < left.organisms.size(); ++index)
    {
        const auto& a = left.organisms[index];
        const auto& b = right.organisms[index];
        if (!sameBits(a.position.x, b.position.x) || !sameBits(a.position.y, b.position.y)
            || !sameBits(a.position.z, b.position.z) || !sameBits(a.presenceAmount, b.presenceAmount)
            || !sameBits(left.mix[index].left, right.mix[index].left)
            || !sameBits(left.mix[index].right, right.mix[index].right)
            || !sameBits(left.mix[index].detail, right.mix[index].detail))
            return false;
    }
    return sameBits(left.environment.dayPhase, right.environment.dayPhase)
        && sameBits(left.environment.seasonPhase, right.environment.seasonPhase)
        && sameBits(left.integrity.crackStress, right.integrity.crackStress);
}
}

int main()
{
    static_assert(aquorbium::Ecosystem::organismCount == 5);
    static_assert(aquorbium::BiomaBrain::organismCount
                  == aquorbium::Ecosystem::organismCount);

    const auto first = simulate(0x12345678u);
    const auto repeated = simulate(0x12345678u);
    const auto different = simulate(0x87654321u);
    REQUIRE(snapshotsEqual(first, repeated));
    REQUIRE(!snapshotsEqual(first, different));

    aquorbium::BiomaBrain brain;
    brain.prepare(48000.0);
    brain.setSeed(73u);
    aquorbium::Ecosystem ecosystem;
    ecosystem.prepare(48000.0);
    ecosystem.setDesire({ -2.0f, 3.0f, 0.6f, -4.0f });
    ecosystem.reset(brain);
    constexpr std::array expectedSpecies {
        aquorbium::Ecosystem::Species::Oscillator,
        aquorbium::Ecosystem::Species::GranularStriker,
        aquorbium::Ecosystem::Species::Resonator,
        aquorbium::Ecosystem::Species::Pulsar,
        aquorbium::Ecosystem::Species::PhysicalString
    };
    for (std::size_t index = 0; index < expectedSpecies.size(); ++index)
        REQUIRE(ecosystem.organism(index).species == expectedSpecies[index]);
    const auto boundedDesire = ecosystem.desireTarget();
    REQUIRE(boundedDesire.calm == 0.0f && boundedDesire.density == 1.0f
        && boundedDesire.danger == 0.0f);

    for (int sample = 0; sample < 48000 * 30; ++sample)
        ecosystem.tickSample(brain);
    REQUIRE(!ecosystem.integrityState().cracked && !ecosystem.integrityState().shattered);

    float leftTotal = 0.0f;
    float rightTotal = 0.0f;
    for (std::size_t index = 0; index < aquorbium::Ecosystem::organismCount; ++index)
    {
        const auto& organism = ecosystem.organism(index);
        const auto& mix = ecosystem.mix(index);
        REQUIRE(finite(organism.position));
        REQUIRE(radius(organism.position) <= ecosystem.aquariumRadius() + 1.0e-4f);
        REQUIRE(std::isfinite(mix.left) && std::isfinite(mix.right));
        REQUIRE(mix.left >= 0.0f && mix.right >= 0.0f);
        leftTotal += mix.left;
        rightTotal += mix.right;
    }
    REQUIRE(leftTotal <= 0.921f && rightTotal <= 0.921f);
    for (std::size_t index = 0; index < aquorbium::Ecosystem::observerCount; ++index)
        REQUIRE(ecosystem.observerDistance(index) > ecosystem.aquariumRadius());

    brain.setSeed(91u);
    ecosystem.setDesire({ 0.5f, 0.5f, 0.5f, 0.5f });
    ecosystem.reset(brain);
    for (int sample = 0; sample < 480; ++sample)
        ecosystem.tickSample(brain);
    const auto detailWithoutLens = ecosystem.mix(0).detail;
    REQUIRE(ecosystem.holdLens(0));
    REQUIRE(!ecosystem.holdLens(1));
    ecosystem.setLensTarget({ 1.0f, 0.0f, 0.0f });
    for (int sample = 0; sample < 480; ++sample)
        ecosystem.tickSample(brain);
    REQUIRE(ecosystem.mix(0).detail > detailWithoutLens);
    ecosystem.releaseLens(0);
    REQUIRE(ecosystem.holdLens(1));

    ecosystem.requestPresence(1, aquorbium::Ecosystem::Presence::Leaving);
    const auto originalPresence = ecosystem.organism(1).presenceAmount;
    for (int sample = 0; sample < 48000 * 5; ++sample)
        ecosystem.tickSample(brain);
    REQUIRE(ecosystem.organism(1).presenceAmount < originalPresence);
    REQUIRE(ecosystem.organism(1).presence == aquorbium::Ecosystem::Presence::Absent);

    ecosystem.applyStress(1.0f);
    for (int sample = 0; sample < 480; ++sample)
        ecosystem.tickSample(brain);
    REQUIRE(ecosystem.integrityState().shattered);
    REQUIRE(ecosystem.integrityState().health == 0.0f);
    for (int sample = 0; sample < 48000; ++sample)
        ecosystem.tickSample(brain);
    REQUIRE(ecosystem.integrityState().shattered);
    REQUIRE(ecosystem.integrityState().crackStress == 1.0f);

    std::cout << "aquorbium ecosystem tests passed\n";
}
