// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/SynthFMVoice.h"
#include "core/Numeric.h"

#include <algorithm>
#include <cmath>

namespace aquorbium
{
namespace
{
constexpr double twoPi = 6.28318530717958647692;
}

void SynthFMVoice::prepare(double newSampleRate) noexcept
{
    sampleRate = numeric::sampleRate(newSampleRate);
    for (int index = 0; index <= tableSize; ++index)
        sineTable[static_cast<std::size_t>(index)] =
            static_cast<float>(std::sin(twoPi * static_cast<double>(index) / tableSize));
    setParameters(controls);
    reset();
}

void SynthFMVoice::reset() noexcept
{
    carrierPhase = 0.0;
    modulatorPhase = 0.0;
    lastFrequencyHz = controls.carrierHz;
    folder.reset();
}

void SynthFMVoice::setParameters(Parameters value) noexcept
{
    const auto limit = static_cast<float>(sampleRate * 0.45);
    controls.carrierHz = numeric::clampFinite(value.carrierHz, 0.0f, limit,
                                               controls.carrierHz);
    controls.modulatorHz = numeric::clampFinite(value.modulatorHz, 0.0f, limit,
                                                 controls.modulatorHz);
    controls.depthHz = numeric::clampFinite(value.depthHz, 0.0f, limit,
                                             controls.depthHz);
    controls.fold = numeric::unit(value.fold, controls.fold);
    controls.level = numeric::unit(value.level, controls.level);
}

float SynthFMVoice::readSine(double phase) const noexcept
{
    const auto position = phase * tableSize;
    const auto index = static_cast<int>(position);
    const auto fraction = static_cast<float>(position - index);
    const auto left = sineTable[static_cast<std::size_t>(index)];
    return left + (sineTable[static_cast<std::size_t>(index + 1)] - left) * fraction;
}

void SynthFMVoice::advance(double& phase, double frequency, double rate) noexcept
{
    // Signed increment is deliberate: negative instantaneous frequency really
    // reverses the carrier phase instead of rectifying it at zero.
    phase += frequency / rate;
    phase -= std::floor(phase);
}

float SynthFMVoice::tick() noexcept
{
    const auto modulator = readSine(modulatorPhase);
    const auto frequency = std::clamp(
        static_cast<double>(controls.carrierHz)
            + static_cast<double>(controls.depthHz) * modulator,
        -sampleRate * 0.45, sampleRate * 0.45);
    lastFrequencyHz = static_cast<float>(frequency);
    const auto carrier = readSine(carrierPhase);
    advance(modulatorPhase, controls.modulatorHz, sampleRate);
    advance(carrierPhase, frequency, sampleRate);

    if (controls.level == 0.0f) return 0.0f;
    if (controls.fold == 0.0f) return carrier * controls.level;
    const auto gain = (0.5 + 3.0 * controls.fold) * 3.14159265358979323846;
    const auto mixed = folder.process(carrier, controls.fold,
                                      static_cast<float>(gain));
    return mixed * controls.level;
}
}
