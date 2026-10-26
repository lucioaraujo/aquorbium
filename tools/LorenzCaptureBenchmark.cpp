// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/BiomaGranularCapture.h"
#include "core/LorenzCV.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <iostream>

namespace
{
double render(aquorbium::LorenzCV& lorenz,
              aquorbium::BiomaGranularCapture& capture,
              std::size_t samples, bool saturated) noexcept
{
    double checksum = 0.0;
    for (std::size_t sample = 0; sample < samples; ++sample)
    {
        if (saturated && sample % 128 == 0)
            (void)capture.trigger({ 100.0f, 1.7f, 0.2f, 1.0f, sample % 256 == 0 });
        const auto control = lorenz.tick();
        const auto sound = capture.process(0.2f);
        checksum += sound.left + sound.right + control.x * 0.001;
    }
    return checksum;
}

void benchmark(double sampleRate, bool saturated)
{
    aquorbium::LorenzCV lorenz;
    aquorbium::BiomaGranularCapture capture;
    lorenz.prepare(sampleRate);
    capture.prepare(sampleRate);
    auto checksum = render(lorenz, capture,
                           static_cast<std::size_t>(sampleRate * 0.4), saturated);
    std::array<double, 3> elapsed {};
    for (auto& seconds : elapsed)
    {
        const auto start = std::chrono::steady_clock::now();
        checksum += render(lorenz, capture,
                           static_cast<std::size_t>(sampleRate * 0.25), saturated);
        seconds = std::chrono::duration<double>(
            std::chrono::steady_clock::now() - start).count();
    }
    std::sort(elapsed.begin(), elapsed.end());
    const auto median = elapsed[1];
    if (!std::isfinite(checksum) || median <= 0.0) std::exit(EXIT_FAILURE);
    std::cout << static_cast<int>(sampleRate) << ','
              << (saturated ? "twelve-grain load" : "idle") << ','
              << median << ',' << median / 0.25 * 100.0 << ','
              << capture.telemetry().triggered << ','
              << capture.telemetry().rejected << ','
              << sizeof(capture) << ',' << checksum << '\n';
}
}

int main()
{
    std::cout << "sample_rate,mode,median_seconds,cpu_percent_one_core,"
                 "triggered,rejected,bytes_capture,checksum\n";
    std::cout << std::fixed << std::setprecision(6);
    for (const auto sampleRate : { 48000.0, 96000.0 })
        for (const auto saturated : { false, true })
            benchmark(sampleRate, saturated);
}
