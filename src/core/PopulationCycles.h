// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

namespace aquorbium
{
// Predator and prey abundances as Lotka-Volterra cycles
// (CONCEITOS_VERIFICACAO_E_APROFUNDAMENTO.md §2.4): plenty of prey, the
// predators grow, the prey become scarce, the predators starve, the prey
// recover - a long form of several minutes that comes from ecology.
//
//   dN/dt = alpha N - beta N P        dP/dt = delta N P - gamma P
//
// Integrated in logarithms with the symplectic Euler method, which keeps
// the orbits closed and the populations positive for any step - the same
// concern for stability that Fontana (2012, XIX CIM: "Interactive sound
// synthesis by the Lotka-Volterra population model") answers with an
// implicit filter network. The equilibrium is N* = P* = 1; small cycles last
// about 2 pi / sqrt(alpha gamma) seconds. Real events feed the model:
// a capture removes prey and feeds the predators.
class PopulationCycles final
{
public:
    void reset(float prey = 1.5f, float predators = 0.7f) noexcept;
    // period of the small cycles, in seconds
    void setPeriod(float seconds) noexcept;
    void advance(float dt) noexcept;
    // a capture: the prey lose a little, the predators gain a little
    void capture(float amount = 1.0f) noexcept;

    [[nodiscard]] float prey() const noexcept;
    [[nodiscard]] float predators() const noexcept;

private:
    float logPrey = 0.405f;      // ln 1.5
    float logPredators = -0.357f; // ln 0.7
    float rate = 6.2831853f / 150.0f;  // alpha = gamma; beta = delta = rate
};
}
