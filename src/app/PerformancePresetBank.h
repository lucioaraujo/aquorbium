// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include "PerformanceParameterState.h"

namespace aquorbium::app
{
// Four local user scenes, with independent copies and transactional restore.
class PerformancePresetBank
{
public:
    static constexpr std::size_t capacity = 4;
    static constexpr int maximumXmlCharacters = 65536;
    static constexpr int maximumNameCharacters = 32;

    [[nodiscard]] static bool validName(const juce::String& name)
    {
        if (name.length() > maximumNameCharacters) return false;
        for (const auto character : name)
            if (character < 0x20 || (character >= 0x7f && character <= 0x9f)
                || (character >= 0xd800 && character <= 0xdfff) || character > 0x10ffff
                || (character & 0xffff) >= 0xfffe
                || character == 0x2028 || character == 0x2029) return false;
        return true;
    }

    [[nodiscard]] bool store(std::size_t slot, const PerformanceParameterState& state)
    {
        if (slot >= capacity)
            return false;
        return store(slot, state, names[slot]);
    }

    [[nodiscard]] bool store(std::size_t slot, const PerformanceParameterState& state,
                             const juce::String& name)
    {
        if (slot >= capacity || !validName(name)) return false;
        auto replacement = state.snapshot();
        const auto normalizedName = name.trim();
        if (occupied(slot) && slots[slot].isEquivalentTo(replacement) && names[slot] == normalizedName)
            return true;
        remember(slot);
        slots[slot] = replacement;
        names[slot] = normalizedName;
        return true;
    }

    [[nodiscard]] juce::String name(std::size_t slot) const
    {
        return slot < capacity ? names[slot] : juce::String {};
    }

    [[nodiscard]] bool rename(std::size_t slot, const juce::String& value)
    {
        if (!occupied(slot) || !validName(value)) return false;
        const auto normalized = value.trim();
        if (names[slot] == normalized) return true;
        remember(slot);
        names[slot] = normalized;
        return true;
    }

    [[nodiscard]] bool canUndo() const noexcept { return previous.available; }
    [[nodiscard]] std::size_t undoSlot() const noexcept
    {
        return previous.available ? previous.slot : capacity;
    }

    [[nodiscard]] bool undo()
    {
        if (!canUndo()) return false;
        slots[previous.slot] = previous.state;
        names[previous.slot] = previous.name;
        previous = {};
        return true;
    }

    [[nodiscard]] bool recall(std::size_t slot, PerformanceParameterState& state) const
    {
        return slot < capacity && slots[slot].isValid() && state.restore(slots[slot]);
    }

    [[nodiscard]] bool occupied(std::size_t slot) const
    {
        return slot < capacity && slots[slot].isValid();
    }

    // V3 (2026-10-22): the scenes with the synthesis device's patches
    [[nodiscard]] juce::ValueTree snapshot() const
    {
        return makeSnapshot(Version::v3);
    }

    // V2 (named) and the parameter-only V1, without the synthesis, so that
    // builds from before each step still read the scenes.
    [[nodiscard]] juce::ValueTree namedSnapshot() const { return makeSnapshot(Version::v2); }
    [[nodiscard]] juce::ValueTree legacySnapshot() const { return makeSnapshot(Version::v1); }

    [[nodiscard]] bool loadPreferences(const juce::PropertySet& preferences)
    {
        const auto withSynth = preferences.getValue("parameter.presetBankV3");
        // as with V2: a present but invalid bank fails, nothing is replaced
        if (withSynth.isNotEmpty()) return restoreXml(withSynth);
        const auto current = preferences.getValue("parameter.presetBankV2");
        if (current.isNotEmpty()) return restoreXml(current);
        const auto legacy = preferences.getValue("parameter.presetBankV1");
        return legacy.isNotEmpty() ? restoreXml(legacy) : restore(PerformancePresetBank {}.snapshot());
    }

    void savePreferences(juce::PropertySet& preferences) const
    {
        preferences.setValue("parameter.presetBankV3", snapshot().toXmlString());
        preferences.setValue("parameter.presetBankV2", namedSnapshot().toXmlString());
        preferences.setValue("parameter.presetBankV1", legacySnapshot().toXmlString());
    }

    [[nodiscard]] bool restoreXml(const juce::String& text)
    {
        const auto xml = parseParameterXml(text, maximumXmlCharacters, 32, true);
        return xml && restore(juce::ValueTree::fromXml(*xml));
    }

    [[nodiscard]] bool restore(const juce::ValueTree& candidate)
    {
        const auto version3 = candidate.hasType("AQUORBIUM_PRESETS_V3");
        const auto version2 = version3 || candidate.hasType("AQUORBIUM_PRESETS_V2");
        if ((!version2 && !candidate.hasType("AQUORBIUM_PRESETS_V1"))
            || candidate.getNumProperties() != 0
            || candidate.getNumChildren() != static_cast<int>(capacity))
            return false;
        PerformancePresetBank replacement;
        for (std::size_t slot = 0; slot < capacity; ++slot)
        {
            const auto entry = candidate.getChild(static_cast<int>(slot));
            const auto named = version2 && entry.hasProperty("name");
            if (!entry.hasType("SLOT") || entry.getNumChildren() > 1
                || entry.getNumProperties() != (named ? 1 : 0)) return false;
            if (entry.getNumChildren() == 0)
            {
                if (named) return false;
                continue;
            }
            const auto value = named ? entry.getProperty("name") : juce::var(juce::String {});
            if (!value.isString() || !validName(value.toString()) || value.toString() != value.toString().trim())
                return false;
            PerformanceParameterState state;
            if (!state.restore(entry.getChild(0)) || !replacement.store(slot, state, value.toString()))
                return false;
        }
        slots = replacement.slots;
        names = replacement.names;
        previous = {}; // Undo is session-local and is never persisted.
        return true;
    }

private:
    enum class Version { v1, v2, v3 };
    [[nodiscard]] juce::ValueTree makeSnapshot(Version version) const
    {
        const auto version2 = version != Version::v1;
        juce::ValueTree bank(version == Version::v3 ? "AQUORBIUM_PRESETS_V3"
                             : version2             ? "AQUORBIUM_PRESETS_V2"
                                                    : "AQUORBIUM_PRESETS_V1");
        for (std::size_t slot = 0; slot < capacity; ++slot)
        {
            juce::ValueTree entry("SLOT");
            if (occupied(slot))
            {
                auto scene = slots[slot].createCopy();
                const auto synth = scene.getChildWithName("SYNTH");
                if (version != Version::v3 && synth.isValid())
                    scene.removeChild(synth, nullptr);
                entry.appendChild(scene, nullptr);
                if (version2 && names[slot].isNotEmpty()) entry.setProperty("name", names[slot], nullptr);
            }
            bank.appendChild(entry, nullptr);
        }
        return bank;
    }

    void remember(std::size_t slot)
    {
        previous = { slots[slot].createCopy(), names[slot], slot, true };
    }

    struct Previous
    {
        juce::ValueTree state;
        juce::String name;
        std::size_t slot = capacity;
        bool available = false;
    };
    std::array<juce::ValueTree, capacity> slots {};
    std::array<juce::String, capacity> names {};
    Previous previous;
};
}
