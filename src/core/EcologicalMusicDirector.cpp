// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/EcologicalMusicDirector.h"

#include "core/Numeric.h"

#include <algorithm>
#include <cmath>

namespace aquorbium
{
namespace
{
using Strategy = Ecosystem::Strategy;
using Species = Ecosystem::Species;
using Relation = EcologicalMusicDirector::Relation;
using Act = EcologicalMusicDirector::Act;
using Phase = EcologicalMusicDirector::Phase;

constexpr std::size_t count = EcologicalMusicDirector::organismCount;

// modes from dark to bright: daylight chooses the colour (a tendency)
constexpr std::array<std::array<int, 7>, 6> modes {{
    { 0, 1, 3, 5, 7, 8, 10 },   // phrygian
    { 0, 2, 3, 5, 7, 8, 10 },   // aeolian
    { 0, 2, 3, 5, 7, 9, 10 },   // dorian
    { 0, 2, 4, 5, 7, 9, 10 },   // mixolydian
    { 0, 2, 4, 5, 7, 9, 11 },   // ionian
    { 0, 2, 4, 6, 7, 9, 11 }    // lydian
}};
// in ice the field crystallises into a lattice: five of the seven degrees
constexpr std::array<int, 5> latticeDegrees { 0, 1, 2, 4, 5 };

// the migrant's regions around the sphere: centres a fifth apart
constexpr std::array<int, 4> regionCentres { 0, 7, 2, 5 };
// without conflict the harmony drifts plagally and modally, two bars a chord,
// and the cycle ends on the fifth degree that leads home
constexpr std::array<int, 6> grazingCycle { 0, 3, 0, 5, 3, 4 };

// the ecosystem physics (Ecosystem.cpp): territory, hunt and fear distances
constexpr float territoryRadius = 1.45f;
constexpr float huntDistance = 3.8f;
constexpr float captureDistance = 0.7f;
constexpr float buzzDistance = 1.25f;
constexpr float rivalDistance = 2.2f;

// sensory consonance by interval class (after Plomp & Levelt 1965 and
// Tenney 1988): unison and octave smooth; fifth, fourth, thirds and sixths
// mid; seconds, sevenths and the tritone rough
constexpr std::array<float, 13> consonanceTable {
    1.00f, 0.05f, 0.35f, 0.70f, 0.75f, 0.80f, 0.12f, 0.90f, 0.70f, 0.75f, 0.35f, 0.08f, 1.00f
};

bool has(const Ecosystem::OrganismState& organism, Strategy strategy) noexcept
{
    return organism.strategies[0] == strategy || organism.strategies[1] == strategy;
}

bool present(const Ecosystem::OrganismState& organism) noexcept
{
    return organism.presenceAmount >= 0.1f;
}

int registerOf(const Ecosystem::OrganismState& organism) noexcept
{
    switch (organism.species)
    {
    case Species::Oscillator: return 62;
    case Species::GranularStriker: return 69;
    case Species::Resonator: return 50;
    case Species::Pulsar: return 57;
    case Species::PhysicalString: return 38;
    }
    return 55;
}

// how much each relation claims of the niche's capacity
float salienceWeight(Relation relation) noexcept
{
    switch (relation)
    {
    case Relation::hunts: return 2.6f;
    case Relation::flees: return 1.4f;
    case Relation::courts: return 1.2f;
    case Relation::rivals: return 1.0f;
    case Relation::guards: return 0.8f;
    case Relation::allied: return 0.5f;
    case Relation::indifferent: return 0.0f;
    }
    return 0.0f;
}

// which relation leads an organism's behaviour when several coexist
int priority(Relation relation) noexcept
{
    switch (relation)
    {
    case Relation::hunts: return 6;
    case Relation::flees: return 5;
    case Relation::courts: return 4;
    case Relation::rivals: return 3;
    case Relation::guards: return 2;
    case Relation::allied: return 1;
    case Relation::indifferent: return 0;
    }
    return 0;
}

bool punctual(Act act) noexcept
{
    return act == Act::capture || act == Act::modulation || act == Act::resolution
        || act == Act::phaseChange || act == Act::rupture || act == Act::bubbles
        || act == Act::departure || act == Act::freeze || act == Act::revolution || act == Act::songCycle
        || act == Act::death || act == Act::birth || act == Act::soundmark;
}

float distance3(float ax, float ay, float az, const Ecosystem::Vec3& b) noexcept
{
    const auto dx = b.x - ax, dy = b.y - ay, dz = b.z - az;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}
} // namespace

// ---- physics and harmony ----------------------------------------------------

EcologicalMusicDirector::Medium EcologicalMusicDirector::mediumFor(Phase phase) noexcept
{
    //                 glide  turb  crystal speed absorb bubbles capacity tempo legato release
    switch (phase)
    {
    // ice: a solid lattice; sound runs fast and clear, pitches are discrete
    case Phase::ice:       return { 0.004f, 0.00f, 1.00f, 3.00f, 0.25f, 0.0f,  4.0f, 0.80f, 0.35f, 0.05f };
    // cold, dense water: viscous and slow, long portamenti, sound a bit slower
    case Phase::cold:      return { 0.220f, 0.05f, 0.25f, 1.30f, 1.00f, 0.0f,  6.0f, 0.86f, 0.95f, 0.35f };
    // temperate: laminar flow, the relations are clearest
    case Phase::temperate: return { 0.090f, 0.15f, 0.00f, 1.30f, 1.00f, 0.0f,  9.0f, 1.00f, 0.70f, 0.18f };
    // turbulent: eddies - rubato, heterophony, phrases broken into cells
    case Phase::turbulent: return { 0.050f, 0.85f, 0.00f, 1.30f, 1.30f, 0.08f, 14.0f, 1.10f, 0.50f, 0.12f };
    // boiling: bubbles; a bubbly liquid is slow and opaque to sound (Wood 1930)
    case Phase::boiling:   return { 0.030f, 0.55f, 0.00f, 0.45f, 6.00f, 0.60f, 11.0f, 1.00f, 0.30f, 0.08f };
    }
    return { 0.090f, 0.15f, 0.00f, 1.30f, 1.00f, 0.0f, 9.0f, 1.00f, 0.70f, 0.18f };
}

float EcologicalMusicDirector::consonance(float semitones) noexcept
{
    const auto interval = std::fmod(std::abs(semitones), 12.0f);
    const auto lower = static_cast<std::size_t>(interval);
    const auto fraction = interval - static_cast<float>(lower);
    return consonanceTable[lower] + (consonanceTable[lower + 1u] - consonanceTable[lower]) * fraction;
}

float EcologicalMusicDirector::compatibility(Relation relation) noexcept
{
    switch (relation)
    {
    case Relation::courts: return 1.0f;
    case Relation::allied: return 0.7f;
    case Relation::indifferent: return 0.0f;
    case Relation::guards: return -0.4f;
    case Relation::rivals: return -0.6f;
    case Relation::hunts: return -0.8f;
    case Relation::flees: return -1.0f;
    }
    return 0.0f;
}

float EcologicalMusicDirector::minnaertMidi(float radiusMillimetres) noexcept
{
    const auto radius = std::max(0.05f, radiusMillimetres) / 1000.0f;
    const auto frequency = 3.26f / radius;
    return 69.0f + 12.0f * std::log2(frequency / 440.0f);
}

// Sound absorption in sea water, after Ainslie & McColm (1998, JASA
// 103:1671): two chemical relaxations (boric acid, below ~1-3 kHz; magnesium
// sulphate, above) and the viscosity of pure water; f in kHz, T in degrees C,
// S in ppt, depth 0. Fresh water (S = 0) keeps only the viscous term.
float EcologicalMusicDirector::absorptionDbPerKm(float hz, float temperatureC, float salinityPpt, float pH) noexcept
{
    const auto f = std::max(0.0f, hz) / 1000.0f;
    const auto t = std::clamp(temperatureC, -2.0f, 30.0f);  // the formula's range
    const auto s = std::clamp(salinityPpt, 0.0f, 40.0f);
    const auto f1 = 0.78f * std::sqrt(s / 35.0f) * std::exp(t / 26.0f);
    const auto f2 = 42.0f * std::exp(t / 17.0f);
    const auto f2sq = f * f;
    const auto boric = f1 > 0.0f ? 0.106f * f1 * f2sq / (f1 * f1 + f2sq) * std::exp((pH - 8.0f) / 0.56f) : 0.0f;
    const auto magnesium = 0.52f * (1.0f + t / 43.0f) * (s / 35.0f) * f2 * f2sq / (f2 * f2 + f2sq);
    const auto water = 0.00049f * f2sq * std::exp(-t / 27.0f);
    return boric + magnesium + water;
}

float EcologicalMusicDirector::maxwellSpeed(float u1, float u2, float u3, float u4, float u5, float u6,
                                           float sigma) noexcept
{
    // Box-Muller: two pairs of uniforms give three (of four) normal deviates
    constexpr float twoPi = 6.28318530717958647692f;
    const auto r1 = std::sqrt(-2.0f * std::log(std::max(u1, 1.0e-7f)));
    const auto r2 = std::sqrt(-2.0f * std::log(std::max(u3, 1.0e-7f)));
    const auto x = r1 * std::cos(twoPi * u2);
    const auto y = r1 * std::sin(twoPi * u2);
    const auto z = r2 * std::cos(twoPi * u4);
    static_cast<void>(u5);
    static_cast<void>(u6);
    return sigma * std::sqrt(x * x + y * y + z * z);
}

// Mackenzie (1981, JASA 70:807), at the surface: ~4.5 m/s per degree near 15 C
float EcologicalMusicDirector::soundSpeedMs(float temperatureC, float salinityPpt) noexcept
{
    const auto t = std::clamp(temperatureC, 0.0f, 30.0f);
    const auto s = std::clamp(salinityPpt, 0.0f, 40.0f);
    return 1448.96f + 4.591f * t - 5.304e-2f * t * t + 2.374e-4f * t * t * t
        + 1.340f * (s - 35.0f) - 1.025e-2f * t * (s - 35.0f);
}

// The aquarium is a miniature ocean: for the organisms' hearing one unit of
// distance stands for ~150 km of sea, for the timbre that reaches the
// listener ~4 km (chosen so that temperate sea water sounds as before; what
// changed is that temperature and salt now act physically)
namespace
{
constexpr float hearingKmPerUnit = 150.0f;
constexpr float timbreKmPerUnit = 4.0f;
}

float EcologicalMusicDirector::receivedLevel(float distance, float midi, float absorption, float temperatureC,
                                             float salinityPpt) noexcept
{
    const auto d = std::max(0.0f, distance);
    const auto hz = 440.0f * std::exp2((midi - 69.0f) / 12.0f);
    const auto lossDb = std::max(0.0f, absorption) * absorptionDbPerKm(hz, temperatureC, salinityPpt)
        * d * hearingKmPerUnit;
    return 1.0f / (1.0f + d) * std::pow(10.0f, -lossDb / 20.0f);
}

float EcologicalMusicDirector::liquidCutoffHz(float absorption, float waterPath, float temperatureC,
                                              float salinityPpt) noexcept
{
    // the frequency at which the path costs 3 dB (absorption rises with f):
    // bisection on a logarithmic axis between 500 Hz and 16 kHz
    const auto km = std::max(0.0f, absorption) * std::max(0.0f, waterPath) * timbreKmPerUnit;
    const auto loss = [&](float hz) { return absorptionDbPerKm(hz, temperatureC, salinityPpt) * km; };
    if (loss(16000.0f) <= 3.0f)
        return 16000.0f;
    if (loss(500.0f) >= 3.0f)
        return 500.0f;
    auto lo = std::log(500.0f), hi = std::log(16000.0f);
    for (int k = 0; k < 24; ++k)
    {
        const auto mid = 0.5f * (lo + hi);
        if (loss(std::exp(mid)) < 3.0f)
            lo = mid;
        else
            hi = mid;
    }
    return std::exp(0.5f * (lo + hi));
}

// ---- lifecycle ----------------------------------------------------------------

void EcologicalMusicDirector::prepare(double controlRateHz) noexcept
{
    controlRate = controlRateHz > 1.0 ? controlRateHz : 100.0;
    reset();
}

void EcologicalMusicDirector::setSeed(std::uint32_t value) noexcept
{
    seed = value == 0u ? 0x6ea1b10u : value;
    reset();
}

void EcologicalMusicDirector::reset() noexcept
{
    randomState = seed;
    tele = {};
    medium = mediumFor(Phase::temperate);
    stepPhase = 0.0;
    stepIndex = 0;
    regionSector = pendingSector = -1;
    sectorDwell = 0;
    lastModulationBar = 0;
    modeIndex = 3;
    peakTension = 0.0f;
    cadencePending = false;
    capacity = 3.0f;
    eventsThisBar = 0;
    ruptureSteps = fermataSteps = 0;
    wasCracked = false;
    started = false;
    voices = {};
    fronts = {};
    heard = {};
    decomposed = {};
    decomposedCount = 0;
    netForHunter.fill(false);
    netNext.fill(false);
    chorus.reset(seed ^ 0x51c0ffeeu);
    populations.reset();  // off equilibrium: the cycle is already turning
    soundmark = {};
    soundmarkPending = false;
    soundmarkCursor = 0;
    soundmarkWait = 0;
    lastSoundmarkBar = 0;
    deniedThisBar = 0;
    callReady.fill(true);
    chorusCycles.fill(0);
    spectraSet = false;
    revolutionSource = -1;
    revolutionBars = 0;
    memoryCountdown = 6u * 16u;
    memoryCursor = 0;
    memoryWait = 0;
}

void EcologicalMusicDirector::setPerformance(Performance value) noexcept
{
    perf.influence = numeric::unit(value.influence, perf.influence);
    perf.reaction = numeric::unit(value.reaction, perf.reaction);
    perf.memory = numeric::unit(value.memory, perf.memory);
    perf.water = numeric::unit(value.water, perf.water);
    perf.salinity = numeric::unit(value.salinity, perf.salinity);
}

float EcologicalMusicDirector::nextUnit() noexcept
{
    randomState ^= randomState << 13u;
    randomState ^= randomState >> 17u;
    randomState ^= randomState << 5u;
    return static_cast<float>(randomState & 0xffffffu) / 16777216.0f;
}

void EcologicalMusicDirector::beginAct(Act act) noexcept
{
    if (tele.act != act || punctual(act))
        ++tele.actCounts[static_cast<std::size_t>(act)];
    tele.act = act;
}

// Every species has its own way of singing (Messiaen's catalogue as a
// method: the song of each species as composed material). The signature is a
// coda - a rhythm, as in the sperm whale's codas (Sharma et al. 2024) - and a
// whistle - a contour, as in the dolphin's signature whistle (Janik & Sayigh
// 2013). Intelligence lengthens it.
void EcologicalMusicDirector::newSignature(Signature& signature, Species species, float intelligence) noexcept
{
    signature = {};
    const auto extra = static_cast<int>(std::lround(numeric::unit(intelligence, 0.5f) * 2.0f));
    int length = 5 + extra;
    std::array<std::uint8_t, 5> durations { 2, 2, 3, 4, 1 };
    int leap = 4;          // the widest step of the contour
    float repeat = 0.10f;  // repeated pitches: the click-like codas
    switch (species)
    {
    case Species::Pulsar:          // codas: short, regular, repeated pitches
        length = 4 + extra;  durations = { 1, 1, 2, 2, 3 };  leap = 2;  repeat = 0.45f;  break;
    case Species::Oscillator:      // the whistle: a wide contour
        length = 5 + extra;  durations = { 2, 2, 3, 4, 1 };  leap = 4;  repeat = 0.05f;  break;
    case Species::Resonator:       // long tones that outline a chord
        length = 3 + extra;  durations = { 4, 6, 8, 4, 6 };  leap = 2;  repeat = 0.10f;  break;
    case Species::PhysicalString:  // the bass: few, long
        length = 3 + extra / 2;  durations = { 4, 8, 4, 8, 6 };  leap = 4;  repeat = 0.20f;  break;
    case Species::GranularStriker: // clicks scattered in register
        length = 5 + extra;  durations = { 1, 1, 1, 2, 3 };  leap = 5;  repeat = 0.25f;  break;
    }
    signature.length = static_cast<std::uint8_t>(std::clamp(length, 3, static_cast<int>(signatureCapacity)));
    int degree = 0;
    for (std::size_t i = 0; i < signature.length; ++i)
    {
        signature.degrees[i] = static_cast<std::int8_t>(degree);
        if (nextUnit() >= repeat)
        {
            const auto size = 1 + static_cast<int>(nextUnit() * static_cast<float>(leap));
            degree += nextUnit() < 0.5f ? size : -size;
            degree = std::clamp(degree, -5, 7);
        }
        signature.steps[i] = durations[static_cast<std::size_t>(nextUnit() * 5.0f) % durations.size()];
    }
    // the coda closes: the last unit is longer
    signature.steps[signature.length - 1u] = static_cast<std::uint8_t>(signature.steps[signature.length - 1u] + 2u);
}

// The chorus (CONCEITOS_VERIFICACAO_E_APROFUNDAMENTO.md §2.1; Krause's
// temporal niche, Aihara's coupled frog choruses): what an organism heard this
// step pulls its calling clock - towards the same phase for allies and a
// hunter locking on, a quarter cycle for a courting answer, the opposite
// phase for rivals, a fleeing prey, a guard and, more gently, indifferent
// neighbours sharing the niche.
void EcologicalMusicDirector::advanceChorus(const Ecosystem& ecosystem) noexcept
{
    // one shared calling cycle (a bar) with small individual detunings, as
    // the males of one frog chorus; slow species call every second cycle.
    // Locking needs the coupling to exceed the detuning, so the clocks must
    // be close - their individuality is the detuning and the skip.
    // Oscillator, Pulsar, GranularStriker, Resonator, PhysicalString
    static constexpr std::array<float, 5> detune { 0.00f, 0.05f, -0.03f, -0.06f, 0.03f };
    for (std::size_t i = 0; i < count; ++i)
    {
        const auto species = std::min<std::size_t>(static_cast<std::size_t>(ecosystem.organism(i).species), 4);
        const auto rubato = 1.0f + (voices[i].rubato - 1.0f) * 0.3f;
        chorus.setNaturalRate(i, (1.0f + detune[species]) / (16.0f * rubato));
    }
    // the threat: those who are not hunters and perceive a hunter at work
    // call together (beaked whales overlap their vocal foraging by 98%,
    // shrinking the time a predator can hear them)
    std::array<bool, count> threatened {};
    for (std::size_t h = 0; h < count; ++h)
        for (std::size_t q = 0; q < count; ++q)
            if (h != q && tele.relation[h][q] == Relation::hunts && tele.bond[h][q] > 0.4f)
                for (std::size_t k = 0; k < count; ++k)
                    if (k != h && tele.relation[k][h] != Relation::hunts
                        && (ecosystem.perceivedBy(k, h) > 0.1f || heard[k][h].level > 0.05f))
                        threatened[k] = true;
    tele.threatened = 0;
    for (std::size_t k = 0; k < count; ++k)
        if (threatened[k])
            tele.threatened = static_cast<std::uint8_t>(tele.threatened | (1u << k));
    // the pull is continuous: proportional to what is still being heard
    const auto gain = 0.20f + perf.reaction * 0.40f;
    for (std::size_t i = 0; i < count; ++i)
        for (std::size_t j = 0; j < count; ++j)
        {
            if (i == j || heard[i][j].level < 0.01f)
                continue;
            auto offset = 0.5f;
            auto weight = 1.0f;
            switch (tele.relation[i][j])
            {
                case Relation::allied:
                case Relation::hunts: offset = 0.0f; break;
                case Relation::courts: offset = 0.25f; break;
                case Relation::indifferent: weight = 0.5f; break;
                default: break;
            }
            if (threatened[i] && threatened[j])
            {
                offset = 0.0f;  // fear synchronises
                weight = 3.0f;
            }
            chorus.hear(i, j, std::min(1.0f, heard[i][j].level * 4.0f), offset, gain * weight);
        }
    const auto fired = chorus.step();
    static constexpr std::array<std::uint8_t, 5> callEvery { 1, 1, 1, 2, 2 };
    for (std::size_t i = 0; i < count; ++i)
    {
        if (fired[i])
        {
            const auto species = std::min<std::size_t>(static_cast<std::size_t>(ecosystem.organism(i).species), 4);
            chorusCycles[i] = static_cast<std::uint8_t>(chorusCycles[i] + 1u);
            if (chorusCycles[i] % callEvery[species] == 0u)
                callReady[i] = true;
        }
        tele.chorusPhase[i] = chorus.phase(i);
    }
}

void EcologicalMusicDirector::beginRevolution(const Ecosystem& ecosystem, std::size_t source) noexcept
{
    const auto& organism = ecosystem.organism(source);
    newSong(voices[source], organism.species, std::min(1.0f, organism.intelligence + 0.2f));
    for (auto& v : voices)
        v.adopted = false;
    voices[source].adopted = true;
    voices[source].phraseEnd = 0;
    revolutionSource = static_cast<int>(source);
    revolutionBars = 24;
    beginAct(Act::revolution);
    // the first revolution gives the place its soundmark
    if (!tele.hasSoundmark)
        markPlace(voices[source].form.current());
}

// A song: two or three themes. The second answers the first (the same
// contour, a third higher, its ending changed); the third, when there is one,
// contrasts. Each theme is a phrase repeated two to four times.
void EcologicalMusicDirector::newSong(Voice& voice, Ecosystem::Species species, float intelligence) noexcept
{
    std::array<Signature, SongForm::maxThemes> themes {};
    newSignature(themes[0], species, intelligence);
    themes[1] = themes[0];
    const auto length = std::min<std::size_t>(themes[1].length, SongForm::capacity);
    for (std::size_t u = 0; u < length; ++u)
        themes[1].degrees[u] = static_cast<std::int8_t>(std::clamp(themes[1].degrees[u] + 2, -5, 7));
    if (length >= 2)
    {
        const auto last = length - 1u;
        themes[1].degrees[last] = static_cast<std::int8_t>(std::clamp(themes[1].degrees[last] - 3, -5, 7));
    }
    newSignature(themes[2], species, intelligence);
    const auto themeCount = static_cast<std::uint8_t>(intelligence > 0.5f ? 3 : 2);
    std::array<std::uint8_t, SongForm::maxThemes> repeats {};
    for (auto& r : repeats)
        r = static_cast<std::uint8_t>(2 + static_cast<int>(nextUnit() * 3.0f));
    voice.form.set(themes, themeCount, repeats);
    voice.song = voice.form.current();
    voice.phraseEnd = 0;
}

SpectralConsonance::Spectrum EcologicalMusicDirector::nativeSpectrum(Species species) noexcept
{
    using S = SpectralConsonance;
    switch (species)
    {
        case Species::Oscillator: return S::harmonic(6, 1.0f);
        case Species::Pulsar: return S::harmonic(6, 0.7f);
        case Species::GranularStriker: return S::harmonic(4, 1.4f);
        case Species::Resonator: return S::blend(S::harmonic(4, 1.0f), 0.5f, S::membrane(), 0.5f);
        case Species::PhysicalString: return S::harmonic(6, 1.0f);
    }
    return S::harmonic(6, 1.0f);
}

void EcologicalMusicDirector::setSpectrum(std::size_t organism, const SpectralConsonance::Spectrum& spectrum) noexcept
{
    if (organism < count && spectrum.count > 0)
    {
        spectraSet = true;
        spectra[organism] = spectrum;
        spectra[organism].count = static_cast<std::uint8_t>(std::min<std::size_t>(spectrum.count, 6));
        // how far the partials are from a harmonic series (0 = harmonic)
        float off = 0.0f, weight = 0.0f;
        for (std::size_t k = 0; k < spectra[organism].count; ++k)
        {
            const auto r = spectra[organism].ratio[k];
            off += spectra[organism].amplitude[k] * std::abs(r - std::round(r));
            weight += spectra[organism].amplitude[k];
        }
        inharmonicity[organism] = weight > 0.0f ? off / weight : 0.0f;
    }
}

// The scale. With harmonic bodies, the mode chosen by the light. When the
// ensemble's partials are inharmonic (membranes, glasses), the scale is born
// from them, as Sethares proposed (given a timbre, which scale?): the seven
// intervals over the centre that sound smoothest with the ensemble's own
// spectrum. Recomputed at each bar.
void EcologicalMusicDirector::updateField() noexcept
{
    float inharmonic = 0.0f;
    for (const auto v : inharmonicity)
        inharmonic += v;
    inharmonic /= static_cast<float>(count);
    const auto& mode = modes[static_cast<std::size_t>(std::clamp(modeIndex, 0, 5))];
    if (inharmonic < 0.06f)
    {
        for (std::size_t k = 0; k < 7; ++k)
            field[k] = mode[k];
        tele.bodyField = false;
    }
    else
    {
        auto ensemble = spectra[0];
        for (std::size_t i = 1; i < count; ++i)
            ensemble = SpectralConsonance::blend(ensemble, 1.0f, spectra[i], 1.0f);
        std::array<float, 12> smooth {};
        for (int k = 1; k < 12; ++k)
            smooth[static_cast<std::size_t>(k)] = SpectralConsonance::consonance(60.0f, ensemble, 60.0f + static_cast<float>(k), ensemble)
                // a light preference for the mode's own degrees: the light still colours
                + ((std::find(mode.begin(), mode.end(), k) != mode.end()) ? 0.05f : 0.0f);
        std::array<int, 11> order {};
        for (int k = 0; k < 11; ++k)
            order[static_cast<std::size_t>(k)] = k + 1;
        std::sort(order.begin(), order.end(), [&](int a, int b) {
            return smooth[static_cast<std::size_t>(a)] > smooth[static_cast<std::size_t>(b)];
        });
        std::array<int, 7> chosen { 0, order[0], order[1], order[2], order[3], order[4], order[5] };
        std::sort(chosen.begin(), chosen.end());
        field = chosen;
        tele.bodyField = true;
    }
    for (std::size_t k = 0; k < 7; ++k)
        tele.field[k] = static_cast<std::int8_t>(field[k]);
}

// ---- the tick -----------------------------------------------------------------

EcologicalMusicDirector::Output EcologicalMusicDirector::tick(const Ecosystem& ecosystem) noexcept
{
    Output out;
    if (perf.influence <= 0.0f)
        return out;

    const auto dt = static_cast<float>(1.0 / controlRate);
    if (!started)
    {
        started = true;
        for (std::size_t i = 0; i < count; ++i)
        {
            const auto& organism = ecosystem.organism(i);
            newSong(voices[i], organism.species, organism.intelligence);
            if (!spectraSet)
                spectra[i] = nativeSpectrum(organism.species);
            voices[i].lastMidi = static_cast<float>(registerOf(organism));
            voices[i].lastPresence = organism.presenceAmount;
            voices[i].rest = static_cast<std::uint16_t>(nextUnit() * 12.0f);
            voices[i].codaRhythm = static_cast<std::uint8_t>(
                CodaExchange::identityRhythm(static_cast<int>(organism.species)));
            // lives already under way, so that they do not all end together
            LifeCycle::beBorn(voices[i].life, 420.0f + 480.0f * nextUnit(), 0);
            voices[i].life.age = 0.6f * nextUnit();
            voices[i].life.energy = 0.6f + 0.2f * nextUnit();
        }
    }

    updateMedium(ecosystem, dt);
    // the populations turn slowly - two to five minutes a cycle, longer with
    // the reef's memory
    populations.setPeriod(120.0f + 180.0f * numeric::unit(perf.memory, 0.5f));
    populations.advance(dt);
    tele.prey = populations.prey();
    tele.channelAxis = SoundChannel::axisHeight(tele.temperatureC);
    tele.predators = populations.predators();

    const auto desire = ecosystem.desireCurrent();
    const auto environment = ecosystem.environmentState();
    const auto target = (64.0f + environment.daylight * 22.0f + desire.density * 14.0f - desire.calm * 8.0f)
        * medium.tempoFactor;
    tele.tempoBpm = tele.tempoBpm <= 0.0f ? target : tele.tempoBpm + (target - tele.tempoBpm) * 0.02f;

    for (std::size_t i = 0; i < count; ++i)
    {
        const auto presence = numeric::unit(ecosystem.organism(i).presenceAmount, 1.0f);
        voices[i].lastPresence += (presence - voices[i].lastPresence) * 0.05f;
        out.level[i] = voices[i].lastPresence;
    }

    stepPhase += static_cast<double>(tele.tempoBpm) / 60.0 * 4.0 / controlRate;
    while (stepPhase >= 1.0)
    {
        stepPhase -= 1.0;
        step(ecosystem, out);
        ++stepIndex;
        if (stepIndex % 16 == 0)
        {
            ++tele.bar;
            tele.notesThisBar.fill(0);
        }
    }
    return out;
}

// ---- 1. the medium: liquid states ---------------------------------------------
//
// The heat is the season's temperature plus the organisms' agitation. The
// phase changes one state at a time and only after the latent heat is paid:
// crossing a threshold starts the payment; returning below it refunds it.
// Without this, a temperature hovering at a boundary would flicker between
// states; with it, a change of state is a rare, formal event.
void EcologicalMusicDirector::updateMedium(const Ecosystem& ecosystem, float dt) noexcept
{
    float speed = 0.0f, weight = 0.0f;
    for (std::size_t i = 0; i < count; ++i)
    {
        const auto& o = ecosystem.organism(i);
        const auto v = std::sqrt(o.velocity.x * o.velocity.x + o.velocity.y * o.velocity.y + o.velocity.z * o.velocity.z);
        speed += v * o.presenceAmount;
        weight += o.presenceAmount;
    }
    // the organisms cruise at ~0.45 units/s; agitation is what exceeds that
    const auto agitation = weight > 0.0f ? std::clamp((speed / weight - 0.40f) / 0.35f, 0.0f, 1.0f) : 0.0f;
    tele.agitation += (agitation - tele.agitation) * std::min(1.0f, dt / 1.5f);
    const auto temperature = numeric::unit(ecosystem.environmentState().temperature, 0.5f);
    // WATER lets the performer warm or cool the aquarium: the phase still
    // obeys the latent heat, so the gesture is felt as a slow change of state
    const auto heat = std::clamp(0.78f * temperature + 0.22f * tele.agitation + (perf.water - 0.5f) * 0.9f,
                                 0.0f, 1.0f);
    tele.heat += (heat - tele.heat) * std::min(1.0f, dt / 2.0f);

    static constexpr std::array<float, 4> upper { 0.16f, 0.38f, 0.62f, 0.84f };
    auto band = 0;
    while (band < 4 && tele.heat >= upper[static_cast<std::size_t>(band)])
        ++band;
    auto phase = static_cast<int>(tele.phase);
    if (!started || tele.capacity <= 0.0f)
        phase = band;  // the first tick: the water is already in some state
    else if (band > phase)
    {
        const auto excess = tele.heat - upper[static_cast<std::size_t>(phase)];
        tele.latent += dt / 7.0f * (1.0f + excess * 6.0f);
    }
    else if (band < phase)
    {
        const auto deficit = upper[static_cast<std::size_t>(phase - 1)] - tele.heat;
        tele.latent -= dt / 7.0f * (1.0f + deficit * 6.0f);
    }
    else
        tele.latent *= 1.0f - std::min(1.0f, dt / 4.0f);

    if (tele.latent >= 1.0f || tele.latent <= -1.0f)
    {
        phase = std::clamp(phase + (tele.latent > 0.0f ? 1 : -1), 0, 4);
        tele.latent = 0.0f;
        beginAct(Act::phaseChange);
        callSoundmark();  // the place's motif returns after the change
        fermataSteps = 8;  // the change of state is held: half a bar of suspension
    }
    tele.phase = static_cast<Phase>(phase);

    // the medium's qualities follow the state smoothly (a phase is a
    // tendency of the water; its properties take a few seconds to settle)
    const auto wanted = mediumFor(tele.phase);
    const auto a = std::min(1.0f, dt / 1.5f);
    const auto follow = [a](float& value, float to) { value += (to - value) * a; };
    if (tele.capacity <= 0.0f)
        medium = wanted;
    follow(medium.glideSeconds, wanted.glideSeconds);
    follow(medium.turbulence, wanted.turbulence);
    follow(medium.crystal, wanted.crystal);
    // the water's temperature (the heat: ~0 to 30 C; ice below zero), and
    // the speed of sound in the liquid by Mackenzie (ice and bubbly water
    // keep their own: fast in ice, slow in bubbles - Wood)
    tele.temperatureC = tele.phase == Phase::ice ? -1.0f : std::clamp(-1.0f + tele.heat * 32.0f, 0.0f, 30.0f);
    auto liquidSpeed = wanted.soundSpeed;
    if (tele.phase != Phase::ice && tele.phase != Phase::boiling)
        liquidSpeed *= soundSpeedMs(tele.temperatureC, salinityPpt()) / soundSpeedMs(15.0f, 35.0f);
    follow(medium.soundSpeed, liquidSpeed);
    follow(medium.absorption, wanted.absorption);
    follow(medium.bubbles, wanted.bubbles);
    follow(medium.capacity, wanted.capacity);
    follow(medium.tempoFactor, wanted.tempoFactor);
    follow(medium.legato, wanted.legato);
    follow(medium.releaseSeconds, wanted.releaseSeconds);
    if (tele.capacity <= 0.0f)
        tele.capacity = capacity;
}

// ---- 2. propagation: sounds travelling through the liquid ------------------------

std::size_t EcologicalMusicDirector::emit(const Ecosystem& ecosystem, std::size_t source, float midi, float accent,
                                          bool click, bool echo) noexcept
{
    // a free slot, or the front that has travelled furthest
    std::size_t slot = 0;
    float furthest = -1.0f;
    for (std::size_t f = 0; f < fronts.size(); ++f)
    {
        if (!fronts[f].live)
        {
            slot = f;
            furthest = -1.0f;
            break;
        }
        if (fronts[f].radius > furthest)
        {
            furthest = fronts[f].radius;
            slot = f;
        }
    }
    const auto& position = ecosystem.organism(source).position;
    auto& front = fronts[slot];
    front = {};
    front.live = true;
    front.source = static_cast<std::uint8_t>(source);
    front.reached = static_cast<std::uint8_t>(1u << source);
    front.click = click;
    front.echo = echo;
    front.midi = midi;
    front.accent = accent;
    front.originX = position.x;
    front.originY = position.y;
    front.originZ = position.z;
    return slot;
}

// The fronts expand at the medium's speed of sound. When one reaches an
// organism, the organism hears it - if spreading and absorption leave enough
// of it - shifted by the Doppler effect of their relative motion. A click
// that reaches the hunter's target reflects: its echo returns to the hunter,
// and the hunter clicks again once the echo arrives, so the click rate is
// set by the two-way travel time and rises as the distance closes - the
// terminal buzz (Au 1993; Miller, Johnson & Tyack 2004).
void EcologicalMusicDirector::propagate(const Ecosystem& ecosystem) noexcept
{
    for (auto& listener : heard)
        for (auto& h : listener)
        {
            h.level *= 0.88f;
            if (h.age < 0xffffu)
                ++h.age;
        }

    const auto threshold = 0.03f + (1.0f - perf.reaction) * 0.06f;
    std::array<std::uint8_t, count> echoFrom {};   // reflector -> hunter + 1
    std::array<float, count> echoMidi {}, echoAccent {};
    constexpr std::uint8_t everyone = static_cast<std::uint8_t>((1u << count) - 1u);
    for (auto& front : fronts)
    {
        if (!front.live)
            continue;
        front.radius += medium.soundSpeed;
        for (std::size_t j = 0; j < count; ++j)
        {
            if ((front.reached & (1u << j)) != 0u)
                continue;
            const auto& listener = ecosystem.organism(j);
            const auto d = distance3(front.originX, front.originY, front.originZ, listener.position);
            if (d > front.radius)
                continue;
            front.reached = static_cast<std::uint8_t>(front.reached | (1u << j));
            if (!present(listener))
                continue;
            if (front.echo)
            {
                // the hunter hears its echo: the next click after a short lag
                const auto lag = d < buzzDistance ? 0 : 1;
                voices[j].clickWait = static_cast<std::uint16_t>(std::min<int>(voices[j].clickWait, lag));
                continue;
            }
            // the deep channel: along its axis the sound travels far, across
            // it falls into a shadow (SoundChannel)
            const auto level = receivedLevel(d, front.midi, medium.absorption, tele.temperatureC, salinityPpt())
                * SoundChannel::gain(front.originY, listener.position.y, d, tele.channelAxis)
                * (0.5f + front.accent * 0.5f);
            const auto ts = SoundChannel::trapping(front.originY, tele.channelAxis);
            const auto tl = SoundChannel::trapping(listener.position.y, tele.channelAxis);
            const auto heardIt = level >= threshold ? 1u : 0u;
            if (ts > 0.5f && tl > 0.5f && d > 1.5f)
            {
                ++tele.reachedInChannel[0];
                tele.reachedInChannel[1] += heardIt;
            }
            else if (std::abs(ts - tl) > 0.5f && d > 1.5f)
            {
                ++tele.reachedAcross[0];
                tele.reachedAcross[1] += heardIt;
            }
            if (level < threshold)
                continue;
            // Doppler: approaching raises the pitch, receding lowers it. The
            // speed ratio is exaggerated into a few dozen cents so that the
            // ear can follow it (a liquid pitch, not a tempered one)
            const auto& src = ecosystem.organism(front.source);
            const auto dx = listener.position.x - src.position.x;
            const auto dy = listener.position.y - src.position.y;
            const auto dz = listener.position.z - src.position.z;
            const auto len = std::max(1.0e-4f, std::sqrt(dx * dx + dy * dy + dz * dz));
            const auto closing = -((listener.velocity.x - src.velocity.x) * dx
                                 + (listener.velocity.y - src.velocity.y) * dy
                                 + (listener.velocity.z - src.velocity.z) * dz) / len;
            const auto cents = std::clamp(closing * 70.0f, -45.0f, 45.0f);
            auto& h = heard[j][front.source];
            h.level = std::max(h.level, level);
            h.midi = front.midi + cents / 100.0f;
            h.age = 0;
            ++tele.wavefrontsHeard;
            // a click that reaches the prey reflects back to the hunter
            if (front.click && tele.relation[front.source][j] == Relation::hunts)
            {
                echoFrom[j] = static_cast<std::uint8_t>(front.source + 1u);
                echoMidi[j] = front.midi;
                echoAccent[j] = front.accent * level;
            }
        }
        if (front.reached == everyone || front.radius > 8.0f)
            front.live = false;
    }
    // the echoes leave the reflectors; only the hunter listens to its own
    for (std::size_t j = 0; j < count; ++j)
        if (echoFrom[j] != 0u)
        {
            const auto slot = emit(ecosystem, j, echoMidi[j], echoAccent[j], false, true);
            const auto hunter = static_cast<unsigned>(echoFrom[j] - 1u);
            fronts[slot].reached = static_cast<std::uint8_t>(everyone & ~(1u << hunter));
        }
}

// ---- 3. the web of relations ---------------------------------------------------
//
// Each directed pair (a -> b) holds one relation. It comes only from what a
// SAW (Ecosystem::perceivedBy, the same underwater vision as the strategies)
// or HEARD (the propagation above); underwater, hearing reaches further than
// sight, so it counts double. A new relation grows fast; an old one decays
// slowly and must fade before another takes its place: an encounter leaves a
// memory, and the music has time to develop it.
void EcologicalMusicDirector::updateWeb(const Ecosystem& ecosystem) noexcept
{
    const auto environment = ecosystem.environmentState();
    const auto threshold = 0.08f + (1.0f - perf.reaction) * 0.4f;
    const auto reach = 0.5f + perf.reaction;
    for (std::size_t a = 0; a < count; ++a)
    {
        const auto& A = ecosystem.organism(a);
        for (std::size_t b = 0; b < count; ++b)
        {
            if (a == b)
                continue;
            const auto& B = ecosystem.organism(b);
            auto type = Relation::indifferent;
            float drive = 0.0f;
            if (present(A) && present(B))
            {
                const auto seen = ecosystem.perceivedBy(a, b);
                const auto sense = std::max(seen, std::min(1.0f, heard[a][b].level * 2.0f));
                const auto d = ecosystem.distanceBetween(a, b);
                const auto mutual = std::min(sense, std::max(ecosystem.perceivedBy(b, a),
                                                              std::min(1.0f, heard[b][a].level * 2.0f)));
                const auto hungry = A.energy < 0.3f && B.strength < A.strength * 0.9f && !has(B, Strategy::Predator);
                const auto predation = has(A, Strategy::Predator) && (has(B, Strategy::Prey) || hungry);
                const auto preyOf = has(A, Strategy::Prey) && has(B, Strategy::Predator);
                const auto fear = 2.2f + A.intelligence * 1.5f;
                if (predation && sense > threshold && d < huntDistance)
                {
                    type = Relation::hunts;
                    drive = sense * std::sqrt(1.0f - d / huntDistance) * reach;
                }
                else if (preyOf && sense > threshold && d < fear)
                {
                    type = Relation::flees;
                    drive = sense * (1.0f - d / fear) * reach * 1.4f;
                }
                else if (environment.matingPulse > 0.45f && mutual > threshold
                         && !(has(A, Strategy::Predator) && has(B, Strategy::Prey))
                         && !(has(A, Strategy::Prey) && has(B, Strategy::Predator)))
                {
                    type = Relation::courts;
                    drive = environment.matingPulse * mutual * reach;
                }
                else if (has(A, Strategy::Territorial) && d < territoryRadius && sense > threshold)
                {
                    type = Relation::guards;
                    drive = sense * (1.0f - d / territoryRadius) * reach * 1.3f;
                }
                else if (A.schoolId >= 0 && A.schoolId == B.schoolId)
                {
                    // the school bond exists without seeing; seeing tightens it
                    type = Relation::allied;
                    drive = A.schoolCohesion * (0.45f + 0.55f * sense);
                }
                else if ((A.species == B.species || (has(A, Strategy::Territorial) && has(B, Strategy::Territorial))
                          || (A.role == Ecosystem::SonicRole::Foreground && B.role == Ecosystem::SonicRole::Foreground))
                         && d < rivalDistance && sense > threshold)
                {
                    type = Relation::rivals;
                    drive = sense * (1.0f - d / rivalDistance) * reach;
                }
            }
            drive = std::clamp(drive, 0.0f, 1.0f);
            auto& current = tele.relation[a][b];
            auto& bond = tele.bond[a][b];
            if (type == current)
                bond += (drive - bond) * (drive > bond ? 0.30f : 0.035f);
            else
            {
                // a stronger relation displaces the old one quickly; fading into
                // indifference is slow (the memory of the encounter)
                bond *= type == Relation::indifferent ? 0.965f : 0.80f;
                if (bond < 0.08f)
                {
                    current = type;
                    bond = drive * 0.3f;
                }
            }
        }
    }
}

float EcologicalMusicDirector::pairCompatibility(std::size_t a, std::size_t b) const noexcept
{
    const auto ab = tele.bond[a][b], ba = tele.bond[b][a];
    const auto total = ab + ba;
    if (total < 1.0e-3f)
        return 0.0f;
    return (compatibility(tele.relation[a][b]) * ab + compatibility(tele.relation[b][a]) * ba) / total;
}

std::size_t EcologicalMusicDirector::strongest(std::size_t organism, Relation relation) const noexcept
{
    std::size_t best = count;
    float bond = 0.0f;
    for (std::size_t b = 0; b < count; ++b)
        if (b != organism && tele.relation[organism][b] == relation && tele.bond[organism][b] > bond)
        {
            bond = tele.bond[organism][b];
            best = b;
        }
    return best;
}

Relation EcologicalMusicDirector::dominant(std::size_t organism, float& intensity, std::size_t& partner) const noexcept
{
    auto best = Relation::indifferent;
    intensity = 0.0f;
    partner = count;
    for (std::size_t b = 0; b < count; ++b)
    {
        if (b == organism)
            continue;
        const auto r = tele.relation[organism][b];
        const auto bond = tele.bond[organism][b];
        if (r == Relation::indifferent || bond < 0.22f)
            continue;
        if (priority(r) > priority(best) || (r == best && bond > intensity))
        {
            best = r;
            intensity = bond;
            partner = b;
        }
    }
    return best;
}

// ---- 4. harmony and incompatibility ------------------------------------------------

float EcologicalMusicDirector::pitchFor(int degree, int registerBase) const noexcept
{
    const auto octave = degree >= 0 ? degree / 7 : -((-degree + 6) / 7);
    const auto index = degree - octave * 7;
    return static_cast<float>(registerBase + tele.centre + field[static_cast<std::size_t>(index)] + octave * 12);
}

bool EcologicalMusicDirector::inField(float midi, bool chromaticAllowed) const noexcept
{
    if (chromaticAllowed && medium.crystal < 0.5f)
        return true;
    const auto pc = ((static_cast<int>(std::lround(midi)) - tele.centre) % 12 + 12) % 12;
    if (medium.crystal >= 0.5f)
    {
        for (const auto d : latticeDegrees)
            if (field[static_cast<std::size_t>(d)] == pc)
                return true;
        return false;
    }
    for (const auto m : field)
        if (m == pc)
            return true;
    return false;
}

// The vertical interval is the relation made audible. Around the proposed
// pitch, each candidate is scored against what the others are sounding:
// compatible partners (allies, a courting pair) reward consonance,
// incompatible ones (hunter and prey, rivals, a guard and its intruder)
// reward friction, weighted by how strong the bond is. The keynote pulls
// gently towards the chord. Incompatibility strong enough opens the field to
// chromatic tones.
float EcologicalMusicDirector::harmonise(std::size_t voice, float proposal, const Output& out,
                                         float stickiness) const noexcept
{
    bool chromatic = tele.tension > 0.65f;
    for (std::size_t b = 0; b < count && !chromatic; ++b)
        if (b != voice && compatibility(tele.relation[voice][b]) < 0.0f && tele.bond[voice][b] > 0.5f)
            chromatic = true;

    static constexpr std::array<int, 5> offsets { 0, -1, 1, -2, 2 };
    const auto chordRoot = pitchFor(tele.chordDegree, 48);
    float best = proposal;
    float bestScore = -1.0e9f;
    for (const auto offset : offsets)
    {
        const auto candidate = std::round(proposal) + static_cast<float>(offset);
        if (!inField(candidate, chromatic))
            continue;
        auto score = -stickiness * static_cast<float>(std::abs(offset));
        for (std::size_t j = 0; j < count; ++j)
        {
            if (j == voice)
                continue;
            const auto sounding = out.notes[j].on || voices[j].sinceNote < 4;
            if (!sounding)
                continue;
            const auto other = out.notes[j].on ? out.notes[j].midi : voices[j].lastMidi;
            const auto w = pairCompatibility(voice, j) * std::max(tele.bond[voice][j], tele.bond[j][voice]);
            if (std::abs(w) < 0.02f)
                continue;  // indifferent: no harmonic stake (and no cost)
            // consonance from the two bodies' partials in their register
            // (Plomp & Levelt 1965; Sethares 1993)
            const auto c = SpectralConsonance::consonance(candidate, spectra[voice], other, spectra[j]);
            score += w >= 0.0f ? w * c : -w * (1.0f - c);
        }
        score += 0.12f * consonance(candidate - chordRoot);
        if (score > bestScore)
        {
            bestScore = score;
            best = candidate;
        }
    }
    if (bestScore < -1.0e8f)
    {
        // nothing within two semitones fits the field (only possible in ice):
        // the nearest lattice pitch
        for (int offset = 3; offset <= 6; ++offset)
        {
            if (inField(std::round(proposal) + static_cast<float>(offset), false))
                return std::round(proposal) + static_cast<float>(offset);
            if (inField(std::round(proposal) - static_cast<float>(offset), false))
                return std::round(proposal) - static_cast<float>(offset);
        }
        return std::round(proposal);
    }
    return best;
}

// Tension is the accumulated incompatibility of the web. Under tension the
// harmony leans to the dominant (and, at its height, to the second degree,
// the phrygian/neapolitan colour); when the conflict dissolves after a peak,
// the music cadences - the end of a hunt or of a flight is a resolution. The
// migrant carries the centre, but it may only modulate in calm water: it must
// stay two bars in a new region, at least sixteen bars after the last change.
void EcologicalMusicDirector::updateHarmony(const Ecosystem& ecosystem) noexcept
{
    float conflict = 0.0f;
    for (std::size_t a = 0; a < count; ++a)
        for (std::size_t b = a + 1; b < count; ++b)
            conflict += std::max(0.0f, -pairCompatibility(a, b)) * std::max(tele.bond[a][b], tele.bond[b][a]);
    const auto target = std::clamp(conflict / 0.9f, 0.0f, 1.0f);
    tele.tension += (target - tele.tension) * (target > tele.tension ? 0.08f : 0.03f);
    peakTension = std::max(peakTension, tele.tension);
    if (peakTension > 0.45f && tele.tension < 0.18f)
    {
        cadencePending = true;
        peakTension = 0.0f;
    }

    const auto pos = stepIndex % 16u;
    if (pos != 0u)
        return;
    const auto environment = ecosystem.environmentState();
    const auto wanted = static_cast<int>(std::lround(numeric::unit(environment.daylight, 0.5f) * 5.0f));
    if (std::abs(wanted - modeIndex) >= 3 || (wanted != modeIndex && tele.bar % 4u == 0u))
        modeIndex += wanted > modeIndex ? 1 : -1;  // the colour moves one mode at a time
    tele.mode = modeIndex;
    updateField();

    if (tele.tension > 0.75f)
        tele.chordDegree = tele.bar % 2u == 0u ? 1 : 4;
    else if (tele.tension > 0.45f)
        tele.chordDegree = 4;
    else if (tele.bar % 2u == 0u)
        tele.chordDegree = grazingCycle[(tele.bar / 2u) % grazingCycle.size()];
}

// ---- 5. density: the niche's carrying capacity ---------------------------------------
//
// The number of events a bar admits is a population: it grows logistically
// towards what the medium and the desire allow (K), and shrinks quickly when
// K falls. The voices share it by salience - presence, role, and the
// relations they are living. At the climax of a hunt the hunter and its prey
// take almost everything and the reef goes quiet around them.
void EcologicalMusicDirector::allocateDensity(const Ecosystem& ecosystem) noexcept
{
    const auto desire = ecosystem.desireCurrent();
    // the prey are the reef's abundance: plenty of them, a fuller niche;
    // scarce, a sparse one (Lotka-Volterra cycles of minutes)
    const auto abundance = std::clamp(0.55f + 0.45f * tele.prey, 0.7f, 1.35f)
        // the habitat's health, heard (ADI and ACI): a healthy soundscape
        // holds more, a degraded one (crowded bands, a drone) less
        * (0.8f + 0.4f * std::clamp(tele.habitat.health, 0.0f, 1.0f));
    const auto k = std::max(2.0f, medium.capacity * (0.7f + 0.6f * numeric::unit(desire.density, 0.5f)) * abundance);
    if (capacity < k)
        capacity += 0.45f * capacity * (1.0f - capacity / k);
    else
        capacity += (k - capacity) * 0.5f;
    capacity = std::max(2.0f, capacity);
    tele.capacity = capacity;
    // hi-fi / lo-fi (Schafer): the pressure on the niche - what was sung
    // plus what it refused - over what it admits
    const auto pressure = static_cast<float>(eventsThisBar + deniedThisBar) / std::max(1.0f, capacity);
    // (measured in the full engine: the pressure sits near 1 and peaks at
    // 1.8-3.3 when the web crowds; the curve begins just under 1)
    const auto x = std::clamp((pressure - 0.9f) / 0.9f, 0.0f, 1.0f);
    tele.lofi += (x * x * (3.0f - 2.0f * x) - tele.lofi) * 0.35f;
    tele.deniedLastBar = deniedThisBar;
    deniedThisBar = 0;
    tele.eventsLastBar = eventsThisBar;
    eventsThisBar = 0;
    cloudThisBar = 0;

    std::array<float, count> salience {};
    float total = 0.0f;
    for (std::size_t i = 0; i < count; ++i)
    {
        const auto& o = ecosystem.organism(i);
        if (!present(o))
            continue;
        auto s = o.role == Ecosystem::SonicRole::Foreground ? 1.0f : 0.7f;
        for (std::size_t b = 0; b < count; ++b)
        {
            if (b == i)
                continue;
            s += tele.bond[i][b] * salienceWeight(tele.relation[i][b]);
            s += tele.bond[b][i] * 0.3f * salienceWeight(tele.relation[b][i]);
        }
        // the hunters weigh as much as there are predators
        for (std::size_t b = 0; b < count; ++b)
            if (b != i && tele.relation[i][b] == Relation::hunts && tele.bond[i][b] > 0.2f)
            {
                s *= std::clamp(tele.predators, 0.5f, 1.8f);
                break;
            }
        salience[i] = s * o.presenceAmount;
        total += salience[i];
    }
    for (std::size_t i = 0; i < count; ++i)
    {
        voices[i].used = 0;
        // a hungry voice saves itself: it sings less
        const auto share = (total > 0.0f ? capacity * salience[i] / total : 0.0f)
            * (1.0f - 0.5f * LifeCycle::hunger(voices[i].life));
        voices[i].quota = salience[i] > 0.0f
            ? static_cast<std::uint8_t>(std::clamp(static_cast<int>(std::lround(share)), 1, 16)) : 0;
    }
}

// ---- life, food and descent (LifeCycle) ---------------------------------------------
//
// Once a bar: the grazers eat as much as the prey abound (Lotka-Volterra),
// the hunters only what they capture; living and singing cost. A life ends
// of age or hunger; after a silence of eight bars the voice is born again,
// singing its parent's song recombined with its mate's.
void EcologicalMusicDirector::liveBar(const Ecosystem& ecosystem) noexcept
{
    const auto barSeconds = 240.0f / std::max(20.0f, tele.tempoBpm);
    for (std::size_t i = 0; i < count; ++i)
    {
        auto& v = voices[i];
        if (v.unborn)
        {
            if (v.silenced == 0)
                beBorn(ecosystem, i);
        }
        else if (present(ecosystem.organism(i)))
        {
            const auto prey = strongest(i, Relation::hunts);
            const auto hunting = prey < count && tele.bond[i][prey] > 0.3f;
            const auto food = hunting ? 0.0f : LifeCycle::graze(v.life, barSeconds, tele.prey);
            if (LifeCycle::live(v.life, barSeconds, food, v.used) != LifeCycle::Event::none)
                die(i);
        }
        tele.energy[i] = v.life.energy;
        tele.age[i] = v.life.age;
        tele.generation[i] = v.life.generation;
    }
}

void EcologicalMusicDirector::die(std::size_t i) noexcept
{
    auto& v = voices[i];
    decomposed[decomposedCount % decomposed.size()] = v.song;  // the reef keeps it
    ++decomposedCount;
    v.silenced = 8u * 16u;
    v.unborn = true;
    v.rubato = 1.6f;
    beginAct(Act::death);
}

void EcologicalMusicDirector::beBorn(const Ecosystem& ecosystem, std::size_t i) noexcept
{
    auto& v = voices[i];
    // the mate: whom it courted, or else its closest ally, or else whoever
    // it was most bound to
    auto mate = strongest(i, Relation::courts);
    if (mate >= count || tele.bond[i][mate] < 0.2f)
        mate = strongest(i, Relation::allied);
    if (mate >= count || tele.bond[i][mate] < 0.2f)
    {
        float best = -1.0f;
        for (std::size_t b = 0; b < count; ++b)
            if (b != i && present(ecosystem.organism(b)) && tele.bond[i][b] + tele.bond[b][i] > best)
            {
                best = tele.bond[i][b] + tele.bond[b][i];
                mate = b;
            }
    }
    if (mate >= count)
        mate = (i + 1) % count;
    const auto& mine = v.form.themes();
    const auto& theirs = voices[mate].form.themes();
    const auto themeCount = std::max<std::uint8_t>(1, v.form.themeCount());
    const auto mateThemes = std::max<std::uint8_t>(1, voices[mate].form.themeCount());
    std::array<Signature, SongForm::maxThemes> child {};
    for (std::size_t t = 0; t < themeCount; ++t)
    {
        const auto cut = nextUnit(), which = nextUnit(), by = nextUnit();
        child[t] = LifeCycle::recombine(mine[t], theirs[std::min<std::size_t>(t, mateThemes - 1u)], cut, which, by);
    }
    std::array<std::uint8_t, SongForm::maxThemes> repeats {};
    for (auto& r : repeats)
        r = static_cast<std::uint8_t>(2 + static_cast<int>(nextUnit() * 3.0f));
    v.form.set(child, themeCount, repeats);
    v.song = v.form.current();
    v.phraseEnd = 0;
    v.cursor = 0;
    v.unborn = false;
    v.rubato = 1.0f;
    const auto generation = static_cast<std::uint16_t>(std::max(v.life.generation, voices[mate].life.generation) + 1u);
    LifeCycle::beBorn(v.life, 420.0f + 480.0f * nextUnit(), generation);
    beginAct(Act::birth);
}

// ---- the soundmark (Schafer 1977) ---------------------------------------------------------
//
// Schafer's keynote (the anchor's pedal) and signals (the guard) were already
// here; the soundmark is the sound that identifies THIS place. The first song
// revolution gives it (or, without one, the first song to come full circle);
// it then returns after every cadence and every change of the water, played
// by the anchor on the current chord, its contour kept: the reef's emblem.
void EcologicalMusicDirector::markPlace(const Signature& motif) noexcept
{
    if (motif.length < 2)
        return;
    soundmark = motif;
    tele.soundmark = motif;
    tele.hasSoundmark = true;
}

void EcologicalMusicDirector::chooseSoundmark() noexcept
{
    // the voice that sang last (and is alive): its phrase is what sounds now
    std::size_t chosen = count;
    std::uint16_t newest = 0xffffu;
    for (std::size_t i = 0; i < count; ++i)
        if (!voices[i].unborn && voices[i].sinceNote < newest && voices[i].song.length >= 2)
        {
            newest = voices[i].sinceNote;
            chosen = i;
        }
    if (chosen >= count)
        return;
    markPlace(voices[chosen].song);
    tele.soundmarkChosen = true;
    soundmarkPending = true;  // heard at once, whatever the spacing
    soundmarkCursor = 0;
    soundmarkWait = 0;
}

void EcologicalMusicDirector::restoreSoundmark(const SongForm::Phrase& motif) noexcept
{
    if (motif.length < 2 || motif.length > SongForm::capacity)
        return;
    SongForm::Phrase clean = motif;
    for (std::size_t u = 0; u < clean.length; ++u)
    {
        clean.degrees[u] = static_cast<std::int8_t>(std::clamp<int>(clean.degrees[u], -5, 7));
        clean.steps[u] = std::clamp<std::uint8_t>(clean.steps[u], 1, 16);
    }
    markPlace(clean);
    tele.soundmarkChosen = true;
}

// the emblem returns, but not as a tic: at least 32 bars (about a minute
// and a half) after its last return (measured in the full engine: after every
// cadence and change it came back every ~35 s)
void EcologicalMusicDirector::callSoundmark() noexcept
{
    if (tele.hasSoundmark && (tele.soundmarkReturns == 0 || tele.bar >= lastSoundmarkBar + 32u))
        soundmarkPending = true;
}

void EcologicalMusicDirector::playSoundmark(const Ecosystem& ecosystem, Output& out, std::size_t keeper) noexcept
{
    if (keeper >= count || !present(ecosystem.organism(keeper)) || voices[keeper].unborn)
    {
        soundmarkPending = false;
        soundmarkCursor = 0;
        return;
    }
    if (soundmarkWait > 0)
    {
        --soundmarkWait;
        return;
    }
    if (soundmarkCursor == 0)
    {
        soundmarkPending = false;
        ++tele.soundmarkReturns;
        lastSoundmarkBar = tele.bar;
        beginAct(Act::soundmark);
    }
    const auto k = soundmarkCursor;
    const auto pitch = pitchFor(tele.chordDegree + soundmark.degrees[k], registerOf(ecosystem.organism(keeper)));
    const auto steps = std::max<std::uint8_t>(1, soundmark.steps[k]);
    play(keeper, ecosystem, out, pitch, 0.85f, true, true, false, static_cast<float>(steps));
    soundmarkWait = static_cast<std::uint16_t>(steps - 1u);
    if (++soundmarkCursor >= soundmark.length)
        soundmarkCursor = 0;
}

float EcologicalMusicDirector::darkness(std::size_t organism) const noexcept
{
    return organism < count ? LifeCycle::darkness(voices[organism].life) : 0.0f;
}

// ---- playing ---------------------------------------------------------------------------

bool EcologicalMusicDirector::play(std::size_t i, const Ecosystem& ecosystem, Output& out, float midi, float accent,
                                   bool structural, bool tempered, bool click, float lengthSteps) noexcept
{
    const auto& organism = ecosystem.organism(i);
    auto& v = voices[i];
    if (!present(organism) || out.notes[i].on)
        return false;
    if (!structural && (v.silenced > 0 || v.frozen > 0))
        return false;
    if (!structural && v.used >= v.quota)
    {
        if (deniedThisBar < 255) ++deniedThisBar;  // the crowding the niche refuses
        return false;
    }

    // the acoustic niche (Krause 1993): indifferent neighbours sounding now
    // within a whole tone - the one in support moves an octave away
    if (organism.role == Ecosystem::SonicRole::Support)
        for (std::size_t j = 0; j < count; ++j)
            if (j != i && out.notes[j].on && std::abs(out.notes[j].midi - midi) < 2.5f
                && std::abs(pairCompatibility(i, j)) < 0.05f)
                midi += midi >= out.notes[j].midi ? 12.0f : -12.0f;

    // the room (RoomCoupling): a loud room fills a register; the voices move
    // an octave away from it, as species shift their band away from a noise
    if (tele.room.active && tele.room.strength > 0.3f && !click && std::abs(midi - tele.room.midi) < 4.0f)
        midi += (midi >= tele.room.midi ? 12.0f : -12.0f);

    // turbulence: the pitch is not held still (a few cents of eddy);
    // crystal: the pitch is a point of the lattice
    if (tempered && medium.crystal >= 0.5f)
        midi = std::round(midi);
    else if (!click)
        midi += (nextUnit() - 0.5f) * medium.turbulence * 0.4f;

    // the Lombard effect (Holt et al. 2009): killer whales raise their calls
    // by 1 dB for every 1 dB of background noise. The noise this organism
    // hears now, in dB above a quiet floor, raises its voice by as many dB
    // (up to 9 dB)
    float noise = 0.0f;
    for (std::size_t j = 0; j < count; ++j)
        if (j != i && heard[i][j].age < 3)
            noise += heard[i][j].level;
    constexpr float quietFloor = 0.08f;
    // the room's noise counts as heard noise: -50 dBFS at the microphone is
    // the quiet floor, every dB above it is a dB of Lombard
    if (tele.room.active)
        noise += quietFloor * std::pow(10.0f, (tele.room.levelDb + 50.0f) / 20.0f);
    const auto noiseDb = noise > quietFloor ? 20.0f * std::log10(noise / quietFloor) : 0.0f;
    accent *= std::pow(10.0f, std::clamp(noiseDb, 0.0f, 9.0f) / 20.0f) * 0.62f;
    // an organism arriving announces itself
    if (organism.presence == Ecosystem::Presence::Entering)
        accent = std::max(accent, 0.95f);

    auto& note = out.notes[i];
    note.on = true;
    note.midi = std::clamp(midi, 24.0f, 96.0f);
    note.accent = std::clamp(accent, 0.0f, 1.0f);
    note.glideSeconds = click ? 0.004f : std::max(0.004f, medium.glideSeconds * (1.0f - medium.crystal));
    // articulation: a note sounds part of its written length and then ends,
    // so rests are silent (2026-10-20, the author: "há timbres com notas
    // longas, que são chatas"); the water decides how legato it is
    const auto stepSeconds = 15.0f / std::max(30.0f, tele.tempoBpm);
    note.durationSeconds = click ? 0.02f
        : std::clamp(lengthSteps * stepSeconds * medium.legato, 0.03f, 3.0f);
    note.releaseSeconds = click ? 0.03f : medium.releaseSeconds;
    // lo-fi (Schafer): in a crowded niche the sounds lose definition - they
    // last longer, slide more, their accents flatten and their pitches beat
    // against each other, until the reef is a mass rather than voices
    if (tele.lofi > 0.0f && !click)
    {
        const auto l = tele.lofi;
        note.durationSeconds = std::min(3.0f, note.durationSeconds * (1.0f + 0.8f * l));
        note.releaseSeconds *= 1.0f + 1.5f * l;
        note.accent += (0.55f - note.accent) * 0.6f * l;
        // ice keeps its lattice: a crowded ice is longer, not blurred
        if (!(tempered && medium.crystal >= 0.5f))
        {
            note.glideSeconds *= 1.0f + l;
            note.midi = std::clamp(note.midi + (nextUnit() - 0.5f) * 0.3f * l, 24.0f, 96.0f);
        }
    }
    v.lastMidi = note.midi;
    v.sinceNote = 0;
    if (v.reserved > 0 && structural && v.phraseAdmitted)
        --v.reserved;  // already counted when the phrase was admitted
    else
    {
        if (v.used < 255) ++v.used;
        if (eventsThisBar < 255) ++eventsThisBar;
    }
    if (tele.notesThisBar[i] < 255) ++tele.notesThisBar[i];
    emit(ecosystem, i, note.midi, note.accent, click, false);
    return true;
}

// The phrase engine: an organism sings its signature (or, in a courtship,
// the partner's) with its rubato, its transformations and, at the end, an
// optional ornament - the elements that individualise sperm whale codas
// (Sharma et al. 2024). Between phrases it rests. A unit that the niche has
// no room for is not heard, but the phrase keeps its time.
bool EcologicalMusicDirector::sing(std::size_t i, const Ecosystem& ecosystem, Output& out, float durationScale,
                                   int registerShift, bool syncopate) noexcept
{
    auto& v = voices[i];
    if (v.rest > 0)
    {
        --v.rest;
        return false;
    }
    if (v.wait > 0)
    {
        --v.wait;
        return false;
    }
    const auto& organism = ecosystem.organism(i);
    const auto& song = v.usingBorrowed ? v.borrowed : v.song;
    const auto pos = stepIndex % 16u;
    if (v.cursor == 0 && syncopate && pos % 2u == 0u)
    {
        v.wait = 0;  // the phrase starts off the beat
        return false;
    }
    // the temporal niche: a phrase begins only when the organism's calling
    // clock fires - its timing is shared with the others by listening
    if (v.cursor == 0)
    {
        if (!callReady[i])
            return false;
        // an animal sings a whole phrase or waits: the niche admits the
        // phrase at its start (Payne & McVay's units are not dropped one by
        // one, which would break the motif); otherwise it waits for its next call
        auto units = static_cast<std::uint8_t>(std::min<int>(song.length, 16));
        if (v.used + units > v.quota)
        {
            if (v.used > 0 || v.quota < 1)
                return false;
            // the niche has room for the head of the theme only: it is sung
            // as a fragment, its beginning kept so the motif is recognised
            units = v.quota;
            v.phraseEnd = units;
        }
        callReady[i] = false;
        v.phraseAdmitted = true;
        v.reserved = units;  // the whole phrase is counted now
        v.used = static_cast<std::uint8_t>(std::min(255, v.used + units));
        eventsThisBar = static_cast<std::uint8_t>(std::min(255, eventsThisBar + units));
    }
    if (v.phraseEnd == 0 || v.phraseEnd > song.length)
        v.phraseEnd = song.length;
    const auto units = static_cast<std::uint8_t>(v.phraseEnd + (v.ornament ? 1u : 0u));
    const auto isOrnament = v.cursor >= v.phraseEnd;

    const auto unit = static_cast<std::size_t>(isOrnament ? v.phraseEnd - 1u : v.cursor);
    int degree = song.degrees[unit] + (isOrnament ? 1 : 0);
    if (v.inverted)
        degree = -degree;
    const auto proposal = pitchFor(tele.chordDegree + v.transpose + degree, registerOf(organism))
        + static_cast<float>(registerShift);
    // its own song resists the pull of the harmony: the intervals of a theme
    // are its identity (a move of one semitone costs three times more)
    const auto midi = harmonise(i, proposal, out, 0.9f);
    // in an exchange the pair speaks in the caller's coda (rhythm x tempo),
    // stretched by each one's rubato; otherwise the song keeps its own rhythm
    auto unitSteps = static_cast<float>(song.steps[unit]);
    if (v.exchangePartner >= 0)
    {
        const auto& caller = voices[std::min(i, static_cast<std::size_t>(v.exchangePartner))];
        unitSteps = static_cast<float>(CodaExchange::steps(caller.codaRhythm, unit, song.length))
            * CodaExchange::tempo(caller.codaTempo);
    }
    const auto steps = isOrnament ? 1.0f : unitSteps * durationScale * v.rubato;
    const auto played = play(i, ecosystem, out, midi, v.cursor == 0 ? 0.85f : (isOrnament ? 0.4f : 0.55f),
                             v.phraseAdmitted, true, false, steps);

    auto wait = std::max(0, static_cast<int>(std::lround(steps)) - 1);
    if (medium.crystal >= 0.5f)
        wait = wait / 2 * 2 + 1;  // the lattice: an even number of steps between onsets
    v.wait = static_cast<std::uint16_t>(wait);

    if (++v.cursor < units)
        return played;

    // ---- between phrases ----
    v.cursor = 0;
    v.phraseAdmitted = false;
    v.reserved = 0;
    v.ornament = false;
    const auto desire = ecosystem.desireCurrent();
    v.rest = static_cast<std::uint16_t>(std::lround((3.0f + desire.calm * 8.0f + nextUnit() * 5.0f)
                                                    * (1.0f + medium.glideSeconds * 2.0f)));
    // rubato drifts from coda to coda; in ice it is frozen. In an exchange
    // it converges, turn by turn, towards the partner's (Sharma et al. 2024)
    if (v.exchangePartner >= 0)
    {
        const auto p = static_cast<std::size_t>(v.exchangePartner);
        v.exchangeTurn = v.exchangePartner == v.lastPartner ? static_cast<std::uint8_t>(std::min(255, v.exchangeTurn + 1)) : 0;
        const auto gap = std::abs(v.rubato - voices[p].rubato);
        if (v.exchangeTurn <= 1)
        {
            tele.rubatoGapEarly += gap;
            ++tele.rubatoTurnsEarly;
        }
        else if (v.exchangeTurn >= 4)
        {
            tele.rubatoGapLate += gap;
            ++tele.rubatoTurnsLate;
        }
        v.rubato = std::clamp(CodaExchange::matchRubato(v.rubato, voices[p].rubato)
                                  * (1.0f + (nextUnit() - 0.5f) * 0.03f * (1.0f - medium.crystal)),
                              0.75f, 1.35f);
        const auto caller = std::min(i, p);
        if (i == caller && nextUnit() < 0.2f)
            v.codaTempo = static_cast<std::uint8_t>(nextUnit() * static_cast<float>(CodaExchange::tempoCount)) % CodaExchange::tempoCount;
        // a clan forms: an ally sometimes takes the caller's identity coda
        if (i != caller && nextUnit() < perf.memory * 0.15f)
            v.codaRhythm = voices[caller].codaRhythm;
    }
    else
        v.rubato = std::clamp(v.rubato * (1.0f + (nextUnit() - 0.5f) * 0.14f * (1.0f - medium.crystal)), 0.75f, 1.35f);
    // the ornament (an extra unit) marks a change of subject - a new relation
    // or a new partner - and comes a little more often in excitement
    const auto changed = v.currentRelation != v.lastRelation || v.exchangePartner != v.lastPartner;
    v.ornament = nextUnit() < CodaExchange::ornamentChance(changed) + desire.danger * 0.15f;
    if (changed)
    {
        ++tele.subjectChanges;
        tele.ornamentsOnChange += v.ornament ? 1u : 0u;
    }
    else
    {
        ++tele.steadyCodas;
        tele.ornamentsSteady += v.ornament ? 1u : 0u;
    }
    v.lastRelation = v.currentRelation;
    v.lastPartner = v.exchangePartner;
    // the song's form (Payne & McVay 1971): the phrase repeats, the next
    // theme begins, or the whole song begins again - the repetition is the
    // identity. MEMORY is the slow drift of the song from cycle to cycle
    // (one unit at a time); ice freezes it.
    const auto advance = v.form.endPhrase();
    if (advance != SongForm::Advance::repeat)
    {
        v.song = v.form.current();
        v.phraseEnd = 0;
    }
    if (advance == SongForm::Advance::songAgain)
    {
        beginAct(Act::songCycle);
        // with no revolution yet, the first song to come full circle after
        // the first minute or so marks the place
        if (!tele.hasSoundmark && tele.bar >= 24u)
            markPlace(v.form.themes()[0]);
        if (nextUnit() < perf.memory * 0.6f * (1.0f - medium.crystal))
        {
            v.form.evolve(nextUnit(), nextUnit(), nextUnit(), nextUnit());
            v.song = v.form.current();
        }
    }
    // turbulence breaks phrases into cells (Kolmogorov's cascade, as a figure)
    if (nextUnit() < medium.turbulence * 0.6f)
        v.phraseEnd = static_cast<std::uint8_t>(std::max(2, song.length / 2));
    else if (nextUnit() < 0.3f)
        v.phraseEnd = 0;
    // a song revolution (Garland et al. 2011): who hears the new song takes
    // it whole, within a few phrases - not unit by unit
    if (revolutionSource >= 0 && static_cast<std::size_t>(revolutionSource) != i && !v.adopted
        && heard[i][static_cast<std::size_t>(revolutionSource)].age < 48
        && nextUnit() < 0.35f + perf.memory * 0.3f)
    {
        v.form = voices[static_cast<std::size_t>(revolutionSource)].form;
        v.song = v.form.current();
        v.adopted = true;
        v.transpose = 0;
        v.inverted = false;
        v.phraseEnd = 0;
        ++tele.songCopies;
        beginAct(Act::revolution);
        v.usingBorrowed = false;
        return played;
    }
    // cultural transmission (Garland et al. 2011): an ally that was heard
    // lends one unit of its song
    const auto ally = strongest(i, Relation::allied);
    if (ally < count && tele.bond[i][ally] > 0.4f && heard[i][ally].age < 24 && nextUnit() < perf.memory * 0.3f)
    {
        const auto& other = voices[ally].song;
        const auto k = static_cast<std::size_t>(nextUnit() * static_cast<float>(std::min(v.song.length, other.length)));
        v.song.degrees[k] = other.degrees[k];
        v.song.steps[k] = other.steps[k];
        v.form.current() = v.song;  // the borrowed unit stays in the song
        ++tele.songCopies;
    }
    v.usingBorrowed = false;
    return played;
}

// ---- the step ----------------------------------------------------------------------------

void EcologicalMusicDirector::step(const Ecosystem& ecosystem, Output& out) noexcept
{
    const auto integrity = ecosystem.integrityState();
    const auto pos = stepIndex % 16u;

    propagate(ecosystem);
    updateWeb(ecosystem);
    updateHarmony(ecosystem);
    advanceChorus(ecosystem);
    if (pos == 0u)
    {
        liveBar(ecosystem);  // before the niche resets what each voice sang
        allocateDensity(ecosystem);
        // a song revolution now and then (more often with MEMORY): someone
        // invents a song, and it may sweep through the reef
        if (revolutionBars > 0)
        {
            if (--revolutionBars == 0)
                revolutionSource = -1;
        }
        else if (nextUnit() < perf.memory * 0.03f)
        {
            const auto source = static_cast<std::size_t>(nextUnit() * static_cast<float>(count)) % count;
            if (present(ecosystem.organism(source)))
                beginRevolution(ecosystem, source);
        }
    }
    netForHunter = netNext;
    netNext.fill(false);
    for (auto& v : voices)
        if (v.sinceNote < 0xffffu)
            ++v.sinceNote;

    std::size_t migrant = count, sessile = count, striker = count;
    for (std::size_t i = 0; i < count; ++i)
    {
        const auto& o = ecosystem.organism(i);
        if (has(o, Strategy::Migratory) && migrant == count) migrant = i;
        if (has(o, Strategy::Sessile) && sessile == count) sessile = i;
        if (o.species == Species::GranularStriker && striker == count) striker = i;
    }

    // ---- departures: who leaves feeds the reef ----
    for (std::size_t i = 0; i < count; ++i)
    {
        const auto leaving = ecosystem.organism(i).presence == Ecosystem::Presence::Leaving;
        if (leaving && !voices[i].wasLeaving)
        {
            decomposed[decomposedCount % decomposed.size()] = voices[i].song;
            ++decomposedCount;
            voices[i].rubato = 1.6f;  // its last phrase slows down
            beginAct(Act::departure);
        }
        voices[i].wasLeaving = leaving;
        // a newcomer brings its own song
        const auto entering = ecosystem.organism(i).presence == Ecosystem::Presence::Entering;
        if (entering && !voices[i].wasEntering && revolutionBars == 0)
            beginRevolution(ecosystem, i);
        voices[i].wasEntering = entering;
    }

    // ---- rupture: a new crack is an identifiable cause ----
    if (integrity.cracked && !wasCracked)
    {
        ruptureSteps = 32;
        beginAct(Act::rupture);
    }
    wasCracked = integrity.cracked;
    if (integrity.shattered)
        return;  // the shattered vessel is silent
    if (ruptureSteps > 0)
    {
        --ruptureSteps;
        if (ruptureSteps < 8 && striker < count && nextUnit() < 0.45f)
            play(striker, ecosystem, out, pitchFor(static_cast<int>(nextUnit() * 14.0f), 72), 0.9f, true, false, false, 1.0f);
        return;
    }

    // ---- a change of state: suspension, only the anchor holds ----
    if (fermataSteps > 0)
    {
        if (fermataSteps == 8 && sessile < count)
            play(sessile, ecosystem, out, pitchFor(tele.chordDegree, registerOf(ecosystem.organism(sessile))), 0.8f, true,
                 true, false, 8.0f);
        --fermataSteps;
        return;
    }

    // ---- the migrant moves the centre, in calm water ----
    if (migrant < count)
    {
        const auto& m = ecosystem.organism(migrant);
        auto angle = std::atan2(m.position.z, m.position.x);
        if (angle < 0.0f) angle += 6.2831853f;
        const auto sector = static_cast<int>(angle / 6.2831853f * 4.0f) % 4;
        if (regionSector < 0)
        {
            regionSector = sector;
            tele.centre = regionCentres[static_cast<std::size_t>(sector)];
        }
        else if (sector != regionSector)
        {
            if (sector == pendingSector)
                sectorDwell = static_cast<std::uint16_t>(std::min<int>(sectorDwell + 1, 0xffff));
            else
            {
                pendingSector = sector;
                sectorDwell = 0;
            }
            if (pos == 0u && sectorDwell >= 32u && tele.bar >= lastModulationBar + 16u && tele.tension < 0.4f)
            {
                regionSector = sector;
                tele.centre = regionCentres[static_cast<std::size_t>(sector)];
                lastModulationBar = tele.bar;
                beginAct(Act::modulation);
                // the migrant announces the new region on its fifth
                play(migrant, ecosystem, out, pitchFor(4, registerOf(m)), 1.0f, true, true, false, 6.0f);
            }
        }
        else
            pendingSector = -1;
    }

    // ---- resolution: the conflict dissolved; the reef cadences together ----
    if (cadencePending && pos == 0u)
    {
        cadencePending = false;
        tele.chordDegree = 0;
        beginAct(Act::resolution);
        callSoundmark();  // and after the cadence
        static constexpr std::array<int, count> tones { 4, 7, 2, 9, 0 };
        for (std::size_t i = 0; i < count; ++i)
        {
            const auto& o = ecosystem.organism(i);
            const auto degree = i == sessile ? 0 : tones[i];
            play(i, ecosystem, out, pitchFor(degree, registerOf(o)), i == sessile ? 0.9f : 0.6f, true, true, false, 12.0f);
            voices[i].rest = static_cast<std::uint16_t>(4 + i);
            voices[i].wait = 0;
            voices[i].cursor = 0;
        }
        return;
    }

    // ---- the soundmark (Schafer): the motif of this place, by the anchor ----
    if (soundmarkPending || soundmarkCursor > 0)
        playSoundmark(ecosystem, out, sessile < count ? sessile : count - 1);

    // ---- boiling: Minnaert bubbles - the pitches of the liquid itself ----
    bool bubbled = false;
    if (striker < count && medium.bubbles > 0.05f && present(ecosystem.organism(striker)))
    {
        auto& v = voices[striker];
        if (v.bubblesLeft == 0 && nextUnit() < medium.bubbles * 0.12f)
        {
            v.bubblesLeft = static_cast<std::uint8_t>(3 + static_cast<int>(nextUnit() * 5.0f));
            v.bubbleRadius = 2.5f + nextUnit() * 2.5f;  // millimetres
            beginAct(Act::bubbles);
        }
        if (v.bubblesLeft > 0 && nextUnit() < 0.6f)
        {
            // the bubble shrinks and rises in pitch; the last one collapses
            const auto last = v.bubblesLeft == 1;
            bubbled = play(striker, ecosystem, out, minnaertMidi(v.bubbleRadius), last ? 0.9f : 0.35f, false, false, false,
                           0.5f);
            v.bubbleRadius *= 0.82f;
            --v.bubblesLeft;
        }
    }

    // ---- the web, organism by organism, the strongest relations first ----
    std::array<std::size_t, count> order {};
    std::array<Relation, count> rel {};
    std::array<float, count> intensity {};
    std::array<std::size_t, count> partner {};
    for (std::size_t i = 0; i < count; ++i)
    {
        order[i] = i;
        rel[i] = dominant(i, intensity[i], partner[i]);
    }
    std::sort(order.begin(), order.end(), [&](std::size_t a, std::size_t b) {
        return priority(rel[a]) != priority(rel[b]) ? priority(rel[a]) > priority(rel[b]) : a < b;
    });

    for (const auto i : order)
    {
        auto& v = voices[i];
        const auto& o = ecosystem.organism(i);
        if (v.silenced > 0) { --v.silenced; continue; }
        if (v.frozen > 0) { --v.frozen; continue; }
        if (!present(o) || (i == striker && bubbled))
            continue;
        const auto p = partner[i];
        // CodaExchange: courting and allied pairs that hear each other speak
        // in codas; the subject is the relation they live now
        v.currentRelation = rel[i];
        v.exchangePartner = (rel[i] == Relation::courts || rel[i] == Relation::allied) && p < count
                && heard[i][p].age < 48
            ? static_cast<int>(p) : -1;

        // the thermal cloud (Xenakis, Pithoprakta: each instrument a molecule,
        // the speeds of its glissandi following Maxwell-Boltzmann): in
        // turbulent and boiling water, an organism free of relations, between
        // its phrases, emits glissandi whose speeds are drawn at the water's
        // temperature - hotter water, faster and more scattered glissandi
        if (rel[i] == Relation::indifferent && i != sessile && medium.turbulence > 0.4f
            && v.cursor == 0 && v.wait == 0
            && static_cast<float>(cloudThisBar) < capacity * (medium.turbulence - 0.3f)
            && nextUnit() < (medium.turbulence - 0.4f) * 0.8f)
        {
            const auto kelvin = (tele.phase == Phase::boiling ? 100.0f : tele.temperatureC) + 273.15f;
            const auto sigma = 5.0f * std::sqrt(kelvin / 288.15f);  // semitones per second
            const auto speed = maxwellSpeed(nextUnit(), nextUnit(), nextUnit(), nextUnit(), nextUnit(), nextUnit(), sigma);
            const auto seconds = 0.3f + nextUnit() * 0.9f;
            const auto home = static_cast<float>(registerOf(o));
            auto target = v.lastMidi + (nextUnit() < 0.5f ? -1.0f : 1.0f) * speed * seconds;
            // the walls of the register reflect the molecule
            if (target > home + 18.0f) target = 2.0f * (home + 18.0f) - target;
            if (target < home - 18.0f) target = 2.0f * (home - 18.0f) - target;
            const auto stepSeconds = 15.0f / std::max(30.0f, tele.tempoBpm);
            // the cloud is a mass of its own: it has its own budget per bar
            // (capacity x turbulence), so it does not take the phrases' place
            if (play(i, ecosystem, out, target, 0.32f + 0.12f * nextUnit(), true, false, false, seconds / stepSeconds))
            {
                out.notes[i].glideSeconds = seconds;  // the glissando IS the note
                ++tele.cloudMolecules;
                ++cloudThisBar;
                v.wait = static_cast<std::uint16_t>(seconds / stepSeconds);
            }
            continue;
        }

        switch (rel[i])
        {
        case Relation::hunts:
        {
            const auto d = ecosystem.distanceBetween(i, p);
            if (v.rest > 0)
            {
                --v.rest;  // satiated after a capture
                break;
            }
            if (d < captureDistance && intensity[i] > 0.45f)
            {
                // capture: the hunter strikes the prey's own pitch; the prey
                // falls silent, and its song goes to the reef
                const auto preyPitch = heard[i][p].age < 64 ? heard[i][p].midi : voices[p].lastMidi;
                play(i, ecosystem, out, std::round(preyPitch), 1.0f, true, true, false, 4.0f);
                if (striker < count && striker != i && striker != p)
                    play(striker, ecosystem, out, std::round(preyPitch) + 13.0f, 0.95f, true, false, false, 1.0f);
                voices[p].silenced = 16;
                decomposed[decomposedCount % decomposed.size()] = voices[p].song;
                ++decomposedCount;
                v.rest = 24;
                v.buzzClicks = 0;
                tele.bond[i][p] *= 0.3f;
                populations.capture();
                // the capture is a meal for the hunter, a wound for the prey
                v.life.energy = std::min(1.0f, v.life.energy + LifeCycle::captureMeal);
                voices[p].life.energy = std::max(0.0f, voices[p].life.energy - LifeCycle::captureWound);
                if (voices[p].life.energy <= 0.0f && !voices[p].unborn)
                    die(p);
                beginAct(Act::capture);
                break;
            }
            // the bubble net: an ally is spiralling around the prey; the hunter
            // stops clicking and calls - a repeated, loud feeding call, as the
            // humpbacks do while the net closes
            if (netForHunter[i])
            {
                if (pos % 4u == 0u)
                {
                    const auto preyPitch = heard[i][p].age < 64 ? heard[i][p].midi : voices[p].lastMidi;
                    auto call = std::round(preyPitch);
                    while (call < static_cast<float>(registerOf(o)) - 5.0f) call += 12.0f;
                    while (call > static_cast<float>(registerOf(o)) + 7.0f) call -= 12.0f;
                    if (play(i, ecosystem, out, harmonise(i, call, out), 0.85f, false, true, false, 3.0f))
                        beginAct(Act::bubbleNet);
                }
                break;
            }
            // imitation: the hunter answers what it HEARD of the prey - a canon
            // whose delay is the propagation, coloured by the Doppler shift
            if (heard[i][p].age == 0 && d < 2.6f && d >= buzzDistance)
            {
                const auto imitation = heard[i][p].midi + 12.0f;
                if (play(i, ecosystem, out, harmonise(i, imitation, out) + (heard[i][p].midi - std::round(heard[i][p].midi)),
                         0.7f, false, false))
                {
                    beginAct(Act::hunt);
                    break;
                }
            }
            // echolocation: a click, then wait for the echo
            if (v.clickWait > 0)
            {
                --v.clickWait;
                break;
            }
            const auto buzz = d < buzzDistance;
            if (buzz && v.buzzClicks < 24)
                ++v.buzzClicks;
            else if (!buzz)
                v.buzzClicks = 0;
            const auto clickPitch = static_cast<float>(registerOf(o) + 19) + static_cast<float>(v.buzzClicks) * 0.5f;
            const auto accent = 0.3f + 0.45f * (1.0f - d / huntDistance) * intensity[i];
            // the buzz is the climax: it is not denied by the niche
            if (play(i, ecosystem, out, clickPitch, accent, buzz, false, true))
                beginAct(buzz ? Act::buzz : Act::hunt);
            v.clickWait = 12;  // until the echo returns (or the hunter gives up waiting)
            break;
        }
        case Relation::flees:
        {
            const auto d = ecosystem.distanceBetween(i, p);
            // acoustic crypsis (Aguilar de Soto et al. 2020): a clever prey,
            // with the hunter close, falls silent instead of running
            if (d < 1.3f && o.intelligence > 0.55f && intensity[i] > 0.6f && nextUnit() < 0.04f)
            {
                // beaked whales fall silent TOGETHER (Aguilar de Soto et al.
                // 2020): those who hear this prey and are not hunters go
                // quiet with it, for the same time
                v.frozen = 24;
                for (std::size_t j = 0; j < count; ++j)
                    if (j != i && j != p && heard[j][i].age < 16 && tele.relation[j][i] != Relation::hunts)
                        voices[j].frozen = std::max<std::uint16_t>(voices[j].frozen, 24);
                beginAct(Act::freeze);
                break;
            }
            // flight: diminution, syncopation, a leap away from the hunter's register
            const auto shift = voices[p].lastMidi >= static_cast<float>(registerOf(o)) ? -7 : 12;
            if (sing(i, ecosystem, out, 0.5f, shift, true))
                beginAct(Act::flight);
            break;
        }
        case Relation::courts:
        {
            // addressing by copying (King & Janik 2013): the caller sings a
            // copy of the OTHER's signature - its name - and the one addressed
            // answers with its OWN signature, promptly; the closer the bond,
            // the shorter the gap. Both speak in the caller's coda and match
            // their rubato over the turns (CodaExchange; Sharma et al. 2024)
            const auto caller = std::min(i, p);
            if (i == caller)
            {
                if (v.cursor == 0 && !v.usingBorrowed)
                {
                    v.borrowed = voices[p].song;
                    v.usingBorrowed = true;
                }
            }
            else
            {
                if (v.cursor == 0 && heard[i][p].age < 8)
                    v.rest = static_cast<std::uint16_t>(std::min<float>(v.rest, (1.0f - intensity[i]) * 12.0f));
                // the rubato is matched turn by turn at the end of each coda
                // (CodaExchange), no longer at once
            }
            if (sing(i, ecosystem, out, 1.0f, 0, false))
                beginAct(Act::duet);
            break;
        }
        case Relation::rivals:
        {
            // hocket: a rival sounds only in the other's silence
            if (out.notes[p].on || voices[p].sinceNote < 1)
            {
                if (v.wait > 0) --v.wait;
                break;
            }
            // display escalates: the rivalry pushes the song upwards
            v.transpose = static_cast<std::int8_t>(intensity[i] > 0.7f ? std::max<int>(v.transpose, 2) : v.transpose);
            if (sing(i, ecosystem, out, 1.0f, 0, false))
                beginAct(Act::rivalry);
            break;
        }
        case Relation::guards:
        {
            // signals off the beat, a minor second against the intruder
            if (pos == 3u || pos == 6u || pos == 11u || pos == 14u)
            {
                const auto against = voices[p].lastMidi + 1.0f;
                auto midi = harmonise(i, against, out);
                while (midi < static_cast<float>(registerOf(o)) - 6.0f) midi += 12.0f;
                while (midi > static_cast<float>(registerOf(o)) + 12.0f) midi -= 12.0f;
                if (play(i, ecosystem, out, midi, 0.9f, false, true, false, 1.0f))
                    beginAct(Act::defence);
            }
            break;
        }
        case Relation::allied:
        {
            // the bubble net (humpback cooperative feeding): the one it follows
            // hunts, and this ally perceives the prey too - it spirals around
            // it, a rising arpeggio of thirds over two octaves, again and again
            {
                const auto prey = strongest(p, Relation::hunts);
                if (prey < count && prey != i && tele.bond[p][prey] > 0.4f
                    && (ecosystem.perceivedBy(i, prey) > 0.1f || heard[i][prey].age < 16))
                {
                    netNext[p] = true;
                    if (pos % 2u == 0u)
                    {
                        const auto turn = static_cast<int>(v.spiral % 7u);
                        v.spiral = static_cast<std::uint8_t>(v.spiral + 1u);
                        const auto midi = pitchFor(tele.chordDegree + 2 * turn, registerOf(o) - 5);
                        if (play(i, ecosystem, out, midi, 0.38f + 0.06f * static_cast<float>(turn), false, true, false, 1.5f))
                            beginAct(Act::bubbleNet);
                    }
                    break;
                }
            }
            // the school: cohesive, members sound with the one they follow
            // (homorhythm, consonant); in turbulent water a step late on its
            // pitch (heterophony)
            const auto followed = out.notes[p].on || voices[p].sinceNote == 1;
            if (followed && intensity[i] > 0.45f && nextUnit() < 0.75f)
            {
                const auto heterophony = medium.turbulence > 0.4f;
                if (heterophony && voices[p].sinceNote == 1)
                {
                    auto midi = voices[p].lastMidi;
                    while (midi < static_cast<float>(registerOf(o)) - 5.0f) midi += 12.0f;
                    while (midi > static_cast<float>(registerOf(o)) + 7.0f) midi -= 12.0f;
                    if (play(i, ecosystem, out, midi, 0.5f, false, false, false, 1.5f))
                        beginAct(Act::school);
                }
                else if (!heterophony && out.notes[p].on)
                {
                    const auto proposal = pitchFor(tele.chordDegree + 2 * static_cast<int>(nextUnit() * 3.0f), registerOf(o));
                    if (play(i, ecosystem, out, harmonise(i, proposal, out), 0.55f, false))
                        beginAct(Act::school);
                }
                break;
            }
            sing(i, ecosystem, out, 1.0f, 0, false);
            break;
        }
        case Relation::indifferent:
        {
            if (i == sessile)
            {
                // the keynote (Schafer): the anchor's pedal on the chord, every bar
                if (pos == 0u)
                {
                    play(i, ecosystem, out, pitchFor(tele.chordDegree, registerOf(o)), 0.75f, false, true, false, 10.0f);
                    break;
                }
                // the reef's memory: fragments of who left or was captured,
                // twice slower and an octave lower (decomposition as nutrient)
                if (memoryCountdown > 0)
                    --memoryCountdown;
                if (decomposedCount > 0 && memoryCountdown == 0)
                {
                    if (memoryWait > 0)
                        --memoryWait;
                    else
                    {
                        const auto& fragment = decomposed[(decomposedCount - 1u) % decomposed.size()];
                        const auto k = memoryCursor % fragment.length;
                        const auto proposal = pitchFor(tele.chordDegree + fragment.degrees[k], registerOf(o) + 12);
                        play(i, ecosystem, out, harmonise(i, proposal, out), 0.5f, false, true, false,
                             static_cast<float>(fragment.steps[k] * 2u));
                        memoryWait = static_cast<std::uint16_t>(fragment.steps[k] * 2u);
                        if (++memoryCursor >= std::max<std::uint8_t>(2, fragment.length / 2u))
                        {
                            memoryCursor = 0;
                            memoryCountdown = static_cast<std::uint16_t>((6u + static_cast<unsigned>(nextUnit() * 6.0f)) * 16u);
                        }
                    }
                    break;
                }
            }
            sing(i, ecosystem, out, 1.0f, 0, false);
            break;
        }
        }
    }
    if (tele.act != Act::grazing && !punctual(tele.act) && tele.tension < 0.1f && pos == 0u)
        beginAct(Act::grazing);
}
} // namespace aquorbium
