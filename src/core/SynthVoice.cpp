// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/SynthVoice.h"

#include <algorithm>
#include <cmath>

namespace aquorbium
{
namespace
{
constexpr float twoPi = 6.28318530717958647692f;
constexpr float pi = 3.14159265358979323846f;
// the drive's curve: linear up to 1, then 1 + tanh(u - 1) (odd), and its
// antiderivative (even): u^2/2 inside, 1/2 + (|u| - 1) + ln cosh(|u| - 1)
// outside, ln cosh(z) written as z + log1p(exp(-2z)) - ln 2 for any z >= 0
double driveShape(double u) noexcept
{
    if (u > 1.0) return 1.0 + std::tanh(u - 1.0);
    if (u < -1.0) return -1.0 + std::tanh(u + 1.0);
    return u;
}
double driveAntiderivative(double u) noexcept
{
    const auto a = std::abs(u);
    if (a <= 1.0) return 0.5 * u * u;
    const auto z = a - 1.0;
    return 0.5 + z + (z + std::log1p(std::exp(-2.0 * z)) - 0.6931471805599453);
}
// FORMANT's level (measured 2026-10-22, SynthVoiceTests): with it a saw
// through the vowels sits -7..+2 dB from the same saw through the low-pass
// at 2 kHz, and noise ~+8 dB (only the harmonics near the formants pass)
constexpr float formantLevel = 2.5f;

float clampf(float v, float lo, float hi) noexcept
{
    return v < lo ? lo : (v > hi ? hi : v);
}

// PolyBLEP residual at a discontinuity of height 2 (Valimaki & Huovilainen;
// Martin Finke) - as in the Rasgo Modular's OSC
double polyBlep(double t, double dt) noexcept
{
    if (dt <= 0.0 || dt >= 0.5)
        return 0.0;
    if (t < dt)
    {
        t /= dt;
        return t + t - t * t - 1.0;
    }
    if (t > 1.0 - dt)
    {
        t = (t - 1.0) / dt;
        return t * t + t + t + 1.0;
    }
    return 0.0;
}

double wrap01(double t) noexcept
{
    return t - std::floor(t);
}

// the envelope's curve: < 0.5 convex (slow start), 0.5 linear, > 0.5
// concave (fast start) - the Rasgo Modular's ENVELOPE
float shapeCurve(float x, float curve) noexcept
{
    const auto cx = clampf(x, 0.0f, 1.0f);
    const auto exponent = curve < 0.5f ? 1.0f + (0.5f - curve) * 6.0f : 1.0f / (1.0f + (curve - 0.5f) * 6.0f);
    return std::pow(cx, exponent);
}
}

// ---- the envelope: a gate-driven ADSR with a continuous curve --------------------------

float SynthVoice::Stage::advance(const Envelope& e, bool gateNow, float sampleRate) noexcept
{
    switch (step)
    {
        case Step::idle:
            value = 0.0f;
            break;
        case Step::attack:
            phase += 1.0f / std::max(1.0f, e.attack * sampleRate);
            if (phase >= 1.0f)
            {
                value = 1.0f;
                step = Step::decay;
                phase = 0.0f;
                start = 1.0f;
            }
            else
                value = start + (1.0f - start) * shapeCurve(phase, e.curve);
            break;
        case Step::decay:
        {
            phase += 1.0f / std::max(1.0f, e.decay * sampleRate);
            const auto target = clampf(e.sustain, 0.0f, 1.0f);
            if (phase >= 1.0f)
            {
                value = target;
                step = gateNow && target > 1.0e-4f ? Step::sustain : Step::release;
                phase = 0.0f;
                start = value;
            }
            else
                value = target + (start - target) * (1.0f - shapeCurve(phase, e.curve));
            break;
        }
        case Step::sustain:
            value = clampf(e.sustain, 0.0f, 1.0f);
            break;
        case Step::release:
            phase += 1.0f / std::max(1.0f, e.release * sampleRate);
            if (phase >= 1.0f)
            {
                value = 0.0f;
                step = Step::idle;
            }
            else
                value = start * (1.0f - shapeCurve(phase, e.curve));
            break;
    }
    return value;
}

// ---- the oscillator: one shape control morphs sine -> tri -> saw -> pulse ----------------

float SynthVoice::Oscillator::render(double increment, float shape, float width, float sub) noexcept
{
    const auto dt = std::fabs(increment);
    const auto p = static_cast<float>(phase);
    const auto sine = std::sin(twoPi * p);
    const auto tri = p < 0.5f ? 4.0f * p - 1.0f : 3.0f - 4.0f * p;
    auto saw = 2.0f * p - 1.0f;
    saw -= static_cast<float>(polyBlep(phase, dt));
    const auto pw = clampf(width, 0.02f, 0.98f);
    auto pulse = p < pw ? 1.0f : -1.0f;
    pulse += static_cast<float>(polyBlep(phase, dt));
    pulse -= static_cast<float>(polyBlep(wrap01(phase + 1.0 - static_cast<double>(pw)), dt));
    // the morph: three segments between neighbouring shapes
    const auto s = clampf(shape, 0.0f, 1.0f) * 3.0f;
    float out;
    if (s < 1.0f)
        out = sine + (tri - sine) * s;
    else if (s < 2.0f)
        out = tri + (saw - tri) * (s - 1.0f);
    else
        out = saw + (pulse - saw) * (s - 2.0f);
    if (sub > 0.0f)
    {
        const auto sdt = dt * 0.5;
        auto square = static_cast<float>(subPhase) < 0.5f ? 1.0f : -1.0f;
        square += static_cast<float>(polyBlep(subPhase, sdt));
        square -= static_cast<float>(polyBlep(wrap01(subPhase + 0.5), sdt));
        out += square * clampf(sub, 0.0f, 1.0f);
    }
    phase = wrap01(phase + increment);
    subPhase = wrap01(subPhase + increment * 0.5);
    return out;
}

// ---- noise: white -> pink (Kellet) -> brown by one colour control -------------------------

float SynthVoice::NoiseSource::render(float white, float colour) noexcept
{
    // Paul Kellet's refined pink filter (music-dsp, public domain)
    b0 = 0.99886f * b0 + white * 0.0555179f;
    b1 = 0.99332f * b1 + white * 0.0750759f;
    b2 = 0.96900f * b2 + white * 0.1538520f;
    b3 = 0.86650f * b3 + white * 0.3104856f;
    b4 = 0.55000f * b4 + white * 0.5329522f;
    b5 = -0.7616f * b5 - white * 0.0168980f;
    const auto pink = (b0 + b1 + b2 + b3 + b4 + b5 + b6 + white * 0.5362f) * 0.11f;
    b6 = white * 0.115926f;
    // brown: a leaky integrator of white noise
    brown = clampf(0.995f * brown + white * 0.1f, -4.0f, 4.0f);
    const auto brownOut = brown * 0.35f;
    const auto c = clampf(colour, 0.0f, 1.0f) * 2.0f;
    return c < 1.0f ? white + (pink - white) * c : pink + (brownOut - pink) * (c - 1.0f);
}

// ---- the voice --------------------------------------------------------------------------------

void SynthVoice::prepare(double sampleRate, std::uint32_t seedValue) noexcept
{
    rate = std::isfinite(sampleRate) && sampleRate >= 8000.0 ? static_cast<float>(sampleRate) : 48000.0f;
    seed = seedValue == 0 ? 0x5a17e5u : seedValue;
    driftInterval = static_cast<std::uint32_t>(std::max(1.0f, rate / 40.0f));
    reset();
}

void SynthVoice::reset() noexcept
{
    random = 0x9e3779b97f4a7c15ull ^ seed;
    std::uint64_t slotSeed = 0x2545f4914f6cdd1dull ^ seed;
    for (auto* slot : { &slotA, &slotB })
    {
        slot->osc = {};
        slot->noise = {};
        slot->fm.prepare(rate);
        slot->fm4.prepare(rate);
        slot->chaos.prepare(rate);
        slot->feedback.prepare(rate, slotSeed ^ 0x53u);
        slot->wavetable.prepare(rate, slotSeed);
        slot->additive.prepare(rate);
        slot->pulsar.prepare(rate);
        slot->string.prepare(rate, slotSeed ^ 0x51u);
        slot->matter.prepare(rate, slotSeed ^ 0x52u);
        slot->body.prepare(rate, static_cast<std::uint32_t>(slotSeed));
        slot->body.setKind(TimbreBody::Kind::native);
        slot->bodyKind = 0;
        slotSeed = slotSeed * 0x9e3779b97f4a7c15ull + 1u;
    }
    ampStage = {};
    modStage = {};
    gateOpen = false;
    accent = 1.0f;
    lfo1Phase = lfo2Phase = 0.0f;
    lfo1Held = lfo2Held = lfo1Target = lfo2Target = 0.0f;
    driftState = 0.0f;
    driftCounter = 0;
    lastB = 0.0f;
    ic1 = ic2 = 0.0f;
    formant.reset();
    driveLast = 0.0;
    controlCounter = 0;
    pendingExcite = false;
}

void SynthVoice::noteOn(float accentValue) noexcept
{
    accent = clampf(accentValue, 0.0f, 1.0f);
    gateOpen = true;
    // the struck and plucked sources are excited by the note - in the next
    // render, when the note's frequency is known
    pendingExcite = true;
    for (auto* stage : { &ampStage, &modStage })
    {
        stage->step = Stage::Step::attack;
        stage->phase = 0.0f;
        stage->start = stage->value;  // retrigger from where it is: no click
    }
}

void SynthVoice::noteOff() noexcept
{
    gateOpen = false;
    for (auto* stage : { &ampStage, &modStage })
        if (stage->step != Stage::Step::idle && stage->step != Stage::Step::release)
        {
            stage->step = Stage::Step::release;
            stage->phase = 0.0f;
            stage->start = stage->value;
        }
}

float SynthVoice::nextWhite() noexcept
{
    // xorshift64*
    random ^= random >> 12;
    random ^= random << 25;
    random ^= random >> 27;
    const auto r = random * 0x2545f4914f6cdd1dull;
    return static_cast<float>(static_cast<double>(r >> 11) / 9007199254740992.0) * 2.0f - 1.0f;
}

float SynthVoice::lfoValue(float& phase, float& held, float& target, const Lfo& lfo) noexcept
{
    const auto increment = clampf(lfo.rate, 0.0f, 40.0f) / rate;
    phase += increment;
    if (phase >= 1.0f)
    {
        phase -= std::floor(phase);
        held = target;
        target = nextWhite();
    }
    switch (lfo.shape)
    {
        case LfoShape::sine: return std::sin(twoPi * phase);
        case LfoShape::triangle: return phase < 0.5f ? 4.0f * phase - 1.0f : 3.0f - 4.0f * phase;
        case LfoShape::square: return phase < 0.5f ? 1.0f : -1.0f;
        case LfoShape::wander:
        {
            // a smooth random: a raised-cosine glide from one random value to the next
            const auto t = 0.5f - 0.5f * std::cos(pi * phase);
            return held + (target - held) * t;
        }
    }
    return 0.0f;
}

void SynthVoice::clearSlot(Slot& slot) noexcept
{
    slot.osc = {};
    slot.noise = {};
    slot.fm.reset();
    slot.fm4.reset();
    slot.chaos.reset();
    slot.feedback.reset();
    slot.wavetable.reset();
    slot.additive.reset();
    slot.pulsar.reset();
    slot.string.reset();
    slot.matter.reset();
    slot.body.reset();
}

void SynthVoice::syncBody(Slot& slot, std::uint8_t bodyKind, bool onset) noexcept
{
    const auto kind = static_cast<std::uint8_t>(std::clamp<int>(bodyKind, 1, static_cast<int>(TimbreBody::kindCount) - 1));
    if (kind == slot.bodyKind)
        return;
    slot.bodyKind = kind;
    // at a note onset or in a silent voice it changes at once (the attack
    // covers it); a sounding voice crossfades
    if (onset || ampStage.step == Stage::Step::idle)
        slot.body.setKindNow(static_cast<TimbreBody::Kind>(kind));
    else
        slot.body.setKind(static_cast<TimbreBody::Kind>(kind));
}

float SynthVoice::source(Source which, Slot& slot, double increment, float shape, float width, float sub,
                         float colour, std::uint8_t bodyKind, float native) noexcept
{
    const auto frequency = static_cast<float>(std::fabs(increment)) * rate;
    switch (which)
    {
        case Source::off: return 0.0f;
        case Source::oscillator: return slot.osc.render(increment, shape, width, sub);
        case Source::noise: return slot.noise.render(nextWhite(), colour);
        case Source::native: return native;
        case Source::fm: return slot.fm.render(frequency, shape, width, colour);
        case Source::fm4: return slot.fm4.render(frequency, shape, width, colour);
        case Source::chaos: return slot.chaos.render(frequency, shape, width, colour);
        case Source::feedback: return slot.feedback.render(frequency, shape, width, colour);
        case Source::wavetable: return slot.wavetable.render(frequency, shape, width, colour);
        case Source::additive: return slot.additive.render(frequency, shape, width, colour);
        case Source::pulsar: return slot.pulsar.render(frequency, shape, width, colour);
        case Source::string: return slot.string.render(frequency, shape, width, colour);
        case Source::matter: return slot.matter.render(frequency, shape, width, colour);
        case Source::body:
        {
            // the organisms' bodies (TimbreBody): which one, and FORMA as its matter
            syncBody(slot, bodyKind, false);
            if (controlCounter == 0)
                slot.body.control(frequency, shape);
            return slot.body.render(frequency, gateOpen ? 1.0f : ampStage.value);
        }
    }
    return 0.0f;
}

float SynthVoice::render(float frequencyHz, float native) noexcept
{
    const auto& p = patch;
    const auto amp = ampStage.advance(p.amp, gateOpen, rate);
    const auto mod = modStage.advance(p.mod, gateOpen, rate);
    auto lfoA = p.lfo1;
    auto lfoB = p.lfo2;
    lfoA.rate = effective.lfo1Rate;
    lfoB.rate = effective.lfo2Rate;
    const auto lfo1 = lfoValue(lfo1Phase, lfo1Held, lfo1Target, lfoA);
    const auto lfo2 = lfoValue(lfo2Phase, lfo2Held, lfo2Target, lfoB);
    // silent between notes: the amp envelope has ended and no note is due.
    // The output would be zero anyway (amp = 0), so the sources and the
    // filter are not computed (measured 2026-10-22: the voices sound ~25-37 %
    // of the time). The filter state is cleared so the next note starts clean.
    if (ampStage.step == Stage::Step::idle && !gateOpen && !pendingExcite)
    {
        ic1 = ic2 = 0.0f;
        formant.reset();
        driveLast = 0.0;
        return 0.0f;
    }

    // the tuning breathes: a slow correlated random walk (the Modular's drift)
    if (++driftCounter >= driftInterval)
    {
        driftCounter = 0;
        driftState = clampf(driftState + nextWhite() * 0.0006f * p.drift * p.drift, -0.045f, 0.045f);
    }
    const auto nyquist = 0.45f * rate;
    if (controlCounter == 0)
    {
        applyMatrix(amp, mod, lfo1, lfo2);
        base = clampf(std::isfinite(frequencyHz) ? frequencyHz : 110.0f, 2.0f, nyquist)
            * std::exp2(driftState + p.vibrato * lfo1 / 1200.0f + effective.pitch / 12.0f);
        fA = clampf(base * std::exp2(p.tuneA / 12.0f + p.fineA / 1200.0f), 2.0f, nyquist);
        fB = clampf(base * std::exp2(p.tuneB / 12.0f + p.fineB / 1200.0f), 2.0f, nyquist);
        // the filter: its cutoff follows the note (keytrack), the mod
        // envelope and LFO 2
        const auto cutoff = clampf(p.cutoff * std::pow(base / 261.63f, clampf(p.keytrack, 0.0f, 1.0f))
                                       * std::exp2(p.filterEnv * mod + p.wah * lfo2 + effective.cutoffOctaves),
                                   20.0f, nyquist);
        g = std::tan(pi * cutoff / rate);
        k = 2.0f - 1.96f * clampf(effective.resonance, 0.0f, 1.0f);
        // FORMANT: the same cutoff is the vocal tract (1000 Hz = the table's
        // voice), so its envelope, key tracking, wah and routes still play
        if (p.filterMode == FilterMode::formant)
            formant.control(effective.vowel, cutoff / 1000.0f, effective.resonance, rate);
        // FM 4-OP: the algorithm and the ratios of C and D
        slotA.fm4.configure(p.algorithmA, p.ratioCA, p.ratioDA);
        slotB.fm4.configure(p.algorithmB, p.ratioCB, p.ratioDB);
    }
    controlCounter = (controlCounter + 1) & 7u;
    if (pendingExcite)
    {
        pendingExcite = false;
        const auto excite = [this](Source which, Slot& slot, float frequency, float width) {
            switch (which)
            {
                case Source::string: slot.string.excite(0.4f + 0.6f * accent, frequency, 0.02f + 0.48f * width); break;
                case Source::matter: slot.matter.excite(0.4f + 0.6f * accent); break;
                case Source::feedback: slot.feedback.excite(0.4f + 0.6f * accent, frequency); break;
                case Source::body:
                    // the kind first: a struck body excited before its kind
                    // was set stayed mute (found by the PROPOSTA tests)
                    syncBody(slot, &slot == &slotA ? patch.bodyA : patch.bodyB, true);
                    slot.body.control(frequency, &slot == &slotA ? effective.shapeA : effective.shapeB);
                    slot.body.excite(0.4f + 0.6f * accent);
                    break;
                default: break;
            }
        };
        excite(p.sourceA, slotA, fA, effective.widthA);
        excite(p.sourceB, slotB, fB, effective.widthB);
    }

    // B first: it can modulate A's frequency (linear, through-zero FM)
    const auto& e = effective;
    const auto b = source(p.sourceB, slotB, static_cast<double>(fB) / rate, e.shapeB, e.widthB, p.subB,
                          e.colourB, p.bodyB, native);
    // FM B -> A acts on the oscillating sources only. The resonant ones
    // (STRING, MATTER, BODY, FEEDBACK) keep their tuning: a resonator re-tuned every
    // sample is not energy-stable - a membrane under FM blew up to 1e37
    // (found by the PROPOSTA tests, 2026-10-22)
    const auto resonantA = p.sourceA == Source::string || p.sourceA == Source::matter || p.sourceA == Source::body
        || p.sourceA == Source::feedback;
    const auto incrementA = resonantA ? static_cast<double>(fA) / rate
                                      : (static_cast<double>(fA) + static_cast<double>(b) * e.fm * 4.0 * fA) / rate;
    auto a = source(p.sourceA, slotA, incrementA, e.shapeA, e.widthA, p.subA, e.colourA, p.bodyA, native);
    // a last guard: a source that ever leaves the finite, sane range is
    // cleared rather than allowed to poison the filter and the mix
    const auto sane = [](float y) { return std::isfinite(y) && std::abs(y) < 64.0f; };
    auto bb = b;
    if (!sane(a))
    {
        a = 0.0f;
        clearSlot(slotA);
    }
    if (!sane(bb))
    {
        bb = 0.0f;
        clearSlot(slotB);
    }
    lastB = bb;
    auto x = a * e.levelA + bb * e.levelB + a * bb * e.ring;

    // the filter: a TPT state-variable filter (Simper)
    if (e.drive > 0.0f)
    {
        // linear up to ~1, then soft - the Modular's FILTER drive - with
        // first-order antiderivative anti-aliasing (ADAA: Parker, Zavalishin
        // & Le Bivic, "Reducing the aliasing of nonlinear waveshaping using
        // continuous-time convolution", DAFx 2016). Plain, a 7 kHz note at
        // full drive folded back -18 dB of non-harmonic energy (2026-10-22)
        const auto u = static_cast<double>(x) * (1.0 + static_cast<double>(e.drive) * 6.0);
        const auto delta = u - driveLast;
        double shaped = 0.0;
        if (std::abs(delta) > 1.0e-6)
            shaped = (driveAntiderivative(u) - driveAntiderivative(driveLast)) / delta;
        else
            shaped = driveShape(0.5 * (u + driveLast));
        driveLast = u;
        x = static_cast<float>(shaped) / (1.0f + e.drive * 2.0f);
    }
    else
        driveLast = 0.0;
    const auto a1 = 1.0f / (1.0f + g * (g + k));
    const auto a2 = g * a1;
    const auto a3 = g * a2;
    const auto v3 = x - ic2;
    const auto v1 = a1 * ic1 + a2 * v3;
    const auto v2 = ic2 + a2 * ic1 + a3 * v3;
    ic1 = 2.0f * v1 - ic1;
    ic2 = 2.0f * v2 - ic2;
    float filtered = v2;
    switch (p.filterMode)
    {
        case FilterMode::lowPass: filtered = v2; break;
        case FilterMode::bandPass: filtered = v1 * k; break;  // unit gain at the centre
        case FilterMode::highPass: filtered = x - k * v1 - v2; break;
        case FilterMode::notch: filtered = x - k * v1; break;
        case FilterMode::formant: filtered = formant.process(x) * formantLevel; break;
    }
    if (!std::isfinite(filtered))
    {
        ic1 = ic2 = 0.0f;
        formant.reset();
        filtered = 0.0f;
    }
    const auto velocity = 1.0f - p.accentToLevel + p.accentToLevel * accent;
    return filtered * amp * e.level * velocity;
}

float SynthVoice::sourceValue(ModSource which, float amp, float mod, float lfo1, float lfo2) const noexcept
{
    switch (which)
    {
        case ModSource::none: return 0.0f;
        case ModSource::ampEnvelope: return amp;
        case ModSource::modEnvelope: return mod;
        case ModSource::lfo1: return lfo1;
        case ModSource::lfo2: return lfo2;
        case ModSource::accent: return accent;
        case ModSource::energy: return ecology.energy;
        case ModSource::speed: return ecology.speed;
        case ModSource::relation: return ecology.relation;
        case ModSource::tension: return ecology.tension;
        case ModSource::distance: return ecology.distance;
        case ModSource::channel: return ecology.channel;
        case ModSource::temperature: return ecology.temperature;
        case ModSource::hunger: return ecology.hunger;
        case ModSource::age: return ecology.age;
        case ModSource::prey: return ecology.prey;
        case ModSource::habitat: return ecology.habitat;
        case ModSource::room: return ecology.room;
    }
    return 0.0f;
}

void SynthVoice::applyMatrix(float amp, float mod, float lfo1, float lfo2) noexcept
{
    const auto& p = patch;
    std::array<float, modDestinationCount> offset {};
    for (const auto& route : p.routes)
    {
        if (route.source == ModSource::none || route.destination == ModDestination::none || route.amount == 0.0f)
            continue;
        const auto value = sourceValue(route.source, amp, mod, lfo1, lfo2);
        offset[static_cast<std::size_t>(route.destination)] += value * clampf(route.amount, -1.0f, 1.0f);
    }
    const auto at = [&offset](ModDestination d) { return offset[static_cast<std::size_t>(d)]; };
    using D = ModDestination;
    auto& e = effective;
    e.pitch = at(D::pitch) * 12.0f;
    e.cutoffOctaves = at(D::cutoff) * 4.0f;
    e.resonance = clampf(p.resonance + at(D::resonance), 0.0f, 1.0f);
    e.drive = clampf(p.drive + at(D::drive), 0.0f, 1.0f);
    e.shapeA = clampf(p.shapeA + at(D::shapeA), 0.0f, 1.0f);
    e.shapeB = clampf(p.shapeB + at(D::shapeB), 0.0f, 1.0f);
    // 0..1: LARGURA is the pulse width for the oscillator (limited to
    // 0.02..0.98 inside it) and another control for the other sources (FM's
    // index, the warp...) whose zero must stay zero
    e.widthA = clampf(p.widthA + at(D::widthA), 0.0f, 1.0f);
    e.widthB = clampf(p.widthB + at(D::widthB), 0.0f, 1.0f);
    e.levelA = clampf(p.levelA + at(D::levelA), 0.0f, 1.0f);
    e.levelB = clampf(p.levelB + at(D::levelB), 0.0f, 1.0f);
    e.ring = clampf(p.ring + at(D::ring), 0.0f, 1.0f);
    e.fm = clampf(p.fm + at(D::fm), 0.0f, 1.0f);
    e.colourA = clampf(p.colourA + at(D::colourA), 0.0f, 1.0f);
    e.colourB = clampf(p.colourB + at(D::colourB), 0.0f, 1.0f);
    e.level = clampf(p.level * (1.0f + at(D::level)), 0.0f, 2.0f);
    e.lfo1Rate = clampf(p.lfo1.rate * std::exp2(at(D::lfo1Rate) * 3.0f), 0.0f, 40.0f);
    e.lfo2Rate = clampf(p.lfo2.rate * std::exp2(at(D::lfo2Rate) * 3.0f), 0.0f, 40.0f);
    e.vowel = clampf(p.vowel + at(D::vowel), 0.0f, 1.0f);
}
}
