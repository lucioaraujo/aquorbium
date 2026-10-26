// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/BiomaGranularStriker.h"
#include "core/GranularPerformance.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

namespace
{
constexpr double sampleRate = 48000.0;
constexpr double durationSeconds = 10.0;
constexpr std::size_t sampleCount = static_cast<std::size_t>(sampleRate * durationSeconds);

struct Character
{
    std::string_view name;
    aquorbium::GranularPerformance::Macros macros;
    std::uint32_t seed;
};

constexpr std::array<Character, 3> characters {{
    { "dust",  { 1.0f, 0.05f, 0.22f, 0.85f }, 0x91c31u },
    { "swarm", { 1.0f, 0.45f, 0.90f, 0.62f }, 0x91c31u },
    { "metal", { 1.0f, 0.92f, 0.55f, 0.18f }, 0x91c31u }
}};

constexpr std::array<double, 9> eventSeconds {
    0.20, 0.73, 1.41, 2.37, 3.05, 4.48, 5.02, 7.16, 8.62
};
constexpr std::array<float, 9> frequencies {
    220.0f, 329.63f, 277.18f, 440.0f, 185.0f, 554.37f, 246.94f, 369.99f, 293.66f
};
constexpr std::array<float, 9> strengths {
    0.72f, 0.58f, 0.85f, 0.64f, 0.92f, 0.55f, 0.78f, 0.68f, 0.82f
};

struct Measurements
{
    double rms = 0.0;
    double peak = 0.0;
    double dc = 0.0;
    double maximumDelta = 0.0;
};

bool writeU16(std::ofstream& stream, std::uint16_t value)
{
    stream.put(static_cast<char>(value & 0xffu));
    stream.put(static_cast<char>((value >> 8u) & 0xffu));
    return static_cast<bool>(stream);
}

bool writeU32(std::ofstream& stream, std::uint32_t value)
{
    for (unsigned shift = 0; shift < 32; shift += 8)
        stream.put(static_cast<char>((value >> shift) & 0xffu));
    return static_cast<bool>(stream);
}

bool writeFloatWav(const std::filesystem::path& path,
                   const std::vector<float>& samples)
{
    const auto dataBytes = static_cast<std::uint32_t>(samples.size() * sizeof(float) * 2);
    std::ofstream stream(path, std::ios::binary);
    if (!stream) return false;
    stream.write("RIFF", 4);
    writeU32(stream, 36u + dataBytes);
    stream.write("WAVEfmt ", 8);
    writeU32(stream, 16u);
    writeU16(stream, 3u);
    writeU16(stream, 2u);
    writeU32(stream, static_cast<std::uint32_t>(sampleRate));
    writeU32(stream, static_cast<std::uint32_t>(sampleRate * sizeof(float) * 2));
    writeU16(stream, static_cast<std::uint16_t>(sizeof(float) * 2));
    writeU16(stream, 32u);
    stream.write("data", 4);
    writeU32(stream, dataBytes);
    for (const auto sample : samples)
    {
        stream.write(reinterpret_cast<const char*>(&sample), sizeof(sample));
        stream.write(reinterpret_cast<const char*>(&sample), sizeof(sample));
    }
    return static_cast<bool>(stream);
}

Measurements measure(const std::vector<float>& samples)
{
    Measurements result;
    double squared = 0.0;
    for (std::size_t index = 0; index < samples.size(); ++index)
    {
        const auto value = static_cast<double>(samples[index]);
        squared += value * value;
        result.peak = std::max(result.peak, std::abs(value));
        result.dc += value;
        if (index > 0)
            result.maximumDelta = std::max(
                result.maximumDelta,
                std::abs(value - static_cast<double>(samples[index - 1])));
    }
    result.rms = std::sqrt(squared / static_cast<double>(samples.size()));
    result.dc /= static_cast<double>(samples.size());
    return result;
}

bool renderCharacter(const std::filesystem::path& directory,
                     const Character& character)
{
    aquorbium::BiomaGranularStriker striker;
    striker.prepare(sampleRate);
    striker.setSeed(character.seed);
    striker.setPerformanceIntent(
        aquorbium::GranularPerformance::map(character.macros));

    std::vector<float> samples(sampleCount);
    std::size_t event = 0;
    for (std::size_t sample = 0; sample < samples.size(); ++sample)
    {
        if (event < eventSeconds.size()
            && sample == static_cast<std::size_t>(eventSeconds[event] * sampleRate))
        {
            aquorbium::BiomaGranularStriker::Parameters generative;
            generative.centerHz = frequencies[event];
            striker.setParameters(generative);
            striker.excite(strengths[event]);
            ++event;
        }
        samples[sample] = striker.tick();
    }

    const auto measurements = measure(samples);
    if (!std::isfinite(measurements.rms) || measurements.rms <= 0.0
        || !std::isfinite(measurements.peak) || measurements.peak >= 1.0
        || std::abs(measurements.dc) >= 0.01)
        return false;

    const auto stem = std::string("AQUORBIUM_STRIKER_")
        + std::string(character.name);
    if (!writeFloatWav(directory / (stem + ".wav"), samples))
        return false;
    std::ofstream report(directory / (stem + ".txt"));
    if (!report) return false;
    const auto mapped = aquorbium::GranularPerformance::map(character.macros);
    report << "Aquorbium Granular Striker character\n"
           << "name=" << character.name << '\n'
           << "sample_rate=48000\n"
           << "duration_seconds=10\n"
           << "events=9\n"
           << "seed=0x91c31\n"
           << "influence=" << character.macros.influence << '\n'
           << "matter=" << character.macros.matter << '\n'
           << "population=" << character.macros.population << '\n'
           << "scatter=" << character.macros.scatter << '\n'
           << "mapped_duration_ms=" << mapped.grainDurationMs << '\n'
           << "mapped_resonance=" << mapped.resonance << '\n'
           << "rms=" << measurements.rms << '\n'
           << "peak=" << measurements.peak << '\n'
           << "dc=" << measurements.dc << '\n'
           << "maximum_adjacent_sample_delta=" << measurements.maximumDelta << '\n';
    return static_cast<bool>(report);
}
}

int main(int argc, char** argv)
{
    const auto directory = argc > 1
        ? std::filesystem::path(argv[1])
        : std::filesystem::path("validation-output/aquorbium-striker-characters");
    std::error_code error;
    std::filesystem::create_directories(directory, error);
    if (error)
    {
        std::cerr << "could not create output directory: " << error.message() << '\n';
        return EXIT_FAILURE;
    }
    for (const auto& character : characters)
    {
        if (!renderCharacter(directory, character))
        {
            std::cerr << "could not render " << character.name << '\n';
            return EXIT_FAILURE;
        }
        std::cout << "wrote " << directory / (std::string("AQUORBIUM_STRIKER_")
            + std::string(character.name) + ".wav") << '\n';
    }
}
