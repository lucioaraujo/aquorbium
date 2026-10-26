// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "app/AudioControlFrame.h"
#include "app/RuntimeAuditProtocol.h"

#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <limits>
#include <utility>

namespace
{
using Frame = aquorbium::app::AudioControlFrame;
using Member = float Frame::*;
struct Control { const char* name; Member member; };
#define CONTROL(member) Control { #member, &Frame::member }
constexpr std::array controls {
    CONTROL(density), CONTROL(drift), CONTROL(cohesion), CONTROL(danger),
    CONTROL(ecosystemMorph), CONTROL(sparseFocus), CONTROL(energyMemory),
    CONTROL(geometricCycle), CONTROL(collectiveDrift), CONTROL(geometryInfluence),
    CONTROL(rhythmDensity), CONTROL(orbitRotation), CONTROL(intervalBias),
    CONTROL(geometryMutation), CONTROL(strikerInfluence), CONTROL(strikerMatter),
    CONTROL(strikerPopulation), CONTROL(strikerScatter), CONTROL(spaceReturn),
    CONTROL(spaceGesture), CONTROL(spaceSize), CONTROL(spaceSpread),
    CONTROL(spaceTone), CONTROL(spaceMemory), CONTROL(environmentInfluence),
    CONTROL(environmentFilterMix), CONTROL(environmentCentre),
    CONTROL(environmentSpan), CONTROL(environmentResonance),
    CONTROL(environmentAmbience), CONTROL(tailInfluence), CONTROL(tailDecay),
    CONTROL(tailDamping), CONTROL(tailNightmare), CONTROL(formInfluence),
    CONTROL(formContext), CONTROL(formInertia), CONTROL(formMutation),
    CONTROL(structureInfluence), CONTROL(structureMemory),
    CONTROL(structureHarmony), CONTROL(structurePhase)
};
#undef CONTROL

bool compare(const char* name, const Frame& a, const Frame& b, bool expectDifference, int seconds = 4,
             bool strikerOnly = false, double minimumRelativeDb = -999.0)
{
    auto engineA = std::make_unique<aquorbium::BiomaEngine>();
    auto engineB = std::make_unique<aquorbium::BiomaEngine>();
    for (auto* engine : { engineA.get(), engineB.get() })
    {
        engine->prepare(48000.0);
        engine->setSeed(42);
        if (strikerOnly)
            for (int species = 0; species < 5; ++species)
            {
                const auto kind = static_cast<aquorbium::Ecosystem::Species>(species);
                engine->setSpeciesGain(kind, kind == aquorbium::Ecosystem::Species::GranularStriker ? 1.0f : 0.0f);
            }
    }
    a.apply(*engineA, true);
    b.apply(*engineB, true);
    std::array<float, 240> leftA {}, rightA {}, leftB {}, rightB {};
    double squaredDifference = 0.0;
    double squaredReference = 0.0;
    float peak = 0.0f;
    bool identical = true;
    // Identical notes every 500 ms. Slow autonomous geometry needs several
    // life events, not only the first gates of an orbit.
    for (int block = 0; block < seconds * 200; ++block)
    {
        if (block % 100 == 0)
            for (auto* engine : { engineA.get(), engineB.get() })
                engine->noteOn(48 + (block / 100) % 5, 0.6f);
        if (block % 100 == 70)
            for (auto* engine : { engineA.get(), engineB.get() })
                for (int note = 48; note < 53; ++note) engine->noteOff(note);
        engineA->render(leftA.data(), rightA.data(), leftA.size());
        engineB->render(leftB.data(), rightB.data(), leftB.size());
        for (std::size_t sample = 0; sample < leftA.size(); ++sample)
        {
            const std::array samples { leftA[sample], rightA[sample], leftB[sample], rightB[sample] };
            for (const auto value : samples)
            {
                if (!std::isfinite(value)) return false;
                peak = std::max(peak, std::abs(value));
            }
            identical = identical
                && std::bit_cast<std::uint32_t>(leftA[sample]) == std::bit_cast<std::uint32_t>(leftB[sample])
                && std::bit_cast<std::uint32_t>(rightA[sample]) == std::bit_cast<std::uint32_t>(rightB[sample]);
            const auto dl = static_cast<double>(leftA[sample]) - leftB[sample];
            const auto dr = static_cast<double>(rightA[sample]) - rightB[sample];
            squaredDifference += dl * dl + dr * dr;
            squaredReference += static_cast<double>(leftA[sample]) * leftA[sample]
                + static_cast<double>(rightA[sample]) * rightA[sample];
        }
    }
    const auto difference = std::sqrt(squaredDifference / (48000.0 * seconds * 2.0));
    const auto reference = std::sqrt(squaredReference / (48000.0 * seconds * 2.0));
    const auto relativeDb = difference > 0.0 && reference > 0.0
        ? 20.0 * std::log10(difference / reference) : -999.0;
    std::cout << name << ",difference_rms=" << difference << ",reference_rms=" << reference
              << ",difference_relative_db=" << relativeDb
              << ",peak=" << peak << '\n';
    // This is an engineering floor for presence in the integrated mix, not a
    // claim of human audibility. Keep a conservative peak margin in this scene.
    const auto passed = peak < 0.8f && (expectDifference
        ? difference > 1.0e-7 && relativeDb >= minimumRelativeDb : identical);
    if (!passed) std::cerr << "control response failed: " << name << '\n';
    return passed;
}
}

int main()
{
    using Protocol = aquorbium::app::RuntimeAuditProtocol;
    using Phase = aquorbium::app::AuditPhase;
    constexpr std::array<std::pair<double, Phase>, 10> transitions {{
        std::pair { 0.0, Phase::baseline }, { 4.0, Phase::macros }, { 9.0, Phase::geometry },
        { 14.0, Phase::lens }, { 18.0, Phase::release }, { 20.0, Phase::scenes },
        { 35.0, Phase::autonomy }, { 45.0, Phase::notes }, { 58.0, Phase::release },
        { 60.0, Phase::finished }
    }};
    for (const auto& [time, expected] : transitions)
        if (Protocol::phase(time, true) != expected) return EXIT_FAILURE;
    if (Protocol::phase(20.0, false) != Phase::finished || Protocol::step(59.999, true) != 239)
        return EXIT_FAILURE;
    for (const auto invalid : { -1.0, std::numeric_limits<double>::infinity(),
                               std::numeric_limits<double>::quiet_NaN(), 1.0e100 })
        if (Protocol::step(invalid, true) != -1) return EXIT_FAILURE;
    for (const auto rate : { 44100.0, 48000.0, 96000.0 })
    {
        auto engine = std::make_unique<aquorbium::BiomaEngine>();
        engine->prepare(rate);
        const auto previous = engine->telemetry();
        engine->setDensity(1.0f);
        engine->setDrift(1.0f);
        if (engine->telemetry().densityCurrent != previous.densityCurrent
            || engine->telemetry().driftCurrent != previous.driftCurrent) return EXIT_FAILURE;
        float left = 0.0f, right = 0.0f;
        engine->render(&left, &right, 1);
        const auto first = engine->telemetry();
        const auto coefficient = static_cast<float>(-std::expm1(-1.0 / (rate * 0.015)));
        if (std::abs(first.densityCurrent - (previous.densityCurrent
                + (1.0f - previous.densityCurrent) * coefficient)) > 1.0e-7f
            || std::abs(first.driftCurrent - (previous.driftCurrent
                + (1.0f - previous.driftCurrent) * coefficient)) > 1.0e-7f
            || first.densityCurrent >= 1.0f || first.driftCurrent >= 1.0f)
            return EXIT_FAILURE;
    }
    Frame enabled;
    enabled.geometryInfluence = enabled.strikerInfluence = enabled.spaceGesture
        = enabled.environmentInfluence = enabled.tailInfluence
        = enabled.formInfluence = enabled.structureInfluence = 1.0f;
    enabled.environmentAmbience = 0.5f;
    // An influence control applied to an entirely neutral intention is
    // correctly a no-op. Use a non-neutral geometry for its response audit.
    enabled.rhythmDensity = 0.8f;
    enabled.orbitRotation = 0.7f;
    enabled.intervalBias = 0.9f;
    enabled.geometryMutation = 0.4f;
    for (const auto control : controls)
    {
        auto a = enabled, b = enabled;
        a.*(control.member) = 0.0f;
        b.*(control.member) = 1.0f;
        // Rotation is cyclic: -one orbit and +one orbit describe
        // the same gate mask. Do not mistake equivalent ends for a dead knob.
        // A one-step shift also avoids the shorter period of repeating masks
        // (e.g. six pulses in eight steps repeats after four steps).
        if (control.member == &Frame::orbitRotation) b.orbitRotation = 0.57f;
        const auto geometry = control.member == &Frame::geometryInfluence
            || control.member == &Frame::rhythmDensity || control.member == &Frame::orbitRotation
            || control.member == &Frame::intervalBias || control.member == &Frame::geometryMutation;
        const auto form = control.member == &Frame::formInfluence
            || control.member == &Frame::formContext
            || control.member == &Frame::formInertia
            || control.member == &Frame::formMutation;
        const auto structure = control.member == &Frame::structureInfluence
            || control.member == &Frame::structureMemory
            || control.member == &Frame::structureHarmony
            || control.member == &Frame::structurePhase;
        const auto striker = control.member == &Frame::strikerInfluence
            || control.member == &Frame::strikerMatter
            || control.member == &Frame::strikerPopulation
            || control.member == &Frame::strikerScatter;
        // audibility floors (2026-10-20): the striker and the space used to sit
        // 25-33 dB under the mix and were not heard; these floors keep them
        // from sinking back (measured after the change: striker -18..-11 dB,
        // space return -10, size -7.5, gesture -16.6)
        const auto minimumDb = striker ? -22.0
            : control.member == &Frame::spaceGesture ? -20.0
            : control.member == &Frame::spaceReturn ? -14.0
            : control.member == &Frame::spaceSize ? -12.0 : -999.0;
        if (!compare(control.name, a, b, true, (geometry || form || structure) ? 20 : 4,
                     false, minimumDb)) return EXIT_FAILURE;
    }
    auto a = enabled, b = enabled;
    a.strikerInfluence = b.strikerInfluence = 0.0f;
    a.strikerMatter = a.strikerPopulation = a.strikerScatter = 0.0f;
    b.strikerMatter = b.strikerPopulation = b.strikerScatter = 1.0f;
    if (!compare("striker_gate_zero", a, b, false)) return EXIT_FAILURE;
    a = b = enabled;
    a.spaceGesture = b.spaceGesture = 0.0f;
    a.spaceSize = a.spaceSpread = a.spaceTone = a.spaceMemory = 0.0f;
    b.spaceSize = b.spaceSpread = b.spaceTone = b.spaceMemory = 1.0f;
    if (!compare("space_gate_zero", a, b, false)) return EXIT_FAILURE;
    a = b = enabled;
    a.geometryInfluence = b.geometryInfluence = 0.0f;
    a.rhythmDensity = a.orbitRotation = a.intervalBias = a.geometryMutation = 0.0f;
    b.rhythmDensity = b.orbitRotation = b.intervalBias = b.geometryMutation = 1.0f;
    if (!compare("geometry_gate_zero", a, b, false)) return EXIT_FAILURE;
    a = b = enabled;
    a.environmentInfluence = b.environmentInfluence = 0.0f;
    a.environmentFilterMix = a.environmentCentre = a.environmentSpan
        = a.environmentResonance = a.environmentAmbience = 0.0f;
    b.environmentFilterMix = b.environmentCentre = b.environmentSpan
        = b.environmentResonance = b.environmentAmbience = 1.0f;
    if (!compare("environment_gate_zero", a, b, false)) return EXIT_FAILURE;
    a = b = enabled;
    a.tailInfluence = b.tailInfluence = 0.0f;
    a.tailDecay = a.tailDamping = a.tailNightmare = 0.0f;
    b.tailDecay = b.tailDamping = b.tailNightmare = 1.0f;
    if (!compare("tail_gate_zero", a, b, false)) return EXIT_FAILURE;
    a = b = enabled;
    a.formInfluence = b.formInfluence = 0.0f;
    a.formContext = a.formInertia = a.formMutation = 0.0f;
    b.formContext = b.formInertia = b.formMutation = 1.0f;
    if (!compare("form_gate_zero", a, b, false, 20)) return EXIT_FAILURE;
    a = b = enabled;
    a.structureInfluence = b.structureInfluence = 0.0f;
    a.structureMemory = a.structureHarmony = a.structurePhase = 0.0f;
    b.structureMemory = b.structureHarmony = b.structurePhase = 1.0f;
    if (!compare("structure_gate_zero", a, b, false, 20)) return EXIT_FAILURE;
    for (const auto control : { Control { "striker_matter_solo", &Frame::strikerMatter },
                               Control { "striker_population_solo", &Frame::strikerPopulation },
                               Control { "striker_scatter_solo", &Frame::strikerScatter } })
    {
        a = b = enabled;
        a.spaceReturn = b.spaceReturn = 0.0f;
        a.*(control.member) = 0.0f;
        b.*(control.member) = 1.0f;
        if (!compare(control.name, a, b, true, 8, true)) return EXIT_FAILURE;
    }
    // A bounded four-voice scenario guards against solving masked controls by
    // simply pushing the combined output into digital clipping.
    auto stress = std::make_unique<aquorbium::BiomaEngine>();
    stress->prepare(48000.0);
    stress->setSeed(42);
    Frame loud;
    loud.density = 1.0f;
    loud.strikerInfluence = loud.strikerMatter = loud.strikerPopulation
        = loud.strikerScatter = 1.0f;
    loud.spaceReturn = loud.spaceGesture = loud.spaceMemory = 1.0f;
    loud.tailInfluence = loud.tailDecay = loud.tailNightmare = 1.0f;
    loud.tailDamping = 0.0f;
    loud.formInfluence = loud.formContext = loud.formMutation = 1.0f;
    loud.formInertia = 0.0f;
    loud.structureInfluence = loud.structureMemory = loud.structureHarmony = 1.0f;
    loud.structurePhase = 0.75f;
    loud.apply(*stress, true);
    for (const auto note : { 48, 55, 60, 67 }) stress->noteOn(note, 1.0f);
    std::array<float, 240> stressLeft {}, stressRight {};
    float stressPeak = 0.0f;
    for (int block = 0; block < 1000; ++block)
    {
        stress->render(stressLeft.data(), stressRight.data(), stressLeft.size());
        for (std::size_t sample = 0; sample < stressLeft.size(); ++sample)
        {
            if (!std::isfinite(stressLeft[sample]) || !std::isfinite(stressRight[sample]))
                return EXIT_FAILURE;
            stressPeak = std::max(stressPeak,
                std::max(std::abs(stressLeft[sample]), std::abs(stressRight[sample])));
        }
    }
    std::cout << "four_voice_stress_peak=" << stressPeak << '\n';
    if (stressPeak >= 0.95f) return EXIT_FAILURE;
    return EXIT_SUCCESS;
}
