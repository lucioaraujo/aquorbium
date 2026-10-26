// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/ChorusPhase.h"

#include <algorithm>
#include <cmath>

namespace aquorbium
{
namespace
{
constexpr float twoPi = 6.28318530717958647692f;

float wrapUnit(float value) noexcept
{
    return value - std::floor(value);
}
}

void ChorusPhase::reset(std::uint32_t seed) noexcept
{
    // spread the starting phases deterministically from the seed
    auto state = seed == 0u ? 0x9e3779b9u : seed;
    for (std::size_t i = 0; i < voiceCount; ++i)
    {
        state ^= state << 13u;
        state ^= state >> 17u;
        state ^= state << 5u;
        phases[i] = static_cast<float>(state & 0xffffu) / 65536.0f;
        rates[i] = 1.0f / 16.0f;
        pull[i] = 0.0f;
    }
}

void ChorusPhase::setNaturalRate(std::size_t voice, float cyclesPerStep) noexcept
{
    if (voice < voiceCount && std::isfinite(cyclesPerStep))
        rates[voice] = std::clamp(cyclesPerStep, 1.0f / 128.0f, 0.5f);
}

void ChorusPhase::hear(std::size_t listener, std::size_t source, float level, float targetOffset,
                       float gain) noexcept
{
    if (listener >= voiceCount || source >= voiceCount || listener == source)
        return;
    // phase error: where the listener is, relative to where the relation wants it
    const auto desired = phases[source] + targetOffset;
    const auto error = desired - phases[listener];
    pull[listener] += std::clamp(level, 0.0f, 1.0f) * std::clamp(gain, 0.0f, 1.0f)
        * std::sin(twoPi * error) / twoPi;
}

std::array<bool, ChorusPhase::voiceCount> ChorusPhase::step() noexcept
{
    std::array<bool, voiceCount> fired {};
    for (std::size_t i = 0; i < voiceCount; ++i)
    {
        // a bounded nudge: never more than half a natural step backwards or forwards
        const auto nudge = std::clamp(pull[i], -0.5f * rates[i], 0.5f * rates[i]);
        const auto next = phases[i] + rates[i] + nudge;
        fired[i] = next >= 1.0f;
        phases[i] = wrapUnit(next);
        pull[i] = 0.0f;
    }
    return fired;
}

float ChorusPhase::difference(std::size_t a, std::size_t b) const noexcept
{
    const auto d = phases[b] - phases[a];
    return d - std::floor(d + 0.5f);
}
}
