// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include "core/ChorusPhase.h"
#include "core/CodaExchange.h"
#include "core/LifeCycle.h"
#include "core/PopulationCycles.h"
#include "core/Ecosystem.h"
#include "core/HabitatHealth.h"
#include "core/RoomCoupling.h"
#include "core/SongForm.h"
#include "core/SoundChannel.h"
#include "core/SpectralConsonance.h"

#include <array>
#include <cstddef>
#include <cstdint>

namespace aquorbium
{
// The music comes from the ecosystem's concepts (ECOLOGICAL_MUSIC_DESIGN.md,
// 2026-10-20; the author: "a base dos acontecimentos são os conceitos do
// instrumento, pautado no ecossistema, a música vem desses conceitos"; "criar
// uma teia de relações, de ataques, fugas"; "harmonia, incompatibilidades";
// "densidade"; "estados líquidos"; "os sons que se propagam pelos líquidos";
// "frequências líquidas").
//
// Five layers, each one an ecological or physical concept with a musical
// consequence:
//
//   1. THE MEDIUM (liquid states): the water's phase - ice, cold, temperate,
//      turbulent, boiling - follows the season's temperature and the
//      organisms' agitation, and changes only after the latent heat is paid
//      (hysteresis: the phase change is a formal boundary, not a flicker).
//      Each phase sets viscosity (portamento), turbulence (rubato,
//      heterophony, fragmentation), crystallinity (a discrete lattice of
//      pitches and a strict grid), the speed of sound, absorption and bubbles.
//
//   2. PROPAGATION (sounds travelling through liquid): every note is a
//      wavefront that expands at the medium's speed of sound. An organism
//      HEARS it only when the front reaches it, attenuated by spreading and by
//      an absorption that grows with the square of the frequency (high notes
//      die near, low notes travel far), and shifted by the Doppler effect of
//      their relative motion. Imitation, answers and echoes are made of what
//      was heard, so the canon's delay IS the propagation time.
//
//   3. THE WEB OF RELATIONS: a directed 5x5 matrix. Each pair holds one
//      relation - hunt, flight, rivalry, alliance, courtship, guard or
//      indifference - with an intensity that grows fast and decays slowly (an
//      encounter leaves a memory). Relations come only from what the organism
//      perceived (vision) or heard (propagation): invariant 2 of the
//      ecosystem contract.
//
//   4. HARMONY AND INCOMPATIBILITY: each relation has an ecological
//      compatibility. When an organism chooses a pitch, compatible partners
//      pull it towards consonance with what they are sounding and
//      incompatible ones towards friction (the vertical interval is the
//      relation made audible). The incompatibilities accumulate as harmonic
//      tension: the chord leans to the dominant, the field admits chromatic
//      tones, and when the conflict dissolves the music cadences.
//
//   5. DENSITY: the acoustic niche has a carrying capacity (events per bar)
//      that grows logistically towards what the medium and the desire allow;
//      voices share it by salience (the hunter at the climax takes almost
//      everything: the reef goes quiet). Local crowding raises the voices
//      (the Lombard effect).
//
// Identity: every organism owns a SIGNATURE - a coda (a rhythm of onsets)
// and a whistle (a contour) - played with rubato and ornaments; allies that
// hear each other slowly copy units of each other's song (cultural
// transmission), a courting organism answers with its partner's signature
// (copying to address), and the sessile anchor keeps the fragments of who
// left or was captured and returns them, slower and lower (decomposition).
//
// Deterministic for a seed, fixed-size state, no allocation, lock or I/O:
// safe on the audio thread. With influence zero it emits nothing.
class EcologicalMusicDirector final
{
public:
    static constexpr std::size_t organismCount = Ecosystem::organismCount;
    static constexpr std::size_t signatureCapacity = 8;
    static constexpr std::size_t wavefrontCapacity = 32;

    struct Performance
    {
        float influence = 0.0f;  // 0 = the previous autonomous route, untouched
        float reaction = 0.6f;   // how strongly organisms react to what they perceive
        float memory = 0.5f;     // 0 = songs repeat, 1 = songs keep transforming
        float water = 0.5f;      // heat bias: 0.5 follows the season, 0 freezes, 1 boils
        float salinity = 1.0f;   // 0 fresh water .. 1 sea (35 ppt): the sea's chemistry absorbs
    };

    struct Note
    {
        bool on = false;
        float midi = 60.0f;          // fractional: Doppler and turbulence are microtonal
        float accent = 0.5f;         // 0..1
        float glideSeconds = 0.035f; // the medium's viscosity
        float durationSeconds = 0.25f; // how long it is held before it ends
        float releaseSeconds = 0.15f;  // how it ends
    };

    enum class Phase : std::uint8_t { ice, cold, temperate, turbulent, boiling };

    enum class Relation : std::uint8_t
    {
        indifferent,  // the niche: each keeps its band
        hunts,        // canon of what was heard, echolocation, terminal buzz
        flees,        // diminution, syncopation, leaps away - or freezes silent
        rivals,       // hocket and friction
        allied,       // homorhythm or heterophony, consonance, song copying
        courts,       // call and response with the partner's signature, then thirds
        guards        // signals off the beat against the intruder
    };
    static constexpr std::size_t relationCount = 7;

    enum class Act : std::uint8_t
    {
        grazing, hunt, buzz, capture, flight, freeze, rivalry, school, duet,
        defence, modulation, resolution, phaseChange, rupture, bubbles, departure,
        bubbleNet,   // cooperative hunt: an ally spirals around the prey, the hunter calls
        revolution,  // a new song sweeps through those who hear it
        songCycle,   // an organism's song (its themes in order) begins again
        death,       // a voice dies of age or hunger: its song goes to the reef
        birth,       // a voice is born, singing its parents' songs recombined
        soundmark    // the motif of this place returns (Schafer)
    };
    static constexpr std::size_t actCount = 22;

    struct Medium
    {
        float glideSeconds;   // viscosity: how liquid the pitch is
        float turbulence;     // rubato, heterophony, fragmentation (Kolmogorov's cascade)
        float crystal;        // 1 = a discrete lattice of pitches and a strict grid
        float soundSpeed;     // aquarium units per 16th step
        float absorption;     // factor on the physical absorption (bubbles scatter more)
        float bubbles;        // nucleation rate of Minnaert bubbles
        float capacity;       // carrying capacity of the acoustic niche, events per bar
        float tempoFactor;
        float legato;         // how much of its written length a note sounds
        float releaseSeconds; // how the note ends: ice stops dry, cold water lingers
    };

    struct Output
    {
        std::array<Note, organismCount> notes {};
        std::array<float, organismCount> level {};  // presence of each voice, 0..1.2
    };

    struct Telemetry
    {
        float tempoBpm = 0.0f;
        int centre = 0;          // pitch class of the harmonic centre
        int mode = 0;            // 0 phrygian .. 5 lydian
        int chordDegree = 0;
        std::uint32_t bar = 0;
        Phase phase = Phase::temperate;
        float heat = 0.5f;       // what drives the phase: temperature and agitation
        float agitation = 0.0f;  // the organisms' mean speed, normalised
        float temperatureC = 15.0f;  // the water's temperature, from the heat
        float latent = 0.0f;     // the latent heat paid towards the next phase, -1..1
        float tension = 0.0f;    // accumulated incompatibility, 0..1
        float capacity = 0.0f;   // events per bar the niche admits now
        std::uint8_t eventsLastBar = 0;
        Act act = Act::grazing;  // the most recent act
        std::array<std::uint32_t, actCount> actCounts {};
        std::array<std::array<Relation, organismCount>, organismCount> relation {};
        std::array<std::array<float, organismCount>, organismCount> bond {};
        std::array<std::uint8_t, organismCount> notesThisBar {};
        std::uint32_t wavefrontsHeard = 0;
        std::uint32_t songCopies = 0;
        std::array<float, organismCount> chorusPhase {};  // each organism's calling clock
        std::uint8_t threatened = 0;  // bit mask: who perceives a hunter at work
        bool bodyField = false;        // the scale was born from the bodies' partials
        std::uint32_t cloudMolecules = 0;  // glissandi of the thermal cloud (Xenakis)
        float prey = 1.0f;       // abundances (Lotka-Volterra), 1 = equilibrium
        float predators = 1.0f;
        float channelAxis = 0.0f;  // height of the deep sound channel (SoundChannel)
        // fronts that reached a listener / were heard: both ends in the
        // channel, and one end in it and the other far from it
        std::array<std::uint32_t, 2> reachedInChannel {}, reachedAcross {};
        // Schafer: the place's soundmark, and how hi-fi or lo-fi it sounds
        bool hasSoundmark = false;
        bool soundmarkChosen = false;       // chosen by the musician (MARCA)
        // CodaExchange: the rubato gap between partners at the first turns
        // and from the fourth on; ornaments on a change of subject or not
        float rubatoGapEarly = 0.0f, rubatoGapLate = 0.0f;
        std::uint32_t rubatoTurnsEarly = 0, rubatoTurnsLate = 0;
        std::uint32_t subjectChanges = 0, ornamentsOnChange = 0, steadyCodas = 0, ornamentsSteady = 0;
        SongForm::Phrase soundmark {};      // the place's motif, for the panel
        std::uint32_t soundmarkReturns = 0;
        float lofi = 0.0f;            // 0 hi-fi (distinct sounds) .. 1 lo-fi (a mass)
        HabitatHealth::Reading habitat {};  // ADI, ACI and health of what is heard
        RoomCoupling::Reading room {};      // the room, as one more organism (optional)
        std::uint8_t deniedLastBar = 0;  // the notes the niche had no room for
        // the voices' lives (LifeCycle): energy, age, generation
        std::array<float, organismCount> energy {};
        std::array<float, organismCount> age {};
        std::array<std::uint16_t, organismCount> generation {};
        std::array<std::int8_t, 7> field {};  // the seven pitch classes in use
    };

    void prepare(double controlRateHz) noexcept;
    void setSeed(std::uint32_t value) noexcept;
    void reset() noexcept;
    void setPerformance(Performance value) noexcept;
    [[nodiscard]] Performance performance() const noexcept { return perf; }

    // Once per control tick (AutonomousEnergy, 100 Hz).
    [[nodiscard]] Output tick(const Ecosystem& ecosystem) noexcept;
    [[nodiscard]] const Telemetry& telemetry() const noexcept { return tele; }
    // age darkens the body: 0 young .. 1 old (the engine closes its highs)
    [[nodiscard]] float darkness(std::size_t organism) const noexcept;

    // ---- the physics and the harmony, exposed for the tests -----------------
    [[nodiscard]] static Medium mediumFor(Phase phase) noexcept;
    // sensory consonance of an interval in semitones, 0 (rough) .. 1 (smooth)
    [[nodiscard]] static float consonance(float semitones) noexcept;
    // the partials of an organism's current sound (its body and second body),
    // for the consonance that depends on timbre and register
    void setSpectrum(std::size_t organism, const SpectralConsonance::Spectrum& spectrum) noexcept;
    // what the aquarium sounds like, measured (HabitatHealth): a healthy
    // habitat holds a fuller niche, a degraded one less
    void setHabitat(const HabitatHealth::Reading& reading) noexcept { tele.habitat = reading; }
    // the room heard through the microphone (RoomCoupling): the Lombard effect
    // answers its noise, and the voices leave the register it fills
    void setRoom(const RoomCoupling::Reading& reading) noexcept { tele.room = reading; }
    // the musician chooses the soundmark: the phrase sounding now (of the
    // voice that sang last) becomes the place's motif, and returns at once;
    // a chosen soundmark is not replaced by revolutions
    void chooseSoundmark() noexcept;
    // a soundmark kept from an earlier session (call after reset/prepare)
    void restoreSoundmark(const SongForm::Phrase& motif) noexcept;
    [[nodiscard]] static SpectralConsonance::Spectrum nativeSpectrum(Ecosystem::Species species) noexcept;
    // ecological compatibility of a relation, -1 (incompatible) .. 1
    [[nodiscard]] static float compatibility(Relation relation) noexcept;
    // Minnaert (1933): the resonance of a gas bubble in water, f ~ 3.26 / R
    // (Hz, R in metres, near the surface); returned as a MIDI pitch
    [[nodiscard]] static float minnaertMidi(float radiusMillimetres) noexcept;
    // spreading plus absorption that grows with the square of the frequency
    [[nodiscard]] static float receivedLevel(float distance, float midi, float absorption,
                                             float temperatureC = 15.0f, float salinityPpt = 35.0f) noexcept;
    // sound absorption in sea water (Ainslie & McColm 1998): boric acid and
    // magnesium sulphate relaxations plus pure water; dB per km
    [[nodiscard]] static float absorptionDbPerKm(float hz, float temperatureC, float salinityPpt,
                                                 float pH = 8.0f) noexcept;
    // the speed of sound in sea water at the surface (Mackenzie 1981), m/s
    [[nodiscard]] static float soundSpeedMs(float temperatureC, float salinityPpt) noexcept;
    [[nodiscard]] float waterTemperatureC() const noexcept { return tele.temperatureC; }
    // a speed drawn from the Maxwell-Boltzmann distribution: the length of a
    // 3D Gaussian vector of deviation sigma (six uniforms in (0, 1])
    [[nodiscard]] static float maxwellSpeed(float u1, float u2, float u3, float u4, float u5, float u6,
                                            float sigma) noexcept;
    [[nodiscard]] float salinityPpt() const noexcept { return 35.0f * perf.salinity; }
    // the timbre of a sound that crossed `waterPath` units of water: the
    // frequency where absorption (growing with f^2) costs 3 dB. The timbral
    // scale is gentler than the hearing scale between organisms (x0.01)
    [[nodiscard]] static float liquidCutoffHz(float absorption, float waterPath,
                                              float temperatureC = 15.0f, float salinityPpt = 35.0f) noexcept;
    // the medium now (smoothed), for the engine's timbre
    [[nodiscard]] float absorption() const noexcept { return medium.absorption; }
    // how much the water bubbles (0 calm .. ~0.6 boiling), for the BubbleField
    [[nodiscard]] float bubbling() const noexcept { return medium.bubbles; }

private:
    // a phrase of a song: units (degree + duration); the song's hierarchy of
    // phrases, themes and song is SongForm (Payne & McVay 1971)
    using Signature = SongForm::Phrase;
    static_assert(signatureCapacity == SongForm::capacity);

    struct Wavefront
    {
        bool live = false;
        std::uint8_t source = 0;
        std::uint8_t reached = 0;   // bit mask of listeners already reached
        bool click = false;         // an echolocation click (its echo returns to the source)
        bool echo = false;
        float midi = 60.0f;
        float accent = 0.5f;
        float originX = 0.0f, originY = 0.0f, originZ = 0.0f;
        float radius = 0.0f;        // aquarium units travelled so far
    };

    struct Heard
    {
        float level = 0.0f;   // decays step by step
        float midi = 60.0f;   // with the Doppler shift
        std::uint16_t age = 0xffffu;
    };

    struct Voice
    {
        Signature song {};            // the phrase being sung now (from the form)
        SongForm form {};             // its song: themes of repeated phrases, in order
        Signature borrowed {};        // a partner's signature, for copying to address
        bool usingBorrowed = false;
        std::uint8_t cursor = 0;
        std::uint16_t wait = 0;
        std::uint16_t rest = 0;
        std::int8_t transpose = 0;
        bool inverted = false;
        float rubato = 1.0f;          // the phrase's tempo, drifting between phrases
        float lastMidi = 60.0f;
        std::uint16_t sinceNote = 0xffffu;
        std::uint8_t used = 0;        // events spent in this bar
        std::uint8_t quota = 0;
        std::uint16_t clickWait = 0;
        std::uint16_t silenced = 0;   // a captured prey is silent for a while
        std::uint16_t frozen = 0;     // acoustic crypsis
        std::uint8_t bubblesLeft = 0;
        float bubbleRadius = 3.0f;
        float lastPresence = 1.0f;    // smoothed presence: the voice level
        std::uint8_t phraseEnd = 0;   // a fragment ends early (turbulence's cascade)
        bool ornament = false;        // an extra unit at the end of the coda
        std::uint8_t buzzClicks = 0;
        bool wasLeaving = false;
        bool wasEntering = false;
        bool phraseAdmitted = false;  // the niche admitted this whole phrase
        std::uint8_t reserved = 0;    // its units, already counted in the niche
        bool adopted = false;         // took the current revolution's song
        std::uint8_t spiral = 0;      // position in the bubble-net spiral
        LifeCycle::Life life {};      // energy, age, generation
        bool unborn = false;          // dead, waiting to be born again
        // CodaExchange: the identity rhythm, the tempo it calls in, and the
        // exchange it is in (partner, turn, the relation it last spoke in)
        std::uint8_t codaRhythm = 0;
        std::uint8_t codaTempo = 1;
        int exchangePartner = -1;
        int lastPartner = -1;
        std::uint8_t exchangeTurn = 0;
        Relation currentRelation = Relation::indifferent;
        Relation lastRelation = Relation::indifferent;
    };

    [[nodiscard]] float nextUnit() noexcept;
    void newSignature(Signature& signature, Ecosystem::Species species, float intelligence) noexcept;
    void newSong(Voice& voice, Ecosystem::Species species, float intelligence) noexcept;
    void updateMedium(const Ecosystem& ecosystem, float dt) noexcept;
    void propagate(const Ecosystem& ecosystem) noexcept;
    std::size_t emit(const Ecosystem& ecosystem, std::size_t source, float midi, float accent, bool click, bool echo) noexcept;
    void updateWeb(const Ecosystem& ecosystem) noexcept;
    void updateHarmony(const Ecosystem& ecosystem) noexcept;
    void allocateDensity(const Ecosystem& ecosystem) noexcept;
    void step(const Ecosystem& ecosystem, Output& out) noexcept;
    void beginAct(Act act) noexcept;

    [[nodiscard]] float pitchFor(int degree, int registerBase) const noexcept;
    [[nodiscard]] bool inField(float midi, bool chromaticAllowed) const noexcept;
    [[nodiscard]] float harmonise(std::size_t voice, float proposal, const Output& out,
                                  float stickiness = 0.3f) const noexcept;
    [[nodiscard]] float pairCompatibility(std::size_t a, std::size_t b) const noexcept;
    [[nodiscard]] std::size_t strongest(std::size_t organism, Relation relation) const noexcept;
    [[nodiscard]] Relation dominant(std::size_t organism, float& intensity, std::size_t& partner) const noexcept;
    bool sing(std::size_t voice, const Ecosystem& ecosystem, Output& out, float durationScale, int registerShift,
              bool syncopate) noexcept;
    bool play(std::size_t voice, const Ecosystem& ecosystem, Output& out, float midi, float accent,
              bool structural, bool tempered = true, bool click = false, float lengthSteps = 2.0f) noexcept;

    Performance perf {};
    Telemetry tele {};
    Medium medium = mediumFor(Phase::temperate);
    double controlRate = 100.0;
    std::uint32_t seed = 0x6ea1b10u;
    std::uint32_t randomState = 0x6ea1b10u;
    double stepPhase = 0.0;
    std::uint32_t stepIndex = 0;
    int regionSector = -1;
    int pendingSector = -1;
    std::uint16_t sectorDwell = 0;
    std::uint32_t lastModulationBar = 0;
    int modeIndex = 3;
    float peakTension = 0.0f;
    bool cadencePending = false;
    float capacity = 6.0f;
    std::uint8_t eventsThisBar = 0;
    std::uint8_t cloudThisBar = 0;  // the thermal cloud's own budget
    std::uint16_t ruptureSteps = 0;
    std::uint16_t fermataSteps = 0;
    bool wasCracked = false;
    bool started = false;
    // cooperative hunting: hunters whose allies are spiralling around the prey
    std::array<bool, organismCount> netForHunter {};
    std::array<bool, organismCount> netNext {};
    // song revolution: whose new song is spreading, and for how many bars
    // the temporal niche: each organism's calling clock, coupled by hearing
    ChorusPhase chorus {};
    // Schafer: the soundmark (the motif of this aquarium) and the lo-fi
    Signature soundmark {};
    bool soundmarkPending = false;
    std::uint32_t lastSoundmarkBar = 0;  // returns at least 32 bars apart
    void callSoundmark() noexcept;
    std::uint8_t soundmarkCursor = 0;
    std::uint16_t soundmarkWait = 0;
    std::uint8_t deniedThisBar = 0;
    void markPlace(const Signature& motif) noexcept;
    void playSoundmark(const Ecosystem& ecosystem, Output& out, std::size_t keeper) noexcept;
    // the populations: abundance cycles of minutes (Lotka-Volterra)
    PopulationCycles populations {};
    std::array<bool, organismCount> callReady {};
    std::array<SpectralConsonance::Spectrum, organismCount> spectra {};
    std::array<float, organismCount> inharmonicity {};
    bool spectraSet = false;
    std::array<int, 7> field { 0, 2, 4, 5, 7, 9, 10 };  // the active scale (a mode, or the bodies')
    void updateField() noexcept;
    std::array<std::uint8_t, organismCount> chorusCycles {};
    int revolutionSource = -1;
    std::uint16_t revolutionBars = 0;
    void beginRevolution(const Ecosystem& ecosystem, std::size_t source) noexcept;
    void advanceChorus(const Ecosystem& ecosystem) noexcept;
    void liveBar(const Ecosystem& ecosystem) noexcept;
    void die(std::size_t voice) noexcept;
    void beBorn(const Ecosystem& ecosystem, std::size_t voice) noexcept;
    std::array<Voice, organismCount> voices {};
    std::array<Wavefront, wavefrontCapacity> fronts {};
    std::array<std::array<Heard, organismCount>, organismCount> heard {};  // heard[listener][source]
    std::array<Signature, 2> decomposed {};  // fragments of who left or was captured
    std::uint8_t decomposedCount = 0;
    std::uint16_t memoryCountdown = 0;
    std::uint8_t memoryCursor = 0;
    std::uint16_t memoryWait = 0;
};
}
