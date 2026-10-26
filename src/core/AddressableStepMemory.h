// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace aquorbium
{
// Fixed event-rate memory for the geometric sequencer. Each address carries
// four independent control values and can be read either by a sequential
// cursor or directly from a normalized control signal.
class AddressableStepMemory final
{
public:
    static constexpr std::size_t maximumSteps = 32;

    struct Step
    {
        float pitchSemitones = 0.0f;
        float gateDuration = 0.5f;
        float strategyBias = 0.0f;
        float desireAmount = 0.5f;
    };

    struct Output
    {
        Step step;
        std::size_t index = 0;
        bool wrapped = false;
        std::uint64_t event = 0;
    };

    AddressableStepMemory() noexcept;

    void setLength(std::size_t value) noexcept;
    [[nodiscard]] bool setStep(std::size_t index, const Step& value) noexcept;
    [[nodiscard]] const Step& step(std::size_t index) const noexcept;
    void reset(std::size_t startIndex = 0) noexcept;

    [[nodiscard]] Output advance() noexcept;
    [[nodiscard]] Output address(float normalizedAddress) noexcept;

    [[nodiscard]] std::size_t length() const noexcept { return activeLength; }
    [[nodiscard]] std::size_t cursor() const noexcept { return sequentialCursor; }

private:
    [[nodiscard]] static Step sanitized(const Step& value) noexcept;
    [[nodiscard]] Output outputFor(std::size_t index, bool wrapped) noexcept;

    std::array<Step, maximumSteps> steps {};
    std::size_t activeLength = 8;
    std::size_t sequentialCursor = 0;
    std::uint64_t eventCounter = 0;
};
}
