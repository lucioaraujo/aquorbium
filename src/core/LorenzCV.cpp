// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/LorenzCV.h"
#include "core/Numeric.h"

#include <cmath>
#include <limits>

namespace aquorbium
{
void LorenzCV::prepare(double newSampleRate) noexcept
{
    sampleRate = numeric::sampleRate(newSampleRate);
    // 15 ms output slew hides the 200 Hz control clock without changing the
    // model integration step or requiring audio-rate RK4.
    slewGain = static_cast<float>(1.0 - std::exp(-1.0 / (0.015 * sampleRate)));
    reset();
}

void LorenzCV::setSeed(std::uint32_t value) noexcept
{
    seed = value == 0u ? 1u : value;
    reset();
}

void LorenzCV::setParameters(Parameters value) noexcept
{
    controls.sigma = numeric::clampFinite(value.sigma, 0.1f, 40.0f, controls.sigma);
    controls.rho = numeric::clampFinite(value.rho, 0.1f, 60.0f, controls.rho);
    controls.beta = numeric::clampFinite(value.beta, 0.1f, 8.0f, controls.beta);
    controls.dT = numeric::clampFinite(value.dT, 0.0005f, 0.02f, controls.dT);
}

void LorenzCV::resetState() noexcept
{
    state = { 0.1 + static_cast<double>(seed & 0xffffu) / 327680.0,
              0.01 + static_cast<double>((seed >> 16) & 0xffffu) / 655360.0,
              0.0 };
}

void LorenzCV::reset() noexcept
{
    resetState();
    current = {};
    target = {};
    controlPhase = 0.0;
    steps = 0;
    recoveryCount = 0;
}

LorenzCV::State LorenzCV::derivative(State value) const noexcept
{
    return { controls.sigma * (value.y - value.x),
             value.x * (controls.rho - value.z) - value.y,
             value.x * value.y - controls.beta * value.z };
}

void LorenzCV::step() noexcept
{
    const auto dt = static_cast<double>(controls.dT);
    const auto k1 = derivative(state);
    const auto k2 = derivative({ state.x + dt * k1.x * 0.5,
                                  state.y + dt * k1.y * 0.5,
                                  state.z + dt * k1.z * 0.5 });
    const auto k3 = derivative({ state.x + dt * k2.x * 0.5,
                                  state.y + dt * k2.y * 0.5,
                                  state.z + dt * k2.z * 0.5 });
    const auto k4 = derivative({ state.x + dt * k3.x,
                                  state.y + dt * k3.y,
                                  state.z + dt * k3.z });
    state.x += dt * (k1.x + 2.0 * k2.x + 2.0 * k3.x + k4.x) / 6.0;
    state.y += dt * (k1.y + 2.0 * k2.y + 2.0 * k3.y + k4.y) / 6.0;
    state.z += dt * (k1.z + 2.0 * k2.z + 2.0 * k3.z + k4.z) / 6.0;
    if (!std::isfinite(state.x) || !std::isfinite(state.y)
        || !std::isfinite(state.z))
    {
        resetState();
        if (recoveryCount != std::numeric_limits<std::uint64_t>::max())
            ++recoveryCount;
    }
    target = { static_cast<float>(std::tanh(state.x / 20.0)),
               static_cast<float>(std::tanh(state.y / 25.0)),
               static_cast<float>(std::tanh((state.z - 25.0) / 25.0)) };
    if (steps != std::numeric_limits<std::uint64_t>::max()) ++steps;
}

LorenzCV::Output LorenzCV::tick() noexcept
{
    controlPhase += 200.0 / sampleRate;
    if (controlPhase >= 1.0)
    {
        controlPhase -= 1.0;
        step();
    }
    current.x += (target.x - current.x) * slewGain;
    current.y += (target.y - current.y) * slewGain;
    current.z += (target.z - current.z) * slewGain;
    return current;
}
}
