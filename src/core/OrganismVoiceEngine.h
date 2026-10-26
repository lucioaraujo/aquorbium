// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include "core/BiomaGranularStriker.h"
#include "core/BiomaModalResonator.h"
#include "core/BiomaMorphOscillator.h"
#include "core/BiomaPulsar.h"
#include "core/Ecosystem.h"

#include <cstdint>

namespace aquorbium
{
// Integration layer between an ecological species and its audible DSP body.
// Ecosystem remains control physics; the individual DSP engines remain usable
// without ecology. This class owns the compact bodies and dispatches without
// virtual calls, allocation, RTTI, or a numeric organism-index convention.
// Large delay memories may be owned once per voice bank and routed explicitly.
class OrganismVoiceEngine final
{
public:
    enum class Generator : std::uint8_t
    {
        Silence,
        MorphOscillator,
        Pulsar,
        GranularStriker,
        ModalResonator,
        PhysicalString,
        MorphFallback
    };

    struct Controls
    {
        Ecosystem::Species species = Ecosystem::Species::Oscillator;
        float frequency = 110.0f;
        float shape = 0.0f;
        float pulsarDensity = 0.4f;
        float pulsarCrackStress = 0.0f;
        float strength = 0.5f;
        float agility = 0.5f;
        float intelligence = 0.5f;
        bool granularStrikerEnabled = true;
        bool modalResonatorEnabled = true;
    };

    struct Telemetry
    {
        Ecosystem::Species species = Ecosystem::Species::Oscillator;
        Generator generator = Generator::Silence;
        bool implemented = false;
        BiomaGranularStriker::Telemetry granularStriker {};
        BiomaModalResonator::Telemetry resonator {};
    };

    void prepare(double sampleRate) noexcept;
    void setSeed(std::uint32_t value) noexcept;
    void reset(float phase = 0.0f) noexcept;
    void setPulseWidth(float value) noexcept { oscillator.setPulseWidth(value); }
    void setGranularStrikerParameters(
        const BiomaGranularStriker::Parameters& value) noexcept
    {
        granularStriker.setParameters(value);
    }
    void setGranularStrikerPerformance(
        const BiomaGranularStriker::PerformanceIntent& value) noexcept
    {
        granularStriker.setPerformanceIntent(value);
    }
    void exciteGranularStriker(float amount) noexcept { granularStriker.excite(amount); }
    void setResonatorParameters(const BiomaModalResonator::Parameters& value) noexcept
    {
        resonator.setParameters(value);
    }
    void setResonatorPerformance(
        const BiomaModalResonator::PerformanceIntent& value) noexcept
    {
        resonator.setPerformanceIntent(value);
    }
    void exciteResonator(float amount) noexcept { resonator.excite(amount); }

    [[nodiscard]] float renderSample(const Controls& controls) noexcept
    {
        // only the generator(s) the species plays are configured: a species
        // never changes, and the other generator's output is never heard.
        // Configuring both every sample was ~10 % of the engine's CPU in the
        // worst case (gprof, 2026-10-22); the output is bit-identical
        // (GoldenRenderTests). The oscillator also runs under the resonator
        // and the striker, for their comparison switches' phase continuity
        const auto pulsarSpecies = controls.species == Ecosystem::Species::Pulsar;
        if (pulsarSpecies)
        {
            pulsar.setFrequency(controls.frequency);
            pulsar.setDensity(controls.pulsarDensity);
            pulsar.setEcology(controls.pulsarCrackStress, controls.strength,
                              controls.agility, controls.intelligence);
        }
        else if (controls.species != Ecosystem::Species::PhysicalString)
        {
            oscillator.setFrequency(controls.frequency);
            oscillator.setShape(controls.shape);
        }

        switch (controls.species)
        {
            case Ecosystem::Species::Oscillator:
                return oscillator.tick();

            case Ecosystem::Species::Pulsar:
                return pulsar.tick();

            case Ecosystem::Species::Resonator:
            {
                // Both bodies advance while the comparison switch is used. The
                // switch changes the audible body, not ecology or phase continuity.
                const auto oscillatorSample = oscillator.tick();
                const auto resonatorSample = resonator.tick();
                return controls.modalResonatorEnabled
                    ? resonatorSample : oscillatorSample;
            }

            case Ecosystem::Species::GranularStriker:
            {
                // As with the modal technical comparison, both paths retain
                // continuity while only one is audible.
                const auto oscillatorSample = oscillator.tick();
                const auto strikerSample = granularStriker.tick();
                return controls.granularStrikerEnabled
                    ? strikerSample : oscillatorSample;
            }

            case Ecosystem::Species::PhysicalString:
                // The 128 KiB delay memory is deliberately owned once by each
                // BiomaEngine voice bank, not replicated inside every dispatcher.
                return 0.0f;
        }

        return 0.0f;
    }
    [[nodiscard]] Telemetry telemetry(Ecosystem::Species species,
                                      bool modalResonatorEnabled,
                                      bool granularStrikerEnabled) const noexcept;
    [[nodiscard]] const BiomaGranularStriker& granularStrikerEngine() const noexcept
    {
        return granularStriker;
    }
    [[nodiscard]] const BiomaModalResonator& modalResonator() const noexcept
    {
        return resonator;
    }

private:
    BiomaMorphOscillator oscillator;
    BiomaPulsar pulsar;
    BiomaGranularStriker granularStriker;
    BiomaModalResonator resonator;
};
}
