// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/LifeCycle.h"

#include <algorithm>
#include <cmath>

namespace aquorbium
{
LifeCycle::Event LifeCycle::live(Life& life, float seconds, float food, int notes) noexcept
{
    if (!(seconds > 0.0f))
        return Event::none;
    life.energy += std::max(0.0f, food) - basalCost * seconds - noteCost * static_cast<float>(std::max(0, notes));
    life.energy = std::clamp(life.energy, 0.0f, 1.0f);
    life.age += seconds / std::max(1.0f, life.lifespan);
    if (life.energy <= 0.0f)
        return Event::starved;
    if (life.age >= 1.0f)
        return Event::oldAge;
    return Event::none;
}

float LifeCycle::graze(const Life& life, float seconds, float preyAbundance) noexcept
{
    const auto prey = std::clamp(preyAbundance, 0.0f, 3.0f);
    return grazingRate * prey * prey * seconds * (1.0f - 0.6f * life.energy);
}

float LifeCycle::hunger(const Life& life) noexcept
{
    return std::clamp(1.0f - life.energy * 3.0f, 0.0f, 1.0f);
}

float LifeCycle::darkness(const Life& life) noexcept
{
    const auto x = std::clamp((life.age - 0.5f) * 2.0f, 0.0f, 1.0f);
    return x * x * (3.0f - 2.0f * x);
}

void LifeCycle::beBorn(Life& life, float lifespan, std::uint16_t generation) noexcept
{
    life.energy = 0.7f;
    life.age = 0.0f;
    life.lifespan = std::max(30.0f, lifespan);
    life.generation = generation;
}

LifeCycle::Phrase LifeCycle::recombine(const Phrase& a, const Phrase& b, float cut, float mutateWhich,
                                       float mutateBy) noexcept
{
    const auto la = static_cast<int>(std::clamp<std::uint8_t>(a.length, 1, SongForm::capacity));
    const auto lb = static_cast<int>(std::clamp<std::uint8_t>(b.length, 1, SongForm::capacity));
    Phrase child;
    // the length lies between the parents'; the head comes from a (at least
    // one unit, leaving at least one for b's tail)
    const auto length = std::clamp((la + lb + 1) / 2, 3, static_cast<int>(SongForm::capacity));
    const auto head = std::clamp(1 + static_cast<int>(cut * static_cast<float>(length - 1)), 1, length - 1);
    for (int u = 0; u < length; ++u)
    {
        const auto i = static_cast<std::size_t>(u);
        if (u < head)
        {
            const auto k = static_cast<std::size_t>(std::min(u, la - 1));
            child.degrees[i] = a.degrees[k];
            child.steps[i] = a.steps[k];
        }
        else
        {
            // b's tail, aligned to b's end
            const auto k = static_cast<std::size_t>(std::clamp(lb - (length - u), 0, lb - 1));
            // b's contour, moved to continue from where a's head stopped
            const auto join = static_cast<std::size_t>(std::clamp(lb - (length - head), 0, lb - 1));
            const auto shift = child.degrees[static_cast<std::size_t>(head - 1)] - b.degrees[join];
            child.degrees[i] = static_cast<std::int8_t>(std::clamp(b.degrees[k] + shift, -5, 7));
            child.steps[i] = b.steps[k];
        }
        child.steps[i] = std::max<std::uint8_t>(child.steps[i], 1);
    }
    child.length = static_cast<std::uint8_t>(length);
    // one mutation: a step up or down (never the first unit, the anchor)
    const auto m = static_cast<std::size_t>(1 + static_cast<int>(mutateWhich * static_cast<float>(length - 1)));
    const auto by = mutateBy < 0.5f ? -1 : 1;
    child.degrees[std::min(m, static_cast<std::size_t>(length - 1))] = static_cast<std::int8_t>(
        std::clamp(child.degrees[std::min(m, static_cast<std::size_t>(length - 1))] + by, -5, 7));
    return child;
}
}
