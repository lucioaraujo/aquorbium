// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/GenerativeInteractionModules.h"

#include "core/Numeric.h"

#include <algorithm>
#include <bit>
#include <cmath>
#include <numbers>

namespace aquorbium
{
namespace
{
constexpr float pi = std::numbers::pi_v<float>;
constexpr float twoPi = 2.0f * pi;

float safeAudio(float value) noexcept
{
    return numeric::clampFinite(value, -2.0f, 2.0f, 0.0f);
}
}

bool FixedLSystemGenerator::configure(std::size_t iterations,
                                      bool alternateRule) noexcept
{
    std::array<Symbol, capacity> current { Symbol::forward };
    std::array<Symbol, capacity> next {};
    auto count = std::size_t { 1 };
    iterations = std::min<std::size_t>(iterations, 4u);
    constexpr std::array<Symbol, 9> primary {
        Symbol::forward, Symbol::left, Symbol::forward, Symbol::right,
        Symbol::forward, Symbol::right, Symbol::forward, Symbol::left,
        Symbol::forward
    };
    constexpr std::array<Symbol, 7> alternate {
        Symbol::forward, Symbol::left, Symbol::forward, Symbol::right,
        Symbol::forward, Symbol::left, Symbol::forward
    };

    for (std::size_t iteration = 0; iteration < iterations; ++iteration)
    {
        auto nextCount = std::size_t { 0 };
        for (std::size_t index = 0; index < count; ++index)
        {
            if (current[index] != Symbol::forward)
            {
                if (nextCount >= capacity) return false;
                next[nextCount++] = current[index];
                continue;
            }
            const auto append = [&](const auto& rule) noexcept {
                if (nextCount + rule.size() > capacity) return false;
                for (const auto symbol : rule) next[nextCount++] = symbol;
                return true;
            };
            if (!(alternateRule ? append(alternate) : append(primary)))
                return false;
        }
        current = next;
        next.fill(Symbol::forward);
        count = nextCount;
    }
    symbols = current;
    symbolCount = count;
    cursor = 0;
    return true;
}

void FixedLSystemGenerator::reset() noexcept
{
    cursor = 0;
}

FixedLSystemGenerator::Event FixedLSystemGenerator::advance() noexcept
{
    if (symbolCount == 0) return {};
    const auto position = cursor;
    const auto symbol = symbols[cursor];
    cursor = (cursor + 1u) % symbolCount;
    if (symbol == Symbol::forward)
        return { true, 0, 1.0f, position };
    if (symbol == Symbol::left)
        return { false, -1, -0.5f, position };
    return { false, 1, 0.5f, position };
}

void CellularAutomatonSequencer::configure(std::uint8_t newRule,
                                            std::uint32_t newState) noexcept
{
    rule = newRule;
    initialState = newState == 0u ? 1u << 15u : newState;
    reset();
}

void CellularAutomatonSequencer::reset() noexcept
{
    state = initialState;
}

CellularAutomatonSequencer::Output CellularAutomatonSequencer::advance() noexcept
{
    std::uint32_t next = 0u;
    for (std::uint32_t index = 0; index < 32u; ++index)
    {
        const auto left = (state >> ((index + 31u) & 31u)) & 1u;
        const auto centre = (state >> index) & 1u;
        const auto right = (state >> ((index + 1u) & 31u)) & 1u;
        const auto neighbourhood = static_cast<std::uint8_t>(
            (left << 2u) | (centre << 1u) | right);
        const auto bit = (rule >> neighbourhood) & 1u;
        next |= static_cast<std::uint32_t>(bit) << index;
    }
    state = next == 0u ? initialState : next;
    return { (state & (1u << 15u)) != 0u,
             static_cast<float>(std::popcount(state)) / 32.0f, state };
}

float SpectralTransferVocoder::Biquad::tick(float input) noexcept
{
    const auto output = b0 * input + z1;
    z1 = b1 * input - a1 * output + z2;
    z2 = b2 * input - a2 * output;
    return output;
}

void SpectralTransferVocoder::prepare(double newSampleRate) noexcept
{
    sampleRate = numeric::sampleRate(newSampleRate);
    setParameters(controls);
    reset();
}

void SpectralTransferVocoder::reset() noexcept
{
    envelope.fill(0.0f);
    for (auto& filter : modulatorFilters) filter.reset();
    for (auto& filter : carrierFilters) filter.reset();
}

void SpectralTransferVocoder::setParameters(Parameters value) noexcept
{
    controls.centre = numeric::unit(value.centre, controls.centre);
    controls.span = numeric::unit(value.span, controls.span);
    controls.resonance = numeric::unit(value.resonance, controls.resonance);
    controls.response = numeric::unit(value.response, controls.response);
    controls.mix = numeric::unit(value.mix, controls.mix);
    envelopeCoefficient = static_cast<float>(-std::expm1(-1.0
        / (sampleRate * (0.004 + controls.response * 0.120))));
    updateCoefficients();
}

void SpectralTransferVocoder::updateCoefficients() noexcept
{
    const auto centreHz = 80.0f * std::exp2(controls.centre * 5.2f);
    const auto spanOctaves = 0.8f + controls.span * 4.5f;
    const auto q = 0.7f + controls.resonance * 8.0f;
    for (std::size_t index = 0; index < bandCount; ++index)
    {
        const auto position = static_cast<float>(index)
            / static_cast<float>(bandCount - 1u) - 0.5f;
        const auto frequency = std::clamp(centreHz
            * std::exp2(position * spanOctaves), 24.0f,
            static_cast<float>(sampleRate * 0.42));
        const auto omega = twoPi * frequency / static_cast<float>(sampleRate);
        const auto alpha = std::sin(omega) / (2.0f * q);
        const auto inverseA0 = 1.0f / (1.0f + alpha);
        Biquad coefficients;
        coefficients.b0 = alpha * inverseA0;
        coefficients.b1 = 0.0f;
        coefficients.b2 = -alpha * inverseA0;
        coefficients.a1 = -2.0f * std::cos(omega) * inverseA0;
        coefficients.a2 = (1.0f - alpha) * inverseA0;
        coefficients.z1 = modulatorFilters[index].z1;
        coefficients.z2 = modulatorFilters[index].z2;
        modulatorFilters[index] = coefficients;
        coefficients.z1 = carrierFilters[index].z1;
        coefficients.z2 = carrierFilters[index].z2;
        carrierFilters[index] = coefficients;
    }
}

float SpectralTransferVocoder::tick(float modulator, float carrier) noexcept
{
    const auto safeModulator = safeAudio(modulator);
    const auto safeCarrier = safeAudio(carrier);
    auto wet = 0.0f;
    for (std::size_t index = 0; index < bandCount; ++index)
    {
        const auto modulated = modulatorFilters[index].tick(safeModulator);
        envelope[index] += (std::abs(modulated) - envelope[index])
            * envelopeCoefficient;
        wet += carrierFilters[index].tick(safeCarrier) * envelope[index];
    }
    wet = std::tanh(wet * (2.2f / static_cast<float>(bandCount)));
    return std::clamp(safeCarrier * (1.0f - controls.mix)
        + wet * controls.mix, -1.0f, 1.0f);
}

void EnvelopeContagionBus::prepare(double newSampleRate) noexcept
{
    const auto rate = numeric::sampleRate(newSampleRate);
    attack = static_cast<float>(-std::expm1(-1.0 / (rate * 0.008)));
    release = static_cast<float>(-std::expm1(-1.0 / (rate * 0.120)));
    reset();
}

void EnvelopeContagionBus::reset() noexcept
{
    envelope.fill(0.0f);
}

EnvelopeContagionBus::Members EnvelopeContagionBus::process(
    const Members& input, float contagion) noexcept
{
    contagion = numeric::unit(contagion, 0.0f);
    for (std::size_t index = 0; index < memberCount; ++index)
    {
        const auto magnitude = std::abs(safeAudio(input[index]));
        envelope[index] += (magnitude - envelope[index])
            * (magnitude > envelope[index] ? attack : release);
    }
    Members output {};
    for (std::size_t index = 0; index < memberCount; ++index)
    {
        const auto previous = envelope[(index + memberCount - 1u) % memberCount];
        const auto next = envelope[(index + 1u) % memberCount];
        output[index] = std::clamp(envelope[index] * (1.0f - contagion)
            + (previous + next) * 0.5f * contagion, 0.0f, 1.0f);
    }
    return output;
}

void NoInputFeedbackBus::prepare(double newSampleRate) noexcept
{
    sampleRate = numeric::sampleRate(newSampleRate);
    setParameters(controls);
    reset();
}

void NoInputFeedbackBus::setSeed(std::uint32_t value) noexcept
{
    seed = value == 0u ? 1u : value;
    reset();
}

void NoInputFeedbackBus::reset() noexcept
{
    leftState.fill(0.0f);
    rightState.fill(0.0f);
    feedbackLeft = 0.0001f;
    feedbackRight = -0.0001f;
    randomState = seed;
}

void NoInputFeedbackBus::setParameters(Parameters value) noexcept
{
    controls.feedback = numeric::clampFinite(value.feedback, 0.0f, 0.985f,
                                               controls.feedback);
    controls.drive = numeric::unit(value.drive, controls.drive);
    controls.colour = numeric::unit(value.colour, controls.colour);
    controls.level = numeric::unit(value.level, controls.level);
    const std::array<float, 3> frequency {
        41.0f + controls.colour * 67.0f,
        270.0f + controls.colour * 610.0f,
        1700.0f + controls.colour * 3400.0f
    };
    for (std::size_t index = 0; index < coefficient.size(); ++index)
        coefficient[index] = 1.0f - std::exp(-twoPi * frequency[index]
            / static_cast<float>(sampleRate));
}

float NoInputFeedbackBus::noise() noexcept
{
    auto value = randomState;
    value ^= value << 13u;
    value ^= value >> 17u;
    value ^= value << 5u;
    randomState = value == 0u ? 1u : value;
    return (static_cast<float>(randomState) / 4294967295.0f * 2.0f - 1.0f)
        * 0.012f;
}

NoInputFeedbackBus::Output NoInputFeedbackBus::tick() noexcept
{
    const auto drive = 1.0f + controls.drive * 7.0f;
    const auto excitation = noise();
    const auto leftLoop = std::tanh((feedbackLeft * controls.feedback
        + feedbackRight * 0.14f + excitation) * drive);
    const auto rightLoop = std::tanh((feedbackRight * controls.feedback
        - feedbackLeft * 0.14f - excitation * 0.83f) * drive);
    for (std::size_t index = 0; index < leftState.size(); ++index)
    {
        leftState[index] += (leftLoop - leftState[index]) * coefficient[index];
        rightState[index] += (rightLoop - rightState[index]) * coefficient[index];
    }
    feedbackLeft = std::tanh((leftState[0] - leftState[1] + leftState[2])
        * 1.35f);
    feedbackRight = std::tanh((rightState[0] - rightState[1] + rightState[2])
        * 1.35f);
    return { std::clamp(feedbackLeft * controls.level, -1.0f, 1.0f),
             std::clamp(feedbackRight * controls.level, -1.0f, 1.0f) };
}

void ReactiveThresholdStrategy::setThresholds(const Thresholds& value,
                                               float newHysteresis) noexcept
{
    auto previous = 0.0f;
    for (std::size_t index = 0; index < thresholds.size(); ++index)
    {
        thresholds[index] = std::max(previous + (index == 0 ? 0.0f : 0.01f),
            numeric::unit(value[index], thresholds[index]));
        thresholds[index] = std::min(thresholds[index],
            1.0f - 0.01f * static_cast<float>(thresholds.size() - 1u - index));
        previous = thresholds[index];
    }
    hysteresis = numeric::clampFinite(newHysteresis, 0.0f, 0.1f, hysteresis);
}

void ReactiveThresholdStrategy::reset(float stimulus) noexcept
{
    current = select(numeric::unit(stimulus, 0.0f));
}

std::size_t ReactiveThresholdStrategy::select(float stimulus) const noexcept
{
    auto result = std::size_t { 0 };
    while (result < thresholds.size() && stimulus >= thresholds[result]) ++result;
    return result;
}

ReactiveThresholdStrategy::Output ReactiveThresholdStrategy::tick(
    float stimulus) noexcept
{
    stimulus = numeric::unit(stimulus, 0.0f);
    const auto candidate = select(stimulus);
    const auto previous = current;
    if (candidate > current && current < thresholds.size()
        && stimulus >= thresholds[current] + hysteresis)
        current = candidate;
    else if (candidate < current && current > 0u
        && stimulus <= thresholds[current - 1u] - hysteresis)
        current = candidate;
    return { current, static_cast<float>(current)
        / static_cast<float>(stateCount - 1u), current != previous };
}

void LockInFocusExtractor::prepare(double newSampleRate) noexcept
{
    sampleRate = numeric::sampleRate(newSampleRate);
    setParameters(controls);
    reset();
}

void LockInFocusExtractor::reset() noexcept
{
    phase = inPhase = quadrature = 0.0f;
}

void LockInFocusExtractor::setParameters(Parameters value) noexcept
{
    controls.referenceHz = numeric::clampFinite(value.referenceHz, 4.0f,
        static_cast<float>(sampleRate * 0.22), controls.referenceHz);
    controls.bandwidthHz = numeric::clampFinite(value.bandwidthHz, 0.5f,
        static_cast<float>(sampleRate * 0.05), controls.bandwidthHz);
    controls.mix = numeric::unit(value.mix, controls.mix);
    controls.gain = numeric::unit(value.gain, controls.gain);
    phaseIncrement = controls.referenceHz / static_cast<float>(sampleRate);
    lowpassCoefficient = 1.0f - std::exp(-twoPi * controls.bandwidthHz
        / static_cast<float>(sampleRate));
}

float LockInFocusExtractor::tick(float input) noexcept
{
    const auto safe = safeAudio(input);
    const auto sine = std::sin(twoPi * phase);
    const auto cosine = std::cos(twoPi * phase);
    phase += phaseIncrement;
    phase -= std::floor(phase);
    inPhase += (safe * cosine - inPhase) * lowpassCoefficient;
    quadrature += (safe * sine - quadrature) * lowpassCoefficient;
    const auto focused = (inPhase * cosine + quadrature * sine) * 2.0f
        * controls.gain;
    return std::clamp(safe * (1.0f - controls.mix)
        + focused * controls.mix, -1.0f, 1.0f);
}

float LockInFocusExtractor::magnitude() const noexcept
{
    return std::clamp(std::sqrt(inPhase * inPhase
        + quadrature * quadrature) * 2.0f, 0.0f, 1.0f);
}

void BehaviourSceneBank::prepare(double newSampleRate) noexcept
{
    sampleRate = numeric::sampleRate(newSampleRate);
    reset();
}

void BehaviourSceneBank::reset() noexcept
{
    selected = 0;
    current = scenes[0];
    target = current;
    coefficient = 1.0f;
}

bool BehaviourSceneBank::setScene(std::size_t index,
                                  const Scene& scene) noexcept
{
    if (index >= sceneCount) return false;
    for (std::size_t dimension = 0; dimension < dimensionCount; ++dimension)
        scenes[index][dimension] = numeric::unit(scene[dimension],
                                                  scenes[index][dimension]);
    if (index == selected) target = scenes[index];
    return true;
}

bool BehaviourSceneBank::recall(std::size_t index,
                                float transitionSeconds) noexcept
{
    if (index >= sceneCount) return false;
    selected = index;
    target = scenes[index];
    transitionSeconds = numeric::clampFinite(transitionSeconds, 0.0f, 10.0f,
                                              0.1f);
    coefficient = transitionSeconds <= 0.0f ? 1.0f
        : static_cast<float>(-std::expm1(-1.0
            / (sampleRate * transitionSeconds)));
    return true;
}

BehaviourSceneBank::Scene BehaviourSceneBank::tick() noexcept
{
    for (std::size_t index = 0; index < dimensionCount; ++index)
    {
        current[index] += (target[index] - current[index]) * coefficient;
        if (std::abs(target[index] - current[index]) < 1.0e-7f)
            current[index] = target[index];
    }
    return current;
}

void ReconfigurableResourcePool::reset() noexcept
{
    writeIndex = 0;
    held = 0.0f;
}

bool ReconfigurableResourcePool::setValue(std::size_t index,
                                          float value) noexcept
{
    if (index >= valueCount) return false;
    values[index] = numeric::clampFinite(value, -1.0f, 1.0f, values[index]);
    return true;
}

float ReconfigurableResourcePool::tick(float input, float address,
                                       bool trigger) noexcept
{
    input = numeric::clampFinite(input, -1.0f, 1.0f, 0.0f);
    address = numeric::unit(address, 0.0f);
    const auto scaled = address * static_cast<float>(valueCount - 1u);
    const auto index = static_cast<std::size_t>(scaled);
    if (mode == Mode::mixer)
    {
        const auto next = std::min(index + 1u, valueCount - 1u);
        const auto fraction = scaled - static_cast<float>(index);
        return values[index] + (values[next] - values[index]) * fraction;
    }
    if (mode == Mode::switcher)
        return values[index];
    if (mode == Mode::recorder)
    {
        if (trigger)
        {
            values[writeIndex] = input;
            held = input;
            writeIndex = (writeIndex + 1u) % valueCount;
        }
        return held;
    }
    return values[(index * 5u + 3u) % valueCount];
}

void LightweightBinauralSpatializer::prepare(double newSampleRate) noexcept
{
    sampleRate = numeric::sampleRate(newSampleRate);
    lowCoefficient = 1.0f - std::exp(-twoPi * 320.0f
        / static_cast<float>(sampleRate));
    setParameters(controls);
    reset();
}

void LightweightBinauralSpatializer::reset() noexcept
{
    delay.fill(0.0f);
    writeIndex = 0;
    low = 0.0f;
}

void LightweightBinauralSpatializer::setParameters(Parameters value) noexcept
{
    controls.azimuth = numeric::clampFinite(value.azimuth, -1.0f, 1.0f,
                                             controls.azimuth);
    controls.elevation = numeric::clampFinite(value.elevation, -1.0f, 1.0f,
                                               controls.elevation);
    controls.distance = numeric::unit(value.distance, controls.distance);
    controls.amount = numeric::unit(value.amount, controls.amount);
}

float LightweightBinauralSpatializer::readDelay(float delaySamples) const noexcept
{
    delaySamples = std::clamp(delaySamples, 0.0f,
                              static_cast<float>(delayCapacity - 2u));
    const auto whole = static_cast<std::size_t>(delaySamples);
    const auto fraction = delaySamples - static_cast<float>(whole);
    const auto first = (writeIndex + delayCapacity - whole) % delayCapacity;
    const auto second = (first + delayCapacity - 1u) % delayCapacity;
    return delay[first] + (delay[second] - delay[first]) * fraction;
}

LightweightBinauralSpatializer::Output LightweightBinauralSpatializer::tick(
    float input) noexcept
{
    const auto safe = safeAudio(input);
    if (controls.amount <= 0.0f) return { safe, safe };
    low += (safe - low) * lowCoefficient;
    const auto high = safe - low;
    delay[writeIndex] = high;
    const auto maximumDelay = std::min(0.00072f * static_cast<float>(sampleRate),
        static_cast<float>(delayCapacity - 2u));
    const auto leftHigh = readDelay(std::max(0.0f, controls.azimuth)
        * maximumDelay);
    const auto rightHigh = readDelay(std::max(0.0f, -controls.azimuth)
        * maximumDelay);
    writeIndex = (writeIndex + 1u) % delayCapacity;

    const auto pan = (controls.azimuth + 1.0f) * pi * 0.25f;
    auto leftGain = std::cos(pan);
    auto rightGain = std::sin(pan);
    const auto shadow = 1.0f - std::abs(controls.azimuth) * 0.38f;
    if (controls.azimuth > 0.0f) leftGain *= shadow;
    else if (controls.azimuth < 0.0f) rightGain *= shadow;
    const auto elevationColour = 0.82f + controls.elevation * 0.18f;
    const auto distanceGain = 1.0f / (1.0f + controls.distance * 1.6f);
    const auto wetLeft = (low * 0.72f + leftHigh * leftGain
        * elevationColour) * distanceGain;
    const auto wetRight = (low * 0.72f + rightHigh * rightGain
        * elevationColour) * distanceGain;
    return { std::clamp(safe * (1.0f - controls.amount)
                + wetLeft * controls.amount, -1.0f, 1.0f),
             std::clamp(safe * (1.0f - controls.amount)
                + wetRight * controls.amount, -1.0f, 1.0f) };
}
}
