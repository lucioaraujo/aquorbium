// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "app/EcologyParameterState.h"
#include "app/PerformanceParameterState.h"
#include "app/PerformancePresetBank.h"
#include "app/Tutorial.h"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>

#define REQUIRE(condition) do { if (!(condition)) { \
    std::cerr << "requirement failed at line " << __LINE__ << ": " #condition "\n"; \
    return EXIT_FAILURE; } } while (false)

bool runSceneFileTests();
bool runScenePlaybackTests();
bool runPresetBankTests();
bool runEngineStressTests();
bool runAutomationClickTests();
bool runBypassSweepTests();

int main(int argc, char** argv)
{
    if (argc == 2 && juce::String(argv[1]) == "--scene-playback")
        return runScenePlaybackTests() ? EXIT_SUCCESS : EXIT_FAILURE;
    if (argc == 2 && juce::String(argv[1]) == "--preset-bank")
        return runPresetBankTests() ? EXIT_SUCCESS : EXIT_FAILURE;
    if (argc == 2 && juce::String(argv[1]) == "--engine-stress")
        return runEngineStressTests() ? EXIT_SUCCESS : EXIT_FAILURE;
    if (argc == 2 && juce::String(argv[1]) == "--automation")
        return runAutomationClickTests() ? EXIT_SUCCESS : EXIT_FAILURE;
    if (argc == 2 && juce::String(argv[1]) == "--bypass-sweep")
        return runBypassSweepTests() ? EXIT_SUCCESS : EXIT_FAILURE;
    using aquorbium::app::EcologyParameterState;
    using Key = EcologyParameterState::Key;

    EcologyParameterState state;
    REQUIRE(state.get(Key::density) == 0.45);
    REQUIRE(state.get(Key::calm) == 0.80);
    REQUIRE(state.get(Key::sparseFocus) == 0.25);
    REQUIRE(state.set(Key::density, 0.73));
    REQUIRE(state.set(Key::calm, -2.0));
    REQUIRE(state.get(Key::calm) == 0.0);
    REQUIRE(state.set(Key::danger, 3.0));
    REQUIRE(state.get(Key::danger) == 1.0);
    REQUIRE(!state.set(Key::density, std::numeric_limits<double>::quiet_NaN()));
    REQUIRE(state.get(Key::density) == 0.73);

    auto ecologicalCopy = state;
    REQUIRE(ecologicalCopy.set(Key::density, 0.12));
    REQUIRE(state.get(Key::density) == 0.73);
    EcologyParameterState ecologicalAssignment;
    ecologicalAssignment = state;
    REQUIRE(ecologicalAssignment.set(Key::density, 0.13));
    REQUIRE(state.get(Key::density) == 0.73);

    auto snapshot = state.snapshot();
    REQUIRE(snapshot.hasType("AQUORBIUM_ECOLOGY_V1"));
    snapshot.setProperty("density", 0.18, nullptr);
    REQUIRE(state.get(Key::density) == 0.73);

    const auto xml = snapshot.toXmlString();
    const auto parsed = juce::parseXML(xml);
    REQUIRE(parsed != nullptr);
    EcologyParameterState recovered;
    REQUIRE(recovered.restore(juce::ValueTree::fromXml(*parsed)));
    REQUIRE(recovered.get(Key::density) == 0.18);
    REQUIRE(recovered.get(Key::danger) == 1.0);

    auto incomplete = recovered.snapshot();
    incomplete.removeProperty("cohesion", nullptr);
    REQUIRE(!recovered.restore(incomplete));
    REQUIRE(recovered.get(Key::density) == 0.18);
    auto invalid = recovered.snapshot();
    invalid.setProperty("danger", std::numeric_limits<double>::infinity(), nullptr);
    REQUIRE(!recovered.restore(invalid));
    REQUIRE(recovered.get(Key::danger) == 1.0);
    REQUIRE(!recovered.restore(juce::ValueTree("OTHER_TYPE")));

    using aquorbium::app::PerformanceParameterState;
    PerformanceParameterState performance;
    REQUIRE(performance.setEcology(recovered.snapshot()));
    for (const auto& parameter : PerformanceParameterState::parameters)
        REQUIRE(performance.set(parameter.id, 0.37));
    performance.setAutonomous(false);
    REQUIRE(!performance.set("unknown", 0.5));
    REQUIRE(!performance.set("spaceReturn", std::numeric_limits<double>::infinity()));
    REQUIRE(performance.get("spaceReturn") == 0.37);
    auto performanceCopy = performance;
    REQUIRE(performanceCopy.set("spaceReturn", 0.11));
    REQUIRE(performance.get("spaceReturn") == 0.37);
    PerformanceParameterState performanceAssignment;
    performanceAssignment = performance;
    performanceAssignment.setAutonomous(true);
    REQUIRE(!performance.autonomous());
    EcologyParameterState isolatedEcology;
    REQUIRE(isolatedEcology.set(Key::density, 0.93));
    REQUIRE(performanceCopy.setEcology(isolatedEcology.snapshot()));
    REQUIRE(performance.ecologySnapshot().getProperty("density").toString().getDoubleValue() == 0.18);
    const auto fullXml = juce::parseXML(performance.snapshot().toXmlString());
    REQUIRE(fullXml != nullptr);
    PerformanceParameterState fullRecovered;
    REQUIRE(fullRecovered.restore(juce::ValueTree::fromXml(*fullXml)));
    REQUIRE(!fullRecovered.autonomous());
    REQUIRE(fullRecovered.ecologySnapshot().isEquivalentTo(recovered.snapshot()));
    for (const auto& parameter : PerformanceParameterState::parameters)
        REQUIRE(fullRecovered.get(parameter.id) == 0.37);

    auto legacyPerformance = performance.snapshot();
    for (std::size_t index = PerformanceParameterState::legacyParameterCount;
         index < PerformanceParameterState::parameters.size(); ++index)
        legacyPerformance.removeProperty(
            PerformanceParameterState::parameters[index].id, nullptr);
    PerformanceParameterState legacyRecovered;
    REQUIRE(legacyRecovered.restore(legacyPerformance));
    REQUIRE(legacyRecovered.get("spaceReturn") == 0.37);
    REQUIRE(legacyRecovered.get("environmentInfluence") == 0.0);
    REQUIRE(legacyRecovered.get("environmentFilterMix") == 0.5);
    REQUIRE(legacyRecovered.get("tailInfluence") == 0.0);
    REQUIRE(legacyRecovered.get("tailDecay") == 0.45);

    auto preTailPerformance = performance.snapshot();
    for (std::size_t index = PerformanceParameterState::spectralEnvironmentParameterCount;
         index < PerformanceParameterState::parameters.size(); ++index)
        preTailPerformance.removeProperty(
            PerformanceParameterState::parameters[index].id, nullptr);
    PerformanceParameterState preTailRecovered;
    REQUIRE(preTailRecovered.restore(preTailPerformance));
    REQUIRE(preTailRecovered.get("environmentInfluence") == 0.37);
    REQUIRE(preTailRecovered.get("tailInfluence") == 0.0);
    REQUIRE(preTailRecovered.get("tailDamping") == 0.55);

    auto preFormPerformance = performance.snapshot();
    for (std::size_t index = PerformanceParameterState::syntheticTailParameterCount;
         index < PerformanceParameterState::parameters.size(); ++index)
        preFormPerformance.removeProperty(
            PerformanceParameterState::parameters[index].id, nullptr);
    PerformanceParameterState preFormRecovered;
    REQUIRE(preFormRecovered.restore(preFormPerformance));
    REQUIRE(preFormRecovered.get("tailInfluence") == 0.37);
    REQUIRE(preFormRecovered.get("formInfluence") == 0.0);
    REQUIRE(preFormRecovered.get("formContext") == 0.5);
    REQUIRE(preFormRecovered.get("formInertia") == 0.35);
    REQUIRE(preFormRecovered.get("formMutation") == 0.05);

    auto preMasterPerformance = performance.snapshot();
    for (std::size_t index = PerformanceParameterState::formMemoryParameterCount;
         index < PerformanceParameterState::parameters.size(); ++index)
        preMasterPerformance.removeProperty(
            PerformanceParameterState::parameters[index].id, nullptr);
    PerformanceParameterState preMasterRecovered;
    REQUIRE(preMasterRecovered.restore(preMasterPerformance));
    REQUIRE(preMasterRecovered.get("formInfluence") == 0.37);
    REQUIRE(preMasterRecovered.get("masterTrim") == 0.75);
    REQUIRE(std::abs(preMasterRecovered.get("masterCeiling") - 5.0 / 5.9) < 1.0e-12);
    REQUIRE(std::abs(preMasterRecovered.get("masterRelease") - 100.0 / 450.0) < 1.0e-12);

    auto preStructurePerformance = performance.snapshot();
    for (std::size_t index = PerformanceParameterState::outputStageParameterCount;
         index < PerformanceParameterState::parameters.size(); ++index)
        preStructurePerformance.removeProperty(
            PerformanceParameterState::parameters[index].id, nullptr);
    PerformanceParameterState preStructureRecovered;
    REQUIRE(preStructureRecovered.restore(preStructurePerformance));
    REQUIRE(preStructureRecovered.get("masterTrim") == 0.37);
    REQUIRE(preStructureRecovered.get("structureInfluence") == 0.0);
    REQUIRE(preStructureRecovered.get("structureMemory") == 0.5);
    REQUIRE(preStructureRecovered.get("structureHarmony") == 0.5);
    REQUIRE(preStructureRecovered.get("structurePhase") == 0.5);

    auto preModuleBankPerformance = performance.snapshot();
    for (std::size_t index = PerformanceParameterState::structureMemoryParameterCount;
         index < PerformanceParameterState::parameters.size(); ++index)
        preModuleBankPerformance.removeProperty(
            PerformanceParameterState::parameters[index].id, nullptr);
    PerformanceParameterState preModuleBankRecovered;
    REQUIRE(preModuleBankRecovered.restore(preModuleBankPerformance));
    REQUIRE(preModuleBankRecovered.get("structureInfluence") == 0.37);
    for (std::size_t index = PerformanceParameterState::structureMemoryParameterCount;
         index < PerformanceParameterState::preEcologicalMusicParameterCount; ++index)
        REQUIRE(preModuleBankRecovered.get(
            PerformanceParameterState::parameters[index].id) == 0.0);
    REQUIRE(preModuleBankRecovered.get("musicInfluence") == 0.0);
    REQUIRE(preModuleBankRecovered.get("musicReaction") == 0.6);
    REQUIRE(preModuleBankRecovered.get("musicMemory") == 0.5);
    REQUIRE(preModuleBankRecovered.get("musicWater") == 0.5);

    auto preCreativeMasterPerformance = performance.snapshot();
    for (std::size_t index = PerformanceParameterState::preCreativeMasterParameterCount;
         index < PerformanceParameterState::parameters.size(); ++index)
        preCreativeMasterPerformance.removeProperty(
            PerformanceParameterState::parameters[index].id, nullptr);
    PerformanceParameterState preCreativeMasterRecovered;
    REQUIRE(preCreativeMasterRecovered.restore(preCreativeMasterPerformance));
    REQUIRE(preCreativeMasterRecovered.get("moduleIntegrityMemory") == 0.37);
    REQUIRE(preCreativeMasterRecovered.get("creativeMaster") == 0.0);

    // scenes saved before the ecological music (54 parameters) keep every
    // value and open with the music at zero: the previous route, untouched
    auto preMusicPerformance = performance.snapshot();
    for (std::size_t index = PerformanceParameterState::preEcologicalMusicParameterCount;
         index < PerformanceParameterState::parameters.size(); ++index)
        preMusicPerformance.removeProperty(PerformanceParameterState::parameters[index].id, nullptr);
    PerformanceParameterState preMusicRecovered;
    REQUIRE(preMusicRecovered.restore(preMusicPerformance));
    REQUIRE(preMusicRecovered.get("creativeMaster") == 0.37);
    REQUIRE(preMusicRecovered.get("musicInfluence") == 0.0);
    REQUIRE(preMusicRecovered.get("musicReaction") == 0.6);
    REQUIRE(preMusicRecovered.get("musicMemory") == 0.5);
    REQUIRE(preMusicRecovered.get("musicWater") == 0.5);

    // scenes saved before the bodies (58 parameters) open with native bodies
    auto preTimbrePerformance = performance.snapshot();
    for (std::size_t index = PerformanceParameterState::preTimbreParameterCount;
         index < PerformanceParameterState::parameters.size(); ++index)
        preTimbrePerformance.removeProperty(PerformanceParameterState::parameters[index].id, nullptr);
    PerformanceParameterState preTimbreRecovered;
    REQUIRE(preTimbreRecovered.restore(preTimbrePerformance));
    REQUIRE(preTimbreRecovered.get("musicWater") == 0.37);
    REQUIRE(preTimbreRecovered.get("timbreBody0") == 0.0);
    REQUIRE(preTimbreRecovered.get("timbreMix4") == 0.0);
    REQUIRE(preTimbreRecovered.get("timbreMatter2") == 0.5);
    REQUIRE(preTimbreRecovered.get("timbreEcology") == 0.0);

    // scenes saved with the seven-body encoding (index / 6) keep their bodies
    auto preKindPerformance = performance.snapshot();
    for (std::size_t index = PerformanceParameterState::preTimbreKindParameterCount;
         index < PerformanceParameterState::parameters.size(); ++index)
        preKindPerformance.removeProperty(PerformanceParameterState::parameters[index].id, nullptr);
    preKindPerformance.setProperty("timbreBody0", 1.0 / 6.0, nullptr);   // voice
    preKindPerformance.setProperty("timbreBody4", 1.0, nullptr);         // fold
    PerformanceParameterState preKindRecovered;
    REQUIRE(preKindRecovered.restore(preKindPerformance));
    REQUIRE(std::abs(preKindRecovered.get("timbreKind0") - 1.0 / 15.0) < 1.0e-9);
    REQUIRE(std::abs(preKindRecovered.get("timbreKind4") - 6.0 / 15.0) < 1.0e-9);

    // scenes saved before the sea/fresh switch (79 parameters) open in the
    // sea - and, being older than SUPPORT too, at 0 dB of support
    auto preSalinityPerformance = performance.snapshot();
    preSalinityPerformance.removeProperty("musicSalinity", nullptr);
    preSalinityPerformance.removeProperty("supportPresence", nullptr);
    preSalinityPerformance.removeProperty("masterEqOrder", nullptr);
    preSalinityPerformance.removeProperty("lensRadius", nullptr);
    preSalinityPerformance.removeProperty("lensReach", nullptr);
    preSalinityPerformance.removeProperty("lensMagnification", nullptr);
    preSalinityPerformance.removeProperty("lensBlur", nullptr);
    preSalinityPerformance.removeProperty("masterMultiband", nullptr);
    preSalinityPerformance.removeProperty("masterGlue", nullptr);
    preSalinityPerformance.removeProperty("masterTilt", nullptr);
    preSalinityPerformance.removeProperty("masterWidth", nullptr);
    preSalinityPerformance.removeProperty("brainClock", nullptr);
    preSalinityPerformance.removeProperty("brainCoordination", nullptr);
    preSalinityPerformance.removeProperty("roomSensitivity", nullptr);
    preSalinityPerformance.removeProperty("roomReaction", nullptr);
    PerformanceParameterState preSalinityRecovered;
    REQUIRE(preSalinityRecovered.restore(preSalinityPerformance));
    REQUIRE(preSalinityRecovered.get("musicSalinity") == 1.0);
    REQUIRE(preSalinityRecovered.get("supportPresence") == 0.2);

    // scenes saved before SUPPORT (80 parameters, 2026-10-23) open at 0 dB
    auto preSupportPerformance = performance.snapshot();
    preSupportPerformance.removeProperty("supportPresence", nullptr);
    preSupportPerformance.removeProperty("masterEqOrder", nullptr);
    preSupportPerformance.removeProperty("lensRadius", nullptr);
    preSupportPerformance.removeProperty("lensReach", nullptr);
    preSupportPerformance.removeProperty("lensMagnification", nullptr);
    preSupportPerformance.removeProperty("lensBlur", nullptr);
    preSupportPerformance.removeProperty("masterMultiband", nullptr);
    preSupportPerformance.removeProperty("masterGlue", nullptr);
    preSupportPerformance.removeProperty("masterTilt", nullptr);
    preSupportPerformance.removeProperty("masterWidth", nullptr);
    preSupportPerformance.removeProperty("brainClock", nullptr);
    preSupportPerformance.removeProperty("brainCoordination", nullptr);
    preSupportPerformance.removeProperty("roomSensitivity", nullptr);
    preSupportPerformance.removeProperty("roomReaction", nullptr);
    PerformanceParameterState preSupportRecovered;
    REQUIRE(preSupportRecovered.restore(preSupportPerformance));
    REQUIRE(preSupportRecovered.get("supportPresence") == 0.2);
    REQUIRE(preSupportRecovered.get("masterEqOrder") == 0.0);

    // scenes saved before the EQ order (81 parameters, 2026-10-23) keep the
    // order they were made with: EQ before the dynamics
    auto preEqOrderPerformance = performance.snapshot();
    preEqOrderPerformance.removeProperty("masterEqOrder", nullptr);
    preEqOrderPerformance.removeProperty("lensRadius", nullptr);
    preEqOrderPerformance.removeProperty("lensReach", nullptr);
    preEqOrderPerformance.removeProperty("lensMagnification", nullptr);
    preEqOrderPerformance.removeProperty("lensBlur", nullptr);
    preEqOrderPerformance.removeProperty("masterMultiband", nullptr);
    preEqOrderPerformance.removeProperty("masterGlue", nullptr);
    preEqOrderPerformance.removeProperty("masterTilt", nullptr);
    preEqOrderPerformance.removeProperty("masterWidth", nullptr);
    preEqOrderPerformance.removeProperty("brainClock", nullptr);
    preEqOrderPerformance.removeProperty("brainCoordination", nullptr);
    preEqOrderPerformance.removeProperty("roomSensitivity", nullptr);
    preEqOrderPerformance.removeProperty("roomReaction", nullptr);
    PerformanceParameterState preEqOrderRecovered;
    REQUIRE(preEqOrderRecovered.restore(preEqOrderPerformance));
    REQUIRE(preEqOrderRecovered.get("masterEqOrder") == 0.0);

    // scenes saved before the four cards (82 parameters, 2026-10-24) open at
    // the classic values
    auto preCardsPerformance = performance.snapshot();
    preCardsPerformance.removeProperty("lensRadius", nullptr);
    preCardsPerformance.removeProperty("lensReach", nullptr);
    preCardsPerformance.removeProperty("lensMagnification", nullptr);
    preCardsPerformance.removeProperty("lensBlur", nullptr);
    preCardsPerformance.removeProperty("masterMultiband", nullptr);
    preCardsPerformance.removeProperty("masterGlue", nullptr);
    preCardsPerformance.removeProperty("masterTilt", nullptr);
    preCardsPerformance.removeProperty("masterWidth", nullptr);
    preCardsPerformance.removeProperty("brainClock", nullptr);
    preCardsPerformance.removeProperty("brainCoordination", nullptr);
    preCardsPerformance.removeProperty("roomSensitivity", nullptr);
    preCardsPerformance.removeProperty("roomReaction", nullptr);
    PerformanceParameterState preCardsRecovered;
    REQUIRE(preCardsRecovered.restore(preCardsPerformance));
    REQUIRE(preCardsRecovered.get("lensRadius") == 0.5 && preCardsRecovered.get("brainCoordination") == 0.0);

    auto fullInvalid = performance.snapshot();
    fullInvalid.setProperty("spaceReturn", "0.2trailing", nullptr);
    REQUIRE(!fullRecovered.restore(fullInvalid));
    REQUIRE(fullRecovered.get("spaceReturn") == 0.37);
    fullInvalid = performance.snapshot();
    fullInvalid.removeProperty("strikerMatter", nullptr);
    REQUIRE(!fullRecovered.restore(fullInvalid));
    fullInvalid = performance.snapshot();
    fullInvalid.setProperty("autonomous", 0.5, nullptr);
    REQUIRE(!fullRecovered.restore(fullInvalid));
    REQUIRE(!fullRecovered.autonomous());
    fullInvalid = performance.snapshot();
    fullInvalid.getChild(0).setProperty("density", "not-a-number", nullptr);
    REQUIRE(!fullRecovered.restore(fullInvalid));
    REQUIRE(fullRecovered.get("spaceReturn") == 0.37);
    auto copied = performance.snapshot();
    copied.setProperty("spaceReturn", 0.92, nullptr);
    REQUIRE(performance.get("spaceReturn") == 0.37);

    using aquorbium::app::PerformancePresetBank;
    PerformancePresetBank bank;
    REQUIRE(!bank.occupied(0));
    REQUIRE(!bank.recall(0, fullRecovered));
    REQUIRE(!bank.store(4, performance));
    for (std::size_t slot = 0; slot < PerformancePresetBank::capacity; ++slot)
    {
        REQUIRE(performance.set("spaceReturn", static_cast<double>(slot) / 4.0));
        REQUIRE(bank.store(slot, performance));
    }
    REQUIRE(performance.set("spaceReturn", 0.95));
    PerformancePresetBank recoveredBank;
    REQUIRE(recoveredBank.restoreXml(bank.snapshot().toXmlString()));
    for (std::size_t slot = 0; slot < PerformancePresetBank::capacity; ++slot)
    {
        REQUIRE(recoveredBank.occupied(slot));
        REQUIRE(recoveredBank.recall(slot, fullRecovered));
        REQUIRE(fullRecovered.get("spaceReturn") == static_cast<double>(slot) / 4.0);
        REQUIRE(!fullRecovered.autonomous());
    }
    auto invalidBank = bank.snapshot();
    invalidBank.getChild(3).getChild(0).removeProperty("strikerMatter", nullptr);
    REQUIRE(!recoveredBank.restore(invalidBank));
    REQUIRE(recoveredBank.recall(3, fullRecovered));
    REQUIRE(fullRecovered.get("spaceReturn") == 0.75);
    REQUIRE(!recoveredBank.restoreXml("<broken"));
    auto unknownBank = bank.snapshot();
    unknownBank.getChild(0).setProperty("unknown", 0.5, nullptr);
    REQUIRE(!recoveredBank.restore(unknownBank));
    REQUIRE(!recoveredBank.restoreXml("<!DOCTYPE a>" + bank.snapshot().toXmlString()));
    REQUIRE(!recovered.restoreXml("<!DOCTYPE a>" + recovered.snapshot().toXmlString()));
    REQUIRE(!recoveredBank.restoreXml(juce::String::repeatedString("x", 65537)));
    auto emptyBank = PerformancePresetBank {};
    REQUIRE(emptyBank.restoreXml(emptyBank.snapshot().toXmlString()));
    REQUIRE(!emptyBank.occupied(0));
    REQUIRE(!emptyBank.recall(0, fullRecovered));
    REQUIRE(fullRecovered.get("spaceReturn") == 0.75);

    // Migration uses an in-memory PropertySet, without a GUI or settings file.
    juce::PropertySet preferences;
    preferences.setValue("parameter.density", 0.61);
    preferences.setValue("parameter.drift", 0.30);
    preferences.setValue("parameter.spaceReturn", 0.22);
    preferences.setValue("parameter.autonomous", false);
    PerformanceParameterState migrated;
    migrated.loadPreferences(preferences);
    EcologyParameterState migratedEcology;
    REQUIRE(migratedEcology.restore(migrated.ecologySnapshot()));
    REQUIRE(migratedEcology.get(Key::density) == 0.61);
    REQUIRE(std::abs(migratedEcology.get(Key::calm) - 0.70) < 1.0e-12);
    REQUIRE(migrated.get("spaceReturn") == 0.22);
    REQUIRE(!migrated.autonomous());

    preferences.setValue("parameter.ecologyStateV1", recovered.snapshot().toXmlString());
    migrated.loadPreferences(preferences);
    REQUIRE(migratedEcology.restore(migrated.ecologySnapshot()));
    REQUIRE(migratedEcology.get(Key::density) == 0.18);
    REQUIRE(migrated.get("spaceReturn") == 0.22);
    preferences.setValue("parameter.performanceStateV1", fullRecovered.snapshot().toXmlString());
    migrated.loadPreferences(preferences);
    REQUIRE(migrated.get("spaceReturn") == 0.75);

    preferences.setValue("parameter.performanceStateV1", "<broken");
    preferences.setValue("parameter.spaceReturn", "0.2trailing");
    migrated.loadPreferences(preferences);
    REQUIRE(migrated.get("spaceReturn") == 0.48);
    REQUIRE(!migrated.restoreXml(juce::String::repeatedString("x", 16385)));
    REQUIRE(!migratedEcology.restoreXml(juce::String::repeatedString("x", 4097)));
    REQUIRE(runSceneFileTests());

    // upper case with accents (2026-10-22): the tutorial's chapter titles
    // came out "A áGUA" through juce::String::toUpperCase
    {
        using aquorbium::app::upper;
        const auto u8 = [](const char* text) { return juce::String::fromUTF8(text); };
        REQUIRE(upper(u8("a água")) == u8("A ÁGUA"));
        REQUIRE(upper(u8("o dispositivo de síntese")) == u8("O DISPOSITIVO DE SÍNTESE"));
        REQUIRE(upper(u8("cœur, été, ça, niño, über, ÿ")) == u8("CŒUR, ÉTÉ, ÇA, NIÑO, ÜBER, Ÿ"));
        REQUIRE(upper(u8("÷ 1/2 ß")) == u8("÷ 1/2 ß"));  // no case: unchanged
        // every chapter title, in every language, has no lower-case letter left
        for (const auto& chapter : aquorbium::app::tutorial::chapters)
            for (const auto language : { aquorbium::app::Lang::en, aquorbium::app::Lang::pt,
                                         aquorbium::app::Lang::fr, aquorbium::app::Lang::es })
            {
                const auto title = upper(aquorbium::app::tr(chapter.title, language));
                for (auto p = title.getCharPointer(); !p.isEmpty();)
                {
                    const auto c = p.getAndAdvance();
                    REQUIRE(aquorbium::app::upperLetter(c) == c);
                }
            }
    }
    return EXIT_SUCCESS;
}
