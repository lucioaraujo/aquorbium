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
constexpr double durationSeconds = 12.0;
constexpr std::size_t frames = static_cast<std::size_t>(sampleRate * durationSeconds);
constexpr std::size_t blockSize = 240;

struct Scenario
{
    const char* name;
    aquorbium::BiomaEngine::SyntheticTailPerformance performance;
};

struct Measurement
{
    double rms = 0.0;
    double peak = 0.0;
    double maximumDelta = 0.0;
    double renderRatio = 0.0;
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

std::pair<std::vector<float>, double> render(const Scenario& scenario)
{
    auto engine = std::make_unique<aquorbium::BiomaEngine>();
    engine->prepare(sampleRate);
    engine->setSeed(0x815ab1eu);
    engine->setDensity(0.62f);
    engine->setDrift(0.38f);
    engine->setCohesion(0.58f);
    engine->setDanger(0.42f);
    engine->setEcosystemBlendImmediately(1.0f);
    engine->setAutonomousEnabled(true);
    engine->setSyntheticTailPerformance(scenario.performance);
    std::vector<float> result(frames * 2);
    std::array<float, blockSize> left {}, right {};
    const auto started = std::chrono::steady_clock::now();
    for (std::size_t offset = 0; offset < frames; offset += blockSize)
    {
        const auto block = offset / blockSize;
        if (block % 400 == 0)
            engine->noteOn(43 + static_cast<int>((block / 400) % 5) * 5, 0.62f);
        if (block % 400 == 260)
            for (int note = 43; note <= 63; note += 5) engine->noteOff(note);
        const auto count = std::min(blockSize, frames - offset);
        engine->render(left.data(), right.data(), count);
        for (std::size_t sample = 0; sample < count; ++sample)
        {
            result[(offset + sample) * 2] = left[sample];
            result[(offset + sample) * 2 + 1] = right[sample];
        }
    }
    const auto elapsed = std::chrono::duration<double>(
        std::chrono::steady_clock::now() - started).count();
    return { std::move(result), elapsed / durationSeconds };
}

Measurement measure(const std::vector<float>& audio, double ratio)
{
    Measurement result;
    result.renderRatio = ratio;
    double squared = 0.0;
    for (std::size_t index = 0; index < audio.size(); ++index)
    {
        const auto value = static_cast<double>(audio[index]);
        if (!std::isfinite(value)) return { -1.0, 2.0, 2.0, ratio };
        squared += value * value;
        result.peak = std::max(result.peak, std::abs(value));
        if (index >= 2)
            result.maximumDelta = std::max(result.maximumDelta,
                std::abs(value - static_cast<double>(audio[index - 2])));
    }
    result.rms = std::sqrt(squared / static_cast<double>(audio.size()));
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
        std::cerr << "usage: aquorbium_synthetic_tail_render NEW_OUTPUT_DIRECTORY\n";
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
        { "bypass", { 0.0f, 0.45f, 0.55f, 0.0f } },
        { "short_dark", { 1.0f, 0.15f, 0.90f, 0.0f } },
        { "long_clear", { 1.0f, 0.90f, 0.10f, 0.0f } },
        { "nightmare", { 1.0f, 0.70f, 0.35f, 1.0f } }
    }};
    std::array<std::vector<float>, 4> audio;
    std::array<Measurement, 4> measurements;
    for (std::size_t index = 0; index < scenarios.size(); ++index)
    {
        auto [rendered, ratio] = render(scenarios[index]);
        audio[index] = std::move(rendered);
        measurements[index] = measure(audio[index], ratio);
        if (measurements[index].rms < 0.0 || measurements[index].peak >= 0.95
            || !writeWav(directory / (std::string(scenarios[index].name) + ".wav"),
                         audio[index])) return EXIT_FAILURE;
    }
    const auto [repeat, repeatRatio] = render(scenarios[0]);
    const auto repeatIdentical = identical(audio[0], repeat);
    std::ofstream report(directory / "REPORT.txt");
    report << "Aquorbium synthetic tail render (technical evidence, not listening approval)\n"
              "sample_rate=48000\nduration_seconds=12\nseed=0x815ab1e\n"
           << "bypass_repeat_bit_identical=" << (repeatIdentical ? 1 : 0) << '\n'
           << "bypass_repeat_render_ratio=" << repeatRatio << "\n\n"
           << std::setprecision(10);
    bool differs = true;
    for (std::size_t index = 0; index < scenarios.size(); ++index)
    {
        const auto& value = measurements[index];
        const auto difference = differenceRms(audio[0], audio[index]);
        differs = differs && (index == 0 || difference > 1.0e-7);
        report << '[' << scenarios[index].name << "]\n"
               << "influence=" << scenarios[index].performance.influence << '\n'
               << "decay=" << scenarios[index].performance.decay << '\n'
               << "damping=" << scenarios[index].performance.damping << '\n'
               << "nightmare=" << scenarios[index].performance.nightmare << '\n'
               << "rms=" << value.rms << '\n' << "peak=" << value.peak << '\n'
               << "maximum_adjacent_sample_delta=" << value.maximumDelta << '\n'
               << "render_ratio=" << value.renderRatio << '\n'
               << "difference_rms_vs_bypass=" << difference << "\n\n";
    }
    if (!report || !repeatIdentical || !differs) return EXIT_FAILURE;
    std::cout << (directory / "REPORT.txt") << '\n';
    return EXIT_SUCCESS;
}
