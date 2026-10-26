// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

// The REC take (2026-10-21): BS.1770 loudness of a reference tone, the true
// peak between samples, the guard for non-finite samples, and a recorded
// file that reads back as valid RIFF with its INFO tags and its measurement.
#include "app/DiagnosticAudioCapture.h"
#include "core/DeliveryProfiles.h"
#include "core/TakeAnalysis.h"

#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <memory>
#include <string>
#include <thread>
#include <vector>

namespace
{
int failures = 0;
void check(bool condition, const char* what)
{
    if (!condition)
    {
        std::cerr << "FAILED: " << what << '\n';
        ++failures;
    }
}
constexpr double pi = 3.14159265358979323846;
std::uint32_t le32(const std::vector<unsigned char>& bytes, std::size_t at)
{
    return static_cast<std::uint32_t>(bytes[at]) | (static_cast<std::uint32_t>(bytes[at + 1]) << 8)
        | (static_cast<std::uint32_t>(bytes[at + 2]) << 16) | (static_cast<std::uint32_t>(bytes[at + 3]) << 24);
}
}

int main()
{
    constexpr double rate = 48000.0;
    // a 997 Hz sine at -20 dBFS in both channels reads -20 LUFS (EBU Tech
    // 3341 case 1: a stereo 1 kHz sine at -23 dBFS reads -23 LUFS)
    {
        aquorbium::TakeAnalysis take;
        take.prepare(rate);
        const auto amplitude = std::pow(10.0, -20.0 / 20.0);
        for (int n = 0; n < static_cast<int>(rate * 10.0); ++n)
        {
            auto l = static_cast<float>(amplitude * std::sin(2.0 * pi * 997.0 * n / rate));
            auto r = l;
            take.push(l, r);
        }
        const auto result = take.result();
        std::cout << "997 Hz at -20 dBFS: " << result.integratedLufs << " LUFS, true peak " << result.truePeakDb
                  << " dBTP, RMS " << result.rmsDb << " dBFS\n";
        check(std::abs(result.integratedLufs - (-20.0f)) < 0.1f, "a -20 dBFS stereo sine reads -20 LUFS");
        check(std::abs(result.maxShortTermLufs - (-20.0f)) < 0.2f, "short-term agrees on a steady tone");
        check(std::abs(result.rmsDb - (-23.01f)) < 0.05f, "RMS of a sine is 3 dB under its peak");
        check(result.overFullScale == 0 && result.nonFinite == 0, "a clean tone has no overs and no invalid samples");
    }
    // LRA, EBU Tech 3342 cases 1-3: a stereo 1 kHz sine, 20 s at one level
    // then 20 s at another, reads the difference (10, 5 and 20 LU, +-1)
    {
        const auto lra = [rate](double firstDb, double secondDb) {
            aquorbium::TakeAnalysis take;
            take.prepare(rate);
            for (int n = 0; n < static_cast<int>(rate * 40.0); ++n)
            {
                const auto db = n < static_cast<int>(rate * 20.0) ? firstDb : secondDb;
                auto l = static_cast<float>(std::pow(10.0, db / 20.0) * std::sin(2.0 * pi * 1000.0 * n / rate));
                auto r = l;
                take.push(l, r);
            }
            return take.result();
        };
        const auto a = lra(-20.0, -30.0), b = lra(-20.0, -15.0), c = lra(-40.0, -20.0);
        std::cout << "LRA: " << a.loudnessRange << " / " << b.loudnessRange << " / " << c.loudnessRange
                  << " LU (EBU 3342: 10 / 5 / 20)\n";
        check(a.loudnessRangeReady && std::abs(a.loudnessRange - 10.0f) <= 1.0f, "EBU 3342 case 1: 10 LU");
        check(std::abs(b.loudnessRange - 5.0f) <= 1.0f, "EBU 3342 case 2: 5 LU");
        check(std::abs(c.loudnessRange - 20.0f) <= 1.0f, "EBU 3342 case 3: 20 LU");
    }
    // the peak between the samples: a sine at a quarter of the rate, sampled
    // 45 degrees off its crest, reads 3 dB low in samples; the true peak finds it
    {
        aquorbium::TakeAnalysis take;
        take.prepare(rate);
        for (int n = 0; n < 48000; ++n)
        {
            auto l = static_cast<float>(0.5 * std::sin(pi / 2.0 * n + pi / 4.0));
            auto r = l;
            take.push(l, r);
        }
        const auto result = take.result();
        std::cout << "fs/4 at 45 degrees: sample peak " << result.samplePeakDb << " dBFS, true peak "
                  << result.truePeakDb << " dBTP\n";
        check(std::abs(result.samplePeakDb - (-9.03f)) < 0.1f, "the samples miss the crest by 3 dB");
        check(std::abs(result.truePeakDb - (-6.02f)) < 0.3f, "the true peak finds the crest between samples");
    }
    // non-finite samples are written as digital silence and counted; full
    // scale and DC are reported
    {
        aquorbium::TakeAnalysis take;
        take.prepare(rate);
        auto l = std::numeric_limits<float>::quiet_NaN();
        auto r = std::numeric_limits<float>::infinity();
        take.push(l, r);
        check(l == 0.0f && r == 0.0f, "NaN and infinity become silence");
        for (int n = 0; n < 1000; ++n)
        {
            auto a = 1.0f, b = 0.25f;
            take.push(a, b);
        }
        const auto result = take.result();
        check(result.nonFinite == 2, "the invalid samples are counted");
        check(result.overFullScale == 1000, "samples at full scale are counted");
        check(result.dcLeft > 0.99f && std::abs(result.dcRight - 0.2498f) < 0.001f, "DC offset per channel");
    }
    // a REC take on disk: valid RIFF, float data, LIST/INFO after the data
    {
        const auto marker = std::chrono::steady_clock::now().time_since_epoch().count();
        const auto file = std::filesystem::temp_directory_path()
            / ("aquorbium_take_" + std::to_string(marker) + ".wav");
        auto rec = std::make_unique<aquorbium::app::DiagnosticAudioCapture>(
            file.string(), 48000u, 1u, aquorbium::app::DiagnosticAudioCapture::recordingSeconds);
        rec->enableAnalysis();
        rec->addInfo("INAM", "Aquorbium test take");
        rec->addInfo("ICMT", "session seed 7");  // odd length with its terminator: padded
        check(rec->start(), "the take starts");
        std::array<float, 480> left {}, right {};
        for (int block = 0; block < 100; ++block)
        {
            for (std::size_t n = 0; n < left.size(); ++n)
            {
                left[n] = static_cast<float>(0.1 * std::sin(2.0 * pi * 440.0 * static_cast<double>(block * 480 + static_cast<int>(n)) / rate));
                right[n] = left[n];
            }
            rec->push(left.data(), right.data(), left.size());
        }
        for (int wait = 0; wait < 500 && !rec->done(); ++wait)
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        rec->stop();
        check(rec->done() && !rec->failed(), "the take is written");
        const auto measured = rec->analysis();
        check(measured.frames == 48000, "the analysis saw every frame");
        check(std::abs(measured.samplePeakDb - (-20.0f)) < 0.1f, "the analysis measured the take");
        std::ifstream in(file, std::ios::binary);
        const std::vector<unsigned char> bytes((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        in.close();
        check(bytes.size() > 44 && le32(bytes, 4) == bytes.size() - 8, "the RIFF size covers data and tags");
        check(le32(bytes, 40) == 48000u * 8u, "the data chunk holds the second");
        const auto list = 44 + 48000u * 8u;
        check(bytes.size() > list + 12 && std::string(bytes.begin() + list, bytes.begin() + list + 4) == "LIST"
                  && std::string(bytes.begin() + list + 8, bytes.begin() + list + 12) == "INFO",
              "LIST/INFO follows the data");
        check(le32(bytes, list + 4) == bytes.size() - list - 8, "the LIST size is exact");
        check(std::string(bytes.begin(), bytes.end()).find("Aquorbium test take") != std::string::npos,
              "the title is in the file");
        std::filesystem::remove(file);
    }
    // a 24-bit PCM take: PCM header, 6 bytes a frame, TPDF dither of +-1 LSB
    // on silence, the level kept, the measurement on the float master
    {
        const auto marker = std::chrono::steady_clock::now().time_since_epoch().count();
        const auto file = std::filesystem::temp_directory_path()
            / ("aquorbium_take24_" + std::to_string(marker) + ".wav");
        auto rec = std::make_unique<aquorbium::app::DiagnosticAudioCapture>(
            file.string(), 48000u, 1u, aquorbium::app::DiagnosticAudioCapture::recordingSeconds);
        rec->enableAnalysis();
        rec->usePcm24();
        check(rec->start(), "the 24-bit take starts");
        std::array<float, 480> left {}, right {};
        for (int block = 0; block < 100; ++block)
        {
            for (std::size_t n = 0; n < left.size(); ++n)
            {
                left[n] = block < 50 ? 0.0f : 0.5f;  // half silence, half a constant 0.5
                right[n] = block < 50 ? 0.0f : -0.25f;
            }
            if (block == 75)
                left[0] = std::numeric_limits<float>::quiet_NaN();
            rec->push(left.data(), right.data(), left.size());
        }
        for (int wait = 0; wait < 500 && !rec->done(); ++wait)
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        rec->stop();
        check(rec->done() && !rec->failed(), "the 24-bit take is written");
        std::ifstream in(file, std::ios::binary);
        const std::vector<unsigned char> bytes((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        in.close();
        const auto le16 = [&](std::size_t at) { return static_cast<unsigned>(bytes[at] | (bytes[at + 1] << 8)); };
        check(bytes.size() >= 44 && le16(20) == 1 && le16(22) == 2 && le16(32) == 6 && le16(34) == 24
                  && le32(bytes, 28) == 48000u * 6u,
              "a PCM 24-bit stereo header");
        check(le32(bytes, 40) == 48000u * 6u && le32(bytes, 4) == bytes.size() - 8, "6 bytes a frame, sizes exact");
        const auto sample = [&](std::size_t frame, std::size_t channel) {
            const auto at = 44 + frame * 6 + channel * 3;
            auto v = static_cast<std::int32_t>(bytes[at] | (bytes[at + 1] << 8) | (bytes[at + 2] << 16));
            if (v & 0x800000)
                v -= 0x1000000;
            return v;
        };
        bool silenceDithered = true, levelKept = true;
        int nonZero = 0;
        for (std::size_t f = 0; f < 24000; ++f)
        {
            const auto v = sample(f, 0);
            silenceDithered = silenceDithered && std::abs(v) <= 1;
            nonZero += v != 0 ? 1 : 0;
        }
        double sumLeft = 0.0;
        for (std::size_t f = 24000; f < 48000; ++f)
        {
            if (f == 36000)
                continue;  // the NaN
            sumLeft += sample(f, 0);
            levelKept = levelKept && std::abs(sample(f, 1) - (-2097152)) <= 2;
        }
        const auto meanLeft = sumLeft / 23999.0;
        std::cout << "PCM24: silence non-zero " << nonZero * 100.0 / 24000.0 << "% (TPDF ~25%), 0.5 reads "
                  << meanLeft << " (4194303.5)\n";
        check(silenceDithered, "dithered silence stays within one LSB");
        check(nonZero > 24000 / 6 && nonZero < 24000 / 3, "TPDF on silence: about a quarter of the samples move");
        check(std::abs(meanLeft - 4194303.5) < 0.05 && levelKept, "the level is kept exactly on average");
        check(sample(36000, 0) >= -1 && sample(36000, 0) <= 1, "NaN is written as silence");
        check(rec->analysis().nonFinite == 1, "and counted on the float master");
        std::filesystem::remove(file);
    }
    // 16-bit PCM (2026-10-22): its header, TPDF at its own LSB, and the
    // measurement of the ENCODED take equal to the file read back from disk
    for (const auto depth : { 16u, 24u })
    {
        const auto marker = std::chrono::steady_clock::now().time_since_epoch().count();
        const auto file = std::filesystem::temp_directory_path()
            / ("aquorbium_take" + std::to_string(depth) + "e_" + std::to_string(marker) + ".wav");
        auto rec = std::make_unique<aquorbium::app::DiagnosticAudioCapture>(
            file.string(), 48000u, 2u, aquorbium::app::DiagnosticAudioCapture::recordingSeconds);
        rec->enableAnalysis();
        if (depth == 16)
            rec->usePcm16();
        else
            rec->usePcm24();
        check(rec->start(), "the PCM take starts");
        std::array<float, 480> left {}, right {};
        std::size_t n = 0;
        for (int block = 0; block < 200; ++block)
        {
            for (std::size_t i = 0; i < left.size(); ++i, ++n)
            {
                // a 997 Hz sine at -20 dBFS; the first 100 ms silent
                const auto x = n < 4800 ? 0.0f : 0.1f * static_cast<float>(std::sin(2.0 * 3.14159265358979 * 997.0 * n / 48000.0));
                left[i] = x;
                right[i] = -x;
            }
            rec->push(left.data(), right.data(), left.size());
        }
        for (int wait = 0; wait < 500 && !rec->done(); ++wait)
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        rec->stop();
        check(rec->done() && !rec->failed(), "the PCM take is written");
        std::ifstream in(file, std::ios::binary);
        const std::vector<unsigned char> bytes((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        in.close();
        const auto le16 = [&](std::size_t at) { return static_cast<unsigned>(bytes[at] | (bytes[at + 1] << 8)); };
        const auto frameBytes = 2u * depth / 8u;
        check(bytes.size() >= 44 && le16(20) == 1 && le16(32) == frameBytes && le16(34) == depth
                  && le32(bytes, 40) == 96000u * frameBytes,
              "a PCM header of the chosen depth, the sizes exact");
        // read the file back and measure it independently
        const auto full = static_cast<float>((1l << (depth - 1)) - 1);
        const auto sample = [&](std::size_t frame, std::size_t channel) {
            const auto at = 44 + frame * frameBytes + channel * (depth / 8u);
            std::int32_t v = 0;
            for (unsigned b = 0; b < depth / 8u; ++b)
                v |= static_cast<std::int32_t>(bytes[at + b]) << (8 * b);
            if (v & (1 << (depth - 1)))
                v -= 1 << depth;
            return v;
        };
        aquorbium::TakeAnalysis reread;
        reread.prepare(48000.0);
        bool silenceDithered = true;
        for (std::size_t f = 0; f < 96000; ++f)
        {
            auto l = static_cast<float>(sample(f, 0)) / full, r = static_cast<float>(sample(f, 1)) / full;
            reread.push(l, r);
            if (f < 4800)
                silenceDithered = silenceDithered && std::abs(sample(f, 0)) <= 1;
        }
        const auto fromDisk = reread.result();
        const auto encoded = rec->encodedAnalysis();
        const auto master = rec->analysis();
        std::cout << "PCM" << depth << ": master " << master.integratedLufs << " LUFS / " << master.truePeakDb
                  << " dBTP; encoded " << encoded.integratedLufs << " / " << encoded.truePeakDb << "; read back "
                  << fromDisk.integratedLufs << " / " << fromDisk.truePeakDb << '\n';
        check(silenceDithered, "dithered silence within one LSB of the chosen depth");
        check(std::abs(encoded.truePeakDb - fromDisk.truePeakDb) < 0.01f
                  && std::abs(encoded.integratedLufs - fromDisk.integratedLufs) < 0.01f
                  && std::abs(encoded.samplePeakDb - fromDisk.samplePeakDb) < 0.01f,
              "the encoded measurement is the file read back from disk");
        check(std::abs(encoded.integratedLufs - master.integratedLufs) < 0.05f
                  && std::abs(encoded.truePeakDb - master.truePeakDb) < 0.05f,
              "the encoding keeps the loudness and the true peak");
        std::filesystem::remove(file);
    }
    // a float take: the encoded measurement is the master's
    {
        const auto marker = std::chrono::steady_clock::now().time_since_epoch().count();
        const auto file = std::filesystem::temp_directory_path() / ("aquorbium_takefe_" + std::to_string(marker) + ".wav");
        auto rec = std::make_unique<aquorbium::app::DiagnosticAudioCapture>(
            file.string(), 48000u, 1u, aquorbium::app::DiagnosticAudioCapture::recordingSeconds);
        rec->enableAnalysis();
        check(rec->start(), "the float take starts");
        std::array<float, 480> left {}, right {};
        for (int block = 0; block < 100; ++block)
        {
            left.fill(0.2f);
            right.fill(-0.2f);
            rec->push(left.data(), right.data(), left.size());
        }
        for (int wait = 0; wait < 500 && !rec->done(); ++wait)
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        rec->stop();
        check(rec->bitsPerSample() == 32 && rec->encodedAnalysis().samplePeakDb == rec->analysis().samplePeakDb,
              "a float take: the encoded measurement is the master's");
        std::filesystem::remove(file);
    }
    // delivery profiles (2026-10-23): a report, the numbers of the sources
    {
        using aquorbium::deliveryProfiles;
        using aquorbium::judgeDelivery;
        const auto& normal = deliveryProfiles[0];
        const auto& quiet = deliveryProfiles[1];
        const auto& ebu = deliveryProfiles[2];
        // Spotify's own example: -20 LUFS peaking at -5 dBTP is raised only to -16 LUFS
        const auto example = judgeDelivery(normal, -20.0f, -5.0f);
        check(std::abs(example.appliedGainDb - 4.0f) < 1.0e-4f && std::abs(example.resultingLufs + 16.0f) < 1.0e-4f
                  && !example.reachesTarget && example.truePeakWithinCeiling,
              "delivery: Spotify's example, -20 LUFS / -5 dBTP raised to -16 LUFS");
        check(std::abs(example.gainToTargetDb - 6.0f) < 1.0e-4f, "delivery: the gain to the target is still reported");
        // a loud take is turned down in full, whatever its peak
        const auto loud = judgeDelivery(normal, -9.0f, -0.3f);
        check(std::abs(loud.appliedGainDb + 5.0f) < 1.0e-4f && loud.reachesTarget && loud.truePeakWithinCeiling,
              "delivery: a loud take is turned down to -14 LUFS");
        const auto soft = judgeDelivery(quiet, -25.0f, -10.0f);
        check(soft.reachesTarget && std::abs(soft.resultingTruePeakDb + 4.0f) < 1.0e-4f,
              "delivery: a soft take with headroom reaches -19 LUFS");
        // a take already over its ceiling is never turned up
        const auto clipped = judgeDelivery(normal, -20.0f, 0.5f);
        check(clipped.appliedGainDb == 0.0f && !clipped.truePeakWithinCeiling,
              "delivery: no positive gain past the ceiling");
        // R128 applies the whole gain and reports the peak that results
        const auto broadcast = judgeDelivery(ebu, -30.0f, -6.0f);
        check(broadcast.reachesTarget && std::abs(broadcast.resultingTruePeakDb - 1.0f) < 1.0e-4f
                  && !broadcast.truePeakWithinCeiling,
              "delivery: R128 gain applied in full, peak over -1 dBTP flagged");
        check(aquorbium::spotifyMasterCeilingDbtp(-12.0f) == -2.0f && aquorbium::spotifyMasterCeilingDbtp(-14.0f) == -1.0f,
              "delivery: Spotify's master ceiling, -2 dBTP above -14 LUFS");
    }

    if (failures == 0)
        std::cout << "take analysis: all checks passed\n";
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
