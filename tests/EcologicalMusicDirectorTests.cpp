// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

// The ecological music director (2026-10-20): the physics and the harmony it
// rests on, influence zero as a true bypass, determinism, and a whole season
// (300 s) of a real ecosystem - the water changes state, the web produces
// hunts, the niche's capacity is respected, ice crystallises the pitches and
// every note stays in range.
#include "core/BiomaBrain.h"
#include "core/BiomaEngine.h"
#include "core/EcologicalMusicDirector.h"
#include "core/Ecosystem.h"

#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <set>
#include <vector>

namespace
{
using Director = aquorbium::EcologicalMusicDirector;

int failures = 0;
void check(bool condition, const char* what)
{
    if (!condition)
    {
        std::cerr << "FAILED: " << what << '\n';
        ++failures;
    }
}

struct Season
{
    std::uint64_t hash = 1469598103934665603ull;
    std::size_t notes = 0;
    std::size_t outOfRange = 0;
    std::size_t iceNotes = 0;
    std::size_t iceTempered = 0;
    std::size_t iceBubbles = 0;
    std::size_t overCapacityBars = 0;
    // Schafer's hi-fi / lo-fi: notes in a crowded niche against a clear one
    double lofiDuration = 0.0, hifiDuration = 0.0, lofiAccent2 = 0.0, hifiAccent2 = 0.0;
    double lofiAccent = 0.0, hifiAccent = 0.0;
    std::size_t lofiNotes = 0, hifiNotes = 0;
    double accentSum = 0.0;
    std::size_t nearRoom = 0;  // notes within 3 semitones of MIDI 62
    std::set<int> phases;
    // the niche by the prey's abundance (Lotka-Volterra)
    // (within each phase of the water, which moves the niche more)
    std::array<double, 5> richCapacity {}, poorCapacity {};
    std::array<std::size_t, 5> richBars {}, poorBars {};
    Director::Telemetry last {};
};

void mix(std::uint64_t& hash, std::uint64_t value)
{
    hash ^= value;
    hash *= 1099511628211ull;
}

Season run(float influence, double seconds, std::uint32_t seed, float memory = 0.5f, float habitatHealth = -1.0f,
           float roomDb = 1.0f)
{
    constexpr double sampleRate = 48000.0;
    constexpr std::size_t controlInterval = 480;  // 100 Hz, as in AutonomousEnergy
    aquorbium::BiomaBrain brain;
    brain.prepare(sampleRate);
    brain.setSeed(seed);
    aquorbium::Ecosystem ecosystem;
    ecosystem.prepare(sampleRate);
    ecosystem.reset(brain);
    Director director;
    director.prepare(sampleRate / static_cast<double>(controlInterval));
    director.setSeed(seed ^ 0x2c1b3c6du);
    director.setPerformance({ influence, 0.6f, memory });
    aquorbium::HabitatHealth::Reading habitat {};
    habitat.health = habitatHealth;
    habitat.measured = habitatHealth >= 0.0f;
    aquorbium::RoomCoupling::Reading room {};
    room.active = roomDb <= 0.0f;
    room.levelDb = roomDb;
    room.strength = std::clamp((roomDb + 60.0f) / 40.0f, 0.0f, 1.0f);
    room.midi = 62.0f;

    Season season;
    std::uint32_t bar = 0;
    int lastPhase = -1;
    int barsInPhase = 0;
    const auto samples = static_cast<std::size_t>(sampleRate * seconds);
    for (std::size_t sample = 0; sample < samples; ++sample)
    {
        ecosystem.tickSample(brain);
        if (sample % controlInterval != 0)
            continue;
        if (habitat.measured)
            director.setHabitat(habitat);  // as the engine does at control rate
        if (room.active)
            director.setRoom(room);
        const auto out = director.tick(ecosystem);
        const auto& t = director.telemetry();
        if (t.bar != bar)
        {
            bar = t.bar;
            // structural events (captures, cadences, the buzz, a change of
            // state) may exceed the share; ordinary singing may not
            if (static_cast<float>(t.eventsLastBar) > std::ceil(t.capacity) + 8.0f)
                ++season.overCapacityBars;
            const auto ph = static_cast<std::size_t>(t.phase);
            barsInPhase = static_cast<int>(ph) == lastPhase ? barsInPhase + 1 : 0;
            lastPhase = static_cast<int>(ph);
            if (barsInPhase < 6)
                ;  // the niche is still settling after a change of state
            else if (t.prey > 1.2f)
            {
                season.richCapacity[ph] += t.capacity;
                ++season.richBars[ph];
            }
            else if (t.prey < 0.85f)
            {
                season.poorCapacity[ph] += t.capacity;
                ++season.poorBars[ph];
            }
        }
        season.phases.insert(static_cast<int>(t.phase));
        for (std::size_t i = 0; i < out.notes.size(); ++i)
        {
            const auto& note = out.notes[i];
            if (!note.on)
                continue;
            ++season.notes;
            season.accentSum += note.accent;
            if (std::abs(note.midi - 62.0f) < 3.0f)
                ++season.nearRoom;
            if (t.lofi > 0.4f)
            {
                season.lofiDuration += note.durationSeconds;
                season.lofiAccent += note.accent;
                season.lofiAccent2 += static_cast<double>(note.accent) * note.accent;
                ++season.lofiNotes;
            }
            else if (t.lofi < 0.1f)
            {
                season.hifiDuration += note.durationSeconds;
                season.hifiAccent += note.accent;
                season.hifiAccent2 += static_cast<double>(note.accent) * note.accent;
                ++season.hifiNotes;
            }
            mix(season.hash, i);
            mix(season.hash, static_cast<std::uint64_t>(std::lround(note.midi * 100.0f)));
            mix(season.hash, sample);
            if (note.midi < 24.0f || note.midi > 96.0f || note.accent < 0.0f || note.accent > 1.0f
                || !(note.glideSeconds > 0.0f))
                ++season.outOfRange;
            if (t.phase == Director::Phase::ice)
            {
                ++season.iceNotes;
                if (std::abs(note.midi - std::round(note.midi)) < 1.0e-4f || note.glideSeconds <= 0.0041f)
                    ++season.iceTempered;
                if (t.act == Director::Act::bubbles)
                    ++season.iceBubbles;
            }
        }
    }
    season.last = director.telemetry();
    return season;
}
}

int main()
{
    // ---- physics and harmony ----
    check(Director::consonance(0.0f) == 1.0f, "unison is fully consonant");
    check(Director::consonance(12.0f) > 0.99f, "the octave is consonant");
    check(Director::consonance(7.0f) > Director::consonance(4.0f), "the fifth is smoother than the third");
    check(Director::consonance(4.0f) > Director::consonance(6.0f), "the third is smoother than the tritone");
    check(Director::consonance(1.0f) < 0.1f && Director::consonance(11.0f) < 0.1f, "seconds and sevenths are rough");
    check(Director::consonance(-7.0f) == Director::consonance(7.0f), "consonance ignores direction");
    check(Director::compatibility(Director::Relation::courts) > 0.0f
              && Director::compatibility(Director::Relation::allied) > 0.0f,
          "courtship and alliance are compatible");
    check(Director::compatibility(Director::Relation::hunts) < 0.0f
              && Director::compatibility(Director::Relation::flees) < 0.0f
              && Director::compatibility(Director::Relation::rivals) < 0.0f,
          "hunt, flight and rivalry are incompatible");
    // Minnaert: a 3 mm bubble rings near 1.09 kHz (MIDI ~84.6); smaller is higher
    check(std::abs(Director::minnaertMidi(3.0f) - 84.65f) < 0.1f, "Minnaert pitch of a 3 mm bubble");
    check(Director::minnaertMidi(1.5f) > Director::minnaertMidi(3.0f), "a smaller bubble is higher");
    check(Director::receivedLevel(3.0f, 60.0f, 1.0f) < Director::receivedLevel(1.0f, 60.0f, 1.0f),
          "sound weakens with distance");
    check(Director::receivedLevel(3.0f, 84.0f, 1.0f) < 0.25f * Director::receivedLevel(3.0f, 48.0f, 1.0f),
          "high notes are absorbed far more than low ones");
    const auto ice = Director::mediumFor(Director::Phase::ice);
    const auto cold = Director::mediumFor(Director::Phase::cold);
    const auto temperate = Director::mediumFor(Director::Phase::temperate);
    const auto turbulent = Director::mediumFor(Director::Phase::turbulent);
    const auto boiling = Director::mediumFor(Director::Phase::boiling);
    check(ice.crystal == 1.0f && ice.bubbles == 0.0f, "ice is a crystal without bubbles");
    check(ice.soundSpeed > temperate.soundSpeed && temperate.soundSpeed > boiling.soundSpeed,
          "sound is fastest in ice and slowest in bubbly water");
    check(ice.capacity < cold.capacity && cold.capacity < temperate.capacity
              && temperate.capacity < turbulent.capacity,
          "the niche's capacity grows with the heat");
    check(cold.glideSeconds > temperate.glideSeconds && ice.glideSeconds < 0.01f,
          "cold water is viscous; ice has no portamento");
    check(turbulent.turbulence > temperate.turbulence && boiling.bubbles > 0.0f,
          "turbulence and bubbles where they belong");

    // ---- liquid frequencies: the timbre after crossing water ----
    check(Director::liquidCutoffHz(1.0f, 6.0f) < Director::liquidCutoffHz(1.0f, 1.0f),
          "the farther through water, the darker");
    check(Director::liquidCutoffHz(boiling.absorption, 4.0f) < Director::liquidCutoffHz(ice.absorption, 4.0f),
          "bubbly water is darker than ice");
    check(Director::liquidCutoffHz(1.0f, 0.0f) == 16000.0f && Director::liquidCutoffHz(50.0f, 8.0f) == 500.0f,
          "the cutoff stays within 500 Hz .. 16 kHz");

    // ---- Maxwell-Boltzmann speeds (Xenakis's thermal cloud) ----
    {
        std::uint32_t state = 12345u;
        const auto uniform = [&state] {
            state = state * 1664525u + 1013904223u;
            return (static_cast<float>(state >> 8u) + 1.0f) / 16777217.0f;
        };
        const auto sample = [&](float sigma, int n, int& belowSigma) {
            double sum = 0.0;
            belowSigma = 0;
            for (int k = 0; k < n; ++k)
            {
                const auto v = Director::maxwellSpeed(uniform(), uniform(), uniform(), uniform(), uniform(), uniform(), sigma);
                sum += v;
                belowSigma += v < sigma ? 1 : 0;
            }
            return sum / n;
        };
        int below = 0;
        const auto mean = sample(1.0f, 40000, below);
        check(std::abs(mean - 1.5958) < 0.03, "the mean speed is 2 sqrt(2/pi) sigma");
        check(std::abs(below / 40000.0 - 0.1987) < 0.01, "P(v < sigma) matches the Maxwell distribution");
        int unused = 0;
        const auto hot = sample(std::sqrt(373.15f / 288.15f), 40000, unused);
        check(std::abs(hot / mean - std::sqrt(373.15 / 288.15)) < 0.03, "hotter water: speeds scale with the square root of T");
    }

    // ---- the sea's chemistry and the speed of sound (Ainslie & McColm; Mackenzie) ----
    check(std::abs(Director::soundSpeedMs(15.0f, 35.0f) - 1507.0f) < 3.0f, "sound runs ~1507 m/s in 15 C sea water");
    check(Director::soundSpeedMs(4.0f, 35.0f) < Director::soundSpeedMs(25.0f, 35.0f), "sound is slower in colder water");
    check(Director::absorptionDbPerKm(10000.0f, 15.0f, 35.0f) > 0.5f && Director::absorptionDbPerKm(10000.0f, 15.0f, 35.0f) < 1.5f,
          "about 1 dB/km at 10 kHz in temperate sea water");
    check(Director::absorptionDbPerKm(3000.0f, 15.0f, 0.0f) < 0.1f * Director::absorptionDbPerKm(3000.0f, 15.0f, 35.0f),
          "fresh water, without the sea's chemistry, absorbs far less");
    check(Director::liquidCutoffHz(1.0f, 4.0f, 15.0f, 0.0f) > Director::liquidCutoffHz(1.0f, 4.0f, 15.0f, 35.0f),
          "fresh water sounds brighter than the sea");

    // ---- in the engine: music at zero leaves the sound untouched; on, it plays ----
    {
        const auto render = [](float influence) {
            aquorbium::BiomaEngine engine;
            engine.prepare(48000.0);
            engine.setSeed(0x815ab1eu);
            engine.setEcosystemBlendImmediately(1.0f);
            engine.setAutonomousEnabled(true);
            engine.setEcologicalMusicPerformance({ influence, 0.6f, 0.5f, 0.5f });
            std::vector<float> left(256), right(256), all;
            for (int block = 0; block < 48000 * 6 / 256; ++block)
            {
                engine.render(left.data(), right.data(), 256);
                all.insert(all.end(), left.begin(), left.end());
            }
            return all;
        };
        const auto off = render(0.0f);
        const auto offAgain = render(0.0f);
        const auto on = render(1.0f);
        check(off == offAgain, "music at zero: the engine is deterministic");
        double energy = 0.0;
        bool finite = true;
        for (const auto sample : on)
        {
            finite = finite && std::isfinite(sample);
            energy += static_cast<double>(sample) * sample;
        }
        check(finite && energy > 1.0e-6, "music on: finite and audible");
        check(on != off, "music on changes the sound");
    }

    // ---- the scale is born from the bodies (Sethares 1993) ----
    {
        const auto fieldOf = [](bool membrane) {
            aquorbium::BiomaBrain brain;
            brain.prepare(48000.0);
            brain.setSeed(3u);
            aquorbium::Ecosystem eco;
            eco.prepare(48000.0);
            eco.reset(brain);
            Director d;
            d.prepare(100.0);
            d.setSeed(5u);
            d.setPerformance({ 1.0f, 0.6f, 0.5f, 0.5f });
            for (std::size_t i = 0; i < 5; ++i)
                d.setSpectrum(i, membrane ? aquorbium::SpectralConsonance::membrane()
                                          : aquorbium::SpectralConsonance::harmonic(6));
            for (long s = 0; s < 48000L * 10; ++s)
            {
                eco.tickSample(brain);
                if (s % 480 == 0)
                    static_cast<void>(d.tick(eco));
            }
            return d.telemetry();
        };
        const auto harmonicField = fieldOf(false);
        const auto membraneField = fieldOf(true);
        check(!harmonicField.bodyField, "harmonic bodies keep the mode");
        check(membraneField.bodyField, "inharmonic bodies give their own scale");
        bool hasMinorSixth = false;
        for (const auto pc : membraneField.field)
            hasMinorSixth = hasMinorSixth || pc == 8;
        check(hasMinorSixth, "a membrane's scale holds the minor sixth (its mode at ~1.593)");
    }

    // ---- HabitatHealth: a healthy soundscape holds a fuller niche ----
    {
        const auto degraded = run(1.0f, 60.0, 0x815ab1eu, 0.5f, 0.1f);
        const auto healthy = run(1.0f, 60.0, 0x815ab1eu, 0.5f, 0.9f);
        std::cout << "habitat: niche " << healthy.last.capacity << " healthy vs " << degraded.last.capacity
                  << " degraded\n";
        check(healthy.last.capacity > degraded.last.capacity * 1.2f, "a healthy habitat holds a fuller niche");
    }

    // ---- RoomCoupling: a loud room raises the voices and empties its register ----
    {
        const auto quiet = run(1.0f, 120.0, 0x815ab1eu);
        const auto loud = run(1.0f, 120.0, 0x815ab1eu, 0.5f, -1.0f, -35.0f);
        const auto accentQuiet = quiet.accentSum / static_cast<double>(std::max<std::size_t>(1, quiet.notes));
        const auto accentLoud = loud.accentSum / static_cast<double>(std::max<std::size_t>(1, loud.notes));
        const auto nearQuiet = static_cast<double>(quiet.nearRoom) / static_cast<double>(std::max<std::size_t>(1, quiet.notes));
        const auto nearLoud = static_cast<double>(loud.nearRoom) / static_cast<double>(std::max<std::size_t>(1, loud.notes));
        std::cout << "room: accent " << accentLoud << " (loud room) vs " << accentQuiet << "; notes near its register "
                  << nearLoud * 100.0 << "% vs " << nearQuiet * 100.0 << "%\n";
        check(accentLoud > accentQuiet * 1.1, "the Lombard effect answers the room");
        check(nearLoud < nearQuiet * 0.5, "the voices leave the register the room fills");
    }

    // ---- the musician's soundmark (MARCA) ----
    {
        constexpr double sampleRate = 48000.0;
        aquorbium::BiomaBrain brain;
        brain.prepare(sampleRate);
        brain.setSeed(0x815ab1eu);
        aquorbium::Ecosystem ecosystem;
        ecosystem.prepare(sampleRate);
        ecosystem.reset(brain);
        Director director;
        director.prepare(100.0);
        director.setSeed(0x815ab1eu ^ 0x2c1b3c6du);
        director.setPerformance({ 1.0f, 0.6f, 1.0f });  // full MEMORY: revolutions are frequent
        const auto live = [&](double seconds) {
            for (std::size_t sample = 0; sample < static_cast<std::size_t>(sampleRate * seconds); ++sample)
            {
                ecosystem.tickSample(brain);
                if (sample % 480 == 0)
                    static_cast<void>(director.tick(ecosystem));
            }
        };
        live(20.0);
        director.chooseSoundmark();
        const auto chosen = director.telemetry().soundmark;
        check(director.telemetry().soundmarkChosen && chosen.length >= 2, "the musician marks the phrase sounding now");
        const auto returnsBefore = director.telemetry().soundmarkReturns;
        live(5.0);
        check(director.telemetry().soundmarkReturns > returnsBefore, "the chosen soundmark is heard at once");
        live(300.0);
        const auto after = director.telemetry().soundmark;
        bool same = after.length == chosen.length;
        for (std::size_t u = 0; same && u < chosen.length; ++u)
            same = after.degrees[u] == chosen.degrees[u] && after.steps[u] == chosen.steps[u];
        check(same, "revolutions do not replace a chosen soundmark");

        Director again;
        again.prepare(100.0);
        again.setSeed(0x1234567u);
        again.restoreSoundmark(chosen);
        const auto restored = again.telemetry().soundmark;
        bool kept = restored.length == chosen.length && again.telemetry().soundmarkChosen;
        for (std::size_t u = 0; kept && u < chosen.length; ++u)
            kept = restored.degrees[u] == chosen.degrees[u];
        check(kept, "a soundmark kept from an earlier session is restored");
        aquorbium::SongForm::Phrase wild {};
        wild.length = 3;
        wild.degrees = { 40, -40, 2 };
        wild.steps = { 0, 200, 4 };
        again.restoreSoundmark(wild);
        const auto clean = again.telemetry().soundmark;
        check(clean.degrees[0] == 7 && clean.degrees[1] == -5 && clean.steps[0] == 1 && clean.steps[1] == 16,
              "a restored soundmark is kept in range");
    }

    // ---- influence zero is a bypass ----
    const auto bypass = run(0.0f, 20.0, 0x815ab1eu);
    check(bypass.notes == 0, "influence zero emits no note");

    // ---- determinism ----
    const auto a = run(1.0f, 40.0, 0x815ab1eu);
    const auto b = run(1.0f, 40.0, 0x815ab1eu);
    check(a.notes > 0, "the director plays");
    check(a.hash == b.hash && a.notes == b.notes, "the same seed gives the same music");
    const auto c = run(1.0f, 40.0, 0x1234567u);
    check(c.hash != a.hash, "another seed gives other music");

    // ---- a whole season ----
    const auto season = run(1.0f, 300.0, 0x815ab1eu);
    const auto& acts = season.last.actCounts;
    const auto act = [&](Director::Act which) { return acts[static_cast<std::size_t>(which)]; };
    check(season.outOfRange == 0, "every note in range, with a valid accent and glide");
    check(season.phases.size() >= 3, "the water passes through at least three states in a season");
    check(act(Director::Act::phaseChange) >= 2, "the changes of state are events");
    check(season.last.cloudMolecules > 0, "turbulent and boiling water make a thermal cloud of glissandi");
    check(act(Director::Act::hunt) + act(Director::Act::buzz) + act(Director::Act::flight) > 0,
          "the web produces hunts or flights");
    check(season.last.wavefrontsHeard > 100, "the organisms hear each other through the water");
    check(season.overCapacityBars == 0, "no bar overflows the niche's capacity");
    // CodaExchange (Sharma et al.): partners match their rubato over the
    // turns, and ornaments cluster where the subject changes
    {
        const auto& t = season.last;
        // the rubato gap summed over five seasons: one season gave only 14
        // to 17 turns from the fourth, and which exchanges a chaotic season
        // produces moves with the last bit of the arithmetic (the macOS arm64
        // build, with fused multiply-add, measured no fall at all, 2026-10-26).
        // The convergence itself is checked turn by turn in CodaExchangeTests.
        double gapEarly = t.rubatoGapEarly, gapLate = t.rubatoGapLate;
        unsigned turnsEarly = t.rubatoTurnsEarly, turnsLate = t.rubatoTurnsLate;
        for (const auto more : { 0x5eed0001u, 0x5eed0002u, 0x5eed0003u, 0x5eed0004u })
        {
            const auto extra = run(1.0f, 300.0, more).last;
            gapEarly += extra.rubatoGapEarly;
            gapLate += extra.rubatoGapLate;
            turnsEarly += extra.rubatoTurnsEarly;
            turnsLate += extra.rubatoTurnsLate;
        }
        const auto early = turnsEarly > 0 ? static_cast<float>(gapEarly / turnsEarly) : 0.0f;
        const auto late = turnsLate > 0 ? static_cast<float>(gapLate / turnsLate) : 0.0f;
        const auto onChange = static_cast<float>(t.ornamentsOnChange) / static_cast<float>(std::max(1u, t.subjectChanges));
        const auto steady = static_cast<float>(t.ornamentsSteady) / static_cast<float>(std::max(1u, t.steadyCodas));
        std::cout << "codas: rubato gap " << early << " at the first turns (" << turnsEarly << ", five seasons), " << late
                  << " from the fourth (" << turnsLate << "); ornaments " << onChange * 100.0f
                  << "% on a change of subject vs " << steady * 100.0f << "% steady\n";
        check(t.rubatoTurnsEarly >= 10 && t.rubatoTurnsLate >= 10, "the season has exchanges of several turns");
        check(turnsLate >= 50, "five seasons give enough late turns to measure");
        check(late < early * 0.6f, "the rubato gap between partners falls over the dialogue");
        check(onChange > 2.5f * steady, "ornaments mark changes of subject");
    }
    // Schafer: the place gets its soundmark and it returns
    check(season.last.hasSoundmark, "the place has a soundmark");
    check(season.last.soundmarkReturns >= 1, "the soundmark returns after cadences and changes of state");
    std::cout << "soundmark returns: " << season.last.soundmarkReturns << "; lo-fi notes " << season.lofiNotes
              << ", hi-fi notes " << season.hifiNotes << '\n';
    if (season.lofiNotes > 20 && season.hifiNotes > 20)
    {
        const auto sd = [](double sum, double sum2, std::size_t n) {
            const auto m = sum / static_cast<double>(n);
            return std::sqrt(std::max(0.0, sum2 / static_cast<double>(n) - m * m));
        };
        const auto lofiDur = season.lofiDuration / static_cast<double>(season.lofiNotes);
        const auto hifiDur = season.hifiDuration / static_cast<double>(season.hifiNotes);
        const auto lofiSd = sd(season.lofiAccent, season.lofiAccent2, season.lofiNotes);
        const auto hifiSd = sd(season.hifiAccent, season.hifiAccent2, season.hifiNotes);
        std::cout << "lo-fi: duration " << lofiDur << " s vs " << hifiDur << " s; accent spread " << lofiSd << " vs "
                  << hifiSd << '\n';
        check(lofiDur > hifiDur * 1.15, "in a crowded niche the notes fuse (longer)");
        check(lofiSd < hifiSd * 0.85, "in a crowded niche the accents flatten");
    }
    else
        check(false, "the season has both crowded and clear bars");
    // Lotka-Volterra: abundant prey fill the niche, scarce prey empty it
    // (fifteen minutes with short memory: about seven cycles of two minutes)
    {
        const auto long_ = run(1.0f, 900.0, 0x815ab1eu, 0.0f);
        std::size_t compared = 0, fuller = 0, rich = 0, poor = 0;
        for (std::size_t ph = 0; ph < 5; ++ph)
        {
            rich += long_.richBars[ph];
            poor += long_.poorBars[ph];
            if (long_.richBars[ph] < 4 || long_.poorBars[ph] < 4)
                continue;
            const auto r = long_.richCapacity[ph] / static_cast<double>(long_.richBars[ph]);
            const auto q = long_.poorCapacity[ph] / static_cast<double>(long_.poorBars[ph]);
            std::cout << "phase " << ph << ": niche " << r << " (prey abundant) vs " << q << " (scarce)\n";
            ++compared;
            if (r > q * 1.1)
                ++fuller;
        }
        check(rich > 10 && poor > 10, "the populations cycle within a season");
        check(compared > 0 && fuller == compared, "in each state of the water, abundant prey make a fuller niche");
        // LifeCycle: in fifteen minutes voices die of age or hunger and are
        // born again with their parents' songs recombined
        const auto deaths = long_.last.actCounts[static_cast<std::size_t>(Director::Act::death)];
        const auto births = long_.last.actCounts[static_cast<std::size_t>(Director::Act::birth)];
        std::uint16_t generation = 0;
        for (const auto g : long_.last.generation)
            generation = std::max(generation, g);
        std::cout << "life: " << deaths << " deaths, " << births << " births, generation " << generation << '\n';
        check(deaths >= 2 && births >= 1, "voices die and are born within fifteen minutes");
        check(births <= deaths && deaths - births <= 5, "each death is followed by a birth");
        check(generation >= 1, "the generations advance");
        // SoundChannel: at the same range, a sound crosses into the shadow
        // far less often than it travels along the channel
        const auto& t = long_.last;
        const auto inChannel = static_cast<double>(t.reachedInChannel[1]) / std::max(1u, t.reachedInChannel[0]);
        const auto across = static_cast<double>(t.reachedAcross[1]) / std::max(1u, t.reachedAcross[0]);
        std::cout << "channel: heard " << inChannel * 100.0 << "% along it, " << across * 100.0 << "% across it\n";
        check(t.reachedInChannel[0] > 20 && t.reachedAcross[0] > 20, "sounds travel along and across the channel");
        check(across < 0.5 * inChannel, "the channel casts shadows: height decides who hears whom");
    }
    if (season.iceNotes > 0)
    {
        check(season.iceTempered * 10 >= season.iceNotes * 9, "ice crystallises the pitches");
        check(season.iceBubbles == 0, "no bubbles in ice");
    }

    // with full MEMORY, a song revolution sweeps the reef within a season,
    // and someone adopts the new song whole
    // (a revolution is rare by nature: two seeds, so the check does not hang
    // on one random path)
    const auto remembering = run(1.0f, 300.0, 0x815ab1eu, 1.0f);
    const auto rememberingToo = run(1.0f, 300.0, 0x2468aceu, 1.0f);
    check(remembering.last.actCounts[static_cast<std::size_t>(Director::Act::revolution)]
              + rememberingToo.last.actCounts[static_cast<std::size_t>(Director::Act::revolution)] >= 2,
          "a song revolution begins and is adopted");
    std::cout << "season acts:";
    for (std::size_t k = 0; k < Director::actCount; ++k)
        std::cout << ' ' << remembering.last.actCounts[k];
    std::cout << '\n';

    if (failures == 0)
        std::cout << "ecological music director: all checks passed (season notes " << season.notes
                  << ", phases " << season.phases.size() << ", ice notes " << season.iceNotes << ")\n";
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
