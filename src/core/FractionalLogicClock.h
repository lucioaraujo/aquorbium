// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include <cstdint>

namespace aquorbium
{
class FractionalLogicClock final
{
public:
    enum class Logic : std::uint8_t { And, Or, Xor };

    struct Ratio
    {
        std::uint8_t numerator = 1;
        std::uint8_t denominator = 1;
    };

    struct Parameters
    {
        float beatsPerMinute = 120.0f;
        Ratio branchA { 1, 1 };
        Ratio branchB { 3, 2 };
        float duty = 0.2f;
        Logic logic = Logic::Or;
    };

    struct Output
    {
        bool branchA = false;
        bool branchB = false;
        bool gate = false;
        bool trigger = false;
        std::uint64_t cyclesA = 0;
        std::uint64_t cyclesB = 0;
    };

    void prepare(double newSampleRate) noexcept;
    void reset(double phaseA = 0.5, double phaseB = 0.5) noexcept;
    void setParameters(Parameters value) noexcept;
    [[nodiscard]] Parameters parameters() const noexcept { return controls; }
    [[nodiscard]] Output tick() noexcept;

private:
    static Ratio sanitized(Ratio value) noexcept;
    [[nodiscard]] static bool combine(bool left, bool right, Logic logic) noexcept;

    Parameters controls {};
    double sampleRate = 48000.0;
    double phaseA = 0.5;
    double phaseB = 0.5;
    double incrementA = 0.0;
    double incrementB = 0.0;
    std::uint64_t cyclesA = 0;
    std::uint64_t cyclesB = 0;
    bool previousGate = false;
};
}
