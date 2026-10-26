// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include "core/SynthSources.h"
#include "core/TimbreBody.h"

#include <array>
#include <cstddef>
#include <cstdint>

namespace aquorbium
{
// The synthesis device's voice, one per organism (SYNTH_DEVICE_DESIGN.md,
// phase 1; 2026-10-21). The author: "os timbres são consequência de
// ferramentas bem desenvolvidas de síntese sonora no instrumento e o músico
// que utilizar o instrumento cria os sons". So this is a tool, not a timbre:
// a semi-modular path the musician sets, played by the ecosystem's notes.
//
//   SOURCE A -+
//             +- levels, ring (A x B), FM (B -> A) -> FILTER -> VCA <- AMP ENV
//   SOURCE B -+                                      ^
//                                   MOD ENV, keytrack, LFO 2 (octaves)
//   LFO 1 -> pitch (vibrato, cents)
//
// Sources: an oscillator (sine -> triangle -> saw -> pulse by one SHAPE
// control, pulse width, a sub-octave, drift), noise (white -> pink -> brown
// by one COLOUR control), or the organism's NATIVE body (its own DSP and its
// timbre body, passed in).
//
// Provenance - adapted from the author's Rasgo Modular
// (RASGO/PROJETOS_AUTORAIS/RASGO_MODULAR, commit 4ce898f, AGPLv3, same
// author), the processing cores only, rewritten into this class:
//   src/dsp/Oscillator.hpp  sha256 bf222eb3...  PolyBLEP saw/pulse/sub,
//                           drift random walk (Valimaki & Huovilainen;
//                           Martin Finke);
//   src/dsp/Filter.hpp      sha256 74b15052...  the TPT state-variable
//                           filter (Andrew Simper, Cytomic 2013);
//   src/dsp/Envelope.hpp    sha256 19e31fc7...  the gate-driven stage
//                           machine with a continuous curve;
//   src/dsp/Noise.hpp       sha256 e1be5572...  Paul Kellet's "economy"
//                           pink filter (music-dsp, public domain).
// Adapted: no port/graph abstraction (plain members), one oscillator with a
// morphing shape instead of five simultaneous outputs, the four filter modes
// from one SVF, and an ADSR (the Modular's ENVELOPE is AD/ASR).
//
// Fixed state, no allocation, lock or I/O: safe on the audio thread.
// Deterministic for a seed.
class SynthVoice final
{
public:
    // phase 3 (2026-10-22): the Rasgo Modular's sources and the organisms'
    // bodies (SynthSources.h, TimbreBody.h)
    enum class Source : std::uint8_t { off, oscillator, noise, native, fm, wavetable, additive, pulsar, string, matter, body,
                                       fm4,  // appended 2026-10-22: saved patches keep their indices
                                       chaos, feedback };  // appended 2026-10-23
    static constexpr std::size_t sourceCount = 14;
    // FORMANT (2026-10-22): five vowel formants; CUTOFF becomes the tract
    enum class FilterMode : std::uint8_t { lowPass, bandPass, highPass, notch, formant };
    static constexpr std::size_t filterModeCount = 5;
    enum class LfoShape : std::uint8_t { sine, triangle, square, wander };

    struct Envelope
    {
        float attack = 0.005f;   // seconds
        float decay = 0.25f;
        float sustain = 0.7f;    // 0..1
        float release = 0.3f;
        float curve = 0.5f;      // 0 convex, 0.5 linear, 1 concave
    };

    // ---- the modulation matrix (phase 2): the ecosystem plays the parameters ----
    enum class ModSource : std::uint8_t
    {
        none, ampEnvelope, modEnvelope, lfo1, lfo2, accent,
        // the ecosystem, each normalised 0..1 by the engine
        energy, speed, relation, tension, distance, channel, temperature, hunger, age, prey, habitat, room
    };
    static constexpr std::size_t modSourceCount = 18;
    enum class ModDestination : std::uint8_t
    {
        none, pitch, cutoff, resonance, drive, shapeA, shapeB, widthA, widthB, levelA, levelB, ring, fm,
        colourA, colourB, level, lfo1Rate, lfo2Rate,
        vowel  // appended 2026-10-22 (FORMANT): saved routes keep their indices
    };
    static constexpr std::size_t modDestinationCount = 19;
    struct Route
    {
        ModSource source = ModSource::none;
        ModDestination destination = ModDestination::none;
        float amount = 0.0f;  // -1..1: pitch +-12 semitones, cutoff +-4 octaves,
                              // rates x2^(+-3), level x(1 +- 1), the rest +-1
    };
    static constexpr std::size_t routeCount = 8;
    // the ecosystem's signals for this organism, 0..1 (set at control rate)
    struct Ecology
    {
        float energy = 0.0f, speed = 0.0f, relation = 0.0f, tension = 0.0f, distance = 0.0f, channel = 0.0f,
              temperature = 0.0f, hunger = 0.0f, age = 0.0f, prey = 0.0f, habitat = 0.0f, room = 0.0f;
    };

    struct Lfo
    {
        float rate = 0.5f;       // Hz
        LfoShape shape = LfoShape::sine;
    };

    struct Patch
    {
        Source sourceA = Source::oscillator;
        Source sourceB = Source::off;
        float shapeA = 0.67f, shapeB = 0.67f;    // sine 0 .. tri 1/3 .. saw 2/3 .. pulse 1
        float widthA = 0.5f, widthB = 0.5f;      // pulse width
        float tuneA = 0.0f, tuneB = 0.0f;        // semitones
        float fineA = 0.0f, fineB = 0.0f;        // cents
        float subA = 0.0f, subB = 0.0f;          // sub-octave level
        float colourA = 0.0f, colourB = 0.0f;    // noise: white 0 .. pink 0.5 .. brown 1
        float levelA = 1.0f, levelB = 0.0f;
        std::uint8_t bodyA = 1, bodyB = 1;       // which body, when the source is BODY (TimbreBody::Kind)
        // FM 4-OP: the algorithm (0..7) and C and D's ratios (0..1 over the
        // musical table; the Modular's defaults 2x and 3x); B's ratio is SHAPE
        std::uint8_t algorithmA = 0, algorithmB = 0;
        float ratioCA = 3.0f / 9.0f, ratioDA = 5.0f / 9.0f, ratioCB = 3.0f / 9.0f, ratioDB = 5.0f / 9.0f;
        float ring = 0.0f;                       // A x B
        float fm = 0.0f;                         // B -> A, linear (through-zero) FM
        float drift = 0.0f;                      // the tuning breathes
        FilterMode filterMode = FilterMode::lowPass;
        float cutoff = 2000.0f;                  // Hz
        float resonance = 0.2f;                  // 0..1
        float keytrack = 0.5f;                   // 0 fixed .. 1 follows the note
        float filterEnv = 1.5f;                  // octaves from the mod envelope (+-)
        float drive = 0.0f;                      // 0..1
        float vowel = 0.0f;                      // FORMANT: A 0 .. E .. I .. O .. U 1
        Envelope amp {};
        Envelope mod { 0.005f, 0.4f, 0.2f, 0.3f, 0.5f };
        Lfo lfo1 { 5.0f, LfoShape::sine };
        Lfo lfo2 { 0.3f, LfoShape::triangle };
        float vibrato = 0.0f;                    // LFO 1 -> pitch, cents
        float wah = 0.0f;                        // LFO 2 -> cutoff, octaves
        float accentToLevel = 0.5f;              // how much the note's accent sets the level
        float level = 0.8f;
        std::array<Route, routeCount> routes {};
    };

    void prepare(double sampleRate, std::uint32_t seed) noexcept;
    void reset() noexcept;
    void setPatch(const Patch& value) noexcept
    {
        patch = value;
        applyMatrix(0.0f, 0.0f, 0.0f, 0.0f);
    }
    void setEcology(const Ecology& value) noexcept { ecology = value; }
    [[nodiscard]] const Ecology& currentEcology() const noexcept { return ecology; }
    [[nodiscard]] const Patch& currentPatch() const noexcept { return patch; }

    // the ecosystem's note: on (with its accent) and off
    void noteOn(float accent) noexcept;
    void noteOff() noexcept;
    [[nodiscard]] bool gate() const noexcept { return gateOpen; }

    // one sample: the note's frequency (with its glide) and the organism's
    // native body, for the NATIVE source
    [[nodiscard]] float render(float frequencyHz, float native) noexcept;

    // the envelopes' values, for tests and the panel
    [[nodiscard]] float ampLevel() const noexcept { return ampStage.value; }
    [[nodiscard]] float modLevel() const noexcept { return modStage.value; }

private:
    struct Stage
    {
        enum class Step : std::uint8_t { idle, attack, decay, sustain, release };
        Step step = Step::idle;
        float phase = 0.0f;
        float start = 0.0f;
        float value = 0.0f;
        float advance(const Envelope& e, bool gate, float sampleRate) noexcept;
    };

    struct Oscillator
    {
        double phase = 0.0, subPhase = 0.0;
        float render(double increment, float shape, float width, float sub) noexcept;
    };

    struct NoiseSource
    {
        float b0 = 0, b1 = 0, b2 = 0, b3 = 0, b4 = 0, b5 = 0, b6 = 0;  // Kellet's pink
        float brown = 0.0f;
        float render(float white, float colour) noexcept;
    };

    // everything a source slot (A or B) can be
    struct Slot
    {
        Oscillator osc {};
        NoiseSource noise {};
        FmSource fm {};
        Fm4Source fm4 {};
        ChaosSource chaos {};
        FeedbackSource feedback {};
        WavetableSource wavetable {};
        AdditiveSource additive {};
        PulsarSource pulsar {};
        StringSource string {};
        MatterSource matter {};
        TimbreBody body {};
        std::uint8_t bodyKind = 0;
    };

    [[nodiscard]] float nextWhite() noexcept;
    [[nodiscard]] float lfoValue(float& phase, float& held, float& target, const Lfo& lfo) noexcept;
    [[nodiscard]] float source(Source which, Slot& slot, double increment, float shape, float width, float sub,
                               float colour, std::uint8_t bodyKind, float native) noexcept;
    void syncBody(Slot& slot, std::uint8_t bodyKind, bool onset) noexcept;
    void clearSlot(Slot& slot) noexcept;
    [[nodiscard]] float sourceValue(ModSource which, float amp, float mod, float lfo1, float lfo2) const noexcept;
    void applyMatrix(float amp, float mod, float lfo1, float lfo2) noexcept;

    // the parameters after the matrix (recomputed at control rate)
    struct Effective
    {
        float pitch = 0.0f, cutoffOctaves = 0.0f, resonance = 0.2f, drive = 0.0f;
        float shapeA = 0.67f, shapeB = 0.67f, widthA = 0.5f, widthB = 0.5f;
        float levelA = 1.0f, levelB = 0.0f, ring = 0.0f, fm = 0.0f, colourA = 0.0f, colourB = 0.0f;
        float level = 0.8f, lfo1Rate = 5.0f, lfo2Rate = 0.3f;
        float vowel = 0.0f;
    };
    Effective effective {};
    Ecology ecology {};

    Patch patch {};
    float rate = 48000.0f;
    std::uint32_t seed = 0x5a17e5u;
    std::uint64_t random = 0x5a17e5u;
    Slot slotA {}, slotB {};
    Stage ampStage {}, modStage {};
    bool gateOpen = false;
    float accent = 1.0f;
    float lfo1Phase = 0.0f, lfo2Phase = 0.0f;
    float lfo1Held = 0.0f, lfo2Held = 0.0f, lfo1Target = 0.0f, lfo2Target = 0.0f;
    float driftState = 0.0f;
    std::uint32_t driftCounter = 0, driftInterval = 1200;
    float lastB = 0.0f;
    // the filter (TPT SVF), and the FORMANT mode's five bands
    float ic1 = 0.0f, ic2 = 0.0f;
    FormantFilter formant {};
    double driveLast = 0.0;  // the drive's previous input (ADAA)
    // control-rate values (every 8 samples): the frequencies and the filter
    // coefficients - tan, pow and exp2 per sample were most of the cost
    std::uint32_t controlCounter = 0;
    bool pendingExcite = false;
    float base = 110.0f, fA = 110.0f, fB = 110.0f;
    float g = 0.1f, k = 2.0f;
};
}
