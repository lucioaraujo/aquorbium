// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include <juce_data_structures/juce_data_structures.h>

#include <algorithm>
#include <string>

namespace aquorbium::app
{
// Reject DTDs and excessive nesting before parsing. Only local scene-bank
// names may opt into predefined XML escapes and printable Unicode character
// references emitted by JUCE (never custom entities or control characters).
[[nodiscard]] inline std::unique_ptr<juce::XmlElement> parseParameterXml(
    const juce::String& text, int maximumBytes, int maximumTags,
    bool allowNameEscapes = false)
{
    if (text.getNumBytesAsUTF8() > static_cast<std::size_t>(maximumBytes)
        || text.contains("<!") || (!allowNameEscapes && text.containsChar('&')))
        return {};
    const auto characters = text.toStdString();
    if (allowNameEscapes)
        for (auto position = characters.find('&'); position != std::string::npos;
             position = characters.find('&', position + 1))
        {
            auto allowed = characters.compare(position, 5, "&amp;") == 0
                || characters.compare(position, 4, "&lt;") == 0
                || characters.compare(position, 4, "&gt;") == 0
                || characters.compare(position, 6, "&quot;") == 0
                || characters.compare(position, 6, "&apos;") == 0;
            if (!allowed && characters.compare(position, 2, "&#") == 0)
            {
                const auto end = characters.find(';', position + 2);
                if (end == std::string::npos || end == position + 2 || end - position > 10)
                    return {};
                unsigned value = 0;
                for (auto digit = position + 2; digit < end; ++digit)
                {
                    if (characters[digit] < '0' || characters[digit] > '9') return {};
                    value = value * 10 + static_cast<unsigned>(characters[digit] - '0');
                    if (value > 0x10ffff) return {};
                }
                allowed = value >= 0x20 && !(value >= 0x7f && value <= 0x9f)
                    && !(value >= 0xd800 && value <= 0xdfff)
                    && (value & 0xffff) < 0xfffe && value != 0x2028 && value != 0x2029;
            }
            if (!allowed) return {};
        }
    if (std::count(characters.begin(), characters.end(), '<') > maximumTags)
        return {};
    auto xml = juce::parseXML(text);
    if (xml && !xml->getAllSubText().containsNonWhitespaceChars())
        return xml;
    return {};
}
}
