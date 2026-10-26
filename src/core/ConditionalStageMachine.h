// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace aquorbium
{
// Eight bounded structural stages with an optional context-driven jump at the
// end of each stage. Context is data-only and does not depend on Ecosystem.
class ConditionalStageMachine final
{
public:
    static constexpr std::size_t stageCount = 8;

    enum class Condition
    {
        none,
        dangerAbove,
        night,
        crackAbove
    };

    struct Stage
    {
        float value = 0.0f;
        std::uint32_t durationTicks = 1;
        std::size_t next = 0;
        std::size_t jump = 0;
        Condition condition = Condition::none;
        float threshold = 0.5f;
    };

    struct Context
    {
        float danger = 0.0f;
        bool isNight = false;
        float crackStress = 0.0f;
    };

    struct Output
    {
        float value = 0.0f;
        std::size_t stage = 0;
        std::size_t nextStage = 0;
        bool entered = false;
        bool advanced = false;
        bool jumped = false;
    };

    ConditionalStageMachine() noexcept;
    void setStage(std::size_t index, Stage value) noexcept;
    void reset(std::size_t initialStage = 0) noexcept;
    [[nodiscard]] Output tick(Context context) noexcept;
    [[nodiscard]] const Stage& stage(std::size_t index) const noexcept;
    [[nodiscard]] std::size_t currentStage() const noexcept { return current; }

private:
    [[nodiscard]] static Context sanitized(Context value) noexcept;
    [[nodiscard]] static bool conditionMet(const Stage& value,
                                           const Context& context) noexcept;

    std::array<Stage, stageCount> stages {};
    std::size_t current = 0;
    std::uint32_t elapsed = 0;
};
}
