// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "app/PerformanceSceneFile.h"

#include <iostream>

#define CHECK(condition) do { if (!(condition)) { \
    std::cerr << "scene file requirement failed at line " << __LINE__ << ": " #condition "\n"; \
    return false; } } while (false)

namespace
{
struct ScratchDirectory
{
    juce::File directory = juce::File::getSpecialLocation(juce::File::tempDirectory)
        .getNonexistentChildFile("aquorbium-scene-tests", "", false);
    ~ScratchDirectory()
    {
        // Only task-owned regular files; never recursively delete a broad path.
        for (const auto& file : directory.findChildFiles(juce::File::findFiles, false))
            static_cast<void>(file.deleteFile());
        static_cast<void>(directory.getChildFile("directory.aquscene").deleteFile());
        static_cast<void>(directory.deleteFile());
    }
};
}

bool runSceneFileTests()
{
    using aquorbium::app::PerformanceParameterState;
    using aquorbium::app::PerformanceSceneFile;
    ScratchDirectory scratch;
    CHECK(scratch.directory.createDirectory().wasOk());
    const auto target = scratch.directory.getChildFile("scene.aquscene");
    PerformanceParameterState source, recovered;
    CHECK(source.set("spaceReturn", 0.19));
    source.setAutonomous(false);
    const auto first = PerformanceSceneFile::write(target, source);
    CHECK(first.result.wasOk());
    CHECK(first.backup == juce::File {});
    CHECK(PerformanceSceneFile::read(target, recovered).wasOk());
    CHECK(recovered.get("spaceReturn") == 0.19 && !recovered.autonomous());
    const auto original = target.loadFileAsString();
    CHECK(source.set("spaceReturn", 0.81));
    CHECK(PerformanceSceneFile::write(target, source).result.failed());
    CHECK(target.loadFileAsString() == original);
    const auto replacement = PerformanceSceneFile::write(target, source, true);
    CHECK(replacement.result.wasOk());
    CHECK(replacement.backup.existsAsFile());
    CHECK(replacement.backup.loadFileAsString() == original);
    CHECK(PerformanceSceneFile::read(target, recovered).wasOk());
    CHECK(recovered.get("spaceReturn") == 0.81);
    const auto second = PerformanceSceneFile::write(target, source, true);
    CHECK(second.result.wasOk() && second.backup != replacement.backup);
    CHECK(replacement.backup.loadFileAsString() == original);

    const auto invalid = scratch.directory.getChildFile("invalid.aquscene");
    const auto preserved = recovered.snapshot().toXmlString();
    CHECK(invalid.replaceWithText("<broken"));
    CHECK(PerformanceSceneFile::read(invalid, recovered).failed());
    CHECK(recovered.snapshot().toXmlString() == preserved);
    CHECK(invalid.replaceWithText(juce::String::repeatedString("x", PerformanceSceneFile::maximumBytes + 1)));
    CHECK(PerformanceSceneFile::read(invalid, recovered).failed());
    CHECK(PerformanceSceneFile::write(invalid, source, true).result.failed());
    CHECK(invalid.getSize() == PerformanceSceneFile::maximumBytes + 1);
    CHECK(invalid.replaceWithText(""));
    CHECK(PerformanceSceneFile::read(invalid, recovered).failed());
    const char invalidUtf8[] = { static_cast<char>(0xff), 'x' };
    CHECK(invalid.replaceWithData(invalidUtf8, sizeof(invalidUtf8)));
    CHECK(PerformanceSceneFile::read(invalid, recovered).failed());
    const char embeddedNull[] = { '<', 0, '>' };
    CHECK(invalid.replaceWithData(embeddedNull, sizeof(embeddedNull)));
    CHECK(PerformanceSceneFile::read(invalid, recovered).failed());
    CHECK(PerformanceSceneFile::read(scratch.directory.getChildFile("missing.aquscene"), recovered).failed());
    const auto directoryTarget = scratch.directory.getChildFile("directory.aquscene");
    CHECK(directoryTarget.createDirectory().wasOk());
    CHECK(PerformanceSceneFile::write(directoryTarget, source, true).result.failed());
    CHECK(PerformanceSceneFile::write(scratch.directory.getChildFile("wrong.xml"), source).result.failed());
    CHECK(PerformanceSceneFile::write(scratch.directory.getChildFile("missing/scene.aquscene"), source).result.failed());

    auto unknown = source.snapshot();
    unknown.setProperty("unknown", 0.5, nullptr);
    CHECK(PerformanceSceneFile::decode(unknown.toXmlString(), recovered).failed());
    unknown = source.snapshot();
    unknown.getChild(0).setProperty("unknown", 0.5, nullptr);
    CHECK(PerformanceSceneFile::decode(unknown.toXmlString(), recovered).failed());
    auto outOfRange = source.snapshot();
    outOfRange.setProperty("spaceReturn", 2.0, nullptr);
    CHECK(PerformanceSceneFile::decode(outOfRange.toXmlString(), recovered).failed());
    outOfRange = source.snapshot();
    outOfRange.getChild(0).setProperty("density", -0.1, nullptr);
    CHECK(PerformanceSceneFile::decode(outOfRange.toXmlString(), recovered).failed());
    const auto validXml = source.snapshot().toXmlString();
    CHECK(PerformanceSceneFile::decode("<!DOCTYPE a [<!ENTITY x SYSTEM 'file:///etc/passwd'>]>" + validXml, recovered).failed());
    CHECK(PerformanceSceneFile::decode(validXml.replace("0.81", "&x;"), recovered).failed());
    CHECK(PerformanceSceneFile::decode("<a><b><c><d><e><f>" + validXml, recovered).failed());
    CHECK(PerformanceSceneFile::decode(validXml + "trailing text", recovered).failed());
    CHECK(PerformanceSceneFile::decode(juce::String::repeatedString("x", 16385), recovered).failed());
    CHECK(recovered.snapshot().toXmlString() == preserved);
    CHECK(PerformanceSceneFile::decode(validXml, recovered).wasOk());
    CHECK(invalid.replaceWithText(validXml + juce::String::repeatedString(" ",
        PerformanceSceneFile::maximumBytes - static_cast<int>(validXml.getNumBytesAsUTF8()))));
    CHECK(invalid.getSize() == PerformanceSceneFile::maximumBytes);
    CHECK(PerformanceSceneFile::read(invalid, recovered).wasOk());
#if JUCE_LINUX || JUCE_MAC
    const auto link = scratch.directory.getChildFile("link.aquscene");
    CHECK(target.createSymbolicLink(link, false));
    CHECK(PerformanceSceneFile::read(link, recovered).failed());
    CHECK(PerformanceSceneFile::write(link, source, true).result.failed());
    CHECK(link.deleteFile());
#endif
    // Staging temporaries have been removed; only scenes and deliberate backups remain.
    CHECK(scratch.directory.findChildFiles(juce::File::findFiles, false).size() == 4);

    // the synthesis device's patches travel in the file (2026-10-22): five
    // full-length patches fit the 16 KiB and the XML tag limit
    {
        PerformanceParameterState withSynth, back;
        std::array<juce::String, PerformanceParameterState::synthPatchCount> patches;
        for (auto& text : patches)
            text = juce::String::repeatedString("cutoff=1234.56;", 60);
        patches[0] = "sourceA=10;bodyA=11;fm=0.25;r0S=6;r0D=2;r0A=-0.5;";
        CHECK(withSynth.setSynth(0.7, patches));
        const auto synthFile = scratch.directory.getChildFile("synth.aquscene");
        CHECK(PerformanceSceneFile::write(synthFile, withSynth).result.wasOk());
        CHECK(synthFile.getSize() < PerformanceSceneFile::maximumBytes);
        CHECK(PerformanceSceneFile::read(synthFile, back).wasOk());
        CHECK(back.hasSynth() && back.synthMix() == 0.7 && back.synthPatch(0) == patches[0]
              && back.synthPatch(4) == patches[4]);
        // a scene from before the device still reads, without synthesis
        PerformanceParameterState plain;
        CHECK(PerformanceSceneFile::read(target, plain).wasOk() && !plain.hasSynth());
        CHECK(synthFile.deleteFile());
    }
    return true;
}
