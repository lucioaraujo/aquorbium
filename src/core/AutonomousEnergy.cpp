// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/AutonomousEnergy.h"

#include "core/Numeric.h"

#include <algorithm>
#include <cmath>

namespace aquorbium
{
namespace
{
// the tonic path, the registers, the degrees and the pace of life are the
// session's world now (SessionGenesis.h); its classic() is what they were
// the form chain's starting state: also what the variation director sees
// while FORM INFLUENCE is zero (the gate is closed)
constexpr std::size_t formRestState = 3;
constexpr std::array<int, BiomaMarkov::stateCount> formSectionOffsets {
    -5, -3, -2, -1, 0, 1, 3, 5
};
constexpr std::array<float, 8> memoryPitchPattern {
    0.0f, 2.0f, 5.0f, 3.0f, 7.0f, 5.0f, 2.0f, 3.0f
};
constexpr std::array<float, 8> memoryGatePattern {
    0.72f, 0.38f, 0.62f, 0.32f, 0.88f, 0.50f, 0.68f, 0.42f
};

float smoothingCoefficient(double rate, double seconds) noexcept
{
    return static_cast<float>(1.0 - std::exp(-1.0 / (rate * seconds)));
}

float midiToFrequency(float midiPitch) noexcept
{
    return 440.0f * std::pow(2.0f, (midiPitch - 69.0f) / 12.0f);
}
}

void AutonomousEnergy::prepare(double newSampleRate) noexcept
{
    sampleRate = numeric::sampleRate(newSampleRate);
    controlIntervalSamples = static_cast<std::uint32_t>(
        std::max(1.0, std::round(sampleRate / 100.0)));
    controlDt = static_cast<float>(static_cast<double>(controlIntervalSamples) / sampleRate);
    temporalField.prepare(1.0 / static_cast<double>(controlDt));
    ecologicalMusic.prepare(1.0 / static_cast<double>(controlDt));
    enableRampSamples = static_cast<std::uint32_t>(
        std::max(1.0, std::round(sampleRate * 0.12)));
    attackSlew = smoothingCoefficient(sampleRate, 0.59);
    releaseSlew = smoothingCoefficient(sampleRate, 1.35);
    musicAttackSlew = smoothingCoefficient(sampleRate, 0.006);
    reset();
}

void AutonomousEnergy::setSeed(std::uint32_t value) noexcept
{
    seed = value == 0 ? 1u : value;
    geometry.setSeed(seed ^ 0xa511e9b3u);
    formMemory.setSeed(seed ^ 0x6d2b79f5u);
    ecologicalMusic.setSeed(seed ^ 0x2c1b3c6du);
    reset();
}

void AutonomousEnergy::reset() noexcept
{
    randomState = seed;
    temporalField.reset();
    formMemory.reset(formRestState);
    compositionDirector.reset();
    ecologicalMusic.reset();
    geometry.reset();
    stepMemory.setLength(memoryPitchPattern.size());
    for (std::size_t index = 0; index < memoryPitchPattern.size(); ++index)
        static_cast<void>(stepMemory.setStep(index, {
            memoryPitchPattern[index], memoryGatePattern[index], 0.0f, 0.5f
        }));
    stepMemory.reset();
    harmonicProgression.reset();
    currentChord = harmonicProgression.selectPhase(0.0f);
    currentStructurePhase = 0.0f;
    currentMemoryStep = 0;
    samplesUntilControlTick = 0;
    currentEnableAmount = 0.0f;
    enableSamplesRemaining = targetEnabled ? enableRampSamples : 0;
    lastTemporalField = 0.0f;
    currentTemporalField = 0.0f;
    section = -1;
    harmonicRootMidi = 36 + world.rootOffset;
    harmonyClock = 0.0f;
    chordIndex = 0;
    livePlan = world.harmony;
    sessionSeconds = 0.0f;
    tension = 0.0f;
    breathUntil = -1.0f;
    lastChordSeen = 0;
    lastChordWasCadence = false;
    chordStartedAt = 0.0f;
    recentCount = 0;
    motifLength = 0;
    queues = {};
    responsesSung = returnsSung = 0;
    cycleStart = 0;
    cycleCount = 0;
    keyOffset = 0;
    harmonyRandomState = 0x9e3779b97f4a7c15ull ^ (static_cast<std::uint64_t>(seed) << 7) ^ static_cast<std::uint64_t>(world.rootOffset + 64);
    assignHarmonyRoles();
    eventCount = 0;
    formContextTarget = 0.5f;
    ecologicalEnergy.fill(0.0f);
    pitchStep.fill(0.0f);
    frequencyMultiplier.fill(1.0f);
    pitchSamplesRemaining.fill(0);
    noteHoldSamples.fill(0);
    noteReleaseSlew.fill(1.0f);

    for (std::size_t index = 0; index < organismCount; ++index)
    {
        lifePhase[index] = nextRandomUnit();
        lifeRate[index] = world.lifeRates[index] * (0.88f + nextRandomUnit() * 0.24f);
        const auto initialPitch = 36.0f + static_cast<float>(world.rootOffset + world.octaves[index])
            + geometry.quantizer().semitonesForDegree(world.degrees[index]);
        voices[index] = {};
        voices[index].midiPitch = initialPitch;
        voices[index].targetMidiPitch = initialPitch;
        voices[index].frequency = midiToFrequency(initialPitch);
        voices[index].targetFrequency = voices[index].frequency;
        voices[index].awake = index == world.leader;
    }
}

void AutonomousEnergy::setFormMemoryPerformance(
    FormMemoryPerformance value) noexcept
{
    formPerformance.influence = numeric::unit(value.influence,
                                               formPerformance.influence);
    formPerformance.context = numeric::unit(value.context,
                                             formPerformance.context);
    formPerformance.inertia = numeric::unit(value.inertia,
                                             formPerformance.inertia);
    formPerformance.mutation = numeric::unit(value.mutation,
                                              formPerformance.mutation);
    formMemory.setParameters({ formPerformance.context,
                               formPerformance.inertia,
                               formPerformance.mutation });
}

void AutonomousEnergy::setStructureMemoryPerformance(
    StructureMemoryPerformance value) noexcept
{
    structurePerformance.influence = numeric::unit(
        value.influence, structurePerformance.influence);
    structurePerformance.memory = numeric::unit(
        value.memory, structurePerformance.memory);
    structurePerformance.harmony = numeric::unit(
        value.harmony, structurePerformance.harmony);
    structurePerformance.phase = numeric::unit(
        value.phase, structurePerformance.phase);
}

void AutonomousEnergy::setEnabled(bool value) noexcept
{
    if (value == targetEnabled)
        return;
    targetEnabled = value;
    const auto target = targetEnabled ? 1.0f : 0.0f;
    const auto distance = std::abs(target - currentEnableAmount);
    enableSamplesRemaining = distance > 0.0f
        ? static_cast<std::uint32_t>(std::max(
            1.0, std::ceil(static_cast<double>(distance) * enableRampSamples)))
        : 0u;
}

void AutonomousEnergy::tickSample(const Ecosystem& ecosystem, float outputEnergy) noexcept
{
    if (samplesUntilControlTick == 0)
    {
        controlTick(ecosystem, outputEnergy);
        samplesUntilControlTick = controlIntervalSamples;
    }
    --samplesUntilControlTick;

    if (enableSamplesRemaining > 0)
    {
        const auto enableTarget = targetEnabled ? 1.0f : 0.0f;
        if (enableSamplesRemaining == 1)
            currentEnableAmount = enableTarget;
        else
            currentEnableAmount += (enableTarget - currentEnableAmount)
                / static_cast<float>(enableSamplesRemaining);
        --enableSamplesRemaining;
    }

    for (std::size_t index = 0; index < organismCount; ++index)
    {
        auto& state = voices[index];
        state.targetEnergy = ecologicalEnergy[index] * currentEnableAmount;
        if (world.harmonic)
        {
            // dynamics follow the form: softer in repose, never above the
            // classic level; the melody falls silent while it breathes
            state.targetEnergy *= 0.45f + 0.55f * tension;
            if (index == melodyLane && sessionSeconds < breathUntil)
                state.targetEnergy = 0.0f;
        }
        auto energyCoefficient = state.targetEnergy >= state.energy ? attackSlew : releaseSlew;
        if (musicMix > 0.0f)
        {
            // a directed note: a quick attack, held for its length, then
            // released as the water decides; between notes the voice is silent
            const auto held = noteHoldSamples[index] > 0;
            if (held)
                --noteHoldSamples[index];
            state.targetEnergy *= 1.0f - musicMix + musicMix * (held ? 1.0f : 0.0f);
            energyCoefficient = state.targetEnergy >= state.energy
                ? attackSlew + (musicAttackSlew - attackSlew) * musicMix
                : releaseSlew + (noteReleaseSlew[index] - releaseSlew) * musicMix;
        }
        state.energy += (state.targetEnergy - state.energy) * energyCoefficient;
        if (std::abs(state.targetEnergy - state.energy) < 1.0e-7f)
            state.energy = state.targetEnergy;
        if (pitchSamplesRemaining[index] > 0)
        {
            state.midiPitch += pitchStep[index];
            state.frequency *= frequencyMultiplier[index];
            --pitchSamplesRemaining[index];
            if (pitchSamplesRemaining[index] == 0)
            {
                state.midiPitch = state.targetMidiPitch;
                state.frequency = state.targetFrequency;
            }
        }
    }
}

void AutonomousEnergy::controlTick(const Ecosystem& ecosystem, float outputEnergy) noexcept
{
    const auto desire = ecosystem.desireCurrent();
    const auto environment = ecosystem.environmentState();
    attackSlew = smoothingCoefficient(sampleRate, 0.18 + static_cast<double>(desire.calm) * 0.82);
    releaseSlew = smoothingCoefficient(sampleRate, 0.45 + static_cast<double>(desire.calm) * 1.80);
    currentTemporalField = temporalField.tick();
    if (world.harmonic)
    {
        // the harmonic rhythm: its own clock, at the session's pace
        harmonyClock += controlDt;
        chordIndex = static_cast<std::size_t>(harmonyClock / std::max(0.5f, livePlan.chordSeconds));
        if (livePlan.length > 0 && chordIndex - cycleStart >= livePlan.length)
        {
            // a new cycle starts where this one ended (one chord's time each)
            harmonyClock = 0.0f;
            chordIndex = 0;
            cycleStart = 0;
            evolveHarmony();
        }
        // the form's tension: the session's arc and the chord's own (a
        // dominant pulls, a tonic rests), smoothed over ~2 s
        sessionSeconds += controlDt;
        const auto& chord = livePlan.at(chordIndex - cycleStart);
        const auto target = std::clamp(0.6f * world.arcAt(sessionSeconds) + 0.4f * chord.tension, 0.0f, 1.0f);
        tension += (target - tension) * std::min(1.0f, controlDt / 2.0f);
        // a new chord: after a cadence the melody breathes for half a chord
        const auto absoluteChord = chordIndex + cycleCount * 1000u;
        if (absoluteChord != lastChordSeen)
        {
            if (lastChordWasCadence)
            {
                breathUntil = sessionSeconds + 0.5f * livePlan.chordSeconds;
                // the phrase just closed: keep it, and let another voice answer
                if (recentCount >= 3)
                {
                    motifLength = recentCount;
                    for (std::size_t k = 0; k < recentCount; ++k)
                        keptMotif[k] = recentMelody[k];
                    scheduleMotif(responderLane, chord.tones[0], true);
                }
                recentCount = 0;
            }
            lastChordSeen = absoluteChord;
            lastChordWasCadence = chord.cadence;
            chordStartedAt = sessionSeconds;
        }
    }
    const auto collectiveEvent = lastTemporalField <= 0.0f && currentTemporalField > 0.0f;
    lastTemporalField = currentTemporalField;

    // One transition per shared ecological boundary. The chain has its own
    // deterministic random stream, so keeping FORM MIX at zero cannot perturb
    // the established autonomous sequence.
    formContextTarget = std::clamp(
        environment.daylight * 0.30f + environment.temperature * 0.25f
            + environment.matingPulse * 0.20f + desire.danger * 0.25f,
        0.0f, 1.0f);
    if (collectiveEvent)
    {
        static_cast<void>(formMemory.advance({ formContextTarget, 1.0f }));
        // the director hears the form only through FORM INFLUENCE: at zero it
        // gets the resting state, so CONTEXT/INERTIA/MUTATION cannot leak into
        // the harmonic phase and accents (ControlResponseTests form_gate_zero,
        // 2026-10-20 review)
        const auto formForDirector = formPerformance.influence > 0.0f
            ? formMemory.currentState() : formRestState;
        static_cast<void>(compositionDirector.advance({
            formForDirector, desire.density, desire.calm,
            desire.cohesion, desire.danger
        }));
    }

    currentStructurePhase = environment.dayPhase
        + (environment.seasonPhase - environment.dayPhase)
            * structurePerformance.phase;
    currentStructurePhase += compositionDirector.plan().harmonicPhaseOffset
        * structurePerformance.influence;
    currentStructurePhase -= std::floor(currentStructurePhase);
    currentChord = harmonicProgression.selectPhase(currentStructurePhase);

    const auto nextSection = currentSection(environment);
    const auto sectionChanged = nextSection != section;
    if (sectionChanged)
    {
        section = nextSection;
        harmonicRootMidi = rootForSection(section);
    }

    // the music of the ecosystem: notes born from what the organisms perceive
    const auto music = ecologicalMusic.tick(ecosystem);
    const auto musicAmount = ecologicalMusic.performance().influence;
    musicMix = musicAmount;

    for (std::size_t index = 0; index < organismCount; ++index)
    {
        // tension brings more events, repose fewer (a generated world)
        const auto formPace = world.harmonic ? 0.35f + 1.3f * tension : 1.0f;
        const auto speed = (0.52f + desire.density * 1.08f + desire.danger * 0.28f) * formPace;
        lifePhase[index] += lifeRate[index] * speed * controlDt;
        auto individualEvent = false;
        if (lifePhase[index] >= 1.0f)
        {
            lifePhase[index] -= std::floor(lifePhase[index]);
            individualEvent = true;
        }

        // in a generated world the collective gestures thin out in repose
        const auto collectiveChance = (0.35f + desire.cohesion * 0.55f)
            * (world.harmonic ? 0.5f + tension : 1.0f);
        const auto participatesInCollective = collectiveEvent
            && (index == 0 || nextRandomUnit() < collectiveChance);
        if (individualEvent || participatesInCollective || sectionChanged)
        {
            makeDecision(index, ecosystem, participatesInCollective || sectionChanged);
            // the generic draw yields to the ecological music as it grows
            if (musicAmount > 0.0f && voices[index].awake && nextRandomUnit() < musicAmount)
                voices[index].awake = false;
        }
        if (musicAmount > 0.0f && music.notes[index].on)
            directedNote(index, music.notes[index].midi, music.notes[index].accent,
                         music.notes[index].glideSeconds, music.notes[index].durationSeconds,
                         music.notes[index].releaseSeconds);

        const auto& organism = ecosystem.organism(index);
        const auto presence = organism.presenceAmount;
        const auto structuralHealth = 0.68f + ecosystem.integrityState().health * 0.32f;
        const auto daylightBreath = 0.80f + environment.daylight * 0.20f;
        const auto excessiveOutput = std::max(0.0f, outputEnergy - 0.30f);
        const auto homeostasis = std::clamp(1.0f - excessiveOutput * 0.85f, 0.45f, 1.0f);
        const auto roleGain = organism.role == Ecosystem::SonicRole::Foreground ? 1.0f : 0.78f;
        const auto accent = voices[index].geometricAccent;
        const auto accentGain = accent > 0.0f
            ? 1.0f + (accent - 0.5f) * 0.24f
                * compositionDirector.plan().accentWeight
                * structurePerformance.influence
            : 1.0f;
        ecologicalEnergy[index] = voices[index].awake
            ? (0.12f + organism.energy * 0.52f) * presence * structuralHealth
                * daylightBreath * homeostasis * roleGain * accentGain
            : 0.0f;

        // The foundation may become extremely quiet, but it never disappears
        // while the bioma is structurally present. Silence remains available
        // through setEnabled(false).
        if (index == 0 && ecosystem.integrityState().health > 0.0f)
            ecologicalEnergy[index] = std::max(ecologicalEnergy[index],
                                               (0.09f + desire.density * 0.08f) * (1.0f - musicAmount));
        if (musicAmount > 0.0f)
            ecologicalEnergy[index] *= 1.0f + (music.level[index] - 1.0f) * musicAmount;
    }
}

void AutonomousEnergy::makeDecision(std::size_t index, const Ecosystem& ecosystem,
                                    bool collectiveEvent) noexcept
{
    auto& state = voices[index];
    const auto desire = ecosystem.desireCurrent();
    const auto& organism = ecosystem.organism(index);
    GeometricSequencer::GenerativeInput geometricInput;
    const auto variation = compositionDirector.plan();
    const auto variationAmount = structurePerformance.influence;
    geometricInput.density = desire.density * (1.0f +
        (variation.rhythmicDensity - 1.0f) * variationAmount);
    geometricInput.rhythmRotation = variation.rhythmRotation * variationAmount;
    geometricInput.cohesion = desire.cohesion;
    geometricInput.danger = desire.danger;
    geometricInput.rootMidi = static_cast<float>(harmonicRootMidi);
    // a generated world stays in its key: the section moves the chord degree
    // within the mode instead of transposing the mode (SessionGenesis.h)
    const auto chordDegree = world.keyCentred && !world.harmonic && section >= 0
        ? world.chordPath[static_cast<std::size_t>((section / std::max(1, world.sectionsPerChord)) & 7)]
        : 0;
    geometricInput.anchorDegree = world.degrees[index] + chordDegree;
    geometricInput.registerSemitones = world.octaves[index];
    const auto stateIndex = std::min(formMemory.currentState(),
                                     formSectionOffsets.size() - 1u);
    const auto formOffset = static_cast<int>(std::lround(
        static_cast<float>(formSectionOffsets[stateIndex])
            * formPerformance.influence));
    geometricInput.section = section + formOffset;
    geometricInput.brainLean = brainLean[index];
    const auto geometricStep = geometry.advance(index, geometricInput);
    const auto orbitPhase = geometricStep.orbitLength > 0
        ? static_cast<float>(geometricStep.orbitStep)
            / static_cast<float>(geometricStep.orbitLength)
        : 0.0f;
    auto address = currentStructurePhase + orbitPhase
        + static_cast<float>(index) / static_cast<float>(organismCount);
    address -= std::floor(address);
    address += variation.contourRotation * variationAmount;
    address -= std::floor(address);
    const auto remembered = stepMemory.address(address);
    auto memoryPitch = remembered.step.pitchSemitones;
    auto memoryGateDuration = remembered.step.gateDuration;
    currentMemoryStep = remembered.index;
    if (variation.retrogradeContour && variationAmount > 0.0f)
    {
        const auto retrograde = stepMemory.address(1.0f - address);
        memoryPitch += (retrograde.step.pitchSemitones - memoryPitch)
            * variationAmount;
        memoryGateDuration += (retrograde.step.gateDuration - memoryGateDuration)
            * variationAmount;
        if (variationAmount >= 0.5f)
            currentMemoryStep = retrograde.index;
    }
    const auto memoryAmount = std::clamp(
        structurePerformance.influence * structurePerformance.memory
            * variation.memoryWeight,
        0.0f, 1.25f);
    const auto harmonyAmount = structurePerformance.influence
        * structurePerformance.harmony
        * (1.0f + (variation.tonalWeight - 1.0f) * variationAmount);
    const auto centredMemoryPitch = memoryPitch - 3.5f;
    const auto chordVoice = index % std::max<std::size_t>(1, currentChord.voiceCount);
    const auto structuralSemitones = centredMemoryPitch * memoryAmount
        + currentChord.semitones[chordVoice] * harmonyAmount;
    state.geometricGate = geometricStep.gateProbability >= 0.5f;
    state.geometricMutation = geometricStep.mutated;
    state.orbitStep = geometricStep.orbitStep;
    state.orbitLength = geometricStep.orbitLength;
    state.geometricAccent = geometricStep.accent;
    if (index == 0)
    {
        state.awake = true;
    }
    else
    {
        const auto wakeProbability = std::clamp(
            0.08f + desire.density * 0.62f + organism.energy * 0.20f
                + ecosystem.environmentState().matingPulse * 0.10f
                + (collectiveEvent ? desire.cohesion * 0.08f : 0.0f),
            0.05f, 0.94f);
        const auto rhythmicProbability = std::clamp(wakeProbability
            * (1.0f + (variation.pulseWeight - 1.0f) * variationAmount),
            0.02f, 0.98f);
        const auto memoryGate = 1.0f
            + (memoryGateDuration - 0.5f) * 1.2f * memoryAmount;
        const auto eventProbability = geometricStep.gateProbability
            * std::clamp(rhythmicProbability * memoryGate, 0.02f, 0.98f);
        state.awake = eventProbability > 0.0f
            && nextRandomUnit() < eventProbability;
    }

    updatePitch(index, ecosystem, geometricStep, structuralSemitones);
    ++state.lifeEvents;
    ++eventCount;

}

void AutonomousEnergy::updatePitch(std::size_t index, const Ecosystem& ecosystem,
                                   const GeometricSequencer::Step& step,
                                   float structuralSemitones) noexcept
{
    const auto desire = ecosystem.desireCurrent();
    // warm water lifts the register two semitones, cold lowers it - a
    // chromatic shift, so a generated world (in one key) leaves it out
    const auto seasonalRegister = world.keyCentred ? 0
        : ecosystem.environmentState().temperature > 0.72f ? 2
        : ecosystem.environmentState().temperature < 0.28f ? -2 : 0;
    auto& state = voices[index];
    state.targetMidiPitch = std::clamp(
        step.midiPitch + static_cast<float>(seasonalRegister) + structuralSemitones,
        24.0f, 88.0f);
    if (world.harmonic)
    {
        auto* motifQueue = queues[0].sounding() && queues[0].lane == index ? &queues[0]
                         : queues[1].sounding() && queues[1].lane == index ? &queues[1] : nullptr;
        if (motifQueue != nullptr)
        {
            // the motif: its next pitch, in this voice's register, in the mode
            const auto key = static_cast<float>(36 + world.rootOffset + keyOffset);
            const auto wanted = key + static_cast<float>(motifQueue->pitches[motifQueue->next]);
            const auto octaves = std::round((state.targetMidiPitch - wanted) / 12.0f);
            state.targetMidiPitch = foldIntoRange(inMode(wanted + 12.0f * octaves), 24.0f, 88.0f);
            ++motifQueue->next;
            if (index == melodyLane)
                ++returnsSung;
            else
                ++responsesSung;
        }
        else
        {
            state.targetMidiPitch = foldIntoRange(voiceHarmony(index, state.targetMidiPitch, step), 24.0f, 88.0f);
            if (index == melodyLane)
            {
                // the melody's notes, as semitones above the key, the latest last
                const auto relative = static_cast<int>(std::lround(state.targetMidiPitch)) - (36 + world.rootOffset + keyOffset);
                if (recentCount == motifCapacity)
                {
                    for (std::size_t k = 1; k < motifCapacity; ++k)
                        recentMelody[k - 1] = recentMelody[k];
                    recentCount = motifCapacity - 1;
                }
                recentMelody[recentCount++] = relative;
            }
        }
    }
    state.targetFrequency = midiToFrequency(state.targetMidiPitch);
    const auto glideSeconds = 0.16 + static_cast<double>(desire.calm) * 0.74;
    const auto glideSamples = static_cast<std::uint32_t>(
        std::max(1.0, std::round(sampleRate * glideSeconds)));
    pitchSamplesRemaining[index] = glideSamples;
    pitchStep[index] = (state.targetMidiPitch - state.midiPitch)
        / static_cast<float>(glideSamples);
    frequencyMultiplier[index] = state.frequency > 0.0f
        ? std::pow(state.targetFrequency / state.frequency,
                   1.0f / static_cast<float>(glideSamples))
        : 1.0f;
}

// a note of the ecological music: the organism sounds now, at this pitch; the
// glide is the medium's viscosity (ice: none, cold water: a long portamento)
void AutonomousEnergy::directedNote(std::size_t index, float midi, float accent, float glideSeconds,
                                    float durationSeconds, float releaseSeconds) noexcept
{
    noteHoldSamples[index] = static_cast<std::uint32_t>(
        std::max(1.0, sampleRate * std::clamp(static_cast<double>(durationSeconds), 0.01, 4.0)));
    noteReleaseSlew[index] = smoothingCoefficient(sampleRate, std::clamp(static_cast<double>(releaseSeconds), 0.01, 2.0));
    auto& state = voices[index];
    state.awake = true;
    state.geometricGate = true;
    state.geometricAccent = std::clamp(accent, 0.0f, 1.0f);
    state.targetMidiPitch = std::clamp(midi, 24.0f, 96.0f);
    if (world.harmonic)
    {
        // the ecological music keeps its rhythm, contour and accents; the
        // session's harmony voices its pitch (measured 2026-10-24: without
        // this only 50-69 % of its notes fell in the sounding chord)
        GeometricSequencer::Step step;
        step.orbitStep = 1;
        step.accent = accent;
        state.targetMidiPitch = foldIntoRange(voiceHarmony(index, state.targetMidiPitch, step), 24.0f, 96.0f);
    }
    state.targetFrequency = midiToFrequency(state.targetMidiPitch);
    const auto glideSamples = static_cast<std::uint32_t>(std::max(1.0, std::round(sampleRate * std::clamp(static_cast<double>(glideSeconds), 0.002, 2.0))));
    pitchSamplesRemaining[index] = glideSamples;
    pitchStep[index] = (state.targetMidiPitch - state.midiPitch) / static_cast<float>(glideSamples);
    frequencyMultiplier[index] = state.frequency > 0.0f
        ? std::pow(state.targetFrequency / state.frequency, 1.0f / static_cast<float>(glideSamples))
        : 1.0f;
    ++state.lifeEvents;
    ++eventCount;
}

float AutonomousEnergy::nextRandomUnit() noexcept
{
    randomState = randomState * 1664525u + 1013904223u;
    return static_cast<float>((randomState >> 8u) & 0x00ffffffu) / 16777215.0f;
}

int AutonomousEnergy::currentSection(const Ecosystem::Environment& environment) const noexcept
{
    const auto daySection = std::clamp(static_cast<int>(environment.dayPhase * 8.0f), 0, 7);
    const auto seasonSection = std::clamp(static_cast<int>(environment.seasonPhase * 4.0f), 0, 3);
    return daySection + seasonSection * 8;
}

// the motif, transformed, queued for a voice: an answer imitates it from the
// new chord's root, inverts it, plays it backwards, or both; a return keeps
// it or inverts it (Schoenberg, *Fundamentals of Musical Composition*, 1967:
// the motive and its variations)
void AutonomousEnergy::scheduleMotif(std::size_t lane, int transposition, bool answer) noexcept
{
    const auto first = keptMotif[0];
    const auto form = static_cast<int>(harmonyUnit() * (answer ? 3.999f : 1.999f));
    auto& q = queues[answer ? 0 : 1];
    q.length = motifLength;
    for (std::size_t k = 0; k < motifLength; ++k)
    {
        const auto source = (form == 2 || form == 3) && answer ? keptMotif[motifLength - 1 - k] : keptMotif[k];
        const auto inverted = form == 1 || form == 3;
        q.pitches[k] = (inverted ? 2 * first - source : source) + (answer ? transposition - first % 12 : 0);
    }
    q.next = 0;
    q.lane = lane;
}

// a voiced pitch kept in range by whole octaves: clamping it would change
// its pitch class (a bass at 23 clamped to 24 played the wrong note)
float AutonomousEnergy::foldIntoRange(float pitch, float low, float high) noexcept
{
    if (!std::isfinite(pitch))
        return low;
    while (pitch < low)
        pitch += 12.0f;
    while (pitch > high)
        pitch -= 12.0f;
    return std::max(pitch, low);
}

// the nearest pitch of the mode (in the current key) to a pitch
float AutonomousEnergy::inMode(float pitch) const noexcept
{
    const auto key = static_cast<float>(36 + world.rootOffset + keyOffset);
    auto best = pitch;
    auto bestDistance = 1.0e9f;
    const auto octave = std::floor((pitch - key) / 12.0f);
    for (float o = octave - 1.0f; o <= octave + 1.0f; o += 1.0f)
        for (std::size_t i = 0; i < world.scaleSize; ++i)
        {
            const auto candidate = key + 12.0f * o + world.scale[i];
            const auto distance = std::abs(candidate - pitch);
            if (distance < bestDistance)
            {
                bestDistance = distance;
                best = candidate;
            }
        }
    return best;
}

// the organisms' roles in the progression: the lowest register is the bass,
// the session's leader sings the melody (the highest voice if the leader is
// the bass), the two highest are the upper layer of a polytonal episode
void AutonomousEnergy::assignHarmonyRoles() noexcept
{
    bassLane = 0;
    for (std::size_t i = 1; i < organismCount; ++i)
        if (world.octaves[i] < world.octaves[bassLane])
            bassLane = i;
    std::size_t highest = bassLane == 0 ? 1 : 0;
    for (std::size_t i = 0; i < organismCount; ++i)
        if (i != bassLane && world.octaves[i] > world.octaves[highest])
            highest = i;
    melodyLane = world.leader != bassLane ? world.leader : highest;
    // the responder: the highest voice that is neither the bass nor the melody
    responderLane = organismCount;
    for (std::size_t i = 0; i < organismCount; ++i)
        if (i != bassLane && i != melodyLane
            && (responderLane == organismCount || world.octaves[i] > world.octaves[responderLane]))
            responderLane = i;
    if (responderLane == organismCount)
        responderLane = melodyLane;
    upperLayer.fill(false);
    for (int k = 0; k < 2; ++k)
    {
        std::size_t best = organismCount;
        for (std::size_t i = 0; i < organismCount; ++i)
            if (i != bassLane && !upperLayer[i] && (best == organismCount || world.octaves[i] > world.octaves[best]))
                best = i;
        if (best < organismCount)
            upperLayer[best] = true;
    }
}

// a note of the sequencer, voiced in the chord sounding now
float AutonomousEnergy::voiceHarmony(std::size_t index, float target, const GeometricSequencer::Step& step) const noexcept
{
    const auto& chord = livePlan.at(chordIndex - cycleStart);
    if (chord.size == 0)
        return target;
    const auto key = static_cast<float>(36 + world.rootOffset + keyOffset);
    const auto layer = chord.layerOffset != 0 && upperLayer[index] ? static_cast<float>(chord.layerOffset) : 0.0f;
    const auto origin = key + layer;
    const auto previous = voices[index].midiPitch;
    // the nearest pitch of a pitch class to a reference
    const auto nearest = [origin](int pitchClass, float reference) {
        const auto base = origin + static_cast<float>(pitchClass);
        const auto octaves = std::round((reference - base) / 12.0f);
        return base + 12.0f * octaves;
    };
    if (index == bassLane)
        return nearest(chord.bass, target);
    // a suspension: early in a new chord an inner voice may hold its note,
    // a step from a chord tone, and resolve on its next note (the 4-3)
    const auto early = sessionSeconds - chordStartedAt < 0.4f * livePlan.chordSeconds;
    if (index != melodyLane && early && previous > 0.0f)
    {
        const auto held = static_cast<int>(std::lround(previous - origin));
        const auto heldClass = ((held % 12) + 12) % 12;
        if (!chord.contains(heldClass))
            for (std::size_t i = 0; i < chord.size; ++i)
            {
                auto distance = std::abs(chord.tones[i] - heldClass);
                distance = std::min(distance, 12 - distance);
                if (distance <= 2 && (index + chordIndex) % 2 == 0)
                    return previous;  // held; it resolves by step at the next note
            }
    }
    // at the climax the melody rises an octave
    if (index == melodyLane && tension > 0.75f)
        target += 12.0f;
    const auto relative = static_cast<int>(std::lround(target - origin));
    const auto pitchClass = ((relative % 12) + 12) % 12;
    if (index == melodyLane)
    {
        // the melody keeps the mode, except on strong beats (a chord tone),
        // and where the chord alters a neighbouring degree (chord-scale:
        // a secondary dominant's leading tone replaces its neighbour)
        const auto strong = step.orbitStep == 0 || step.accent > 0.85f;
        if (!strong && chord.contains(pitchClass))
            return target;
        if (!strong)
        {
            for (std::size_t i = 0; i < chord.size; ++i)
            {
                auto distance = std::abs(chord.tones[i] - pitchClass);
                distance = std::min(distance, 12 - distance);
                if (distance == 1)
                    return nearest(chord.tones[i], target);
            }
            // a passing note stays in the mode (a pitch from the ecological
            // music may come from its own scale)
            return inMode(target) + layer;
        }
    }
    // the inner voices, and the melody on strong beats: the chord tone that
    // moves least from the voice's last note, near the sequencer's register
    auto best = target;
    auto bestCost = 1.0e9f;
    for (std::size_t i = 0; i < chord.size; ++i)
    {
        const auto candidate = nearest(chord.tones[i], target);
        for (const auto shift : { -12.0f, 0.0f, 12.0f })
        {
            const auto c = candidate + shift;
            const auto cost = std::abs(c - previous) + 0.5f * std::abs(c - target);
            if (cost < bestCost)
            {
                bestCost = cost;
                best = c;
            }
        }
    }
    return best;
}

float AutonomousEnergy::harmonyUnit() noexcept
{
    harmonyRandomState += 0x9e3779b97f4a7c15ull;
    auto z = harmonyRandomState;
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ull;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebull;
    z ^= z >> 31;
    return static_cast<float>(z >> 40) / 16777216.0f;
}

// the progression at the turn of a cycle (the Trioio's scale and harmony
// machines: follow, transfigure, interrupt - TRIOIO/docs/ESCALAS.md): a new
// progression in the same language and key; or a modulation to a near key
// (the dominant, the subdominant, the relative, a tone up); or another
// language; and every third cycle the home key again - a form A B A'
void AutonomousEnergy::evolveHarmony() noexcept
{
    ++cycleCount;
    auto random = [this]() { return harmonyUnit(); };
    const auto choice = harmonyUnit();
    auto language = static_cast<int>(livePlan.language);
    if (cycleCount % 3 == 0)
        keyOffset = 0;
    else if (choice < 0.25f)
    {
        static constexpr std::array<int, 4> near { 7, 5, -3, 2 };
        keyOffset += near[static_cast<std::size_t>(harmonyUnit() * 3.999f)];
        keyOffset = ((keyOffset + 6) % 12 + 12) % 12 - 6;
    }
    else if (choice < 0.45f)
        language = -1;  // drawn anew
    const auto tempo = livePlan.chordSeconds > 0.0f ? 5.0f / livePlan.chordSeconds : 1.0f;
    livePlan = HarmonyPlan::generate(world.scale, world.scaleSize, tempo, random, language);
    harmonicRootMidi = 36 + world.rootOffset + keyOffset;
    // every second cycle the melody brings its motif back, transformed
    if (motifLength >= 3 && cycleCount % 2 == 0)
        scheduleMotif(melodyLane, 0, false);
}

int AutonomousEnergy::rootForSection(int current) const noexcept
{
    const auto daySection = current & 7;
    const auto seasonSection = (current / 8) & 3;
    constexpr std::array<int, 4> seasonalOffsets { 0, 2, -2, -5 };
    if (world.keyCentred)
        return 36 + world.rootOffset + keyOffset;  // the key stays (or modulates); the chord moves
    return 36 + world.rootOffset + world.rootPath[static_cast<std::size_t>(daySection)]
        + seasonalOffsets[static_cast<std::size_t>(seasonSection)];
}

const AutonomousEnergy::OrganismVoice& AutonomousEnergy::voice(std::size_t index) const noexcept
{
    return voices[std::min(index, organismCount - 1)];
}

AutonomousEnergy::Telemetry AutonomousEnergy::telemetry() const noexcept
{
    Telemetry result;
    result.enabled = targetEnabled;
    result.enableAmount = currentEnableAmount;
    result.temporalField = currentTemporalField;
    result.harmonicRootMidi = harmonicRootMidi;
    result.harmonyChord = chordIndex;
    result.chord = currentChordOfPlan();
    result.keyOffset = keyOffset;
    result.harmonyCycles = cycleCount;
    result.harmonyLanguage = livePlan.language;
    result.tension = tension;
    result.breathing = sessionSeconds < breathUntil;
    result.melodyLane = melodyLane;
    result.responderLane = responderLane;
    result.responsesSung = responsesSung;
    result.returnsSung = returnsSung;
    result.motifSounding = queues[0].sounding() || queues[1].sounding();
    result.motifLane = queues[1].sounding() ? queues[1].lane : queues[0].lane;
    result.music = ecologicalMusic.telemetry();
    result.musicInfluence = ecologicalMusic.performance().influence;
    result.eventCount = eventCount;
    result.formState = formMemory.currentState();
    result.formTransitionCount = formMemory.transitionCount();
    result.formContextTarget = formContextTarget;
    result.structureInfluence = structurePerformance.influence;
    result.structureMemory = structurePerformance.memory;
    result.structureHarmony = structurePerformance.harmony;
    result.structurePhase = structurePerformance.phase;
    result.memoryStep = currentMemoryStep;
    result.harmonicSection = currentChord.sectionIndex;
    result.composition = compositionPlan();
    result.voices = voices;
    for (const auto& state : voices)
    {
        result.totalEnergy += state.energy;
        if (state.energy > 0.001f || state.targetEnergy > 0.001f)
            ++result.activeOrganisms;
    }
    result.totalEnergy /= static_cast<float>(organismCount);
    return result;
}
}
