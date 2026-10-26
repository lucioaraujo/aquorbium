// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace aquorbium
{
class BoundedRoutingMatrix final
{
public:
    static constexpr std::size_t channelCount = 5;
    using Values = std::array<float, channelCount>;
    struct Cell
    {
        float gain = 0.0f;
        float contextDepth = 0.0f;
    };

    [[nodiscard]] bool setCell(std::size_t output, std::size_t input,
                               Cell value) noexcept;
    [[nodiscard]] Cell cell(std::size_t output, std::size_t input) const noexcept;
    [[nodiscard]] Values process(const Values& input, float context) const noexcept;

private:
    std::array<std::array<Cell, channelCount>, channelCount> cells {};
};

class SharedParameterPool final
{
public:
    static constexpr std::size_t valueCount = 16;
    static constexpr std::size_t consumerCount = 8;
    enum class Mode : std::uint8_t { direct, rotate, hold };
    struct Route
    {
        std::size_t first = 0;
        std::size_t second = 0;
        float blend = 0.0f;
    };

    [[nodiscard]] bool setValue(std::size_t index, float value) noexcept;
    [[nodiscard]] float value(std::size_t index) const noexcept;
    [[nodiscard]] bool setRoute(std::size_t consumer, Route route) noexcept;
    void setMode(Mode value) noexcept { mode = value; }
    void reset() noexcept;
    [[nodiscard]] float read(std::size_t consumer, float context,
                             bool latchEvent = false) noexcept;

private:
    std::array<float, valueCount> values {};
    std::array<Route, consumerCount> routes {};
    std::array<float, consumerCount> held {};
    std::array<bool, consumerCount> hasHeld {};
    Mode mode = Mode::direct;
};
}
