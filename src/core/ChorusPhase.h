// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace aquorbium
{
// The temporal acoustic niche, as a chorus of coupled phase oscillators
// (CONCEITOS_VERIFICACAO_E_APROFUNDAMENTO.md §2.1). Krause describes the
// niche in frequency AND in time: in a mature habitat each species shifts its
// timing so as not to be masked. Aihara et al. model frog choruses as coupled
// phase oscillators: two neighbours call in anti-phase, three in tri-phase,
// neighbourhoods form two clusters in anti-phase (Sci Rep 4:3891, 2014;
// R Soc Open Sci 6:181117, 2019) - the ideas, rewritten here.
//
// Each organism owns a calling clock. It fires when its phase wraps; the
// ecological music lets a phrase begin only then. What an organism HEARS of
// another pulls its phase towards a target offset that depends on their
// relation: 0 (in phase: allies, a hunter locking on), 1/2 (anti-phase:
// rivals, a prey avoiding its hunter, indifferent neighbours sharing the
// niche), 1/4 (call and answer: a courting pair). The pull is a Kuramoto-like
// sine of the phase error, scaled by how loudly the other was heard, so
// alternation and synchrony EMERGE from listening rather than from rules.
//
// Fixed-size, deterministic, no allocation: safe on the audio thread.
class ChorusPhase final
{
public:
    static constexpr std::size_t voiceCount = 5;

    void reset(std::uint32_t seed) noexcept;
    // the natural calling rate of a voice, in cycles per step (16th note)
    void setNaturalRate(std::size_t voice, float cyclesPerStep) noexcept;
    // during a step: `listener` heard `source` at `level` (0..1); its phase
    // is pulled towards source + targetOffset (in cycles) with `gain`
    void hear(std::size_t listener, std::size_t source, float level, float targetOffset, float gain) noexcept;
    // advance one step; returns which voices fired (their phase wrapped)
    [[nodiscard]] std::array<bool, voiceCount> step() noexcept;

    [[nodiscard]] float phase(std::size_t voice) const noexcept { return phases[voice]; }
    // the signed phase difference b - a, wrapped to [-0.5, 0.5)
    [[nodiscard]] float difference(std::size_t a, std::size_t b) const noexcept;

private:
    std::array<float, voiceCount> phases {};
    std::array<float, voiceCount> rates {};
    std::array<float, voiceCount> pull {};
};
}
