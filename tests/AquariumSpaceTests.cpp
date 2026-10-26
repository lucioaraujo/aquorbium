// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/AquariumPose.h"
#include "core/AquariumSpatializer.h"
#include "core/BiomaBrain.h"
#include "core/Ecosystem.h"

#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>

#define REQUIRE(condition) do { if (!(condition)) { \
    std::cerr << "requirement failed at line " << __LINE__ << ": " #condition "\n"; \
    return EXIT_FAILURE; } } while (false)

namespace
{
using aquorbium::SpatialVector3;

float distance(SpatialVector3 left, SpatialVector3 right)
{
    const auto x = left.x - right.x;
    const auto y = left.y - right.y;
    const auto z = left.z - right.z;
    return std::sqrt(x * x + y * y + z * z);
}

bool sameBits(float left, float right)
{
    return std::bit_cast<std::uint32_t>(left) == std::bit_cast<std::uint32_t>(right);
}
}

int main()
{
    constexpr float pi = 3.14159265358979323846f;
    const auto pose = aquorbium::AquariumPose::fromYawPitchRoll(
        { 1.7f, -0.8f, 2.1f }, pi * 0.37f, -pi * 0.18f, pi * 0.11f);
    const SpatialVector3 local { 2.2f, -1.1f, 0.7f };
    const auto world = pose.localToWorld(local);
    REQUIRE(distance(pose.worldToLocal(world), local) < 2.0e-6f);

    const SpatialVector3 direction { -0.3f, 0.6f, 0.8f };
    const auto worldDirection = pose.localDirectionToWorld(direction);
    REQUIRE(distance(pose.worldDirectionToLocal(worldDirection), direction) < 2.0e-6f);

    aquorbium::AquariumPose retained = pose;
    const auto beforeInvalid = retained.localToWorld(local);
    REQUIRE(!retained.set({ std::numeric_limits<float>::quiet_NaN(), 0.0f, 0.0f }, {}));
    REQUIRE(distance(retained.localToWorld(local), beforeInvalid) == 0.0f);
    REQUIRE(!retained.set({}, { 0.0f, 0.0f, 0.0f, 0.0f }));
    REQUIRE(distance(retained.localToWorld(local), beforeInvalid) == 0.0f);

    // Physical pose changes world coordinates, never local containment.
    aquorbium::BiomaBrain brain;
    brain.prepare(48000.0);
    brain.setSeed(0x57a11u);
    aquorbium::Ecosystem ecosystem;
    ecosystem.prepare(48000.0);
    ecosystem.reset(brain);
    const auto localBefore = ecosystem.organism(0).position;
    ecosystem.setAquariumPose(pose);
    const auto transformed = ecosystem.aquariumToWorld(localBefore);
    REQUIRE(distance(ecosystem.worldToAquarium(transformed), localBefore) < 2.0e-5f);
    REQUIRE(distance(ecosystem.organism(0).position, localBefore) == 0.0f);
    REQUIRE(distance(ecosystem.organismWorldPosition(0), localBefore) > 0.1f);

    // A visual/physical pose is acoustically inert by default. Linking it is
    // explicit, so camera or layout animation cannot become hidden modulation.
    aquorbium::BiomaBrain referenceBrain;
    aquorbium::BiomaBrain movedBrain;
    referenceBrain.prepare(48000.0);
    movedBrain.prepare(48000.0);
    referenceBrain.setSeed(0x9137u);
    movedBrain.setSeed(0x9137u);
    aquorbium::Ecosystem referenceEcosystem;
    aquorbium::Ecosystem movedEcosystem;
    referenceEcosystem.prepare(48000.0);
    movedEcosystem.prepare(48000.0);
    referenceEcosystem.reset(referenceBrain);
    movedEcosystem.reset(movedBrain);
    movedEcosystem.setAquariumPose(pose);
    for (int sample = 0; sample < 4800; ++sample)
    {
        referenceEcosystem.tickSample(referenceBrain);
        movedEcosystem.tickSample(movedBrain);
    }
    for (std::size_t index = 0; index < aquorbium::Ecosystem::organismCount; ++index)
    {
        REQUIRE(sameBits(referenceEcosystem.mix(index).left, movedEcosystem.mix(index).left));
        REQUIRE(sameBits(referenceEcosystem.mix(index).right, movedEcosystem.mix(index).right));
    }
    movedEcosystem.setAquariumMotionAudioEnabled(true);
    for (int sample = 0; sample < 480; ++sample)
    {
        referenceEcosystem.tickSample(referenceBrain);
        movedEcosystem.tickSample(movedBrain);
    }
    bool linkedPoseChangedMix = false;
    for (std::size_t index = 0; index < aquorbium::Ecosystem::organismCount; ++index)
        linkedPoseChangedMix = linkedPoseChangedMix
            || !sameBits(referenceEcosystem.mix(index).left, movedEcosystem.mix(index).left)
            || !sameBits(referenceEcosystem.mix(index).right, movedEcosystem.mix(index).right);
    REQUIRE(linkedPoseChangedMix);

    aquorbium::AquariumSpatializer spatializer;
    spatializer.prepare(48000.0);
    spatializer.setTarget({ 0.0f, 0.8f, 0.0f, 1.0f });
    for (int sample = 0; sample < 48000; ++sample)
    {
        const auto output = spatializer.tick(0.25f);
        REQUIRE(std::isfinite(output.left) && std::isfinite(output.right));
    }
    const auto centre = spatializer.telemetry();
    REQUIRE(std::abs(centre.targetLeftGain - centre.targetRightGain) < 1.0e-6f);
    REQUIRE(std::abs(centre.targetLeftGain * centre.targetLeftGain
                     + centre.targetRightGain * centre.targetRightGain - 0.64f) < 1.0e-5f);

    aquorbium::AquariumSpatializer gainPair;
    gainPair.prepare(48000.0);
    gainPair.setGainPair(0.31f, 0.77f, 0.42f, 0.68f);
    const auto validPair = gainPair.telemetry();
    gainPair.setGainPair(std::numeric_limits<float>::quiet_NaN(), 0.5f, 0.5f, 0.5f);
    const auto retainedPair = gainPair.telemetry();
    REQUIRE(sameBits(validPair.target.pan, retainedPair.target.pan)
        && sameBits(validPair.target.gain, retainedPair.target.gain)
        && sameBits(validPair.target.depth, retainedPair.target.depth)
        && sameBits(validPair.target.width, retainedPair.target.width));

    spatializer.setTarget({ -1.0f, 1.0f, 0.0f, 1.0f });
    for (int sample = 0; sample < 48000; ++sample)
        (void) spatializer.tick(0.2f);
    const auto leftEdge = spatializer.telemetry();
    REQUIRE(leftEdge.currentLeftGain > 0.999f && leftEdge.currentRightGain < 0.001f);

    spatializer.setTarget({ 1.0f, 1.0f, 0.0f, 1.0f });
    auto previous = spatializer.tick(0.2f);
    float maximumAutomationDelta = 0.0f;
    for (int sample = 0; sample < 4800; ++sample)
    {
        const auto output = spatializer.tick(0.2f);
        maximumAutomationDelta = std::max(maximumAutomationDelta,
            std::max(std::abs(output.left - previous.left),
                     std::abs(output.right - previous.right)));
        previous = output;
    }
    REQUIRE(maximumAutomationDelta < 0.002f);

    spatializer.setTarget({ 1.0f, 1.0f, 0.0f, 0.0f });
    REQUIRE(std::abs(spatializer.telemetry().effectivePan) < 1.0e-7f);

    aquorbium::AquariumSpatializer near;
    aquorbium::AquariumSpatializer deep;
    near.prepare(48000.0);
    deep.prepare(48000.0);
    near.setTarget({ 0.0f, 1.0f, 0.0f, 1.0f });
    deep.setTarget({ 0.0f, 1.0f, 1.0f, 1.0f });
    double nearEnergy = 0.0;
    double deepEnergy = 0.0;
    for (int sample = 0; sample < 48000; ++sample)
    {
        const auto input = (sample & 1) == 0 ? 0.4f : -0.4f;
        const auto nearSample = near.tick(input);
        const auto deepSample = deep.tick(input);
        if (sample > 4800)
        {
            nearEnergy += static_cast<double>(nearSample.left) * nearSample.left;
            deepEnergy += static_cast<double>(deepSample.left) * deepSample.left;
        }
    }
    REQUIRE(nearEnergy > deepEnergy * 20.0);

    aquorbium::AquariumSpatializer reference;
    aquorbium::AquariumSpatializer challenged;
    for (auto* item : { &reference, &challenged })
    {
        item->prepare(48000.0);
        item->setTarget({ -0.27f, 0.73f, 0.46f, 0.82f });
    }
    challenged.setTarget({ std::numeric_limits<float>::quiet_NaN(),
                           std::numeric_limits<float>::infinity(),
                           -std::numeric_limits<float>::infinity(),
                           std::numeric_limits<float>::quiet_NaN() });
    for (int sample = 0; sample < 8192; ++sample)
    {
        const auto input = std::sin(static_cast<float>(sample) * 0.071f);
        const auto a = reference.tick(input);
        const auto b = challenged.tick(input);
        REQUIRE(sameBits(a.left, b.left) && sameBits(a.right, b.right));
    }

    aquorbium::AquariumSpatializer silent;
    silent.prepare(48000.0);
    for (int sample = 0; sample < 4096; ++sample)
    {
        const auto output = silent.tick(0.0f);
        REQUIRE(output.left == 0.0f && output.right == 0.0f);
    }

    std::cout << "aquorbium pose and spatializer tests passed\n";
}
