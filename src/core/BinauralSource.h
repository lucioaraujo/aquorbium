// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include <array>
#include <cstddef>

namespace aquorbium
{
// One sound source heard through a head (2026-10-21; the author chose
// "fones e caixas (seletor)": the loudspeakers keep the stereo mix, the
// headphones get this). Brown & Duda's structural model of binaural sound
// (1998, IEEE Trans. Speech and Audio Processing 6:476-488), rewritten from
// the paper - no measured HRTF data, no third-party files:
//
//   - HEAD SHADOW: per ear a one-pole, one-zero filter
//       H(w, theta) = (1 + j alpha w / 2w0) / (1 + j w / 2w0), w0 = c / a,
//       alpha(theta) = (1 + amin/2) + (1 - amin/2) cos(theta / thetamin * pi),
//     amin = 0.1, thetamin = 150 degrees, theta the angle between the source
//     and the ear's axis: the near ear's highs rise, the far ear's fall;
//   - the INTERAURAL TIME DIFFERENCE (Woodworth's spherical head):
//       T(theta) = -a/c cos(theta)          for theta < 90 degrees,
//                = a/c (theta - pi/2)       beyond;
//     a = 8.75 cm, c = 343 m/s: up to ~0.66 ms between the ears;
//   - the PINNA: the direct sound and five short echoes whose delays depend
//     on the elevation and, through cos(azimuth / 2), on front and back:
//       tau_k = A_k cos(az / 2) sin(D_k (90 deg - el)) + B_k   (44.1 kHz samples),
//       rho = 0.5, -1, 0.5, -0.25, 0.25;  A = 1, 5, 5, 5, 5;
//       B = 2, 4, 7, 11, 13;  D = 1, 0.5, 0.5, 0.5, 0.5
//     (D varies from listener to listener; these are the paper's typical
//     values as we read them).
//
// Direction at control rate, delays smoothed per sample. Fixed memory, no
// allocation: safe on the audio thread.
class BinauralSource final
{
public:
    struct Sample
    {
        float left = 0.0f;
        float right = 0.0f;
    };

    static constexpr float headRadius = 0.0875f;  // metres
    static constexpr float soundSpeed = 343.0f;   // in air, at the listener's head

    void prepare(double sampleRate) noexcept;
    void reset() noexcept;
    // azimuth: radians, + to the right; elevation: radians, + up
    void setDirection(float azimuth, float elevation) noexcept;
    [[nodiscard]] Sample tick(float input) noexcept;

    // the model's pieces, for the tests
    [[nodiscard]] static float interauralDelaySeconds(float earAngle) noexcept;  // delay to one ear
    [[nodiscard]] static float shadowAlpha(float earAngle) noexcept;

private:
    struct Shadow
    {
        float b0 = 1.0f, b1 = 0.0f, a1 = 0.0f, x1 = 0.0f, y1 = 0.0f;
        void set(float alpha, float sampleRate) noexcept;
        float process(float x) noexcept;
    };

    static constexpr std::size_t lineSize = 128;  // > the ITD and the pinna's echoes at 96 kHz
    [[nodiscard]] static float read(const std::array<float, lineSize>& line, std::size_t write, float delay) noexcept;

    float rate = 48000.0f;
    std::array<float, lineSize> input {};    // for the pinna's echoes
    std::array<float, lineSize> pinnaOut {}; // for the ears' delays
    std::size_t write = 0;
    std::array<float, 5> echoTarget {}, echo {};
    float leftDelayTarget = 0.0f, rightDelayTarget = 0.0f, leftDelay = 0.0f, rightDelay = 0.0f;
    float smoothing = 0.001f;
    Shadow leftShadow {}, rightShadow {};
};
}
