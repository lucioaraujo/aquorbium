// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/BiomaGranularCapture.h"
#include "core/Numeric.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace aquorbium
{
namespace
{
constexpr double pi = 3.14159265358979323846;
}

void BiomaGranularCapture::count(std::uint64_t& value) noexcept
{
    if (value != std::numeric_limits<std::uint64_t>::max()) ++value;
}

BiomaGranularCapture::GrainParameters BiomaGranularCapture::sanitize(
    GrainParameters value, GrainParameters fallback) noexcept
{
    value.durationMs = numeric::clampFinite(value.durationMs, 5.0f, 150.0f,
                                             fallback.durationMs);
    value.speed = numeric::clampFinite(value.speed, 0.5f, 2.0f, fallback.speed);
    value.pan = numeric::clampFinite(value.pan, -1.0f, 1.0f, fallback.pan);
    value.gain = numeric::unit(value.gain, fallback.gain);
    return value;
}

void BiomaGranularCapture::prepare(double newSampleRate) noexcept
{
    sampleRate = numeric::sampleRate(newSampleRate);
    for (std::size_t index = 0; index <= windowSize; ++index)
        window[index] = static_cast<float>(0.5 - 0.5 * std::cos(
            2.0 * pi * static_cast<double>(index) / windowSize));
    fastGain = static_cast<float>(1.0 - std::exp(-1.0 / (0.003 * sampleRate)));
    slowGain = static_cast<float>(1.0 - std::exp(-1.0 / (0.05 * sampleRate)));
    reset();
}

void BiomaGranularCapture::reset() noexcept
{
    history.fill(0.0f);
    grains.fill({});
    written = 0;
    fastEnvelope = 0.0f;
    slowEnvelope = 0.0f;
    refractorySamples = 0;
    onsetArmed = true;
    onsetCount = 0;
    triggeredCount = 0;
    rejectedCount = 0;
}

void BiomaGranularCapture::setGrainParameters(GrainParameters value) noexcept
{
    grainDefaults = sanitize(value, grainDefaults);
}

void BiomaGranularCapture::setOnsetParameters(OnsetParameters value) noexcept
{
    onsetControls.ratio = numeric::clampFinite(value.ratio, 1.5f, 12.0f,
                                                onsetControls.ratio);
    onsetControls.floor = numeric::clampFinite(value.floor, 0.001f, 0.5f,
                                                onsetControls.floor);
    onsetControls.refractoryMs = numeric::clampFinite(value.refractoryMs,
        10.0f, 1000.0f, onsetControls.refractoryMs);
    onsetControls.autoCapture = value.autoCapture;
}

bool BiomaGranularCapture::trigger(GrainParameters value) noexcept
{
    value = sanitize(value, grainDefaults);
    const auto requested = static_cast<std::uint32_t>(
        std::round(static_cast<double>(value.durationMs) * sampleRate / 1000.0));
    const auto length = std::clamp(requested, 2u, maximumGrainSamples);
    const auto oldestOffset = 2u * length + safetyGap;
    if (written <= oldestOffset)
    {
        count(rejectedCount);
        return false;
    }
    for (auto& grain : grains)
    {
        if (grain.length != 0) continue;
        const auto offset = value.reverse ? safetyGap : oldestOffset;
        const auto start = (written - offset) & (historyCapacity - 1u);
        grain.position = static_cast<double>(start);
        grain.increment = value.reverse ? -value.speed : value.speed;
        const auto angle = static_cast<double>(value.pan + 1.0f) * pi * 0.25;
        // Fixed 12-voice ceiling bounds the sum even under full overlap.
        const auto amplitude = static_cast<double>(value.gain) * 0.8
            / static_cast<double>(grainCapacity);
        grain.leftGain = static_cast<float>(amplitude * std::cos(angle));
        grain.rightGain = static_cast<float>(amplitude * std::sin(angle));
        grain.age = 0;
        grain.length = length;
        count(triggeredCount);
        return true;
    }
    count(rejectedCount);
    return false;
}

float BiomaGranularCapture::readHistory(double position) const noexcept
{
    const auto index = static_cast<std::size_t>(position);
    const auto fraction = static_cast<float>(position - static_cast<double>(index));
    const auto next = (index + 1u) & (historyCapacity - 1u);
    return history[index] + (history[next] - history[index]) * fraction;
}

float BiomaGranularCapture::readWindow(std::uint32_t age,
                                       std::uint32_t length) const noexcept
{
    const auto position = static_cast<double>(age) * windowSize
        / static_cast<double>(length - 1u);
    const auto index = static_cast<std::size_t>(position);
    const auto fraction = static_cast<float>(position - static_cast<double>(index));
    if (index >= windowSize) return window[windowSize];
    return window[index] + (window[index + 1u] - window[index]) * fraction;
}

BiomaGranularCapture::Sample BiomaGranularCapture::process(float input) noexcept
{
    const auto source = numeric::clampFinite(input, -1.0f, 1.0f, 0.0f);
    history[static_cast<std::size_t>(written) & (historyCapacity - 1u)] = source;
    if (written != std::numeric_limits<std::uint64_t>::max()) ++written;

    const auto magnitude = std::abs(source);
    fastEnvelope += (magnitude - fastEnvelope) * fastGain;
    slowEnvelope += (magnitude - slowEnvelope) * slowGain;
    if (refractorySamples > 0) --refractorySamples;
    const auto threshold = slowEnvelope * onsetControls.ratio;
    if (fastEnvelope < threshold * 0.7f) onsetArmed = true;
    if (onsetArmed && refractorySamples == 0
        && fastEnvelope > onsetControls.floor && fastEnvelope > threshold)
    {
        onsetArmed = false;
        refractorySamples = static_cast<std::uint32_t>(
            onsetControls.refractoryMs * sampleRate / 1000.0);
        count(onsetCount);
        if (onsetControls.autoCapture)
            (void)trigger(grainDefaults);
    }

    Sample output;
    for (auto& grain : grains)
    {
        if (grain.length == 0) continue;
        const auto value = readHistory(grain.position)
            * readWindow(grain.age, grain.length);
        output.left += value * grain.leftGain;
        output.right += value * grain.rightGain;
        ++grain.age;
        if (grain.age >= grain.length)
        {
            grain.length = 0;
            continue;
        }
        grain.position += grain.increment;
        if (grain.position < 0.0) grain.position += historyCapacity;
        if (grain.position >= historyCapacity) grain.position -= historyCapacity;
    }
    return output;
}

BiomaGranularCapture::Telemetry BiomaGranularCapture::telemetry() const noexcept
{
    Telemetry result;
    result.onsets = onsetCount;
    result.triggered = triggeredCount;
    result.rejected = rejectedCount;
    for (const auto& grain : grains)
        if (grain.length != 0) ++result.activeGrains;
    return result;
}
}
