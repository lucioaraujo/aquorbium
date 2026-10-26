// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include <array>
#include <complex>
#include <cstddef>
#include <cstdint>

namespace aquorbium
{
// The health of the habitat, heard (CONCEITOS_VERIFICACAO_E_APROFUNDAMENTO.md
// §2.1; 2026-10-21). Ecoacoustics measures a soundscape with indices; the
// method reference is scikit-maad (Ulloa et al. 2021, BSD-3), from which
// nothing is copied - the two indices are rewritten from their papers:
//
//   ADI, the Acoustic Diversity Index (Villanueva-Rivera et al. 2011,
//   Landscape Ecology 26:1233): the Shannon entropy of how often each
//   frequency band is above -50 dB under the window's loudest band (as
//   soundecology normalises the spectrogram to its maximum, so that a quiet
//   reef is not mistaken for an empty one). Bands used evenly: high; one band
//   crowded by everyone: low. Adapted: ten bands spaced logarithmically from
//   50 Hz to 12.8 kHz (the original uses 1 kHz bands up to 10 kHz, made for
//   birds; the reef lives lower), normalised to 0..1.
//
//   ACI, the Acoustic Complexity Index (Pieretti et al. 2011, Ecological
//   Indicators 11:868): for each band, the sum of the changes of intensity
//   between successive frames over the sum of the intensity. Articulated,
//   living sound: high; a steady drone: near zero. Adapted: per band rather
//   than per bin, averaged over the bands that are sounding.
//
// Health joins both: even occupancy and living articulation. A healthy
// habitat leaves bands free and keeps its voices distinct; a degraded one
// piles on itself or drones. It runs on the audio thread: a 4096-point FFT
// every 4096 samples (85 ms) into fixed arrays, no allocation after prepare().
class HabitatHealth final
{
public:
    static constexpr std::size_t fftSize = 4096;
    static constexpr std::size_t bandCount = 10;

    struct Reading
    {
        float adi = 0.0f;     // 0 .. 1
        float aci = 0.0f;     // mean over sounding bands, ~0 .. 2
        float health = 0.5f;  // 0 degraded .. 1 healthy; 0.5 until measured
        bool measured = false;
    };

    void prepare(double sampleRate, double windowSeconds = 5.0) noexcept;
    void reset() noexcept;
    void push(float sample) noexcept;
    [[nodiscard]] Reading reading() const noexcept { return last; }

    // the ACI that counts as fully articulated (a tone pulsing 100 ms on and
    // off reads about this)
    static constexpr float aciReference = 0.8f;
    static constexpr float thresholdDb = -30.0f;   // under the window's loudest band
    static constexpr float silenceDb = -100.0f;    // below this nothing is measured
    static constexpr std::size_t maxFramesPerWindow = 256;

private:
    void analyseFrame() noexcept;
    void closeWindow() noexcept;

    std::array<float, fftSize> input {};
    std::array<float, fftSize> window {};
    std::array<std::complex<float>, fftSize> spectrum {};
    std::array<std::complex<float>, fftSize / 2> twiddle {};
    std::array<std::uint16_t, fftSize> bitReverse {};
    std::array<std::uint16_t, bandCount + 1> bandEdge {};  // FFT bins
    std::size_t filled = 0;
    std::uint32_t framesPerWindow = 234;
    std::uint32_t frames = 0;
    std::array<std::array<float, bandCount>, maxFramesPerWindow> bandDb {};  // per frame
    std::array<double, bandCount> intensitySum {};
    std::array<double, bandCount> changeSum {};
    std::array<float, bandCount> previous {};
    bool hasPrevious = false;
    Reading last {};
};
}
