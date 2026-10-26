// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

// Synthesis patches as text (2026-10-21): a patch survives the round trip,
// unknown keys are ignored, missing keys keep their defaults, values are
// clamped to their ranges.
#include "core/SynthPatchCodec.h"

#include <cmath>
#include <cstdlib>
#include <iostream>

namespace
{
using aquorbium::SynthPatchCodec;
using aquorbium::SynthVoice;
int failures = 0;
void check(bool condition, const char* what)
{
    if (!condition)
    {
        std::cerr << "FAILED: " << what << '\n';
        ++failures;
    }
}
bool near(float a, float b) { return std::abs(a - b) <= 1.0e-4f * std::max(1.0f, std::abs(b)); }
}

int main()
{
    SynthVoice::Patch p;
    p.sourceA = SynthVoice::Source::noise;
    p.sourceB = SynthVoice::Source::native;
    p.shapeA = 0.123f;
    p.tuneB = -7.0f;
    p.fineA = 12.5f;
    p.filterMode = SynthVoice::FilterMode::notch;
    p.cutoff = 1234.5f;
    p.filterEnv = -2.25f;
    p.amp.release = 3.5f;
    p.mod.curve = 0.9f;
    p.lfo2.shape = SynthVoice::LfoShape::wander;
    p.vibrato = 33.0f;
    p.routes[3] = { SynthVoice::ModSource::hunger, SynthVoice::ModDestination::cutoff, -0.75f };
    p.routes[7] = { SynthVoice::ModSource::room, SynthVoice::ModDestination::lfo2Rate, 0.5f };
    const auto text = SynthPatchCodec::encode(p);
    const auto back = SynthPatchCodec::decode(text);
    check(back.sourceA == p.sourceA && back.sourceB == p.sourceB && back.filterMode == p.filterMode
              && back.lfo2.shape == p.lfo2.shape,
          "the choices survive the round trip");
    check(near(back.shapeA, p.shapeA) && near(back.tuneB, p.tuneB) && near(back.fineA, p.fineA)
              && near(back.cutoff, p.cutoff) && near(back.filterEnv, p.filterEnv) && near(back.amp.release, p.amp.release)
              && near(back.mod.curve, p.mod.curve) && near(back.vibrato, p.vibrato),
          "the values survive the round trip");
    check(back.routes[3].source == p.routes[3].source && back.routes[3].destination == p.routes[3].destination
              && near(back.routes[3].amount, -0.75f) && back.routes[7].source == p.routes[7].source
              && near(back.routes[7].amount, 0.5f) && back.routes[0].source == SynthVoice::ModSource::none,
          "the matrix survives the round trip");

    const auto defaults = SynthPatchCodec::decode("");
    const SynthVoice::Patch fresh;
    check(near(defaults.cutoff, fresh.cutoff) && defaults.sourceA == fresh.sourceA, "an empty text gives the defaults");
    const auto odd = SynthPatchCodec::decode("cutoff=99999;resonance=-3;futureKnob=4;sourceA=99;tuneA=abc;widthA=0;");
    check(near(odd.cutoff, 20000.0f) && near(odd.resonance, 0.0f) && odd.sourceA == static_cast<SynthVoice::Source>(SynthVoice::sourceCount - 1)
              && near(odd.widthA, 0.0f),
          "out-of-range values are clamped, unknown keys ignored");
    check(near(odd.tuneA, 0.0f), "a malformed number reads as zero, within range");

    if (failures == 0)
        std::cout << "synth patch codec: all checks passed (" << text.size() << " characters per patch)\n";
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
