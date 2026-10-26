// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/SynthSources.h"

#include <algorithm>
#include <cmath>

namespace aquorbium
{
namespace
{
constexpr float twoPi = 6.28318530717958647692f;
constexpr float pi = 3.14159265358979323846f;

float clampf(float v, float lo, float hi) noexcept
{
    return v < lo ? lo : (v > hi ? hi : v);
}

std::uint64_t xorshift(std::uint64_t& s) noexcept
{
    s ^= s >> 12;
    s ^= s << 25;
    s ^= s >> 27;
    return s * 0x2545f4914f6cdd1dull;
}

float whiteFrom(std::uint64_t& s) noexcept
{
    return static_cast<float>(static_cast<std::int32_t>(xorshift(s) >> 32)) / 2147483648.0f;
}

// a shared sine table (2048 points, linear interpolation), as the Modular's
// OPERATOR and ADDITIVE use
struct SineTable
{
    static constexpr int size = 2048;
    std::array<float, size + 1> values {};
    SineTable() noexcept
    {
        for (int i = 0; i <= size; ++i)
            values[static_cast<std::size_t>(i)] = std::sin(twoPi * static_cast<float>(i) / size);
    }
    float operator()(float phase) const noexcept
    {
        const auto x = (phase - std::floor(phase)) * static_cast<float>(size);
        const auto i = static_cast<int>(x);
        const auto fraction = x - static_cast<float>(i);
        const auto a = values[static_cast<std::size_t>(i)];
        return a + (values[static_cast<std::size_t>(i + 1)] - a) * fraction;
    }
    // a phase already in [0, 1): no floor (the additive's inner loop)
    float wrapped(float phase) const noexcept
    {
        const auto x = phase * static_cast<float>(size);
        const auto i = static_cast<int>(x);
        const auto fraction = x - static_cast<float>(i);
        const auto a = values[static_cast<std::size_t>(i)];
        return a + (values[static_cast<std::size_t>(i + 1)] - a) * fraction;
    }
};
const SineTable& sine() noexcept
{
    static const SineTable table;
    return table;
}

// ---- the wavetable's shared tables --------------------------------------------------------
// 16 frames from one fixed spectral recipe (saw -> square -> formant -> sine:
// no data file - the author's decision for the Modular, 2026-09-06), each in
// 10 band-limited mip-maps (all 512 harmonics down to 1)
struct Tables
{
    static constexpr int frames = WavetableSource::frames;
    static constexpr int length = WavetableSource::length;
    static constexpr int mips = WavetableSource::mips;
    static constexpr int maxHarmonic = length / 2;
    std::array<float, static_cast<std::size_t>(frames * mips * length)> data {};

    static float recipe(int h, float pos01) noexcept
    {
        const auto segment = pos01 * 3.0f;
        const auto a = std::min(3, static_cast<int>(segment));
        const auto t = segment - static_cast<float>(a);
        const auto arche = [h](int which) -> float {
            switch (which)
            {
                case 0: return 1.0f / static_cast<float>(h);                             // saw
                case 1: return (h & 1) ? 1.0f / static_cast<float>(h) : 0.0f;           // square
                case 2:                                                                  // formant
                {
                    const auto hf = static_cast<float>(h);
                    const auto bell = [hf](float c) {
                        const auto d = (hf - c) / 1.6f;
                        return std::exp(-d * d);
                    };
                    return 0.9f * bell(3.0f) + 0.7f * bell(8.0f) + 0.5f * bell(14.0f);
                }
                default: return h == 1 ? 1.0f : 0.0f;                                  // sine
            }
        };
        const auto hi = std::min(3, a + 1);
        return arche(a) * (1.0f - t) + arche(hi) * t;
    }

    Tables() noexcept
    {
        std::array<float, length> sinLut {};
        for (int i = 0; i < length; ++i)
            sinLut[static_cast<std::size_t>(i)] = std::sin(twoPi * static_cast<float>(i) / length);
        std::array<float, maxHarmonic + 1> amp {};
        for (int frame = 0; frame < frames; ++frame)
        {
            const auto pos01 = static_cast<float>(frame) / static_cast<float>(frames - 1);
            float norm = 0.0f;
            for (int h = 1; h <= maxHarmonic; ++h)
            {
                amp[static_cast<std::size_t>(h)] = recipe(h, pos01);
                norm += std::fabs(amp[static_cast<std::size_t>(h)]);
            }
            const auto g = norm > 1.0e-6f ? 0.9f / norm : 0.0f;
            for (int s = 0; s < length; ++s)
            {
                float acc = 0.0f;
                int mip = mips - 1;
                int threshold = 1;
                for (int h = 1; h <= maxHarmonic; ++h)
                {
                    acc += amp[static_cast<std::size_t>(h)] * g * sinLut[static_cast<std::size_t>((h * s) & (length - 1))];
                    if (h == threshold)
                    {
                        at(frame, mip)[s] = acc;
                        --mip;
                        threshold <<= 1;
                    }
                }
                while (mip >= 0)
                {
                    at(frame, mip)[s] = acc;
                    --mip;
                }
            }
        }
    }
    float* at(int frame, int mip) noexcept
    {
        return data.data() + static_cast<std::size_t>((frame * mips + mip) * length);
    }
    const float* at(int frame, int mip) const noexcept
    {
        return data.data() + static_cast<std::size_t>((frame * mips + mip) * length);
    }
};
const Tables& tables() noexcept
{
    static const Tables built;  // built once (in prepare, never on the audio thread)
    return built;
}
}

// ---- FM: the B -> A chain, B with feedback ------------------------------------------------

void FmSource::prepare(float sampleRate) noexcept
{
    rate = sampleRate;
    static_cast<void>(sine());
    reset();
}

void FmSource::reset() noexcept
{
    phaseA = phaseB = a1 = a2 = 0.0f;
}

float FmSource::render(float frequency, float ratioMacro, float indexMacro, float feedbackMacro) noexcept
{
    // the Modular's musical ratio table
    static constexpr std::array<float, 10> ratios { 0.5f, 1.0f, 1.5f, 2.0f, 2.5f, 3.0f, 4.0f, 5.0f, 7.0f, 9.0f };
    const auto ratio = ratios[static_cast<std::size_t>(std::lround(clampf(ratioMacro, 0.0f, 1.0f) * 9.0f))];
    const auto index = clampf(indexMacro, 0.0f, 1.0f);
    const auto depth = index * index * 6.0f;                 // up to ~6 cycles of deviation
    const auto feedback = clampf(feedbackMacro, 0.0f, 1.0f);
    const auto fb = feedback * feedback;
    const auto& s = sine();
    const auto f = clampf(frequency, 0.01f, 0.45f * rate);
    phaseA += f / rate;
    phaseA -= std::floor(phaseA);
    phaseB += f * ratio / rate;
    phaseB -= std::floor(phaseB);
    // the modulator B feeds back on its own phase (the mean of two samples, as the DX7)
    const auto b = s(phaseB + fb * 0.5f * (a1 + a2));
    a2 = a1;
    a1 = b;
    return s(phaseA + depth * b);
}

// ---- FM 4-OP (the Modular's OPERATOR) ---------------------------------------------------

namespace
{
constexpr std::array<float, 10> musicalRatios { 0.5f, 1.0f, 1.5f, 2.0f, 2.5f, 3.0f, 4.0f, 5.0f, 7.0f, 9.0f };
float musicalRatio(float macro) noexcept
{
    return musicalRatios[static_cast<std::size_t>(std::lround(clampf(macro, 0.0f, 1.0f) * 9.0f))];
}
// who modulates whom (A > B > C > D in a fixed order) and the carriers
struct Algorithm
{
    bool aToB, aToC, aToD, bToC, bToD, cToD;
    std::array<bool, 4> carrier;
};
constexpr std::array<Algorithm, Fm4Source::algorithms> fmAlgorithms {{
    { true, false, false, true, false, true, { false, false, false, true } },    // 0 A>B>C>D
    { true, false, false, true, false, false, { false, false, true, true } },    // 1 A>B>C, D
    { true, false, false, false, false, true, { false, true, false, true } },    // 2 A>B, C>D
    { false, true, false, true, false, true, { false, false, false, true } },    // 3 A>C, B>C>D
    { false, false, true, false, true, true, { false, false, false, true } },    // 4 A>D, B>D, C>D
    { true, false, false, false, false, false, { false, true, true, true } },    // 5 A>B, C, D
    { true, true, true, false, false, false, { false, true, true, true } },      // 6 A>B, A>C, A>D
    { false, false, false, false, false, false, { true, true, true, true } } }}; // 7 A, B, C, D (additive)
}

float Fm4Source::ratioOf(float macro) noexcept
{
    return musicalRatio(macro);
}

void Fm4Source::prepare(float sampleRate) noexcept
{
    rate = sampleRate;
    reset();
}

void Fm4Source::reset() noexcept
{
    phase.fill(0.0f);
    a1 = a2 = 0.0f;
}

void Fm4Source::configure(int algorithmIndex, float ratioCMacro, float ratioDMacro) noexcept
{
    algorithm = std::clamp(algorithmIndex, 0, algorithms - 1);
    ratioC = musicalRatio(ratioCMacro);
    ratioD = musicalRatio(ratioDMacro);
}

float Fm4Source::render(float frequency, float ratioBMacro, float indexMacro, float feedbackMacro) noexcept
{
    const auto& alg = fmAlgorithms[static_cast<std::size_t>(algorithm)];
    const std::array<float, 4> ratio { 1.0f, musicalRatio(ratioBMacro), ratioC, ratioD };
    const auto index = clampf(indexMacro, 0.0f, 1.0f);
    const auto depth = index * index * 6.0f;
    const auto feedback = clampf(feedbackMacro, 0.0f, 1.0f);
    const auto fb = feedback * feedback;
    const auto f = clampf(frequency, 0.01f, 0.45f * rate);
    for (std::size_t i = 0; i < 4; ++i)
    {
        phase[i] += f * ratio[i] / rate;
        phase[i] -= std::floor(phase[i]);
    }
    const auto& s = sine();
    const auto a = s(phase[0] + fb * 0.5f * (a1 + a2));
    const auto b = s(phase[1] + depth * (alg.aToB ? a : 0.0f));
    const auto c = s(phase[2] + depth * ((alg.aToC ? a : 0.0f) + (alg.bToC ? b : 0.0f)));
    const auto d = s(phase[3] + depth * ((alg.aToD ? a : 0.0f) + (alg.bToD ? b : 0.0f) + (alg.cToD ? c : 0.0f)));
    a2 = a1;
    a1 = a;
    float sum = 0.0f;
    int carriers = 0;
    const std::array<float, 4> op { a, b, c, d };
    for (std::size_t i = 0; i < 4; ++i)
        if (alg.carrier[i])
        {
            sum += op[i];
            ++carriers;
        }
    const auto y = sum / static_cast<float>(std::max(1, carriers));
    // the Modular's soft clip: transparent up to 1, asymptote 1.5
    if (y > 1.0f) return 1.0f + 0.5f * std::tanh((y - 1.0f) / 0.5f);
    if (y < -1.0f) return -1.0f - 0.5f * std::tanh((-y - 1.0f) / 0.5f);
    return y;
}

// ---- WAVETABLE -----------------------------------------------------------------------------

void WavetableSource::prepare(float sampleRate, std::uint64_t seedValue) noexcept
{
    rate = sampleRate;
    seed = seedValue | 1u;
    driftInterval = static_cast<std::uint32_t>(std::max(1.0f, sampleRate / 30.0f));
    static_cast<void>(tables());
    reset();
}

void WavetableSource::reset() noexcept
{
    phase = 0.0f;
    driftPosition = driftTarget = 0.0f;
    driftCount = 0;
    random = seed;
}

float WavetableSource::render(float frequency, float position, float warpMacro, float drift) noexcept
{
    const auto& t = tables();
    // drift: a slow autonomous sweep of the position
    if (drift > 0.0f && ++driftCount >= driftInterval)
    {
        driftCount = 0;
        driftTarget = whiteFrom(random) * 0.12f * clampf(drift, 0.0f, 1.0f);
    }
    driftPosition += (driftTarget - driftPosition) * 0.0015f;
    const auto f = clampf(frequency, 0.01f, 0.45f * rate);
    phase += f / rate;
    phase -= std::floor(phase);
    // the phase warp (Casio CZ phase distortion, the EMW WAVE-6's "wave cut")
    const auto warp = clampf(warpMacro, 0.0f, 1.0f);
    auto p = phase;
    if (warp > 0.0f)
    {
        const auto k = 0.5f - 0.45f * warp;
        p = phase < k ? phase * (0.5f / k) : 0.5f + (phase - k) * (0.5f / (1.0f - k));
    }
    // the mip whose highest harmonic stays under Nyquist
    const auto mipF = std::log2(std::max(1.0e-6f, static_cast<float>(length) * f / rate));
    const auto mip = std::clamp(static_cast<int>(std::ceil(mipF)), 0, mips - 1);
    const auto slot = clampf(position + driftPosition, 0.0f, 1.0f) * static_cast<float>(frames - 1);
    const auto s0 = std::min(frames - 2, static_cast<int>(slot));
    const auto blend = slot - static_cast<float>(s0);
    const auto read = [&t, mip](int frame, float ph) {
        const auto* table = t.at(frame, mip);
        const auto x = ph * static_cast<float>(length);
        const auto i = static_cast<int>(x) & (length - 1);
        const auto j = (i + 1) & (length - 1);
        const auto fr = x - std::floor(x);
        return table[i] + (table[j] - table[i]) * fr;
    };
    const auto a = read(s0, p), b = read(s0 + 1, p);
    return (a + (b - a) * blend) * 1.4f;  // the recipe is normalised to 0.9 of the absolute sum
}

// ---- ADDITIVE ------------------------------------------------------------------------------

void AdditiveSource::prepare(float sampleRate) noexcept
{
    rate = sampleRate;
    static_cast<void>(sine());
    reset();
}

void AdditiveSource::reset() noexcept
{
    phase.fill(0.0f);
    follower = 1.0f;
    peak = 1.0e-6f;
    lastTilt = -1.0f;
    counter = 0;
}

void AdditiveSource::recompute(float tilt, float odd, float stretch) noexcept
{
    const auto e = 2.6f - 2.45f * tilt;
    float sum = 0.0f;
    for (int k = 1; k <= partials; ++k)
    {
        const auto kf = static_cast<float>(k);
        ratio[static_cast<std::size_t>(k - 1)] = kf + stretch * 0.004f * kf * (kf - 1.0f);
        auto a = std::pow(kf, -e);
        a *= (k & 1) ? (1.0f + odd) : (1.0f - odd);
        a = std::max(0.0f, a);
        amplitude[static_cast<std::size_t>(k - 1)] = a;
        sum += a;
    }
    norm = 0.9f / std::max(sum, 1.0e-4f);
    audible = 1;
    for (int k = 0; k < partials; ++k)
        if (amplitude[static_cast<std::size_t>(k)] * norm > 1.0e-4f)
            audible = k + 1;
    lastTilt = tilt;
    lastOdd = odd;
    lastStretch = stretch;
}

float AdditiveSource::render(float frequency, float tiltMacro, float oddMacro, float stretchMacro) noexcept
{
    const auto tilt = clampf(tiltMacro, 0.0f, 1.0f);
    const auto odd = clampf(oddMacro * 2.0f - 1.0f, -1.0f, 1.0f);       // 0.5 = balanced
    const auto stretch = clampf(stretchMacro * 2.0f - 1.0f, -1.0f, 1.0f);  // 0.5 = harmonic
    if (std::abs(tilt - lastTilt) > 1.0e-4f || std::abs(odd - lastOdd) > 1.0e-4f
        || std::abs(stretch - lastStretch) > 1.0e-4f)
        recompute(tilt, odd, stretch);
    const auto nyquist = 0.5f * rate;
    const auto cut = 0.98f * nyquist, fade = 0.83f * nyquist;
    const auto f0 = clampf(frequency, 0.01f, cut);
    const auto& s = sine();
    float acc = 0.0f;
    const auto increment = f0 / rate;
    for (std::size_t k = 0; k < static_cast<std::size_t>(audible); ++k)
    {
        const auto fk = f0 * ratio[k];
        if (fk >= cut)
            break;
        auto ph = phase[k] + increment * ratio[k];  // < 0.49 per sample: one wrap at most
        if (ph >= 1.0f)
            ph -= 1.0f;
        phase[k] = ph;
        auto g = amplitude[k];
        if (fk > fade)
            g *= (cut - fk) / (cut - fade);
        acc += g * s.wrapped(ph);
    }
    // the additive sum is peaky: a gain follower and a tanh, as in the Modular
    const auto y = std::tanh(acc * norm * follower * 1.6f);
    peak = std::max(peak, std::abs(y));
    if (++counter >= 256)
    {
        counter = 0;
        const auto target = std::min(1.0f, 0.9f / peak);
        follower += (target - follower) * (target < follower ? 0.5f : 0.02f);
        peak = 1.0e-6f;
    }
    return y;
}

// ---- PULSAR (Curtis Roads) -------------------------------------------------------------------

void PulsarSource::prepare(float sampleRate) noexcept
{
    rate = sampleRate;
    reset();
}

void PulsarSource::reset() noexcept
{
    phase = 0.0;
    grains = {};
    next = 0;
}

float PulsarSource::render(float frequency, float formantMacro, float shape, float window) noexcept
{
    const auto f0 = clampf(frequency, 5.0f, 0.5f * rate);
    // the pulsaret's inner frequency, 0.1x .. 8x the pitch, on a log scale
    const auto formant = 0.1f * std::pow(80.0f, clampf(formantMacro, 0.0f, 1.0f));
    phase += static_cast<double>(f0) / rate;
    if (phase >= 1.0)
    {
        phase -= std::floor(phase);
        auto& g = grains[next % grains.size()];
        g.active = true;
        g.position = 0.0f;
        g.duration = rate / clampf(f0 * formant, 5.0f, 0.45f * rate);
        ++next;
    }
    const auto sh = clampf(shape, 0.0f, 1.0f);
    const auto w = clampf(window, 0.0f, 1.0f);
    float out = 0.0f;
    for (auto& g : grains)
    {
        if (!g.active)
            continue;
        const auto t = g.position / g.duration;
        if (t >= 1.0f)
        {
            g.active = false;
            continue;
        }
        // one sine cycle -> 2-3 cycles with a third partial (the pulsaret)
        const auto cycles = 1.0f + sh * 2.0f;
        const auto carrier = std::sin(twoPi * t * cycles) + sh * 0.3f * std::sin(3.0f * twoPi * t * cycles);
        // the window: rectangular -> Hann -> exponential decay (percussive)
        float env;
        if (w < 0.4f)
        {
            const auto flat = 1.0f - w / 0.4f;
            const auto edge = 0.5f - 0.49f * flat;
            env = t < edge ? 0.5f - 0.5f * std::cos(pi * t / edge)
                : (t > 1.0f - edge ? 0.5f - 0.5f * std::cos(pi * (1.0f - t) / edge) : 1.0f);
        }
        else
        {
            const auto hann = 0.5f - 0.5f * std::cos(twoPi * t);
            const auto expo = (1.0f - std::exp(-40.0f * t)) * std::exp(-4.0f * t) * (1.0f - t);
            env = hann + (expo - hann) * ((w - 0.4f) / 0.6f);
        }
        out += carrier * env;
        g.position += 1.0f;
    }
    return std::tanh(out * 1.4f);
}

// ---- STRING (Karplus-Strong, Jaffe & Smith) -------------------------------------------------

void StringSource::prepare(float sampleRate, std::uint64_t seedValue) noexcept
{
    rate = sampleRate;
    seed = seedValue | 1u;
    reset();
}

void StringSource::reset() noexcept
{
    buffer.fill(0.0f);
    write = 0;
    lowpass = allpassIn = allpassOut = 0.0f;
    random = seed;
}

float StringSource::white() noexcept
{
    return whiteFrom(random);
}

void StringSource::excite(float velocity, float frequency, float position) noexcept
{
    // fill the loop with noise, shaped by a comb at the pluck position (a mode
    // whose node falls there is not excited)
    const auto f = clampf(frequency, 15.0f, 0.45f * rate);
    const auto delay = std::clamp(static_cast<int>(rate / f - 0.5f), 2, static_cast<int>(capacity) - 3);
    const auto lag = std::max(1, static_cast<int>(clampf(position, 0.02f, 0.5f) * static_cast<float>(delay)));
    float previous = 0.0f;
    for (int i = 0; i < delay; ++i)
    {
        const auto n = white();
        const auto shaped = 0.5f * (n - previous);
        previous = n;
        const auto comb = i >= lag ? 1.0f : static_cast<float>(i) / static_cast<float>(lag);
        const auto index = (write + capacity - static_cast<std::size_t>(delay) + static_cast<std::size_t>(i)) % capacity;
        buffer[index] += clampf(velocity, 0.0f, 1.0f) * shaped * comb * 1.8f;
    }
}

float StringSource::render(float frequency, float damping, float /*position*/, float sustain) noexcept
{
    const auto f0 = clampf(frequency, 15.0f, 0.45f * rate);
    const auto a = 0.05f + (1.0f - clampf(damping, 0.0f, 1.0f)) * 0.9f;  // the loop's loss filter
    // the loss filter delays the fundamental by ~(1 - a) / a samples; taken
    // out of the loop so the string stays in tune (measured 2026-10-22: -4.4
    // cents at 196 Hz without it - the Modular's STRING has the same offset)
    const auto loop = rate / f0 - (1.0f - a) / a;
    const auto whole = std::clamp(static_cast<int>(loop - 0.5f), 2, static_cast<int>(capacity) - 3);
    const auto fraction = loop - static_cast<float>(whole);
    const auto c = (1.0f - fraction) / (1.0f + fraction);
    const auto feedback = 0.86f + clampf(sustain, 0.0f, 1.0f) * 0.139f;  // 0.86 .. 0.999
    const auto delayed = buffer[(write + capacity - static_cast<std::size_t>(whole)) % capacity];
    lowpass += a * (delayed - lowpass);
    const auto ap = c * lowpass + allpassIn - c * allpassOut;
    allpassIn = lowpass;
    allpassOut = ap;
    buffer[write] = std::tanh(feedback * ap);
    write = (write + 1) % capacity;
    return ap;
}

// ---- MATTER (modal) -------------------------------------------------------------------------

void MatterSource::prepare(float sampleRate, std::uint64_t seedValue) noexcept
{
    rate = sampleRate;
    seed = seedValue | 1u;
    reset();
}

void MatterSource::reset() noexcept
{
    bank = {};
    burst = strength = 0.0f;
    lastF = -1.0f;
    random = seed;
}

float MatterSource::white() noexcept
{
    return whiteFrom(random);
}

void MatterSource::excite(float velocity) noexcept
{
    burst = 1.0f;
    strength = clampf(velocity, 0.0f, 1.0f);
}

void MatterSource::update(float f0, float structure, float brightness, float damping) noexcept
{
    const auto nyquist = rate * 0.49f;
    float norm = 0.0f;
    const auto brightExp = 2.0f - brightness * 2.0f;
    const auto t60Base = 0.05f + (1.0f - damping) * 4.0f;
    constexpr float position = 0.18f;
    for (int i = 0; i < modes; ++i)
    {
        const auto n = static_cast<float>(i + 1);
        const auto f = f0 * n * (1.0f + structure * 0.055f * static_cast<float>(i));  // stiffness stretches
        auto& m = bank[static_cast<std::size_t>(i)];
        if (f >= nyquist)
        {
            m.live = false;
            continue;
        }
        const auto w = twoPi * f / rate;
        const auto t60 = t60Base / (1.0f + static_cast<float>(i) * 0.35f);
        const auto r = std::min(0.99995f, std::exp(-6.9077553f / (t60 * rate)));
        m.a1 = 2.0f * r * std::cos(w);
        m.a2 = r * r;
        const auto target = std::pow(n, -brightExp) * std::fabs(std::sin(n * pi * position));
        m.b0 = target * std::sin(w);
        m.live = true;
        norm = std::max(norm, target);
    }
    if (norm > 0.0f)
        for (auto& m : bank)
            m.b0 /= norm;
    lastF = f0;
    lastStructure = structure;
    lastBrightness = brightness;
    lastDamping = damping;
}

float MatterSource::render(float frequency, float structure, float brightness, float damping) noexcept
{
    const auto f0 = clampf(frequency, 10.0f, rate * 0.48f);
    const auto s = clampf(structure, 0.0f, 1.0f), b = clampf(brightness, 0.0f, 1.0f), d = clampf(damping, 0.0f, 1.0f);
    if (std::abs(f0 - lastF) > 0.01f * lastF || std::abs(s - lastStructure) > 1.0e-3f
        || std::abs(b - lastBrightness) > 1.0e-3f || std::abs(d - lastDamping) > 1.0e-3f)
        update(f0, s, b, d);
    float excitation = 0.0f;
    if (burst > 1.0e-4f)
    {
        excitation = strength * 6.0f * white() * burst;
        burst *= 0.9985f;  // a burst of ~10 ms
        if (burst < 2.0e-4f)
            burst = 0.0f;
    }
    float y = 0.0f;
    for (auto& m : bank)
    {
        if (!m.live)
            continue;
        const auto v = m.a1 * m.z1 - m.a2 * m.z2 + m.b0 * excitation;
        m.z2 = m.z1;
        m.z1 = v;
        y += v;
    }
    return std::tanh(y * 0.5f);
}

// ---- FORMANT (filter mode) ---------------------------------------------------------------

namespace
{
// sung vowels, bass voice: frequency (Hz), gain (dB re F1), bandwidth (Hz)
// (Fant 1960; the Csound manual's formant tables) - A E I O U
constexpr float formantFrequency[FormantFilter::vowels][FormantFilter::bands] {
    { 600.0f, 1040.0f, 2250.0f, 2450.0f, 2750.0f },
    { 400.0f, 1620.0f, 2400.0f, 2800.0f, 3100.0f },
    { 250.0f, 1750.0f, 2600.0f, 3050.0f, 3340.0f },
    { 400.0f, 750.0f, 2400.0f, 2600.0f, 2900.0f },
    { 350.0f, 600.0f, 2400.0f, 2675.0f, 2950.0f } };
constexpr float formantGainDb[FormantFilter::vowels][FormantFilter::bands] {
    { 0.0f, -7.0f, -9.0f, -9.0f, -20.0f },
    { 0.0f, -12.0f, -9.0f, -12.0f, -18.0f },
    { 0.0f, -30.0f, -16.0f, -22.0f, -28.0f },
    { 0.0f, -11.0f, -21.0f, -20.0f, -40.0f },
    { 0.0f, -20.0f, -32.0f, -28.0f, -36.0f } };
constexpr float formantBandwidth[FormantFilter::vowels][FormantFilter::bands] {
    { 60.0f, 70.0f, 110.0f, 120.0f, 130.0f },
    { 40.0f, 80.0f, 100.0f, 120.0f, 120.0f },
    { 60.0f, 90.0f, 100.0f, 120.0f, 120.0f },
    { 40.0f, 80.0f, 100.0f, 120.0f, 120.0f },
    { 40.0f, 80.0f, 100.0f, 120.0f, 120.0f } };
}

void FormantFilter::reset() noexcept
{
    ic1.fill(0.0f);
    ic2.fill(0.0f);
}

void FormantFilter::control(float vowel, float tract, float resonance, float sampleRate) noexcept
{
    const auto position = clampf(vowel, 0.0f, 1.0f) * static_cast<float>(vowels - 1);
    const auto segment = std::min(vowels - 2, static_cast<int>(position));
    const auto t = position - static_cast<float>(segment);
    const auto shift = clampf(tract, 0.25f, 4.0f);
    const auto narrow = 1.0f + clampf(resonance, 0.0f, 1.0f) * 8.0f;
    const auto top = 0.45f * sampleRate;
    for (int b = 0; b < bands; ++b)
    {
        const auto i = static_cast<std::size_t>(b);
        const auto fa = formantFrequency[segment][b], fb = formantFrequency[segment + 1][b];
        const auto f = clampf(std::exp(std::log(fa) * (1.0f - t) + std::log(fb) * t) * shift, 20.0f, top);
        const auto width = (formantBandwidth[segment][b] * (1.0f - t) + formantBandwidth[segment + 1][b] * t) / narrow;
        const auto db = formantGainDb[segment][b] * (1.0f - t) + formantGainDb[segment + 1][b] * t;
        k[i] = clampf(width * shift / f, 0.02f, 2.0f);  // the widths scale with the tract
        g[i] = std::tan(pi * f / sampleRate);
        // a band-pass of damping k peaks at 1/k and passes a band ~ k wide.
        // x sqrt(k) keeps the energy of a broad spectrum constant as RES
        // narrows the bands (x k, unit peak, as in the Modular, lost ~30 dB
        // and more with RES: measured 2026-10-22); the table's gain sets the
        // vowel's colour
        gain[i] = std::pow(10.0f, db / 20.0f) * std::sqrt(k[i]);
    }
}

float FormantFilter::process(float x) noexcept
{
    // the Modular's soft saturation inside the loop: a bounded resonator
    const auto saturate = [](float v) {
        if (v > 1.0f) return 1.0f + std::tanh(v - 1.0f);
        if (v < -1.0f) return -1.0f + std::tanh(v + 1.0f);
        return v;
    };
    float out = 0.0f;
    for (std::size_t b = 0; b < static_cast<std::size_t>(bands); ++b)
    {
        const auto a1 = 1.0f / (1.0f + g[b] * (g[b] + k[b]));
        const auto v1 = a1 * (ic1[b] + g[b] * (x - ic2[b]));
        const auto v2 = ic2[b] + g[b] * v1;
        ic1[b] = 2.0f * saturate(v1) - ic1[b];
        ic2[b] = 2.0f * saturate(v2) - ic2[b];
        out += v1 * gain[b];
    }
    return out;
}

// ---- CHAOS ------------------------------------------------------------------
void ChaosSource::prepare(float sampleRate) noexcept
{
    rate = sampleRate;
    dcCoefficient = std::exp(-2.0f * 3.14159265f * 12.0f / rate);
    powerCoefficient = 1.0f - std::exp(-1.0f / (rate * 0.080f));
    reset();
}

void ChaosSource::reset() noexcept
{
    x = 1.0;
    v = 0.0;
    phase = 0.0;
    dcIn = dcOut = 0.0f;
    power = 0.09f;  // the target level: the follower starts at unity gain
}

float ChaosSource::render(float frequency, float forcing, float damping, float colour) noexcept
{
    const auto f = clampf(frequency, 15.0f, 0.45f * rate);
    const auto gamma = 0.15 + 0.55 * static_cast<double>(clampf(forcing, 0.0f, 1.0f));
    const auto delta = 0.08 + 0.42 * static_cast<double>(clampf(damping, 0.0f, 1.0f));
    // one forcing cycle per note period: the dimensionless step is 2 pi f / rate;
    // RK4 in sub-steps of at most 0.15
    constexpr double twoPi = 6.283185307179586;
    const auto h = twoPi * static_cast<double>(f) / static_cast<double>(rate);
    const auto steps = std::max(1, static_cast<int>(std::ceil(h / 0.15)));
    const auto dt = h / steps;
    const auto acceleration = [gamma, delta](double px, double pv, double t) {
        return -delta * pv + px - px * px * px + gamma * std::cos(t);
    };
    for (int i = 0; i < steps; ++i)
    {
        const auto k1x = v, k1v = acceleration(x, v, phase);
        const auto k2x = v + 0.5 * dt * k1v, k2v = acceleration(x + 0.5 * dt * k1x, v + 0.5 * dt * k1v, phase + 0.5 * dt);
        const auto k3x = v + 0.5 * dt * k2v, k3v = acceleration(x + 0.5 * dt * k2x, v + 0.5 * dt * k2v, phase + 0.5 * dt);
        const auto k4x = v + dt * k3v, k4v = acceleration(x + dt * k3x, v + dt * k3v, phase + dt);
        x += dt / 6.0 * (k1x + 2.0 * k2x + 2.0 * k3x + k4x);
        v += dt / 6.0 * (k1v + 2.0 * k2v + 2.0 * k3v + k4v);
        phase += dt;
    }
    if (phase >= twoPi)
        phase -= twoPi * std::floor(phase / twoPi);
    if (!std::isfinite(x) || !std::isfinite(v) || std::abs(x) > 10.0 || std::abs(v) > 20.0)
    {
        x = 1.0;
        v = 0.0;
    }
    const auto c = clampf(colour, 0.0f, 1.0f);
    const auto raw = static_cast<float>(x) * (1.0f - c) + static_cast<float>(v) * c * 0.8f;
    // a well is an offset: the DC block keeps the motion, not the well
    dcOut = raw - dcIn + dcCoefficient * dcOut;
    dcIn = raw;
    // the level follower: towards an RMS of 0.3, boost at most 8x, cut 0.2x
    power += (dcOut * dcOut - power) * powerCoefficient;
    const auto level = std::clamp(0.3f / std::sqrt(std::max(power, 1.0e-6f)), 0.2f, 8.0f);
    return std::clamp(dcOut * level, -1.5f, 1.5f);
}

// ---- FEEDBACK ----------------------------------------------------------------
void FeedbackSource::prepare(float sampleRate, std::uint64_t value) noexcept
{
    rate = sampleRate;
    seed = value ^ 0xbb67ae8584caa73bull;
    dcCoefficient = std::exp(-2.0f * 3.14159265f * 12.0f / rate);
    reset();
}

void FeedbackSource::reset() noexcept
{
    buffer.fill(0.0f);
    write = 0;
    lowpass = dcIn = dcOut = 0.0f;
    lastFrequency = lastColour = lastDetune = -1.0f;
    random = seed;
}

void FeedbackSource::excite(float velocity, float frequency) noexcept
{
    // one period of noise in the loop: the loop's own gain does the rest
    const auto f = clampf(frequency, 15.0f, 0.45f * rate);
    const auto period = std::clamp(static_cast<int>(rate / f), 2, static_cast<int>(capacity) - 3);
    for (int i = 0; i < period; ++i)
    {
        const auto index = (write + capacity - static_cast<std::size_t>(period) + static_cast<std::size_t>(i)) % capacity;
        buffer[index] += clampf(velocity, 0.0f, 1.0f) * 0.4f * whiteFrom(random);
    }
}

float FeedbackSource::tap(float delay) const noexcept
{
    const auto d = std::clamp(delay, 1.0f, static_cast<float>(capacity - 3));
    const auto whole = static_cast<std::size_t>(d);
    const auto fraction = d - static_cast<float>(whole);
    const auto a = buffer[(write + capacity - whole) % capacity];
    const auto b = buffer[(write + capacity - whole - 1) % capacity];
    return a + (b - a) * fraction;
}

float FeedbackSource::render(float frequency, float gain, float detune, float colour) noexcept
{
    const auto f = clampf(frequency, 15.0f, 0.45f * rate);
    const auto c = clampf(colour, 0.0f, 1.0f);
    const auto d = clampf(detune, 0.0f, 1.0f);
    if (f != lastFrequency || c != lastColour || d != lastDetune)
    {
        lastFrequency = f;
        lastColour = c;
        lastDetune = d;
        // the low-pass sits between 1.5x and 16x the note. At the fundamental
        // its loss (and the second tap's) is compensated, so GAIN is the loop
        // gain there, and its phase (and the DC block's) is taken out of the
        // delay, so the loop is in tune
        const auto cutoff = clampf(f * (1.5f + 14.5f * c), 20.0f, 0.45f * rate);
        k = 1.0f - std::exp(-2.0f * 3.14159265f * cutoff / rate);
        const auto w = 2.0f * 3.14159265f * f / rate;
        const auto pole = 1.0f - k;
        const auto lowpassMagnitude = k / std::sqrt(1.0f - 2.0f * pole * std::cos(w) + pole * pole);
        const auto lowpassDelay = std::atan2(pole * std::sin(w), 1.0f - pole * std::cos(w));
        const auto r = dcCoefficient;
        const auto dcLead = std::atan2(std::sin(w), 1.0f - std::cos(w))
            - std::atan2(r * std::sin(w), 1.0f - r * std::cos(w));
        loopDelay = std::max(1.0f, rate / f - (lowpassDelay - dcLead) / w);
        // the second tap, 0.5 d of a period later, weighted 0.5 d
        const auto a = 0.5f * d, lag = w * 0.5f * d * loopDelay;
        const auto real = 1.0f - a + a * std::cos(lag), imaginary = -a * std::sin(lag);
        const auto tapsMagnitude = std::sqrt(real * real + imaginary * imaginary);
        // at most 4x: where the two taps cancel the fundamental, the loop
        // sings in another mode instead (DETUNE's instability)
        compensation = 1.0f / std::max(lowpassMagnitude * tapsMagnitude, 0.25f);
    }
    const auto main = tap(loopDelay);
    const auto loop = d > 0.0f ? main + (tap(loopDelay * (1.0f + 0.5f * d)) - main) * 0.5f * d : main;
    lowpass += (loop - lowpass) * k;
    dcOut = lowpass - dcIn + dcCoefficient * dcOut;
    dcIn = lowpass;
    const auto loopGain = (1.03f + 1.5f * clampf(gain, 0.0f, 1.0f)) * compensation;
    // a whisper of noise keeps it alive however it was left
    auto y = std::tanh(loopGain * dcOut + 1.0e-4f * whiteFrom(random));
    if (!std::isfinite(y))
        y = 0.0f;
    buffer[write] = y;
    write = (write + 1) % capacity;
    return y * 0.5f;
}
}
