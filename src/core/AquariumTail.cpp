// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/AquariumTail.h"
#include "core/Numeric.h"

#include <algorithm>
#include <cmath>

namespace aquorbium
{
void AquariumTail::prepare(double newSampleRate) noexcept
{
    sampleRate = numeric::sampleRate(newSampleRate);
    updateCoefficients();
    reset();
}

void AquariumTail::reset() noexcept
{
    for (auto& line : delay) line.fill(0.0f);
    writeIndex.fill(0u);
    lowpassState.fill(0.0f);
    currentEnergy = 0.0f;
}

void AquariumTail::setParameters(Parameters value) noexcept
{
    controls.size = numeric::unit(value.size, controls.size);
    controls.decaySeconds = numeric::clampFinite(
        value.decaySeconds, 0.15f, 20.0f, controls.decaySeconds);
    controls.damping = numeric::unit(value.damping, controls.damping);
    controls.diffusion = numeric::unit(value.diffusion, controls.diffusion);
    controls.nightmare = numeric::unit(value.nightmare, controls.nightmare);
    controls.level = numeric::unit(value.level, controls.level);
    updateCoefficients();
}

void AquariumTail::updateCoefficients() noexcept
{
    static constexpr std::array<double, lineCount> baseMilliseconds {
        29.7, 37.1, 41.1, 47.7
    };
    const auto sizeScale = 0.45 + 1.55 * static_cast<double>(controls.size);
    for (std::size_t line = 0; line < lineCount; ++line)
    {
        const auto samples = static_cast<std::size_t>(std::round(
            baseMilliseconds[line] * sizeScale * sampleRate / 1000.0));
        delayLength[line] = std::clamp(samples, std::size_t { 2 },
                                       lineCapacity - 1u);
        const auto delaySeconds = static_cast<double>(delayLength[line]) / sampleRate;
        feedbackGain[line] = static_cast<float>(std::clamp(
            std::pow(0.001, delaySeconds / controls.decaySeconds), 0.0, 0.985));
    }
    // This pole is intentionally a timbral control rather than a room model.
    dampingPole = 0.03f + controls.damping * 0.94f;
}

AquariumTail::StereoSample AquariumTail::tick(float inputLeft,
                                               float inputRight) noexcept
{
    const auto left = numeric::clampFinite(inputLeft, -2.0f, 2.0f, 0.0f);
    const auto right = numeric::clampFinite(inputRight, -2.0f, 2.0f, 0.0f);
    const auto mono = (left + right) * 0.5f;
    const auto side = (left - right) * 0.5f;

    std::array<float, lineCount> read {};
    for (std::size_t line = 0; line < lineCount; ++line)
    {
        read[line] = delay[line][writeIndex[line]];
        lowpassState[line] += (read[line] - lowpassState[line])
            * (1.0f - dampingPole);
        read[line] = lowpassState[line];
    }

    // Orthonormal Hadamard diffusion. The nightmare matrix is another signed
    // permutation; their convex interpolation cannot increase vector norm.
    const std::array<float, lineCount> diffuse {
        (read[0] + read[1] + read[2] + read[3]) * 0.5f,
        (read[0] - read[1] + read[2] - read[3]) * 0.5f,
        (read[0] + read[1] - read[2] - read[3]) * 0.5f,
        (read[0] - read[1] - read[2] + read[3]) * 0.5f
    };
    const std::array<float, lineCount> twisted {
        read[1], -read[2], read[3], -read[0]
    };
    const auto nightmare = controls.nightmare;
    const auto injection = 0.18f + controls.diffusion * 0.12f;
    const std::array<float, lineCount> input {
        mono + side, mono - side, -mono + side, -mono - side
    };
    for (std::size_t line = 0; line < lineCount; ++line)
    {
        const auto feedback = diffuse[line] * (1.0f - nightmare)
            + twisted[line] * nightmare;
        delay[line][writeIndex[line]] = input[line] * injection
            + feedback * feedbackGain[line];
        writeIndex[line] = (writeIndex[line] + 1u) % delayLength[line];
    }

    const auto width = 0.55f + controls.diffusion * 0.45f;
    const auto wetLeft = (read[0] + read[2] + (read[1] - read[3]) * width) * 0.25f;
    const auto wetRight = (read[1] + read[3] + (read[2] - read[0]) * width) * 0.25f;
    const auto level = controls.level * 0.7f;
    const auto result = StereoSample { wetLeft * level, wetRight * level };
    const auto instantEnergy = (result.left * result.left
                                + result.right * result.right) * 0.5f;
    currentEnergy += (instantEnergy - currentEnergy) * 0.001f;
    return result;
}

AquariumTail::Telemetry AquariumTail::telemetry() const noexcept
{
    return { delayLength, feedbackGain, currentEnergy };
}
}
