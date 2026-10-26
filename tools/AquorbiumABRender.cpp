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
#include <iterator>
#include <optional>
#include <string>
#include <vector>

namespace
{
constexpr double sampleRate = 48000.0;
constexpr std::size_t durationSamples = static_cast<std::size_t>(sampleRate * 20.0);
constexpr std::size_t telemetryInterval = 240;

struct Event
{
    std::size_t sample = 0;
    bool noteOn = true;
    int note = 60;
    float velocity = 0.8f;
};

struct TelemetryFrame
{
    std::size_t sample = 0;
    aquorbium::BiomaEngine::Telemetry state {};
};

struct RenderedTake
{
    std::vector<float> left;
    std::vector<float> right;
    std::vector<TelemetryFrame> telemetry;
};

enum class LensPosition { None, Tracking, Mirrored };

constexpr std::array<Event, 10> score {{
    { 0, true, 36, 0.78f },
    { 96000, false, 36, 0.0f },
    { 120000, true, 43, 0.72f },
    { 216000, false, 43, 0.0f },
    { 240000, true, 48, 0.68f },
    { 336000, false, 48, 0.0f },
    { 384000, true, 55, 0.74f },
    { 480000, false, 55, 0.0f },
    { 576000, true, 60, 0.70f },
    { 768000, false, 60, 0.0f },
}};

RenderedTake renderTake(
    aquorbium::BiomaEngine::EcosystemRenderMode mode,
    std::array<float, aquorbium::Ecosystem::observerCount> listeningWeights
        = { 0.25f, 0.25f, 0.25f, 0.25f },
    LensPosition lensPosition = LensPosition::None)
{
    aquorbium::BiomaEngine engine;
    engine.prepare(sampleRate);
    engine.setSeed(0xA0B0B1u);
    engine.setDesire({ 0.32f, 0.72f, 0.66f, 0.58f });
    engine.setListeningBlend(listeningWeights);
    engine.setEcosystemBlendImmediately(
        mode == aquorbium::BiomaEngine::EcosystemRenderMode::Active ? 1.0f : 0.0f);

    // The controlled A/B keeps the ecological control stream identical. A
    // later listening pass may enable feedback to assess that relationship.
    engine.setEcosystemOutputFeedbackEnabled(false);
    if (lensPosition != LensPosition::None) engine.setAutonomousEnabled(true);

    RenderedTake result;
    result.left.resize(durationSamples);
    result.right.resize(durationSamples);
    result.telemetry.reserve(durationSamples / telemetryInterval + 1);

    std::size_t eventIndex = 0;
    for (std::size_t offset = 0; offset < durationSamples; offset += telemetryInterval)
    {
        if (lensPosition != LensPosition::None)
        {
            // Identical ecology, but one Lens follows organism 0 and the other
            // follows its mirrored direction. Targets update on the same
            // 200 Hz schedule, independent of rendered callback boundaries.
            auto target = engine.telemetry().organisms[0].position;
            if (lensPosition == LensPosition::Mirrored)
                target = { -target.x, -target.y, -target.z };
            if (offset >= 96000 && offset < 576000) engine.setLensGesture(true, target);
            if (offset == 576000) engine.setLensGesture(false, target);
        }
        while (eventIndex < score.size() && score[eventIndex].sample == offset)
        {
            const auto& event = score[eventIndex++];
            if (event.noteOn)
                engine.noteOn(event.note, event.velocity);
            else
                engine.noteOff(event.note);
        }
        const auto count = std::min(telemetryInterval, durationSamples - offset);
        engine.render(result.left.data() + offset, result.right.data() + offset, count);
        result.telemetry.push_back({ offset + count, engine.telemetry() });
    }
    return result;
}

double rms(const RenderedTake& take)
{
    double total = 0.0;
    for (std::size_t index = 0; index < take.left.size(); ++index)
        total += static_cast<double>(take.left[index]) * take.left[index]
            + static_cast<double>(take.right[index]) * take.right[index];
    return std::sqrt(total / static_cast<double>(take.left.size() * 2));
}

double peak(const RenderedTake& take)
{
    double value = 0.0;
    for (std::size_t index = 0; index < take.left.size(); ++index)
    {
        value = std::max(value, static_cast<double>(std::abs(take.left[index])));
        value = std::max(value, static_cast<double>(std::abs(take.right[index])));
    }
    return value;
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

bool writeWav(const std::filesystem::path& path, const RenderedTake& take, float gain)
{
    const auto dataBytes = static_cast<std::uint32_t>(take.left.size() * sizeof(float) * 2);
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
    for (std::size_t index = 0; index < take.left.size(); ++index)
    {
        const auto left = take.left[index] * gain;
        const auto right = take.right[index] * gain;
        stream.write(reinterpret_cast<const char*>(&left), sizeof(left));
        stream.write(reinterpret_cast<const char*>(&right), sizeof(right));
    }
    return static_cast<bool>(stream);
}

bool writeTelemetryCsv(const std::filesystem::path& path, const RenderedTake& take)
{
    std::ofstream stream(path);
    if (!stream) return false;
    stream << "sample,seconds,mode,morph_target,morph_current,feedback,active_voices,output_energy,"
              "calm,density,cohesion,danger,"
              "day,season,temperature,daylight,mating,health,crack_stress,cracked,shattered";
    for (std::size_t index = 0; index < aquorbium::Ecosystem::organismCount; ++index)
        stream << ",organism" << index << "_x,organism" << index << "_y,organism" << index
               << "_z,mix" << index << "_left,mix" << index << "_right,mix" << index << "_detail";
    stream << ",lens_x,lens_y,lens_z,lens_target_x,lens_target_y,lens_target_z,lens_owner,lens_focused_organisms\n"
           << std::setprecision(7);

    for (const auto& frame : take.telemetry)
    {
        const auto& value = frame.state;
        stream << frame.sample << ',' << static_cast<double>(frame.sample) / sampleRate << ','
               << (value.renderMode == aquorbium::BiomaEngine::EcosystemRenderMode::Active ? "active" : "neutral")
               << ',' << value.ecosystemBlendTarget << ',' << value.ecosystemBlendCurrent
               << ',' << (value.outputFeedbackEnabled ? 1 : 0) << ',' << value.activeVoices << ','
               << value.outputEnergy << ',' << value.desire.calm << ',' << value.desire.density << ','
               << value.desire.cohesion << ',' << value.desire.danger << ','
               << value.environment.dayPhase << ',' << value.environment.seasonPhase << ','
               << value.environment.temperature << ',' << value.environment.daylight << ','
               << value.environment.matingPulse << ',' << value.integrity.health << ','
               << value.integrity.crackStress << ',' << (value.integrity.cracked ? 1 : 0) << ','
               << (value.integrity.shattered ? 1 : 0);
        for (std::size_t index = 0; index < aquorbium::Ecosystem::organismCount; ++index)
        {
            const auto& organism = value.organisms[index];
            const auto& mix = value.mix[index];
            stream << ',' << organism.position.x << ',' << organism.position.y << ',' << organism.position.z
                   << ',' << mix.left << ',' << mix.right << ',' << mix.detail;
        }
        stream << ',' << value.lens.surfacePosition.x << ',' << value.lens.surfacePosition.y
               << ',' << value.lens.surfacePosition.z << ',' << value.lens.targetPosition.x
               << ',' << value.lens.targetPosition.y << ',' << value.lens.targetPosition.z
               << ',' << value.lens.heldByObserver << ',' << value.lensFocusedOrganisms << '\n';
    }
    return static_cast<bool>(stream);
}

bool writeAnalysisManifest(const std::filesystem::path& path, double neutralRms, double activeRms,
                           double neutralPeak, double activePeak, float activeGain)
{
    std::ofstream stream(path);
    if (!stream) return false;
    stream << "Aquorbium controlled Ecosystem A/B\n"
              "seed=0xA0B0B1\n"
              "sample_rate=48000\n"
              "duration_seconds=20\n"
              "output_feedback=off\n"
              "neutral_rms=" << neutralRms << "\n"
              "active_rms_before_match=" << activeRms << "\n"
              "neutral_peak=" << neutralPeak << "\n"
              "active_peak_before_match=" << activePeak << "\n"
              "active_gain_to_match_neutral_rms=" << activeGain << "\n"
              "\n"
              "The active WAV is RMS-matched to neutral for listening. The raw measurements\n"
              "remain above. Both takes receive identical note events and deterministic\n"
              "ecological control; output-energy feedback is disabled by design.\n";
    return static_cast<bool>(stream);
}

bool writeListeningProtocol(const std::filesystem::path& path)
{
    std::ofstream stream(path);
    if (!stream) return false;
    stream << R"(# Aquorbium — escuta A/B cega do Ecossistema

## Antes de ouvir

Ouça somente `A.wav` e `B.wav`. Não abra `analysis/` nem use a opção
`--reveal` antes de preencher esta ficha: eles contêm a identidade técnica dos
takes. Ambos têm 20 segundos, a mesma seed, os mesmos eventos e RMS igualado.
O feedback de energia está desligado nos dois lados para isolar a influência da
mixagem ecológica.

Use fones ou monitores conhecidos, nível moderado e constante. Ouça em ordem
alternada ao menos três vezes, sem tentar identificar o algoritmo primeiro.

## Ficha de resposta

Sessão: ____________________    Data: ____________________

1. Qual take apresenta relações mais legíveis entre primeiro plano e fundo?
   A / B / indistinto

2. Qual take apresenta maior sensação de profundidade, posição e perspectiva?
   A / B / indistinto

3. Qual take sustenta melhor interesse ao longo dos 20 segundos?
   A / B / indistinto

4. Qual take você escolheria como base para performance? Por quê?

   _________________________________________________________________
   _________________________________________________________________

5. Houve algum problema? Marque e descreva tempo aproximado.

   mascaramento / perda de grave / imagem estéreo instável / transiente ruim /
   outro: ___________________________________________________________

6. Descreva em palavras livres o que cada take faz musicalmente.

   A: ______________________________________________________________
   B: ______________________________________________________________

Depois de preencher, execute novamente o renderizador com `--reveal` para
criar `REVEAL_AFTER_LISTENING.txt`. Só então compare a resposta com o estado
técnico guardado em `analysis/`.
)";
    return static_cast<bool>(stream);
}

bool writeReveal(const std::filesystem::path& path, bool activeIsA)
{
    std::ofstream stream(path);
    if (!stream) return false;
    stream << "Aquorbium controlled Ecosystem A/B — answer key\n\n"
           << (activeIsA ? "A.wav = Active Ecossistema\nB.wav = Neutral route\n"
                         : "A.wav = Neutral route\nB.wav = Active Ecossistema\n")
           << "\nThe technical evidence is in analysis/. Interpret the answer only after\n"
              "the blind listening notes; the comparison tests mix behaviour, not an\n"
              "absolute judgement of the future instrument.\n";
    return static_cast<bool>(stream);
}

bool writePerspectiveManifest(const std::filesystem::path& path,
                              std::size_t observer, double equalRms,
                              double focusedRms, float focusedGain)
{
    std::ofstream stream(path);
    if (!stream) return false;
    stream << "Aquorbium controlled perspective A/B\n"
           << "observer=" << observer << "\n"
           << "seed=0xA0B0B1\n"
           << "sample_rate=48000\n"
           << "duration_seconds=20\n"
           << "output_feedback=off\n"
           << "equal_rms=" << equalRms << "\n"
           << "focused_rms_before_match=" << focusedRms << "\n"
           << "focused_gain_to_match_equal_rms=" << focusedGain << "\n"
           << "A.wav=focused (RMS matched)\n"
           << "B.wav=equal weights\n";
    return static_cast<bool>(stream);
}

bool writePerspectiveProtocol(const std::filesystem::path& path)
{
    std::ofstream stream(path);
    if (!stream) return false;
    stream << R"(# Aquorbium — escuta cega das perspectivas

Ouça somente A.wav e B.wav antes de abrir analysis/ ou usar --reveal.
As duas tomadas têm a mesma seed, os mesmos eventos, 20 segundos e RMS
igualado. O feedback de energia está desligado. Compare em nível moderado.

Sessão: ____________________    Data: ____________________

1. Que tomada deixa primeiro plano e sustentação mais legíveis?
   A / B / indistinto

2. Qual permite localizar melhor os cinco organismos no espaço?
   A / B / indistinto

3. Qual é mais interessante musicalmente, e em que momentos?
   _________________________________________________________________

4. Alguma tomada perde grave, mascara ataques ou cansa a escuta?
   _________________________________________________________________

5. Descreva livremente A e B antes de revelar a chave.
   A: ______________________________________________________________
   B: ______________________________________________________________

Depois de preencher, rode novamente o mesmo comando com --reveal.
Essa opção só grava a chave; não reescreve os WAVs ou esta ficha.
)";
    return static_cast<bool>(stream);
}

bool writePerspectiveReveal(const std::filesystem::path& path,
                            std::size_t observer)
{
    std::ofstream stream(path);
    if (!stream) return false;
    stream << "Aquorbium perspective A/B — answer key\n\n"
           << "A.wav = observer " << observer << " alone in the blend\n"
           << "B.wav = four observers equally weighted\n"
           << "Both use the Active ecosystem route.\n";
    return static_cast<bool>(stream);
}
}

int main(int argc, char* argv[])
{
    std::filesystem::path output = "validation-output/aquorbium-ecosystem-ab";
    bool reveal = false;
    bool lens = false;
    std::optional<std::size_t> focusedObserver;
    for (int index = 1; index < argc; ++index)
    {
        const std::string argument = argv[index];
        if (argument == "--lens" && !lens)
        {
            lens = true;
            continue;
        }
        if (argument == "--reveal")
        {
            reveal = true;
            continue;
        }
        if (argument.starts_with("--observer="))
        {
            if (argument.size() != 12 || argument[11] < '0' || argument[11] > '3'
                || focusedObserver.has_value())
            {
                std::cerr << "observer must be one of 0, 1, 2, 3\n";
                return EXIT_FAILURE;
            }
            focusedObserver = static_cast<std::size_t>(argument[11] - '0');
            continue;
        }
        if (argument.starts_with("--"))
        {
            std::cerr << "usage: aquorbium_ab_render [output-directory] [--lens] [--observer=0..3] [--reveal]\n";
            return EXIT_FAILURE;
        }
        if (output != "validation-output/aquorbium-ecosystem-ab")
        {
            std::cerr << "usage: aquorbium_ab_render [output-directory] [--lens] [--observer=0..3] [--reveal]\n";
            return EXIT_FAILURE;
        }
        output = argument;
    }

    if (lens && focusedObserver && *focusedObserver != 0)
    {
        std::cerr << "Lens belongs to observer 0; use --observer=0 or equal weights\n";
        return EXIT_FAILURE;
    }
    const auto lensManifestPrefix = std::string("Aquorbium controlled Lens A/B\nprotocol=tracking-v1\nweights=")
        + (focusedObserver ? "observer0\n" : "equal\n");

    const auto analysisDirectory = output / "analysis";
    if (reveal)
    {
        if (!std::filesystem::exists(output / "A.wav")
            || !std::filesystem::exists(output / "B.wav"))
        {
            std::cerr << "no existing A/B session to reveal\n";
            return EXIT_FAILURE;
        }
        std::ifstream manifest(analysisDirectory / "MANIFEST.txt");
        const std::string contents((std::istreambuf_iterator<char>(manifest)),
                                   std::istreambuf_iterator<char>());
        const auto expected = lens ? lensManifestPrefix : focusedObserver
            ? "Aquorbium controlled perspective A/B\nobserver="
                + std::to_string(*focusedObserver) + "\n"
            : std::string("Aquorbium controlled Ecosystem A/B\n");
        if (contents.find(expected) != 0)
        {
            std::cerr << "existing A/B session does not match requested mode\n";
            return EXIT_FAILURE;
        }
        bool success = false;
        if (lens)
        {
            std::ofstream key(output / "REVEAL_AFTER_LISTENING.txt");
            key << "A.wav = Lens tracks organism 0\nB.wav = Lens tracks its mirrored direction\n"
                   "Held from 2 to 12 seconds; feedback disabled; autonomous bank enabled.\n";
            success = static_cast<bool>(key);
        }
        else success = focusedObserver
            ? writePerspectiveReveal(output / "REVEAL_AFTER_LISTENING.txt", *focusedObserver)
            : writeReveal(output / "REVEAL_AFTER_LISTENING.txt", true);
        if (!success)
        {
            std::cerr << "could not write answer key\n";
            return EXIT_FAILURE;
        }
        std::cout << "Answer key written without changing the listening files\n";
        return EXIT_SUCCESS;
    }

    if (std::filesystem::exists(output / "A.wav")
        || std::filesystem::exists(output / "B.wav")
        || std::filesystem::exists(output / "LISTENING_PROTOCOL.md"))
    {
        std::cerr << "A/B session already exists; choose a new output directory\n";
        return EXIT_FAILURE;
    }
    std::error_code error;
    std::filesystem::create_directories(output, error);
    if (error)
    {
        std::cerr << "could not create " << output << ": " << error.message() << '\n';
        return EXIT_FAILURE;
    }

    std::filesystem::create_directories(analysisDirectory, error);
    if (error)
    {
        std::cerr << "could not create " << analysisDirectory << ": " << error.message() << '\n';
        return EXIT_FAILURE;
    }

    if (lens)
    {
        const auto weights = focusedObserver ? std::array<float, 4> { 1, 0, 0, 0 }
                                            : std::array<float, 4> { 0.25f, 0.25f, 0.25f, 0.25f };
        const auto front = renderTake(aquorbium::BiomaEngine::EcosystemRenderMode::Active,
                                      weights, LensPosition::Tracking);
        const auto opposite = renderTake(aquorbium::BiomaEngine::EcosystemRenderMode::Active,
                                         weights, LensPosition::Mirrored);
        const auto frontRms = rms(front);
        const auto oppositeRms = rms(opposite);
        if (!(frontRms > 1.0e-12) || !(oppositeRms > 1.0e-12)
            || !std::isfinite(frontRms) || !std::isfinite(oppositeRms))
        {
            std::cerr << "invalid Lens A/B energy\n";
            return EXIT_FAILURE;
        }
        const auto gain = static_cast<float>(oppositeRms / frontRms);
        if (!std::isfinite(gain) || gain * peak(front) >= 1.0 || peak(opposite) >= 1.0)
        {
            std::cerr << "Lens A/B level exceeds audition ceiling\n";
            return EXIT_FAILURE;
        }
        double difference = 0;
        double maxStep = 0;
        for (std::size_t i = 0; i < front.left.size(); ++i)
        {
            const auto dl = static_cast<double>(front.left[i]) * gain - opposite.left[i];
            const auto dr = static_cast<double>(front.right[i]) * gain - opposite.right[i];
            difference += dl * dl + dr * dr;
            if (i > 0)
                maxStep = std::max({ maxStep,
                    std::abs(static_cast<double>(front.left[i] - front.left[i - 1]) * gain),
                    std::abs(static_cast<double>(front.right[i] - front.right[i - 1]) * gain),
                    std::abs(static_cast<double>(opposite.left[i] - opposite.left[i - 1])),
                    std::abs(static_cast<double>(opposite.right[i] - opposite.right[i - 1])) });
        }
        const auto differenceRms = std::sqrt(difference / static_cast<double>(front.left.size() * 2));
        const auto focusedFrames = [](const RenderedTake& take) {
            return std::count_if(take.telemetry.begin(), take.telemetry.end(),
                                 [](const TelemetryFrame& frame) { return frame.state.lensFocusedOrganisms > 0; });
        };
        std::ofstream manifest(analysisDirectory / "MANIFEST.txt");
        manifest << lensManifestPrefix << std::setprecision(9)
                 << "sample_rate=48000\nduration_seconds=20\nseed=0xA0B0B1\n"
                    "held_seconds=2..12\nautonomous=1\nfeedback=0\nautonomy_homeostasis=enabled\n"
                    "target_updates_hz=200\ntargets=organism0_direction_and_mirror\n"
                 << "A_raw_rms=" << frontRms << "\nB_raw_rms=" << oppositeRms
                 << "\nA_gain=" << gain << "\nA_peak=" << peak(front) * gain
                 << "\nB_peak=" << peak(opposite) << "\ndifference_rms=" << differenceRms
                 << "\nmax_sample_delta=" << maxStep
                 << "\nA_focused_frames=" << focusedFrames(front)
                 << "\nB_focused_frames=" << focusedFrames(opposite)
                 << "\nTechnical difference is not proof of perceptual audibility or absence of clicks.\n";
        manifest.flush();
        std::ofstream protocol(output / "LISTENING_PROTOCOL.md");
        protocol << "Aquorbium — escuta da Lupa\n\n"
                    "Dois alvos móveis: direção do organismo 0 e direção espelhada.\n"
                    "Mesma seed, notas e pesos. RMS global igualado.\n"
                    "Banco autônomo ligado, feedback ecológico desligado; posse de 2 a 12 s.\n"
                    "A homeostase do banco autônomo permanece ativa.\n"
                    "A posição desliza a 0,55 rad/s; não muda instantaneamente.\n"
                    "Compare foco, timbre, presença e panorama, especialmente de 3 a 12 s.\n"
                    "Anote cliques/estalos ao pressionar (2 s), mover e soltar (12 s).\n"
                    "RMS igualado no arquivo inteiro não iguala cada trecho individual.\n\n"
                    "Diferença percebida: \nPreferência: \nCliques e instantes: \nObservações: \n\n"
                    "Após preencher, repita o comando com --reveal para obter a chave.\n";
        protocol.flush();
        const auto success = static_cast<bool>(manifest) && static_cast<bool>(protocol)
            && writeWav(output / "A.wav", front, gain)
            && writeWav(output / "B.wav", opposite, 1.0f)
            && writeTelemetryCsv(analysisDirectory / "A-telemetry.csv", front)
            && writeTelemetryCsv(analysisDirectory / "B-telemetry.csv", opposite);
        if (!success)
        {
            std::cerr << "could not write Lens A/B artefacts\n";
            return EXIT_FAILURE;
        }
        std::cout << "Lens A/B written to " << output << "; difference_rms=" << differenceRms
                  << "; max_sample_delta=" << maxStep << '\n';
        return EXIT_SUCCESS;
    }

    if (focusedObserver)
    {
        auto weights = std::array<float, aquorbium::Ecosystem::observerCount> {};
        weights[*focusedObserver] = 1.0f;
        const auto equal = renderTake(aquorbium::BiomaEngine::EcosystemRenderMode::Active);
        const auto focused = renderTake(
            aquorbium::BiomaEngine::EcosystemRenderMode::Active, weights);
        const auto equalRms = rms(equal);
        const auto focusedRms = rms(focused);
        if (!(focusedRms > 1.0e-12) || !std::isfinite(focusedRms)
            || !std::isfinite(equalRms))
        {
            std::cerr << "invalid perspective A/B energy\n";
            return EXIT_FAILURE;
        }
        const auto focusedGain = static_cast<float>(equalRms / focusedRms);
        if (!std::isfinite(focusedGain) || focusedGain * peak(focused) >= 1.0)
        {
            std::cerr << "perspective A/B level exceeds float audition ceiling\n";
            return EXIT_FAILURE;
        }
        const auto success = writeWav(output / "A.wav", focused, focusedGain)
            && writeWav(output / "B.wav", equal, 1.0f)
            && writeTelemetryCsv(analysisDirectory / "equal-telemetry.csv", equal)
            && writeTelemetryCsv(analysisDirectory / "focused-telemetry.csv", focused)
            && writePerspectiveManifest(analysisDirectory / "MANIFEST.txt",
                                        *focusedObserver, equalRms, focusedRms, focusedGain)
            && writePerspectiveProtocol(output / "LISTENING_PROTOCOL.md");
        if (!success)
        {
            std::cerr << "could not write perspective A/B artefacts\n";
            return EXIT_FAILURE;
        }
        std::cout << "Blind perspective A/B session written to " << output << '\n';
        return EXIT_SUCCESS;
    }

    const auto neutral = renderTake(aquorbium::BiomaEngine::EcosystemRenderMode::Neutral);
    const auto active = renderTake(aquorbium::BiomaEngine::EcosystemRenderMode::Active);
    constexpr bool activeIsA = true;
    const auto neutralRms = rms(neutral);
    const auto activeRms = rms(active);
    const auto activeGain = activeRms > 1.0e-12 ? static_cast<float>(neutralRms / activeRms) : 1.0f;
    const auto success = writeWav(output / "A.wav", activeIsA ? active : neutral,
                                  activeIsA ? activeGain : 1.0f)
        && writeWav(output / "B.wav", activeIsA ? neutral : active,
                    activeIsA ? 1.0f : activeGain)
        && writeTelemetryCsv(analysisDirectory / "neutral-telemetry.csv", neutral)
        && writeTelemetryCsv(analysisDirectory / "ecosystem-active-telemetry.csv", active)
        && writeAnalysisManifest(analysisDirectory / "MANIFEST.txt", neutralRms, activeRms,
                                 peak(neutral), peak(active), activeGain)
        && writeListeningProtocol(output / "LISTENING_PROTOCOL.md");
    if (!success)
    {
        std::cerr << "could not write A/B artefacts to " << output << '\n';
        return EXIT_FAILURE;
    }
    std::cout << "Blind A/B session written to " << output << '\n';
    return EXIT_SUCCESS;
}
