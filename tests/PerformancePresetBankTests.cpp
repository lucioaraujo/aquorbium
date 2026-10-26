// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "app/PerformancePresetBank.h"
#include "app/PerformanceSceneControls.h"

#include <iostream>

#define CHECK(condition) do { if (!(condition)) { \
    std::cerr << "preset bank requirement failed at line " << __LINE__ << ": " #condition "\n"; \
    return false; } } while (false)

bool runPresetBankTests()
{
    using namespace aquorbium::app;
    PerformancePresetBank bank;
    PerformanceParameterState first, second, recalled;
    CHECK(first.set("spaceReturn", 0.23));
    CHECK(second.set("spaceReturn", 0.71));
    const auto name = juce::String::fromUTF8("Été & bois <'\"> aquatique");
    CHECK(!bank.canUndo() && bank.undoSlot() == PerformancePresetBank::capacity);
    CHECK(!bank.undo() && !bank.rename(0, name));
    CHECK(bank.store(0, first, name));
    CHECK(bank.name(0) == name && bank.canUndo() && bank.undoSlot() == 0);
    CHECK(bank.undo());
    CHECK(!bank.occupied(0) && bank.name(0).isEmpty() && !bank.canUndo());
    CHECK(bank.store(0, first, name));
    CHECK(bank.store(0, second)); // Import/store preserves the local name.
    CHECK(bank.recall(0, recalled) && recalled.get("spaceReturn") == 0.71);
    CHECK(bank.name(0) == name);
    CHECK(bank.undo());
    CHECK(bank.recall(0, recalled) && recalled.get("spaceReturn") == 0.23);
    CHECK(bank.name(0) == name && !bank.undo());
    CHECK(bank.rename(0, "  Calme  "));
    CHECK(bank.name(0) == "Calme");
    CHECK(bank.rename(0, "Calme")); // No-op must retain the previous undo.
    CHECK(bank.store(0, first));
    CHECK(bank.undo() && bank.name(0) == name);
    CHECK(bank.rename(0, ""));
    CHECK(bank.name(0).isEmpty() && bank.undo() && bank.name(0) == name);

    const auto thirtyTwo = juce::String::repeatedString(juce::String::fromUTF8("é"), 32);
    CHECK(bank.rename(0, thirtyTwo));
    const auto unchanged = bank.snapshot();
    CHECK(!bank.rename(0, thirtyTwo + "x"));
    CHECK(!bank.rename(0, "a\nb") && !bank.rename(0, "a\tb"));
    CHECK(!bank.rename(0, juce::String::fromUTF8("a\xE2\x80\xA8" "b")));
    CHECK(!bank.rename(4, "No") && !bank.store(4, second, "No"));
    CHECK(!bank.store(0, second, "a\nb"));
    CHECK(bank.snapshot().isEquivalentTo(unchanged) && bank.canUndo());
    CHECK(bank.undo() && bank.name(0) == name);

    CHECK(bank.store(3, second, "Derniere"));
    CHECK(bank.undoSlot() == 3 && bank.undo());
    CHECK(!bank.occupied(3) && bank.name(0) == name);
    CHECK(bank.snapshot().hasType("AQUORBIUM_PRESETS_V3"));
    CHECK(bank.namedSnapshot().hasType("AQUORBIUM_PRESETS_V2"));
    CHECK(bank.legacySnapshot().hasType("AQUORBIUM_PRESETS_V1"));
    CHECK(bank.legacySnapshot().getChild(0).getNumProperties() == 0);

    PerformancePresetBank roundtrip;
    CHECK(roundtrip.restoreXml(bank.snapshot().toXmlString()));
    CHECK(roundtrip.name(0) == name && !roundtrip.canUndo());
    CHECK(roundtrip.recall(0, recalled) && recalled.get("spaceReturn") == 0.23);
    auto external = bank.snapshot();
    external.getChild(0).setProperty("name", "Modified", nullptr);
    external.getChild(0).getChild(0).setProperty("spaceReturn", 0.99, nullptr);
    CHECK(bank.name(0) == name);
    CHECK(bank.recall(0, recalled) && recalled.get("spaceReturn") == 0.23);

    CHECK(roundtrip.rename(0, "Preserve undo"));
    const auto protectedBank = roundtrip.snapshot();
    auto invalid = bank.snapshot();
    invalid.getChild(0).setProperty("name", 7, nullptr);
    CHECK(!roundtrip.restore(invalid));
    invalid = bank.snapshot();
    invalid.getChild(0).setProperty("name", " padded ", nullptr);
    CHECK(!roundtrip.restore(invalid));
    invalid = bank.snapshot();
    invalid.getChild(1).setProperty("name", "Empty slot", nullptr);
    CHECK(!roundtrip.restore(invalid));
    invalid = bank.snapshot();
    invalid.getChild(0).setProperty("unexpected", "value", nullptr);
    CHECK(!roundtrip.restore(invalid));
    CHECK(!roundtrip.restoreXml(bank.snapshot().toXmlString().replace("&amp;", "&#10;")));
    CHECK(!roundtrip.restoreXml(bank.snapshot().toXmlString().replace("&amp;", "&#0;")));
    CHECK(!roundtrip.restoreXml(bank.snapshot().toXmlString().replace("&amp;", "&#55296;")));
    CHECK(!roundtrip.restoreXml(bank.snapshot().toXmlString().replace("&amp;", "&#1114112;")));
    CHECK(!roundtrip.restoreXml(bank.snapshot().toXmlString().replace("&amp;", "&#0000000000000038;")));
    CHECK(!roundtrip.restoreXml(bank.snapshot().toXmlString().replace("&amp;", "&unknown;")));
    CHECK(!roundtrip.restoreXml("<!DOCTYPE test>" + bank.snapshot().toXmlString()));
    CHECK(roundtrip.snapshot().isEquivalentTo(protectedBank) && roundtrip.canUndo());
    CHECK(roundtrip.undo() && roundtrip.name(0) == name);
    const auto emoji = juce::String::fromUTF8("Aquário \xF0\x9F\x8C\x8A");
    CHECK(roundtrip.rename(0, emoji));
    PerformancePresetBank unicode;
    CHECK(unicode.restoreXml(roundtrip.snapshot().toXmlString()) && unicode.name(0) == emoji);

    PerformancePresetBank legacy;
    CHECK(legacy.restore(bank.legacySnapshot()));
    CHECK(legacy.name(0).isEmpty() && !legacy.canUndo());
    CHECK(legacy.recall(0, recalled) && recalled.get("spaceReturn") == 0.23);
    juce::PropertySet preferences;
    preferences.setValue("parameter.presetBankV1", bank.legacySnapshot().toXmlString());
    CHECK(legacy.loadPreferences(preferences));
    bank.savePreferences(preferences);
    CHECK(legacy.loadPreferences(preferences) && legacy.name(0) == name);
    CHECK(legacy.rename(0, "Mutation"));
    const auto beforeInvalidLoad = legacy.snapshot();
    preferences.setValue("parameter.presetBankV3", "<broken");
    CHECK(!legacy.loadPreferences(preferences));
    CHECK(preferences.getValue("parameter.presetBankV3") == "<broken");
    CHECK(legacy.snapshot().isEquivalentTo(beforeInvalidLoad) && legacy.canUndo());
    CHECK(legacy.undo() && legacy.name(0) == name);
    const juce::PropertySet emptyPreferences;
    CHECK(legacy.loadPreferences(emptyPreferences));
    CHECK(!legacy.occupied(0) && legacy.name(0).isEmpty() && !legacy.canUndo());

    // Naming and undo never change the captured parameters or transients.
    AudioControlFrame transients;
    transients.lensHeld = true;
    transients.lensX = 0.25f;
    const auto parametersBefore = recalled.snapshot();
    CHECK(bank.rename(0, "Music unchanged"));
    CHECK(bank.recall(0, recalled));
    CHECK(recalled.snapshot().isEquivalentTo(parametersBefore));
    const auto mapped = sceneControls(recalled, transients);
    CHECK(mapped.spaceReturn == static_cast<float>(0.23) && mapped.lensHeld && mapped.lensX == 0.25f);
    CHECK(bank.undo() && bank.name(0) == name);
    // V3 (2026-10-22): the synthesis travels with the scene; V2 and V1 leave
    // it out, so builds from before the device still read the bank
    {
        PerformancePresetBank synthBank;
        PerformanceParameterState withSynth, back;
        std::array<juce::String, PerformanceParameterState::synthPatchCount> patches {
            "sourceA=4;cutoff=900;", "sourceA=8;", "", "level=1.2;r0S=6;r0D=2;r0A=-0.25;", "sourceB=10;bodyB=11;" };
        CHECK(withSynth.set("spaceReturn", 0.4));
        CHECK(withSynth.setSynth(0.65, patches));
        CHECK(synthBank.store(1, withSynth, "Synth"));
        PerformancePresetBank v3;
        CHECK(v3.restoreXml(synthBank.snapshot().toXmlString()));
        CHECK(v3.recall(1, back) && back.hasSynth() && back.synthMix() == 0.65);
        for (std::size_t i = 0; i < patches.size(); ++i)
            CHECK(back.synthPatch(i) == patches[i]);
        // the named V2 and the V1 carry no synthesis, and restore cleanly
        CHECK(!synthBank.namedSnapshot().getChild(1).getChild(0).getChildWithName("SYNTH").isValid());
        CHECK(!synthBank.legacySnapshot().getChild(1).getChild(0).getChildWithName("SYNTH").isValid());
        PerformancePresetBank v2;
        CHECK(v2.restore(synthBank.namedSnapshot()) && v2.recall(1, back) && !back.hasSynth()
              && back.get("spaceReturn") == 0.4);
        // the preferences: V3 first
        juce::PropertySet synthPreferences;
        synthBank.savePreferences(synthPreferences);
        PerformancePresetBank loaded;
        CHECK(loaded.loadPreferences(synthPreferences) && loaded.recall(1, back) && back.hasSynth());
        // the bank's XML limits hold four full scenes with five long patches
        PerformanceParameterState full;
        std::array<juce::String, PerformanceParameterState::synthPatchCount> longPatches;
        for (auto& text : longPatches)
            text = juce::String::repeatedString("cutoff=1234.56;", 60);  // 900 characters
        CHECK(full.setSynth(1.0, longPatches));
        PerformancePresetBank crowded;
        for (std::size_t slot = 0; slot < PerformancePresetBank::capacity; ++slot)
            CHECK(crowded.store(slot, full, "Cheia " + juce::String(static_cast<int>(slot))));
        PerformancePresetBank crowdedBack;
        CHECK(crowdedBack.restoreXml(crowded.snapshot().toXmlString()));
        CHECK(crowdedBack.recall(3, back) && back.synthPatch(4) == longPatches[4]);
        // invalid synthesis: rejected, nothing replaced
        CHECK(!full.setSynth(1.5, patches));
        auto bad = patches;
        bad[2] = "cutoff=<script>;";
        CHECK(!full.setSynth(0.5, bad));
        bad[2] = juce::String::repeatedString("x", PerformanceParameterState::maximumSynthPatchCharacters + 1);
        CHECK(!full.setSynth(0.5, bad));
        auto tampered = synthBank.snapshot();
        tampered.getChild(1).getChild(0).getChildWithName("SYNTH").setProperty("extra", 1, nullptr);
        CHECK(!loaded.restore(tampered));
        tampered = synthBank.snapshot();
        tampered.getChild(1).getChild(0).getChildWithName("SYNTH").setProperty("mix", "abc", nullptr);
        CHECK(!loaded.restore(tampered));
        // the ecology can change without losing the synthesis
        CHECK(withSynth.setEcology(withSynth.ecologySnapshot()) && withSynth.hasSynth());
    }
    std::cout << "Preset bank V3: synthesis patches, V2/V1 without them, limits and strict validation passed\n";
    std::cout << "Preset bank V2: names, one-step undo, V1 migration, strict XML and parameter isolation passed\n";
    return true;
}
