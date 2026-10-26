// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include "core/SynthVoice.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <type_traits>

namespace aquorbium
{
// A synthesis patch as text, for the settings and the scenes (2026-10-21):
// "key=value;key=value;...". Unknown keys are ignored and missing keys keep
// their defaults, so a patch saved today still loads when the voice grows
// new parameters. Every value is clamped to its range on reading.
class SynthPatchCodec final
{
public:
    using Patch = SynthVoice::Patch;

    [[nodiscard]] static std::string encode(const Patch& p)
    {
        std::string out;
        const auto put = [&out](const char* key, float value) {
            char buffer[64];
            std::snprintf(buffer, sizeof buffer, "%s=%.6g;", key, static_cast<double>(value));
            out += buffer;
        };
        forEach(const_cast<Patch&>(p), [&put](const char* key, float& value, float, float) { put(key, value); });
        return out;
    }

    [[nodiscard]] static Patch decode(const std::string& text)
    {
        Patch p;
        std::size_t start = 0;
        while (start < text.size())
        {
            const auto end = std::min(text.find(';', start), text.size());
            const auto pair = text.substr(start, end - start);
            const auto equals = pair.find('=');
            if (equals != std::string::npos)
            {
                const auto key = pair.substr(0, equals);
                const auto value = static_cast<float>(std::strtod(pair.c_str() + equals + 1, nullptr));
                forEach(p, [&key, value](const char* name, float& field, float lo, float hi) {
                    if (key == name && std::isfinite(value))
                        field = std::clamp(value, lo, hi);
                });
            }
            start = end + 1;
        }
        return p;
    }

private:
    // every parameter as a float with its range; enumerations travel as
    // their index (through a temporary)
    template <typename Visitor>
    static void forEach(Patch& p, Visitor&& visit)
    {
        const auto enumeration = [&visit](const char* key, auto& value, float count) {
            auto index = static_cast<float>(static_cast<int>(value));
            visit(key, index, 0.0f, count - 1.0f);
            value = static_cast<std::remove_reference_t<decltype(value)>>(static_cast<int>(std::lround(index)));
        };
        enumeration("sourceA", p.sourceA, static_cast<float>(SynthVoice::sourceCount));
        enumeration("sourceB", p.sourceB, static_cast<float>(SynthVoice::sourceCount));
        {
            // which body, when the source is BODY (1 .. TimbreBody::kindCount - 1)
            auto a = static_cast<float>(p.bodyA), b = static_cast<float>(p.bodyB);
            visit("bodyA", a, 1.0f, static_cast<float>(TimbreBody::kindCount - 1));
            visit("bodyB", b, 1.0f, static_cast<float>(TimbreBody::kindCount - 1));
            p.bodyA = static_cast<std::uint8_t>(std::lround(a));
            p.bodyB = static_cast<std::uint8_t>(std::lround(b));
        }
        {
            // FM 4-OP (2026-10-22): the algorithm 0..7 and C and D's ratios
            auto a = static_cast<float>(p.algorithmA), b = static_cast<float>(p.algorithmB);
            visit("algorithmA", a, 0.0f, static_cast<float>(Fm4Source::algorithms - 1));
            visit("algorithmB", b, 0.0f, static_cast<float>(Fm4Source::algorithms - 1));
            p.algorithmA = static_cast<std::uint8_t>(std::lround(a));
            p.algorithmB = static_cast<std::uint8_t>(std::lround(b));
            visit("ratioCA", p.ratioCA, 0.0f, 1.0f);
            visit("ratioDA", p.ratioDA, 0.0f, 1.0f);
            visit("ratioCB", p.ratioCB, 0.0f, 1.0f);
            visit("ratioDB", p.ratioDB, 0.0f, 1.0f);
        }
        visit("shapeA", p.shapeA, 0.0f, 1.0f);
        visit("shapeB", p.shapeB, 0.0f, 1.0f);
        visit("widthA", p.widthA, 0.0f, 1.0f);  // 0 = FM index 0, no warp (phase 3)
        visit("widthB", p.widthB, 0.0f, 1.0f);
        visit("tuneA", p.tuneA, -24.0f, 24.0f);
        visit("tuneB", p.tuneB, -24.0f, 24.0f);
        visit("fineA", p.fineA, -50.0f, 50.0f);
        visit("fineB", p.fineB, -50.0f, 50.0f);
        visit("subA", p.subA, 0.0f, 1.0f);
        visit("subB", p.subB, 0.0f, 1.0f);
        visit("colourA", p.colourA, 0.0f, 1.0f);
        visit("colourB", p.colourB, 0.0f, 1.0f);
        visit("levelA", p.levelA, 0.0f, 1.0f);
        visit("levelB", p.levelB, 0.0f, 1.0f);
        visit("ring", p.ring, 0.0f, 1.0f);
        visit("fm", p.fm, 0.0f, 1.0f);
        visit("drift", p.drift, 0.0f, 1.0f);
        enumeration("filterMode", p.filterMode, static_cast<float>(SynthVoice::filterModeCount));
        visit("cutoff", p.cutoff, 20.0f, 20000.0f);
        visit("resonance", p.resonance, 0.0f, 1.0f);
        visit("keytrack", p.keytrack, 0.0f, 1.0f);
        visit("filterEnv", p.filterEnv, -4.0f, 4.0f);
        visit("drive", p.drive, 0.0f, 1.0f);
        visit("vowel", p.vowel, 0.0f, 1.0f);
        visit("ampA", p.amp.attack, 0.001f, 5.0f);
        visit("ampD", p.amp.decay, 0.001f, 5.0f);
        visit("ampS", p.amp.sustain, 0.0f, 1.0f);
        visit("ampR", p.amp.release, 0.001f, 8.0f);
        visit("ampC", p.amp.curve, 0.0f, 1.0f);
        visit("modA", p.mod.attack, 0.001f, 5.0f);
        visit("modD", p.mod.decay, 0.001f, 5.0f);
        visit("modS", p.mod.sustain, 0.0f, 1.0f);
        visit("modR", p.mod.release, 0.001f, 8.0f);
        visit("modC", p.mod.curve, 0.0f, 1.0f);
        visit("lfo1Rate", p.lfo1.rate, 0.01f, 20.0f);
        enumeration("lfo1Shape", p.lfo1.shape, 4.0f);
        visit("lfo2Rate", p.lfo2.rate, 0.01f, 20.0f);
        enumeration("lfo2Shape", p.lfo2.shape, 4.0f);
        visit("vibrato", p.vibrato, 0.0f, 100.0f);
        visit("wah", p.wah, 0.0f, 4.0f);
        visit("accent", p.accentToLevel, 0.0f, 1.0f);
        visit("level", p.level, 0.0f, 1.5f);
        for (std::size_t r = 0; r < SynthVoice::routeCount; ++r)
        {
            char source[16], destination[16], amount[16];
            std::snprintf(source, sizeof source, "r%zuS", r);
            std::snprintf(destination, sizeof destination, "r%zuD", r);
            std::snprintf(amount, sizeof amount, "r%zuA", r);
            enumeration(source, p.routes[r].source, static_cast<float>(SynthVoice::modSourceCount));
            enumeration(destination, p.routes[r].destination, static_cast<float>(SynthVoice::modDestinationCount));
            visit(amount, p.routes[r].amount, -1.0f, 1.0f);
        }
    }
};
}
