// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/TimbreBody.h"

#include "core/Numeric.h"

#include <algorithm>
#include <cmath>

namespace aquorbium
{
namespace
{
constexpr float twoPi = 6.28318530717958647692f;
constexpr float pi = 3.14159265358979323846f;

// average formant frequencies of five vowels (men, Peterson & Barney 1952):
// [u], [ɔ], [ɑ], [ɛ], [i]; the bandwidths are typical values
constexpr std::array<std::array<float, 3>, 5> vowelFormants {{
    { 300.0f, 870.0f, 2240.0f },
    { 570.0f, 840.0f, 2410.0f },
    { 730.0f, 1090.0f, 2440.0f },
    { 530.0f, 1840.0f, 2480.0f },
    { 270.0f, 2290.0f, 3010.0f }
}};
constexpr std::array<float, 3> formantBandwidth { 70.0f, 100.0f, 140.0f };

// level calibration per kind: first ~0.25 RMS alone at drive 1 (the unit
// tests), then matched in the engine to the native bodies over a season with
// the ecological music (2026-10-20: voice -2.0, matter -2.5, breath -4.0,
// string -12.2, drop -5.2, fold +2.4, membrane -3.3, glass +0.8, choir -1.7 dB
// before; the string stays ~5 dB under so a pluck's peak stays bounded)
constexpr std::array<float, TimbreBody::kindCount> kindGain {
    0.0f, 1.39f, 53.0f, 2.05f, 2.00f, 2.00f, 0.30f, 0.26f, 0.41f, 1.71f, 2.09f, 0.44f
};

// the circular membrane's first modes relative to the fundamental (ratios of
// the zeros of the Bessel functions, Rayleigh)
constexpr std::array<float, 6> membraneRatios { 1.0f, 1.593f, 2.135f, 2.295f, 2.653f, 2.917f };
constexpr std::array<float, 6> membraneWeights { 1.0f, 0.75f, 0.55f, 0.45f, 0.30f, 0.22f };
// a rubbed glass's partials (approximate (m,0) bending modes of one wine
// glass; real glasses differ - Jundt et al. 2006)
constexpr std::array<float, 4> glassRatios { 1.0f, 2.32f, 4.25f, 6.63f };
// three voices of a choir: detuning in cents and vibrato rates
constexpr std::array<float, 3> choirCents { -9.0f, 0.0f, 11.0f };
constexpr std::array<float, 3> choirRates { 4.7f, 5.3f, 5.9f };

float polyBlep(float phase, float increment) noexcept
{
    if (phase < increment)
    {
        const auto t = phase / increment;
        return t + t - t * t - 1.0f;
    }
    if (phase > 1.0f - increment)
    {
        const auto t = (phase - 1.0f) / increment;
        return t * t + t + t + 1.0f;
    }
    return 0.0f;
}
} // namespace

void TimbreBody::Svf::set(float frequency, float q, float sampleRate) noexcept
{
    const auto f = std::clamp(frequency, 20.0f, sampleRate * 0.45f);
    g = std::tan(pi * f / sampleRate);
    k = 1.0f / std::max(0.5f, q);
    a1 = 1.0f / (1.0f + g * (g + k));
    a2 = g * a1;
    a3 = g * a2;
}

float TimbreBody::Svf::bandPass(float input) noexcept
{
    // the trapezoidal state-variable filter (Zavalishin, The Art of VA Filter
    // Design; Simper/Cytomic 2013); k * band is unit gain at the centre
    const auto v3 = input - ic2;
    const auto v1 = a1 * ic1 + a2 * v3;
    const auto v2 = ic2 + a2 * ic1 + a3 * v3;
    ic1 = 2.0f * v1 - ic1;
    ic2 = 2.0f * v2 - ic2;
    return k * v1;
}

const char* TimbreBody::name(Kind kind) noexcept
{
    switch (kind)
    {
        case Kind::native: return "nativo";
        case Kind::voice: return "voz";
        case Kind::matter: return "matéria";
        case Kind::breath: return "sopro";
        case Kind::string: return "corda";
        case Kind::drop: return "gota";
        case Kind::fold: return "dobra";
        case Kind::membrane: return "membrana";
        case Kind::glass: return "vidro";
        case Kind::choir: return "coro";
        case Kind::bow: return "arco";
        case Kind::reed: return "palheta";
    }
    return "?";
}

void TimbreBody::prepare(double sampleRate, std::uint32_t seedValue) noexcept
{
    rate = numeric::sampleRate(sampleRate);
    sampleRateF = static_cast<float>(rate);
    seed = seedValue == 0u ? 0x7b1dbu : seedValue;
    fadeStep = static_cast<float>(1.0 / (rate * 0.015));
    modal.prepare(rate);
    modal.setSeed(seed ^ 0x51ed27u);
    reset();
}

void TimbreBody::reset() noexcept
{
    noiseState = seed;
    clearState();
    active = target;
    fade = 1.0f;
}

void TimbreBody::clearState() noexcept
{
    glottalPhase = vibratoPhase = 0.0f;
    for (auto& f : formants)
        f.ic1 = f.ic2 = 0.0f;
    modal.reset();
    delay.fill(0.0f);
    writeIndex = 0;
    loopState = lastOut = pluckEnergy = 0.0f;
    dcIn = dcOut = 0.0f;
    drops = {};
    rainClock = 0.0f;
    foldPhase = 0.0f;
    m1.fill(0.0f);
    m2.fill(0.0f);
    malletEnergy = malletState = sag = 0.0f;
    glassPhase.fill(0.0f);
    swell = 0.0f;
    choirPhase = { 0.0f, 0.37f, 0.71f };
    choirVibrato = { 0.0f, 0.33f, 0.66f };
    nutDelay.fill(0.0f);
    nutWrite = 0;
    bridgeLoss = bowBody = bowAttack = 0.0f;
    bellState = 0.0f;
    radiation = 0.0f;
    bridgeAllpass = {};
    nutAllpass = {};
    boreAllpass = {};
}

void TimbreBody::setKind(Kind kind) noexcept
{
    target = kind;
}

void TimbreBody::setKindNow(Kind kind) noexcept
{
    target = active = kind;
    clearState();
    fade = 1.0f;
    control(frequencyValue, matterValue);
}

float TimbreBody::nextNoise() noexcept
{
    noiseState ^= noiseState << 13u;
    noiseState ^= noiseState >> 17u;
    noiseState ^= noiseState << 5u;
    return static_cast<float>(noiseState & 0xffffffu) / 8388608.0f - 1.0f;
}

void TimbreBody::control(float frequencyHz, float matter) noexcept
{
    frequencyValue = numeric::clampFinite(frequencyHz, 10.0f, sampleRateF * 0.45f, frequencyValue);
    matterValue = numeric::unit(matter, matterValue);
    const auto kind = active == Kind::native ? target : active;
    if (kind == Kind::voice || kind == Kind::breath || kind == Kind::choir)
    {
        if (kind == Kind::voice || kind == Kind::choir)
        {
            // the vowels, walked continuously
            const auto position = matterValue * 4.0f;
            const auto lower = std::min(3, static_cast<int>(position));
            const auto t = position - static_cast<float>(lower);
            for (std::size_t i = 0; i < 3; ++i)
            {
                const auto f = vowelFormants[static_cast<std::size_t>(lower)][i] * (1.0f - t)
                    + vowelFormants[static_cast<std::size_t>(lower + 1)][i] * t;
                formants[i].set(f, f / formantBandwidth[i], sampleRateF);
            }
        }
        else
        {
            // a tube's resonances at the first three harmonics; air (MATTER)
            // widens them and the tone becomes breath
            const auto q = 60.0f - matterValue * 52.0f;
            for (std::size_t i = 0; i < 3; ++i)
                formants[i].set(frequencyValue * static_cast<float>(i + 1), q, sampleRateF);
        }
    }
    if (kind == Kind::matter)
    {
        BiomaModalResonator::Parameters p;
        p.fundamentalHz = frequencyValue;
        p.material = matterValue;                              // wood .. glass .. metal
        p.decaySeconds = 0.25f + matterValue * matterValue * 2.2f;
        p.brightness = 0.30f + matterValue * 0.60f;
        p.hardness = 0.35f + matterValue * 0.50f;
        p.strikePosition = 0.31f;
        modal.setParameters(p);
    }
}

void TimbreBody::spawnDrop(float frequencyHz, float velocity) noexcept
{
    // the free slot, or the quietest drop
    std::size_t slot = 0;
    for (std::size_t i = 1; i < drops.size(); ++i)
        if (drops[i].amplitude < drops[slot].amplitude)
            slot = i;
    auto& d = drops[slot];
    // a drop rings two octaves or so above the organism's note, never lower
    // than 300 Hz (a ~1 cm bubble) nor higher than 7 kHz
    d.frequency = std::clamp(frequencyHz * std::exp2(2.0f + nextNoise() * 0.6f), 300.0f, 7000.0f);
    const auto tau = 0.012f + (nextNoise() * 0.5f + 0.5f) * 0.04f;
    d.decay = std::exp(-1.0f / (tau * sampleRateF));
    // the rising pitch of a drop (van den Doel 2005): ~ +50 % over its life
    d.rise = std::exp(std::log(1.5f) / (3.0f * tau * sampleRateF));
    d.amplitude = std::clamp(velocity, 0.0f, 1.0f) * 0.8f;
    d.phase = 0.0f;
}

void TimbreBody::excite(float velocity) noexcept
{
    const auto v = std::clamp(velocity, 0.0f, 1.0f);
    switch (active)
    {
        case Kind::matter: modal.excite(v); break;
        case Kind::string: pluckEnergy = std::max(pluckEnergy, v); break;
        case Kind::bow: bowAttack = std::max(bowAttack, v); break;
        case Kind::membrane:
            malletEnergy = std::max(malletEnergy, v);
            sag = 1.0f;
            break;
        case Kind::drop:
            spawnDrop(frequencyValue, v);
            if (matterValue > 0.5f)
                spawnDrop(frequencyValue, v * 0.6f);
            break;
        default: break;
    }
}

float TimbreBody::readDelay(float delaySamples) const noexcept
{
    const auto d = std::clamp(delaySamples, 1.0f, static_cast<float>(delayCapacity - 2));
    auto position = static_cast<float>(writeIndex) - d;
    if (position < 0.0f)
        position += static_cast<float>(delayCapacity);
    const auto index = static_cast<std::size_t>(position);
    const auto fraction = position - static_cast<float>(index);
    const auto a = delay[index % delayCapacity];
    const auto b = delay[(index + 1) % delayCapacity];
    return a + (b - a) * fraction;
}

void TimbreBody::writeDelay(float value) noexcept
{
    delay[writeIndex] = value;
    writeIndex = (writeIndex + 1) % delayCapacity;
}

float TimbreBody::readAllpass(const std::array<float, delayCapacity>& line, std::size_t write, float delaySamples,
                              Allpass& allpass) const noexcept
{
    const auto d = std::clamp(delaySamples, 1.2f, static_cast<float>(delayCapacity - 3));
    const auto whole = static_cast<std::size_t>(std::floor(d - 0.1f));
    const auto fraction = d - static_cast<float>(whole);
    const auto c = (1.0f - fraction) / (1.0f + fraction);
    const auto x = line[(write + delayCapacity - whole) % delayCapacity];
    return allpass.process(x, c);
}

void TimbreBody::writeNut(float value) noexcept
{
    nutDelay[nutWrite] = value;
    nutWrite = (nutWrite + 1) % delayCapacity;
}

float TimbreBody::renderKind(Kind which, float frequencyHz, float drive) noexcept
{
    const auto f = numeric::clampFinite(frequencyHz, 10.0f, sampleRateF * 0.45f, frequencyValue);
    switch (which)
    {
        case Kind::native:
            return 0.0f;
        case Kind::voice:
        {
            // a slow, living vibrato; a band-limited glottal saw with a breath
            vibratoPhase += 5.2f / sampleRateF;
            vibratoPhase -= std::floor(vibratoPhase);
            const auto pitch = f * (1.0f + 0.0045f * std::sin(twoPi * vibratoPhase));
            const auto increment = std::min(0.5f, pitch / sampleRateF);
            glottalPhase += increment;
            glottalPhase -= std::floor(glottalPhase);
            const auto source = 2.0f * glottalPhase - 1.0f - polyBlep(glottalPhase, increment)
                + nextNoise() * 0.04f;
            float out = 0.0f;
            for (std::size_t i = 0; i < 3; ++i)
                out += formants[i].bandPass(source) * formantGain[i];
            return out;
        }
        case Kind::matter:
            return modal.tick();
        case Kind::breath:
        {
            // breath noise through the tube's resonances, with a soft tone at
            // the fundamental that thins as the air grows
            glottalPhase += f / sampleRateF;
            glottalPhase -= std::floor(glottalPhase);
            const auto air = nextNoise();
            float out = 0.0f;
            for (std::size_t i = 0; i < 3; ++i)
                out += formants[i].bandPass(air) * (i == 0 ? 1.0f : 0.5f / static_cast<float>(i));
            return out * (1.5f + matterValue) + std::sin(twoPi * glottalPhase) * 0.25f * (1.0f - matterValue);
        }
        case Kind::string:
        {
            // Karplus-Strong: a loop one period long, averaged (the string's
            // losses); plucked by a burst, or bowed by a continuous friction
            const auto period = sampleRateF / f;
            const auto a = readDelay(period - 0.5f);
            const auto b = readDelay(period + 0.5f);
            const auto damping = 0.996f - (1.0f - matterValue) * 0.004f;
            auto input = 0.0f;
            if (pluckEnergy > 1.0e-4f)
            {
                input += nextNoise() * pluckEnergy;
                pluckEnergy *= std::exp(-1.0f / std::max(8.0f, period));
            }
            if (matterValue > 0.5f)
                input += nextNoise() * (matterValue - 0.5f) * 0.12f * std::clamp(drive * 4.0f, 0.0f, 1.0f);
            const auto y = 0.5f * (a + b) * damping + input;
            writeDelay(std::clamp(y, -2.0f, 2.0f));
            // DC blocker
            const auto out = y - dcIn + 0.995f * dcOut;
            dcIn = y;
            dcOut = out;
            return out;
        }
        case Kind::drop:
        {
            // rain: more drops as MATTER grows, while the organism has energy
            if (matterValue > 0.2f && drive > 0.02f)
            {
                rainClock += matterValue * matterValue * 18.0f / sampleRateF * std::min(1.0f, drive * 3.0f);
                if (rainClock >= 1.0f)
                {
                    rainClock = -0.4f * (nextNoise() * 0.5f + 0.5f);
                    spawnDrop(f, 0.35f + (nextNoise() * 0.5f + 0.5f) * 0.45f);
                }
            }
            float out = 0.0f;
            for (auto& d : drops)
            {
                if (d.amplitude < 1.0e-5f)
                    continue;
                out += d.amplitude * std::sin(twoPi * d.phase);
                d.phase += d.frequency / sampleRateF;
                d.phase -= std::floor(d.phase);
                d.frequency = std::min(d.frequency * d.rise, sampleRateF * 0.45f);
                d.amplitude *= d.decay;
            }
            return out;
        }
        case Kind::membrane:
        {
            // the mallet: a short, soft burst (low-passed noise)
            auto mallet = 0.0f;
            if (malletEnergy > 1.0e-4f)
            {
                malletState += (nextNoise() - malletState) * 0.25f;
                mallet = malletState * malletEnergy;
                malletEnergy *= 0.92f;
            }
            // the pitch sags after the strike, a little more on a loose skin
            sag *= 0.9995f;
            const auto bend = 1.0f + sag * (0.08f - matterValue * 0.06f);
            // a loose, muffled water drum decays fast; a tight skin rings
            const auto decaySeconds = 0.08f + matterValue * matterValue * 1.2f;
            const auto radius = std::exp(-1.0f / (decaySeconds * sampleRateF));
            float out = 0.0f;
            for (std::size_t m = 0; m < membraneModes; ++m)
            {
                const auto fm = std::min(f * membraneRatios[m] * bend, sampleRateF * 0.45f);
                const auto theta = twoPi * fm / sampleRateF;
                const auto c = 2.0f * radius * std::cos(theta);
                // an impulse rings with amplitude ~ input / sin(theta):
                // normalised, so low and high membranes speak alike
                const auto y = c * m1[m] - radius * radius * m2[m]
                    + mallet * membraneWeights[m] * std::sin(theta);
                m2[m] = m1[m];
                m1[m] = y;
                out += y;
            }
            return out;
        }
        case Kind::glass:
        {
            // the rubbing swells slowly while there is energy, and fades
            const auto target = drive > 0.01f ? 1.0f : 0.0f;
            swell += (target - swell) * (target > swell ? 0.00025f : 0.0004f);
            const auto beat = 0.4f + matterValue * 2.6f;  // Hz between each pair
            float out = 0.0f;
            for (std::size_t k = 0; k < 4; ++k)
            {
                const auto fk = f * glassRatios[k];
                if (fk > sampleRateF * 0.45f)
                    break;
                const auto weight = std::pow(0.25f + matterValue * 0.5f, static_cast<float>(k));
                for (std::size_t pair = 0; pair < 2; ++pair)
                {
                    auto& phase = glassPhase[k * 2 + pair];
                    phase += (fk + (pair == 0 ? -0.5f : 0.5f) * beat) / sampleRateF;
                    phase -= std::floor(phase);
                    out += std::sin(twoPi * phase) * weight * 0.5f;
                }
            }
            return out * swell;
        }
        case Kind::choir:
        {
            float source = 0.0f;
            for (std::size_t v = 0; v < 3; ++v)
            {
                choirVibrato[v] += choirRates[v] / sampleRateF;
                choirVibrato[v] -= std::floor(choirVibrato[v]);
                const auto pitch = f * std::exp2(choirCents[v] / 1200.0f)
                    * (1.0f + 0.005f * std::sin(twoPi * choirVibrato[v]));
                const auto increment = std::min(0.5f, pitch / sampleRateF);
                choirPhase[v] += increment;
                choirPhase[v] -= std::floor(choirPhase[v]);
                source += 2.0f * choirPhase[v] - 1.0f - polyBlep(choirPhase[v], increment);
            }
            source = source * 0.45f + nextNoise() * 0.05f;
            float out = 0.0f;
            for (std::size_t i = 0; i < 3; ++i)
                out += formants[i].bandPass(source) * formantGain[i];
            return out;
        }
        case Kind::fold:
        {
            // a sine folded on itself; the fold depth is limited by the pitch
            // so the harmonics stay below Nyquist
            foldPhase += f / sampleRateF;
            foldPhase -= std::floor(foldPhase);
            const auto depth = std::min(1.0f + matterValue * 5.0f, std::max(1.0f, sampleRateF * 0.18f / f));
            return std::sin(0.5f * pi * depth * std::sin(twoPi * foldPhase));
        }
        case Kind::bow:
        {
            // the period split at the bow: 1/8 towards the bridge, 7/8
            // towards the nut, each a round trip that inverts at its end
            // MATTER moves the bow from sul tasto (beta 0.18: far from the
            // bridge, sweet) to sul ponticello (0.08: near it, bright and
            // glassy), with the pressure Schelleng's diagram asks for there
            const auto beta = 0.18f - 0.10f * matterValue;
            const auto period = sampleRateF / f;
            // the bridge loses the highs on each reflection; its one-pole
            // (coefficient 0.55) delays the low partials by (1 - 0.55) / 0.55
            // = 0.82 samples, taken out of the line, less 0.15 samples
            // measured at the junction, so the pitch stays true
            const auto fromBridgeRaw = -readAllpass(delay, writeIndex, period * beta - 0.668f, bridgeAllpass);
            bridgeLoss += (fromBridgeRaw - bridgeLoss) * 0.55f;
            const auto fromBridge = bridgeLoss * 0.995f;
            const auto fromNut = -readAllpass(nutDelay, nutWrite, period * (1.0f - beta), nutAllpass);
            const auto stringVelocity = fromBridge + fromNut;
            // the bow: its speed follows the organism's energy, faster at an
            // onset; its pressure (MATTER) widens the sticking region
            bowAttack *= 0.9995f;
            const auto bowVelocity = (0.12f + 0.25f * bowAttack) * std::clamp(drive, 0.0f, 1.0f);
            const auto difference = bowVelocity - stringVelocity;
            // (measured: below ~0.15 the bow slips into the octave - a
            // double slip: the minimum bow force of Schelleng's diagram)
            const auto stick = 0.15f + 0.14f * matterValue * matterValue;  // grows towards the bridge
            const auto x = difference / stick;
            const auto friction = 1.0f / (1.0f + x * x * x * x);  // ~1 sticking, ->0 slipping
            const auto injected = friction * difference;
            writeDelay(std::clamp(fromNut + injected, -2.0f, 2.0f));   // towards the bridge
            writeNut(std::clamp(fromBridge + injected, -2.0f, 2.0f));  // towards the nut
            // what the body hears: the force at the bridge, softened; the
            // level the pressure adds (+6.5 dB from tasto to ponticello,
            // measured) is taken back, so MATTER moves the colour, not the volume
            const auto level = 0.19f / (0.19f + 0.22f * matterValue * matterValue);
            bowBody += (fromBridge * level - bowBody) * 0.45f;
            const auto out = bowBody - dcIn + 0.995f * dcOut;
            dcIn = bowBody;
            dcOut = out;
            return out;
        }
        case Kind::reed:
        {
            // a tube closed at the reed and open at the bell: the wave makes
            // a round trip in half a period and inverts at the bell, so the
            // tone has the period of the note and only odd harmonics
            // the bell: an inverting reflection through a two-point average
            // (a zero at Nyquist); the embouchure (MATTER) lets more of the
            // direct, brighter wave through. The average's half-sample delay
            // is taken out of the line, so the pitch stays true.
            const auto dark = 1.0f - 0.7f * matterValue;
            const auto half = 0.5f * sampleRateF / f;
            const auto returning = readAllpass(delay, writeIndex, half - 0.5f * dark, boreAllpass);
            const auto average = 0.5f * (returning + bellState);
            bellState = returning;
            const auto incoming = -0.95f * ((1.0f - dark) * returning + dark * average);
            // the player's mouth: just above the reed's threshold (~0.5,
            // measured) while the organism has energy, a little more with it;
            // below, the reed stops
            const auto energy = std::clamp(drive, 0.0f, 1.0f);
            const auto mouth = energy > 0.02f ? (0.62f + 0.3f * energy) * std::min(1.0f, energy * 20.0f) : 0.0f;
            const auto blown = mouth + nextNoise() * 0.006f * energy;
            // the reed: a reflection that grows towards 1 (closed) as the
            // pressure across it grows (Smith's reflection table)
            const auto difference = incoming - blown;
            const auto reflection = std::clamp(0.7f - 0.3f * difference, -1.0f, 1.0f);
            const auto outgoing = blown + reflection * difference;
            writeDelay(std::clamp(outgoing, -2.0f, 2.0f));
            // what radiates from the bell: the embouchure (MATTER) from a
            // covered, dark tone (~500 Hz) to an open, bright one (~6 kHz),
            // outside the loop so it does not change the oscillation
            const auto cutoff = 500.0f * std::exp2(matterValue * 3.6f);
            radiation += (outgoing - radiation) * (1.0f - std::exp(-twoPi * cutoff / sampleRateF));
            const auto out = radiation - dcIn + 0.995f * dcOut;
            dcIn = radiation;
            dcOut = out;
            return out;
        }
    }
    return 0.0f;
}

float TimbreBody::render(float frequencyHz, float drive) noexcept
{
    if (active != target)
    {
        fade -= fadeStep;
        if (fade <= 0.0f)
        {
            active = target;
            clearState();
            fade = 0.0f;
            control(frequencyValue, matterValue);
        }
    }
    else if (fade < 1.0f)
        fade = std::min(1.0f, fade + fadeStep);
    if (active == Kind::native)
        return 0.0f;
    return renderKind(active, frequencyHz, drive) * kindGain[static_cast<std::size_t>(active)] * fade;
}
}
