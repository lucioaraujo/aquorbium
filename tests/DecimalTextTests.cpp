// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

// The decimal reader that replaced std::from_chars for floating point
// (2026-10-26, for macOS 10.15): exact like it, and blind to the locale.
#include "core/DecimalText.h"

#include <clocale>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

namespace
{
int failures = 0;
void check(bool condition, const std::string& what)
{
    if (!condition)
    {
        std::cerr << "FAILED: " << what << '\n';
        ++failures;
    }
}
}

int main()
{
    using aquorbium::text::readDecimal;
    using aquorbium::text::readWholeDecimal;
    double v = 0.0;
    check(readWholeDecimal("0.45", v) && v == 0.45, "0.45");
    check(readWholeDecimal("-1e-05", v) && v == -1e-05, "-1e-05");
    check(readWholeDecimal("12", v) && v == 12.0, "12");
    check(readWholeDecimal(".5", v) && v == 0.5, ".5");
    for (const auto* bad : { "", "+1", "abc", "1.2.3", "0,45", "1e", "-", ".", "nan", "inf", "1e400", " 1" })
        check(!readWholeDecimal(bad, v), std::string("rejected: '") + bad + "'");
    check(readDecimal("0.62:evolution", v) == 4 && v == 0.62, "a number inside a rule");
    check(readDecimal("2e:x", v) == 1 && v == 2.0, "an 'e' that is not an exponent");
    // exact round trip, as std::from_chars: every %.17g of a spread of doubles
    std::uint64_t state = 0x9e3779b97f4a7c15ull;
    auto exact = true;
    for (int i = 0; i < 20000 && exact; ++i)
    {
        state ^= state << 13;
        state ^= state >> 7;
        state ^= state << 17;
        const auto x = static_cast<double>(state >> 11) / 9007199254740992.0 * (i % 2 == 0 ? 1.0 : -1000.0);
        char buffer[64];
        std::snprintf(buffer, sizeof buffer, "%.17g", x);
        exact = readWholeDecimal(buffer, v) && v == x;
    }
    check(exact, "17 significant digits read back exactly");
    // a desktop with a comma for the decimal point (French, Brazilian)
    if (std::setlocale(LC_ALL, "fr_FR.UTF-8") != nullptr || std::setlocale(LC_ALL, "pt_BR.UTF-8") != nullptr)
    {
        check(readWholeDecimal("0.45", v) && v == 0.45, "0.45 under a comma locale");
        std::setlocale(LC_ALL, "C");
    }
    else
        std::cout << "no comma locale installed: that case not run\n";
    if (failures == 0)
        std::cout << "decimal text tests passed\n";
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
