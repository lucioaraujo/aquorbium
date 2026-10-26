// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include "ParameterXml.h"
#include "core/DecimalText.h"

#include <array>
#include <cmath>
#include <string>

namespace aquorbium::app
{
// GUI-thread state only. Audio receives a separate plain coherent frame.
class EcologyParameterState
{
public:
    static constexpr int maximumXmlCharacters = 4096;
    enum class Key : std::size_t
    {
        density,
        calm,
        cohesion,
        danger,
        sparseFocus,
        energyMemory,
        geometricCycle,
        collectiveDrift
    };

    EcologyParameterState()
    {
        for (const auto& item : parameters)
            state.setProperty(item.id, item.initial, nullptr);
    }

    EcologyParameterState(const EcologyParameterState& other) : state(other.snapshot()) {}
    EcologyParameterState& operator=(const EcologyParameterState& other)
    {
        if (this != &other)
            state = other.snapshot();
        return *this;
    }

    [[nodiscard]] bool set(Key key, double value)
    {
        if (!std::isfinite(value))
            return false;
        state.setProperty(descriptor(key).id, juce::jlimit(0.0, 1.0, value), nullptr);
        return true;
    }

    [[nodiscard]] double get(Key key) const
    {
        const auto& item = descriptor(key);
        return static_cast<double>(state.getProperty(item.id, item.initial));
    }

    [[nodiscard]] juce::ValueTree snapshot() const { return state.createCopy(); }

    [[nodiscard]] bool restoreXml(const juce::String& text)
    {
        const auto xml = parseParameterXml(text, maximumXmlCharacters, 3);
        return xml && restore(juce::ValueTree::fromXml(*xml));
    }

    [[nodiscard]] bool restore(const juce::ValueTree& candidate)
    {
        if (!candidate.hasType("AQUORBIUM_ECOLOGY_V1") || candidate.getNumChildren() != 0
            || candidate.getNumProperties() != static_cast<int>(parameters.size()))
            return false;
        // Build a complete sanitized replacement before changing the live state.
        EcologyParameterState replacement;
        for (const auto& item : parameters)
        {
            if (!candidate.hasProperty(item.id))
                return false;
            const auto property = candidate.getProperty(item.id);
            double value = 0.0;
            if (property.isString())
            {
                // JUCE turns XML attributes into strings on ValueTree::fromXml.
                if (!text::readWholeDecimal(property.toString().toStdString(), value))
                    return false;
            }
            else if (property.isDouble() || property.isInt() || property.isInt64())
                value = static_cast<double>(property);
            else
                return false;
            if (!replacement.set(item.key, value))
                return false;
        }
        state = replacement.snapshot();
        return true;
    }

    // the persisted name of a key ("density", "calm"...), for the VARIA
    [[nodiscard]] static const char* idOf(Key key) { return descriptor(key).id; }

private:
    struct Descriptor
    {
        Key key;
        const char* id;
        double initial;
    };

    static constexpr std::array<Descriptor, 8> parameters {{
        { Key::density, "density", 0.45 },
        { Key::calm, "calm", 0.80 },
        { Key::cohesion, "cohesion", 0.50 },
        { Key::danger, "danger", 0.50 },
        { Key::sparseFocus, "sparseFocus", 0.25 },
        { Key::energyMemory, "energyMemory", 0.25 },
        { Key::geometricCycle, "geometricCycle", 0.25 },
        { Key::collectiveDrift, "collectiveDrift", 0.25 }
    }};

    static const Descriptor& descriptor(Key key)
    {
        return parameters[static_cast<std::size_t>(key)];
    }

    juce::ValueTree state { "AQUORBIUM_ECOLOGY_V1" };
};
}
