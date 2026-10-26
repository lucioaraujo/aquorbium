// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include <array>
#include <cstddef>

namespace aquorbium
{
// Fixed-memory fallback voice for short mono samples. Loading belongs outside
// the audio callback; trigger and tick are bounded and allocation-free.
class SamplePercussionVoice final
{
public:
    static constexpr std::size_t maximumSamples = 4;
    static constexpr std::size_t maximumFrames = 4096;

    void prepare(double newSampleRate) noexcept;
    [[nodiscard]] bool loadSample(std::size_t slot, const float* data,
                                  std::size_t frames) noexcept;
    void clearSample(std::size_t slot) noexcept;
    [[nodiscard]] bool trigger(std::size_t slot, float pitchSemitones,
                               float decaySeconds, float velocity) noexcept;
    [[nodiscard]] float tick() noexcept;
    void stop() noexcept;

    [[nodiscard]] bool active() const noexcept { return playing; }
    [[nodiscard]] std::size_t selectedSample() const noexcept { return selected; }

private:
    struct Sample
    {
        std::array<float, maximumFrames> data {};
        std::size_t length = 0;
    };

    std::array<Sample, maximumSamples> samples {};
    double sampleRate = 48000.0;
    double position = 0.0;
    double increment = 1.0;
    float envelope = 0.0f;
    float envelopeDecay = 0.0f;
    std::size_t selected = 0;
    bool playing = false;
};
}
