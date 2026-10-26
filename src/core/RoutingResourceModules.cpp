// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/RoutingResourceModules.h"

#include "core/Numeric.h"

#include <algorithm>
#include <cmath>

namespace aquorbium
{
bool BoundedRoutingMatrix::setCell(std::size_t output, std::size_t input,
                                   Cell value) noexcept
{
    if (output >= channelCount || input >= channelCount)
        return false;
    auto& destination = cells[output][input];
    destination.gain = numeric::clampFinite(value.gain, -1.0f, 1.0f,
                                            destination.gain);
    destination.contextDepth = numeric::clampFinite(value.contextDepth,
        -1.0f, 1.0f, destination.contextDepth);
    return true;
}

BoundedRoutingMatrix::Cell BoundedRoutingMatrix::cell(
    std::size_t output, std::size_t input) const noexcept
{
    return output < channelCount && input < channelCount
        ? cells[output][input] : Cell {};
}

BoundedRoutingMatrix::Values BoundedRoutingMatrix::process(
    const Values& input, float context) const noexcept
{
    Values output {};
    const auto bipolarContext = numeric::unit(context, 0.5f) * 2.0f - 1.0f;
    for (std::size_t destination = 0; destination < channelCount; ++destination)
    {
        auto sum = 0.0f;
        auto gainSum = 0.0f;
        for (std::size_t source = 0; source < channelCount; ++source)
        {
            const auto gain = std::clamp(cells[destination][source].gain
                + cells[destination][source].contextDepth * bipolarContext,
                -1.0f, 1.0f);
            const auto safe = numeric::clampFinite(input[source], -4.0f, 4.0f, 0.0f);
            sum += safe * gain;
            gainSum += std::abs(gain);
        }
        output[destination] = std::clamp(sum / std::max(1.0f, gainSum),
                                         -1.0f, 1.0f);
    }
    return output;
}

bool SharedParameterPool::setValue(std::size_t index, float newValue) noexcept
{
    if (index >= valueCount)
        return false;
    values[index] = numeric::clampFinite(newValue, -1.0f, 1.0f, values[index]);
    return true;
}

float SharedParameterPool::value(std::size_t index) const noexcept
{
    return index < valueCount ? values[index] : 0.0f;
}

bool SharedParameterPool::setRoute(std::size_t consumer, Route route) noexcept
{
    if (consumer >= consumerCount || route.first >= valueCount
        || route.second >= valueCount)
        return false;
    route.blend = numeric::unit(route.blend, routes[consumer].blend);
    routes[consumer] = route;
    return true;
}

void SharedParameterPool::reset() noexcept
{
    held.fill(0.0f);
    hasHeld.fill(false);
}

float SharedParameterPool::read(std::size_t consumer, float context,
                                bool latchEvent) noexcept
{
    if (consumer >= consumerCount)
        return 0.0f;
    auto route = routes[consumer];
    if (mode == Mode::rotate)
    {
        const auto offset = static_cast<std::size_t>(
            numeric::wrapUnit(context) * static_cast<float>(valueCount)) % valueCount;
        route.first = (route.first + offset) % valueCount;
        route.second = (route.second + offset) % valueCount;
    }
    const auto live = values[route.first] * (1.0f - route.blend)
        + values[route.second] * route.blend;
    if (mode != Mode::hold)
        return live;
    if (latchEvent || !hasHeld[consumer])
    {
        held[consumer] = live;
        hasHeld[consumer] = true;
    }
    return held[consumer];
}
}
