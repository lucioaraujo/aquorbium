// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/BiomaMarkov.h"
#include "core/Numeric.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace aquorbium
{
BiomaMarkov::BiomaMarkov() noexcept
{
    restoreDefaultTransitions();
    reset();
}

void BiomaMarkov::setSeed(std::uint32_t value) noexcept
{
    seed = value == 0u ? 1u : value;
    reset(state);
}

void BiomaMarkov::reset(std::size_t initialState) noexcept
{
    state = std::min(initialState, stateCount - 1u);
    randomState = seed;
    transitions = 0;
}

void BiomaMarkov::setParameters(Parameters value) noexcept
{
    controls.contextInfluence = numeric::unit(value.contextInfluence,
                                               controls.contextInfluence);
    controls.inertia = numeric::unit(value.inertia, controls.inertia);
    controls.mutation = numeric::unit(value.mutation, controls.mutation);
}

BiomaMarkov::TransitionRow BiomaMarkov::sanitized(
    TransitionRow value, std::size_t fallbackState) noexcept
{
    double sum = 0.0;
    for (auto& weight : value)
    {
        weight = std::isfinite(weight) ? std::max(0.0f, weight) : 0.0f;
        sum += weight;
    }
    if (sum <= 0.0)
    {
        value.fill(0.0f);
        value[fallbackState] = 1.0f;
        return value;
    }
    for (auto& weight : value)
        weight = static_cast<float>(weight / sum);
    return value;
}

void BiomaMarkov::setTransitionRow(std::size_t row,
                                   TransitionRow weights) noexcept
{
    if (row >= stateCount) return;
    matrix[row] = sanitized(weights, row);
}

void BiomaMarkov::restoreDefaultTransitions() noexcept
{
    for (std::size_t row = 0; row < stateCount; ++row)
    {
        TransitionRow weights {};
        weights[row] = 0.48f;
        weights[(row + 1u) % stateCount] = 0.24f;
        weights[(row + stateCount - 1u) % stateCount] = 0.16f;
        weights[(row + 2u) % stateCount] = 0.08f;
        weights[(row + stateCount - 2u) % stateCount] = 0.04f;
        matrix[row] = sanitized(weights, row);
    }
}

float BiomaMarkov::nextRandomUnit() noexcept
{
    randomState = randomState * 1664525u + 1013904223u;
    return static_cast<float>((randomState >> 8) & 0x00ffffffu) / 16777216.0f;
}

std::size_t BiomaMarkov::advance(Context context) noexcept
{
    const auto target = numeric::unit(context.target, 0.5f);
    const auto intensity = numeric::unit(context.intensity, 0.0f)
        * controls.contextInfluence;
    TransitionRow weights {};
    double sum = 0.0;
    for (std::size_t candidate = 0; candidate < stateCount; ++candidate)
    {
        const auto position = static_cast<float>(candidate)
            / static_cast<float>(stateCount - 1u);
        const auto attraction = std::max(0.0f, 1.0f - std::abs(position - target));
        auto weight = matrix[state][candidate] * (1.0f - intensity)
            + attraction * intensity;
        weight = weight * (1.0f - controls.mutation) + controls.mutation;
        if (candidate == state) weight += controls.inertia;
        weights[candidate] = weight;
        sum += weight;
    }

    auto draw = static_cast<double>(nextRandomUnit()) * sum;
    auto selected = stateCount - 1u;
    for (std::size_t candidate = 0; candidate < stateCount; ++candidate)
    {
        draw -= weights[candidate];
        if (draw <= 0.0)
        {
            selected = candidate;
            break;
        }
    }
    state = selected;
    if (transitions != std::numeric_limits<std::uint64_t>::max()) ++transitions;
    return state;
}

const BiomaMarkov::TransitionRow& BiomaMarkov::transitionRow(
    std::size_t index) const noexcept
{
    return matrix[std::min(index, stateCount - 1u)];
}
}
