// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/BiomaPhysicalString.h"

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
constexpr double durationSeconds = 12.0;
constexpr std::size_t sampleCount = static_cast<std::size_t>(
    sampleRate * durationSeconds);

struct Character
{
    std::string_view name;
    float decaySeconds;
    float brightness;
    float hardness;
    float pluckPosition;
};

constexpr std::array<Character, 3> characters {{
    { "filament", 2.8f, 0.34f, 0.20f, 0.14f },
    { "shell",    1.4f, 0.57f, 0.63f, 0.29f },
    { "blade",    5.8f, 0.96f, 0.94f, 0.46f }
}};

constexpr std::array<double, 8> eventSeconds {
    0.16, 1.23, 2.71, 4.04, 5.82, 7.36, 9.18, 10.42
};
constexpr std::array<float, 8> frequencies {
    82.41f, 123.47f, 98.00f, 164.81f, 73.42f, 146.83f, 110.00f, 196.00f
};
constexpr std::array<float, 8> strengths {
    0.82f, 0.61f, 0.74f, 0.56f, 0.90f, 0.66f, 0.78f, 0.58f
};

struct Measurements
{
    double rms = 0.0;
    double peak = 0.0;
    double dc = 0.0;
    double maximumDelta = 0.0;
    double highFrequencyRatio = 0.0;
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
    const auto dataBytes = static_cast<std::uint32_t>(
        samples.size() * sizeof(float) * 2);
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
    double differenceSquared = 0.0;
    for (std::size_t index = 0; index < samples.size(); ++index)
    {
        const auto value = static_cast<double>(samples[index]);
        squared += value * value;
        result.peak = std::max(result.peak, std::abs(value));
        result.dc += value;
        if (index > 0)
        {
            const auto delta = value - static_cast<double>(samples[index - 1]);
            differenceSquared += delta * delta;
            result.maximumDelta = std::max(result.maximumDelta, std::abs(delta));
        }
    }
    result.rms = std::sqrt(squared / static_cast<double>(samples.size()));
    result.dc /= static_cast<double>(samples.size());
    result.highFrequencyRatio = squared > 0.0 ? differenceSquared / squared : 0.0;
    return result;
}

bool renderCharacter(const std::filesystem::path& directory,
                     const Character& character)
{
    aquorbium::BiomaPhysicalString string;
    string.prepare(sampleRate);
    string.setSeed(0x6a291u);

    std::vector<float> samples(sampleCount);
    std::size_t event = 0;
    for (std::size_t sample = 0; sample < samples.size(); ++sample)
    {
        if (event < eventSeconds.size()
            && sample == static_cast<std::size_t>(eventSeconds[event] * sampleRate))
        {
            string.setParameters({ frequencies[event], character.decaySeconds,
                                   character.brightness, character.hardness,
                                   character.pluckPosition });
            string.excite(strengths[event]);
            ++event;
        }
        samples[sample] = string.tick();
    }

    const auto measurements = measure(samples);
    if (!std::isfinite(measurements.rms) || measurements.rms <= 0.0
        || !std::isfinite(measurements.peak) || measurements.peak >= 1.0
        || std::abs(measurements.dc) >= 0.01)
        return false;

    const auto stem = std::string("AQUORBIUM_STRING_")
        + std::string(character.name);
    if (!writeFloatWav(directory / (stem + ".wav"), samples))
        return false;
    std::ofstream report(directory / (stem + ".txt"));
    if (!report) return false;
    report << "Aquorbium Physical String character\n"
           << "name=" << character.name << '\n'
           << "sample_rate=48000\n"
           << "duration_seconds=12\n"
           << "events=8\n"
           << "seed=0x6a291\n"
           << "decay_seconds=" << character.decaySeconds << '\n'
           << "brightness=" << character.brightness << '\n'
           << "hardness=" << character.hardness << '\n'
           << "pluck_position=" << character.pluckPosition << '\n'
           << "rms=" << measurements.rms << '\n'
           << "peak=" << measurements.peak << '\n'
           << "dc=" << measurements.dc << '\n'
           << "maximum_adjacent_sample_delta=" << measurements.maximumDelta << '\n'
           << "high_frequency_ratio=" << measurements.highFrequencyRatio << '\n';
    return static_cast<bool>(report);
}
}

int main(int argc, char** argv)
{
    const auto directory = argc > 1
        ? std::filesystem::path(argv[1])
        : std::filesystem::path(
            "validation-output/aquorbium-string-characters-refined");
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
        std::cout << "wrote "
                  << directory / (std::string("AQUORBIUM_STRING_")
                      + std::string(character.name) + ".wav") << '\n';
    }
}
