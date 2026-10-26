// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/BiomaEngine.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

namespace
{
constexpr std::size_t maximumBlockSize = 1024;
constexpr int measuredIterations = 5;
constexpr double secondsPerIteration = 0.5;

struct Result
{
    double sampleRate = 0.0;
    std::size_t blockSize = 0;
    std::size_t voices = 0;
    double medianElapsedSeconds = 0.0;
    double medianCpuPercent = 0.0;
    double medianRealtimeMultiple = 0.0;
    double checksum = 0.0;
};

double renderSamples(aquorbium::BiomaEngine& engine, std::size_t blockSize,
                     std::size_t totalSamples,
                     std::array<float, maximumBlockSize>& left,
                     std::array<float, maximumBlockSize>& right) noexcept
{
    double checksum = 0.0;
    std::size_t rendered = 0;
    while (rendered < totalSamples)
    {
        const auto count = std::min(blockSize, totalSamples - rendered);
        engine.render(left.data(), right.data(), count);
        checksum += static_cast<double>(left[count - 1])
            + static_cast<double>(right[count - 1]);
        rendered += count;
    }
    return checksum;
}

Result benchmark(double sampleRate, std::size_t blockSize, std::size_t voices,
                 bool granularStrikerEnabled, bool moduleBankEnabled,
                 bool creativeMasterEnabled)
{
    aquorbium::BiomaEngine engine;
    engine.prepare(sampleRate);
    engine.setSeed(0xA91B42C3u);
    engine.setDesire({ 0.35f, 0.68f, 0.62f, 0.48f });
    engine.setEcosystemBlendImmediately(1.0f);
    engine.setEcosystemOutputFeedbackEnabled(true);
    engine.setAutonomousEnabled(true);
    engine.setGranularStrikerEnabled(granularStrikerEnabled);
    engine.setCreativeMasterInfluence(creativeMasterEnabled ? 0.70f : 0.0f);
    if (moduleBankEnabled)
    {
        aquorbium::IntegratedModuleBank::Controls controls {};
        controls.fill(0.6f);
        controls[aquorbium::IntegratedModuleBank::mix] = 0.65f;
        engine.setIntegratedModuleBankControls(controls);
    }

    constexpr std::array<int, aquorbium::BiomaEngine::voiceCount> notes { 36, 43, 48, 55 };
    for (std::size_t voice = 0; voice < voices; ++voice)
        engine.noteOn(notes[voice], 0.72f - static_cast<float>(voice) * 0.06f);

    std::array<float, maximumBlockSize> left {};
    std::array<float, maximumBlockSize> right {};
    const auto measuredSamples = static_cast<std::size_t>(
        std::llround(sampleRate * secondsPerIteration));
    const auto warmupSamples = static_cast<std::size_t>(std::llround(sampleRate * 0.25));
    auto checksum = renderSamples(engine, blockSize, warmupSamples, left, right);

    std::array<double, measuredIterations> elapsed {};
    for (auto& duration : elapsed)
    {
        const auto start = std::chrono::steady_clock::now();
        checksum += renderSamples(engine, blockSize, measuredSamples, left, right);
        const auto end = std::chrono::steady_clock::now();
        duration = std::chrono::duration<double>(end - start).count();
    }
    std::sort(elapsed.begin(), elapsed.end());
    const auto median = elapsed[elapsed.size() / 2];
    const auto audioDuration = static_cast<double>(measuredSamples) / sampleRate;

    return { sampleRate, blockSize, voices, median,
             median / audioDuration * 100.0,
             audioDuration / median,
             checksum };
}
}

int main(int argc, char** argv)
{
    auto granularStrikerEnabled = true;
    auto moduleBankEnabled = false;
    auto creativeMasterEnabled = false;
    for (int argument = 1; argument < argc; ++argument)
    {
        const std::string option(argv[argument]);
        if (option == "--granular-off")
            granularStrikerEnabled = false;
        else if (option == "--module-bank-on")
            moduleBankEnabled = true;
        else if (option == "--creative-master-on")
            creativeMasterEnabled = true;
        else
        {
            std::cerr << "unknown option: " << option << '\n';
            return EXIT_FAILURE;
        }
    }
    constexpr std::array<double, 3> sampleRates { 44100.0, 48000.0, 96000.0 };
    constexpr std::array<std::size_t, 5> blockSizes { 32, 64, 128, 256, 512 };
    constexpr std::array<std::size_t, 3> voiceCounts { 0, 1, 4 };

    std::cout << "sample_rate,block_size,external_voices,autonomy,granular_striker,module_bank,creative_master,"
                 "median_seconds,cpu_percent,realtime_multiple,checksum\n";
    std::cout << std::fixed << std::setprecision(6);
    for (const auto sampleRate : sampleRates)
    {
        for (const auto blockSize : blockSizes)
        {
            for (const auto voices : voiceCounts)
            {
                const auto result = benchmark(
                    sampleRate, blockSize, voices, granularStrikerEnabled,
                    moduleBankEnabled, creativeMasterEnabled);
                if (!std::isfinite(result.checksum)
                    || !std::isfinite(result.medianRealtimeMultiple)
                    || result.medianElapsedSeconds <= 0.0)
                {
                    std::cerr << "invalid benchmark result\n";
                    return EXIT_FAILURE;
                }
                std::cout << static_cast<std::uint32_t>(result.sampleRate) << ','
                          << result.blockSize << ',' << result.voices << ",1,"
                          << (granularStrikerEnabled ? 1 : 0) << ','
                          << (moduleBankEnabled ? 1 : 0) << ','
                          << (creativeMasterEnabled ? 1 : 0) << ','
                          << result.medianElapsedSeconds << ','
                          << result.medianCpuPercent << ','
                          << result.medianRealtimeMultiple << ','
                          << result.checksum << '\n';
            }
        }
    }
}
