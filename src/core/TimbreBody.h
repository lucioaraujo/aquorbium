// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include "core/BiomaModalResonator.h"

#include <array>
#include <cstddef>
#include <cstdint>

namespace aquorbium
{
// Bodies an organism can sing with (2026-10-20, the author: "só há sons
// sintéticos, não há variação de timbres, precisamos de módulos que
// trabalhem essa variação sonora"; "não consigo mudar os timbres").
//
// Until now every species had one fixed DSP body. A TimbreBody is a second
// body, chosen per organism and mixed with the native one. Each is written
// here from published theory (no third-party code), each with an underwater
// or organic character, and each with one MATTER control that moves through
// its own range:
//
//   voice   - a glottal source through three vocal formants; MATTER walks the
//             vowels [u]-[ɔ]-[ɑ]-[ɛ]-[i] (Peterson & Barney 1952). With the glides of
//             the ecological music it sings like a whale.
//   matter  - the project's modal resonator; MATTER moves its modes from
//             harmonic (wood, a tube) to the inharmonic ratios of a membrane,
//             longer and brighter.
//   breath  - a blown tube, heard as its breath: noise through the
//             resonances of a tube at the first three harmonics, with a soft
//             tone at the fundamental (corrected 2026-10-21: this comment once
//             promised a jet waveguide that the code never had - the reed
//             below is the waveguide); MATTER adds air and widens the
//             resonances.
//   string  - a waveguide string (Karplus & Strong 1983; Jaffe & Smith 1983);
//             MATTER goes from plucked to a noise-excited sustain (not a real
//             bow: that is the next body).
//   bow     - a bowed string (McIntyre, Schumacher & Woodhouse 1983; Smith,
//             Physical Audio Signal Processing, "Bowed strings"): the string
//             as two round-trip delays on either side of the bow, the bow
//             coupled to it by a smooth stick-slip
//             friction curve - it sticks while the string moves with it and
//             slips when the difference of velocities grows - so the
//             Helmholtz motion arises from the physics. MATTER is where the
//             bow plays: sul tasto (far from the bridge, sweet) to sul
//             ponticello (near it, bright), with the pressure rising as it
//             nears the bridge (Schelleng 1973: the minimum bow force grows
//             there; too little makes the string slip twice per period).
//   reed    - a single reed on a tube closed at the reed and open at the
//             bell, so only odd harmonics resonate (a clarinet's register):
//             a delay of half a period, an inverting, lossy bell, and the
//             reed as a reflection that depends on the pressure across it
//             (Smith, PASP, "Single-reed instruments"; McIntyre et al. 1983).
//             MATTER is the embouchure: soft and dark to bright and buzzy.
//   drop    - drops of water: each is a sine whose pitch rises as it decays
//             (Minnaert 1933; van den Doel 2005, "Physically based models for
//             liquid sounds"); MATTER goes from single drops to rain.
//   fold    - a sine folded on itself (the west-coast "timbre" of Buchla's
//             259, as in RASGO_MODULAR's SHAPE, rewritten); MATTER folds more.
//   membrane - a struck membrane: six modes at the ratios of a circular
//             membrane (ratios of zeros of the Bessel functions J0..J3:
//             Rayleigh, The Theory of Sound), struck by a soft mallet; its pitch sags just
//             after the strike (tension modulation). MATTER goes from a loose,
//             muffled water drum to a tight, ringing skin.
//   glass   - a rubbed glass (the glass harmonica): a few inharmonic partials,
//             each a pair of close sines that beat, swelling slowly while the
//             organism has energy (Rossing, The Science of Sound; the ratios
//             differ from glass to glass - Jundt et al. 2006 measured 1:2.37
//             and 1:2.52 - and ours are one plausible glass). MATTER
//             brightens it and quickens the beating.
//   choir   - three voices, a little out of tune with each other and with
//             their own vibratos, through the same vowel formants (the
//             chorus effect of a group singing). MATTER walks the vowels.
//
// Fixed memory, no allocation, lock or I/O. A change of body fades out and
// in over ~15 ms, so it never clicks.
class TimbreBody final
{
public:
    enum class Kind : std::uint8_t { native, voice, matter, breath, string, drop, fold, membrane, glass, choir, bow, reed };
    static constexpr std::size_t kindCount = 12;
    // the persisted encoding of a body choice: index / kindScale (room for 16)
    static constexpr float kindScale = 15.0f;
    static constexpr std::size_t delayCapacity = 4096;  // >= 48 kHz / 20 Hz, x2 rates covered by clamp
    static constexpr std::size_t dropCapacity = 6;

    void prepare(double sampleRate, std::uint32_t seed) noexcept;
    void reset() noexcept;
    void setKind(Kind kind) noexcept;
    // without the crossfade, for a silent body (the synthesis device's
    // slots, 2026-10-22): an onset right after the change must not be lost
    // to the fade, or a struck body stays mute for its first note
    void setKindNow(Kind kind) noexcept;
    [[nodiscard]] Kind kind() const noexcept { return target; }

    // control rate (~200 Hz): pitch and character
    void control(float frequencyHz, float matter) noexcept;
    // a note onset of this organism
    void excite(float velocity) noexcept;
    // per sample: `drive` is the organism's energy (0 = silent, the
    // sustained bodies stop blowing/bowing); ~unit level for drive 1
    [[nodiscard]] float render(float frequencyHz, float drive) noexcept;

    [[nodiscard]] static const char* name(Kind kind) noexcept;

private:
    struct Svf
    {
        float g = 0.0f, k = 1.0f, a1 = 0.0f, a2 = 0.0f, a3 = 0.0f;
        float ic1 = 0.0f, ic2 = 0.0f;
        void set(float frequency, float q, float sampleRate) noexcept;
        float bandPass(float input) noexcept;
    };

    struct Drop
    {
        float phase = 0.0f;
        float frequency = 0.0f;
        float rise = 0.0f;       // per-sample multiplicative pitch rise
        float amplitude = 0.0f;
        float decay = 0.0f;      // per-sample multiplier
    };

    [[nodiscard]] float nextNoise() noexcept;
    [[nodiscard]] float readDelay(float delaySamples) const noexcept;
    void writeDelay(float value) noexcept;
    void spawnDrop(float frequencyHz, float velocity) noexcept;
    [[nodiscard]] float renderKind(Kind which, float frequencyHz, float drive) noexcept;
    void clearState() noexcept;

    double rate = 48000.0;
    float sampleRateF = 48000.0f;
    std::uint32_t seed = 0x7b1dbu;
    std::uint32_t noiseState = 0x7b1dbu;
    Kind active = Kind::native;
    Kind target = Kind::native;
    float fade = 1.0f;
    float fadeStep = 1.0f / 720.0f;
    float matterValue = 0.5f;
    float frequencyValue = 110.0f;

    // voice
    float glottalPhase = 0.0f;
    float vibratoPhase = 0.0f;
    std::array<Svf, 3> formants {};
    std::array<float, 3> formantGain { 1.0f, 0.6f, 0.3f };
    // matter
    BiomaModalResonator modal;
    // breath and string share one delay line
    std::array<float, delayCapacity> delay {};
    std::size_t writeIndex = 0;
    float loopState = 0.0f;
    float lastOut = 0.0f;
    float pluckEnergy = 0.0f;
    float dcIn = 0.0f, dcOut = 0.0f;
    // drop
    std::array<Drop, dropCapacity> drops {};
    float rainClock = 0.0f;
    // fold
    float foldPhase = 0.0f;
    // membrane: six two-pole resonators, a mallet and a pitch sag
    static constexpr std::size_t membraneModes = 6;
    std::array<float, membraneModes> m1 {}, m2 {};
    float malletEnergy = 0.0f;
    float malletState = 0.0f;
    float sag = 0.0f;
    // glass: four partials, each a beating pair; a slow swell
    std::array<float, 8> glassPhase {};
    float swell = 0.0f;
    // choir: three glottal sources
    std::array<float, 3> choirPhase {};
    std::array<float, 3> choirVibrato {};
    // bow: the bridge side uses `delay`, the nut side its own line
    std::array<float, delayCapacity> nutDelay {};
    std::size_t nutWrite = 0;
    float bridgeLoss = 0.0f;   // the bridge's lowpass state
    float bowBody = 0.0f;      // a gentle lowpass on the output
    float bowAttack = 0.0f;    // the bow speeds up at a note onset
    // reed: the bell's two-point average, and the tone that radiates
    float bellState = 0.0f;
    float radiation = 0.0f;
    // the waveguides' fractional delays: first-order allpass interpolation
    // (Jaffe & Smith 1983), so the loop keeps its harmonics at every pitch -
    // linear interpolation is a lowpass whose strength depends on the
    // fraction (measured 2026-10-21: the reed's third harmonic went from
    // -13 dB at 110 Hz to -57 dB at 220 Hz)
    struct Allpass
    {
        float x1 = 0.0f, y1 = 0.0f;
        float process(float x, float c) noexcept
        {
            const auto y = c * x + x1 - c * y1;
            x1 = x;
            y1 = y;
            return y;
        }
    };
    Allpass bridgeAllpass {}, nutAllpass {}, boreAllpass {};
    // reads a line `delaySamples` back through an allpass (integer part
    // direct, fraction 0.1..1.1 in the allpass)
    [[nodiscard]] float readAllpass(const std::array<float, delayCapacity>& line, std::size_t write,
                                    float delaySamples, Allpass& allpass) const noexcept;
    void writeNut(float value) noexcept;
};
}
