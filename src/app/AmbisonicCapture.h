// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include <algorithm>
#include <array>
#include <atomic>
#include <bit>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <limits>
#include <memory>
#include <string>
#include <thread>

namespace aquorbium::app
{
// The REC's first-order Ambisonics file (2026-10-23): AmbiX, four channels
// in ACN order (W, Y, Z, X) with SN3D, 32-bit float, WAVE_FORMAT_EXTENSIBLE
// with a zero channel mask (the channels are not loudspeakers) and a fact
// chunk. The same design as DiagnosticAudioCapture: the audio callback only
// copies into a lock-free ring; a writer thread owns the file, created
// exclusively (never overwriting a take), with the header refreshed every few
// seconds so a crash still leaves a readable file.
class AmbisonicCapture final
{
public:
    static constexpr std::size_t channels = 4;
    static constexpr std::size_t ringFrames = 1u << 16;
    static constexpr std::uint32_t frameBytes = channels * 4u;
    static constexpr std::size_t headerBytes = 80;

    AmbisonicCapture(std::string path, unsigned rate, unsigned seconds)
        : filePath(std::move(path)), sampleRate(rate), durationSeconds(seconds),
          frameLimit(std::min<std::uint64_t>(static_cast<std::uint64_t>(rate) * seconds,
                                             (0xffffffffull - 4096ull) / frameBytes)),
          ring(std::make_unique<std::array<std::array<float, channels>, ringFrames>>()) {}
    AmbisonicCapture(const AmbisonicCapture&) = delete;
    AmbisonicCapture& operator=(const AmbisonicCapture&) = delete;
    ~AmbisonicCapture() { stop(); }

    bool start()
    {
        if (worker.joinable() || sampleRate < 8000 || sampleRate > 192000 || durationSeconds < 1)
            return false;
        file = std::fopen(filePath.c_str(), "wbx");
        if (file == nullptr)
            return false;
        if (!writeHeader(0))
        {
            std::fclose(file);
            file = nullptr;
            std::remove(filePath.c_str());
            return false;
        }
        wallDeadline = std::chrono::steady_clock::now() + std::chrono::seconds(durationSeconds + 2);
        try { worker = std::thread([this] { writerLoop(); }); }
        catch (...)
        {
            std::fclose(file);
            file = nullptr;
            std::remove(filePath.c_str());
            return false;
        }
        return true;
    }

    // audio callback only: never waits, allocates or touches the file; a full
    // ring drops the whole block, counted
    void push(const float* w, const float* y, const float* z, const float* x, std::size_t frames) noexcept
    {
        if (finished.load(std::memory_order_relaxed) || w == nullptr || y == nullptr || z == nullptr || x == nullptr)
            return;
        const auto elapsed = elapsedFrames.load(std::memory_order_relaxed);
        const auto count = static_cast<std::size_t>(std::min<std::uint64_t>(
            frames, frameLimit - std::min(elapsed, frameLimit)));
        if (count == 0) { finished.store(true, std::memory_order_release); return; }
        const auto write = writeIndex.load(std::memory_order_relaxed);
        const auto read = readIndex.load(std::memory_order_acquire);
        if (count > ringFrames - static_cast<std::size_t>(write - read))
            droppedFrames.fetch_add(count, std::memory_order_relaxed);
        else
        {
            for (std::size_t i = 0; i < count; ++i)
                (*ring)[(write + i) & (ringFrames - 1)] = { w[i], y[i], z[i], x[i] };
            writeIndex.store(write + count, std::memory_order_release);
        }
        elapsedFrames.store(elapsed + count, std::memory_order_relaxed);
        if (elapsed + count >= frameLimit)
            finished.store(true, std::memory_order_release);
    }

    void stop() noexcept
    {
        finished.store(true, std::memory_order_release);
        if (worker.joinable()) worker.join();
    }
    [[nodiscard]] bool done() const noexcept { return writerDone.load(std::memory_order_acquire); }
    [[nodiscard]] bool failed() const noexcept { return writeFailed.load(std::memory_order_acquire); }
    [[nodiscard]] std::uint64_t captured() const noexcept { return writtenFrames.load(std::memory_order_acquire); }
    [[nodiscard]] std::uint64_t dropped() const noexcept { return droppedFrames.load(std::memory_order_relaxed); }

private:
    static_assert(std::endian::native == std::endian::little);
    static_assert(sizeof(float) == 4 && std::numeric_limits<float>::is_iec559);

    bool writeHeader(std::uint32_t frames) noexcept
    {
        std::array<unsigned char, headerBytes> h {};
        const auto tag = [&h](std::size_t at, const char* text) {
            for (std::size_t i = 0; i < 4; ++i) h[at + i] = static_cast<unsigned char>(text[i]);
        };
        const auto put = [&h](std::size_t at, std::uint32_t value, std::size_t bytes) {
            for (std::size_t i = 0; i < bytes; ++i) h[at + i] = static_cast<unsigned char>(value >> (8 * i));
        };
        const auto dataBytes = frames * frameBytes;
        tag(0, "RIFF");
        put(4, static_cast<std::uint32_t>(headerBytes - 8) + dataBytes, 4);
        tag(8, "WAVE");
        tag(12, "fmt ");
        put(16, 40, 4);
        put(20, 0xfffe, 2);                 // WAVE_FORMAT_EXTENSIBLE
        put(22, channels, 2);
        put(24, sampleRate, 4);
        put(28, sampleRate * frameBytes, 4);
        put(32, frameBytes, 2);
        put(34, 32, 2);
        put(36, 22, 2);                     // extension size
        put(38, 32, 2);                     // valid bits
        put(40, 0, 4);                      // channel mask: not loudspeakers
        // SubFormat: KSDATAFORMAT_SUBTYPE_IEEE_FLOAT
        static constexpr std::array<unsigned char, 16> ieeeFloat {
            0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00,
            0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71 };
        std::copy(ieeeFloat.begin(), ieeeFloat.end(), h.begin() + 44);
        tag(60, "fact");
        put(64, 4, 4);
        put(68, frames, 4);
        tag(72, "data");
        put(76, dataBytes, 4);
        return std::fseek(file, 0, SEEK_SET) == 0 && std::fwrite(h.data(), 1, h.size(), file) == h.size();
    }

    void writerLoop() noexcept
    {
        std::array<std::array<float, channels>, 1024> scratch {};
        std::uint64_t total = 0, lastHeader = 0;
        for (;;)
        {
            const auto read = readIndex.load(std::memory_order_relaxed);
            const auto write = writeIndex.load(std::memory_order_acquire);
            const auto count = static_cast<std::size_t>(std::min<std::uint64_t>(scratch.size(), write - read));
            if (count == 0)
            {
                if (finished.load(std::memory_order_acquire)) break;
                if (std::chrono::steady_clock::now() >= wallDeadline)
                {
                    finished.store(true, std::memory_order_release);
                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
                continue;
            }
            for (std::size_t i = 0; i < count; ++i)
            {
                scratch[i] = (*ring)[(read + i) & (ringFrames - 1)];
                for (auto& value : scratch[i])
                    if (!std::isfinite(value))
                        value = 0.0f;  // digital silence, as in the stereo take
            }
            if (std::fwrite(scratch.data(), frameBytes, count, file) != count)
            {
                writeFailed.store(true, std::memory_order_release);
                finished.store(true, std::memory_order_release);
                break;
            }
            total += count;
            readIndex.store(read + count, std::memory_order_release);
            if (total - lastHeader >= static_cast<std::uint64_t>(sampleRate) * 4u)
            {
                lastHeader = total;
                if (!writeHeader(static_cast<std::uint32_t>(total)) || std::fseek(file, 0, SEEK_END) != 0)
                {
                    writeFailed.store(true, std::memory_order_release);
                    finished.store(true, std::memory_order_release);
                    break;
                }
            }
        }
        if (!writeHeader(static_cast<std::uint32_t>(total)) || std::fflush(file) != 0)
            writeFailed.store(true, std::memory_order_release);
        if (std::fclose(file) != 0)
            writeFailed.store(true, std::memory_order_release);
        file = nullptr;
        writtenFrames.store(total, std::memory_order_release);
        writerDone.store(true, std::memory_order_release);
    }

    std::string filePath;
    unsigned sampleRate = 0, durationSeconds = 0;
    std::uint64_t frameLimit = 0;
    std::unique_ptr<std::array<std::array<float, channels>, ringFrames>> ring;
    std::FILE* file = nullptr;
    std::thread worker;
    std::chrono::steady_clock::time_point wallDeadline {};
    std::atomic<std::uint64_t> writeIndex { 0 }, readIndex { 0 };
    std::atomic<std::uint64_t> droppedFrames { 0 }, elapsedFrames { 0 }, writtenFrames { 0 };
    std::atomic<bool> finished { false }, writerDone { false }, writeFailed { false };
};
}
