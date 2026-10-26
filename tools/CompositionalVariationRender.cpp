// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/BiomaEngine.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

namespace
{
constexpr double sampleRate = 48000.0;
constexpr std::size_t blockSize = 256;
constexpr std::size_t telemetryInterval = 4800;
constexpr std::size_t maximumSamples =
    static_cast<std::size_t>(sampleRate * 180.0);

struct Frame
{
    std::size_t sample = 0;
    aquorbium::AutonomousEnergy::Telemetry state {};
};

struct Render
{
    std::vector<float> interleaved;
    std::vector<Frame> frames;
    std::size_t samples = 0;
    double renderSeconds = 0.0;
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

const char* segmentName(aquorbium::CompositionalVariationDirector::Segment segment)
{
    using Segment = aquorbium::CompositionalVariationDirector::Segment;
    switch (segment)
    {
        case Segment::establish: return "A_establish";
        case Segment::variation: return "A_prime_variation";
        case Segment::answer: return "B_answer";
        case Segment::suspension: return "suspension";
    }
    return "unknown";
}

Render render(float structureInfluence)
{
    aquorbium::BiomaEngine engine;
    engine.prepare(sampleRate);
    engine.setSeed(0x815ab1eu);
    engine.setDesire({ 0.62f, 0.56f, 0.68f, 0.42f });
    engine.setEcosystemOutputFeedbackEnabled(false);
    engine.setEcosystemBlendImmediately(1.0f);
    engine.setAutonomousEnabled(true);
    engine.setStructureMemoryPerformance(
        { structureInfluence, 0.8f, 0.8f, 0.5f });

    aquorbium::IntegratedModuleBank::Controls bank {};
    bank[aquorbium::IntegratedModuleBank::mix] = 1.0f;
    bank[aquorbium::IntegratedModuleBank::synthBodies] = 0.18f;
    bank[aquorbium::IntegratedModuleBank::percussionPulses] = 0.12f;
    bank[aquorbium::IntegratedModuleBank::fractalVector] = 0.10f;
    bank[aquorbium::IntegratedModuleBank::transposeSelect] = 0.10f;
    engine.setIntegratedModuleBankControls(bank);

    Render result;
    result.interleaved.resize(maximumSamples * 2);
    result.frames.reserve(maximumSamples / telemetryInterval + 1);
    std::array<float, blockSize> left {};
    std::array<float, blockSize> right {};
    std::size_t nextTelemetry = telemetryInterval;
    const auto started = std::chrono::steady_clock::now();

    for (std::size_t offset = 0; offset < maximumSamples; offset += blockSize)
    {
        const auto count = std::min(blockSize, maximumSamples - offset);
        engine.render(left.data(), right.data(), count);
        for (std::size_t index = 0; index < count; ++index)
        {
            result.interleaved[(offset + index) * 2] = left[index];
            result.interleaved[(offset + index) * 2 + 1] = right[index];
        }
        const auto end = offset + count;
        if (end >= nextTelemetry || end == maximumSamples)
        {
            const auto state = engine.telemetry().autonomy;
            result.frames.push_back({ end, state });
            result.samples = end;
            nextTelemetry += telemetryInterval;
            if (state.composition.number >= 3
                && state.composition.segment
                    == aquorbium::CompositionalVariationDirector::Segment::suspension)
                break;
        }
    }

    result.renderSeconds = std::chrono::duration<double>(
        std::chrono::steady_clock::now() - started).count();
    result.interleaved.resize(result.samples * 2);
    return result;
}

bool writeWav(const std::filesystem::path& path, const Render& render)
{
    const auto dataBytes = static_cast<std::uint32_t>(
        render.interleaved.size() * sizeof(float));
    std::ofstream stream(path, std::ios::binary);
    if (!stream) return false;
    stream.write("RIFF", 4);
    writeU32(stream, 36u + dataBytes);
    stream.write("WAVEfmt ", 8);
    writeU32(stream, 16u);
    writeU16(stream, 3u);
    writeU16(stream, 2u);
    writeU32(stream, static_cast<std::uint32_t>(sampleRate));
    writeU32(stream, static_cast<std::uint32_t>(sampleRate * 8.0));
    writeU16(stream, 8u);
    writeU16(stream, 32u);
    stream.write("data", 4);
    writeU32(stream, dataBytes);
    stream.write(reinterpret_cast<const char*>(render.interleaved.data()),
                 static_cast<std::streamsize>(dataBytes));
    return static_cast<bool>(stream);
}

bool writeCsv(const std::filesystem::path& path, const Render& render)
{
    std::ofstream stream(path);
    if (!stream) return false;
    stream << "seconds,segment,segment_number,events_in_segment,segment_length,"
              "structure_influence,rhythm_rotation,"
              "memory_weight,accent_weight,active_organisms,total_energy\n"
           << std::setprecision(8);
    for (const auto& frame : render.frames)
    {
        const auto& state = frame.state;
        stream << static_cast<double>(frame.sample) / sampleRate << ','
               << segmentName(state.composition.segment) << ','
               << state.composition.number << ','
               << static_cast<unsigned>(state.composition.eventsInSegment) << ','
               << static_cast<unsigned>(state.composition.segmentLength) << ','
               << state.structureInfluence << ','
               << state.composition.rhythmRotation << ','
               << state.composition.memoryWeight << ','
               << state.composition.accentWeight << ','
               << state.activeOrganisms << ',' << state.totalEnergy << '\n';
    }
    return static_cast<bool>(stream);
}

bool writeReport(const std::filesystem::path& path, const Render& bypass,
                 const Render& active)
{
    std::ofstream stream(path);
    if (!stream) return false;
    stream << "Aquorbium compositional variation audition render\n"
              "sample_rate=48000\nblock_size=256\nseed=0x815ab1e\n"
              "maximum_duration_seconds=180\n"
              "audio=stereo IEEE float32\n"
              "output_feedback=off\n"
              "module_bank=synth_bodies,percussion_pulses,fractal_vector,transpose_select\n"
              "musical_approval=not_assessed; human listening required\n"
           << std::setprecision(8)
           << "bypass_seconds=" << static_cast<double>(bypass.samples) / sampleRate << '\n'
           << "bypass_render_seconds=" << bypass.renderSeconds << '\n'
           << "bypass_last_segment="
           << (bypass.frames.empty() ? "none"
                                     : segmentName(bypass.frames.back().state.composition.segment))
           << '\n'
           << "active_seconds=" << static_cast<double>(active.samples) / sampleRate << '\n'
           << "active_render_seconds=" << active.renderSeconds << '\n'
           << "active_last_segment="
           << (active.frames.empty() ? "none"
                                     : segmentName(active.frames.back().state.composition.segment))
           << '\n'
           << "four_segments_reached="
           << (!active.frames.empty()
               && active.frames.back().state.composition.number >= 3 ? 1 : 0)
           << '\n';
    return static_cast<bool>(stream);
}
}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cerr << "usage: aquorbium_compositional_variation_render NEW_OUTPUT_DIRECTORY\n";
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

    const auto bypass = render(0.0f);
    if (bypass.samples == 0 || bypass.frames.empty()
        || !writeWav(directory / "structure_bypass.wav", bypass)
        || !writeCsv(directory / "structure_bypass.csv", bypass))
        return EXIT_FAILURE;

    const auto active = render(1.0f);
    if (active.samples == 0 || active.frames.empty()
        || !writeWav(directory / "structure_active.wav", active)
        || !writeCsv(directory / "structure_active.csv", active)
        || !writeReport(directory / "REPORT.txt", bypass, active))
        return EXIT_FAILURE;

    std::cout << directory << '\n';
    return active.frames.back().state.composition.number >= 3
        ? EXIT_SUCCESS : EXIT_FAILURE;
}
