// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/BiomaEngine.h"

#include <algorithm>
#include <array>
#include <bit>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <vector>

namespace
{
constexpr double sampleRate = 48000.0;
constexpr double durationSeconds = 20.0;
constexpr std::size_t frames = static_cast<std::size_t>(sampleRate * durationSeconds);
constexpr std::size_t blockSize = 240;

struct Scenario
{
    const char* name;
    aquorbium::AutonomousEnergy::FormMemoryPerformance performance;
};

struct Result
{
    std::vector<float> audio;
    aquorbium::AutonomousEnergy::Telemetry telemetry;
    double renderRatio = 0.0;
    double rms = 0.0;
    double peak = 0.0;
    double maximumDelta = 0.0;
};

void writeU16(std::ofstream& stream, std::uint16_t value)
{
    stream.put(static_cast<char>(value & 0xffu));
    stream.put(static_cast<char>((value >> 8u) & 0xffu));
}

void writeU32(std::ofstream& stream, std::uint32_t value)
{
    for (unsigned shift = 0; shift < 32; shift += 8)
        stream.put(static_cast<char>((value >> shift) & 0xffu));
}

bool writeWav(const std::filesystem::path& path, const std::vector<float>& audio)
{
    const auto bytes = static_cast<std::uint32_t>(audio.size() * sizeof(float));
    std::ofstream stream(path, std::ios::binary);
    if (!stream) return false;
    stream.write("RIFF", 4); writeU32(stream, 36u + bytes);
    stream.write("WAVEfmt ", 8); writeU32(stream, 16u);
    writeU16(stream, 3u); writeU16(stream, 2u);
    writeU32(stream, 48000u); writeU32(stream, 48000u * 8u);
    writeU16(stream, 8u); writeU16(stream, 32u);
    stream.write("data", 4); writeU32(stream, bytes);
    stream.write(reinterpret_cast<const char*>(audio.data()),
                 static_cast<std::streamsize>(bytes));
    return static_cast<bool>(stream);
}

Result render(const Scenario& scenario)
{
    auto engine = std::make_unique<aquorbium::BiomaEngine>();
    engine->prepare(sampleRate);
    engine->setSeed(0x815ab1eu);
    engine->setDesire({ 0.62f, 0.56f, 0.68f, 0.42f });
    engine->setEcosystemOutputFeedbackEnabled(false);
    engine->setAutonomousEnabled(true);
    engine->setFormMemoryPerformance(scenario.performance);
    Result result;
    result.audio.resize(frames * 2);
    std::array<float, blockSize> left {}, right {};
    const auto started = std::chrono::steady_clock::now();
    for (std::size_t offset = 0; offset < frames; offset += blockSize)
    {
        const auto count = std::min(blockSize, frames - offset);
        engine->render(left.data(), right.data(), count);
        for (std::size_t sample = 0; sample < count; ++sample)
        {
            result.audio[(offset + sample) * 2] = left[sample];
            result.audio[(offset + sample) * 2 + 1] = right[sample];
        }
    }
    result.renderRatio = std::chrono::duration<double>(
        std::chrono::steady_clock::now() - started).count() / durationSeconds;
    result.telemetry = engine->telemetry().autonomy;
    double squared = 0.0;
    for (std::size_t index = 0; index < result.audio.size(); ++index)
    {
        const auto value = static_cast<double>(result.audio[index]);
        if (!std::isfinite(value)) { result.rms = -1.0; return result; }
        squared += value * value;
        result.peak = std::max(result.peak, std::abs(value));
        if (index >= 2)
            result.maximumDelta = std::max(result.maximumDelta,
                std::abs(value - static_cast<double>(result.audio[index - 2])));
    }
    result.rms = std::sqrt(squared / static_cast<double>(result.audio.size()));
    return result;
}

double differenceRms(const std::vector<float>& first,
                     const std::vector<float>& second)
{
    double squared = 0.0;
    for (std::size_t index = 0; index < first.size(); ++index)
    {
        const auto difference = static_cast<double>(first[index]) - second[index];
        squared += difference * difference;
    }
    return std::sqrt(squared / static_cast<double>(first.size()));
}

bool identical(const std::vector<float>& first, const std::vector<float>& second)
{
    if (first.size() != second.size()) return false;
    for (std::size_t index = 0; index < first.size(); ++index)
        if (std::bit_cast<std::uint32_t>(first[index])
            != std::bit_cast<std::uint32_t>(second[index])) return false;
    return true;
}
}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cerr << "usage: aquorbium_form_memory_render NEW_OUTPUT_DIRECTORY\n";
        return EXIT_FAILURE;
    }
    const std::filesystem::path directory(argv[1]);
    std::error_code error;
    if (std::filesystem::exists(directory, error) || error
        || !std::filesystem::create_directories(directory, error) || error)
    {
        std::cerr << "output directory must be new\n";
        return EXIT_FAILURE;
    }
    const std::array<Scenario, 4> scenarios {{
        { "bypass", { 0.0f, 1.0f, 1.0f, 1.0f } },
        { "ecological", { 1.0f, 1.0f, 0.15f, 0.05f } },
        { "persistent", { 1.0f, 0.2f, 1.0f, 0.0f } },
        { "volatile", { 1.0f, 0.8f, 0.0f, 1.0f } }
    }};
    std::array<Result, 4> results;
    for (std::size_t index = 0; index < scenarios.size(); ++index)
    {
        results[index] = render(scenarios[index]);
        if (results[index].rms <= 0.0 || results[index].peak >= 0.95
            || results[index].telemetry.formTransitionCount == 0
            || !writeWav(directory / (std::string(scenarios[index].name) + ".wav"),
                         results[index].audio)) return EXIT_FAILURE;
    }
    const auto repeat = render(scenarios[0]);
    const auto repeatIdentical = identical(results[0].audio, repeat.audio);
    std::ofstream report(directory / "REPORT.txt");
    report << "Aquorbium form-memory render (technical evidence, not listening approval)\n"
              "sample_rate=48000\nduration_seconds=20\nseed=0x815ab1e\n"
           << "bypass_repeat_bit_identical=" << (repeatIdentical ? 1 : 0) << "\n\n"
           << std::setprecision(10);
    auto differs = true;
    for (std::size_t index = 0; index < scenarios.size(); ++index)
    {
        const auto difference = differenceRms(results[0].audio, results[index].audio);
        differs = differs && (index == 0 || difference > 1.0e-7);
        const auto& performance = scenarios[index].performance;
        const auto& result = results[index];
        report << '[' << scenarios[index].name << "]\n"
               << "influence=" << performance.influence << '\n'
               << "context=" << performance.context << '\n'
               << "inertia=" << performance.inertia << '\n'
               << "mutation=" << performance.mutation << '\n'
               << "final_state=" << result.telemetry.formState << '\n'
               << "transitions=" << result.telemetry.formTransitionCount << '\n'
               << "context_target=" << result.telemetry.formContextTarget << '\n'
               << "rms=" << result.rms << '\n' << "peak=" << result.peak << '\n'
               << "maximum_adjacent_sample_delta=" << result.maximumDelta << '\n'
               << "render_ratio=" << result.renderRatio << '\n'
               << "difference_rms_vs_bypass=" << difference << "\n\n";
    }
    if (!report || !repeatIdentical || !differs) return EXIT_FAILURE;
    std::cout << (directory / "REPORT.txt") << '\n';
    return EXIT_SUCCESS;
}
