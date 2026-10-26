// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/EcologicalNoiseBed.h"

#include "core/Numeric.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <numbers>

namespace aquorbium
{
void EcologicalNoiseBed::prepare(double sampleRate) noexcept
{
    preparedSampleRate = numeric::sampleRate(sampleRate);
    updateDerived();
    reset();
}

void EcologicalNoiseBed::reset() noexcept
{
    left = {};
    right = {};
    leftRandom = seed;
    rightRandom = seed ^ 0xa511e9b3u;
    if (rightRandom == 0u)
        rightRandom = 2u;
    holdCounter = 0;
}

void EcologicalNoiseBed::setSeed(std::uint32_t value) noexcept
{
    seed = value == 0u ? 1u : value;
    reset();
}

void EcologicalNoiseBed::setParameters(Parameters value) noexcept
{
    controls.seasonPhase = numeric::wrapUnit(
        value.seasonPhase, controls.seasonPhase);
    controls.dayPhase = numeric::wrapUnit(value.dayPhase, controls.dayPhase);
    controls.turbidity = numeric::unit(value.turbidity, controls.turbidity);
    controls.temperature = numeric::unit(value.temperature, controls.temperature);
    controls.crackStress = numeric::unit(value.crackStress, controls.crackStress);
    controls.level = numeric::clampFinite(
        value.level, 0.0f, 0.5f, controls.level);
    updateDerived();
}

EcologicalNoiseBed::StereoSample EcologicalNoiseBed::tick() noexcept
{
    const auto nextLeft = makeTexture(left, leftRandom) * controls.level;
    const auto nextRight = makeTexture(right, rightRandom) * controls.level;
    if (holdCounter == 0)
    {
        left.held = nextLeft;
        right.held = nextRight;
        holdCounter = holdSamples - 1u;
    }
    else
    {
        --holdCounter;
    }
    return { left.held, right.held };
}

EcologicalNoiseBed::Telemetry EcologicalNoiseBed::telemetry() const noexcept
{
    return { morphPosition, colourCutoffHz, holdSamples };
}

float EcologicalNoiseBed::nextSigned(std::uint32_t& state) noexcept
{
    state = state * 1664525u + 1013904223u;
    return static_cast<float>((state >> 8u) & 0x00ffffffu)
        / 8388607.5f - 1.0f;
}

float EcologicalNoiseBed::makeTexture(ChannelState& state,
                                      std::uint32_t& randomState) noexcept
{
    const auto white = nextSigned(randomState);
    state.colour += (white - state.colour) * colourCoefficient;
    state.deep += (state.colour - state.deep) * deepCoefficient;
    state.crackle = std::abs(white) > 0.94f
        ? white : state.crackle * 0.985f;
    if (std::abs(state.colour) < 1.0e-20f) state.colour = 0.0f;
    if (std::abs(state.deep) < 1.0e-20f) state.deep = 0.0f;
    if (std::abs(state.crackle) < 1.0e-20f) state.crackle = 0.0f;

    const auto high = white - state.colour;
    const std::array<float, 4> textures {
        state.deep * 0.90f,
        state.colour * 0.85f,
        white * 0.65f,
        high * 0.35f + state.crackle * 0.30f
    };
    const auto first = static_cast<std::size_t>(morphPosition) & 3u;
    const auto second = (first + 1u) & 3u;
    const auto fraction = morphPosition - std::floor(morphPosition);
    return textures[first] + (textures[second] - textures[first]) * fraction;
}

void EcologicalNoiseBed::updateDerived() noexcept
{
    morphPosition = numeric::wrapUnit(
        controls.seasonPhase + controls.dayPhase * 0.25f) * 4.0f;
    const auto brightness = std::clamp(
        (1.0f - controls.turbidity) * 0.65f
            + controls.temperature * 0.35f,
        0.0f, 1.0f);
    colourCutoffHz = std::clamp(
        180.0f * std::exp2(brightness * 6.3f), 80.0f,
        static_cast<float>(preparedSampleRate * 0.42));
    colourCoefficient = static_cast<float>(1.0 - std::exp(
        -2.0 * std::numbers::pi * static_cast<double>(colourCutoffHz)
        / preparedSampleRate));
    const auto deepCutoff = std::max(25.0f, colourCutoffHz * 0.08f);
    deepCoefficient = static_cast<float>(1.0 - std::exp(
        -2.0 * std::numbers::pi * static_cast<double>(deepCutoff)
        / preparedSampleRate));
    holdSamples = 1u + static_cast<std::size_t>(std::lround(
        controls.crackStress * controls.crackStress * 63.0f));
    holdCounter = std::min(holdCounter, holdSamples - 1u);
}
}
