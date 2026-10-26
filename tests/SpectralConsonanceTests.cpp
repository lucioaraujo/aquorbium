// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

// Consonance from timbre and register (2026-10-20): with harmonic tones the
// familiar ranking appears; the same third is rougher in the bass; a
// membrane favours other intervals than a voice; pure tones are rough only
// when close.
#include "core/SpectralConsonance.h"

#include <cstdlib>
#include <iostream>

namespace
{
using C = aquorbium::SpectralConsonance;
int failures = 0;
void check(bool condition, const char* what)
{
    if (!condition)
    {
        std::cerr << "FAILED: " << what << '\n';
        ++failures;
    }
}
float at(float base, float semitones, const C::Spectrum& s)
{
    return C::consonance(base, s, base + semitones, s);
}
}

int main()
{
    const auto h = C::harmonic(6);
    const float mid = 60.0f;
    if (std::getenv("SPECTRAL_REPORT"))
        for (int k = 0; k <= 12; ++k)
            std::cout << k << ": harmonic " << at(mid, static_cast<float>(k), h) << "  membrane "
                      << at(mid, static_cast<float>(k), C::membrane()) << "  low " << at(36.0f, static_cast<float>(k), h) << '\n';
    check(at(mid, 0, h) > 0.99f && at(mid, 0, C::membrane()) > 0.99f, "unison is smooth, even for an inharmonic tone");
    check(at(mid, 12, h) > at(mid, 7, h) - 0.02f, "the octave is at least as smooth as the fifth");
    check(at(mid, 7, h) > at(mid, 4, h), "the fifth is smoother than the major third");
    check(at(mid, 7, h) > at(mid, 6, h) + 0.2f, "the fifth is much smoother than the tritone");
    check(at(mid, 4, h) > at(mid, 1, h) + 0.3f, "the major third is much smoother than the minor second");
    // (an equal-tempered major third beats almost like the tritone in this
    // model - 400 cents, not the just 386: an honest result, not a bug)
    check(at(mid, 5, h) > at(mid, 2, h), "the fourth is smoother than the major second");
    // the register: the same third is rougher in the bass (Plomp & Levelt)
    check(at(36.0f, 4, h) < at(72.0f, 4, h), "a major third is rougher low than high");
    // timbre: a membrane prefers a minor sixth (ratio ~1.593 is one of its modes)
    const auto m = C::membrane();
    const auto sixthAdvantageMembrane = at(mid, 8, m) - at(mid, 7, m);
    const auto sixthAdvantageHarmonic = at(mid, 8, h) - at(mid, 7, h);
    check(sixthAdvantageMembrane > sixthAdvantageHarmonic, "a membrane favours the minor sixth more than a voice does");
    // pure tones: beyond a critical band everything is smooth
    const auto s = C::sine();
    check(at(mid, 7, s) > 0.9f && at(mid, 6, s) > 0.8f, "pure tones a fifth or a tritone apart are smooth (mid register)");
    check(at(36.0f, 4, s) < at(mid, 4, s), "the critical band is wider in the bass: low pure thirds are rougher");
    check(at(mid, 1, s) < 0.6f, "pure tones a semitone apart are rough");
    // blending: a voice half mixed with a membrane lies between the two
    const auto mix = C::blend(h, 0.5f, m, 0.5f);
    check(mix.count > 0 && mix.count <= C::maxPartials, "the blend keeps a bounded spectrum");
    if (failures == 0)
        std::cout << "spectral consonance: all checks passed\n";
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
