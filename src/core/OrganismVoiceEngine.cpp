// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/OrganismVoiceEngine.h"

#include "core/Numeric.h"

namespace aquorbium
{
void OrganismVoiceEngine::prepare(double sampleRate) noexcept
{
    const auto rate = numeric::sampleRate(sampleRate);
    oscillator.prepare(rate);
    pulsar.prepare(rate);
    granularStriker.prepare(rate);
    resonator.prepare(rate);
    reset();
}

void OrganismVoiceEngine::setSeed(std::uint32_t value) noexcept
{
    const auto seed = value == 0 ? 1u : value;
    pulsar.setSeed(seed ^ 0xa39f27d1u);
    granularStriker.setSeed(seed ^ 0x4c957f2du);
    resonator.setSeed(seed);
}

void OrganismVoiceEngine::reset(float phase) noexcept
{
    oscillator.reset(phase);
    pulsar.reset(phase);
    granularStriker.reset();
    resonator.reset();
}

OrganismVoiceEngine::Telemetry OrganismVoiceEngine::telemetry(
    Ecosystem::Species species, bool modalResonatorEnabled,
    bool granularStrikerEnabled) const noexcept
{
    auto generator = Generator::Silence;
    auto implemented = true;
    switch (species)
    {
        case Ecosystem::Species::Oscillator:
            generator = Generator::MorphOscillator;
            break;
        case Ecosystem::Species::Pulsar:
            generator = Generator::Pulsar;
            break;
        case Ecosystem::Species::Resonator:
            generator = modalResonatorEnabled
                ? Generator::ModalResonator : Generator::MorphFallback;
            break;
        case Ecosystem::Species::GranularStriker:
            generator = granularStrikerEnabled
                ? Generator::GranularStriker : Generator::MorphFallback;
            break;
        case Ecosystem::Species::PhysicalString:
            generator = Generator::PhysicalString;
            implemented = false;
            break;
    }
    return { species, generator, implemented, granularStriker.telemetry(),
             resonator.telemetry() };
}
}
