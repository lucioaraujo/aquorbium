// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "app/PerformanceSceneControls.h"
#include "app/PerformanceSceneFile.h"
#include "app/PerformancePresetBank.h"
#include "core/FixedLatestSnapshot.h"

#include <bit>
#include <cmath>
#include <iostream>
#include <memory>

#define CHECK(condition) do { if (!(condition)) { \
    std::cerr << "scene playback requirement failed at line " << __LINE__ << ": " #condition "\n"; \
    return false; } } while (false)

bool runScenePlaybackTests()
{
    using namespace aquorbium::app;
    using Key = EcologyParameterState::Key;
    PerformancePresetBank bank;
    std::array<AudioControlFrame, PerformancePresetBank::capacity> originalFrames;
    AudioControlFrame lens;
    lens.lensHeld = true;
    lens.lensX = 0.0f;
    lens.lensY = 1.0f;
    lens.lensZ = 0.0f;
    for (std::size_t slot = 0; slot < PerformancePresetBank::capacity; ++slot)
    {
        const auto value = static_cast<double>(slot) / 3.0;
        PerformanceParameterState state, imported;
        EcologyParameterState ecology;
        CHECK(ecology.set(Key::density, value));
        CHECK(ecology.set(Key::calm, 1.0 - value));
        CHECK(ecology.set(Key::cohesion, value));
        CHECK(ecology.set(Key::danger, value));
        CHECK(ecology.set(Key::sparseFocus, value));
        CHECK(ecology.set(Key::energyMemory, value));
        CHECK(ecology.set(Key::geometricCycle, value));
        CHECK(ecology.set(Key::collectiveDrift, value));
        CHECK(state.setEcology(ecology.snapshot()));
        for (std::size_t index = 0; index < PerformanceParameterState::parameters.size(); ++index)
            CHECK(state.set(PerformanceParameterState::parameters[index].id,
                static_cast<double>(index + 1) / 20.0 + static_cast<double>(slot) / 100.0));
        state.setAutonomous(slot % 2 == 0);
        originalFrames[slot] = sceneControls(state, lens);
        CHECK(PerformanceSceneFile::decode(state.snapshot().toXmlString(), imported).wasOk());
        CHECK(bank.store(slot, imported));
        CHECK(bank.rename(slot, "Scene " + juce::String(static_cast<int>(slot) + 1)));
        const auto mapped = sceneControls(imported, lens);
        CHECK(mapped.density == static_cast<float>(value));
        CHECK(mapped.drift == static_cast<float>(value));
        CHECK(mapped.cohesion == static_cast<float>(value));
        CHECK(mapped.danger == static_cast<float>(value));
        CHECK(mapped.sparseFocus == static_cast<float>(value));
        CHECK(mapped.energyMemory == static_cast<float>(value));
        CHECK(mapped.geometricCycle == static_cast<float>(value));
        CHECK(mapped.collectiveDrift == static_cast<float>(value));
#define CHECK_MAPPING(member) CHECK(mapped.member == static_cast<float>(state.get(#member)))
        CHECK_MAPPING(ecosystemMorph);
        CHECK_MAPPING(geometryInfluence);
        CHECK_MAPPING(rhythmDensity);
        CHECK_MAPPING(orbitRotation);
        CHECK_MAPPING(intervalBias);
        CHECK_MAPPING(geometryMutation);
        CHECK_MAPPING(strikerInfluence);
        CHECK_MAPPING(strikerMatter);
        CHECK_MAPPING(strikerPopulation);
        CHECK_MAPPING(strikerScatter);
        CHECK_MAPPING(spaceReturn);
        CHECK_MAPPING(spaceGesture);
        CHECK_MAPPING(spaceSize);
        CHECK_MAPPING(spaceSpread);
        CHECK_MAPPING(spaceTone);
        CHECK_MAPPING(spaceMemory);
        CHECK_MAPPING(environmentInfluence);
        CHECK_MAPPING(environmentFilterMix);
        CHECK_MAPPING(environmentCentre);
        CHECK_MAPPING(environmentSpan);
        CHECK_MAPPING(environmentResonance);
        CHECK_MAPPING(environmentAmbience);
        CHECK_MAPPING(tailInfluence);
        CHECK_MAPPING(tailDecay);
        CHECK_MAPPING(tailDamping);
        CHECK_MAPPING(tailNightmare);
        CHECK_MAPPING(formInfluence);
        CHECK_MAPPING(formContext);
        CHECK_MAPPING(formInertia);
        CHECK_MAPPING(formMutation);
        CHECK_MAPPING(masterTrim);
        CHECK_MAPPING(masterCeiling);
        CHECK_MAPPING(masterRelease);
        CHECK_MAPPING(creativeMaster);
        CHECK_MAPPING(structureInfluence);
        CHECK_MAPPING(structureMemory);
        CHECK_MAPPING(structureHarmony);
        CHECK_MAPPING(structurePhase);
        CHECK_MAPPING(musicInfluence);
        CHECK_MAPPING(musicReaction);
        CHECK_MAPPING(musicMemory);
        CHECK_MAPPING(musicWater);
        for (std::size_t i = 0; i < 5; ++i)
        {
            CHECK(mapped.timbreBody[i] == static_cast<float>(state.get(("timbreKind" + juce::String(static_cast<int>(i))).toRawUTF8())));
            CHECK(mapped.timbreMatter[i] == static_cast<float>(state.get(("timbreMatter" + juce::String(static_cast<int>(i))).toRawUTF8())));
            CHECK(mapped.timbreMix[i] == static_cast<float>(state.get(("timbreMix" + juce::String(static_cast<int>(i))).toRawUTF8())));
        }
        CHECK_MAPPING(timbreEcology);
        CHECK_MAPPING(musicSalinity);
#undef CHECK_MAPPING
        for (std::size_t index = 0; index < mapped.moduleBank.size(); ++index)
            CHECK(mapped.moduleBank[index] == static_cast<float>(state.get(
                PerformanceParameterState::parameters[
                    PerformanceParameterState::structureMemoryParameterCount + index].id)));
        CHECK(mapped.autonomous == (slot % 2 == 0));
        CHECK(mapped.lensHeld && mapped.lensX == 0.0f && mapped.lensY == 1.0f);
    }
    for (const auto sampleRate : { 44100.0, 48000.0, 96000.0 })
    for (const std::size_t samples : { 64u, 257u, 512u })
    {
        auto reference = std::make_unique<aquorbium::BiomaEngine>();
        auto roundtrip = std::make_unique<aquorbium::BiomaEngine>();
        for (auto* engine : { reference.get(), roundtrip.get() })
        {
            engine->prepare(sampleRate);
            engine->setSeed(0x20260916u);
        }
        aquorbium::FixedLatestSnapshot<AudioControlFrame> mailbox;
        AudioControlFrame applied;
        std::array<float, 512> leftA {}, rightA {}, leftB {}, rightB {};
        double energy = 0.0;
        for (int block = 0; block < 128; ++block)
        {
            if (block % 8 == 0)
            {
                const auto slot = static_cast<std::size_t>((block / 8) % 4);
                PerformanceParameterState imported;
                // Coalesced intermediate scenes must not leak mixed fields.
                for (std::size_t candidate = 0; candidate < 4; ++candidate)
                {
                    CHECK(bank.recall(candidate, imported));
                    mailbox.publish(sceneControls(imported, lens));
                }
                CHECK(bank.recall(slot, imported));
                // Store/import undo operates on the bank, not the currently
                // sounding frame. Its restored parameters still travel intact.
                auto overwritten = imported;
                CHECK(overwritten.set("spaceReturn", 0.99));
                CHECK(bank.store(slot, overwritten));
                CHECK(bank.undo());
                CHECK(bank.recall(slot, imported));
                mailbox.publish(sceneControls(imported, lens));
                CHECK(mailbox.consumeLatest(applied));
                originalFrames[slot].apply(*reference, block == 0);
                applied.apply(*roundtrip, block == 0);
                CHECK(!mailbox.consumeLatest(applied));
            }
            if (block % 16 == 0)
                for (auto* engine : { reference.get(), roundtrip.get() })
                    engine->noteOn(48 + (block / 16) % 12, 0.72f);
            if (block % 16 == 12)
                for (auto* engine : { reference.get(), roundtrip.get() })
                    engine->noteOff(48 + (block / 16) % 12);
            reference->render(leftA.data(), rightA.data(), samples);
            roundtrip->render(leftB.data(), rightB.data(), samples);
            for (std::size_t sample = 0; sample < samples; ++sample)
            {
                CHECK(std::isfinite(leftB[sample]) && std::isfinite(rightB[sample]));
                CHECK(std::bit_cast<std::uint32_t>(leftA[sample]) == std::bit_cast<std::uint32_t>(leftB[sample]));
                CHECK(std::bit_cast<std::uint32_t>(rightA[sample]) == std::bit_cast<std::uint32_t>(rightB[sample]));
                energy += static_cast<double>(leftB[sample]) * leftB[sample];
            }
        }
        CHECK(energy > 0.0);
    }
    // the creative MASTER's EQ order travels with the scene (2026-10-23)
    {
        PerformanceParameterState state, imported;
        CHECK(!sceneControls(state, lens).masterEqAfterDynamics);
        CHECK(state.set("masterEqOrder", 1.0));
        CHECK(PerformanceSceneFile::decode(state.snapshot().toXmlString(), imported).wasOk());
        CHECK(sceneControls(imported, lens).masterEqAfterDynamics);
    }
    std::cout << "Scene XML -> named slot -> store/undo -> coherent frame -> DSP: 9 configurations, 144 transitions, bitwise match\n";
    return true;
}
