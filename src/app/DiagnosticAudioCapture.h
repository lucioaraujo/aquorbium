// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include "core/TakeAnalysis.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <bit>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <initializer_list>
#include <limits>
#include <string>
#include <thread>
#include <utility>

namespace aquorbium::app
{
// One audio producer, one disk writer. The audio thread only copies into a
// bounded ring; all file I/O and header updates happen on the writer thread.
class DiagnosticAudioCapture final
{
public:
    static constexpr std::size_t ringFrames = 1u << 17;
    static constexpr unsigned maximumSeconds = 120;
    // the REC button (2026-10-21): an hour at most, 1.4 GB at 48 kHz
    static constexpr unsigned recordingSeconds = 3600;

    DiagnosticAudioCapture(std::string path, unsigned rate, unsigned seconds, unsigned limit = maximumSeconds)
        : filePath(std::move(path)), sampleRate(rate), durationSeconds(seconds),
          secondsLimit(std::min(limit, recordingSeconds)),
          // a RIFF data chunk holds at most 4 GiB (8 bytes a frame)
          frameLimit(std::min<std::uint64_t>(static_cast<std::uint64_t>(rate) * seconds,
                                             (0xffffffffull - 4096ull) / 8ull)) {}

    // ---- the REC take (2026-10-21), set before start() ----
    // measure the take on the writer thread (non-finite samples become
    // digital silence) - see core/TakeAnalysis.h
    void enableAnalysis() noexcept { analysing = true; }
    // RIFF LIST/INFO tags written after the data (INAM, IART, ICRD, ISFT,
    // ICMT...): four-character id and an ASCII/UTF-8 value
    void addInfo(const char* id, std::string value)
    {
        if (infoCount < info.size() && id != nullptr && std::char_traits<char>::length(id) == 4)
            info[infoCount++] = { std::string(id), std::move(value) };
    }
    // valid after done()
    [[nodiscard]] aquorbium::TakeAnalysis::Result analysis() const noexcept { return analysisResult; }
    // write 24-bit PCM with TPDF dither instead of 32-bit float (set before
    // start()). As in Navalha 2 (docs/DITHER_TPDF.md): dither belongs to the
    // integer encoding, never to the float bus; two uniform values from a
    // xorshift32 sequence are subtracted per sample (triangular, +-1 LSB),
    // left and right taking successive values; non-finite samples become
    // digital silence. The measurement stays on the float master.
    void usePcm24() noexcept { useFixedPoint(24); }
    // 16-bit PCM (2026-10-22), dithered the same way at its own LSB
    void usePcm16() noexcept { useFixedPoint(16); }
    [[nodiscard]] bool isPcm24() const noexcept { return bits == 24; }
    [[nodiscard]] bool isPcm16() const noexcept { return bits == 16; }
    [[nodiscard]] unsigned bitsPerSample() const noexcept { return bits; }
    // the take measured AFTER the encoding (2026-10-22): the very integers
    // written to the file, read back as floats. Equal to analysis() for a
    // float take; for PCM it shows what the dither and the rounding did to
    // the true peak and the loudness. Valid after done().
    [[nodiscard]] aquorbium::TakeAnalysis::Result encodedAnalysis() const noexcept
    {
        return fixedPoint() ? encodedResult : analysisResult;
    }

    DiagnosticAudioCapture(const DiagnosticAudioCapture&) = delete;
    DiagnosticAudioCapture& operator=(const DiagnosticAudioCapture&) = delete;

    ~DiagnosticAudioCapture() { stop(); }

    bool start()
    {
        if (worker.joinable() || sampleRate < 8000 || sampleRate > 192000
            || durationSeconds < 1 || durationSeconds > secondsLimit)
            return false;
        // C11 exclusive-create flag: never truncate a previous recording.
        file = std::fopen(filePath.c_str(), "wbx");
        if (file == nullptr) return false;
        if (!writeHeader(0))
        {
            std::fclose(file);
            file = nullptr;
            std::remove(filePath.c_str()); // only the newly-created empty file
            return false;
        }
        wallDeadline = std::chrono::steady_clock::now()
            + std::chrono::seconds(durationSeconds + 2);
        try { worker = std::thread([this] { writerLoop(); }); }
        catch (...)
        {
            std::fclose(file);
            file = nullptr;
            std::remove(filePath.c_str()); // no writer was started
            return false;
        }
        return true;
    }

    // Called only from the audio callback, after render. Never waits, allocates
    // or touches the FILE. A full ring drops an entire block, counted explicitly.
    void push(const float* left, const float* right, std::size_t frames) noexcept
    {
        if (finished.load(std::memory_order_relaxed) || left == nullptr || right == nullptr)
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
            for (std::size_t index = 0; index < count; ++index)
                ring[(write + index) & (ringFrames - 1)] = { left[index], right[index] };
            writeIndex.store(write + count, std::memory_order_release);
        }
        elapsedFrames.store(elapsed + count, std::memory_order_relaxed);
        if (elapsed + count >= frameLimit)
            finished.store(true, std::memory_order_release);
    }

    // Safe from the GUI or destructor, never from the audio callback.
    void stop() noexcept
    {
        requestStop();
        if (worker.joinable()) worker.join();
    }

    // A sample-rate change can be reported by the audio callback. The GUI
    // later joins the writer; the callback itself must never wait for disk.
    void requestStop() noexcept { finished.store(true, std::memory_order_release); }

    [[nodiscard]] bool done() const noexcept { return writerDone.load(std::memory_order_acquire); }
    [[nodiscard]] bool failed() const noexcept { return writeFailed.load(std::memory_order_acquire); }
    [[nodiscard]] bool stoppedByWallDeadline() const noexcept
    {
        return wallDeadlineReached.load(std::memory_order_acquire);
    }
    [[nodiscard]] std::uint64_t captured() const noexcept { return writtenFrames.load(std::memory_order_acquire); }
    [[nodiscard]] std::uint64_t dropped() const noexcept { return droppedFrames.load(std::memory_order_relaxed); }
    [[nodiscard]] std::uint64_t elapsed() const noexcept { return elapsedFrames.load(std::memory_order_relaxed); }
    [[nodiscard]] unsigned rate() const noexcept { return sampleRate; }
    [[nodiscard]] unsigned requestedSeconds() const noexcept { return durationSeconds; }

private:
    static_assert(std::endian::native == std::endian::little);
    static_assert(sizeof(float) == 4 && std::numeric_limits<float>::is_iec559);
    static_assert(std::atomic<std::uint64_t>::is_always_lock_free);

    static void put16(std::array<unsigned char, 44>& bytes, std::size_t offset,
                      std::uint16_t value) noexcept
    {
        for (unsigned index = 0; index < 2; ++index)
            bytes[offset + index] = static_cast<unsigned char>(value >> (index * 8));
    }
    static void put32(std::array<unsigned char, 44>& bytes, std::size_t offset,
                      std::uint32_t value) noexcept
    {
        for (unsigned index = 0; index < 4; ++index)
            bytes[offset + index] = static_cast<unsigned char>(value >> (index * 8));
    }
    bool writeHeader(std::uint32_t dataBytes, std::uint32_t trailingBytes = 0) noexcept
    {
        std::array<unsigned char, 44> header {};
        for (const auto& [offset, tag] : { std::pair { 0u, "RIFF" }, std::pair { 8u, "WAVE" },
                                            std::pair { 12u, "fmt " }, std::pair { 36u, "data" } })
            for (unsigned index = 0; index < 4; ++index) header[offset + index] = static_cast<unsigned char>(tag[index]);
        put32(header, 4, dataBytes + 36 + trailingBytes);
        put32(header, 16, 16); // PCM-style fmt body
        put16(header, 20, fixedPoint() ? 1 : 3);  // PCM or IEEE float, interleaved stereo
        put16(header, 22, 2);
        put32(header, 24, sampleRate);
        put32(header, 28, sampleRate * frameBytes());
        put16(header, 32, static_cast<std::uint16_t>(frameBytes()));
        put16(header, 34, static_cast<std::uint16_t>(bits));
        put32(header, 40, dataBytes);
        return std::fseek(file, 0, SEEK_SET) == 0
            && std::fwrite(header.data(), 1, header.size(), file) == header.size();
    }
    // appends the LIST/INFO chunk after the data; returns its size in bytes
    // (0 when there is nothing to write or the write failed)
    std::uint32_t writeInfo(std::uint64_t dataBytes) noexcept
    {
        if (infoCount == 0 || std::fseek(file, 0, SEEK_END) != 0)
            return 0;
        std::uint32_t written = 0;
        const auto put = [this, &written](const void* data, std::size_t size) {
            if (std::fwrite(data, 1, size, file) != size)
                return false;
            written += static_cast<std::uint32_t>(size);
            return true;
        };
        const auto le32 = [](std::uint32_t value) {
            std::array<unsigned char, 4> bytes {};
            for (unsigned index = 0; index < 4; ++index)
                bytes[index] = static_cast<unsigned char>(value >> (index * 8));
            return bytes;
        };
        if ((dataBytes & 1u) != 0)  // chunks start on even offsets
        {
            const unsigned char zero = 0;
            if (!put(&zero, 1)) return 0;
        }
        std::uint32_t body = 4;  // "INFO"
        for (std::size_t index = 0; index < infoCount; ++index)
        {
            const auto size = static_cast<std::uint32_t>(info[index].second.size() + 1);
            body += 8 + size + (size & 1u);
        }
        const auto listSize = le32(body);
        if (!put("LIST", 4) || !put(listSize.data(), 4) || !put("INFO", 4))
            return 0;
        for (std::size_t index = 0; index < infoCount; ++index)
        {
            const auto& [id, value] = info[index];
            const auto size = static_cast<std::uint32_t>(value.size() + 1);
            const auto sizeBytes = le32(size);
            const char pad[2] = { 0, 0 };
            if (!put(id.data(), 4) || !put(sizeBytes.data(), 4) || !put(value.c_str(), size)
                || ((size & 1u) != 0 && !put(pad, 1)))
                return 0;
        }
        return written;
    }

    void useFixedPoint(unsigned depth) noexcept
    {
        bits = depth;
        frameLimit = std::min<std::uint64_t>(frameLimit, (0xffffffffull - 4096ull) / frameBytes());
    }
    [[nodiscard]] bool fixedPoint() const noexcept { return bits != 32; }
    [[nodiscard]] std::uint32_t frameBytes() const noexcept { return 2u * bits / 8u; }

    float uniform() noexcept
    {
        ditherState ^= ditherState << 13;
        ditherState ^= ditherState >> 17;
        ditherState ^= ditherState << 5;
        return static_cast<float>(ditherState) / 4294967296.0f;
    }

    // float frames to 16- or 24-bit little-endian PCM, TPDF dithered at that
    // depth's LSB; the integers written are also measured (encodedAnalysis)
    const unsigned char* pack(const std::array<std::array<float, 2>, 1024>& frames, std::size_t count) noexcept
    {
        const auto bytesPerSample = bits / 8u;
        const auto full = static_cast<long>((1l << (bits - 1)) - 1);  // 32767 or 8388607
        const auto scale = static_cast<float>(full);
        for (std::size_t index = 0; index < count; ++index)
        {
            std::array<float, 2> decoded {};
            for (std::size_t channel = 0; channel < 2; ++channel)
            {
                auto x = frames[index][channel];
                if (!std::isfinite(x))
                    x = 0.0f;
                const auto dither = uniform() - uniform();
                const auto value = static_cast<std::int32_t>(std::clamp(
                    std::lround(static_cast<double>(x) * scale + dither), -full - 1, full));
                auto* out = &packed[(index * 2 + channel) * bytesPerSample];
                for (unsigned b = 0; b < bytesPerSample; ++b)
                    out[b] = static_cast<unsigned char>((value >> (8 * b)) & 0xff);
                decoded[channel] = static_cast<float>(value) / scale;
            }
            if (analysing)
                encodedAnalysis_.push(decoded[0], decoded[1]);
        }
        return packed.data();
    }

    void writerLoop() noexcept
    {
        if (analysing)
        {
            takeAnalysis.prepare(static_cast<double>(sampleRate));
            encodedAnalysis_.prepare(static_cast<double>(sampleRate));
        }
        std::array<std::array<float, 2>, 1024> scratch {};
        std::uint64_t total = 0;
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
                    wallDeadlineReached.store(true, std::memory_order_release);
                    finished.store(true, std::memory_order_release);
                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
                continue;
            }
            for (std::size_t index = 0; index < count; ++index)
                scratch[index] = ring[(read + index) & (ringFrames - 1)];
            if (analysing)
                for (std::size_t index = 0; index < count; ++index)
                    takeAnalysis.push(scratch[index][0], scratch[index][1]);
            const auto written = fixedPoint() ? std::fwrite(pack(scratch, count), frameBytes(), count, file)
                                              : std::fwrite(scratch.data(), 8, count, file);
            if (written != count)
            {
                writeFailed.store(true, std::memory_order_release);
                finished.store(true, std::memory_order_release);
                break;
            }
            total += count;
            readIndex.store(read + count, std::memory_order_release);
            // keep the header valid every few seconds, so that a crash or a
            // power cut leaves a playable file
            if (total - lastHeaderFrames >= static_cast<std::uint64_t>(sampleRate) * 4u)
            {
                lastHeaderFrames = total;
                if (!writeHeader(static_cast<std::uint32_t>(total * frameBytes())) || std::fseek(file, 0, SEEK_END) != 0)
                {
                    writeFailed.store(true, std::memory_order_release);
                    finished.store(true, std::memory_order_release);
                    break;
                }
            }
        }
        if (analysing)
        {
            analysisResult = takeAnalysis.result();
            encodedResult = encodedAnalysis_.result();
        }
        const auto tags = writeInfo(total * frameBytes());
        if (!writeHeader(static_cast<std::uint32_t>(total * frameBytes()), tags) || std::fflush(file) != 0)
            writeFailed.store(true, std::memory_order_release);
        if (std::fclose(file) != 0)
            writeFailed.store(true, std::memory_order_release);
        file = nullptr;
        writtenFrames.store(total, std::memory_order_release);
        writerDone.store(true, std::memory_order_release);
    }

    std::string filePath;
    unsigned sampleRate = 0, durationSeconds = 0, secondsLimit = maximumSeconds;
    std::uint64_t frameLimit = 0;
    std::uint64_t lastHeaderFrames = 0;  // writer-owned
    bool analysing = false;               // set before start()
    unsigned bits = 32;                   // 32 float, 24 or 16 PCM; set before start()
    std::uint32_t ditherState = 0x9e3779b9u;  // writer-owned
    std::array<unsigned char, 1024 * 6> packed {};  // writer-owned
    aquorbium::TakeAnalysis takeAnalysis {};  // writer-owned
    aquorbium::TakeAnalysis::Result analysisResult {};  // read after done()
    aquorbium::TakeAnalysis encodedAnalysis_ {};  // writer-owned: the integers written
    aquorbium::TakeAnalysis::Result encodedResult {};  // read after done()
    std::array<std::pair<std::string, std::string>, 8> info {};
    std::size_t infoCount = 0;
    std::array<std::array<float, 2>, ringFrames> ring {};
    std::FILE* file = nullptr; // writer-owned after start(), joined before destruction
    std::thread worker;
    std::chrono::steady_clock::time_point wallDeadline {};
    std::atomic<std::uint64_t> writeIndex { 0 }, readIndex { 0 };
    std::atomic<std::uint64_t> droppedFrames { 0 }, elapsedFrames { 0 }, writtenFrames { 0 };
    std::atomic<bool> finished { false }, writerDone { false }, writeFailed { false };
    std::atomic<bool> wallDeadlineReached { false };
};
}
