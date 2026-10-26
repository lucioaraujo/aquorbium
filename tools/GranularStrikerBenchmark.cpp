// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/BiomaGranularStriker.h"

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

double render(std::array<aquorbium::BiomaGranularStriker, bodyCount>& bodies,
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
        checksum += sum;
    }
    return checksum;
}

Result benchmark(double sampleRate, bool saturated)
{
    std::array<aquorbium::BiomaGranularStriker, bodyCount> bodies;
    for (std::size_t index = 0; index < bodies.size(); ++index)
    {
        auto& body = bodies[index];
        body.prepare(sampleRate);
        body.setSeed(0x91a37u + static_cast<std::uint32_t>(index) * 0x85ebca6bu);
        body.setParameters({ 1800.0f + static_cast<float>(index) * 370.0f,
                             40.0f, 1.0f, 1.0f, 1.0f, 1.0f });
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
    std::cout << "sample_rate,bodies,mode,strike_interval_samples,median_seconds,"
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
                      << sizeof(aquorbium::BiomaGranularStriker) << '\n';
        }
    }
}
