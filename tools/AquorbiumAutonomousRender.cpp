// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/BiomaEngine.h"

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
#include <vector>

namespace
{
constexpr double sampleRate = 48000.0;
constexpr double durationSeconds = 30.0;
constexpr std::size_t durationSamples = static_cast<std::size_t>(sampleRate * durationSeconds);
constexpr std::size_t blockSize = 256;
constexpr std::size_t telemetryInterval = 4800;

struct Frame
{
    std::size_t sample = 0;
    aquorbium::BiomaEngine::Telemetry telemetry {};
};

struct Render
{
    std::vector<float> left;
    std::vector<float> right;
    std::vector<Frame> frames;
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

bool writeFloatWav(const std::filesystem::path& path, const Render& render)
{
    const auto dataBytes = static_cast<std::uint32_t>(render.left.size() * sizeof(float) * 2);
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
    for (std::size_t sample = 0; sample < render.left.size(); ++sample)
    {
        stream.write(reinterpret_cast<const char*>(&render.left[sample]), sizeof(float));
        stream.write(reinterpret_cast<const char*>(&render.right[sample]), sizeof(float));
    }
    return static_cast<bool>(stream);
}

Render renderAutonomous(bool modalResonatorEnabled, bool granularStrikerEnabled,
                        bool spatializerEnabled, bool outputFeedbackEnabled,
                        bool multiTapSpaceEnabled, const std::string& soloSpecies)
{
    aquorbium::BiomaEngine engine;
    engine.prepare(sampleRate);
    engine.setSeed(0x815ab1eu);
    engine.setDensity(0.45f);
    engine.setDrift(0.20f);
    engine.setCohesion(0.58f);
    engine.setDanger(0.38f);
    engine.setEcosystemBlendImmediately(1.0f);
    engine.setEcosystemOutputFeedbackEnabled(outputFeedbackEnabled);
    engine.setAutonomousEnabled(true);
    engine.setModalResonatorEnabled(modalResonatorEnabled);
    engine.setGranularStrikerEnabled(granularStrikerEnabled);
    engine.setSpatializerEnabled(spatializerEnabled);
    engine.setMultiTapSpaceEnabled(multiTapSpaceEnabled);
    if (!soloSpecies.empty())
    {
        constexpr std::array species {
            aquorbium::Ecosystem::Species::Oscillator,
            aquorbium::Ecosystem::Species::GranularStriker,
            aquorbium::Ecosystem::Species::Resonator,
            aquorbium::Ecosystem::Species::Pulsar,
            aquorbium::Ecosystem::Species::PhysicalString
        };
        constexpr std::array<const char*, species.size()> names {
            "oscillator", "granular", "resonator", "pulsar", "string"
        };
        for (std::size_t index = 0; index < species.size(); ++index)
            engine.setSpeciesGain(species[index], soloSpecies == names[index] ? 1.0f : 0.0f);
    }

    Render result;
    result.left.resize(durationSamples);
    result.right.resize(durationSamples);
    result.frames.reserve(durationSamples / telemetryInterval + 1);
    std::size_t nextTelemetrySample = telemetryInterval;

    for (std::size_t offset = 0; offset < durationSamples; offset += blockSize)
    {
        const auto count = std::min(blockSize, durationSamples - offset);
        engine.render(result.left.data() + offset, result.right.data() + offset, count);
        const auto end = offset + count;
        if (end >= nextTelemetrySample || end == durationSamples)
        {
            result.frames.push_back({ end, engine.telemetry() });
            nextTelemetrySample += telemetryInterval;
        }
    }
    return result;
}

struct Measurements
{
    double rms = 0.0;
    double peak = 0.0;
    double dcLeft = 0.0;
    double dcRight = 0.0;
    double maximumDelta = 0.0;
};

Measurements measure(const Render& render)
{
    Measurements result;
    double squared = 0.0;
    for (std::size_t sample = 0; sample < render.left.size(); ++sample)
    {
        const auto left = static_cast<double>(render.left[sample]);
        const auto right = static_cast<double>(render.right[sample]);
        squared += left * left + right * right;
        result.peak = std::max({ result.peak, std::abs(left), std::abs(right) });
        result.dcLeft += left;
        result.dcRight += right;
        if (sample > 0)
        {
            result.maximumDelta = std::max(
                result.maximumDelta,
                std::abs(left - static_cast<double>(render.left[sample - 1])));
            result.maximumDelta = std::max(
                result.maximumDelta,
                std::abs(right - static_cast<double>(render.right[sample - 1])));
        }
    }
    result.rms = std::sqrt(squared / static_cast<double>(render.left.size() * 2));
    result.dcLeft /= static_cast<double>(render.left.size());
    result.dcRight /= static_cast<double>(render.right.size());
    return result;
}

bool writeTelemetry(const std::filesystem::path& path, const Render& render)
{
    std::ofstream stream(path);
    if (!stream) return false;
    stream << "seconds,output_energy,bioenergy,active_organisms,root,event_count,day,season"
              ",granular_particles,granular_energy,granular_excitations,granular_stolen"
              ",resonator_energy,resonator_excitations,resonator_material,resonator_decay"
              ",string_energy,string_excitations,string_frequency,string_decay"
              ",string_brightness,string_hardness,string_pluck_position";
    stream << ",space_wet_energy,space_return,space_feedback";
    for (std::size_t index = 0; index < aquorbium::AutonomousEnergy::organismCount; ++index)
        stream << ",organism" << index << "_energy,organism" << index << "_pitch"
               << ",organism" << index << "_pan,organism" << index << "_depth_cutoff";
    stream << '\n' << std::setprecision(7);
    for (const auto& frame : render.frames)
    {
        const auto& state = frame.telemetry;
        stream << static_cast<double>(frame.sample) / sampleRate << ','
               << state.outputEnergy << ',' << state.autonomy.totalEnergy << ','
               << state.autonomy.activeOrganisms << ',' << state.autonomy.harmonicRootMidi << ','
               << state.autonomy.eventCount << ',' << state.environment.dayPhase << ','
               << state.environment.seasonPhase << ','
               << state.granularStriker.activeParticles << ','
               << state.granularStriker.particleEnergy << ','
               << state.granularStriker.excitationCount << ','
               << state.granularStriker.stolenParticleCount << ','
               << state.resonator.modalEnergy << ','
               << state.resonator.excitationCount << ','
               << state.resonator.parameters.material << ','
               << state.resonator.parameters.decaySeconds << ','
               << state.physicalString.stringEnergy << ','
               << state.physicalString.excitationCount << ','
               << state.physicalString.parameters.fundamentalHz << ','
               << state.physicalString.parameters.decaySeconds << ','
               << state.physicalString.parameters.brightness << ','
               << state.physicalString.parameters.hardness << ','
               << state.physicalString.parameters.pluckPosition << ','
               << state.multiTapSpace.wetEnergy << ','
               << state.multiTapSpaceReturnCurrent << ','
               << state.multiTapSpace.currentFeedback;
        for (std::size_t index = 0; index < state.autonomy.voices.size(); ++index)
        {
            const auto& voice = state.autonomy.voices[index];
            stream << ',' << voice.energy << ',' << voice.midiPitch
                   << ',' << state.spatial[index].effectivePan
                   << ',' << state.spatial[index].depthCutoffHz;
        }
        stream << '\n';
    }
    return static_cast<bool>(stream);
}

bool writeReport(const std::filesystem::path& path, const Measurements& measurements,
                 const aquorbium::BiomaEngine::Telemetry& finalState,
                 const std::string& soloSpecies)
{
    std::ofstream stream(path);
    if (!stream) return false;
    stream << "Aquorbium autonomous render\n"
              "external_note_events=0\n"
              "sample_rate=48000\n"
              "duration_seconds=30\n"
              "seed=0x815ab1e\n"
              "solo_species=" << (soloSpecies.empty() ? "none" : soloSpecies) << "\n"
              "output_feedback=" << (finalState.outputFeedbackEnabled ? "on" : "off") << "\n"
              "granular_striker=" << (finalState.granularStrikerEnabled ? "on" : "off") << "\n"
              "modal_resonator=" << (finalState.modalResonatorEnabled ? "on" : "off") << "\n"
              "spatializer=" << (finalState.spatializerEnabled ? "on" : "off") << "\n"
              "multi_tap_space=" << (finalState.multiTapSpaceEnabled ? "on" : "off") << "\n"
              "multi_tap_space_return=" << finalState.multiTapSpaceReturnCurrent << "\n"
              "multi_tap_space_wet_energy=" << finalState.multiTapSpace.wetEnergy << "\n"
              "multi_tap_space_feedback=" << finalState.multiTapSpace.currentFeedback << "\n"
              "rms=" << measurements.rms << "\n"
              "peak=" << measurements.peak << "\n"
              "dc_left=" << measurements.dcLeft << "\n"
              "dc_right=" << measurements.dcRight << "\n"
              "maximum_adjacent_sample_delta=" << measurements.maximumDelta << "\n"
              "final_active_organisms=" << finalState.autonomy.activeOrganisms << "\n"
              "final_event_count=" << finalState.autonomy.eventCount << "\n"
              "granular_excitations=" << finalState.granularStriker.excitationCount << "\n"
              "granular_stolen_particles=" << finalState.granularStriker.stolenParticleCount << "\n"
              "granular_final_energy=" << finalState.granularStriker.particleEnergy << "\n"
              "resonator_excitations=" << finalState.resonator.excitationCount << "\n"
              "resonator_final_energy=" << finalState.resonator.modalEnergy << "\n"
              "resonator_material=" << finalState.resonator.parameters.material << "\n"
              "resonator_decay_seconds=" << finalState.resonator.parameters.decaySeconds << "\n"
              "physical_string_excitations=" << finalState.physicalString.excitationCount << "\n"
              "physical_string_final_energy=" << finalState.physicalString.stringEnergy << "\n"
              "physical_string_frequency=" << finalState.physicalString.parameters.fundamentalHz << "\n"
              "physical_string_decay_seconds=" << finalState.physicalString.parameters.decaySeconds << "\n"
              "final_harmonic_root_midi=" << finalState.autonomy.harmonicRootMidi << "\n";
    return static_cast<bool>(stream);
}
}

int main(int argc, char** argv)
{
    const auto outputDirectory = argc > 1
        ? std::filesystem::path(argv[1])
        : std::filesystem::path("validation-output/aquorbium-autonomous");
    std::error_code error;
    std::filesystem::create_directories(outputDirectory, error);
    if (error)
    {
        std::cerr << "could not create output directory: " << error.message() << '\n';
        return EXIT_FAILURE;
    }

    auto modalResonatorEnabled = true;
    auto granularStrikerEnabled = true;
    auto spatializerEnabled = true;
    auto outputFeedbackEnabled = true;
    auto multiTapSpaceEnabled = true;
    std::string soloSpecies;
    for (int argument = 2; argument < argc; ++argument)
    {
        const std::string option(argv[argument]);
        if (option == "--resonator-off") modalResonatorEnabled = false;
        else if (option == "--granular-off") granularStrikerEnabled = false;
        else if (option == "--space-off") spatializerEnabled = false;
        else if (option == "--multitap-off") multiTapSpaceEnabled = false;
        else if (option == "--feedback-off") outputFeedbackEnabled = false;
        else if (option.starts_with("--solo="))
        {
            soloSpecies = option.substr(7);
            if (soloSpecies != "oscillator" && soloSpecies != "granular"
                && soloSpecies != "resonator" && soloSpecies != "pulsar"
                && soloSpecies != "string")
            {
                std::cerr << "unknown solo species: " << soloSpecies << '\n';
                return EXIT_FAILURE;
            }
        }
        else
        {
            std::cerr << "unknown option: " << option << '\n';
            return EXIT_FAILURE;
        }
    }
    const auto render = renderAutonomous(
        modalResonatorEnabled, granularStrikerEnabled,
        spatializerEnabled, outputFeedbackEnabled, multiTapSpaceEnabled,
        soloSpecies);
    const auto measurements = measure(render);
    const auto finalState = render.frames.back().telemetry;
    if (!std::isfinite(measurements.rms) || measurements.rms <= 0.0
        || !std::isfinite(measurements.peak) || measurements.peak >= 1.0
        || finalState.activeVoices != 0 || !finalState.autonomousEnabled
        || finalState.granularStriker.excitationCount == 0
        || finalState.resonator.excitationCount == 0
        || finalState.physicalString.excitationCount == 0)
    {
        std::cerr << "autonomous render failed validation\n";
        return EXIT_FAILURE;
    }

    const auto wav = outputDirectory / "AQUORBIUM_AUTONOMOUS.wav";
    if (!writeFloatWav(wav, render)
        || !writeTelemetry(outputDirectory / "telemetry.csv", render)
        || !writeReport(outputDirectory / "REPORT.txt", measurements, finalState,
                        soloSpecies))
    {
        std::cerr << "could not write autonomous validation files\n";
        return EXIT_FAILURE;
    }

    std::cout << "wrote " << wav << '\n'
              << "rms=" << measurements.rms << " peak=" << measurements.peak
              << " max_delta=" << measurements.maximumDelta << '\n'
              << "external_note_events=0 events=" << finalState.autonomy.eventCount
              << " active_organisms=" << finalState.autonomy.activeOrganisms
              << " granular_excitations=" << finalState.granularStriker.excitationCount
              << " resonator_excitations=" << finalState.resonator.excitationCount
              << " string_excitations=" << finalState.physicalString.excitationCount << '\n';
}
