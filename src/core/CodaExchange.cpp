// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/CodaExchange.h"

#include <algorithm>
#include <cmath>

namespace aquorbium
{
std::uint8_t CodaExchange::steps(int rhythm, std::size_t unit, std::size_t length) noexcept
{
    const auto n = std::max<std::size_t>(1, length);
    const auto k = std::min(unit, n - 1);
    const auto position = n > 1 ? static_cast<float>(k) / static_cast<float>(n - 1) : 0.0f;
    int s = 2;
    switch (static_cast<Rhythm>(std::clamp(rhythm, 0, rhythmCount - 1)))
    {
        case regular: s = 2; break;
        case twoShortLong: s = (k % 3u) == 2u ? 4 : 1; break;
        case accelerando: s = static_cast<int>(std::lround(4.0f - 3.0f * position)); break;
        case ritardando: s = static_cast<int>(std::lround(1.0f + 3.0f * position)); break;
        case longShort: s = (k % 2u) == 0u ? 3 : 1; break;
        case shortLong: s = (k % 2u) == 0u ? 1 : 3; break;
    }
    return static_cast<std::uint8_t>(std::max(1, s));
}

float CodaExchange::tempo(int type) noexcept
{
    switch (std::clamp(type, 0, tempoCount - 1))
    {
        case 0: return 0.75f;
        case 2: return 1.35f;
        default: return 1.0f;
    }
}

float CodaExchange::matchRubato(float own, float partner) noexcept
{
    return own + (partner - own) * matchRate;
}

float CodaExchange::ornamentChance(bool contextChanged) noexcept
{
    return contextChanged ? 0.55f : 0.05f;
}

int CodaExchange::identityRhythm(int species) noexcept
{
    // Oscillator, Pulsar, GranularStriker, Resonator, PhysicalString
    static constexpr int identity[] { longShort, regular, twoShortLong, ritardando, shortLong };
    return identity[std::clamp(species, 0, 4)];
}
}
