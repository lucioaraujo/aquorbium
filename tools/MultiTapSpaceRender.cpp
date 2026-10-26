// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/MultiTapSpace.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string_view>
#include <vector>

namespace
{
constexpr double sampleRate = 48000.0;
constexpr float pi = 3.14159265358979323846f;
constexpr float auditionGain = 2.5f;

struct Measurements
{
    double rms = 0.0;
    double peak = 0.0;
    double dcLeft = 0.0;
    double dcRight = 0.0;
    double maximumDelta = 0.0;
    double sideRms = 0.0;
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
                   const std::vector<float>& interleaved)
{
    const auto dataBytes = static_cast<std::uint32_t>(
        interleaved.size() * sizeof(float));
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
    stream.write(reinterpret_cast<const char*>(interleaved.data()),
                 static_cast<std::streamsize>(dataBytes));
    return static_cast<bool>(stream);
}

Measurements measure(const std::vector<float>& samples)
{
    Measurements result;
    double squared = 0.0;
    double sideSquared = 0.0;
    float previousLeft = 0.0f;
    float previousRight = 0.0f;
    const auto frameCount = samples.size() / 2;
    for (std::size_t frame = 0; frame < frameCount; ++frame)
    {
        const auto left = samples[frame * 2];
        const auto right = samples[frame * 2 + 1];
        squared += static_cast<double>(left) * left
            + static_cast<double>(right) * right;
        const auto side = static_cast<double>(left - right);
        sideSquared += side * side;
        result.peak = std::max({ result.peak, std::abs(static_cast<double>(left)),
                                 std::abs(static_cast<double>(right)) });
        result.dcLeft += left;
        result.dcRight += right;
        if (frame > 0)
            result.maximumDelta = std::max(
                result.maximumDelta,
                std::max(std::abs(static_cast<double>(left - previousLeft)),
                         std::abs(static_cast<double>(right - previousRight))));
        previousLeft = left;
        previousRight = right;
    }
    result.rms = std::sqrt(squared / static_cast<double>(samples.size()));
    result.sideRms = std::sqrt(sideSquared / static_cast<double>(frameCount));
    result.dcLeft /= static_cast<double>(frameCount);
    result.dcRight /= static_cast<double>(frameCount);
    return result;
}

bool writeReport(const std::filesystem::path& path, std::string_view name,
                 double duration, const Measurements& value)
{
    std::ofstream report(path);
    if (!report) return false;
    report << "Aquorbium MultiTapSpace reproducible render\n"
           << "name=" << name << '\n'
           << "sample_rate=48000\n"
           << "duration_seconds=" << duration << '\n'
           << "taps=8\n"
           << "interpolation=third_order_lagrange\n"
           << "feedback=0.31\n"
           << "audition_gain=" << auditionGain << '\n'
           << "rms=" << value.rms << '\n'
           << "peak=" << value.peak << '\n'
           << "dc_left=" << value.dcLeft << '\n'
           << "dc_right=" << value.dcRight << '\n'
           << "maximum_adjacent_sample_delta=" << value.maximumDelta << '\n'
           << "side_rms=" << value.sideRms << '\n';
    return static_cast<bool>(report);
}

bool renderImpulse(const std::filesystem::path& directory)
{
    constexpr double duration = 6.0;
    const auto frames = static_cast<std::size_t>(duration * sampleRate);
    std::vector<float> samples(frames * 2);
    aquorbium::MultiTapSpace space;
    space.prepare(sampleRate);
    space.setParameters(aquorbium::MultiTapSpace::aquariumEarlyReflections());
    for (std::size_t frame = 0; frame < frames; ++frame)
    {
        const auto impulse = frame == static_cast<std::size_t>(sampleRate * 0.25)
            ? 0.75f : 0.0f;
        const auto output = space.tick(impulse, impulse);
        samples[frame * 2] = output.left * auditionGain;
        samples[frame * 2 + 1] = output.right * auditionGain;
    }
    const auto metrics = measure(samples);
    return metrics.peak > 0.0 && metrics.peak < 1.0
        && writeFloatWav(directory / "AQUORBIUM_MULTITAP_IMPULSE.wav", samples)
        && writeReport(directory / "AQUORBIUM_MULTITAP_IMPULSE.txt",
                       "wet_impulse_response", duration, metrics);
}

bool renderProbe(const std::filesystem::path& directory)
{
    constexpr double duration = 16.0;
    constexpr std::array<double, 10> events {
        0.30, 1.41, 2.76, 4.12, 5.83, 7.24, 9.10, 11.02, 13.14, 14.62
    };
    constexpr std::array<float, 10> frequencies {
        73.42f, 110.0f, 146.83f, 92.50f, 185.0f,
        82.41f, 123.47f, 164.81f, 98.0f, 196.0f
    };
    const auto frames = static_cast<std::size_t>(duration * sampleRate);
    std::vector<float> samples(frames * 2);
    aquorbium::MultiTapSpace space;
    space.prepare(sampleRate);
    space.setParameters(aquorbium::MultiTapSpace::aquariumEarlyReflections());

    std::size_t eventIndex = 0;
    double phase = 0.0;
    float envelope = 0.0f;
    float frequency = frequencies[0];
    for (std::size_t frame = 0; frame < frames; ++frame)
    {
        if (eventIndex < events.size()
            && frame == static_cast<std::size_t>(events[eventIndex] * sampleRate))
        {
            frequency = frequencies[eventIndex];
            envelope = 1.0f;
            ++eventIndex;
        }
        if (frame % 240 == 0)
        {
            const auto seconds = static_cast<float>(frame / sampleRate);
            const auto journey = std::clamp((seconds - 6.0f) / 8.0f, 0.0f, 1.0f);
            space.setPerformanceIntent(
                { journey * 0.72f, 0.5f + journey * 0.38f,
                  1.0f - journey * 0.44f, 0.5f - journey * 0.22f,
                  0.35f + journey * 0.48f });
        }

        phase += 2.0 * static_cast<double>(pi) * frequency / sampleRate;
        if (phase >= 2.0 * static_cast<double>(pi))
            phase -= 2.0 * static_cast<double>(pi);
        const auto body = static_cast<float>(
            std::sin(phase) * 0.72 + std::sin(phase * 2.013) * 0.20
            + std::sin(phase * 3.941) * 0.08);
        const auto input = body * envelope * 0.34f;
        envelope *= 0.99982f;
        if (envelope < 1.0e-8f)
            envelope = 0.0f;

        const auto slowPan = std::sin(static_cast<float>(frame) * 0.000019f) * 0.28f;
        const auto inputLeft = input * (0.72f - slowPan);
        const auto inputRight = input * (0.72f + slowPan);
        const auto wet = space.tick(inputLeft, inputRight);
        samples[frame * 2] = (inputLeft * 0.58f + wet.left * 1.10f)
            * auditionGain;
        samples[frame * 2 + 1] = (inputRight * 0.58f + wet.right * 1.10f)
            * auditionGain;
    }

    const auto metrics = measure(samples);
    return std::isfinite(metrics.rms) && metrics.rms > 0.0 && metrics.peak < 1.0
        && writeFloatWav(directory / "AQUORBIUM_MULTITAP_MUSICAL_PROBE.wav", samples)
        && writeReport(directory / "AQUORBIUM_MULTITAP_MUSICAL_PROBE.txt",
                       "dry_wet_musical_probe", duration, metrics);
}
}

int main(int argc, char** argv)
{
    const auto directory = argc > 1
        ? std::filesystem::path(argv[1])
        : std::filesystem::path("validation-output/aquorbium-multitap-space");
    std::error_code error;
    std::filesystem::create_directories(directory, error);
    if (error || !renderImpulse(directory) || !renderProbe(directory))
    {
        std::cerr << "could not render MultiTapSpace evidence\n";
        return EXIT_FAILURE;
    }
    std::cout << "wrote " << directory / "AQUORBIUM_MULTITAP_IMPULSE.wav" << '\n'
              << "wrote " << directory / "AQUORBIUM_MULTITAP_MUSICAL_PROBE.wav"
              << '\n';
}
