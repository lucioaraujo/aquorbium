// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include <cstddef>
#include <locale>
#include <sstream>
#include <string>
#include <string_view>

namespace aquorbium::text
{
// A decimal number read from text, for the publication (2026-10-26).
// std::from_chars for floating point needs macOS 13.3 or later in Apple's
// libc++, and the app targets macOS 10.15 (the RASGO multiplatform standard);
// strtod follows the C locale's decimal point, which a French or Brazilian
// desktop may set to a comma. This reads the C locale's grammar - [sign]
// digits [. digits] [(e|E) [sign] digits] - and converts it through a stream
// imbued with the classic locale, correctly rounded as from_chars was.
// It allocates: not for the audio thread.
//
// Returns how many characters of `text` the number takes (0: no number at
// the start of `text`), and writes the value.
inline std::size_t readDecimal(std::string_view text, double& value) noexcept
{
    std::size_t at = 0;
    const auto digits = [&]() {
        const auto from = at;
        while (at < text.size() && text[at] >= '0' && text[at] <= '9')
            ++at;
        return at - from;
    };
    if (at < text.size() && text[at] == '-')  // as from_chars: no leading '+'
        ++at;
    auto mantissa = digits();
    if (at < text.size() && text[at] == '.')
    {
        ++at;
        mantissa += digits();
    }
    if (mantissa == 0)
        return 0;
    if (at < text.size() && (text[at] == 'e' || text[at] == 'E'))
    {
        const auto mark = at++;
        if (at < text.size() && (text[at] == '-' || text[at] == '+'))
            ++at;
        if (digits() == 0)
            at = mark;  // "2e" is the number 2 followed by an 'e'
    }
    try
    {
        std::istringstream stream { std::string(text.substr(0, at)) };
        stream.imbue(std::locale::classic());
        double parsed = 0.0;
        stream >> parsed;
        if (stream.fail())
            return 0;
        value = parsed;
        return at;
    }
    catch (...)
    {
        return 0;
    }
}

// the whole of `text` is one number
inline bool readWholeDecimal(std::string_view text, double& value) noexcept
{
    double parsed = 0.0;
    if (text.empty() || readDecimal(text, parsed) != text.size())
        return false;
    value = parsed;
    return true;
}
}
