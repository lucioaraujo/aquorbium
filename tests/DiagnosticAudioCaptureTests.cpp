// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "app/DiagnosticAudioCapture.h"

#include <array>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

namespace
{
std::uint32_t read32(const std::array<unsigned char, 44>& bytes, std::size_t at)
{
    return static_cast<std::uint32_t>(bytes[at])
        | (static_cast<std::uint32_t>(bytes[at + 1]) << 8)
        | (static_cast<std::uint32_t>(bytes[at + 2]) << 16)
        | (static_cast<std::uint32_t>(bytes[at + 3]) << 24);
}
}

int main()
{
    const auto marker = std::chrono::steady_clock::now().time_since_epoch().count();
    const auto file = std::filesystem::temp_directory_path()
        / ("aquorbium_diagnostic_capture_" + std::to_string(marker) + ".wav");
    const auto fail = [&](const char* reason) {
        std::cerr << reason << '\n';
        return 1;
    };
    auto capture = std::make_unique<aquorbium::app::DiagnosticAudioCapture>(
        file.string(), 48000u, 1u);
    if (!capture->start()) return fail("capture did not start");
    std::array<float, 512> left {}, right {};
    for (std::size_t index = 0; index < left.size(); ++index)
    {
        left[index] = static_cast<float>(index) / 1024.0f;
        right[index] = -left[index];
    }
    for (unsigned block = 0; block < 94; ++block)
        capture->push(left.data(), right.data(), left.size());
    capture->stop();
    if (!capture->done() || capture->failed() || capture->dropped() != 0
        || capture->elapsed() != 48000 || capture->captured() != 48000)
        return fail("capture counters invalid");

    std::ifstream input(file, std::ios::binary);
    std::array<unsigned char, 44> header {};
    input.read(reinterpret_cast<char*>(header.data()), header.size());
    if (!input || std::memcmp(header.data(), "RIFF", 4) != 0
        || std::memcmp(header.data() + 8, "WAVEfmt ", 8) != 0
        || std::memcmp(header.data() + 36, "data", 4) != 0
        || header[20] != 3 || header[22] != 2
        || read32(header, 24) != 48000 || read32(header, 40) != 48000u * 8u)
        return fail("WAV header invalid");
    std::array<float, 2> first {};
    input.read(reinterpret_cast<char*>(first.data()), sizeof(first));
    std::array<float, 2> secondFrame {};
    input.read(reinterpret_cast<char*>(secondFrame.data()), sizeof(secondFrame));
    if (!input || first[0] != 0.0f || first[1] != 0.0f
        || secondFrame[0] != 1.0f / 1024.0f || secondFrame[1] != -1.0f / 1024.0f
        || std::filesystem::file_size(file) != 44u + 48000u * 8u)
        return fail("WAV payload invalid");
    input.close();
    auto second = std::make_unique<aquorbium::app::DiagnosticAudioCapture>(
        file.string(), 48000u, 1u);
    if (second->start()) return fail("existing WAV was overwritten");
    std::filesystem::remove(file);

    // If the audio callback disappears, the writer must still finish the WAV
    // before the external process timeout, without depending on the GUI timer.
    const auto stalledFile = file.string() + ".stalled.wav";
    auto stalled = std::make_unique<aquorbium::app::DiagnosticAudioCapture>(
        stalledFile, 48000u, 1u);
    if (!stalled->start()) return fail("stalled capture did not start");
    stalled->push(left.data(), right.data(), left.size());
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    while (!stalled->done() && std::chrono::steady_clock::now() < deadline)
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    if (!stalled->done()) return fail("stalled capture did not finish by wall deadline");
    stalled->stop();
    if (!stalled->stoppedByWallDeadline() || stalled->failed()
        || stalled->captured() != left.size())
        return fail("stalled capture status invalid");
    std::ifstream stalledInput(stalledFile, std::ios::binary);
    stalledInput.read(reinterpret_cast<char*>(header.data()), header.size());
    if (!stalledInput || read32(header, 40) != left.size() * 8u
        || std::filesystem::file_size(stalledFile) != 44u + left.size() * 8u)
        return fail("stalled WAV was not finalized");
    stalledInput.close();
    std::filesystem::remove(stalledFile);
    return 0;
}
