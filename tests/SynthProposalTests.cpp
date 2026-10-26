// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

// PROPOSTA (2026-10-22): the starting patches are a pure function of
// (organism, seed), keep each species' gesture and each strategy's ecology,
// stay valid patches, and every one of them sounds - bounded - when played.
// No timbre is judged here: the criterion is the tool's contract.
#include "core/SynthPatchCodec.h"
#include "core/SynthProposal.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <memory>
#include <set>
#include <string>

namespace
{
using aquorbium::SynthPatchCodec;
using aquorbium::SynthProposal;
using aquorbium::SynthVoice;
using Source = SynthVoice::Source;
int failures = 0;
void check(bool condition, const std::string& what)
{
    if (!condition)
    {
        std::cerr << "FAILED: " << what << '\n';
        ++failures;
    }
}
constexpr int seeds = 120;
}

int main()
{
    // 1. deterministic, and a new seed gives a new patch
    for (std::size_t o = 0; o < SynthProposal::organisms; ++o)
    {
        check(SynthPatchCodec::encode(SynthProposal::propose(o, 1234u)) == SynthPatchCodec::encode(SynthProposal::propose(o, 1234u)),
              "the same seed gives the same patch, organism " + std::to_string(o));
        std::set<std::string> distinct;
        for (int s = 0; s < seeds; ++s)
            distinct.insert(SynthPatchCodec::encode(SynthProposal::propose(o, static_cast<std::uint32_t>(s))));
        check(distinct.size() == static_cast<std::size_t>(seeds), "every seed gives another patch, organism " + std::to_string(o));
    }
    check(SynthPatchCodec::encode(SynthProposal::propose(0, 7u)) != SynthPatchCodec::encode(SynthProposal::propose(1, 7u)),
          "the organisms differ for the same seed");

    std::set<int> sourcesUsed;
    int unlevelled = 0, singers = 0;
    float quietest = 1.0f, loudest = 0.0f;
    for (int s = 0; s < seeds; ++s)
        for (std::size_t o = 0; o < SynthProposal::organisms; ++o)
        {
            const auto seed = static_cast<std::uint32_t>(s * 7919 + 13);
            const auto p = SynthProposal::propose(o, seed);
            const auto tag = " (organism " + std::to_string(o) + ", seed " + std::to_string(seed) + ")";
            sourcesUsed.insert(static_cast<int>(p.sourceA));
            if (p.filterMode == SynthVoice::FilterMode::formant)
            {
                ++singers;
                check(o == 0 && std::any_of(p.routes.begin(), p.routes.end(), [](const SynthVoice::Route& r) {
                          return r.destination == SynthVoice::ModDestination::vowel; }),
                      "a FORMANT proposal is MORPH's, with its vowel routed" + tag);
            }
            sourcesUsed.insert(static_cast<int>(p.sourceB));

            // 2. a valid patch: the codec's clamping changes nothing
            const auto text = SynthPatchCodec::encode(p);
            check(SynthPatchCodec::encode(SynthPatchCodec::decode(text)) == text, "the proposal is inside the codec's ranges" + tag);
            check(p.sourceA != Source::off && p.sourceA != Source::native, "source A is a synthesis source" + tag);

            // 3. the species' gesture
            switch (o)
            {
            case 0: check(p.amp.sustain >= 0.55f, "MORPH sustains" + tag); break;
            case 1: check(p.amp.sustain == 0.0f && p.amp.attack <= 0.004f, "GRAIN is struck" + tag); break;
            case 2: check(p.amp.decay >= 1.0f && p.amp.release >= 0.8f, "MODAL resonates" + tag); break;
            case 3: check(p.wah > 0.0f, "PULSAR's formant moves" + tag); break;
            default:
                check(p.sourceA == Source::string || p.sourceA == Source::body || p.sourceA == Source::oscillator,
                      "STRING is a string, a body or a bowed oscillator" + tag);
            }
            // 4. the strategy's ecology is always the first route
            static const SynthVoice::ModSource first[] { SynthVoice::ModSource::hunger, SynthVoice::ModSource::tension,
                                                         SynthVoice::ModSource::distance, SynthVoice::ModSource::prey,
                                                         SynthVoice::ModSource::habitat };
            check(p.routes[0].source == first[o] && p.routes[0].amount != 0.0f, "the strategy's signal leads the matrix" + tag);

            // 5. it sounds, bounded and levelled: the audition of the patch
            // as proposed (a note held 0.6 s at 220 Hz)
            auto voice = std::make_unique<SynthVoice>();
            const auto heard = SynthProposal::audition(*voice, p, seed);
            quietest = std::min(quietest, heard.loudness);
            loudest = std::max(loudest, heard.peak);
            check(heard.finite, "the note stays finite" + tag);
            check(heard.loudness > 0.02f, "the note is heard (its loudest 10 ms above -34 dBFS)" + tag);
            check(heard.peak <= 1.5f + 1.0e-3f, "the note's peak stays under 1.5" + tag);
            const auto offTarget = std::abs(20.0f * std::log10(std::max(heard.loudness, 1.0e-6f) / SynthProposal::targetLoudness));
            if (offTarget > 6.0f)
                ++unlevelled;
        }
    // the levelling brings (nearly) every proposal within 6 dB of the target
    check(unlevelled <= seeds * 5 / 50, "at least 98 % of the proposals within 6 dB of the target loudness");
    // MORPH sings now and then (about a quarter of its proposals)
    check(singers >= seeds / 8 && singers <= seeds / 2, "MORPH proposes the FORMANT mode about a quarter of the time");
    // 6. the proposals visit the whole device, not one corner of it
    check(sourcesUsed.size() >= 11, "the proposals use at least eleven of the twelve sources (off included)");

    std::cout << "proposals: " << sourcesUsed.size() << " sources used, quietest (loudest 10 ms, RMS) " << quietest
              << ", loudest peak " << loudest << ", " << unlevelled << " of " << seeds * 5 << " beyond 6 dB of the target, " << singers << " FORMANT\n";
    if (failures == 0)
        std::cout << "synth proposal: all checks passed\n";
    return failures == 0 ? 0 : 1;
}
