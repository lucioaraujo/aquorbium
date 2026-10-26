// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/MultiTapSpace.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <memory>
#include <new>

#define REQUIRE(condition) do { if (!(condition)) { \
    std::cerr << "requirement failed at line " << __LINE__ << ": " #condition "\n"; \
    return EXIT_FAILURE; } } while (false)

namespace
{
std::atomic<bool> trackAllocations { false };
std::atomic<std::size_t> allocationCount { 0 };

void countAllocation() noexcept
{
    if (trackAllocations.load(std::memory_order_relaxed))
        allocationCount.fetch_add(1, std::memory_order_relaxed);
}

bool sameBits(float left, float right)
{
    return std::bit_cast<std::uint32_t>(left) == std::bit_cast<std::uint32_t>(right);
}

aquorbium::MultiTapSpace::Parameters oneTap(
    float delayMilliseconds, float pan, float cutoffHz, float feedback = 0.0f)
{
    aquorbium::MultiTapSpace::Parameters result;
    result.taps[0] = { delayMilliseconds, 1.0f, pan, cutoffHz, true };
    result.feedback = feedback;
    return result;
}
}

void* operator new(std::size_t size)
{
    countAllocation();
    if (auto* memory = std::malloc(size == 0 ? 1 : size))
        return memory;
    throw std::bad_alloc {};
}

void* operator new[](std::size_t size) { return ::operator new(size); }
void operator delete(void* memory) noexcept { std::free(memory); }
void operator delete[](void* memory) noexcept { std::free(memory); }
void operator delete(void* memory, std::size_t) noexcept { std::free(memory); }
void operator delete[](void* memory, std::size_t) noexcept { std::free(memory); }

int main()
{
    using aquorbium::MultiTapSpace;

    auto space = std::make_unique<MultiTapSpace>();
    space->prepare(48000.0);
    for (int sample = 0; sample < 4096; ++sample)
    {
        const auto output = space->tick(0.0f, 0.0f);
        REQUIRE(output.left == 0.0f && output.right == 0.0f);
    }

    // A 504.5-sample delay occupies four neighbouring samples under cubic
    // Lagrange interpolation and remains exactly silent before its support.
    space->prepare(48000.0);
    space->setParameters(oneTap(504.5f / 48.0f, 0.0f, 20000.0f));
    std::array<float, 510> impulseOutput {};
    for (std::size_t sample = 0; sample < impulseOutput.size(); ++sample)
    {
        const auto input = sample == 0 ? 1.0f : 0.0f;
        impulseOutput[sample] = space->tick(input, input).left;
    }
    for (std::size_t sample = 0; sample < 503; ++sample)
        REQUIRE(impulseOutput[sample] == 0.0f);
    REQUIRE(std::abs(impulseOutput[503]) > 1.0e-4f);
    REQUIRE(std::abs(impulseOutput[504]) > std::abs(impulseOutput[503]));
    REQUIRE(std::abs(impulseOutput[505]) > std::abs(impulseOutput[503]));
    REQUIRE(std::abs(impulseOutput[506]) > 1.0e-4f);

    // Independent pan puts a reflection on the requested edge.
    space->prepare(48000.0);
    space->setParameters(oneTap(8.0f, -1.0f, 20000.0f));
    double leftEnergy = 0.0;
    double rightEnergy = 0.0;
    for (int sample = 0; sample < 1200; ++sample)
    {
        const auto input = sample == 0 ? 0.5f : 0.0f;
        const auto output = space->tick(input, input);
        leftEnergy += static_cast<double>(output.left) * output.left;
        rightEnergy += static_cast<double>(output.right) * output.right;
    }
    REQUIRE(leftEnergy > 0.01);
    REQUIRE(rightEnergy < leftEnergy * 1.0e-10);

    // The per-tap low-pass changes spectral energy without changing topology.
    auto open = std::make_unique<MultiTapSpace>();
    auto submerged = std::make_unique<MultiTapSpace>();
    open->prepare(48000.0);
    submerged->prepare(48000.0);
    open->setParameters(oneTap(5.0f, 0.0f, 20000.0f));
    submerged->setParameters(oneTap(5.0f, 0.0f, 180.0f));
    double openEnergy = 0.0;
    double submergedEnergy = 0.0;
    for (int sample = 0; sample < 12000; ++sample)
    {
        const auto input = (sample & 1) == 0 ? 0.25f : -0.25f;
        const auto a = open->tick(input, input);
        const auto b = submerged->tick(input, input);
        if (sample > 1000)
        {
            openEnergy += static_cast<double>(a.left) * a.left;
            submergedEnergy += static_cast<double>(b.left) * b.left;
        }
    }
    REQUIRE(openEnergy > submergedEnergy * 100.0);

    // Feedback creates a bounded memory tail and silence remains an exact
    // fixed point even at the maximum public setting.
    space->prepare(48000.0);
    space->setParameters(oneTap(23.7f, 0.25f, 9000.0f, 0.88f));
    double lateEnergy = 0.0;
    float peak = 0.0f;
    for (int sample = 0; sample < 240000; ++sample)
    {
        const auto input = sample == 0 ? 0.8f : 0.0f;
        const auto output = space->tick(input, input);
        REQUIRE(std::isfinite(output.left) && std::isfinite(output.right));
        peak = std::max({ peak, std::abs(output.left), std::abs(output.right) });
        if (sample > 2400)
            lateEnergy += static_cast<double>(output.left) * output.left
                + static_cast<double>(output.right) * output.right;
    }
    REQUIRE(lateEnergy > 1.0e-6);
    REQUIRE(peak < 1.0f);

    // The analytical normalization is exercised at its actual worst authored
    // case: eight coincident full-gain taps and maximum feedback.
    MultiTapSpace::Parameters coincident;
    for (auto& tap : coincident.taps)
        tap = { 7.37f, 1.0f, 0.0f, 20000.0f, true };
    coincident.feedback = 0.88f;
    space->prepare(48000.0);
    space->setParameters(coincident);
    peak = 0.0f;
    double finalWindowEnergy = 0.0;
    for (int sample = 0; sample < 240000; ++sample)
    {
        const auto input = sample == 0 ? 0.9f : 0.0f;
        const auto output = space->tick(input, input);
        REQUIRE(std::isfinite(output.left) && std::isfinite(output.right));
        peak = std::max({ peak, std::abs(output.left), std::abs(output.right) });
        if (sample > 235200)
            finalWindowEnergy += static_cast<double>(output.left) * output.left
                + static_cast<double>(output.right) * output.right;
    }
    REQUIRE(peak < 1.0f);
    REQUIRE(finalWindowEnergy < 1.0e-16);

    space->reset();
    for (int sample = 0; sample < 4096; ++sample)
    {
        const auto output = space->tick(0.0f, 0.0f);
        REQUIRE(output.left == 0.0f && output.right == 0.0f);
    }

    // Autonomous, hybrid and complete performance mappings are continuous
    // variants of the same DSP, not separate signal paths.
    space->prepare(48000.0);
    space->setParameters(MultiTapSpace::aquariumEarlyReflections());
    const auto autonomous = space->telemetry();
    space->setPerformanceIntent({ 0.5f, 1.0f, 0.0f, 1.0f, 0.9f });
    const auto hybrid = space->telemetry();
    space->setPerformanceIntent({ 1.0f, 1.0f, 0.0f, 1.0f, 0.9f });
    const auto performance = space->telemetry();
    REQUIRE(autonomous.activeTaps == MultiTapSpace::tapCount);
    REQUIRE(autonomous.taps[0].targetDelayMilliseconds
            < hybrid.taps[0].targetDelayMilliseconds);
    REQUIRE(hybrid.taps[0].targetDelayMilliseconds
            < performance.taps[0].targetDelayMilliseconds);
    REQUIRE(std::abs(performance.taps[0].currentPan)
            < std::abs(autonomous.taps[0].currentPan));
    REQUIRE(performance.targetFeedback > autonomous.targetFeedback);

    // A complete macro jump is rendered as a trajectory, with no destructive
    // sample discontinuity.
    space->prepare(48000.0);
    space->setParameters(MultiTapSpace::aquariumEarlyReflections());
    MultiTapSpace::StereoSample previous {};
    for (int sample = 0; sample < 4800; ++sample)
        previous = space->tick(std::sin(static_cast<float>(sample) * 0.031f) * 0.2f,
                               std::sin(static_cast<float>(sample) * 0.027f) * 0.2f);
    space->setPerformanceIntent({ 1.0f, 1.0f, 0.0f, 0.0f, 1.0f });
    float maximumAutomationDelta = 0.0f;
    for (int sample = 0; sample < 12000; ++sample)
    {
        const auto output = space->tick(
            std::sin(static_cast<float>(sample + 4800) * 0.031f) * 0.2f,
            std::sin(static_cast<float>(sample + 4800) * 0.027f) * 0.2f);
        maximumAutomationDelta = std::max(
            maximumAutomationDelta,
            std::max(std::abs(output.left - previous.left),
                     std::abs(output.right - previous.right)));
        previous = output;
    }
    REQUIRE(maximumAutomationDelta < 0.05f);

    // Invalid fields retain the previous valid snapshot; finite extremes clamp.
    space->prepare(48000.0);
    const auto valid = MultiTapSpace::aquariumEarlyReflections();
    space->setParameters(valid);
    auto invalid = valid;
    invalid.taps[0].delayMilliseconds = std::numeric_limits<float>::quiet_NaN();
    invalid.taps[0].gain = std::numeric_limits<float>::infinity();
    invalid.taps[0].pan = -std::numeric_limits<float>::infinity();
    invalid.taps[0].cutoffHz = std::numeric_limits<float>::quiet_NaN();
    invalid.feedback = std::numeric_limits<float>::quiet_NaN();
    space->setParameters(invalid);
    const auto retained = space->parameters();
    REQUIRE(sameBits(retained.taps[0].delayMilliseconds,
                     valid.taps[0].delayMilliseconds));
    REQUIRE(sameBits(retained.taps[0].gain, valid.taps[0].gain));
    REQUIRE(sameBits(retained.taps[0].pan, valid.taps[0].pan));
    REQUIRE(sameBits(retained.taps[0].cutoffHz, valid.taps[0].cutoffHz));
    REQUIRE(sameBits(retained.feedback, valid.feedback));

    // Reset is reproducible bit for bit and runtime mutation allocates nothing.
    constexpr std::size_t reproducibleSamples = 8192;
    std::array<MultiTapSpace::StereoSample, reproducibleSamples> reference {};
    space->reset();
    for (std::size_t sample = 0; sample < reference.size(); ++sample)
    {
        const auto input = sample % 977 == 0 ? 0.4f : 0.0f;
        reference[sample] = space->tick(input, -input * 0.3f);
    }
    space->reset();
    for (std::size_t sample = 0; sample < reference.size(); ++sample)
    {
        const auto input = sample % 977 == 0 ? 0.4f : 0.0f;
        const auto output = space->tick(input, -input * 0.3f);
        REQUIRE(sameBits(output.left, reference[sample].left));
        REQUIRE(sameBits(output.right, reference[sample].right));
    }

    allocationCount.store(0, std::memory_order_relaxed);
    trackAllocations.store(true, std::memory_order_release);
    for (int sample = 0; sample < 20000; ++sample)
    {
        if (sample % 257 == 0)
        {
            const auto gesture = static_cast<float>(sample % 1000) / 999.0f;
            space->setPerformanceIntent(
                { gesture, 1.0f - gesture, gesture, 0.25f + gesture * 0.5f,
                  gesture });
        }
        const auto output = space->tick(
            std::sin(static_cast<float>(sample) * 0.071f) * 0.2f,
            std::sin(static_cast<float>(sample) * 0.053f) * 0.2f);
        if (!std::isfinite(output.left) || !std::isfinite(output.right))
            return EXIT_FAILURE;
    }
    trackAllocations.store(false, std::memory_order_release);
    REQUIRE(allocationCount.load(std::memory_order_relaxed) == 0);

    // Seeded ecological delay drift is reproducible, bounded and opt-in.
    auto driftA = std::make_unique<MultiTapSpace>();
    auto driftB = std::make_unique<MultiTapSpace>();
    for (auto* driftSpace : { driftA.get(), driftB.get() })
    {
        driftSpace->setSeed(0x51a7u);
        driftSpace->prepare(48000.0);
        driftSpace->setParameters(MultiTapSpace::aquariumEarlyReflections());
        driftSpace->setDriftIntent({ 1.0f, 2.0f, 1.2f });
    }
    for (int sample = 0; sample < 48000; ++sample)
    {
        const auto input = sample % 1301 == 0 ? 0.2f : 0.0f;
        const auto a = driftA->tick(input, -input);
        const auto b = driftB->tick(input, -input);
        REQUIRE(sameBits(a.left, b.left));
        REQUIRE(sameBits(a.right, b.right));
    }
    const auto driftTelemetry = driftA->telemetry();
    REQUIRE(std::abs(driftTelemetry.driftValue) > 1.0e-5f);
    REQUIRE(driftTelemetry.driftDepthMilliseconds > 0.0f);
    REQUIRE(driftTelemetry.driftDepthMilliseconds <= 1.21f);

    std::cout << "aquorbium multi-tap space tests passed\n";
}
