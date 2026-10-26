// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/MasterChain.h"

#include "core/Numeric.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <numbers>

namespace aquorbium
{
namespace
{
constexpr float inverseSqrtTwo = 0.7071067811865475f;

float dbToGain(float value) noexcept
{
    return std::pow(10.0f, value / 20.0f);
}

float gainReductionDb(float gain) noexcept
{
    return gain < 1.0f
        ? -20.0f * std::log10(std::max(gain, 1.0e-9f)) : 0.0f;
}
}

float MasterChain::Biquad::tick(float input) noexcept
{
    const auto output = b0 * input + z1;
    z1 = b1 * input - a1 * output + z2;
    z2 = b2 * input - a2 * output;
    return output;
}

void MasterChain::count(std::uint64_t& value) noexcept
{
    if (value != std::numeric_limits<std::uint64_t>::max()) ++value;
}

void MasterChain::prepare(double newSampleRate) noexcept
{
    sampleRate = numeric::sampleRate(newSampleRate);
    influenceSmoothing = static_cast<float>(
        -std::expm1(-1.0 / (sampleRate * 0.045)));
    constexpr std::array<float, bandCount> attackMs { 32.0f, 14.0f, 5.0f };
    constexpr std::array<float, bandCount> releaseMs { 260.0f, 160.0f, 95.0f };
    for (std::size_t band = 0; band < bandCount; ++band)
    {
        bandState[band].attack = static_cast<float>(-std::expm1(
            -1.0 / (sampleRate * attackMs[band] * 0.001)));
        bandState[band].release = static_cast<float>(-std::expm1(
            -1.0 / (sampleRate * releaseMs[band] * 0.001)));
    }
    glueAttack = static_cast<float>(-std::expm1(-1.0 / (sampleRate * 0.028)));
    glueRelease = static_cast<float>(-std::expm1(-1.0 / (sampleRate * 0.380)));
    updateFilters();
    reset();
}

void MasterChain::reset() noexcept
{
    for (auto& channel : filters)
        for (auto& splitFilters : channel)
            for (auto& response : splitFilters)
                for (auto& stage : response) stage.reset();
    for (auto& state : bandState)
    {
        state.envelope = 0.0f;
        state.gain = 1.0f;
    }
    glueEnvelope = 0.0f;
    glueGain = 1.0f;
    currentInfluence = targetInfluence;
    inputPeak = outputPeak = 0.0f;
    effectiveWidth = 1.0f;
    invalidSampleCount = 0;
}

void MasterChain::setParameters(Parameters value) noexcept
{
    controls.influence = numeric::unit(value.influence, controls.influence);
    controls.multiband = numeric::unit(value.multiband, controls.multiband);
    controls.glue = numeric::unit(value.glue, controls.glue);
    controls.tilt = numeric::clampFinite(value.tilt, -1.0f, 1.0f,
                                          controls.tilt);
    controls.width = numeric::clampFinite(value.width, 0.25f, 1.75f,
                                           controls.width);
    controls.crackStress = numeric::unit(value.crackStress,
                                          controls.crackStress);
    controls.eqAfterDynamics = value.eqAfterDynamics;
    targetInfluence = controls.influence;
}

void MasterChain::setFilter(Biquad& filter, float frequency,
                            bool highpass) noexcept
{
    const auto omega = 2.0f * std::numbers::pi_v<float> * frequency
        / static_cast<float>(sampleRate);
    const auto cosine = std::cos(omega);
    const auto alpha = std::sin(omega) / (2.0f * inverseSqrtTwo);
    const auto inverseA0 = 1.0f / (1.0f + alpha);
    if (highpass)
    {
        filter.b0 = (1.0f + cosine) * 0.5f * inverseA0;
        filter.b1 = -(1.0f + cosine) * inverseA0;
        filter.b2 = filter.b0;
    }
    else
    {
        filter.b0 = (1.0f - cosine) * 0.5f * inverseA0;
        filter.b1 = (1.0f - cosine) * inverseA0;
        filter.b2 = filter.b0;
    }
    filter.a1 = -2.0f * cosine * inverseA0;
    filter.a2 = (1.0f - alpha) * inverseA0;
}

void MasterChain::updateFilters() noexcept
{
    const auto maximum = static_cast<float>(sampleRate * 0.42);
    lowCrossoverHz = std::clamp(lowCrossoverHz, 40.0f, maximum * 0.25f);
    highCrossoverHz = std::clamp(highCrossoverHz,
                                  lowCrossoverHz * 2.0f, maximum);
    for (auto& channel : filters)
    {
        for (std::size_t response = 0; response < 2; ++response)
        {
            for (auto& stage : channel[0][response])
                setFilter(stage, lowCrossoverHz, response == 1u);
            for (auto& stage : channel[1][response])
                setFilter(stage, highCrossoverHz, response == 1u);
        }
    }
}

MasterChain::Bands MasterChain::split(std::size_t channel,
                                      float input) noexcept
{
    auto low = input;
    auto aboveLow = input;
    for (auto& stage : filters[channel][0][0]) low = stage.tick(low);
    for (auto& stage : filters[channel][0][1]) aboveLow = stage.tick(aboveLow);
    auto middle = aboveLow;
    auto high = aboveLow;
    for (auto& stage : filters[channel][1][0]) middle = stage.tick(middle);
    for (auto& stage : filters[channel][1][1]) high = stage.tick(high);
    return { { low, middle, high } };
}

float MasterChain::processBand(std::size_t band, float left, float right,
                               float& outputLeft, float& outputRight) noexcept
{
    auto& state = bandState[band];
    const auto detector = std::max(std::abs(left), std::abs(right));
    state.envelope += (detector - state.envelope)
        * (detector > state.envelope ? state.attack : state.release);
    constexpr std::array<float, bandCount> thresholdDb { -20.0f, -24.0f, -27.0f };
    constexpr std::array<float, bandCount> maximumRatio { 3.0f, 4.0f, 5.0f };
    const auto ratio = 1.0f + controls.multiband
        * (maximumRatio[band] - 1.0f);
    const auto envelopeDb = 20.0f * std::log10(std::max(state.envelope, 1.0e-8f));
    const auto overDb = std::max(0.0f, envelopeDb - thresholdDb[band]);
    const auto targetGain = dbToGain(
        -overDb * (1.0f - 1.0f / ratio) * controls.multiband);
    state.gain += (targetGain - state.gain)
        * (targetGain < state.gain ? state.attack : state.release);
    outputLeft = left * state.gain;
    outputRight = right * state.gain;
    return state.gain;
}

float MasterChain::processGlue(float detector) noexcept
{
    glueEnvelope += (detector - glueEnvelope)
        * (detector > glueEnvelope ? glueAttack : glueRelease);
    const auto envelopeDb = 20.0f * std::log10(std::max(glueEnvelope, 1.0e-8f));
    const auto overDb = std::max(0.0f, envelopeDb + 15.0f);
    const auto ratio = 1.0f + controls.glue * 1.6f;
    const auto target = dbToGain(
        -overDb * (1.0f - 1.0f / ratio) * controls.glue);
    glueGain += (target - glueGain)
        * (target < glueGain ? glueAttack : glueRelease);
    return glueGain;
}

MasterChain::Sample MasterChain::process(float left, float right) noexcept
{
    if (!std::isfinite(left)) { left = 0.0f; count(invalidSampleCount); }
    if (!std::isfinite(right)) { right = 0.0f; count(invalidSampleCount); }
    if (currentInfluence == 0.0f && targetInfluence == 0.0f)
        return { left, right };

    currentInfluence += (targetInfluence - currentInfluence)
        * influenceSmoothing;
    if (std::abs(targetInfluence - currentInfluence) < 1.0e-7f)
        currentInfluence = targetInfluence;
    inputPeak = std::max(inputPeak, std::max(std::abs(left), std::abs(right)));

    auto leftBands = split(0u, left);
    auto rightBands = split(1u, right);
    const auto tiltDb = controls.tilt * 3.0f;
    const std::array<float, bandCount> eqGain {
        dbToGain(-tiltDb), 1.0f, dbToGain(tiltDb)
    };
    for (std::size_t band = 0; band < bandCount; ++band)
    {
        if (!controls.eqAfterDynamics)
        {
            leftBands.value[band] *= eqGain[band];
            rightBands.value[band] *= eqGain[band];
        }
        float compressedLeft = 0.0f, compressedRight = 0.0f;
        static_cast<void>(processBand(band, leftBands.value[band],
            rightBands.value[band], compressedLeft, compressedRight));
        leftBands.value[band] = compressedLeft;
        rightBands.value[band] = compressedRight;
        if (controls.eqAfterDynamics)
        {
            leftBands.value[band] *= eqGain[band];
            rightBands.value[band] *= eqGain[band];
        }
    }

    effectiveWidth = std::clamp(controls.width
        * (1.0f - controls.crackStress * 0.72f), 0.25f, 1.75f);
    for (std::size_t band = 1; band < bandCount; ++band)
    {
        const auto middle = (leftBands.value[band]
            + rightBands.value[band]) * 0.5f;
        const auto side = (leftBands.value[band]
            - rightBands.value[band]) * 0.5f * effectiveWidth;
        leftBands.value[band] = middle + side;
        rightBands.value[band] = middle - side;
    }

    auto wetLeft = 0.0f;
    auto wetRight = 0.0f;
    for (std::size_t band = 0; band < bandCount; ++band)
    {
        wetLeft += leftBands.value[band];
        wetRight += rightBands.value[band];
    }
    const auto glue = processGlue(std::max(std::abs(wetLeft),
                                           std::abs(wetRight)));
    wetLeft *= glue;
    wetRight *= glue;
    if (!std::isfinite(wetLeft) || !std::isfinite(wetRight))
    {
        reset();
        return { left, right };
    }
    Sample output {
        left + (wetLeft - left) * currentInfluence,
        right + (wetRight - right) * currentInfluence
    };
    outputPeak = std::max(outputPeak,
        std::max(std::abs(output.left), std::abs(output.right)));
    return output;
}

MasterChain::Telemetry MasterChain::telemetry() const noexcept
{
    Telemetry result;
    for (std::size_t band = 0; band < bandCount; ++band)
        result.bandGainReductionDb[band] = gainReductionDb(bandState[band].gain);
    result.glueGainReductionDb = gainReductionDb(glueGain);
    result.currentInfluence = currentInfluence;
    result.effectiveWidth = effectiveWidth;
    result.inputPeak = inputPeak;
    result.outputPeak = outputPeak;
    result.invalidSamples = invalidSampleCount;
    return result;
}
}
