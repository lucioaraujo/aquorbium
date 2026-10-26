// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace aquorbium
{
// Four bounded personality-memory states: continuously renewed, repeating,
// gradually replaced, or held at one position.
class EvolutionLoopMachine final
{
public:
    static constexpr std::size_t maximumLength = 64;

    enum class Mode
    {
        random,
        looping,
        evolving,
        locked
    };

    struct Output
    {
        float value = 0.0f;
        std::size_t index = 0;
        bool mutated = false;
        bool wrapped = false;
        Mode mode = Mode::random;
    };

    void setSeed(std::uint32_t value) noexcept;
    void setLength(std::size_t value) noexcept;
    void setMode(Mode value) noexcept { currentMode = value; }
    void reset() noexcept;
    void rewind() noexcept { cursor = 0; }
    [[nodiscard]] Output advance(float evolutionProbability = 0.0f) noexcept;

    [[nodiscard]] std::size_t length() const noexcept { return activeLength; }
    [[nodiscard]] Mode mode() const noexcept { return currentMode; }
    [[nodiscard]] float memoryValue(std::size_t index) const noexcept;

private:
    [[nodiscard]] float nextUnit() noexcept;
    [[nodiscard]] float nextSigned() noexcept { return nextUnit() * 2.0f - 1.0f; }

    std::array<float, maximumLength> memory {};
    std::uint32_t seed = 1u;
    std::uint32_t randomState = 1u;
    std::size_t activeLength = 8;
    std::size_t cursor = 0;
    Mode currentMode = Mode::random;
};
}
