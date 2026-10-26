// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/BiomaEngine.h"

#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string_view>
#include <vector>

namespace
{
constexpr double sampleRate = 48000.0;
constexpr double durationSeconds = 12.0;
constexpr std::size_t frameCount = static_cast<std::size_t>(
    sampleRate * durationSeconds);
constexpr std::size_t blockSize = 240;
constexpr std::array<float, 7> bandEdgesHz {
    125.0f, 250.0f, 500.0f, 1000.0f, 2000.0f, 4000.0f, 8000.0f
};

struct Scenario
{
    const char* name;
    aquorbium::BiomaEngine::SpectralEnvironmentPerformance performance;
};

struct Measurements
{
    double rms = 0.0;
    double peak = 0.0;
    double dcLeft = 0.0;
    double dcRight = 0.0;
    double maximumDelta = 0.0;
    std::array<double, 8> bandRms {};
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
    writeU16(stream, 3u); // IEEE float
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

std::vector<float> render(const Scenario& scenario)
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
    engine->setSpectralEnvironmentPerformance(scenario.performance);

    std::vector<float> result(frameCount * 2);
    std::array<float, blockSize> left {}, right {};
    for (std::size_t offset = 0; offset < frameCount; offset += blockSize)
    {
        const auto block = offset / blockSize;
        if (block % 400 == 0)
            engine->noteOn(43 + static_cast<int>((block / 400) % 5) * 5, 0.62f);
        if (block % 400 == 260)
            for (int note = 43; note <= 63; note += 5)
                engine->noteOff(note);
        const auto count = std::min(blockSize, frameCount - offset);
        engine->render(left.data(), right.data(), count);
        for (std::size_t sample = 0; sample < count; ++sample)
        {
            result[(offset + sample) * 2] = left[sample];
            result[(offset + sample) * 2 + 1] = right[sample];
        }
    }
    return result;
}

Measurements measure(const std::vector<float>& samples)
{
    Measurements result;
    std::array<double, bandEdgesHz.size()> lowpass {};
    std::array<double, bandEdgesHz.size()> coefficients {};
    for (std::size_t index = 0; index < coefficients.size(); ++index)
        coefficients[index] = 1.0 - std::exp(
            -2.0 * 3.14159265358979323846
            * static_cast<double>(bandEdgesHz[index]) / sampleRate);

    double squared = 0.0;
    std::array<double, 8> bandSquared {};
    auto previousLeft = 0.0;
    auto previousRight = 0.0;
    for (std::size_t frame = 0; frame < samples.size() / 2; ++frame)
    {
        const auto left = static_cast<double>(samples[frame * 2]);
        const auto right = static_cast<double>(samples[frame * 2 + 1]);
        const auto mono = (left + right) * 0.5;
        squared += left * left + right * right;
        result.peak = std::max({ result.peak, std::abs(left), std::abs(right) });
        result.dcLeft += left;
        result.dcRight += right;
        if (frame > 0)
            result.maximumDelta = std::max({ result.maximumDelta,
                std::abs(left - previousLeft), std::abs(right - previousRight) });
        previousLeft = left;
        previousRight = right;

        for (std::size_t band = 0; band < lowpass.size(); ++band)
            lowpass[band] += (mono - lowpass[band]) * coefficients[band];
        auto lower = 0.0;
        for (std::size_t band = 0; band < lowpass.size(); ++band)
        {
            const auto value = lowpass[band] - lower;
            bandSquared[band] += value * value;
            lower = lowpass[band];
        }
        const auto high = mono - lowpass.back();
        bandSquared.back() += high * high;
    }
    const auto frames = static_cast<double>(samples.size() / 2);
    result.rms = std::sqrt(squared / static_cast<double>(samples.size()));
    result.dcLeft /= frames;
    result.dcRight /= frames;
    for (std::size_t band = 0; band < result.bandRms.size(); ++band)
        result.bandRms[band] = std::sqrt(bandSquared[band] / frames);
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

bool identical(const std::vector<float>& first,
               const std::vector<float>& second)
{
    if (first.size() != second.size()) return false;
    for (std::size_t index = 0; index < first.size(); ++index)
        if (std::bit_cast<std::uint32_t>(first[index])
            != std::bit_cast<std::uint32_t>(second[index]))
            return false;
    return true;
}

bool writeReport(const std::filesystem::path& path,
                 const std::array<Scenario, 4>& scenarios,
                 const std::array<Measurements, 4>& measurements,
                 const std::array<double, 4>& differences,
                 bool baselineRepeatIdentical)
{
    std::ofstream report(path);
    if (!report) return false;
    report << "Aquorbium spectral environment render (technical evidence, not listening approval)\n"
              "sample_rate=48000\n"
              "duration_seconds=12\n"
              "seed=0x815ab1e\n"
              "band_edges_hz=125,250,500,1000,2000,4000,8000\n"
              "baseline_repeat_bit_identical=" << (baselineRepeatIdentical ? 1 : 0) << "\n\n";
    report << std::setprecision(10);
    for (std::size_t index = 0; index < scenarios.size(); ++index)
    {
        const auto& scenario = scenarios[index];
        const auto& value = measurements[index];
        report << '[' << scenario.name << "]\n"
               << "influence=" << scenario.performance.influence << '\n'
               << "filter_mix=" << scenario.performance.filterMix << '\n'
               << "centre=" << scenario.performance.centre << '\n'
               << "span=" << scenario.performance.span << '\n'
               << "resonance=" << scenario.performance.resonance << '\n'
               << "ambience=" << scenario.performance.ambience << '\n'
               << "rms=" << value.rms << '\n'
               << "peak=" << value.peak << '\n'
               << "dc_left=" << value.dcLeft << '\n'
               << "dc_right=" << value.dcRight << '\n'
               << "maximum_adjacent_sample_delta=" << value.maximumDelta << '\n'
               << "difference_rms_vs_bypass=" << differences[index] << '\n'
               << "band_rms=";
        for (std::size_t band = 0; band < value.bandRms.size(); ++band)
            report << (band == 0 ? "" : ",") << value.bandRms[band];
        report << "\n\n";
    }
    return static_cast<bool>(report);
}
}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cerr << "usage: aquorbium_spectral_environment_render NEW_OUTPUT_DIRECTORY\n";
        return EXIT_FAILURE;
    }
    const std::filesystem::path directory(argv[1]);
    std::error_code error;
    if (std::filesystem::exists(directory, error) || error
        || !std::filesystem::create_directories(directory, error) || error)
    {
        std::cerr << "output directory must not already exist\n";
        return EXIT_FAILURE;
    }

    constexpr std::array scenarios {
        Scenario { "bypass", { 0.0f, 0.5f, 0.5f, 0.5f, 0.45f, 0.0f } },
        Scenario { "filter", { 1.0f, 1.0f, 0.30f, 0.75f, 0.65f, 0.0f } },
        Scenario { "ambience", { 1.0f, 0.0f, 0.50f, 0.50f, 0.45f, 1.0f } },
        Scenario { "combined", { 0.75f, 0.70f, 0.62f, 0.60f, 0.55f, 0.45f } }
    };
    std::array<Measurements, scenarios.size()> measurements {};
    std::array<double, scenarios.size()> differences {};
    const auto baseline = render(scenarios[0]);
    const auto repeatedBaseline = render(scenarios[0]);
    const auto baselineRepeatIdentical = identical(baseline, repeatedBaseline);
    measurements[0] = measure(baseline);
    if (!baselineRepeatIdentical
        || !writeFloatWav(directory / "00_BYPASS.wav", baseline))
        return EXIT_FAILURE;

    for (std::size_t index = 1; index < scenarios.size(); ++index)
    {
        const auto samples = render(scenarios[index]);
        measurements[index] = measure(samples);
        differences[index] = differenceRms(baseline, samples);
        if (!std::isfinite(measurements[index].rms)
            || measurements[index].peak >= 0.95
            || differences[index] <= 1.0e-7
            || !writeFloatWav(directory / (std::to_string(index)
                + "_" + scenarios[index].name + ".wav"), samples))
            return EXIT_FAILURE;
    }
    if (!writeReport(directory / "REPORT.txt", scenarios, measurements,
                     differences, baselineRepeatIdentical))
        return EXIT_FAILURE;
    std::cout << "wrote spectral environment evidence to " << directory << '\n';
}
