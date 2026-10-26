// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/PhilosophyMixEngine.h"

#include "core/Numeric.h"

#include <algorithm>
#include <cmath>

namespace aquorbium
{
PhilosophyVectorMixer::Values PhilosophyVectorMixer::weights(
    float x, float y) noexcept
{
    const auto safeX = numeric::unit(x, 0.5f);
    const auto safeY = numeric::unit(y, 0.5f);
    return {
        (1.0f - safeX) * (1.0f - safeY),
        safeX * (1.0f - safeY),
        (1.0f - safeX) * safeY,
        safeX * safeY
    };
}

float PhilosophyVectorMixer::mix(const Values& sources, float x, float y) noexcept
{
    const auto blend = weights(x, y);
    auto output = 0.0f;
    for (std::size_t index = 0; index < sourceCount; ++index)
        output += numeric::clampFinite(sources[index], -4.0f, 4.0f, 0.0f)
            * blend[index];
    return output;
}

PhilosophySequencer::PhilosophySequencer() noexcept
{
    steps[0] = { 0.0f, 0.0f, 1 };
    steps[1] = { 1.0f, 0.0f, 1 };
    steps[2] = { 1.0f, 1.0f, 1 };
    steps[3] = { 0.0f, 1.0f, 1 };
}

void PhilosophySequencer::setLength(std::size_t value) noexcept
{
    stepCount = std::clamp<std::size_t>(value, 1, maximumSteps);
    currentStep %= stepCount;
    pulsesAtStep = 0;
}

bool PhilosophySequencer::setStep(std::size_t index, Step value) noexcept
{
    if (index >= maximumSteps)
        return false;
    value.x = numeric::unit(value.x, steps[index].x);
    value.y = numeric::unit(value.y, steps[index].y);
    value.durationPulses = std::max<std::uint16_t>(1, value.durationPulses);
    steps[index] = value;
    return true;
}

void PhilosophySequencer::reset(std::size_t index) noexcept
{
    currentStep = std::min(index, stepCount - 1);
    pulsesAtStep = 0;
    previousClock = false;
}

PhilosophySequencer::Output PhilosophySequencer::tick(bool clockHigh) noexcept
{
    auto advanced = false;
    if (clockHigh && !previousClock)
    {
        ++pulsesAtStep;
        if (pulsesAtStep >= steps[currentStep].durationPulses)
        {
            currentStep = (currentStep + 1u) % stepCount;
            pulsesAtStep = 0;
            advanced = true;
        }
    }
    previousClock = clockHigh;
    return { PhilosophyVectorMixer::weights(steps[currentStep].x,
                                             steps[currentStep].y),
             currentStep, advanced };
}

void PhilosophyMorph::prepare(double sampleRate) noexcept
{
    preparedSampleRate = numeric::sampleRate(sampleRate);
    updateDuration();
}

void PhilosophyMorph::setTransitionSeconds(float value) noexcept
{
    transitionTime = numeric::clampFinite(value, 0.001f, 30.0f, transitionTime);
    updateDuration();
}

void PhilosophyMorph::reset(std::size_t philosophy) noexcept
{
    targetIndex = std::min<std::size_t>(philosophy, 3);
    current.fill(0.0f);
    current[targetIndex] = 1.0f;
    start = current;
    destination = current;
    elapsedSamples = durationSamples;
}

void PhilosophyMorph::setTarget(std::size_t philosophy) noexcept
{
    const auto next = std::min<std::size_t>(philosophy, 3);
    if (next == targetIndex && elapsedSamples >= durationSamples)
        return;
    targetIndex = next;
    start = current;
    destination.fill(0.0f);
    destination[targetIndex] = 1.0f;
    elapsedSamples = 0;
}

PhilosophyMorph::Output PhilosophyMorph::tick(
    const PhilosophyVectorMixer::Values& sources) noexcept
{
    if (elapsedSamples < durationSamples)
        ++elapsedSamples;
    const auto amount = static_cast<float>(elapsedSamples)
        / static_cast<float>(durationSamples);
    auto mixed = 0.0f;
    for (std::size_t index = 0; index < current.size(); ++index)
    {
        current[index] = start[index]
            + (destination[index] - start[index]) * amount;
        mixed += current[index]
            * numeric::clampFinite(sources[index], -4.0f, 4.0f, 0.0f);
    }
    return { current, mixed, elapsedSamples < durationSamples };
}

void PhilosophyMorph::updateDuration() noexcept
{
    durationSamples = std::max<std::size_t>(1, static_cast<std::size_t>(
        std::lround(static_cast<double>(transitionTime) * preparedSampleRate)));
    elapsedSamples = std::min(elapsedSamples, durationSamples);
}

PhilosophyMatrixMixer::PhilosophyMatrixMixer() noexcept
{
    for (std::size_t index = 0; index < outputCount; ++index)
        matrix[index][index].gain = 1.0f;
}

bool PhilosophyMatrixMixer::setCell(std::size_t output, std::size_t source,
                                    Cell value) noexcept
{
    if (output >= outputCount || source >= sourceCount)
        return false;
    auto& destination = matrix[output][source];
    destination.gain = numeric::clampFinite(
        value.gain, -1.0f, 1.0f, destination.gain);
    destination.modulationDepth = numeric::clampFinite(
        value.modulationDepth, -1.0f, 1.0f, destination.modulationDepth);
    return true;
}

PhilosophyMatrixMixer::Cell PhilosophyMatrixMixer::cell(
    std::size_t output, std::size_t source) const noexcept
{
    return output < outputCount && source < sourceCount
        ? matrix[output][source] : Cell {};
}

PhilosophyMatrixMixer::Output PhilosophyMatrixMixer::process(
    const std::array<float, sourceCount>& sources,
    const std::array<float, sourceCount>& modulation) const noexcept
{
    Output output;
    for (std::size_t row = 0; row < outputCount; ++row)
    {
        auto sum = 0.0f;
        auto gainBudget = 0.0f;
        for (std::size_t column = 0; column < sourceCount; ++column)
        {
            const auto cv = numeric::clampFinite(
                modulation[column], -1.0f, 1.0f, 0.0f);
            const auto gain = std::clamp(
                matrix[row][column].gain
                    + matrix[row][column].modulationDepth * cv,
                -1.0f, 1.0f);
            sum += numeric::clampFinite(
                sources[column], -4.0f, 4.0f, 0.0f) * gain;
            gainBudget += std::abs(gain);
        }
        output.normalization[row] = 1.0f / std::max(1.0f, gainBudget);
        output.channel[row] = sum * output.normalization[row];
    }
    return output;
}
}
