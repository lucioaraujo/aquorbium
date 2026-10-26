// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/AdvancedVoiceEcologyModules.h"

#include "core/Numeric.h"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace aquorbium
{
namespace
{
constexpr float pi = std::numbers::pi_v<float>;
constexpr float twoPi = 2.0f * pi;

float safe(float value) noexcept
{
    return numeric::clampFinite(value, -4.0f, 4.0f, 0.0f);
}

float advancePhase(float phase, float increment) noexcept
{
    phase += increment;
    return phase - std::floor(phase);
}
}

void BiomaBassVoice::prepare(double newSampleRate) noexcept
{
    sampleRate = numeric::sampleRate(newSampleRate);
    drift.prepare(sampleRate);
    drift.setParameters({ 0.42f, 0.38f, 0.28f });
    energyAttack = static_cast<float>(-std::expm1(-1.0 / (sampleRate * 0.008)));
    energyRelease = static_cast<float>(-std::expm1(-1.0 / (sampleRate * 0.180)));
    setParameters(controls);
    reset();
}

void BiomaBassVoice::setSeed(std::uint32_t value) noexcept
{
    drift.setSeed((value == 0u ? 1u : value) ^ 0x48f2a19bu);
    reset();
}

void BiomaBassVoice::reset() noexcept
{
    phase = { 0.0f, 0.31f, 0.67f };
    modulatorPhase = subPhase = 0.0f;
    filterState.fill(0.0f);
    state = {};
    drift.reset();
    folder.reset();
}

void BiomaBassVoice::setParameters(Parameters value) noexcept
{
    controls.frequencyHz = numeric::clampFinite(value.frequencyHz, 18.0f,
        static_cast<float>(sampleRate * 0.16), controls.frequencyHz);
    controls.fmRatio = numeric::clampFinite(value.fmRatio, 0.125f, 8.0f,
                                             controls.fmRatio);
    controls.fmDepth = numeric::unit(value.fmDepth, controls.fmDepth);
    controls.detuneCents = numeric::clampFinite(value.detuneCents, 0.0f, 64.0f,
                                                 controls.detuneCents);
    controls.subSemitones = numeric::clampFinite(value.subSemitones,
        -36.0f, -5.0f, controls.subSemitones);
    controls.fold = numeric::unit(value.fold, controls.fold);
    controls.cutoff = numeric::unit(value.cutoff, controls.cutoff);
    controls.resonance = numeric::unit(value.resonance, controls.resonance);
    controls.poles = std::clamp<std::size_t>(value.poles, 2u, 4u);
    controls.energyControl = numeric::unit(value.energyControl,
                                            controls.energyControl);
    const auto cutoffHz = std::clamp(32.0f * std::exp2(controls.cutoff * 8.5f),
        24.0f, static_cast<float>(sampleRate * 0.42));
    filterCoefficient = 1.0f - std::exp(
        -twoPi * cutoffHz / static_cast<float>(sampleRate));
    subIncrement = controls.frequencyHz * std::exp2(
        controls.subSemitones / 12.0f) / static_cast<float>(sampleRate);
}

float BiomaBassVoice::polyBlep(float phaseValue, float increment) noexcept
{
    if (increment <= 0.0f) return 0.0f;
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

float BiomaBassVoice::saw(float phaseValue, float increment) noexcept
{
    return phaseValue * 2.0f - 1.0f - polyBlep(phaseValue, increment);
}

float BiomaBassVoice::tick() noexcept
{
    const auto reduction = std::clamp(state.energy * controls.energyControl
        * 2.2f, 0.0f, 0.88f);
    state.effectiveFold = controls.fold * (1.0f - reduction * 0.78f);
    state.effectiveDetuneCents = controls.detuneCents
        * (1.0f - reduction * 0.72f);
    const auto driftValue = drift.tick() * 0.35f;
    const auto modulator = std::sin(twoPi * modulatorPhase);
    const auto instantaneous = controls.frequencyHz
        * (1.0f + modulator * controls.fmDepth * 1.45f);
    state.instantaneousFrequencyHz = instantaneous;
    modulatorPhase = advancePhase(modulatorPhase,
        controls.frequencyHz * controls.fmRatio / static_cast<float>(sampleRate));

    const std::array<float, 3> spread { -1.0f, 0.0f, 1.0f };
    auto body = 0.0f;
    for (std::size_t index = 0; index < phase.size(); ++index)
    {
        const auto cents = spread[index] * state.effectiveDetuneCents
            * (1.0f + driftValue);
        const auto frequency = instantaneous
            * std::max(0.9f, 1.0f + cents * 0.00057762265f);
        const auto signedIncrement = std::clamp(
            frequency / static_cast<float>(sampleRate), -0.22f, 0.22f);
        body += saw(phase[index], std::abs(signedIncrement));
        phase[index] = advancePhase(phase[index], signedIncrement);
    }
    body /= 3.0f;
    const auto sub = std::sin(twoPi * subPhase);
    subPhase = advancePhase(subPhase, subIncrement);
    const auto raw = body * 0.72f + sub * 0.38f;

    // The non-linearity deliberately precedes the filter. Gain compensation
    // keeps increased folding from becoming a hidden loudness control.
    const auto foldDrive = 1.0f + state.effectiveFold * 7.0f;
    const auto folded = folder.process(raw, state.effectiveFold, foldDrive);
    const auto compensated = folded
        / std::sqrt(1.0f + state.effectiveFold * 2.8f);

    auto filteredInput = compensated
        - filterState[controls.poles - 1u] * controls.resonance * 0.72f;
    filteredInput = std::tanh(filteredInput * (1.0f + controls.resonance));
    for (auto& stage : filterState)
    {
        stage += (filteredInput - stage) * filterCoefficient;
        filteredInput = stage;
    }
    const auto selected = filterState[controls.poles - 1u];
    const auto fundamental = std::sin(twoPi * phase[1]);
    const auto output = std::clamp(std::tanh(selected * 1.35f
        + fundamental * controls.resonance * 0.16f), -1.0f, 1.0f);
    const auto magnitude = std::abs(output);
    state.energy += (magnitude - state.energy)
        * (magnitude > state.energy ? energyAttack : energyRelease);
    return output;
}

void SchoolWaveVoice::prepare(double newSampleRate) noexcept
{
    sampleRate = numeric::sampleRate(newSampleRate);
    setParameters(controls);
    reset();
}

void SchoolWaveVoice::reset() noexcept
{
    for (std::size_t index = 0; index < phase.size(); ++index)
        phase[index] = static_cast<float>(index) / static_cast<float>(phase.size());
}

void SchoolWaveVoice::setParameters(Parameters value) noexcept
{
    controls.frequencyHz = numeric::clampFinite(value.frequencyHz, 12.0f,
        static_cast<float>(sampleRate * 0.18), controls.frequencyHz);
    controls.morph = numeric::unit(value.morph, controls.morph);
    controls.waveCut = numeric::unit(value.waveCut, controls.waveCut);
    controls.spread = numeric::unit(value.spread, controls.spread);
    controls.level = numeric::unit(value.level, controls.level);
}

float SchoolWaveVoice::polyBlep(float phaseValue, float increment) noexcept
{
    if (phaseValue < increment)
    {
        const auto x = phaseValue / std::max(increment, 1.0e-6f);
        return x + x - x * x - 1.0f;
    }
    if (phaseValue > 1.0f - increment)
    {
        const auto x = (phaseValue - 1.0f) / std::max(increment, 1.0e-6f);
        return x * x + x + x + 1.0f;
    }
    return 0.0f;
}

float SchoolWaveVoice::waveform(float phaseValue, float increment,
                                float morph, float cut) noexcept
{
    // the base shape, naive: sine -> triangle -> saw
    const auto shape = [morph](float at) {
        const auto sine = std::sin(twoPi * at);
        const auto triangle = 1.0f - 4.0f * std::abs(at - 0.5f);
        const auto sawValue = at * 2.0f - 1.0f;
        return morph < 0.5f
            ? sine + (triangle - sine) * (morph * 2.0f)
            : triangle + (sawValue - triangle) * ((morph - 0.5f) * 2.0f);
    };
    // the wave cut scales the part after the boundary by `after`; the cut
    // and the wrap are the two jumps of the result, and both get a polyBLEP
    // residual of their own height (2026-10-23: without it the cut aliased
    // like a naive sawtooth, -10 to -28 dB off the harmonic grid,
    // AliasingMeasureTests). With the cut centred this is the former polyBLEP
    // saw exactly.
    const auto boundary = 0.12f + cut * 0.76f;
    const auto cutDepth = std::abs(cut - 0.5f) * 1.5f;
    const auto after = 1.0f - 1.72f * cutDepth;
    auto value = shape(phaseValue) * (phaseValue < boundary ? 1.0f : after);
    const auto wrapJump = shape(0.0f) - after * shape(1.0f);
    value += 0.5f * wrapJump * polyBlep(phaseValue, increment);
    if (cutDepth > 0.0f)
    {
        const auto cutJump = (after - 1.0f) * shape(boundary);
        auto relative = phaseValue - boundary;
        relative -= std::floor(relative);
        value += 0.5f * cutJump * polyBlep(relative, increment);
    }
    return std::clamp(value, -1.0f, 1.0f);
}

SchoolWaveVoice::Output SchoolWaveVoice::tick(float sharedDirection,
                                               float sharedPhase) noexcept
{
    const auto direction = numeric::clampFinite(sharedDirection,
        -1.0f, 1.0f, 0.0f);
    const auto shared = numeric::wrapUnit(sharedPhase);
    Output output;
    for (std::size_t index = 0; index < phase.size(); ++index)
    {
        const auto relative = (static_cast<float>(index) - 2.5f) / 2.5f;
        const auto cents = relative * controls.spread * 34.0f
            + direction * relative * 7.0f;
        const auto frequency = controls.frequencyHz
            * std::max(0.92f, 1.0f + cents * 0.00057762265f);
        const auto increment = std::clamp(
            frequency / static_cast<float>(sampleRate), 0.0f, 0.22f);
        const auto value = waveform(numeric::wrapUnit(phase[index]
            + shared * relative * 0.08f), increment, controls.morph,
            controls.waveCut);
        const auto pan = (relative + 1.0f) * pi * 0.25f;
        output.left += value * std::cos(pan);
        output.right += value * std::sin(pan);
        phase[index] = advancePhase(phase[index], increment);
    }
    const auto gain = controls.level * 0.24f;
    output.left = std::clamp(output.left * gain, -1.0f, 1.0f);
    output.right = std::clamp(output.right * gain, -1.0f, 1.0f);
    return output;
}

void HarmonicRippleVoice::prepare(double newSampleRate) noexcept
{
    sampleRate = numeric::sampleRate(newSampleRate);
    setParameters(controls);
    reset();
}

void HarmonicRippleVoice::reset() noexcept
{
    phase = 0.0;
}

void HarmonicRippleVoice::setParameters(Parameters value) noexcept
{
    controls.frequencyHz = numeric::clampFinite(value.frequencyHz, 12.0f,
        static_cast<float>(sampleRate * 0.08), controls.frequencyHz);
    controls.ripples = std::clamp<std::size_t>(value.ripples, 1u, 8u);
    controls.formant = numeric::unit(value.formant, controls.formant);
    controls.cluster = numeric::unit(value.cluster, controls.cluster);
    controls.level = numeric::unit(value.level, controls.level);
}

float HarmonicRippleVoice::tick() noexcept
{
    phase += controls.frequencyHz / sampleRate;
    phase -= std::floor(phase);
    const auto pulseDistance = std::abs(static_cast<float>(phase) * 2.0f - 1.0f);
    const auto envelope = std::pow(std::max(0.0f, 1.0f - pulseDistance),
        2.0f + controls.cluster * 10.0f);
    const auto centre = 2.0f + controls.formant * 7.0f;
    auto ripple = 0.0f;
    auto weightSum = 0.0f;
    const auto angle = twoPi * static_cast<float>(phase);
    const auto sinAngle = std::sin(angle);
    const auto cosAngle = std::cos(angle);
    auto sinPrevious = sinAngle;
    auto sinCurrent = 2.0f * sinAngle * cosAngle;
    // harmonics fade out between 0.30 and 0.40 of the sample rate, so none
    // folds back from above Nyquist (2026-10-23: at 3 kHz the ninth harmonic
    // aliased, -29 dB off the harmonic grid, AliasingMeasureTests); the sum
    // keeps the full weights, so a fading harmonic lowers, never reshapes,
    // the ripple
    const auto fundamentalRatio = static_cast<float>(controls.frequencyHz / sampleRate);
    for (std::size_t index = 0; index < controls.ripples; ++index)
    {
        const auto harmonic = static_cast<float>(index + 2u);
        const auto weight = std::exp(-std::abs(harmonic - centre)
            * (0.28f + controls.cluster * 0.22f));
        const auto band = std::clamp((0.40f - harmonic * fundamentalRatio) * 10.0f, 0.0f, 1.0f);
        ripple += sinCurrent * weight * band;
        weightSum += weight;
        const auto next = 2.0f * cosAngle * sinCurrent - sinPrevious;
        sinPrevious = sinCurrent;
        sinCurrent = next;
    }
    const auto fundamental = sinAngle;
    const auto normalized = weightSum > 0.0f ? ripple / weightSum : 0.0f;
    return std::clamp((fundamental * (1.0f - controls.cluster * 0.55f)
        + normalized * envelope * (0.35f + controls.cluster * 0.65f))
        * controls.level, -1.0f, 1.0f);
}

void RecentMixWavetable::prepare(double newSampleRate) noexcept
{
    sampleRate = numeric::sampleRate(newSampleRate);
    setParameters(controls);
    reset();
}

void RecentMixWavetable::reset() noexcept
{
    history.fill(0.0f);
    for (auto& item : table) item.fill(0.0f);
    written = 0;
    activeTable = 0;
    captureIndex = tableSize;
    captureStart = 0;
    phase = 0.0f;
    hasTable = false;
}

void RecentMixWavetable::setParameters(Parameters value) noexcept
{
    controls.frequencyHz = numeric::clampFinite(value.frequencyHz, 8.0f,
        static_cast<float>(sampleRate * 0.16), controls.frequencyHz);
    controls.colour = numeric::unit(value.colour, controls.colour);
    controls.level = numeric::unit(value.level, controls.level);
}

bool RecentMixWavetable::triggerCapture() noexcept
{
    if (written < tableSize || captureIndex < tableSize) return false;
    captureStart = static_cast<std::size_t>(written - tableSize)
        & (historySize - 1u);
    captureIndex = 0;
    return true;
}

RecentMixWavetable::Output RecentMixWavetable::tick(float input) noexcept
{
    history[static_cast<std::size_t>(written) & (historySize - 1u)]
        = numeric::clampFinite(input, -1.0f, 1.0f, 0.0f);
    ++written;
    if (captureIndex < tableSize)
    {
        const auto inactive = 1u - activeTable;
        table[inactive][captureIndex] = history[(captureStart + captureIndex)
            & (historySize - 1u)];
        ++captureIndex;
        if (captureIndex == tableSize)
        {
            activeTable = inactive;
            hasTable = true;
        }
    }

    auto value = 0.0f;
    if (hasTable)
    {
        const auto position = phase * static_cast<float>(tableSize);
        const auto index = static_cast<std::size_t>(position) & (tableSize - 1u);
        const auto next = (index + 1u) & (tableSize - 1u);
        const auto fraction = position - std::floor(position);
        const auto raw = table[activeTable][index]
            + (table[activeTable][next] - table[activeTable][index]) * fraction;
        const auto coloured = std::tanh(raw * (1.0f + controls.colour * 5.0f));
        value = raw * (1.0f - controls.colour) + coloured * controls.colour;
    }
    phase = advancePhase(phase, controls.frequencyHz
        / static_cast<float>(sampleRate));
    return { std::clamp(value * controls.level, -1.0f, 1.0f),
             hasTable, captureIndex < tableSize };
}

void FrequencyShifter::prepare(double newSampleRate) noexcept
{
    sampleRate = numeric::sampleRate(newSampleRate);
    constexpr auto centre = static_cast<int>(tapCount / 2u);
    for (std::size_t index = 0; index < tapCount; ++index)
    {
        const auto k = static_cast<int>(index) - centre;
        const auto window = 0.42f - 0.5f * std::cos(twoPi
            * static_cast<float>(index) / static_cast<float>(tapCount - 1u))
            + 0.08f * std::cos(2.0f * twoPi * static_cast<float>(index)
                / static_cast<float>(tapCount - 1u));
        hilbert[index] = k != 0 && std::abs(k) % 2 == 1
            ? 2.0f / (pi * static_cast<float>(k)) * window : 0.0f;
    }
    setParameters(controls);
    reset();
}

void FrequencyShifter::reset() noexcept
{
    history.fill(0.0f);
    writeIndex = 0;
    carrierPhase = 0.0f;
}

void FrequencyShifter::setParameters(Parameters value) noexcept
{
    controls.shiftHz = numeric::clampFinite(value.shiftHz,
        static_cast<float>(-sampleRate * 0.20),
        static_cast<float>(sampleRate * 0.20), controls.shiftHz);
    controls.mix = numeric::unit(value.mix, controls.mix);
}

float FrequencyShifter::tick(float input) noexcept
{
    const auto source = safe(input);
    history[writeIndex] = source;
    constexpr auto delay = tapCount / 2u;
    const auto delayed = history[(writeIndex + tapCount - delay) % tapCount];
    auto quadrature = 0.0f;
    for (std::size_t tap = 0; tap < tapCount; ++tap)
        quadrature += hilbert[tap]
            * history[(writeIndex + tapCount - tap) % tapCount];
    writeIndex = (writeIndex + 1u) % tapCount;
    const auto cosine = std::cos(twoPi * carrierPhase);
    const auto sine = std::sin(twoPi * carrierPhase);
    carrierPhase = advancePhase(carrierPhase,
        controls.shiftHz / static_cast<float>(sampleRate));
    const auto shifted = delayed * cosine - quadrature * sine;
    return std::clamp(source * (1.0f - controls.mix)
        + shifted * controls.mix, -1.0f, 1.0f);
}

void CrossFeedbackStateMachine::prepare(double newSampleRate) noexcept
{
    const auto rate = numeric::sampleRate(newSampleRate);
    attack = static_cast<float>(-std::expm1(-1.0 / (rate * 0.012)));
    release = static_cast<float>(-std::expm1(-1.0 / (rate * 0.180)));
    reset();
}

void CrossFeedbackStateMachine::reset() noexcept
{
    envelope = 0.0f;
    current = State::calm;
}

void CrossFeedbackStateMachine::setParameters(Parameters value) noexcept
{
    controls.lowThreshold = numeric::clampFinite(value.lowThreshold,
        0.01f, 0.65f, controls.lowThreshold);
    controls.highThreshold = numeric::clampFinite(value.highThreshold,
        controls.lowThreshold + 0.05f, 0.95f, controls.highThreshold);
    controls.hysteresis = numeric::clampFinite(value.hysteresis,
        0.0f, 0.20f, controls.hysteresis);
    controls.influence = numeric::unit(value.influence, controls.influence);
}

CrossFeedbackStateMachine::Output CrossFeedbackStateMachine::tick(
    float audio, float context) noexcept
{
    const auto magnitude = std::abs(safe(audio));
    envelope += (magnitude - envelope)
        * (magnitude > envelope ? attack : release);
    const auto energy = std::clamp(envelope
        * (0.75f + numeric::unit(context, 0.5f) * 0.50f), 0.0f, 1.0f);
    const auto previous = current;
    switch (current)
    {
        case State::calm:
            if (energy > controls.lowThreshold + controls.hysteresis)
                current = State::listen;
            break;
        case State::listen:
            if (energy < controls.lowThreshold - controls.hysteresis)
                current = State::calm;
            else if (energy > controls.highThreshold + controls.hysteresis)
                current = State::respond;
            break;
        case State::respond:
            if (energy < controls.highThreshold - controls.hysteresis)
                current = State::listen;
            else if (energy > std::min(0.98f,
                controls.highThreshold + 0.24f + controls.hysteresis))
                current = State::overload;
            break;
        case State::overload:
            if (energy < controls.highThreshold + 0.12f)
                current = State::respond;
            break;
    }
    constexpr std::array<float, 4> stateValue { -0.65f, -0.18f, 0.48f, 1.0f };
    return { stateValue[static_cast<std::size_t>(current)] * controls.influence,
             envelope, current, previous != current };
}

SpotlightMixer::Output SpotlightMixer::process(const Sources& sources,
                                                float focusPhase,
                                                float duckBackground) noexcept
{
    const auto position = numeric::wrapUnit(focusPhase)
        * static_cast<float>(sourceCount);
    const auto primary = static_cast<std::size_t>(position) % sourceCount;
    const auto next = (primary + 1u) % sourceCount;
    const auto fraction = position - std::floor(position);
    const auto foreground = safe(sources[primary]) * (1.0f - fraction)
        + safe(sources[next]) * fraction;
    const auto duck = 1.0f - numeric::unit(duckBackground, 0.0f) * 0.78f;
    auto backgroundLeft = 0.0f;
    auto backgroundRight = 0.0f;
    for (std::size_t index = 0; index < sourceCount; ++index)
    {
        if (index == primary || index == next) continue;
        const auto pan = static_cast<float>(index)
            / static_cast<float>(sourceCount - 1u) * pi * 0.5f;
        const auto value = safe(sources[index]) * 0.30f * duck;
        backgroundLeft += value * std::cos(pan);
        backgroundRight += value * std::sin(pan);
    }
    return { std::clamp(foreground * 0.58f + backgroundLeft, -1.0f, 1.0f),
             std::clamp(foreground * 0.58f + backgroundRight, -1.0f, 1.0f),
             primary };
}

void HungerProcessor::prepare(double newSampleRate) noexcept
{
    const auto rate = numeric::sampleRate(newSampleRate);
    smoothing = static_cast<float>(-std::expm1(-1.0 / (rate * 0.035)));
    reset();
}

void HungerProcessor::setSeed(std::uint32_t value) noexcept
{
    seed = value == 0u ? 1u : value;
    reset();
}

void HungerProcessor::reset() noexcept
{
    supply = gate = gateTarget = 1.0f;
    countdown = 0;
    randomState = seed;
}

void HungerProcessor::setParameters(Parameters value) noexcept
{
    controls.density = numeric::unit(value.density, controls.density);
    controls.scarcity = numeric::unit(value.scarcity, controls.scarcity);
    controls.instability = numeric::unit(value.instability, controls.instability);
}

float HungerProcessor::tick(float input) noexcept
{
    const auto targetSupply = std::clamp(1.0f
        - controls.density * controls.scarcity * 0.82f, 0.18f, 1.0f);
    supply += (targetSupply - supply) * smoothing;
    if (countdown == 0u)
    {
        randomState = randomState * 1664525u + 1013904223u;
        const auto random = static_cast<float>((randomState >> 8u) & 0xffffu)
            / 65535.0f;
        const auto failure = (1.0f - supply) * controls.instability;
        gateTarget = random < failure
            ? 0.22f + random * 0.45f : 1.0f;
        countdown = 16u + ((randomState >> 17u) & 0xffu);
    }
    --countdown;
    gate += (gateTarget - gate) * std::min(0.08f, smoothing * 24.0f);
    const auto starved = safe(input) * supply * gate;
    return std::clamp(std::tanh(starved * (1.0f
        + (1.0f - supply) * 1.8f)), -1.0f, 1.0f);
}

void BoundedSelfFeedback::prepare(double newSampleRate) noexcept
{
    sampleRate = numeric::sampleRate(newSampleRate);
    setParameters(controls);
    reset();
}

void BoundedSelfFeedback::reset() noexcept
{
    delayed = filtered = 0.0f;
}

void BoundedSelfFeedback::setParameters(Parameters value) noexcept
{
    controls.feedback = numeric::unit(value.feedback, controls.feedback);
    controls.tone = numeric::unit(value.tone, controls.tone);
    controls.drive = numeric::unit(value.drive, controls.drive);
    controls.mix = numeric::unit(value.mix, controls.mix);
    const auto cutoff = 70.0f * std::exp2(controls.tone * 7.5f);
    filterCoefficient = 1.0f - std::exp(-twoPi * cutoff
        / static_cast<float>(sampleRate));
}

float BoundedSelfFeedback::tick(float input) noexcept
{
    const auto source = safe(input);
    const auto loop = std::clamp(source + delayed * controls.feedback * 0.92f,
        -3.0f, 3.0f);
    const auto shaped = std::tanh(loop * (1.0f + controls.drive * 5.0f));
    filtered += (shaped - filtered) * filterCoefficient;
    delayed = std::clamp(filtered, -1.0f, 1.0f);
    return std::clamp(source * (1.0f - controls.mix)
        + delayed * controls.mix, -1.0f, 1.0f);
}

void WaterInertia::prepare(double newSampleRate) noexcept
{
    sampleRate = numeric::sampleRate(newSampleRate);
    setParameters(controls);
    reset();
}

void WaterInertia::reset(float value) noexcept
{
    current = numeric::clampFinite(value, -1.0f, 1.0f, 0.0f);
}

void WaterInertia::setParameters(Parameters value) noexcept
{
    controls.inertia = numeric::unit(value.inertia, controls.inertia);
    controls.viscosity = numeric::unit(value.viscosity, controls.viscosity);
    controls.asymmetry = numeric::clampFinite(value.asymmetry,
        -1.0f, 1.0f, controls.asymmetry);
    const auto baseSeconds = 0.004f + controls.inertia * controls.inertia
        * (0.12f + controls.viscosity * 1.3f);
    const auto riseSeconds = baseSeconds
        * std::exp2(controls.asymmetry * 1.5f);
    const auto fallSeconds = baseSeconds
        * std::exp2(-controls.asymmetry * 1.5f);
    riseCoefficient = static_cast<float>(-std::expm1(
        -1.0 / (sampleRate * riseSeconds)));
    fallCoefficient = static_cast<float>(-std::expm1(
        -1.0 / (sampleRate * fallSeconds)));
}

float WaterInertia::tick(float target) noexcept
{
    const auto bounded = numeric::clampFinite(target, -1.0f, 1.0f, current);
    current += (bounded - current)
        * (bounded > current ? riseCoefficient : fallCoefficient);
    if (std::abs(current) < 1.0e-20f) current = 0.0f;
    return current;
}
}
