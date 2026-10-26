// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include "EcologyParameterState.h"

namespace aquorbium::app
{
// Persistence boundary, used only by the GUI. Never passed to the audio thread.
class PerformanceParameterState
{
public:
    static constexpr int maximumXmlCharacters = 16384;
    struct Parameter
    {
        const char* id;
        double initial;
    };

    static constexpr std::array<Parameter, 94> parameters {{
        { "ecosystemMorph", 1.0 },
        { "geometryInfluence", 0.0 },
        { "rhythmDensity", 0.5 },
        { "orbitRotation", 0.5 },
        { "intervalBias", 0.5 },
        { "geometryMutation", 0.0 },
        { "strikerInfluence", 0.0 },
        { "strikerMatter", 0.5 },
        { "strikerPopulation", 0.5 },
        { "strikerScatter", 0.5 },
        { "spaceReturn", 0.48 },
        { "spaceGesture", 0.0 },
        { "spaceSize", 0.5 },
        { "spaceSpread", 1.0 },
        { "spaceTone", 0.5 },
        { "spaceMemory", 0.35 },
        { "environmentInfluence", 0.0 },
        { "environmentFilterMix", 0.5 },
        { "environmentCentre", 0.5 },
        { "environmentSpan", 0.5 },
        { "environmentResonance", 0.45 },
        { "environmentAmbience", 0.0 },
        { "tailInfluence", 0.0 },
        { "tailDecay", 0.45 },
        { "tailDamping", 0.55 },
        { "tailNightmare", 0.0 },
        { "formInfluence", 0.0 },
        { "formContext", 0.5 },
        { "formInertia", 0.35 },
        { "formMutation", 0.05 },
        { "masterTrim", 0.75 },
        { "masterCeiling", 5.0 / 5.9 },
        { "masterRelease", 100.0 / 450.0 },
        { "structureInfluence", 0.0 },
        { "structureMemory", 0.5 },
        { "structureHarmony", 0.5 },
        { "structurePhase", 0.5 },
        { "moduleBankMix", 0.0 },
        { "moduleSynthBodies", 0.0 },
        { "moduleGranularMemory", 0.0 },
        { "modulePercussionPulses", 0.0 },
        { "moduleFractalVector", 0.0 },
        { "moduleLorenzDrift", 0.0 },
        { "moduleFluidPhase", 0.0 },
        { "moduleGestureWaves", 0.0 },
        { "moduleClockEvents", 0.0 },
        { "moduleTransposeSelect", 0.0 },
        { "moduleBranchPermute", 0.0 },
        { "moduleRotateLayout", 0.0 },
        { "modulePatternLoop", 0.0 },
        { "moduleShiftProbability", 0.0 },
        { "moduleEvolutionStages", 0.0 },
        { "moduleIntegrityMemory", 0.0 },
        // Appended to preserve every previous parameter index and scene.
        { "creativeMaster", 0.0 },
        // The ecological music (2026-10-20), appended for the same reason.
        { "musicInfluence", 0.0 },
        { "musicReaction", 0.6 },
        { "musicMemory", 0.5 },
        { "musicWater", 0.5 },
        // The organisms' bodies (2026-10-20): body (index / 6), MATTER and MIX
        // per organism, then ECOLOGY; appended, as before.
        { "timbreBody0", 0.0 },
        { "timbreBody1", 0.0 },
        { "timbreBody2", 0.0 },
        { "timbreBody3", 0.0 },
        { "timbreBody4", 0.0 },
        { "timbreMatter0", 0.5 },
        { "timbreMatter1", 0.5 },
        { "timbreMatter2", 0.5 },
        { "timbreMatter3", 0.5 },
        { "timbreMatter4", 0.5 },
        { "timbreMix0", 0.0 },
        { "timbreMix1", 0.0 },
        { "timbreMix2", 0.0 },
        { "timbreMix3", 0.0 },
        { "timbreMix4", 0.0 },
        { "timbreEcology", 0.0 },
        // 2026-10-20: the body choice, encoded index / 15 (room for 16 bodies).
        // timbreBody* (index / 6, seven bodies) remain, read only to migrate.
        { "timbreKind0", 0.0 },
        { "timbreKind1", 0.0 },
        { "timbreKind2", 0.0 },
        { "timbreKind3", 0.0 },
        { "timbreKind4", 0.0 },
        // 2026-10-20: the water's salinity, 1 = sea (35 ppt), 0 = fresh water
        { "musicSalinity", 1.0 },
        // 2026-10-23: SUPPORT, the support species' level, -6..+24 dB; 0.2 =
        // 0 dB (the balance until then)
        { "supportPresence", 0.2 },
        // 2026-10-23: the creative MASTER's EQ order, 0 = EQ before the
        // dynamics (the order until then), 1 = after
        { "masterEqOrder", 0.0 },
        // 2026-10-24: the LUPA, MASTER CRIATIVO, CÉREBRO and SALA cards; 0.5
        // is the classic value of each (brainCoordination: 0)
        { "lensRadius", 0.5 },
        { "lensReach", 0.5 },
        { "lensMagnification", 0.5 },
        { "lensBlur", 0.5 },
        { "masterMultiband", 0.5 },
        { "masterGlue", 0.5 },
        { "masterTilt", 0.5 },
        { "masterWidth", 0.5 },
        { "brainClock", 0.5 },
        { "brainCoordination", 0.0 },
        { "roomSensitivity", 0.5 },
        { "roomReaction", 0.5 }
    }};
    static constexpr std::size_t legacyParameterCount = 16;
    static constexpr std::size_t spectralEnvironmentParameterCount = 22;
    static constexpr std::size_t syntheticTailParameterCount = 26;
    static constexpr std::size_t formMemoryParameterCount = 30;
    static constexpr std::size_t outputStageParameterCount = 33;
    static constexpr std::size_t structureMemoryParameterCount = 37;
    static constexpr std::size_t preCreativeMasterParameterCount = 53;
    static constexpr std::size_t preEcologicalMusicParameterCount = 54;
    static constexpr std::size_t preTimbreParameterCount = 58;
    static constexpr std::size_t preTimbreKindParameterCount = 74;
    static constexpr std::size_t preSalinityParameterCount = 79;
    static constexpr std::size_t preSupportParameterCount = 80;
    static constexpr std::size_t preEqOrderParameterCount = 81;
    static constexpr std::size_t preCardsParameterCount = 82;

    PerformanceParameterState()
    {
        for (const auto& parameter : parameters)
            state.setProperty(parameter.id, parameter.initial, nullptr);
        state.setProperty("autonomous", 1, nullptr);
        state.appendChild(ecology.snapshot(), nullptr);
    }

    PerformanceParameterState(const PerformanceParameterState& other)
        : ecology(other.ecology), state(other.snapshot()) {}
    PerformanceParameterState& operator=(const PerformanceParameterState& other)
    {
        if (this != &other)
        {
            ecology = other.ecology;
            state = other.snapshot();
        }
        return *this;
    }

    [[nodiscard]] bool set(const char* id, double value)
    {
        if (!isKnown(id) || !std::isfinite(value))
            return false;
        state.setProperty(id, juce::jlimit(0.0, 1.0, value), nullptr);
        return true;
    }

    [[nodiscard]] double get(const char* id) const
    {
        return static_cast<double>(state.getProperty(id));
    }

    void setAutonomous(bool enabled) { state.setProperty("autonomous", enabled ? 1 : 0, nullptr); }
    [[nodiscard]] bool autonomous() const { return get("autonomous") > 0.5; }

    [[nodiscard]] bool setEcology(const juce::ValueTree& candidate)
    {
        if (!ecology.restore(candidate))
            return false;
        // the ecology is always the first child; the synthesis (if any) stays
        const auto synth = synthNode();
        state.removeAllChildren(nullptr);
        state.appendChild(ecology.snapshot(), nullptr);
        if (synth.isValid())
            state.appendChild(synth, nullptr);
        return true;
    }

    // ---- the synthesis device (2026-10-22): an optional second child --------
    // SYNTH { mix, patch0 .. patch4 }: the SYNTHESIS mix and the five patches
    // as SynthPatchCodec text ("key=value;"), decoded by the owner. A scene
    // without it predates the device: recalled, it sounds as it was saved
    // (SYNTHESIS 0) and the patches being edited are kept.
    static constexpr std::size_t synthPatchCount = 5;
    static constexpr int maximumSynthPatchCharacters = 2048;

    [[nodiscard]] static bool validSynthPatch(const juce::String& text)
    {
        if (text.length() > maximumSynthPatchCharacters)
            return false;
        for (const auto c : text)
            if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')
                  || c == '.' || c == '=' || c == ';' || c == '-' || c == '+'))
                return false;
        return true;
    }

    [[nodiscard]] bool setSynth(double mix, const std::array<juce::String, synthPatchCount>& patches)
    {
        if (!std::isfinite(mix) || mix < 0.0 || mix > 1.0)
            return false;
        juce::ValueTree synth("SYNTH");
        synth.setProperty("mix", mix, nullptr);
        for (std::size_t i = 0; i < synthPatchCount; ++i)
        {
            if (!validSynthPatch(patches[i]))
                return false;
            synth.setProperty(synthPatchId(i), patches[i], nullptr);
        }
        clearSynth();
        state.appendChild(synth, nullptr);
        return true;
    }
    void clearSynth()
    {
        const auto synth = synthNode();
        if (synth.isValid())
            state.removeChild(synth, nullptr);
    }
    [[nodiscard]] bool hasSynth() const { return synthNode().isValid(); }
    [[nodiscard]] double synthMix() const { return hasSynth() ? static_cast<double>(synthNode().getProperty("mix")) : 0.0; }
    [[nodiscard]] juce::String synthPatch(std::size_t i) const
    {
        return hasSynth() && i < synthPatchCount ? synthNode().getProperty(synthPatchId(i)).toString() : juce::String {};
    }
    // without the synthesis: what builds before the device can read
    [[nodiscard]] juce::ValueTree snapshotWithoutSynth() const
    {
        auto copy = state.createCopy();
        const auto synth = copy.getChildWithName("SYNTH");
        if (synth.isValid())
            copy.removeChild(synth, nullptr);
        return copy;
    }

    [[nodiscard]] juce::ValueTree ecologySnapshot() const { return ecology.snapshot(); }
    [[nodiscard]] juce::ValueTree snapshot() const { return state.createCopy(); }

    [[nodiscard]] bool restoreXml(const juce::String& text)
    {
        const auto xml = parseParameterXml(text, maximumXmlCharacters, 5);
        return xml && restore(juce::ValueTree::fromXml(*xml));
    }

    // Pure settings migration: PropertySet may be in memory, with no file I/O.
    // Complete V1 wins; otherwise ecological V1 wins over scalar legacy keys.
    void loadPreferences(const juce::PropertySet& preferences)
    {
        if (restoreXml(preferences.getValue("parameter.performanceStateV1")))
            return;
        PerformanceParameterState migrated;
        EcologyParameterState legacyEcology;
        if (!legacyEcology.restoreXml(preferences.getValue("parameter.ecologyStateV1")))
        {
            using Key = EcologyParameterState::Key;
            struct LegacyEcology { Key key; const char* id; double initial; bool inverted; };
            constexpr std::array<LegacyEcology, 8> legacy {{
                { Key::density, "density", 0.45, false },
                { Key::calm, "drift", 0.20, true },
                { Key::cohesion, "cohesion", 0.50, false },
                { Key::danger, "danger", 0.50, false },
                { Key::sparseFocus, "blendSparseFocus", 0.25, false },
                { Key::energyMemory, "blendEnergyMemory", 0.25, false },
                { Key::geometricCycle, "blendGeometricCycle", 0.25, false },
                { Key::collectiveDrift, "blendCollectiveDrift", 0.25, false }
            }};
            for (const auto& item : legacy)
            {
                const auto value = readLegacy(preferences, item.id, item.initial);
                static_cast<void>(legacyEcology.set(item.key, item.inverted ? 1.0 - value : value));
            }
        }
        static_cast<void>(migrated.setEcology(legacyEcology.snapshot()));
        for (const auto& parameter : parameters)
            static_cast<void>(migrated.set(parameter.id,
                readLegacy(preferences, parameter.id, parameter.initial)));
        migrated.setAutonomous(preferences.getBoolValue("parameter.autonomous", true));
        static_cast<void>(restore(migrated.snapshot()));
    }

    [[nodiscard]] bool restore(const juce::ValueTree& candidate)
    {
        if (!candidate.hasType("AQUORBIUM_PERFORMANCE_V1")
            || candidate.getNumChildren() < 1 || candidate.getNumChildren() > 2)
            return false;
        const auto propertyCount = candidate.getNumProperties();
        const auto legacy = propertyCount == static_cast<int>(legacyParameterCount) + 1;
        const auto preTail = propertyCount
            == static_cast<int>(spectralEnvironmentParameterCount) + 1;
        const auto preForm = propertyCount
            == static_cast<int>(syntheticTailParameterCount) + 1;
        const auto preMaster = propertyCount
            == static_cast<int>(formMemoryParameterCount) + 1;
        const auto preStructure = propertyCount
            == static_cast<int>(outputStageParameterCount) + 1;
        const auto preModuleBank = propertyCount
            == static_cast<int>(structureMemoryParameterCount) + 1;
        const auto preCreativeMaster = propertyCount
            == static_cast<int>(preCreativeMasterParameterCount) + 1;
        const auto preEcologicalMusic = propertyCount
            == static_cast<int>(preEcologicalMusicParameterCount) + 1;
        const auto preTimbre = propertyCount
            == static_cast<int>(preTimbreParameterCount) + 1;
        const auto preTimbreKind = propertyCount
            == static_cast<int>(preTimbreKindParameterCount) + 1;
        const auto preSalinity = propertyCount
            == static_cast<int>(preSalinityParameterCount) + 1;
        const auto preSupport = propertyCount
            == static_cast<int>(preSupportParameterCount) + 1;
        const auto preEqOrder = propertyCount
            == static_cast<int>(preEqOrderParameterCount) + 1;
        const auto preCards = propertyCount
            == static_cast<int>(preCardsParameterCount) + 1;
        if (!legacy && !preTail && !preForm && !preMaster
            && !preStructure && !preModuleBank && !preCreativeMaster
            && !preEcologicalMusic && !preTimbre && !preTimbreKind && !preSalinity && !preSupport && !preEqOrder && !preCards
            && propertyCount != static_cast<int>(parameters.size()) + 1)
            return false;
        PerformanceParameterState replacement;
        if (!replacement.setEcology(candidate.getChild(0)))
            return false;
        for (std::size_t index = 0; index < parameters.size(); ++index)
        {
            const auto& parameter = parameters[index];
            if ((legacy && index >= legacyParameterCount)
                || (preTail && index >= spectralEnvironmentParameterCount)
                || (preForm && index >= syntheticTailParameterCount)
                || (preMaster && index >= formMemoryParameterCount)
                || (preStructure && index >= outputStageParameterCount)
                || (preModuleBank && index >= structureMemoryParameterCount)
                || (preCreativeMaster
                    && index >= preCreativeMasterParameterCount)
                || (preEcologicalMusic
                    && index >= preEcologicalMusicParameterCount)
                || (preTimbre && index >= preTimbreParameterCount)
                || (preTimbreKind && index >= preTimbreKindParameterCount)
                || (preSalinity && index >= preSalinityParameterCount)
                || (preSupport && index >= preSupportParameterCount)
                || (preEqOrder && index >= preEqOrderParameterCount)
                || (preCards && index >= preCardsParameterCount))
                continue;
            double value = 0.0;
            if (!readNumber(candidate.getProperty(parameter.id), value)
                || !replacement.set(parameter.id, value))
                return false;
        }
        double enabled = 0.0;
        if (!readNumber(candidate.getProperty("autonomous"), enabled)
            || enabled < 0.0 || enabled > 1.0 || (enabled > 0.0 && enabled < 1.0))
            return false;
        replacement.setAutonomous(enabled > 0.5);
        if (candidate.getNumChildren() == 2)
        {
            // the synthesis: exactly mix and the five patches, all valid
            const auto synth = candidate.getChild(1);
            if (!synth.hasType("SYNTH") || synth.getNumChildren() != 0
                || synth.getNumProperties() != static_cast<int>(synthPatchCount) + 1)
                return false;
            double mix = 0.0;
            if (!readNumber(synth.getProperty("mix"), mix))
                return false;
            std::array<juce::String, synthPatchCount> patches;
            for (std::size_t i = 0; i < synthPatchCount; ++i)
            {
                const auto value = synth.getProperty(synthPatchId(i));
                if (!value.isString())
                    return false;
                patches[i] = value.toString();
            }
            if (!replacement.setSynth(mix, patches))
                return false;
        }
        if (preTimbreKind)
            for (int i = 0; i < 5; ++i)
            {
                // the seven first bodies keep their order: index = old * 6
                const auto index = std::round(replacement.get(("timbreBody" + juce::String(i)).toRawUTF8()) * 6.0);
                static_cast<void>(replacement.set(("timbreKind" + juce::String(i)).toRawUTF8(), index / 15.0));
            }
        state = replacement.snapshot();
        static_cast<void>(ecology.restore(replacement.ecologySnapshot()));
        return true;
    }

private:
    [[nodiscard]] juce::ValueTree synthNode() const { return state.getChildWithName("SYNTH"); }
    [[nodiscard]] static juce::Identifier synthPatchId(std::size_t i)
    {
        static const std::array<juce::Identifier, synthPatchCount> ids { "patch0", "patch1", "patch2", "patch3", "patch4" };
        return ids[std::min(i, synthPatchCount - 1)];
    }

    static double readLegacy(const juce::PropertySet& preferences, const char* id, double initial)
    {
        double value = initial;
        if (!readNumber(preferences.getValue("parameter." + juce::String(id)), value))
            return initial;
        return juce::jlimit(0.0, 1.0, value);
    }

    static bool isKnown(const char* id)
    {
        for (const auto& parameter : parameters)
            if (juce::String(id) == parameter.id)
                return true;
        return false;
    }

    static bool readNumber(const juce::var& property, double& value)
    {
        if (property.isString())
        {
            return text::readWholeDecimal(property.toString().toStdString(), value) && std::isfinite(value);
        }
        if (!property.isDouble() && !property.isInt() && !property.isInt64())
            return false;
        value = static_cast<double>(property);
        return std::isfinite(value);
    }

    EcologyParameterState ecology;
    juce::ValueTree state { "AQUORBIUM_PERFORMANCE_V1" };
};
}
