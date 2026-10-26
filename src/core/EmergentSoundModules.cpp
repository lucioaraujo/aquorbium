// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/EmergentSoundModules.h"

#include "core/Numeric.h"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace aquorbium
{
namespace
{
constexpr float pi = std::numbers::pi_v<float>;

float safeSignal(float value) noexcept
{
    return numeric::clampFinite(value, -4.0f, 4.0f, 0.0f);
}

template <typename Filter>
void configureBandpass(Filter& filter, float frequency, float q,
                       double sampleRate) noexcept
{
    const auto omega = 2.0f * pi * frequency / static_cast<float>(sampleRate);
    const auto alpha = std::sin(omega) / (2.0f * q);
    const auto inverseA0 = 1.0f / (1.0f + alpha);
    filter.b0 = alpha * inverseA0;
    filter.b1 = 0.0f;
    filter.b2 = -alpha * inverseA0;
    filter.a1 = -2.0f * std::cos(omega) * inverseA0;
    filter.a2 = (1.0f - alpha) * inverseA0;
}

template <typename Filter>
float tickBiquad(Filter& filter, float input) noexcept
{
    const auto output = filter.b0 * input + filter.z1;
    filter.z1 = filter.b1 * input - filter.a1 * output + filter.z2;
    filter.z2 = filter.b2 * input - filter.a2 * output;
    if (!std::isfinite(output) || !std::isfinite(filter.z1)
        || !std::isfinite(filter.z2))
    {
        filter.reset();
        return 0.0f;
    }
    if (std::abs(filter.z1) < 1.0e-20f) filter.z1 = 0.0f;
    if (std::abs(filter.z2) < 1.0e-20f) filter.z2 = 0.0f;
    return output;
}
}

void ReeseVoice::prepare(double sampleRate) noexcept
{
    preparedSampleRate = numeric::sampleRate(sampleRate);
    drift.prepare(preparedSampleRate);
    drift.setParameters({ 0.37f, 0.42f, 0.31f });
    reset();
}

void ReeseVoice::setSeed(std::uint32_t value) noexcept
{
    drift.setSeed(value ^ 0x7a91c53du);
    reset();
}

void ReeseVoice::reset() noexcept
{
    phase = { 0.0f, 0.31f, 0.67f };
    drift.reset();
}

void ReeseVoice::setParameters(Parameters value) noexcept
{
    controls.frequencyHz = numeric::clampFinite(value.frequencyHz, 18.0f,
        static_cast<float>(preparedSampleRate * 0.20), controls.frequencyHz);
    controls.detuneCents = numeric::clampFinite(value.detuneCents, 0.0f, 72.0f,
                                                controls.detuneCents);
    controls.driftAmount = numeric::unit(value.driftAmount,
                                         controls.driftAmount);
}

float ReeseVoice::polyBlep(float phaseValue, float increment) noexcept
{
    if (increment <= 0.0f)
        return 0.0f;
    if (phaseValue < increment)
    {
        const auto x = phaseValue / increment;
        return x + x - x * x - 1.0f;
    }
    if (phaseValue > 1.0f - increment)
    {
        const auto x = (phaseValue - 1.0f) / increment;
        return x * x + x + x + 1.0f;
    }
    return 0.0f;
}

float ReeseVoice::saw(float phaseValue, float increment) noexcept
{
    return phaseValue * 2.0f - 1.0f - polyBlep(phaseValue, increment);
}

float ReeseVoice::tick() noexcept
{
    const auto driftValue = drift.tick() * controls.driftAmount;
    const auto detune = controls.detuneCents * (1.0f + driftValue * 0.45f);
    const std::array<float, 3> cents { -detune, 0.0f, detune };
    auto output = 0.0f;
    for (std::size_t index = 0; index < phase.size(); ++index)
    {
        const auto frequency = controls.frequencyHz
            * std::exp2(cents[index] / 1200.0f);
        const auto increment = std::clamp(
            frequency / static_cast<float>(preparedSampleRate), 0.0f, 0.24f);
        output += saw(phase[index], increment);
        phase[index] += increment;
        if (phase[index] >= 1.0f) phase[index] -= 1.0f;
    }
    return std::clamp(output / 3.0f, -1.0f, 1.0f);
}

float DualPeakFilter::Biquad::tick(float input) noexcept
{
    return tickBiquad(*this, input);
}

void DualPeakFilter::prepare(double sampleRate) noexcept
{
    preparedSampleRate = numeric::sampleRate(sampleRate);
    updateCoefficients();
    reset();
}

void DualPeakFilter::reset() noexcept
{
    for (auto& filter : peak) filter.reset();
    previousA = previousB = 0.0f;
}

void DualPeakFilter::setParameters(Parameters value) noexcept
{
    controls.centre = numeric::unit(value.centre, controls.centre);
    controls.separation = numeric::unit(value.separation, controls.separation);
    controls.resonance = numeric::unit(value.resonance, controls.resonance);
    controls.crossFeedback = numeric::unit(value.crossFeedback,
                                           controls.crossFeedback);
    controls.series = numeric::unit(value.series, controls.series);
    updateCoefficients();
}

void DualPeakFilter::updateCoefficients() noexcept
{
    const auto centreHz = 55.0f * std::exp2(controls.centre * 7.0f);
    const auto separationOctaves = 0.08f + controls.separation * 3.5f;
    const auto q = 0.65f + controls.resonance * 11.0f;
    const auto maximum = static_cast<float>(preparedSampleRate * 0.42);
    configureBandpass(peak[0], std::clamp(
        centreHz * std::exp2(-separationOctaves * 0.5f), 24.0f, maximum),
        q, preparedSampleRate);
    configureBandpass(peak[1], std::clamp(
        centreHz * std::exp2(separationOctaves * 0.5f), 24.0f, maximum),
        q, preparedSampleRate);
}

float DualPeakFilter::tick(float input) noexcept
{
    const auto safe = safeSignal(input);
    const auto feedback = controls.crossFeedback * 0.32f;
    const auto a = peak[0].tick(std::clamp(safe + previousB * feedback,
                                          -4.0f, 4.0f));
    const auto bInput = safe * (1.0f - controls.series)
        + a * controls.series + previousA * feedback;
    const auto b = peak[1].tick(std::clamp(bInput, -4.0f, 4.0f));
    previousA = std::clamp(a, -2.0f, 2.0f);
    previousB = std::clamp(b, -2.0f, 2.0f);
    const auto parallel = (a + b) * 0.65f;
    return std::clamp(parallel * (1.0f - controls.series) + b * controls.series,
                      -1.0f, 1.0f);
}

float ResonantFilterSequencer::Biquad::tick(float input) noexcept
{
    return tickBiquad(*this, input);
}

void ResonantFilterSequencer::prepare(double sampleRate) noexcept
{
    preparedSampleRate = numeric::sampleRate(sampleRate);
    updateCoefficients();
    reset();
}

void ResonantFilterSequencer::reset() noexcept
{
    for (auto& filter : filters) filter.reset();
}

void ResonantFilterSequencer::setParameters(Parameters value) noexcept
{
    controls.baseFrequencyHz = numeric::clampFinite(value.baseFrequencyHz,
        24.0f, static_cast<float>(preparedSampleRate * 0.08),
        controls.baseFrequencyHz);
    controls.resonance = numeric::unit(value.resonance, controls.resonance);
    updateCoefficients();
}

void ResonantFilterSequencer::updateCoefficients() noexcept
{
    const auto q = 0.8f + controls.resonance * 10.0f;
    const auto maximum = static_cast<float>(preparedSampleRate * 0.42);
    constexpr float majorSeventh = 11.0f / 12.0f;
    for (std::size_t index = 0; index < filters.size(); ++index)
        configureBandpass(filters[index], std::clamp(controls.baseFrequencyHz
            * std::exp2(majorSeventh * static_cast<float>(index)),
            24.0f, maximum), q, preparedSampleRate);
}

ResonantFilterSequencer::Output ResonantFilterSequencer::tick(
    float input, float rotationPhase, float encounter) noexcept
{
    const auto safe = safeSignal(input);
    std::array<float, bandCount> band {};
    for (std::size_t index = 0; index < filters.size(); ++index)
        band[index] = filters[index].tick(safe);
    const auto position = numeric::wrapUnit(rotationPhase) * bandCount;
    const auto primary = static_cast<std::size_t>(position) % bandCount;
    const auto secondary = (primary + bandCount / 2u) % bandCount;
    const auto encounterAmount = numeric::unit(encounter, 0.0f);
    const auto value = band[primary] * (1.0f - encounterAmount * 0.35f)
        + band[secondary] * encounterAmount * 0.75f;
    return { std::clamp(value * 1.6f, -1.0f, 1.0f), primary, secondary };
}

float FrequencyIsolator::Biquad::tick(float input) noexcept
{
    return tickBiquad(*this, input);
}

void FrequencyIsolator::prepare(double sampleRate) noexcept
{
    preparedSampleRate = numeric::sampleRate(sampleRate);
    updateCoefficients();
    reset();
}

void FrequencyIsolator::reset() noexcept
{
    for (auto& filter : filters) filter.reset();
}

void FrequencyIsolator::setParameters(Parameters value) noexcept
{
    controls.focus = numeric::unit(value.focus, controls.focus);
    controls.centre = numeric::unit(value.centre, controls.centre);
    controls.width = numeric::unit(value.width, controls.width);
    updateCoefficients();
}

void FrequencyIsolator::updateCoefficients() noexcept
{
    const auto frequency = std::clamp(45.0f * std::exp2(controls.centre * 8.0f),
        24.0f, static_cast<float>(preparedSampleRate * 0.40));
    const auto q = 0.65f + (1.0f - controls.width) * 8.5f;
    for (auto& filter : filters)
        configureBandpass(filter, frequency, q, preparedSampleRate);
}

float FrequencyIsolator::tick(float input) noexcept
{
    const auto safe = safeSignal(input);
    const auto isolated = filters[1].tick(filters[0].tick(safe)) * 2.4f;
    return std::clamp(safe * (1.0f - controls.focus)
        + isolated * controls.focus, -1.0f, 1.0f);
}

void InteractionProcessor::prepare(double sampleRate) noexcept
{
    const auto rate = numeric::sampleRate(sampleRate);
    smoothing = static_cast<float>(-std::expm1(-1.0 / (rate * 0.012)));
    reset();
}

void InteractionProcessor::reset() noexcept
{
    smoothedContact = smoothedPursuit = 0.0f;
}

void InteractionProcessor::setParameters(Parameters value) noexcept
{
    controls.amount = numeric::unit(value.amount, controls.amount);
    controls.harmonicRichness = numeric::unit(value.harmonicRichness,
                                              controls.harmonicRichness);
    controls.pursuitDucking = numeric::unit(value.pursuitDucking,
                                            controls.pursuitDucking);
    controls.mode = value.mode == Mode::xorLogic ? Mode::xorLogic : Mode::ring;
}

InteractionProcessor::Output InteractionProcessor::tick(
    float predator, float preyInput, float contact, float pursuit) noexcept
{
    const auto a = safeSignal(predator);
    const auto b = safeSignal(preyInput);
    smoothedContact += (numeric::unit(contact, 0.0f) - smoothedContact) * smoothing;
    smoothedPursuit += (numeric::unit(pursuit, 0.0f) - smoothedPursuit) * smoothing;
    const auto ring = std::clamp(a * b * 1.8f, -1.0f, 1.0f);
    const auto xorAmplitude = std::sqrt(std::abs(a * b));
    const auto xorSignal = ((a >= 0.0f) != (b >= 0.0f) ? 1.0f : -1.0f)
        * xorAmplitude;
    const auto logicBlend = controls.mode == Mode::xorLogic
        ? 0.55f + controls.harmonicRichness * 0.45f
        : controls.harmonicRichness * 0.35f;
    const auto artifact = (ring * (1.0f - logicBlend) + xorSignal * logicBlend)
        * controls.amount * smoothedContact;
    const auto preyGain = 1.0f - smoothedPursuit
        * controls.pursuitDucking * 0.82f;
    const auto preyOutput = b * preyGain;
    return { std::clamp(artifact, -1.0f, 1.0f), preyOutput,
             std::clamp(preyOutput + artifact, -1.0f, 1.0f) };
}

void PowerStarve::prepare(double sampleRate) noexcept
{
    const auto rate = numeric::sampleRate(sampleRate);
    smoothing = static_cast<float>(-std::expm1(-1.0 / (rate * 0.025)));
    reset();
}

void PowerStarve::setSeed(std::uint32_t value) noexcept
{
    seed = value == 0u ? 1u : value;
    reset();
}

void PowerStarve::reset() noexcept
{
    currentStress = 0.0f;
    held = 0.0f;
    counter = 0;
    randomState = seed;
}

void PowerStarve::setParameters(Parameters value) noexcept
{
    controls.stress = numeric::unit(value.stress, controls.stress);
    controls.depth = numeric::unit(value.depth, controls.depth);
    controls.instability = numeric::unit(value.instability, controls.instability);
}

float PowerStarve::tick(float input) noexcept
{
    const auto safe = safeSignal(input);
    currentStress += (controls.stress - currentStress) * smoothing;
    const auto effect = currentStress * controls.depth;
    const auto period = 1u + static_cast<std::uint32_t>(
        std::floor(effect * controls.instability * 31.0f));
    if (counter++ % period == 0u)
    {
        randomState = randomState * 1664525u + 1013904223u;
        const auto jitter = static_cast<float>((randomState >> 16u) & 0xffffu)
            / 65535.0f;
        const auto levels = 16.0f + (1.0f - effect) * 4080.0f;
        held = std::round((safe + (jitter - 0.5f) * effect * 0.025f) * levels)
            / levels;
    }
    const auto supply = 1.0f - effect * 0.82f;
    const auto starved = held * supply;
    return std::clamp(safe * (1.0f - effect) + starved * effect,
                      -1.0f, 1.0f);
}

QuadratureStereoRotator::Output QuadratureStereoRotator::process(
    float left, float right, float phase) noexcept
{
    const auto angle = numeric::wrapUnit(phase) * 2.0f * pi;
    const auto cosine = std::cos(angle);
    const auto sine = std::sin(angle);
    const auto safeLeft = safeSignal(left);
    const auto safeRight = safeSignal(right);
    return { std::clamp(safeLeft * cosine - safeRight * sine, -1.0f, 1.0f),
             std::clamp(safeLeft * sine + safeRight * cosine, -1.0f, 1.0f) };
}

void PolygonalOscillator::prepare(double sampleRate) noexcept
{
    preparedSampleRate = numeric::sampleRate(sampleRate);
    reset();
}

void PolygonalOscillator::reset(float value) noexcept
{
    phase = numeric::wrapUnit(value);
}

void PolygonalOscillator::setParameters(Parameters value) noexcept
{
    controls.frequencyHz = numeric::clampFinite(value.frequencyHz, 0.01f,
        static_cast<float>(preparedSampleRate * 0.10), controls.frequencyHz);
    controls.vertices = std::clamp<std::size_t>(value.vertices, 3u, 16u);
    controls.roundness = numeric::unit(value.roundness, controls.roundness);
    controls.rotation = numeric::wrapUnit(value.rotation, controls.rotation);
}

PolygonalOscillator::Output PolygonalOscillator::tick() noexcept
{
    phase += controls.frequencyHz / preparedSampleRate;
    phase -= std::floor(phase);
    const auto angle = static_cast<float>((phase + controls.rotation)
        * 2.0 * std::numbers::pi);
    const auto sector = 2.0f * pi / static_cast<float>(controls.vertices);
    auto local = std::fmod(angle + pi, sector);
    if (local < 0.0f) local += sector;
    local -= sector * 0.5f;
    const auto polygonRadius = std::cos(pi / static_cast<float>(controls.vertices))
        / std::max(0.05f, std::cos(local));
    const auto radius = polygonRadius * (1.0f - controls.roundness)
        + controls.roundness;
    const auto x = std::clamp(radius * std::cos(angle), -1.0f, 1.0f);
    const auto y = std::clamp(radius * std::sin(angle), -1.0f, 1.0f);
    return { std::clamp((x + y) * 0.70710678f, -1.0f, 1.0f), x, y };
}
}
