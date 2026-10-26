// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

// A season of the aquarium (300 s: the ecosystem's season length), rendered
// three times - without and with the ecological music, and with the organisms'
// second bodies (voice, drop, matter, breath, string) moved by the ecology -
// for the author's listening.
// Writes stereo float32 WAVs streamed to disk (no 100 MB buffers), a timeline
// CSV and a LISTENING GUIDE with every act at its second: where the hunts,
// captures, flights, duets, cadences and changes of state happen.
#include "core/BiomaEngine.h"

#include <array>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>

namespace
{
constexpr double sampleRate = 48000.0;
constexpr std::size_t blockSize = 256;
constexpr std::size_t seconds = 300;
constexpr std::size_t totalSamples = static_cast<std::size_t>(sampleRate) * seconds;

using Music = aquorbium::EcologicalMusicDirector;

const char* actName(Music::Act act)
{
    switch (act)
    {
        case Music::Act::grazing: return "pastagem";
        case Music::Act::hunt: return "caca (canone e ecolocalizacao)";
        case Music::Act::buzz: return "buzz terminal";
        case Music::Act::capture: return "captura";
        case Music::Act::flight: return "fuga (diminuicao)";
        case Music::Act::freeze: return "presa congela (silencio)";
        case Music::Act::rivalry: return "rivalidade (hoquetus)";
        case Music::Act::school: return "cardume (homorritmia/heterofonia)";
        case Music::Act::duet: return "dueto (chamado e resposta)";
        case Music::Act::defence: return "defesa do territorio";
        case Music::Act::modulation: return "modulacao (o migrante)";
        case Music::Act::resolution: return "resolucao (cadencia)";
        case Music::Act::phaseChange: return "mudanca de estado da agua";
        case Music::Act::rupture: return "ruptura do vidro";
        case Music::Act::bubbles: return "bolhas (Minnaert)";
        case Music::Act::departure: return "partida (vira memoria)";
        case Music::Act::bubbleNet: return "rede de bolhas (caca cooperativa)";
        case Music::Act::revolution: return "revolucao de canto";
        case Music::Act::songCycle: return "o canto recomeca";
        case Music::Act::death: return "uma voz morre (o canto vai ao recife)";
        case Music::Act::birth: return "uma voz nasce (cantos dos pais recombinados)";
        case Music::Act::soundmark: return "a marca sonora do lugar volta (Schafer)";
    }
    return "?";
}

const char* phaseName(Music::Phase phase)
{
    switch (phase)
    {
        case Music::Phase::ice: return "gelo";
        case Music::Phase::cold: return "fria";
        case Music::Phase::temperate: return "temperada";
        case Music::Phase::turbulent: return "turbulenta";
        case Music::Phase::boiling: return "fervura";
    }
    return "?";
}

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

void writeHeader(std::ofstream& stream, std::uint32_t dataBytes)
{
    stream.write("RIFF", 4);
    writeU32(stream, 36u + dataBytes);
    stream.write("WAVEfmt ", 8);
    writeU32(stream, 16u);
    writeU16(stream, 3u);  // IEEE float
    writeU16(stream, 2u);
    writeU32(stream, static_cast<std::uint32_t>(sampleRate));
    writeU32(stream, static_cast<std::uint32_t>(sampleRate * 8.0));
    writeU16(stream, 8u);
    writeU16(stream, 32u);
    stream.write("data", 4);
    writeU32(stream, dataBytes);
}

bool render(const std::filesystem::path& directory, const char* name, float influence, bool bodies = false)
{
    aquorbium::BiomaEngine engine;
    engine.prepare(sampleRate);
    engine.setSeed(0x815ab1eu);
    engine.setDensity(0.45f);
    engine.setDrift(0.20f);
    engine.setCohesion(0.58f);
    engine.setDanger(0.38f);
    engine.setEcosystemBlendImmediately(1.0f);
    engine.setEcosystemOutputFeedbackEnabled(true);
    engine.setAutonomousEnabled(true);
    engine.setEcologicalMusicPerformance({ influence, 0.6f, 0.5f });
    engine.setModalResonatorEnabled(true);
    engine.setGranularStrikerEnabled(true);
    engine.setSpatializerEnabled(true);
    engine.setMultiTapSpaceEnabled(true);
    if (bodies)
    {
        // the organisms' second bodies, transformed by the ecology
        using Kind = aquorbium::TimbreBody::Kind;
        aquorbium::BiomaEngine::TimbrePerformance timbre;
        timbre.body = { Kind::voice, Kind::drop, Kind::matter, Kind::breath, Kind::string };
        timbre.matter = { 0.35f, 0.55f, 0.60f, 0.40f, 0.30f };
        timbre.mix = { 0.85f, 0.85f, 0.85f, 0.85f, 0.85f };
        timbre.ecology = 1.0f;
        engine.setTimbrePerformance(timbre);
    }

    std::ofstream wav(directory / (std::string(name) + ".wav"), std::ios::binary);
    std::ofstream csv(directory / (std::string(name) + ".csv"));
    std::ofstream guide(directory / (std::string(name) + "_ROTEIRO_DE_ESCUTA.txt"));
    if (!wav || !csv || !guide)
        return false;
    writeHeader(wav, static_cast<std::uint32_t>(totalSamples * 2u * sizeof(float)));
    csv << "seconds,phase,heat,tension,capacity,events_last_bar,act,centre,mode,chord,tempo\n"
        << std::setprecision(5);
    guide << "Aquorbium - uma estacao (300 s), influencia da musica ecologica = " << influence << "\n"
             "Cada linha: o segundo, o acontecimento, o estado da agua e a tensao.\n\n";

    std::array<float, blockSize> left {}, right {};
    std::array<float, blockSize * 2> interleaved {};
    std::array<std::uint32_t, Music::actCount> lastCounts {};
    auto lastPhase = Music::Phase::temperate;
    bool first = true;
    double peak = 0.0;
    for (std::size_t offset = 0; offset < totalSamples; offset += blockSize)
    {
        engine.render(left.data(), right.data(), blockSize);
        for (std::size_t i = 0; i < blockSize; ++i)
        {
            interleaved[i * 2] = left[i];
            interleaved[i * 2 + 1] = right[i];
            peak = std::max(peak, static_cast<double>(std::max(std::abs(left[i]), std::abs(right[i]))));
        }
        wav.write(reinterpret_cast<const char*>(interleaved.data()), sizeof(interleaved));

        const auto t = static_cast<double>(offset + blockSize) / sampleRate;
        const auto& m = engine.telemetry().autonomy.music;
        if ((offset / blockSize) % 19 == 0)  // ~0.1 s
            csv << t << ',' << phaseName(m.phase) << ',' << m.heat << ',' << m.tension << ',' << m.capacity << ','
                << static_cast<unsigned>(m.eventsLastBar) << ',' << actName(m.act) << ',' << m.centre << ','
                << m.mode << ',' << m.chordDegree << ',' << m.tempoBpm << '\n';
        if (influence > 0.0f)
        {
            if (first || m.phase != lastPhase)
                guide << std::fixed << std::setprecision(1) << std::setw(6) << t << " s  == agua: "
                      << phaseName(m.phase) << " ==\n";
            for (std::size_t a = 0; a < Music::actCount; ++a)
                if (m.actCounts[a] != lastCounts[a])
                    guide << std::fixed << std::setprecision(1) << std::setw(6) << t << " s  "
                          << actName(static_cast<Music::Act>(a)) << "  (tensao "
                          << std::setprecision(2) << m.tension << ")\n";
            lastCounts = m.actCounts;
            lastPhase = m.phase;
            first = false;
        }
    }
    const auto& m = engine.telemetry().autonomy.music;
    guide << "\npico=" << peak << "  frentes ouvidas=" << m.wavefrontsHeard << "  copias de canto=" << m.songCopies
          << "\nmusical_approval=not_assessed; escuta humana necessaria\n";
    return static_cast<bool>(wav) && static_cast<bool>(csv) && peak > 1.0e-4;
}
}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cerr << "usage: aquorbium_ecological_music_render NEW_OUTPUT_DIRECTORY\n";
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
    if (!render(directory, "antes_sem_musica_ecologica", 0.0f) || !render(directory, "depois_musica_ecologica", 1.0f)
        || !render(directory, "corpos_e_timbre_ecologico", 1.0f, true))
        return EXIT_FAILURE;
    std::cout << directory << '\n';
    return EXIT_SUCCESS;
}
