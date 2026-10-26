// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/BubbleField.h"

#include <algorithm>
#include <cmath>

namespace aquorbium
{
namespace
{
constexpr float twoPi = 6.28318530717958647692f;
constexpr float pi = 3.14159265358979323846f;
constexpr float delta = 0.1f;          // total damping of a ~mm bubble (Leighton)
constexpr float hinze = 1.0e-3f;       // the break in the size distribution (m)
constexpr float smallest = 0.25e-3f;   // 13 kHz
constexpr float largest = 6.0e-3f;     // 540 Hz
}

float BubbleField::minnaertHz(float radiusMetres) noexcept
{
    return 3.26f / std::max(1.0e-5f, radiusMetres);
}

float BubbleField::coupledLowHz(float f0, float radius, float distance) noexcept
{
    return f0 / std::sqrt(1.0f + radius / std::max(distance, radius * 1.01f));
}

float BubbleField::coupledHighHz(float f0, float radius, float distance) noexcept
{
    return f0 / std::sqrt(1.0f - radius / std::max(distance, radius * 1.01f));
}

float BubbleField::drawRadius(float u) noexcept
{
    // Deane & Stokes 2002: number density ~ R^(-3/2) below the Hinze scale,
    // ~ R^(-10/3) above. Sampled by inverting each branch's cumulative
    // distribution; the branches are weighted by their integrals so the
    // density is continuous at the break.
    const auto x = std::clamp(u, 1.0e-6f, 1.0f - 1.0e-6f);
    // integrals of R^-a between the limits (a = 3/2 below, 10/3 above, the
    // upper branch scaled so both meet at the Hinze scale)
    const auto lowA = 1.5f, highA = 10.0f / 3.0f;
    const auto lowMass = (std::pow(smallest, 1.0f - lowA) - std::pow(hinze, 1.0f - lowA)) / (lowA - 1.0f);
    const auto scale = std::pow(hinze, highA - lowA);  // continuity at the break
    const auto highMass = scale * (std::pow(hinze, 1.0f - highA) - std::pow(largest, 1.0f - highA)) / (highA - 1.0f);
    const auto total = lowMass + highMass;
    if (x * total < lowMass)
    {
        // R^(1-a) = smallest^(1-a) - t * (a-1) * mass_fraction
        const auto t = x * total;
        const auto value = std::pow(smallest, 1.0f - lowA) - t * (lowA - 1.0f);
        return std::pow(value, 1.0f / (1.0f - lowA));
    }
    const auto t = (x * total - lowMass) / scale;
    const auto value = std::pow(hinze, 1.0f - highA) - t * (highA - 1.0f);
    return std::clamp(std::pow(value, 1.0f / (1.0f - highA)), hinze, largest);
}

void BubbleField::prepare(double sampleRate, std::uint32_t seedValue) noexcept
{
    rate = std::isfinite(sampleRate) && sampleRate >= 8000.0 ? sampleRate : 48000.0;
    sampleRateF = static_cast<float>(rate);
    seed = seedValue == 0 ? 0xb0bb1e5u : seedValue;
    reset();
}

void BubbleField::reset() noexcept
{
    state = seed;
    bubbles = {};
    sinceBirth = 0xffffffu;
    lastRadius = lastFrequency = 0.0f;
    lastPan = 0.5f;
    bornCount = pairCount = 0;
}

void BubbleField::setRate(float bubblesPerSecond) noexcept
{
    const auto r = std::isfinite(bubblesPerSecond) ? std::clamp(bubblesPerSecond, 0.0f, 400.0f) : 0.0f;
    perSample = r / sampleRateF;
}

float BubbleField::nextUnit() noexcept
{
    state ^= state << 13u;
    state ^= state >> 17u;
    state ^= state << 5u;
    return static_cast<float>(state & 0xffffffu) / 16777216.0f;
}

void BubbleField::spawn(float radius, float frequency, float amplitude, float pan) noexcept
{
    std::size_t slot = 0;
    for (std::size_t i = 1; i < bubbles.size(); ++i)
        if (bubbles[i].amplitude < bubbles[slot].amplitude)
            slot = i;
    auto& b = bubbles[slot];
    const auto f = std::clamp(frequency, 200.0f, sampleRateF * 0.42f);
    b.phase = 0.0f;
    b.increment = f / sampleRateF;
    // the decay of a damped oscillator: amplitude ~ exp(-pi f delta t)
    b.decay = std::exp(-pi * f * delta / sampleRateF);
    // the pitch rises ~10-30 % over the life (about five time constants)
    const auto life = 5.0f / (pi * f * delta);
    b.rise = std::exp(std::log(1.1f + 0.2f * nextUnit()) / (life * sampleRateF));
    // a larger bubble rings louder (its pressure ~ R), within limits
    b.amplitude = amplitude * std::clamp(radius / hinze, 0.4f, 2.5f) * 0.25f;
    const auto angle = std::clamp(pan, 0.0f, 1.0f) * 0.5f * pi;
    b.left = std::cos(angle);
    b.right = std::sin(angle);
}

BubbleField::Sample BubbleField::render() noexcept
{
    if (perSample > 0.0f)
    {
        if (sinceBirth < 0xffffffu)
            ++sinceBirth;
        if (nextUnit() < perSample)
        {
            const auto radius = drawRadius(nextUnit());
            const auto f0 = minnaertHz(radius);
            const auto pan = 0.15f + 0.7f * nextUnit();
            const auto amplitude = 0.5f + 0.5f * nextUnit();
            // born within 3 ms of the last one, close by: a coupled pair
            if (sinceBirth < static_cast<std::uint32_t>(0.003f * sampleRateF) && lastRadius > 0.0f)
            {
                const auto r = 0.5f * (radius + lastRadius);
                const auto f = 0.5f * (f0 + lastFrequency);
                const auto d = r * (2.0f + 3.0f * nextUnit());
                // the in-phase mode radiates strongly, the antiphase one weakly
                spawn(r, coupledLowHz(f, r, d), amplitude, 0.5f * (pan + lastPan));
                spawn(r, coupledHighHz(f, r, d), amplitude * 0.3f, 0.5f * (pan + lastPan));
                ++pairCount;
                lastRadius = 0.0f;  // a pair is complete
            }
            else
            {
                spawn(radius, f0, amplitude, pan);
                lastRadius = radius;
                lastFrequency = f0;
                lastPan = pan;
            }
            ++bornCount;
            sinceBirth = 0;
        }
    }
    Sample out;
    for (auto& b : bubbles)
    {
        if (b.amplitude < 1.0e-6f)
        {
            b.amplitude = 0.0f;
            continue;
        }
        const auto s = b.amplitude * std::sin(twoPi * b.phase);
        out.left += s * b.left;
        out.right += s * b.right;
        b.phase += b.increment;
        b.phase -= std::floor(b.phase);
        b.increment = std::min(b.increment * b.rise, 0.45f);
        b.amplitude *= b.decay;
    }
    return out;
}
}
