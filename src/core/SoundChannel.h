// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

namespace aquorbium
{
// The deep sound channel inside the sphere (CONCEITOS_VERIFICACAO_E_APROFUNDAMENTO.md
// §2.11; 2026-10-21). In the ocean the speed of sound falls with temperature
// towards the depths and rises again with pressure; the minimum - the SOFAR
// axis (Ewing & Worzel 1948) - bends every ray that leaves it back towards
// it, so sound made near the axis travels thousands of kilometres. Munk's
// canonical profile (1974, JASA 55:220-226) describes it:
//
//   c(z) = c1 [1 + eps (eta + e^-eta - 1)],  eta = 2 (z - z1) / B,
//   eps = 0.00737, B = 1300 m.
//
// Its axis is deep in warm seas (~1000-1300 m) and rises to the surface in
// polar water - under the ice the whole channel is a surface duct.
//
// Rewritten for the aquarium (y is height; the top of the sphere is the
// surface): the axis follows the water's temperature; inside the channel
// the spreading is cylindrical (10 log r instead of 20 log r: Urick,
// Principles of Underwater Sound, ch. 5), so organisms near the axis hear
// each other across the whole reef; a sound made in the channel does not
// leave it towards a listener far above or below, and a sound made outside
// does not enter it (refraction makes shadow zones). Bellhop (M. Porter's
// Acoustics Toolbox) is the study reference for the rays; no ray tracing is
// embedded - only these consequences. Static, no state: safe anywhere.
class SoundChannel final
{
public:
    static constexpr float sphereRadius = 3.84f;      // where the organisms swim
    static constexpr float halfWidth = 0.85f;          // the channel's half thickness, in units
    static constexpr float oceanDepthMetres = 5000.0f; // the sphere's height, as an ocean

    // the axis height, from the surface in polar water (0 C) to a third of
    // the depth in warm water (30 C); ice keeps it under the surface
    [[nodiscard]] static float axisHeight(float temperatureC) noexcept;
    // Munk's c(z)/c1 at a height, for an axis at axisY
    [[nodiscard]] static float speedRatio(float y, float axisY) noexcept;
    // how much of a sound is held in the channel at this height, 0 .. 1
    [[nodiscard]] static float trapping(float y, float axisY) noexcept;
    // the factor on the free-field level 1 / (1 + d): > 1 along the channel,
    // < 1 into a shadow, ~1 away from it
    [[nodiscard]] static float gain(float sourceY, float listenerY, float distance, float axisY) noexcept;
};
}
