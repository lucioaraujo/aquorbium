// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace aquorbium
{
// The synthesis device's other sources (SYNTH_DEVICE_DESIGN.md, phase 3;
// 2026-10-22). Each is played through three controls of the source's
// section - FORMA, LARGURA, COR - whose meaning depends on the source:
//
//   FM         ratio of B to A (a musical table) | index | A's feedback
//   WAVETABLE  position (saw > square > formant > sine) | phase warp | drift
//   ADDITIVE   tilt (dark > bright) | odd/even balance | stretch (inharmonic)
//   PULSAR     formant (0.1x .. 8x the pitch) | pulsaret shape | window
//   STRING     damping | pluck position | sustain
//   MATTER     structure (harmonic > stretched) | brightness | damping
//
// Provenance - adapted from the author's Rasgo Modular
// (RASGO/PROJETOS_AUTORAIS/RASGO_MODULAR, commit 4ce898f, AGPLv3, same
// author), the processing cores only:
//   src/dsp/Operator.hpp     sha256 3e167ab18c2979f9...  (2 of the 4 operators:
//                            the B->A chain with A's feedback; the ratio table)
//   src/dsp/Wavetable.hpp    sha256 776460b77adf247a...  (the 16-frame recipe,
//                            10 band-limited mip-maps, the phase warp; no capture)
//   src/dsp/Additive.hpp     sha256 92f44a63099fc136...  (64 partials, tilt,
//                            odd, stretch, the gain follower; no comb)
//   src/dsp/Pulsar.hpp       sha256 a59b0c73648942e0...  (Roads's pulsaret
//                            train, 4 grain voices; no jitter/mask/spread)
//   src/dsp/StringVoice.hpp  sha256 f76c04c9e0f2bd76...  (Karplus-Strong with
//                            the fractional all-pass and the pluck comb)
//   src/dsp/Matter.hpp       sha256 dc1af55119a8b8d5...  (24 two-pole modes,
//                            structure, brightness, damping, position)
// Adapted: plain classes, fixed arrays (no std::vector after prepare), the
// three macros mapped as above, and an excite() for the note onsets. The
// wavetable's tables are built once and shared read-only by every voice.
// No allocation, lock or I/O in render(): safe on the audio thread.

class FmSource final
{
public:
    void prepare(float sampleRate) noexcept;
    void reset() noexcept;
    [[nodiscard]] float render(float frequency, float ratio, float index, float feedback) noexcept;

private:
    float rate = 48000.0f;
    float phaseA = 0.0f, phaseB = 0.0f, a1 = 0.0f, a2 = 0.0f;
};

// FM 4-OP (2026-10-22): the whole OPERATOR of the Rasgo Modular - four sine
// operators A B C D, eight routing algorithms, B C D's ratios from the
// musical table, A's feedback (the mean of two samples, as in the DX7), the
// carriers' sum divided by their number and a soft clip as a safety net.
// Adapted from src/dsp/Operator.hpp (commit 4ce898f, sha256 3e167ab18c2979f9...,
// AGPLv3, same author): the shared sine table instead of its own LUT, no drift
// (the voice has DRIFT), the algorithm and ratios C/D set at control rate.
class Fm4Source final
{
public:
    static constexpr int algorithms = 8;
    // the musical ratio table: a 0..1 macro to 0.5x .. 9x (for the editor too)
    [[nodiscard]] static float ratioOf(float macro) noexcept;
    void prepare(float sampleRate) noexcept;
    void reset() noexcept;
    // control rate: the algorithm 0..7 and the ratios of C and D (0..1 over
    // the musical table)
    void configure(int algorithm, float ratioC, float ratioD) noexcept;
    // per sample: B's ratio (0..1 over the table), the index and A's feedback
    [[nodiscard]] float render(float frequency, float ratioB, float index, float feedback) noexcept;

private:
    float rate = 48000.0f;
    int algorithm = 0;
    float ratioC = 2.0f, ratioD = 3.0f;
    std::array<float, 4> phase {};
    float a1 = 0.0f, a2 = 0.0f;
};

class WavetableSource final
{
public:
    static constexpr int frames = 16;
    static constexpr int length = 1024;
    static constexpr int mips = 10;
    void prepare(float sampleRate, std::uint64_t seed) noexcept;
    void reset() noexcept;
    [[nodiscard]] float render(float frequency, float position, float warp, float drift) noexcept;

private:
    float rate = 48000.0f;
    float phase = 0.0f;
    float driftPosition = 0.0f, driftTarget = 0.0f;
    std::uint32_t driftCount = 0, driftInterval = 1600;
    std::uint64_t random = 0x123456789abcdefull, seed = 0x123456789abcdefull;
};

class AdditiveSource final
{
public:
    static constexpr int partials = 64;
    void prepare(float sampleRate) noexcept;
    void reset() noexcept;
    [[nodiscard]] float render(float frequency, float tilt, float odd, float stretch) noexcept;

private:
    void recompute(float tilt, float odd, float stretch) noexcept;
    float rate = 48000.0f;
    std::array<float, partials> phase {}, ratio {}, amplitude {};
    int audible = partials;  // partials above -80 dB of the sum (the rest is not computed)
    float norm = 0.1f, follower = 1.0f, peak = 1.0e-6f;
    float lastTilt = -1.0f, lastOdd = -9.0f, lastStretch = -9.0f;
    std::uint32_t counter = 0;
};

class PulsarSource final
{
public:
    void prepare(float sampleRate) noexcept;
    void reset() noexcept;
    [[nodiscard]] float render(float frequency, float formant, float shape, float window) noexcept;

private:
    struct Grain
    {
        bool active = false;
        float position = 0.0f, duration = 1.0f;
    };
    float rate = 48000.0f;
    double phase = 0.0;
    std::array<Grain, 4> grains {};
    std::uint32_t next = 0;
};

class StringSource final
{
public:
    static constexpr std::size_t capacity = 4096;  // >= 48 kHz / 15 Hz, at 96 kHz down to ~24 Hz
    void prepare(float sampleRate, std::uint64_t seed) noexcept;
    void reset() noexcept;
    void excite(float velocity, float frequency, float position) noexcept;
    [[nodiscard]] float render(float frequency, float damping, float position, float sustain) noexcept;

private:
    [[nodiscard]] float white() noexcept;
    float rate = 48000.0f;
    std::array<float, capacity> buffer {};
    std::size_t write = 0;
    float lowpass = 0.0f, allpassIn = 0.0f, allpassOut = 0.0f;
    std::uint64_t random = 0x6a09e667f3bcc909ull, seed = 0x6a09e667f3bcc909ull;
};

class MatterSource final
{
public:
    static constexpr int modes = 24;
    void prepare(float sampleRate, std::uint64_t seed) noexcept;
    void reset() noexcept;
    void excite(float velocity) noexcept;
    [[nodiscard]] float render(float frequency, float structure, float brightness, float damping) noexcept;

private:
    struct Mode
    {
        float a1 = 0.0f, a2 = 0.0f, b0 = 0.0f, z1 = 0.0f, z2 = 0.0f;
        bool live = false;
    };
    void update(float frequency, float structure, float brightness, float damping) noexcept;
    [[nodiscard]] float white() noexcept;
    float rate = 48000.0f;
    std::array<Mode, modes> bank {};
    float burst = 0.0f, strength = 0.0f;
    float lastF = -1.0f, lastStructure = -1.0f, lastBrightness = -1.0f, lastDamping = -1.0f;
    std::uint64_t random = 0x3c6ef372fe94f82bull, seed = 0x3c6ef372fe94f82bull;
};

// FORMANT - a filter mode (2026-10-22): five band-passes in parallel whose
// centres, widths and gains follow a table of sung vowels, A > E > I > O > U,
// walked by one control. Adapted from the author's Rasgo Modular
// (src/dsp/Formant.hpp, commit 4ce898f, sha256 079366eeb48e3006..., AGPLv3,
// same author): the five TPT SVF band-passes (Andrew Simper) with the soft
// saturation in the loop, the log-frequency interpolation between vowels,
// and the width divided by (1 + 8 res). Left out: the vocoder, the drift
// and the dry/wet mix (the voice has its own SYNTHESIS mix).
// The vowel table is published phonetic data - a bass voice, F1..F5 with
// gain and bandwidth (Gunnar Fant, "Acoustic Theory of Speech Production",
// 1960; the formant tables in the appendix of the Csound manual) - facts,
// not code; the same values as the Modular's.
// The tract: the five formants move together by `tract` (1 = the table's
// voice; the voice passes cutoff / 1000 Hz, so CUTOFF becomes the size of
// the vocal tract and keeps its envelope, key tracking, wah and routes).
class FormantFilter final
{
public:
    static constexpr int bands = 5;
    static constexpr int vowels = 5;
    void reset() noexcept;
    // control rate: the vowel 0..1 (A..U), the tract multiplier, resonance 0..1
    void control(float vowel, float tract, float resonance, float sampleRate) noexcept;
    [[nodiscard]] float process(float x) noexcept;

private:
    std::array<float, bands> g {}, k {}, gain {}, ic1 {}, ic2 {};
};

// CHAOS (2026-10-23): a forced Duffing oscillator, x'' + d x' - x + x^3 =
// g cos(t), its forcing at the note's frequency - the double well of the
// Rasgo Modular's CHAOS (guia/36_chaos.md), rewritten as an audio source
// that is pitched by its forcing. FORCING (g, 0.15..0.70) leads from a small
// periodic orbit in one well, through chaos (with periodic windows of period
// 3 and 6), to a large periodic orbit across both wells - measured
// 2026-10-23; at this forcing frequency the way into chaos is a crisis, not
// a cascade of period doublings. DAMPING (d) moves those borders; COLOUR goes
// from the position to the brighter velocity. A slow level follower (~80 ms)
// keeps the regimes at one loudness: the small orbit is ~12 dB under chaos.
// Ueda (1979); Guckenheimer & Holmes, "Nonlinear Oscillations, Dynamical
// Systems, and Bifurcations of Vector Fields" (1983), section 2.2.
class ChaosSource final
{
public:
    void prepare(float sampleRate) noexcept;
    void reset() noexcept;
    [[nodiscard]] float render(float frequency, float forcing, float damping, float colour) noexcept;

private:
    float rate = 48000.0f;
    double x = 1.0, v = 0.0, phase = 0.0;
    float dcIn = 0.0f, dcOut = 0.0f, dcCoefficient = 0.999f;
    float power = 0.09f, powerCoefficient = 0.001f;
};

// FEEDBACK (2026-10-23): a loop with no input - a delay tuned to the note's
// period, a low-pass, a DC block and a tanh, its gain held just above unity
// at the fundamental, so it sings by itself, as a no-input mixer does.
// GAIN from barely singing to saturated; DETUNE adds a second tap off the
// period (towards roughness and instability); COLOUR opens the loop's
// low-pass. Excited at the note onset, sustained by its own gain.
class FeedbackSource final
{
public:
    static constexpr std::size_t capacity = 4096;
    void prepare(float sampleRate, std::uint64_t seed) noexcept;
    void reset() noexcept;
    void excite(float velocity, float frequency) noexcept;
    [[nodiscard]] float render(float frequency, float gain, float detune, float colour) noexcept;

private:
    [[nodiscard]] float tap(float delay) const noexcept;
    float rate = 48000.0f;
    std::array<float, capacity> buffer {};
    std::size_t write = 0;
    float lowpass = 0.0f, dcIn = 0.0f, dcOut = 0.0f, dcCoefficient = 0.999f;
    float lastFrequency = -1.0f, lastColour = -1.0f, lastDetune = -1.0f;
    float k = 0.5f, compensation = 1.0f, loopDelay = 0.0f;
    std::uint64_t random = 0xbb67ae8584caa73bull, seed = 0xbb67ae8584caa73bull;
};
}
