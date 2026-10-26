// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace aquorbium
{
// Bounded structural-decision prototype. State meanings remain outside this
// class so ecology can later map them to presence, strategy or musical form.
class BiomaMarkov final
{
public:
    static constexpr std::size_t stateCount = 8;
    using TransitionRow = std::array<float, stateCount>;

    struct Parameters
    {
        float contextInfluence = 0.45f;
        float inertia = 0.35f;
        float mutation = 0.05f;
    };

    struct Context
    {
        float target = 0.5f;
        float intensity = 0.0f;
    };

    BiomaMarkov() noexcept;
    void setSeed(std::uint32_t value) noexcept;
    void reset(std::size_t initialState = 0) noexcept;
    void setParameters(Parameters value) noexcept;
    void setTransitionRow(std::size_t state, TransitionRow weights) noexcept;
    void restoreDefaultTransitions() noexcept;
    [[nodiscard]] std::size_t advance(Context context) noexcept;
    [[nodiscard]] std::size_t advance() noexcept { return advance(Context {}); }
    [[nodiscard]] std::size_t currentState() const noexcept { return state; }
    [[nodiscard]] Parameters parameters() const noexcept { return controls; }
    [[nodiscard]] const TransitionRow& transitionRow(std::size_t index) const noexcept;
    [[nodiscard]] std::uint64_t transitionCount() const noexcept { return transitions; }

private:
    [[nodiscard]] float nextRandomUnit() noexcept;
    static TransitionRow sanitized(TransitionRow value,
                                   std::size_t fallbackState) noexcept;

    std::array<TransitionRow, stateCount> matrix {};
    Parameters controls {};
    std::uint32_t seed = 1u;
    std::uint32_t randomState = 1u;
    std::size_t state = 0;
    std::uint64_t transitions = 0;
};
}
