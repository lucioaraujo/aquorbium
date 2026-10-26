// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/MultiTapSpace.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <memory>

namespace
{
constexpr std::size_t measuredIterations = 5;

struct Result
{
    double sampleRate = 0.0;
    bool excited = false;
    double medianSeconds = 0.0;
    double cpuPercent = 0.0;
    double realtimeMultiple = 0.0;
    double checksum = 0.0;
};

double render(aquorbium::MultiTapSpace& space, std::size_t sampleCount,
              bool excited) noexcept
{
    double checksum = 0.0;
    for (std::size_t sample = 0; sample < sampleCount; ++sample)
    {
        const auto left = excited
            ? std::sin(static_cast<float>(sample) * 0.071f) * 0.22f : 0.0f;
        const auto right = excited
            ? std::sin(static_cast<float>(sample) * 0.053f) * 0.19f : 0.0f;
        const auto output = space.tick(left, right);
        checksum += static_cast<double>(output.left) * 0.37 + output.right * 0.63;
    }
    return checksum;
}

Result benchmark(double sampleRate, bool excited)
{
    auto space = std::make_unique<aquorbium::MultiTapSpace>();
    space->prepare(sampleRate);
    space->setParameters(aquorbium::MultiTapSpace::aquariumEarlyReflections());
    space->setPerformanceIntent({ 0.65f, 0.82f, 0.74f, 0.61f, 0.78f });
    auto checksum = render(*space, static_cast<std::size_t>(sampleRate * 0.15),
                           excited);
    const auto measuredSamples = static_cast<std::size_t>(sampleRate * 0.5);
    std::array<double, measuredIterations> elapsed {};
    for (auto& duration : elapsed)
    {
        const auto start = std::chrono::steady_clock::now();
        checksum += render(*space, measuredSamples, excited);
        const auto end = std::chrono::steady_clock::now();
        duration = std::chrono::duration<double>(end - start).count();
    }
    std::sort(elapsed.begin(), elapsed.end());
    const auto median = elapsed[elapsed.size() / 2];
    return { sampleRate, excited, median, median / 0.5 * 100.0,
             0.5 / median, checksum };
}
}

int main()
{
    std::cout << "sample_rate,taps,mode,median_seconds,cpu_percent,"
                 "realtime_multiple,checksum,bytes_per_space\n";
    std::cout << std::fixed << std::setprecision(6);
    for (const auto sampleRate : { 48000.0, 96000.0 })
    {
        for (const auto excited : { false, true })
        {
            const auto result = benchmark(sampleRate, excited);
            if (!std::isfinite(result.checksum) || result.medianSeconds <= 0.0)
                return EXIT_FAILURE;
            std::cout << static_cast<std::size_t>(result.sampleRate) << ','
                      << aquorbium::MultiTapSpace::tapCount << ','
                      << (result.excited ? "excited" : "silence") << ','
                      << result.medianSeconds << ',' << result.cpuPercent << ','
                      << result.realtimeMultiple << ',' << result.checksum << ','
                      << sizeof(aquorbium::MultiTapSpace) << '\n';
        }
    }
}
