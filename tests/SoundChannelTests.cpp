// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

// The deep sound channel (2026-10-21): Munk's minimum at the axis, the axis
// rising to the surface in cold water, sound ducted along the channel,
// shadows across it, and the free field away from it.
#include "core/SoundChannel.h"

#include <cmath>
#include <cstdlib>
#include <iostream>

namespace
{
using aquorbium::SoundChannel;
int failures = 0;
void check(bool condition, const char* what)
{
    if (!condition)
    {
        std::cerr << "FAILED: " << what << '\n';
        ++failures;
    }
}
float db(float x) { return 20.0f * std::log10(x); }
}

int main()
{
    const auto warm = SoundChannel::axisHeight(25.0f);
    const auto polar = SoundChannel::axisHeight(0.0f);
    check(polar > warm, "cold water raises the axis");
    check(polar > 0.85f * SoundChannel::sphereRadius, "in polar water the axis is just under the surface");
    check(warm < 1.5f && warm > -1.0f, "in warm water the axis is deep");

    // Munk: the slowest water is the axis
    const auto atAxis = SoundChannel::speedRatio(warm, warm);
    check(std::abs(atAxis - 1.0f) < 1.0e-6f, "c = c1 on the axis");
    check(SoundChannel::speedRatio(warm + 1.5f, warm) > atAxis && SoundChannel::speedRatio(warm - 1.5f, warm) > atAxis,
          "sound is faster above and below the axis");

    const auto along = SoundChannel::gain(warm, warm + 0.1f, 6.0f, warm);
    const auto across = SoundChannel::gain(warm, warm - 3.0f, 4.0f, warm);
    const auto outside = SoundChannel::gain(warm - 3.0f, warm - 3.2f, 4.0f, warm);
    const auto near = SoundChannel::gain(warm, warm - 3.0f, 0.3f, warm);
    std::cout << "along the channel at 6 units: " << db(along) << " dB; into the shadow at 4: " << db(across)
              << " dB; far from it: " << db(outside) << " dB; across but near: " << db(near) << " dB\n";
    check(db(along) > 7.0f, "along the axis the sound travels far (cylindrical spreading)");
    check(db(across) < -8.0f, "from the channel to a listener far below: a shadow");
    check(std::abs(db(outside)) < 0.5f, "away from the channel the field is free");
    check(db(near) > -4.0f, "near the source the direct path still arrives");
    check(std::abs(SoundChannel::gain(warm, warm - 3.0f, 4.0f, warm) - SoundChannel::gain(warm - 3.0f, warm, 4.0f, warm))
              < 1.0e-6f,
          "the shadow is reciprocal");
    if (failures == 0)
        std::cout << "sound channel: all checks passed\n";
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
