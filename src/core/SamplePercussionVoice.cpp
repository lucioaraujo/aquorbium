// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/SamplePercussionVoice.h"

#include "core/Numeric.h"

#include <algorithm>
#include <cmath>

namespace aquorbium
{
void SamplePercussionVoice::prepare(double newSampleRate) noexcept
{
    sampleRate = numeric::sampleRate(newSampleRate);
    stop();
}

bool SamplePercussionVoice::loadSample(std::size_t slot, const float* data,
                                       std::size_t frames) noexcept
{
    if (slot >= maximumSamples || data == nullptr || frames < 2
        || frames > maximumFrames)
        return false;
    auto& sample = samples[slot];
    for (std::size_t frame = 0; frame < frames; ++frame)
        sample.data[frame] = std::clamp(
            std::isfinite(data[frame]) ? data[frame] : 0.0f, -1.0f, 1.0f);
    sample.length = frames;
    if (playing && selected == slot)
        stop();
    return true;
}

void SamplePercussionVoice::clearSample(std::size_t slot) noexcept
{
    if (slot >= maximumSamples)
        return;
    samples[slot].length = 0;
    if (playing && selected == slot)
        stop();
}

bool SamplePercussionVoice::trigger(std::size_t slot, float pitchSemitones,
                                    float decaySeconds, float velocity) noexcept
{
    if (slot >= maximumSamples || samples[slot].length < 2)
        return false;
    const auto pitch = std::clamp(
        std::isfinite(pitchSemitones) ? pitchSemitones : 0.0f, -36.0f, 36.0f);
    const auto decay = std::clamp(
        std::isfinite(decaySeconds) ? decaySeconds : 0.25f, 0.005f, 8.0f);
    const auto level = std::clamp(
        std::isfinite(velocity) ? velocity : 0.0f, 0.0f, 1.0f);

    selected = slot;
    position = 0.0;
    increment = std::exp2(static_cast<double>(pitch) / 12.0);
    envelope = level;
    envelopeDecay = static_cast<float>(std::exp(
        -6.907755278982137 / (static_cast<double>(decay) * sampleRate)));
    playing = level > 0.0f;
    return true;
}

float SamplePercussionVoice::tick() noexcept
{
    if (!playing)
        return 0.0f;
    const auto& sample = samples[selected];
    const auto index = static_cast<std::size_t>(position);
    if (index + 1 >= sample.length || envelope < 1.0e-6f)
    {
        stop();
        return 0.0f;
    }
    const auto fraction = static_cast<float>(position - static_cast<double>(index));
    const auto value = sample.data[index]
        + (sample.data[index + 1] - sample.data[index]) * fraction;
    const auto output = value * envelope;
    position += increment;
    envelope *= envelopeDecay;
    return std::clamp(output, -1.0f, 1.0f);
}

void SamplePercussionVoice::stop() noexcept
{
    position = 0.0;
    envelope = 0.0f;
    playing = false;
}
}
