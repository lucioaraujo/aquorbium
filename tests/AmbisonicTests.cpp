// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

// First-order Ambisonics, AmbiX (2026-10-23): the panner recovers every
// direction it was given; a stereo pair lands at +-30 degrees; in the engine
// the bus changes nothing in the output, and its W is (L + R) / sqrt(2) of
// the mix before the creative MASTER (the sum of the stems).
#include "app/AmbisonicCapture.h"
#include "core/AmbisonicEncoder.h"
#include "core/BiomaEngine.h"

#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <chrono>
#include <thread>
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <vector>

namespace
{
int failures = 0;
void check(bool condition, const std::string& what)
{
    if (!condition)
    {
        std::cerr << "FAILED: " << what << '\n';
        ++failures;
    }
}
constexpr float pi = 3.14159265358979f;
}

int main()
{
    // ---- the panner: AmbiX axes and every direction recovered --------------
    {
        struct Case
        {
            const char* name;
            float azimuthRight, elevation, x, y, z;
        };
        const std::array<Case, 5> cases {{
            { "front", 0.0f, 0.0f, 1.0f, 0.0f, 0.0f },
            { "right (Y negative in AmbiX)", pi / 2.0f, 0.0f, 0.0f, -1.0f, 0.0f },
            { "left", -pi / 2.0f, 0.0f, 0.0f, 1.0f, 0.0f },
            { "behind", pi, 0.0f, -1.0f, 0.0f, 0.0f },
            { "above", 0.0f, pi / 2.0f, 0.0f, 0.0f, 1.0f },
        }};
        for (const auto& c : cases)
        {
            aquorbium::FoaPanner panner;
            panner.prepare(48000.0);
            panner.setDirection(c.azimuthRight, c.elevation);
            const auto e = panner.tick(0.5f);
            check(std::abs(e.w - 0.5f) < 1.0e-6f && std::abs(e.x - 0.5f * c.x) < 1.0e-5f
                      && std::abs(e.y - 0.5f * c.y) < 1.0e-5f && std::abs(e.z - 0.5f * c.z) < 1.0e-5f,
                  std::string("panner axis: ") + c.name);
        }
        // a grid of directions: the first-order vector over W is the unit
        // direction, and its length is 1 (a single source: X^2+Y^2+Z^2 = W^2)
        double worst = 0.0;
        for (int a = -8; a <= 8; ++a)
            for (int b = -3; b <= 3; ++b)
            {
                const auto azimuth = static_cast<float>(a) * pi / 8.0f;
                const auto elevation = static_cast<float>(b) * pi / 8.0f;
                aquorbium::FoaPanner panner;
                panner.prepare(48000.0);
                panner.setDirection(azimuth, elevation);
                const auto e = panner.tick(1.0f);
                const auto recoveredAzimuth = std::atan2(-e.y, e.x);  // back to right-positive
                const auto recoveredElevation = std::atan2(e.z, std::sqrt(e.x * e.x + e.y * e.y));
                const auto length = std::sqrt(e.x * e.x + e.y * e.y + e.z * e.z);
                auto azimuthError = std::abs(std::remainder(recoveredAzimuth - azimuth, 2.0f * pi));
                if (std::abs(std::cos(elevation)) < 1.0e-3f)
                    azimuthError = 0.0f;  // the pole has no azimuth
                worst = std::max({ worst, static_cast<double>(azimuthError),
                                   static_cast<double>(std::abs(recoveredElevation - elevation)),
                                   static_cast<double>(std::abs(length - 1.0f)) });
            }
        std::cout << "panner: worst direction error over 119 directions " << worst << '\n';
        check(worst < 1.0e-4, "panner: every direction recovered");

        // the direction glides: a jump from front to right is not a step
        aquorbium::FoaPanner panner;
        panner.prepare(48000.0);
        panner.setDirection(0.0f, 0.0f);
        static_cast<void>(panner.tick(1.0f));
        panner.setDirection(pi / 2.0f, 0.0f);
        const auto next = panner.tick(1.0f);
        check(next.x > 0.99f && next.y > -0.01f, "panner: a direction change glides (~20 ms)");

        // the stereo pair: L alone sits at +30 degrees (left), R at -30
        const auto left = aquorbium::FoaPanner::encodeStereo(1.0f, 0.0f);
        const auto right = aquorbium::FoaPanner::encodeStereo(0.0f, 1.0f);
        const auto leftAzimuth = std::atan2(left.y, left.x) * 180.0f / pi;
        const auto rightAzimuth = std::atan2(right.y, right.x) * 180.0f / pi;
        check(std::abs(leftAzimuth - 30.0f) < 1.0e-3f && std::abs(rightAzimuth + 30.0f) < 1.0e-3f
                  && std::abs(left.w - 0.70710678f) < 1.0e-6f,
              "stereo pair at +-30 degrees, W = 1/sqrt(2)");
    }

    // ---- the engine: the bus is silent to the output; W is the mix ---------
    {
        constexpr double rate = 48000.0;
        constexpr std::size_t block = 256;
        auto plain = std::make_unique<aquorbium::BiomaEngine>();
        auto withBus = std::make_unique<aquorbium::BiomaEngine>();
        for (auto* engine : { plain.get(), withBus.get() })
        {
            engine->prepare(rate);
            engine->setSeed(0xa3b1u);
            engine->setAutonomousEnabled(true);
            engine->setEcologicalMusicPerformance({ 0.6f, 0.6f, 0.5f, 0.5f, 1.0f });
            engine->noteOn(48, 0.8f);
        }
        std::array<std::array<float, block>, 4> foa {};
        aquorbium::BiomaEngine::AmbisonicBuffers bus;
        for (std::size_t c = 0; c < 4; ++c)
            bus.channel[c] = foa[c].data();
        bus.capacity = block;
        std::array<std::array<float, block>, aquorbium::BiomaEngine::stemCount> stemL {}, stemR {};
        aquorbium::BiomaEngine::StemBuffers stems;
        for (std::size_t s = 0; s < aquorbium::BiomaEngine::stemCount; ++s)
        {
            stems.left[s] = stemL[s].data();
            stems.right[s] = stemR[s].data();
        }
        stems.capacity = block;

        std::array<float, block> la {}, ra {}, lb {}, rb {};
        bool identical = true, finite = true;
        double worstW = 0.0, energyW = 0.0, energyXyz = 0.0, mixPeak = 0.0;
        for (std::size_t done = 0; done < static_cast<std::size_t>(rate * 20.0); done += block)
        {
            plain->render(la.data(), ra.data(), block);
            // like the stems, the bus is handed over before every render
            withBus->setAmbisonics(bus);
            withBus->setStems(stems);
            withBus->render(lb.data(), rb.data(), block);
            for (std::size_t i = 0; i < block; ++i)
            {
                identical = identical && std::bit_cast<std::uint32_t>(la[i]) == std::bit_cast<std::uint32_t>(lb[i])
                    && std::bit_cast<std::uint32_t>(ra[i]) == std::bit_cast<std::uint32_t>(rb[i]);
                double mixL = 0.0, mixR = 0.0;
                for (std::size_t s = 0; s < aquorbium::BiomaEngine::stemCount; ++s)
                {
                    mixL += stemL[s][i];
                    mixR += stemR[s][i];
                }
                const auto expectedW = (mixL + mixR) * 0.70710678;
                worstW = std::max(worstW, std::abs(static_cast<double>(foa[0][i]) - expectedW));
                mixPeak = std::max(mixPeak, std::abs(expectedW));
                for (std::size_t c = 0; c < 4; ++c)
                    finite = finite && std::isfinite(foa[c][i]);
                energyW += static_cast<double>(foa[0][i]) * foa[0][i];
                energyXyz += static_cast<double>(foa[1][i]) * foa[1][i] + static_cast<double>(foa[2][i]) * foa[2][i]
                    + static_cast<double>(foa[3][i]) * foa[3][i];
            }
        }
        const auto ratio = energyXyz / std::max(energyW, 1.0e-30);
        std::cout << "engine: output identical with the bus " << (identical ? "yes" : "NO")
                  << "; |W - (L+R)/sqrt2| at most " << worstW << " (mix peak " << mixPeak
                  << "); directional/omni energy " << ratio << '\n';
        check(identical, "engine: the ambisonic bus changes nothing in the output");
        check(finite, "engine: the bus is finite");
        check(mixPeak > 1.0e-3 && worstW < 1.0e-5 * std::max(1.0, mixPeak * 10.0),
              "engine: W is (L + R) / sqrt(2) of the mix before the creative MASTER");
        // incoherent sources each give X^2+Y^2+Z^2 = W^2: the ratio sits near
        // 1; a broken encoder (zeros, or gains above unity) leaves that band
        check(ratio > 0.5 && ratio < 1.5, "engine: directional energy is that of first-order sources");
    }

    // ---- the REC's AmbiX file: header and samples read back ----------------
    {
        const auto file = std::filesystem::temp_directory_path()
            / ("aquorbium-ambix-test-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()) + ".wav");
        constexpr std::size_t frames = 48000;
        {
            aquorbium::app::AmbisonicCapture capture(file.string(), 48000, 10);
            check(capture.start(), "ambix capture: starts");
            std::array<std::array<float, 480>, 4> blockData {};
            for (std::size_t done = 0; done < frames; done += 480)
            {
                for (std::size_t i = 0; i < 480; ++i)
                    for (std::size_t c = 0; c < 4; ++c)
                        blockData[c][i] = static_cast<float>(c + 1) * 0.1f
                            + static_cast<float>((done + i) % 100) * 1.0e-4f;
                blockData[2][7] = std::numeric_limits<float>::quiet_NaN();  // becomes silence
                capture.push(blockData[0].data(), blockData[1].data(), blockData[2].data(), blockData[3].data(), 480);
                std::this_thread::sleep_for(std::chrono::microseconds(200));
            }
            capture.stop();
            check(capture.done() && !capture.failed() && capture.captured() == frames && capture.dropped() == 0,
                  "ambix capture: every frame written");
            // an existing take is never overwritten
            aquorbium::app::AmbisonicCapture again(file.string(), 48000, 10);
            check(!again.start(), "ambix capture: never overwrites a file");
        }
        std::ifstream in(file, std::ios::binary);
        const std::vector<unsigned char> bytes { std::istreambuf_iterator<char>(in), {} };
        const auto u16 = [&bytes](std::size_t at) { return static_cast<unsigned>(bytes[at] | (bytes[at + 1] << 8)); };
        const auto u32 = [&bytes](std::size_t at) {
            return static_cast<std::uint32_t>(bytes[at]) | (static_cast<std::uint32_t>(bytes[at + 1]) << 8)
                | (static_cast<std::uint32_t>(bytes[at + 2]) << 16) | (static_cast<std::uint32_t>(bytes[at + 3]) << 24);
        };
        const auto f32 = [&](std::size_t frame, std::size_t channel) {
            return std::bit_cast<float>(u32(80 + (frame * 4 + channel) * 4));
        };
        check(bytes.size() == 80 + frames * 16, "ambix file: size");
        check(std::string(bytes.begin(), bytes.begin() + 4) == "RIFF" && u32(4) == bytes.size() - 8
                  && std::string(bytes.begin() + 8, bytes.begin() + 16) == "WAVEfmt ",
              "ambix file: RIFF/WAVE");
        check(u32(16) == 40 && u16(20) == 0xfffe && u16(22) == 4 && u32(24) == 48000 && u16(32) == 16
                  && u16(34) == 32 && u32(40) == 0 && bytes[44] == 3 && bytes[59] == 0x71,
              "ambix file: extensible, 4 channels, float, no speaker mask");
        check(std::string(bytes.begin() + 60, bytes.begin() + 64) == "fact" && u32(68) == frames
                  && std::string(bytes.begin() + 72, bytes.begin() + 76) == "data" && u32(76) == frames * 16,
              "ambix file: fact and data");
        check(f32(0, 0) == 0.1f && f32(0, 3) == 0.4f && f32(7, 2) == 0.0f && f32(8, 2) == 0.3f + 8.0e-4f,
              "ambix file: W Y Z X in order, non-finite as silence");
        std::filesystem::remove(file);
    }

    if (failures == 0)
        std::cout << "ambisonics: all checks passed\n";
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
