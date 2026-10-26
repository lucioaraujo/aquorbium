// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/BiomaBrain.h"
#include "core/BiomaEngine.h"
#include "core/Ecosystem.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <vector>

namespace
{
using Mix = aquorbium::Ecosystem::MixState;
std::array<float, 4> fields(Mix value)
{
    return { value.left, value.right, value.observation, value.detail };
}

bool smoothingAndMotion(double rate)
{
    aquorbium::BiomaBrain brain;
    brain.prepare(rate);
    brain.setSeed(91);
    aquorbium::Ecosystem ecology;
    ecology.prepare(rate);
    ecology.reset(brain);
    const auto coefficient = static_cast<float>(-std::expm1(-1.0 / (rate * 0.015)));
    std::array<Mix, aquorbium::Ecosystem::organismCount> previous {};
    std::array<Mix, aquorbium::Ecosystem::organismCount> previousTarget {};
    for (std::size_t i = 0; i < previous.size(); ++i)
    {
        previous[i] = ecology.audioMix(i);
        previousTarget[i] = ecology.mix(i);
        if (fields(previous[i]) != fields(ecology.mix(i))) return false;
    }
    if (ecology.lensFocusedOrganisms() != 0 || !ecology.holdLens(0)
        || ecology.lensFocusedOrganisms() != 1) return false;
    ecology.setLensTarget({ -1.0f, 0.0f, 0.0f });
    bool targetJumpObserved = false;
    const auto samples = static_cast<std::size_t>(rate * 7.0);
    const auto releaseSample = static_cast<std::size_t>(rate * 3.0);
    float largestStep = 0.0f;
    float largestTargetStep = 0.0f;
    for (std::size_t sample = 0; sample < samples; ++sample)
    {
        if (sample == releaseSample)
        {
            ecology.releaseLens(0);
            if (ecology.lensFocusedOrganisms() != 0) return false;
        }
        if (sample == releaseSample + static_cast<std::size_t>(std::round(rate / 200.0)))
            ecology.holdLens(0);
        ecology.tickSample(brain);
        for (std::size_t i = 0; i < previous.size(); ++i)
        {
            const auto before = fields(previous[i]);
            const auto target = fields(ecology.mix(i));
            const auto oldTarget = fields(previousTarget[i]);
            const auto current = fields(ecology.audioMix(i));
            for (std::size_t field = 0; field < before.size(); ++field)
            {
                const auto expected = before[field] + (target[field] - before[field]) * coefficient;
                if (!std::isfinite(current[field]) || std::abs(current[field] - expected) > 2.0e-7f
                    || current[field] < 0.0f || current[field] > 1.0f) return false;
                const auto step = std::abs(current[field] - before[field]);
                largestStep = std::max(largestStep, step);
                largestTargetStep = std::max(largestTargetStep, std::abs(target[field] - oldTarget[field]));
                if (step > coefficient + 2.0e-7f) return false;
                targetJumpObserved |= std::abs(target[field] - before[field]) > 0.01f;
            }
            previous[i] = ecology.audioMix(i);
            previousTarget[i] = ecology.mix(i);
        }
        const auto point = ecology.lensState().surfacePosition;
        const auto radius = std::sqrt(point.x * point.x + point.y * point.y + point.z * point.z);
        if (!std::isfinite(radius) || std::abs(radius - 1.0f) > 2.0e-6f) return false;
    }
    const auto point = ecology.lensState().surfacePosition;
    if (point.x != -1.0f || point.y != 0.0f || point.z != 0.0f || !targetJumpObserved) return false;
    // Arrival must stay exactly at the target rather than oscillating around it.
    for (int sample = 0; sample < 1000; ++sample) ecology.tickSample(brain);
    const auto arrived = ecology.lensState().surfacePosition;
    if (arrived.x != point.x || arrived.y != point.y || arrived.z != point.z) return false;
    std::cout << "rate=" << rate << " largest_audio_mix_step=" << largestStep
              << " largest_control_mix_step=" << largestTargetStep << '\n';
    return true;
}

struct Audio
{
    std::vector<float> left;
    std::vector<float> right;
};

Audio render(double rate, std::size_t block, bool opposite, bool solo)
{
    aquorbium::BiomaEngine engine;
    engine.prepare(rate);
    engine.setSeed(91);
    engine.setEcosystemOutputFeedbackEnabled(false);
    engine.setAutonomousEnabled(true);
    engine.setEcosystemBlendImmediately(1.0f);
    engine.setListeningBlend(solo ? std::array<float, 4> { 1, 0, 0, 0 }
                                 : std::array<float, 4> { 0.25f, 0.25f, 0.25f, 0.25f });
    engine.noteOn(48, 0.7f);
    engine.setLensGesture(true, { opposite ? -1.0f : 1.0f, 0, 0 });
    Audio result;
    const auto length = static_cast<std::size_t>(rate * 8.0);
    result.left.resize(length);
    result.right.resize(length);
    const std::array<std::size_t, 4> boundaries {
        static_cast<std::size_t>(rate * 3.0), static_cast<std::size_t>(rate * 6.0),
        static_cast<std::size_t>(rate * 7.0), length
    };
    std::size_t next = 0;
    for (std::size_t offset = 0; offset < length;)
    {
        if (offset == boundaries[next])
        {
            if (next == 0) engine.setLensGesture(false, { opposite ? -1.0f : 1.0f, 0, 0 });
            if (next == 1) engine.setLensGesture(true, { opposite ? -1.0f : 1.0f, 0, 0 });
            if (next == 2) engine.noteOff(48);
            ++next;
        }
        const auto count = std::min(block, boundaries[next] - offset);
        engine.render(result.left.data() + offset, result.right.data() + offset, count);
        offset += count;
    }
    return result;
}

bool soundPathAndBlockIndependence(double rate, bool solo)
{
    const auto front = render(rate, 257, false, solo);
    const auto opposite = render(rate, 257, true, solo);
    const auto differentBlocks = render(rate, 64, true, solo);
    if (opposite.left != differentBlocks.left || opposite.right != differentBlocks.right)
    {
        std::cerr << "Audio differs with callback size, rate=" << rate << " solo=" << solo << '\n';
        return false;
    }
    double energy = 0;
    double difference = 0;
    double peak = 0;
    for (std::size_t i = 0; i < front.left.size(); ++i)
    {
        const std::array<float, 4> values { front.left[i], front.right[i], opposite.left[i], opposite.right[i] };
        for (const auto value : values)
        {
            if (!std::isfinite(value)) return false;
            peak = std::max(peak, static_cast<double>(std::abs(value)));
        }
        const auto dl = static_cast<double>(front.left[i]) - opposite.left[i];
        const auto dr = static_cast<double>(front.right[i]) - opposite.right[i];
        energy += static_cast<double>(front.left[i]) * front.left[i]
            + static_cast<double>(front.right[i]) * front.right[i];
        difference += dl * dl + dr * dr;
    }
    // This establishes an effective DSP path, NOT perceptual audibility.
    std::cout << "rate=" << rate << " solo=" << solo
              << " relative_difference_rms=" << std::sqrt(difference / energy)
              << " peak=" << peak << '\n';
    // The fixed targets only catch the initial near-glass passage briefly.
    // Require a measurable path above float noise, not an arbitrary hearing
    // threshold. The tracking A/B renderer provides the longer listening case.
    if (!(energy > 1.0e-6) || !(difference / energy > 1.0e-10) || peak >= 1.0) return false;
    return true;
}
}

int main()
{
    for (const auto rate : { 44100.0, 48000.0, 96000.0 })
        if (!smoothingAndMotion(rate) || !soundPathAndBlockIndependence(rate, false)
            || !soundPathAndBlockIndependence(rate, true))
        {
            std::cerr << "Lens audio regression failed at " << rate << " Hz\n";
            return EXIT_FAILURE;
        }
    return EXIT_SUCCESS;
}
