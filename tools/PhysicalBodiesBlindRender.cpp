// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/BiomaGranularStriker.h"
#include "core/BiomaModalResonator.h"
#include "core/BiomaPhysicalString.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

namespace
{
constexpr double sampleRate = 48000.0;
constexpr double durationSeconds = 18.0;
constexpr std::size_t sampleCount = static_cast<std::size_t>(
    sampleRate * durationSeconds);
// Chosen from the highest crest-factor body so every take reaches exactly the
// same RMS without touching the peak ceiling or requiring a limiter.
constexpr float targetRms = 0.016f;
constexpr float peakCeiling = 0.70f;
constexpr std::uint32_t sharedSeed = 0x7b412u;

enum class Body : std::uint8_t
{
    PhysicalString,
    ModalResonator,
    GranularStriker
};

// The public files expose only A/B/C. The key is printed deliberately through
// --reveal after the listener has formed an impression.
constexpr std::array<Body, 3> blindOrder {
    Body::ModalResonator,
    Body::PhysicalString,
    Body::GranularStriker
};
constexpr std::array<std::string_view, 3> takeNames { "A", "B", "C" };

constexpr std::array<double, 10> eventSeconds {
    0.35, 1.62, 3.08, 4.91, 6.10, 7.84, 9.73, 11.21, 13.46, 15.37
};
constexpr std::array<float, 10> frequencies {
    82.41f, 123.47f, 98.00f, 164.81f, 73.42f,
    146.83f, 110.00f, 196.00f, 92.50f, 174.61f
};
constexpr std::array<float, 10> strengths {
    0.82f, 0.61f, 0.74f, 0.56f, 0.90f,
    0.66f, 0.78f, 0.58f, 0.86f, 0.64f
};

struct Measurements
{
    double rms = 0.0;
    double peak = 0.0;
    double dc = 0.0;
    double maximumDelta = 0.0;
    double highFrequencyRatio = 0.0;
    double crestFactor = 0.0;
    double gainApplied = 1.0;
};

std::string_view bodyName(Body body) noexcept
{
    switch (body)
    {
        case Body::PhysicalString: return "Physical String";
        case Body::ModalResonator: return "Modal Resonator";
        case Body::GranularStriker: return "Granular Striker";
    }
    return "Unknown";
}

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
    result.crestFactor = result.rms > 0.0 ? result.peak / result.rms : 0.0;
    return result;
}

void applyEndFade(std::vector<float>& samples)
{
    const auto fadeSamples = static_cast<std::size_t>(sampleRate * 0.08);
    for (std::size_t offset = 0; offset < fadeSamples; ++offset)
    {
        const auto gain = 1.0f - static_cast<float>(offset)
            / static_cast<float>(fadeSamples - 1);
        samples[samples.size() - fadeSamples + offset] *= gain;
    }
}

Measurements normalize(std::vector<float>& samples)
{
    const auto before = measure(samples);
    if (before.rms <= 0.0 || before.peak <= 0.0)
        return before;
    const auto rmsGain = static_cast<double>(targetRms) / before.rms;
    const auto peakGain = static_cast<double>(peakCeiling) / before.peak;
    const auto gain = std::min(rmsGain, peakGain);
    for (auto& sample : samples)
        sample = static_cast<float>(static_cast<double>(sample) * gain);
    auto after = measure(samples);
    after.gainApplied = gain;
    return after;
}

std::vector<float> renderPhysicalString()
{
    aquorbium::BiomaPhysicalString body;
    body.prepare(sampleRate);
    body.setSeed(sharedSeed);
    std::vector<float> samples(sampleCount);
    std::size_t event = 0;
    for (std::size_t sample = 0; sample < samples.size(); ++sample)
    {
        if (event < eventSeconds.size()
            && sample == static_cast<std::size_t>(eventSeconds[event] * sampleRate))
        {
            body.setParameters({ frequencies[event], 2.4f, 0.66f, 0.58f, 0.27f });
            body.excite(strengths[event]);
            ++event;
        }
        samples[sample] = body.tick();
    }
    applyEndFade(samples);
    return samples;
}

std::vector<float> renderModalResonator()
{
    aquorbium::BiomaModalResonator body;
    body.prepare(sampleRate);
    body.setSeed(sharedSeed);
    std::vector<float> samples(sampleCount);
    std::size_t event = 0;
    for (std::size_t sample = 0; sample < samples.size(); ++sample)
    {
        if (event < eventSeconds.size()
            && sample == static_cast<std::size_t>(eventSeconds[event] * sampleRate))
        {
            body.setParameters({ frequencies[event], 0.34f, 2.4f,
                                 0.66f, 0.58f, 0.27f });
            body.excite(strengths[event]);
            ++event;
        }
        samples[sample] = body.tick();
    }
    applyEndFade(samples);
    return samples;
}

std::vector<float> renderGranularStriker()
{
    aquorbium::BiomaGranularStriker body;
    body.prepare(sampleRate);
    body.setSeed(sharedSeed);
    std::vector<float> samples(sampleCount);
    std::size_t event = 0;
    for (std::size_t sample = 0; sample < samples.size(); ++sample)
    {
        if (event < eventSeconds.size()
            && sample == static_cast<std::size_t>(eventSeconds[event] * sampleRate))
        {
            body.setParameters({ frequencies[event] * 5.0f, 18.0f, 0.58f,
                                 0.42f, 0.66f, 0.50f });
            body.excite(strengths[event]);
            ++event;
        }
        samples[sample] = body.tick();
    }
    applyEndFade(samples);
    return samples;
}

std::vector<float> renderBody(Body body)
{
    switch (body)
    {
        case Body::PhysicalString: return renderPhysicalString();
        case Body::ModalResonator: return renderModalResonator();
        case Body::GranularStriker: return renderGranularStriker();
    }
    return {};
}

double correlation(const std::vector<float>& left,
                   const std::vector<float>& right)
{
    double product = 0.0;
    double leftSquared = 0.0;
    double rightSquared = 0.0;
    for (std::size_t sample = 0; sample < left.size(); ++sample)
    {
        const auto a = static_cast<double>(left[sample]);
        const auto b = static_cast<double>(right[sample]);
        product += a * b;
        leftSquared += a * a;
        rightSquared += b * b;
    }
    const auto denominator = std::sqrt(leftSquared * rightSquared);
    return denominator > 0.0 ? product / denominator : 0.0;
}

bool writeListeningGuide(const std::filesystem::path& directory)
{
    std::ofstream guide(directory / "LISTEN.md");
    if (!guide) return false;
    guide << "# Aquorbium — comparação cega dos corpos físicos\n\n"
             "Ouça `A.wav`, `B.wav` e `C.wav` em qualquer ordem. Os três usam "
             "a mesma seed, dez eventos, relações de altura e intensidades. "
             "Cada motor traduz o pitch anchor segundo sua física. O RMS é "
             "igual e nenhum take toca o teto de pico.\n\n"
             "Não procure um vencedor. Observe somente:\n\n"
             "- que matéria e gesto cada take sugere;\n"
             "- se a energia parece partícula, corpo ou filamento;\n"
             "- qual função seria distinta dentro do aquário;\n"
             "- quais dois se sobrepõem musicalmente.\n\n"
             "A chave não é escrita nesta pasta. Quando a impressão estiver "
             "formada, execute a ferramenta com `--reveal`.\n";
    return static_cast<bool>(guide);
}
}

int main(int argc, char** argv)
{
    const auto reveal = argc > 1 && std::string_view(argv[1]) == "--reveal";
    if (reveal)
    {
        for (std::size_t take = 0; take < blindOrder.size(); ++take)
            std::cout << takeNames[take] << " = " << bodyName(blindOrder[take]) << '\n';
        return EXIT_SUCCESS;
    }

    const auto directory = argc > 1
        ? std::filesystem::path(argv[1])
        : std::filesystem::path(
            "validation-output/aquorbium-physical-bodies-refined");
    std::error_code error;
    std::filesystem::create_directories(directory, error);
    if (error)
    {
        std::cerr << "could not create output directory: " << error.message() << '\n';
        return EXIT_FAILURE;
    }

    std::array<std::vector<float>, blindOrder.size()> audio;
    std::array<Measurements, blindOrder.size()> measurements;
    for (std::size_t take = 0; take < blindOrder.size(); ++take)
    {
        audio[take] = renderBody(blindOrder[take]);
        measurements[take] = normalize(audio[take]);
        if (!std::isfinite(measurements[take].rms)
            || measurements[take].rms <= 0.0
            || measurements[take].peak > peakCeiling + 1.0e-5f
            || std::abs(measurements[take].dc) > 0.001)
            return EXIT_FAILURE;
        if (!writeFloatWav(directory / (std::string(takeNames[take]) + ".wav"),
                           audio[take]))
            return EXIT_FAILURE;
    }

    std::ofstream report(directory / "ANALYSIS.csv");
    if (!report) return EXIT_FAILURE;
    report << "take,rms,peak,dc,max_delta,hf_ratio,crest_factor,gain_applied\n";
    report << std::setprecision(9);
    for (std::size_t take = 0; take < blindOrder.size(); ++take)
    {
        const auto& value = measurements[take];
        report << takeNames[take] << ',' << value.rms << ',' << value.peak << ','
               << value.dc << ',' << value.maximumDelta << ','
               << value.highFrequencyRatio << ',' << value.crestFactor << ','
               << value.gainApplied << '\n';
    }
    report << "correlation_A_B," << correlation(audio[0], audio[1]) << '\n'
           << "correlation_A_C," << correlation(audio[0], audio[2]) << '\n'
           << "correlation_B_C," << correlation(audio[1], audio[2]) << '\n';
    if (!report || !writeListeningGuide(directory))
        return EXIT_FAILURE;

    for (const auto name : takeNames)
        std::cout << "wrote " << directory / (std::string(name) + ".wav") << '\n';
    std::cout << "blind key withheld; use --reveal only after listening\n";
}
