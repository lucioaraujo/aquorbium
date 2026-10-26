// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/BiomaPhysicalString.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <iomanip>
#include <iostream>

namespace
{
constexpr std::size_t bodyCount = 5;
constexpr std::size_t measuredIterations = 5;

struct Result
{
    double sampleRate = 0.0;
    bool saturated = false;
    double medianSeconds = 0.0;
    double cpuPercent = 0.0;
    double realtimeMultiple = 0.0;
    double checksum = 0.0;
};

double render(std::array<aquorbium::BiomaPhysicalString, bodyCount>& bodies,
              std::size_t sampleCount, bool saturated) noexcept
{
    double checksum = 0.0;
    for (std::size_t sample = 0; sample < sampleCount; ++sample)
    {
        if (saturated && sample % 128 == 0)
            for (auto& body : bodies)
                body.excite(1.0f);
        float sum = 0.0f;
        for (auto& body : bodies)
            sum += body.tick();
        checksum += static_cast<double>(sum);
    }
    return checksum;
}

Result benchmark(double sampleRate, bool saturated)
{
    std::array<aquorbium::BiomaPhysicalString, bodyCount> bodies;
    for (std::size_t index = 0; index < bodies.size(); ++index)
    {
        auto& body = bodies[index];
        body.prepare(sampleRate);
        body.setSeed(0x273a1u + static_cast<std::uint32_t>(index) * 0x85ebca6bu);
        body.setParameters({ 32.7f + static_cast<float>(index) * 7.3f,
                             12.0f, 1.0f, 1.0f, 0.5f });
    }
    const auto measuredSamples = static_cast<std::size_t>(sampleRate * 0.5);
    auto checksum = render(bodies, static_cast<std::size_t>(sampleRate * 0.15),
                           saturated);
    std::array<double, measuredIterations> elapsed {};
    for (auto& duration : elapsed)
    {
        const auto start = std::chrono::steady_clock::now();
        checksum += render(bodies, measuredSamples, saturated);
        const auto end = std::chrono::steady_clock::now();
        duration = std::chrono::duration<double>(end - start).count();
    }
    std::sort(elapsed.begin(), elapsed.end());
    const auto median = elapsed[elapsed.size() / 2];
    return { sampleRate, saturated, median, median / 0.5 * 100.0,
             0.5 / median, checksum };
}
}

int main()
{
    std::cout << "sample_rate,bodies,mode,excitation_interval_samples,median_seconds,"
                 "cpu_percent,realtime_multiple,checksum,bytes_per_body\n";
    std::cout << std::fixed << std::setprecision(6);
    for (const auto sampleRate : { 48000.0, 96000.0 })
    {
        for (const auto saturated : { false, true })
        {
            const auto result = benchmark(sampleRate, saturated);
            if (!std::isfinite(result.checksum) || result.medianSeconds <= 0.0)
                return EXIT_FAILURE;
            std::cout << static_cast<std::size_t>(result.sampleRate) << ','
                      << bodyCount << ',' << (saturated ? "saturated" : "idle") << ','
                      << (saturated ? 128 : 0) << ',' << result.medianSeconds << ','
                      << result.cpuPercent << ',' << result.realtimeMultiple << ','
                      << result.checksum << ','
                      << sizeof(aquorbium::BiomaPhysicalString) << '\n';
        }
    }
}
