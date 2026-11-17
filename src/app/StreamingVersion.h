// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

// Aquorbium — versão de entrega (streaming) da tomada do REC.
#pragma once

#include "core/Dither.h"
#include "core/DeliveryMaster.h"

#include <juce_audio_formats/juce_audio_formats.h>

#include <algorithm>
#include <memory>
#include <unordered_map>
#include <vector>

// Master de entrega (16/11/2026, saida-ref 1.1; o autor: "nivele todos os
// instrumentos para que fiquem preparados para os spotifys da vida"). Depois
// que a tomada crua fecha, uma versão a -14 LUFS / <= -1 dBTP é escrita ao lado
// (<nome>_streaming.wav, 24 bits com TPDF) com um relatório (<nome>_streaming.txt).
// Offline, lendo e escrevendo em blocos (memória constante), num fio próprio:
// o áudio e a tela não esperam. O som ao vivo e a tomada crua não mudam.
inline void writeStreamingVersion(const juce::File& take)
{
    juce::AudioFormatManager formats;
    formats.registerBasicFormats();
    std::unique_ptr<juce::AudioFormatReader> reader(formats.createReaderFor(take));
    if (reader == nullptr || reader->lengthInSamples <= 0)
        return;
    struct ReaderSource final : aquorbium::DeliveryMaster::Source
    {
        juce::AudioFormatReader& in;
        juce::int64 position = 0;
        juce::AudioBuffer<float> buffer { 2, 8192 };
        explicit ReaderSource(juce::AudioFormatReader& r) : in(r) {}
        bool rewind() override { position = 0; return true; }
        std::size_t read(float* left, float* right, std::size_t frames) override
        {
            const auto count = static_cast<int>(std::min<juce::int64>(static_cast<juce::int64>(std::min<std::size_t>(frames, 8192)),
                                                                        in.lengthInSamples - position));
            if (count <= 0) return 0;
            if (!in.read(&buffer, 0, count, position, true, true)) return 0;
            const auto* l = buffer.getReadPointer(0);
            const auto* r = buffer.getReadPointer(in.numChannels > 1 ? 1 : 0);
            std::copy_n(l, count, left);
            std::copy_n(r, count, right);
            position += count;
            return static_cast<std::size_t>(count);
        }
    } source(*reader);
    const auto target = take.getSiblingFile(take.getFileNameWithoutExtension() + "_streaming.wav");
    juce::TemporaryFile temporary(target);
    std::unique_ptr<juce::OutputStream> stream = temporary.getFile().createOutputStream();
    if (stream == nullptr)
        return;
    juce::WavAudioFormat wav;
    std::unordered_map<juce::String, juce::String> metadata {
        { "INAM", "Aquorbium (streaming)" },
        { "IART", juce::String::fromUTF8("Aquorbium / Lúcio de Araújo") },
        { "ICMT", juce::String::fromUTF8("Versão de entrega: -14 LUFS, true peak <= -1 dBTP, 24-bit, TPDF dither") } };
    auto writer = wav.createWriterFor(stream, juce::AudioFormatWriterOptions {}
                                                  .withSampleRate(reader->sampleRate).withNumChannels(2)
                                                  .withBitsPerSample(24).withMetadataValues(metadata));
    if (writer == nullptr)
        return;
    struct WriterSink final : aquorbium::DeliveryMaster::Sink
    {
        juce::AudioFormatWriter& out;
        aquorbium::TpdfQuantiser dither { 24, 0x51ED2026u };
        std::vector<float> l = std::vector<float>(8192), r = std::vector<float>(8192);
        explicit WriterSink(juce::AudioFormatWriter& w) : out(w) {}
        bool write(const float* left, const float* right, std::size_t frames) override
        {
            for (std::size_t done = 0; done < frames;)
            {
                const auto n = std::min<std::size_t>(frames - done, l.size());
                for (std::size_t i = 0; i < n; ++i)
                {
                    l[i] = aquorbium::floatForJuceInt24(dither.quantise(left[done + i]));
                    r[i] = aquorbium::floatForJuceInt24(dither.quantise(right[done + i]));
                }
                const float* channels[] { l.data(), r.data() };
                if (!out.writeFromFloatArrays(channels, 2, static_cast<int>(n))) return false;
                done += n;
            }
            return true;
        }
    } sink(*writer);
    const auto report = aquorbium::DeliveryMaster::process(source, reader->sampleRate, sink);
    writer.reset();
    if (report.failed || !temporary.overwriteTargetFileWithTemporary())
        return;
    juce::String text;
    text << "Aquorbium - versao de entrega (streaming), saida-ref 1.2\n"
         << "tomada crua: " << take.getFileName() << "\n"
         << "alvo: -14 LUFS, true peak <= -1 dBTP; limitador no maximo 6 dB (guarda de dinamica)\n"
         << "entrada: " << juce::String(report.inputLufs, 1) << " LUFS, " << juce::String(report.inputTruePeakDb, 2) << " dBTP\n"
         << "ganho: " << juce::String(report.gainDb, 2) << " dB; limitador: " << juce::String(report.limiterReductionDb, 2) << " dB\n"
         << "saida: " << juce::String(report.outputLufs, 1) << " LUFS, " << juce::String(report.outputTruePeakDb, 2) << " dBTP\n"
         << (report.silent ? "silencio: copiada como veio\n"
             : report.reachedTarget ? "alvo atingido\n"
             : "alvo nao atingido sem esmagar a dinamica: entregue no mais alto possivel (os servicos sobem faixas baixas no modo normal)\n");
    target.withFileExtension("txt").replaceWithText(text);
}

