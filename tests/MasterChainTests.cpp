// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/MasterChain.h"

#include <algorithm>
#include <atomic>
#include <bit>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <new>

#define REQUIRE(condition) do { if (!(condition)) { \
    std::cerr << "requirement failed at line " << __LINE__ << ": " #condition "\n"; \
    return EXIT_FAILURE; } } while (false)

namespace
{
std::atomic<bool> trackAllocations { false };
std::atomic<std::size_t> allocationCount { 0 };
}

void* operator new(std::size_t size)
{
    if (trackAllocations.load(std::memory_order_relaxed))
        allocationCount.fetch_add(1, std::memory_order_relaxed);
    if (auto* memory = std::malloc(size == 0 ? 1 : size)) return memory;
    throw std::bad_alloc {};
}
void* operator new[](std::size_t size) { return ::operator new(size); }
void operator delete(void* memory) noexcept { std::free(memory); }
void operator delete[](void* memory) noexcept { std::free(memory); }
void operator delete(void* memory, std::size_t) noexcept { std::free(memory); }
void operator delete[](void* memory, std::size_t) noexcept { std::free(memory); }

int main()
{
    aquorbium::MasterChain master;
    master.prepare(48000.0);
    for (int sample = 0; sample < 4096; ++sample)
    {
        const auto left = std::sin(static_cast<float>(sample) * 0.071f) * 0.4f;
        const auto right = std::cos(static_cast<float>(sample) * 0.043f) * 0.2f;
        const auto output = master.process(left, right);
        REQUIRE(std::bit_cast<std::uint32_t>(output.left)
                == std::bit_cast<std::uint32_t>(left));
        REQUIRE(std::bit_cast<std::uint32_t>(output.right)
                == std::bit_cast<std::uint32_t>(right));
    }

    master.setParameters({ 1.0f, 0.9f, 0.8f, 0.6f, 1.5f, 0.0f, false });
    double inputEnergy = 0.0, outputEnergy = 0.0;
    for (int sample = 0; sample < 96000; ++sample)
    {
        const auto left = std::sin(static_cast<float>(sample) * 0.19f) * 0.85f;
        const auto right = std::sin(static_cast<float>(sample) * 0.071f) * 0.55f;
        const auto output = master.process(left, right);
        REQUIRE(std::isfinite(output.left) && std::isfinite(output.right));
        inputEnergy += static_cast<double>(left) * left
            + static_cast<double>(right) * right;
        outputEnergy += static_cast<double>(output.left) * output.left
            + static_cast<double>(output.right) * output.right;
    }
    const auto open = master.telemetry();
    REQUIRE(outputEnergy < inputEnergy);
    REQUIRE(open.bandGainReductionDb[1] > 0.1f);
    REQUIRE(open.glueGainReductionDb > 0.05f);
    REQUIRE(open.effectiveWidth > 1.0f);

    master.setParameters({ 1.0f, 0.7f, 0.5f, 0.0f, 1.5f, 1.0f, false });
    for (int sample = 0; sample < 12000; ++sample)
        static_cast<void>(master.process(0.4f, -0.4f));
    REQUIRE(master.telemetry().effectiveWidth < 0.5f);

    const auto invalid = master.process(
        std::numeric_limits<float>::quiet_NaN(),
        std::numeric_limits<float>::infinity());
    REQUIRE(std::isfinite(invalid.left) && std::isfinite(invalid.right));
    REQUIRE(master.telemetry().invalidSamples == 2u);

    // the EQ order (2026-10-23): the tilt before or after the dynamics is a
    // real difference, and changing it while playing makes no step. At the
    // instant of the switch x*eq*c equals x*c*eq; the difference arrives only
    // as the band compressor's envelope reacts (32 ms attack), so no glide is
    // needed - checked: the same result with and without one.
    // A 50 Hz sine in the low band, tilt +1 (the low band -3 dB).
    {
        const auto rms = [](bool after) {
            aquorbium::MasterChain chain;
            chain.prepare(48000.0);
            chain.setParameters({ 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, after });
            double energy = 0.0;
            for (int sample = 0; sample < 96000; ++sample)
            {
                const auto x = std::sin(static_cast<float>(sample) * 6.2831853f * 50.0f / 48000.0f) * 0.9f;
                const auto y = chain.process(x, x);
                if (sample >= 48000)
                    energy += static_cast<double>(y.left) * y.left;
            }
            return 10.0 * std::log10(energy / 48000.0);
        };
        const auto before = rms(false), after = rms(true);
        std::cout << "EQ before dynamics " << before << " dB, after " << after << " dB\n";
        REQUIRE(std::abs(after - before) > 0.3);

        aquorbium::MasterChain chain;
        chain.prepare(48000.0);
        chain.setParameters({ 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, false });
        float steadyStep = 0.0f, switchStep = 0.0f, previous = 0.0f;
        double lateEnergy = 0.0;
        for (int sample = 0; sample < 192000; ++sample)
        {
            if (sample == 96000 + 211)   // off the zero crossing
                chain.setParameters({ 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, true });
            const auto x = std::sin(static_cast<float>(sample) * 6.2831853f * 50.0f / 48000.0f) * 0.9f;
            const auto y = chain.process(x, x).left;
            const auto step = std::abs(y - previous);
            previous = y;
            if (sample > 48000 && sample < 96000)
                steadyStep = std::max(steadyStep, step);
            if (sample >= 96000 && sample < 96000 + 9600)
                switchStep = std::max(switchStep, step);
            if (sample >= 144000)
                lateEnergy += static_cast<double>(y) * y;
        }
        const auto late = 10.0 * std::log10(lateEnergy / 48000.0);
        std::cout << "EQ order switch: largest step " << switchStep << " (steady " << steadyStep
                  << "), settles at " << late << " dB\n";
        REQUIRE(switchStep <= steadyStep * 1.05f);
        REQUIRE(std::abs(late - after) < 0.05);
    }

    allocationCount.store(0, std::memory_order_relaxed);
    trackAllocations.store(true, std::memory_order_release);
    for (int sample = 0; sample < 48000; ++sample)
        static_cast<void>(master.process(
            std::sin(static_cast<float>(sample) * 0.031f) * 0.7f,
            std::sin(static_cast<float>(sample) * 0.047f) * 0.6f));
    trackAllocations.store(false, std::memory_order_release);
    REQUIRE(allocationCount.load(std::memory_order_relaxed) == 0u);

    std::cout << "Aquorbium creative master tests passed\n";
}
