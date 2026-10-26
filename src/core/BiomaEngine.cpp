// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/BiomaEngine.h"
#include "core/FlushDenormals.h"
#include "core/Numeric.h"

#include <algorithm>
#include <cmath>

namespace aquorbium
{
namespace
{
// the bubbling water's level in the dry mix (BubbleField), set against the
// organisms in boiling water (2026-10-21)
constexpr float bubbleLevel = 0.017f;  // ~10 dB under the organisms at the boil (measured)
// the binaural mode's level: the head's mono sum against the stereo pair,
// matched by measurement (2026-10-21: equal within 0.05 dB)
constexpr float binauralLevel = 0.52f;
// the synthesis voice's level against the organisms' native bodies, matched
// by measurement (2026-10-21)
constexpr float synthLevel = 0.37f;

float morphValue(float neutral, float active, float amount) noexcept
{
    if (amount <= 0.0f) return neutral;
    if (amount >= 1.0f) return active;
    return neutral + (active - neutral) * amount;
}

float spatialDepth(const Ecosystem::MixState& mix) noexcept
{
    const auto openness = std::clamp(
        0.18f + mix.observation * 0.62f + mix.detail * 0.55f, 0.0f, 1.0f);
    return 1.0f - openness;
}

BiomaGranularStriker::Parameters granularParameters(
    float frequency, const Ecosystem::OrganismState& organism,
    const Ecosystem::Desire& desire, float detail, float accent) noexcept
{
    BiomaGranularStriker::Parameters result;
    result.centerHz = frequency;
    result.grainDurationMs = 2.0f + (1.0f - organism.agility) * 22.0f
        + desire.calm * 7.0f;
    result.particleDensity = std::clamp(
        0.10f + desire.density * 0.42f + organism.intelligence * 0.30f
            + accent * 0.18f,
        0.0f, 1.0f);
    result.spectralSpread = std::clamp(
        0.10f + organism.turbidity * 0.48f + desire.danger * 0.24f
            + (1.0f - desire.cohesion) * 0.18f,
        0.0f, 1.0f);
    result.resonance = std::clamp(
        0.18f + organism.strength * 0.58f + detail * 0.24f, 0.0f, 1.0f);
    result.irregularity = std::clamp(
        0.08f + organism.agility * 0.42f + desire.danger * 0.22f
            + (1.0f - desire.cohesion) * 0.20f + accent * 0.08f,
        0.0f, 1.0f);
    return result;
}

BiomaPhysicalString::Parameters physicalStringParameters(
    float frequency, const Ecosystem::OrganismState& organism,
    const Ecosystem::Desire& desire, const Ecosystem::Environment& environment,
    float detail, float accent) noexcept
{
    BiomaPhysicalString::Parameters result;
    result.fundamentalHz = frequency;
    result.decaySeconds = 0.55f + desire.calm * 3.4f + organism.strength * 0.8f;
    result.brightness = std::clamp(
        0.12f + detail * 0.40f + environment.daylight * 0.18f
            + organism.intelligence * 0.18f,
        0.0f, 1.0f);
    result.hardness = std::clamp(
        0.08f + accent * 0.52f + desire.danger * 0.20f
            + organism.strength * 0.15f,
        0.0f, 1.0f);
    result.pluckPosition = std::clamp(
        0.27f + organism.position.x * 0.04f
            + (organism.agility - 0.5f) * 0.08f,
        0.04f, 0.50f);
    return result;
}

float organismPanPosition(std::size_t index) noexcept
{
    constexpr auto centre = static_cast<float>(Ecosystem::organismCount - 1) * 0.5f;
    return centre > 0.0f ? (static_cast<float>(index) - centre) / centre : 0.0f;
}

std::size_t speciesGainSlot(Ecosystem::Species species) noexcept
{
    switch (species)
    {
        case Ecosystem::Species::Oscillator: return 0;
        case Ecosystem::Species::GranularStriker: return 1;
        case Ecosystem::Species::Resonator: return 2;
        case Ecosystem::Species::Pulsar: return 3;
        case Ecosystem::Species::PhysicalString: return 4;
    }
    return 0;
}

bool hasEventBody(Ecosystem::Species species) noexcept
{
    return species == Ecosystem::Species::GranularStriker
        || species == Ecosystem::Species::Resonator
        || species == Ecosystem::Species::PhysicalString;
}

float bodyCalibrationGain(Ecosystem::Species species) noexcept
{
    // Ratios begin with the level-matched ten-impact comparison, then include
    // the presence correction measured in autonomous stems (the sparse modal
    // tail required another 3x). They do not alter resonator, particle or
    // delay-line state.
    switch (species)
    {
        case Ecosystem::Species::GranularStriker: return 9.0f;
        case Ecosystem::Species::Resonator: return 16.5f;
        case Ecosystem::Species::PhysicalString: return 19.0f;
        case Ecosystem::Species::Oscillator:
        case Ecosystem::Species::Pulsar: return 1.0f;
    }
    return 1.0f;
}
}

BiomaEngine::BiomaEngine()
    : multiTapSpace(std::make_unique<MultiTapSpace>()),
      syntheticTail(std::make_unique<AquariumTail>()),
      moduleBank(std::make_unique<IntegratedModuleBank>()),
      masterChain(std::make_unique<MasterChain>()),
      outputStage(std::make_unique<OutputStage>()),
      habitat(std::make_unique<HabitatHealth>()),
      synthVoiceBank(std::make_unique<std::array<SynthVoice, Ecosystem::organismCount>>())
{
}

BiomaEngine::~BiomaEngine() = default;

void BiomaEngine::prepare(double newSampleRate) noexcept
{
    sampleRate = numeric::sampleRate(newSampleRate);
    directControlSmoothing = static_cast<float>(-std::expm1(-1.0 / (sampleRate * 0.015)));
    audioDensity = density;
    audioDrift = drift;
    strikerMixCurrent = strikerMixTarget;
    synthMix = synthMixTarget;
    supportCurrent = supportTarget;
    voices.fill({});
    for (std::size_t index = 0; index < voices.size(); ++index)
    {
        for (std::size_t organism = 0; organism < voices[index].organism.size(); ++organism)
        {
            auto& source = voices[index].organism[organism];
            source.prepare(sampleRate);
            source.setSeed(engineSeed
                ^ (0x9e3779b9u * static_cast<std::uint32_t>(index + 1))
                ^ (0x85ebca6bu * static_cast<std::uint32_t>(organism + 1)));
            source.setPulseWidth(sessionPulseWidth(organism));
        }
        for (auto& spatializer : voices[index].spatializer)
            spatializer.prepare(sampleRate);
        voices[index].physicalString.prepare(sampleRate);
        voices[index].physicalString.setSeed(
            engineSeed ^ 0x19f47a2du
                ^ (0x9e3779b9u * static_cast<std::uint32_t>(index + 1)));
    }
    brain.prepare(sampleRate);
    ecosystem.prepare(sampleRate);
    ecosystem.reset(brain);
    autonomousEnergy.prepare(sampleRate);
    schoolSpectralLeft.prepare(sampleRate);
    schoolSpectralRight.prepare(sampleRate);
    ecologicalNoise.prepare(sampleRate);
    schoolSpectralLeft.setIdentity(0u, engineSeed ^ 0x521fa93u);
    schoolSpectralRight.setIdentity(0u, engineSeed ^ 0x521fa93u);
    ecologicalNoise.setSeed(engineSeed ^ 0xb417c29u);
    currentSpectralEnvironment = targetSpectralEnvironment;
    updateSpectralEnvironmentParameters();
    multiTapSpace->setSeed(engineSeed ^ 0x2d6f84b1u);
    multiTapSpace->prepare(sampleRate);
    multiTapSpaceControlInterval = static_cast<std::uint32_t>(
        std::max(1.0, std::round(sampleRate / 200.0)));
    samplesUntilMultiTapSpaceControl = 0;
    updateMultiTapSpaceParameters();
    multiTapSpace->reset();
    syntheticTail->prepare(sampleRate);
    currentSyntheticTail = targetSyntheticTail;
    updateSyntheticTailParameters();
    syntheticTail->reset();
    moduleBank->prepare(sampleRate);
    moduleBank->setSeed(engineSeed ^ 0xd61f0a43u);
    masterChain->prepare(sampleRate);
    updateCreativeMasterParameters();
    masterChain->reset();
    masterFeedback.prepare(200.0);
    outputStage->prepare(sampleRate);
    habitat->prepare(sampleRate);
    room.prepare(sampleRate);
    bubbleField.prepare(sampleRate, engineSeed ^ 0x5eab0bbu);
    for (auto& head : binaural)
        head.prepare(sampleRate);
    for (auto& head : midiBinaural)
        head.prepare(sampleRate);
    for (auto& panner : ambisonicPanners)
        panner.prepare(sampleRate);
    for (std::size_t i = 0; i < synthVoices.size(); ++i)
        synthVoices[i].prepare(sampleRate, engineSeed ^ (0x51e7u + static_cast<std::uint32_t>(i) * 0x9e37u));
    binauralStep = static_cast<float>(1.0 / (0.05 * sampleRate));
    multiTapSpaceReturnSmoothing = static_cast<float>(
        1.0 - std::exp(-1.0 / (sampleRate * 0.080)));
    currentMultiTapSpaceReturn = targetMultiTapSpaceReturn;
    autonomousDcCoefficient = static_cast<float>(
        std::exp(-2.0 * 3.14159265358979323846 * 10.0 / sampleRate));
    resetAutonomousVoice();
    ecosystemBlendRampSamples = static_cast<std::uint32_t>(
        std::max(1.0, std::round(sampleRate * 0.08)));
    ecosystemBlendSamplesRemaining = 0;
    currentEcosystemBlend = targetEcosystemBlend;
    lastOutputEnergy = 0.0f;
    finalOutputFeedbackSample = 0.0f;
    brain.setEcologicalFeedback(0.0f);
    taps = {};
    tapAttack = static_cast<float>(1.0 - std::exp(-1.0 / (sampleRate * 0.010)));
    tapRelease = static_cast<float>(1.0 - std::exp(-1.0 / (sampleRate * 0.150)));
}

void BiomaEngine::setSeed(std::uint32_t value) noexcept
{
    engineSeed = value == 0 ? 1u : value;
    brain.setSeed(value);
    ecosystem.reset(brain);
    autonomousEnergy.setSeed(value ^ 0x6d2b79f5u);
    schoolSpectralLeft.setIdentity(0u, engineSeed ^ 0x521fa93u);
    schoolSpectralRight.setIdentity(0u, engineSeed ^ 0x521fa93u);
    schoolSpectralLeft.reset();
    schoolSpectralRight.reset();
    ecologicalNoise.setSeed(engineSeed ^ 0xb417c29u);
    multiTapSpace->setSeed(engineSeed ^ 0x2d6f84b1u);
    for (std::size_t index = 0; index < voices.size(); ++index)
    {
        for (std::size_t organism = 0; organism < voices[index].organism.size(); ++organism)
            voices[index].organism[organism].setSeed(engineSeed
                ^ (0x9e3779b9u * static_cast<std::uint32_t>(index + 1))
                ^ (0x85ebca6bu * static_cast<std::uint32_t>(organism + 1)));
        voices[index].physicalString.setSeed(
            engineSeed ^ 0x19f47a2du
                ^ (0x9e3779b9u * static_cast<std::uint32_t>(index + 1)));
    }
    resetAutonomousVoice();
    updateMultiTapSpaceParameters();
    multiTapSpace->reset();
    syntheticTail->reset();
    moduleBank->setSeed(engineSeed ^ 0xd61f0a43u);
    bubbleField.prepare(sampleRate, engineSeed ^ 0x5eab0bbu);
    for (std::size_t i = 0; i < synthVoices.size(); ++i)
        synthVoices[i].prepare(sampleRate, engineSeed ^ (0x51e7u + static_cast<std::uint32_t>(i) * 0x9e37u));
    masterChain->reset();
    masterFeedback.reset();
    brain.setEcologicalFeedback(0.0f);
    outputStage->reset();
    habitat->reset();
    room.reset();
    bubbleField.reset();
    for (auto& head : binaural)
        head.reset();
    for (auto& head : midiBinaural)
        head.reset();
    for (auto& panner : ambisonicPanners)
        panner.reset();
    midiBinauralInput.fill(0.0f);
    binauralMix = binauralTarget;
    for (auto& voice : synthVoices)
        voice.reset();
    synthEvents.fill(0);
    synthLiquid.fill(0.0f);
    samplesUntilMultiTapSpaceControl = 0;
    lastOutputEnergy = 0.0f;
    finalOutputFeedbackSample = 0.0f;
    taps = {};
}

void BiomaEngine::setDensity(float value) noexcept
{
    if (!std::isfinite(value))
        return;
    density = numeric::unit(value, density);
    auto desire = ecosystem.desireTarget();
    desire.density = density;
    ecosystem.setDesire(desire);
}

void BiomaEngine::setDrift(float value) noexcept
{
    if (!std::isfinite(value))
        return;
    drift = numeric::unit(value, drift);
    auto desire = ecosystem.desireTarget();
    desire.calm = 1.0f - drift;
    ecosystem.setDesire(desire);
}

void BiomaEngine::setCohesion(float value) noexcept
{
    if (!std::isfinite(value))
        return;
    auto desire = ecosystem.desireTarget();
    desire.cohesion = numeric::unit(value, desire.cohesion);
    ecosystem.setDesire(desire);
}

void BiomaEngine::setDanger(float value) noexcept
{
    if (!std::isfinite(value))
        return;
    auto desire = ecosystem.desireTarget();
    desire.danger = numeric::unit(value, desire.danger);
    ecosystem.setDesire(desire);
}

void BiomaEngine::setDesire(Ecosystem::Desire value) noexcept
{
    const auto previous = ecosystem.desireTarget();
    const Ecosystem::Desire sanitized {
        numeric::unit(value.calm, previous.calm),
        numeric::unit(value.density, previous.density),
        numeric::unit(value.cohesion, previous.cohesion),
        numeric::unit(value.danger, previous.danger)
    };
    density = sanitized.density;
    drift = 1.0f - sanitized.calm;
    ecosystem.setDesire(sanitized);
}

void BiomaEngine::setLensGesture(bool held, Ecosystem::Vec3 target) noexcept
{
    constexpr std::size_t performerObserver = 0;
    if (held)
    {
        if (ecosystem.holdLens(performerObserver))
            ecosystem.setLensTarget(target);
    }
    else
        ecosystem.releaseLens(performerObserver);
}

void BiomaEngine::setEcosystemRenderMode(EcosystemRenderMode value) noexcept
{
    setEcosystemBlend(value == EcosystemRenderMode::Active ? 1.0f : 0.0f);
}

BiomaEngine::EcosystemRenderMode BiomaEngine::ecosystemRenderMode() const noexcept
{
    return targetEcosystemBlend >= 0.5f ? EcosystemRenderMode::Active
                                       : EcosystemRenderMode::Neutral;
}

void BiomaEngine::setEcosystemBlend(float value) noexcept
{
    const auto bounded = numeric::unit(value, targetEcosystemBlend);
    if (bounded == targetEcosystemBlend)
        return;
    targetEcosystemBlend = bounded;
    const auto distance = std::abs(targetEcosystemBlend - currentEcosystemBlend);
    ecosystemBlendSamplesRemaining = distance > 0.0f
        ? static_cast<std::uint32_t>(std::max(
            1.0, std::ceil(static_cast<double>(distance) * ecosystemBlendRampSamples)))
        : 0u;
}

void BiomaEngine::setEcosystemBlendImmediately(float value) noexcept
{
    setEcosystemBlend(value);
    currentEcosystemBlend = targetEcosystemBlend;
    ecosystemBlendSamplesRemaining = 0;
}

void BiomaEngine::setEcosystemOutputFeedbackEnabled(bool value) noexcept
{
    outputFeedbackEnabled = value;
}

void BiomaEngine::setMultiTapSpaceReturn(float value) noexcept
{
    if (!std::isfinite(value))
        return;
    targetMultiTapSpaceReturn = std::clamp(value, 0.0f, 0.75f);
}

void BiomaEngine::setMultiTapSpacePerformance(
    const MultiTapSpace::PerformanceIntent& value) noexcept
{
    multiTapSpace->setPerformanceIntent(value);
}

void BiomaEngine::setSpectralEnvironmentPerformance(
    SpectralEnvironmentPerformance value) noexcept
{
    targetSpectralEnvironment.influence = numeric::unit(
        value.influence, targetSpectralEnvironment.influence);
    targetSpectralEnvironment.filterMix = numeric::unit(
        value.filterMix, targetSpectralEnvironment.filterMix);
    targetSpectralEnvironment.centre = numeric::unit(
        value.centre, targetSpectralEnvironment.centre);
    targetSpectralEnvironment.span = numeric::unit(
        value.span, targetSpectralEnvironment.span);
    targetSpectralEnvironment.resonance = numeric::unit(
        value.resonance, targetSpectralEnvironment.resonance);
    targetSpectralEnvironment.ambience = numeric::unit(
        value.ambience, targetSpectralEnvironment.ambience);
}

void BiomaEngine::setSyntheticTailPerformance(
    SyntheticTailPerformance value) noexcept
{
    targetSyntheticTail.influence = numeric::unit(
        value.influence, targetSyntheticTail.influence);
    targetSyntheticTail.decay = numeric::unit(
        value.decay, targetSyntheticTail.decay);
    targetSyntheticTail.damping = numeric::unit(
        value.damping, targetSyntheticTail.damping);
    targetSyntheticTail.nightmare = numeric::unit(
        value.nightmare, targetSyntheticTail.nightmare);
}

void BiomaEngine::setCreativeMasterInfluence(float value) noexcept
{
    targetCreativeMasterInfluence = numeric::unit(
        value, targetCreativeMasterInfluence);
}

void BiomaEngine::setGranularStrikerPerformance(
    const BiomaGranularStriker::PerformanceIntent& value) noexcept
{
    // Zero preserves the autonomous balance. Manual influence also raises
    // this transient body into the full mix; the 15 ms ramp avoids clicks.
    // Up to 40x (was 8x): at 8x the striker stayed ~30 dB under the mix and
    // its controls were not heard (author's test 2026-09-18/20; measured again
    // in the 2026-10-20 review by ControlResponseTests).
    strikerMixTarget = 1.0f + 39.0f * numeric::unit(value.influence, 0.0f);
    for (auto& source : autonomousVoice.organism)
        source.setGranularStrikerPerformance(value);
    for (auto& voice : voices)
        for (auto& source : voice.organism)
            source.setGranularStrikerPerformance(value);
}

void BiomaEngine::setResonatorPerformance(
    const BiomaModalResonator::PerformanceIntent& value) noexcept
{
    for (auto& source : autonomousVoice.organism)
        source.setResonatorPerformance(value);
    for (auto& voice : voices)
        for (auto& source : voice.organism)
            source.setResonatorPerformance(value);
}

void BiomaEngine::setSpeciesGain(Ecosystem::Species species, float value) noexcept
{
    if (!std::isfinite(value))
        return;
    speciesGains[speciesGainSlot(species)] = std::clamp(value, 0.0f, 2.0f);
}

float BiomaEngine::speciesGain(Ecosystem::Species species) const noexcept
{
    return speciesGains[speciesGainSlot(species)];
}

BiomaEngine::Voice& BiomaEngine::findVoiceFor(int midiNote) noexcept
{
    for (auto& voice : voices)
        if (voice.gate && voice.midiNote == midiNote)
            return voice;
    for (auto& voice : voices)
        if (!voice.gate && voice.envelope <= 0.0001f)
            return voice;
    return *std::min_element(voices.begin(), voices.end(), [](const Voice& left, const Voice& right) {
        return left.envelope < right.envelope;
    });
}

void BiomaEngine::resetAutonomousVoice() noexcept
{
    autonomousVoice = {};
    autonomousDcInput.fill(0.0f);
    autonomousDcOutput.fill(0.0f);
    liquidCoefficient.fill(1.0f);
    liquidState.fill(0.0f);
    liquidCurrent.fill(autonomousEnergy.ecologicalMusicDirector().performance().influence);
    lastAutonomousLifeEvent.fill(0);
    for (std::size_t index = 0; index < autonomousVoice.frequency.size(); ++index)
    {
        autonomousVoice.organism[index].prepare(sampleRate);
        autonomousVoice.organism[index].setSeed(
            engineSeed ^ 0x43d2f1a7u
                ^ (0x85ebca6bu * static_cast<std::uint32_t>(index + 1)));
        autonomousVoice.organism[index].setPulseWidth(sessionPulseWidth(index));
        autonomousVoice.spatializer[index].prepare(sampleRate);
    }
    autonomousVoice.physicalString.prepare(sampleRate);
    autonomousVoice.physicalString.setSeed(engineSeed ^ 0x52c81f3bu);
    for (std::size_t index = 0; index < timbreBodies.size(); ++index)
    {
        timbreBodies[index].setKind(timbre.body[index]);
        timbreBodies[index].prepare(sampleRate,
            engineSeed ^ 0x1b873593u ^ (0x9e3779b9u * static_cast<std::uint32_t>(index + 1)));
    }
    timbreMixCurrent = timbreMixTarget;
    timbreMixSmoothing = static_cast<float>(1.0 - std::exp(-1.0 / (sampleRate * 0.040)));
}

void BiomaEngine::setTimbrePerformance(const TimbrePerformance& value) noexcept
{
    timbre = value;
    for (std::size_t index = 0; index < timbreBodies.size(); ++index)
    {
        const auto kind = static_cast<std::size_t>(value.body[index]) < TimbreBody::kindCount
            ? value.body[index] : TimbreBody::Kind::native;
        timbre.body[index] = kind;
        timbre.matter[index] = numeric::unit(value.matter[index], 0.5f);
        timbre.mix[index] = numeric::unit(value.mix[index], 0.0f);
        timbreBodies[index].setKind(kind);
    }
    timbre.ecology = numeric::unit(value.ecology, 0.0f);
}

// Control rate. MATTER and presence of each organism's second body, moved by
// what the ecosystem lives - a tendency, as everywhere in the instrument:
//   the water: ice hardens and brightens, cold softens, boiling sharpens,
//     turbulence makes the matter waver;
//   the web: a hunter sharpens and comes forward, a prey softens (breath),
//     a courting pair moves to the middle of its range (the open vowel),
//     rivals and guards harden;
//   vigour brightens; a damaged vessel darkens.
void BiomaEngine::updateTimbreTargets() noexcept
{
    const auto& music = autonomousEnergy.ecologicalMusicDirector();
    const auto& telemetry = music.telemetry();
    const auto ecology = timbre.ecology;
    static constexpr std::array<float, 5> phaseOffset { 0.30f, -0.15f, 0.0f, 0.10f, 0.20f };
    const auto water = phaseOffset[static_cast<std::size_t>(telemetry.phase)];
    timbreWobblePhase += 0.35f / 200.0f;
    timbreWobblePhase -= std::floor(timbreWobblePhase);
    const auto health = ecosystem.integrityState().health;
    for (std::size_t i = 0; i < timbreBodies.size(); ++i)
    {
        const auto& organism = ecosystem.organism(i);
        auto offset = water + (organism.energy - 0.5f) * 0.3f - (1.0f - health) * 0.3f;
        if (telemetry.phase == EcologicalMusicDirector::Phase::turbulent)
            offset += 0.10f * std::sin(6.2831853f * (timbreWobblePhase + 0.2f * static_cast<float>(i)));
        auto presence = 0.0f;
        for (std::size_t b = 0; b < timbreBodies.size(); ++b)
        {
            if (b == i)
                continue;
            const auto bond = telemetry.bond[i][b];
            switch (telemetry.relation[i][b])
            {
                case EcologicalMusicDirector::Relation::hunts:
                    offset += 0.25f * bond; presence += 0.30f * bond; break;
                case EcologicalMusicDirector::Relation::flees:
                    offset -= 0.20f * bond; presence += 0.20f * bond; break;
                case EcologicalMusicDirector::Relation::courts:
                    offset += (0.5f - timbre.matter[i]) * 0.6f * bond; presence += 0.25f * bond; break;
                case EcologicalMusicDirector::Relation::rivals:
                    offset += 0.15f * bond; break;
                case EcologicalMusicDirector::Relation::guards:
                    offset += 0.10f * bond; break;
                default: break;
            }
        }
        const auto chosen = timbre.body[i] != TimbreBody::Kind::native;
        const auto matter = std::clamp(timbre.matter[i] + ecology * offset, 0.0f, 1.0f);
        timbreMixTarget[i] = chosen ? std::clamp(timbre.mix[i] + ecology * presence, 0.0f, 1.0f) : 0.0f;
        timbreState.matter[i] = matter;
        timbreState.mix[i] = timbreMixTarget[i];
        timbreBodies[i].control(autonomousEnergy.voice(i).frequency, matter);

        if (i == 0)
        {
            autonomousEnergy.setHabitatHealth(habitat->reading());
            {
                auto reading = roomEnabled ? room.reading() : RoomCoupling::Reading {};
                // the SALA card: the room read louder or softer, the music
                // reacting more or less; the classic values touch nothing
                if (roomEnabled && reading.active && (roomSensitivityDb != 0.0f || roomReaction != 1.0f))
                {
                    reading.levelDb += roomSensitivityDb;
                    reading.strength = std::clamp((reading.levelDb - RoomCoupling::floorDb) / 40.0f * roomReaction,
                                                  0.0f, 1.0f);
                }
                autonomousEnergy.setRoom(reading);
            }
            // the water bubbles with the medium: ~30/s turbulent, ~240/s boiling
            const auto& director = autonomousEnergy.ecologicalMusicDirector();
            const auto influence = director.performance().influence;
            bubblesActive = influence > 0.0f;
            bubbleField.setRate(bubblesActive ? director.bubbling() * 400.0f * influence : 0.0f);
        }
        // the organism's partials, for the ecological harmony: its native
        // body blended with its second body by the mix (Sethares 1993)
        using SC = SpectralConsonance;
        const auto native = EcologicalMusicDirector::nativeSpectrum(organism.species);
        if (timbreMixTarget[i] <= 0.0f)
            autonomousEnergy.setEcologicalMusicSpectrum(i, native);
        else
        {
            SC::Spectrum body;
            switch (timbre.body[i])
            {
                case TimbreBody::Kind::voice:
                case TimbreBody::Kind::choir: body = SC::harmonic(6, 1.1f); break;
                case TimbreBody::Kind::bow: body = SC::harmonic(8, 1.0f); break;   // Helmholtz motion: a sawtooth
                case TimbreBody::Kind::reed: body = SC::odd(5); break;             // closed-open tube
                case TimbreBody::Kind::matter:
                    body = SC::blend(SC::harmonic(4, 1.0f), 1.0f - matter, SC::membrane(), matter); break;
                case TimbreBody::Kind::breath: body = SC::harmonic(3, 1.5f); break;
                case TimbreBody::Kind::string: body = SC::harmonic(6, 1.0f); break;
                case TimbreBody::Kind::drop: body = SC::sine(); break;
                case TimbreBody::Kind::fold: body = SC::odd(static_cast<std::uint8_t>(2 + matter * 4.0f)); break;
                case TimbreBody::Kind::membrane: body = SC::membrane(); break;
                case TimbreBody::Kind::glass: body = SC::glass(); break;
                case TimbreBody::Kind::native: body = native; break;
            }
            autonomousEnergy.setEcologicalMusicSpectrum(
                i, SC::blend(native, 1.0f - timbreMixTarget[i], body, timbreMixTarget[i]));
        }
    }
}

void BiomaEngine::noteOn(int midiNote, float velocity) noexcept
{
    if (!std::isfinite(velocity))
        return;
    const auto boundedMidiNote = std::clamp(midiNote, 0, 127);
    auto& voice = findVoiceFor(boundedMidiNote);
    const auto continuingSameNote = voice.gate && voice.midiNote == boundedMidiNote;
    const auto root = 440.0f
        * std::pow(2.0f, (static_cast<float>(boundedMidiNote) - 69.0f) / 12.0f);
    voice.midiNote = boundedMidiNote;
    voice.velocity = std::clamp(velocity, 0.0f, 1.0f);
    if (!continuingSameNote)
    {
        for (auto& source : voice.organism)
            source.reset();
        voice.physicalString.reset();
        for (auto& spatializer : voice.spatializer)
            spatializer.reset();
    }
    for (std::size_t index = 0; index < voice.frequency.size(); ++index)
    {
        voice.frequency[index] = root * (1.0f + (brain.nextRandomUnit() - 0.5f) * drift * 0.18f)
            * static_cast<float>(index + 1) * 0.5f;
        voice.organism[index].setPulseWidth(sessionPulseWidth(index));
    }
    const auto desire = ecosystem.desireCurrent();
    const auto integrity = ecosystem.integrityState();
    BiomaModalResonator::Parameters resonatorParameters;
    resonatorParameters.fundamentalHz = voice.frequency[2];
    resonatorParameters.material = std::clamp(0.18f + density * 0.62f, 0.0f, 1.0f);
    resonatorParameters.decaySeconds = 0.20f + (1.0f - drift) * 2.2f;
    resonatorParameters.brightness = std::clamp(
        0.20f + voice.velocity * 0.58f + density * 0.18f, 0.0f, 1.0f);
    resonatorParameters.hardness = std::clamp(
        0.18f + voice.velocity * 0.72f, 0.0f, 1.0f);
    resonatorParameters.strikePosition = std::clamp(
        0.08f + static_cast<float>(boundedMidiNote) / 127.0f * 0.84f, 0.02f, 0.98f);
    for (std::size_t index = 0; index < voice.organism.size(); ++index)
    {
        const auto& organism = ecosystem.organism(index);
        if (organism.species == Ecosystem::Species::Resonator)
        {
            resonatorParameters.fundamentalHz = voice.frequency[index];
            voice.organism[index].setResonatorParameters(resonatorParameters);
            voice.organism[index].exciteResonator(voice.velocity);
        }
        else if (organism.species == Ecosystem::Species::GranularStriker)
        {
            voice.organism[index].setGranularStrikerParameters(granularParameters(
                voice.frequency[index], organism, desire, ecosystem.audioMix(index).detail,
                voice.velocity));
            voice.organism[index].exciteGranularStriker(std::clamp(
                voice.velocity * (0.55f + organism.strength * 0.45f)
                    * integrity.health,
                0.0f, 1.0f));
        }
        else if (organism.species == Ecosystem::Species::PhysicalString)
        {
            auto parameters = physicalStringParameters(
                voice.frequency[index], organism, desire,
                ecosystem.environmentState(), ecosystem.audioMix(index).detail,
                voice.velocity);
            parameters.decaySeconds *= 0.65f + integrity.health * 0.35f;
            voice.physicalString.setParameters(parameters);
            voice.physicalString.excite(std::clamp(
                voice.velocity * (0.50f + organism.strength * 0.50f)
                    * integrity.health,
                0.0f, 1.0f));
        }
    }
    voice.gate = true;
}

void BiomaEngine::noteOff(int midiNote) noexcept
{
    for (auto& voice : voices)
        if (voice.midiNote == midiNote)
            voice.gate = false;
}

void BiomaEngine::noteOff() noexcept
{
    for (auto& voice : voices)
        voice.gate = false;
}

std::size_t BiomaEngine::activeVoiceCount() const noexcept
{
    return static_cast<std::size_t>(std::count_if(voices.begin(), voices.end(), [](const Voice& voice) {
        return voice.gate || voice.envelope > 0.0001f;
    }));
}

BiomaEngine::Telemetry BiomaEngine::telemetry() const noexcept
{
    Telemetry result;
    result.densityTarget = density;
    result.densityCurrent = audioDensity;
    result.feedbackTaps = taps;
    result.brainRegister = brain.registerBits();
    {
        auto mood = 0.0f;
        for (const auto v : brain.correlation())
            mood += v;
        result.brainMood = mood / static_cast<float>(Ecosystem::organismCount);
    }
    result.room = room.reading();
    result.roomEnabled = roomEnabled;
    result.driftTarget = drift;
    result.driftCurrent = audioDrift;
    result.renderMode = ecosystemRenderMode();
    result.ecosystemBlendTarget = targetEcosystemBlend;
    result.ecosystemBlendCurrent = currentEcosystemBlend;
    result.outputFeedbackEnabled = outputFeedbackEnabled;
    result.autonomousEnabled = autonomousEnergy.enabled();
    result.granularStrikerEnabled = granularStrikerEnabled;
    result.modalResonatorEnabled = modalResonatorEnabled;
    result.spatializerEnabled = spatializerEnabled;
    result.multiTapSpaceEnabled = multiTapSpaceEnabled;
    result.aquariumMotionAudioEnabled = ecosystem.aquariumMotionAudioEnabled();
    result.activeVoices = activeVoiceCount();
    result.outputEnergy = lastOutputEnergy;
    result.masterFeedback = masterFeedback.value();
    result.multiTapSpaceReturnTarget = targetMultiTapSpaceReturn;
    result.multiTapSpaceReturnCurrent = currentMultiTapSpaceReturn;
    result.spectralEnvironmentTarget = targetSpectralEnvironment;
    result.spectralEnvironmentCurrent = currentSpectralEnvironment;
    result.syntheticTailTarget = targetSyntheticTail;
    result.syntheticTailCurrent = currentSyntheticTail;
    result.autonomy = autonomousEnergy.telemetry();
    result.desire = ecosystem.desireCurrent();
    result.environment = ecosystem.environmentState();
    result.integrity = ecosystem.integrityState();
    result.lens = ecosystem.lensState();
    result.lensFocusedOrganisms = ecosystem.lensFocusedOrganisms();
    result.multiTapSpace = multiTapSpace->telemetry();
    result.syntheticTail = syntheticTail->telemetry();
    result.moduleBank = moduleBank->telemetry();
    result.creativeMaster = masterChain->telemetry();
    result.output = outputStage->telemetry();
    for (std::size_t index = 0; index < Ecosystem::organismCount; ++index)
    {
        result.organisms[index] = ecosystem.organism(index);
        result.mix[index] = ecosystem.mix(index);
        result.sources[index] = autonomousVoice.organism[index].telemetry(
            result.organisms[index].species, modalResonatorEnabled,
            granularStrikerEnabled);
        result.spatial[index] = autonomousVoice.spatializer[index].telemetry();
        if (result.organisms[index].species == Ecosystem::Species::GranularStriker)
            result.granularStriker = result.sources[index].granularStriker;
        if (result.organisms[index].species == Ecosystem::Species::Resonator)
            result.resonator = result.sources[index].resonator;
        if (result.organisms[index].species == Ecosystem::Species::PhysicalString)
        {
            result.sources[index].implemented = true;
            result.sources[index].generator = OrganismVoiceEngine::Generator::PhysicalString;
            result.physicalString = autonomousVoice.physicalString.telemetry();
        }
    }
    return result;
}

float BiomaEngine::advanceEcosystemBlend() noexcept
{
    if (ecosystemBlendSamplesRemaining == 0)
        return currentEcosystemBlend;
    if (ecosystemBlendSamplesRemaining == 1)
        currentEcosystemBlend = targetEcosystemBlend;
    else
        currentEcosystemBlend += (targetEcosystemBlend - currentEcosystemBlend)
            / static_cast<float>(ecosystemBlendSamplesRemaining);
    --ecosystemBlendSamplesRemaining;
    return currentEcosystemBlend;
}

float BiomaEngine::advanceMultiTapSpaceReturn() noexcept
{
    currentMultiTapSpaceReturn +=
        (targetMultiTapSpaceReturn - currentMultiTapSpaceReturn)
        * multiTapSpaceReturnSmoothing;
    if (std::abs(targetMultiTapSpaceReturn - currentMultiTapSpaceReturn) < 1.0e-7f)
        currentMultiTapSpaceReturn = targetMultiTapSpaceReturn;
    return currentMultiTapSpaceReturn;
}

void BiomaEngine::updateMultiTapSpaceParameters() noexcept
{
    auto parameters = MultiTapSpace::aquariumEarlyReflections();
    const auto environment = ecosystem.environmentState();
    const auto desire = ecosystem.desireCurrent();
    const auto integrity = ecosystem.integrityState();

    // The vessel stays recognisable while daylight, temperature, collective
    // behaviour and damage bend its reflection field in bounded ranges.
    const auto sizeScale = 0.92f + environment.temperature * 0.10f
        + integrity.crackStress * 0.12f;
    // the Brain's mood (the mean of its correlated leanings) opens or closes
    // the reflections by up to 10 %, as far as the SPACE gesture reaches;
    // at zero it is the former spread exactly (2026-10-23)
    const auto& lean = brain.correlation();
    auto mood = 0.0f;
    for (const auto value : lean)
        mood += value;
    mood /= static_cast<float>(lean.size());
    const auto brainSpread = 1.0f + 0.10f * mood * multiTapSpace->performanceIntent().influence;
    const auto spreadScale = std::clamp(
        0.88f + (1.0f - desire.cohesion) * 0.14f
            + desire.danger * 0.08f - integrity.crackStress * 0.10f,
        0.72f, 1.08f) * brainSpread;
    const auto toneScale = (0.70f + environment.daylight * 0.30f)
        * (0.80f + integrity.health * 0.20f);
    const auto gainScale = 0.88f + desire.density * 0.12f
        + environment.matingPulse * 0.04f;
    const auto composition = autonomousEnergy.compositionPlan();
    for (auto& tap : parameters.taps)
    {
        tap.delayMilliseconds *= sizeScale;
        tap.pan = std::clamp(tap.pan * spreadScale, -1.0f, 1.0f);
        tap.cutoffHz *= toneScale;
        tap.gain = std::clamp(tap.gain * gainScale, 0.0f, 1.0f);
    }
    parameters.feedback = std::clamp(
        0.22f + desire.calm * 0.13f + desire.density * 0.04f
            + integrity.crackStress * 0.08f
            + (composition.spaceWeight - 1.0f)
                * composition.influence * 0.06f,
        0.0f, 0.50f);
    multiTapSpace->setParameters(parameters);
    multiTapSpace->setDriftIntent({
        0.06f + audioDrift * 0.18f,
        0.07f + desire.density * 0.36f,
        0.55f + integrity.crackStress * 1.10f
    });
}

void BiomaEngine::updateSpectralEnvironmentParameters() noexcept
{
    const SchoolSpectralBank::Parameters filters {
        currentSpectralEnvironment.centre,
        currentSpectralEnvironment.span,
        currentSpectralEnvironment.resonance
    };
    schoolSpectralLeft.setParameters(filters);
    schoolSpectralRight.setParameters(filters);

    const auto environment = ecosystem.environmentState();
    const auto integrity = ecosystem.integrityState();
    auto turbidity = 0.0f;
    for (std::size_t index = 0; index < Ecosystem::organismCount; ++index)
        turbidity += ecosystem.organism(index).turbidity;
    turbidity /= static_cast<float>(Ecosystem::organismCount);
    ecologicalNoise.setParameters({
        environment.seasonPhase, environment.dayPhase, turbidity,
        environment.temperature, integrity.crackStress, 0.035f
    });
}

void BiomaEngine::updateSyntheticTailParameters() noexcept
{
    // SIZE is deliberately fixed until delay-length modulation receives a
    // discontinuity-free implementation. The public DECAY macro maps
    // exponentially enough for short rooms and long synthetic suspensions.
    const auto decaySeconds = 0.30f
        * std::pow(40.0f, currentSyntheticTail.decay);
    syntheticTail->setParameters({
        0.55f, decaySeconds, currentSyntheticTail.damping, 0.75f,
        currentSyntheticTail.nightmare, 0.35f
    });
}

void BiomaEngine::updateCreativeMasterParameters() noexcept
{
    const auto desire = ecosystem.desireCurrent();
    const auto environment = ecosystem.environmentState();
    const auto integrity = ecosystem.integrityState();
    const auto& shape = creativeMasterShape;
    masterChain->setParameters({
        targetCreativeMasterInfluence,
        std::clamp(0.25f + desire.density * 0.65f + shape.multiband, 0.0f, 1.0f),
        std::clamp(0.20f + desire.calm * 0.60f + shape.glue, 0.0f, 1.0f),
        std::clamp((environment.daylight - 0.5f) * 0.50f + shape.tilt, -1.0f, 1.0f),
        std::clamp(0.85f + (1.0f - desire.cohesion) * 0.45f + shape.width, 0.25f, 1.75f),
        integrity.crackStress,
        creativeMasterEqAfterDynamics
    });
}

void BiomaEngine::render(float* left, float* right, std::size_t samples) noexcept
{
    if (left == nullptr || right == nullptr) return;
    // denormals flushed to zero while rendering, whatever the host does
    const ScopedFlushDenormals noDenormals;
    const auto attack = 1.0f / static_cast<float>(sampleRate * 0.025);
    const auto release = 1.0f / static_cast<float>(sampleRate * 0.45);
    for (std::size_t sample = 0; sample < samples; ++sample)
    {
        if (binauralMix != binauralTarget)
        {
            if (binauralMix <= 0.0f)
            {
                for (auto& head : binaural)
                    head.reset();  // a fresh head, no stale buffers
                for (auto& head : midiBinaural)
                    head.reset();
            }
            binauralMix = binauralTarget > binauralMix ? std::min(binauralTarget, binauralMix + binauralStep)
                                                       : std::max(binauralTarget, binauralMix - binauralStep);
        }
        // These direct source controls used to bypass the ecological ramp,
        // jumping oscillator shape, pulsar duty and acoustic gain together.
        audioDensity += (density - audioDensity) * directControlSmoothing;
        // SUPPORT glides too (a jump of +24 dB would click)
        if (supportCurrent != supportTarget)
        {
            supportCurrent += (supportTarget - supportCurrent) * directControlSmoothing;
            if (std::abs(supportTarget - supportCurrent) < 1.0e-6f)
                supportCurrent = supportTarget;
        }
        // SYNTHESIS glides; it lands exactly on its target, so that 0 is
        // still the previous sound, bit for bit
        if (synthMix != synthMixTarget)
        {
            synthMix += (synthMixTarget - synthMix) * directControlSmoothing;
            if (std::abs(synthMixTarget - synthMix) < 1.0e-6f)
                synthMix = synthMixTarget;
        }
        audioDrift += (drift - audioDrift) * directControlSmoothing;
        strikerMixCurrent += (strikerMixTarget - strikerMixCurrent)
            * directControlSmoothing;
        const auto smoothSpectral = [this](float& current, float target) {
            current += (target - current) * directControlSmoothing;
            if (std::abs(target - current) < 1.0e-7f)
                current = target;
        };
        smoothSpectral(currentSpectralEnvironment.influence,
                       targetSpectralEnvironment.influence);
        smoothSpectral(currentSpectralEnvironment.filterMix,
                       targetSpectralEnvironment.filterMix);
        smoothSpectral(currentSpectralEnvironment.centre,
                       targetSpectralEnvironment.centre);
        smoothSpectral(currentSpectralEnvironment.span,
                       targetSpectralEnvironment.span);
        smoothSpectral(currentSpectralEnvironment.resonance,
                       targetSpectralEnvironment.resonance);
        smoothSpectral(currentSpectralEnvironment.ambience,
                       targetSpectralEnvironment.ambience);
        smoothSpectral(currentSyntheticTail.influence,
                       targetSyntheticTail.influence);
        smoothSpectral(currentSyntheticTail.decay,
                       targetSyntheticTail.decay);
        smoothSpectral(currentSyntheticTail.damping,
                       targetSyntheticTail.damping);
        smoothSpectral(currentSyntheticTail.nightmare,
                       targetSyntheticTail.nightmare);
        const auto ecosystemBlend = advanceEcosystemBlend();
        ecosystem.tickSample(brain);
        // audioMix moves every sample. Recomputing polar pan and filter
        // coefficients from it every sample defeats the control-rate contract.
        // Targets use the existing sample-clocked 200 Hz cadence; each acoustic
        // body still smooths gains (20 ms) and tone (45 ms) at audio rate.
        const auto updateSpatialTargets = samplesUntilMultiTapSpaceControl == 0;
        if (updateSpatialTargets)
        {
            updateTimbreTargets();
            autonomousEnergy.setBrainLean(brain.correlation());
            updateMultiTapSpaceParameters();
            updateSpectralEnvironmentParameters();
            updateSyntheticTailParameters();
            updateCreativeMasterParameters();
            const auto masterState = masterChain->telemetry();
            brain.setEcologicalFeedback(masterFeedback.process(
                masterState.bandGainReductionDb,
                masterState.glueGainReductionDb,
                masterState.currentInfluence));
            samplesUntilMultiTapSpaceControl = multiTapSpaceControlInterval;
        }
        --samplesUntilMultiTapSpaceControl;
        autonomousEnergy.tickSample(ecosystem, lastOutputEnergy);
        const auto organismDrift = brain.nextDrift(audioDrift);
        const auto crackStress = ecosystem.integrityState().crackStress;
        float l = 0.0f, r = 0.0f;
        for (std::size_t voiceIndex = 0; voiceIndex < voices.size(); ++voiceIndex)
        {
            auto& voice = voices[voiceIndex];
            voice.envelope += voice.gate ? attack : -release;
            voice.envelope = std::clamp(voice.envelope, 0.0f, 1.0f);
            if (voice.envelope <= 0.0f) continue;
            const auto voicePan = (static_cast<float>(voiceIndex) - 1.5f) / 1.5f * 0.32f;
            const auto voiceLeft = std::sqrt((1.0f - voicePan) * 0.5f);
            const auto voiceRight = std::sqrt((1.0f + voicePan) * 0.5f);
            for (std::size_t organism = 0; organism < voice.organism.size(); ++organism)
            {
                const auto& organismState = ecosystem.organism(organism);
                const auto species = organismState.species;
                const auto& ecologicalMix = ecosystem.audioMix(organism);
                const auto instantaneousFrequency = voice.frequency[organism]
                    * (1.0f + organismDrift[organism] * 0.025f);
                const auto neutralShape = audioDensity * 0.72f
                    + std::abs(organismDrift[organism]) * 0.28f;
                const auto activeShape = audioDensity * 0.66f
                    + std::abs(organismDrift[organism]) * 0.24f
                    + ecologicalMix.detail * 0.10f;
                const auto shape = morphValue(neutralShape, activeShape, ecosystemBlend);
                const auto neutralPulsarDensity = audioDensity + organismDrift[organism] * 0.18f;
                const auto activePulsarDensity = audioDensity + organismDrift[organism] * 0.16f
                    + (ecologicalMix.detail - 0.5f) * 0.08f;
                const auto pulsarDensity = morphValue(neutralPulsarDensity,
                                                      activePulsarDensity,
                                                      ecosystemBlend);
                OrganismVoiceEngine::Controls sourceControls;
                sourceControls.species = species;
                sourceControls.frequency = instantaneousFrequency;
                sourceControls.shape = std::clamp(shape, 0.0f, 1.0f);
                sourceControls.pulsarDensity = std::clamp(pulsarDensity, 0.0f, 1.0f);
                sourceControls.pulsarCrackStress = crackStress;
                sourceControls.strength = organismState.strength;
                sourceControls.agility = organismState.agility;
                sourceControls.intelligence = organismState.intelligence;
                sourceControls.granularStrikerEnabled = granularStrikerEnabled;
                sourceControls.modalResonatorEnabled = modalResonatorEnabled;
                const auto bodySample = species == Ecosystem::Species::PhysicalString
                    ? voice.physicalString.tick()
                    : voice.organism[organism].renderSample(sourceControls);
                const auto biased = bodySample * bodyCalibrationGain(species)
                    * speciesGain(species) * supportFactor(species)
                    * (species == Ecosystem::Species::GranularStriker
                        ? strikerMixCurrent : 1.0f);
                // the autonomous route's DC blocker, here too (2026-10-22)
                const auto raw = biased - voice.dcInput[organism]
                    + autonomousDcCoefficient * voice.dcOutput[organism];
                voice.dcInput[organism] = biased;
                voice.dcOutput[organism] = raw;
                const auto neutralSignal = raw * voice.envelope * voice.velocity
                    * (0.028f + audioDensity * 0.032f);
                const auto organismPan = organismPanPosition(organism) * 0.35f;
                const auto pan = std::clamp(voicePan + organismPan
                                                + organismDrift[organism] * 0.18f,
                                            -1.0f, 1.0f);
                const auto neutralLeft = neutralSignal * (1.0f - pan) * 0.5f;
                const auto neutralRight = neutralSignal * (1.0f + pan) * 0.5f;

                const auto activeSignal = raw * voice.envelope * voice.velocity
                    * (0.045f + audioDensity * 0.050f);
                const auto directLeftGain = ecologicalMix.left * voiceLeft * 1.41421356f;
                const auto directRightGain = ecologicalMix.right * voiceRight * 1.41421356f;
                if (updateSpatialTargets)
                    voice.spatializer[organism].setGainPair(
                        directLeftGain, directRightGain, spatialDepth(ecologicalMix),
                        std::clamp(1.0f - crackStress * 0.72f, 0.28f, 1.0f));
                const auto spatial = voice.spatializer[organism].tick(activeSignal);
                const auto activeLeft = spatializerEnabled
                    ? spatial.left : activeSignal * directLeftGain;
                const auto activeRight = spatializerEnabled
                    ? spatial.right : activeSignal * directRightGain;

                // The two routes are strongly correlated because they share a
                // source. Linear interpolation avoids the +3 dB hump that an
                // equal-power crossfade would create at the centre.
                if (binauralMix > 0.0f)
                {
                    // headphones (2026-10-22): as the autonomous route, the
                    // active part crossfades into the organism's head; the
                    // head's share is summed here and heard once per organism
                    l += neutralLeft + (activeLeft * (1.0f - binauralMix) - neutralLeft) * ecosystemBlend;
                    r += neutralRight + (activeRight * (1.0f - binauralMix) - neutralRight) * ecosystemBlend;
                    midiBinauralInput[organism] += (activeLeft + activeRight) * binauralLevel
                        * binauralMix * ecosystemBlend;
                }
                else
                {
                    l += morphValue(neutralLeft, activeLeft, ecosystemBlend);
                    r += morphValue(neutralRight, activeRight, ecosystemBlend);
                }
            }
        }
        // the MIDI voices through their organisms' heads
        if (binauralMix > 0.0f)
            for (std::size_t organism = 0; organism < midiBinaural.size(); ++organism)
            {
                const auto heard = midiBinaural[organism].tick(midiBinauralInput[organism]);
                l += heard.left;
                r += heard.right;
                midiBinauralInput[organism] = 0.0f;
            }

        const auto ambisonicActive = ambisonics.capacity > sample && ambisonics.channel[0] != nullptr;
        FoaSample ambisonicOrganisms {};
        float organismsAddedLeft = 0.0f, organismsAddedRight = 0.0f;
        for (std::size_t organism = 0; organism < autonomousVoice.organism.size(); ++organism)
        {
            const auto& autonomousState = autonomousEnergy.voice(organism);
            const auto& organismState = ecosystem.organism(organism);
            const auto& ecologicalMix = ecosystem.audioMix(organism);
            const auto instantaneousFrequency = autonomousState.frequency
                * (1.0f + organismDrift[organism] * 0.014f);
            const auto neutralShape = audioDensity * 0.58f
                + std::abs(organismDrift[organism]) * 0.24f
                + autonomousState.energy * 0.10f;
            const auto activeShape = audioDensity * 0.54f
                + std::abs(organismDrift[organism]) * 0.21f
                + ecologicalMix.detail * 0.15f
                + autonomousState.energy * 0.10f;
            // the session's colour for this body (0 in the classic world)
            const auto shape = morphValue(neutralShape, activeShape, ecosystemBlend)
                + sessionWorld.shapeOffsets[organism];
            const auto pulsarDensity = audioDensity + organismDrift[organism] * 0.14f
                + (ecologicalMix.detail - 0.5f) * ecosystemBlend * 0.10f;

            if (autonomousState.lifeEvents != lastAutonomousLifeEvent[organism])
            {
                lastAutonomousLifeEvent[organism] = autonomousState.lifeEvents;
                if (autonomousState.awake && autonomousState.geometricGate)
                    timbreBodies[organism].excite(0.45f + autonomousState.geometricAccent * 0.55f);
                if (autonomousState.awake && autonomousState.geometricGate)
                {
                    const auto environment = ecosystem.environmentState();
                    const auto desire = ecosystem.desireCurrent();
                    const auto integrity = ecosystem.integrityState();
                    if (organismState.species == Ecosystem::Species::Resonator)
                    {
                        BiomaModalResonator::Parameters resonatorParameters;
                        resonatorParameters.fundamentalHz = instantaneousFrequency;
                        resonatorParameters.material = std::clamp(
                            0.12f + organismState.turbidity * 0.56f
                                + environment.temperature * 0.24f,
                            0.0f, 1.0f);
                        resonatorParameters.decaySeconds = (0.18f + desire.calm * 2.8f)
                            * (0.55f + integrity.health * 0.45f);
                        resonatorParameters.brightness = std::clamp(
                            ecologicalMix.detail * 0.65f + environment.daylight * 0.20f
                                + organismState.energy * 0.15f,
                            0.0f, 1.0f);
                        resonatorParameters.hardness = std::clamp(
                            desire.danger * 0.65f
                                + autonomousState.geometricAccent * 0.35f,
                            0.0f, 1.0f);
                        resonatorParameters.strikePosition = std::clamp(
                            0.5f + organismState.position.x
                                / (ecosystem.aquariumRadius() * 2.0f),
                            0.02f, 0.98f);
                        autonomousVoice.organism[organism].setResonatorParameters(
                            resonatorParameters);
                        autonomousVoice.organism[organism].exciteResonator(std::clamp(
                            (0.25f + organismState.energy * 0.45f
                                + autonomousState.geometricAccent * 0.30f)
                                * integrity.health,
                            0.0f, 1.0f));
                    }
                    else if (organismState.species
                             == Ecosystem::Species::GranularStriker)
                    {
                        autonomousVoice.organism[organism].setGranularStrikerParameters(
                            granularParameters(
                                instantaneousFrequency, organismState, desire,
                                ecologicalMix.detail, autonomousState.geometricAccent));
                        autonomousVoice.organism[organism].exciteGranularStriker(
                            std::clamp(
                                (0.22f + organismState.energy * 0.38f
                                    + autonomousState.geometricAccent * 0.40f)
                                    * integrity.health,
                                0.0f, 1.0f));
                    }
                    else if (organismState.species
                             == Ecosystem::Species::PhysicalString)
                    {
                        auto parameters = physicalStringParameters(
                            instantaneousFrequency, organismState, desire,
                            environment, ecologicalMix.detail,
                            autonomousState.geometricAccent);
                        parameters.decaySeconds *= 0.65f + integrity.health * 0.35f;
                        autonomousVoice.physicalString.setParameters(parameters);
                        autonomousVoice.physicalString.excite(std::clamp(
                            (0.22f + organismState.energy * 0.43f
                                + autonomousState.geometricAccent * 0.35f)
                                * integrity.health,
                            0.0f, 1.0f));
                    }
                }
            }

            // Every implemented DSP body keeps advancing while its organism
            // sleeps. Energy, not a restarted phase, determines audibility.
            OrganismVoiceEngine::Controls sourceControls;
            sourceControls.species = organismState.species;
            sourceControls.frequency = instantaneousFrequency;
            sourceControls.shape = std::clamp(shape, 0.0f, 1.0f);
            sourceControls.pulsarDensity = std::clamp(pulsarDensity, 0.0f, 1.0f);
            sourceControls.pulsarCrackStress = crackStress;
            sourceControls.strength = organismState.strength;
            sourceControls.agility = organismState.agility;
            sourceControls.intelligence = organismState.intelligence;
            sourceControls.granularStrikerEnabled = granularStrikerEnabled;
            sourceControls.modalResonatorEnabled = modalResonatorEnabled;
            const auto bodySample = organismState.species == Ecosystem::Species::PhysicalString
                ? autonomousVoice.physicalString.tick()
                : autonomousVoice.organism[organism].renderSample(sourceControls);
            auto raw = bodySample * bodyCalibrationGain(organismState.species)
                * speciesGain(organismState.species) * supportFactor(organismState.species)
                * (organismState.species == Ecosystem::Species::GranularStriker
                    ? strikerMixCurrent : 1.0f);
            // the organism's second body, mixed with the native one; skipped
            // entirely while no body is mixed in (bit-identical)
            if (timbreMixTarget[organism] > 0.0f || timbreMixCurrent[organism] > 0.0f)
            {
                timbreMixCurrent[organism] += (timbreMixTarget[organism] - timbreMixCurrent[organism])
                    * timbreMixSmoothing;
                if (timbreMixCurrent[organism] < 1.0e-5f && timbreMixTarget[organism] <= 0.0f)
                    timbreMixCurrent[organism] = 0.0f;
                const auto second = timbreBodies[organism].render(instantaneousFrequency, autonomousState.energy);
                raw += (second - raw) * timbreMixCurrent[organism];
            }
            const auto acSignal = raw - autonomousDcInput[organism]
                + autonomousDcCoefficient * autonomousDcOutput[organism];
            autonomousDcInput[organism] = raw;
            autonomousDcOutput[organism] = acSignal;
            // Frequencies made liquid: what reaches the listener crossed water,
            // which absorbs the highs more than the lows (in proportion to f^2)
            // and more in a bubbly liquid than in ice. The farther the
            // organism swims from the glass, the darker it sounds. With the
            // ecological music at zero this path is skipped: bit-identical.
            const auto& music = autonomousEnergy.ecologicalMusicDirector();
            const auto liquidTarget = music.performance().influence;
            if (liquidTarget > 0.0f || liquidCurrent[organism] > 0.0f)
            {
                liquidCurrent[organism] += (liquidTarget - liquidCurrent[organism]) * timbreMixSmoothing;
                if (std::abs(liquidTarget - liquidCurrent[organism]) < 1.0e-5f)
                    liquidCurrent[organism] = liquidTarget;
            }
            const auto liquid = liquidCurrent[organism];
            auto heardSignal = acSignal;
            if (liquid > 0.0f)
            {
                if (updateSpatialTargets)
                {
                    // age darkens the body (LifeCycle): the old lose their highs;
                    // a lo-fi reef (Schafer) blurs everyone a little
                    const auto cutoff = EcologicalMusicDirector::liquidCutoffHz(
                        music.absorption(), ecosystem.waterPathToListener(organism),
                        music.waterTemperatureC(), music.salinityPpt())
                        * (1.0f - 0.55f * music.darkness(organism))
                        * (1.0f - 0.35f * music.telemetry().lofi);
                    liquidCoefficient[organism] = static_cast<float>(
                        1.0 - std::exp(-2.0 * 3.14159265358979323846 * cutoff / sampleRate));
                }
                liquidState[organism] += (acSignal - liquidState[organism]) * liquidCoefficient[organism];
                heardSignal = acSignal + (liquidState[organism] - acSignal) * liquid;
            }
            else
                liquidState[organism] = acSignal;  // no jump when the music returns
            // An impact already captures ecological energy at excitation.
            // Event bodies retain a bounded audibility floor so a later sleep
            // decision does not erase their physically decaying memory.
            // the ecological music articulates its notes: the floor that keeps
            // an event body ringing after a sleep decision yields to the note
            // envelope, so a resonance does not outlast its note
            const auto bodyFloor = 0.28f * (1.0f - liquid);
            const auto audibilityEnergy = hasEventBody(organismState.species)
                ? autonomousEnergy.enableAmount()
                    * (liquid > 0.0f ? bodyFloor + autonomousState.energy * (1.0f - bodyFloor)
                                     : 0.28f + autonomousState.energy * 0.72f)
                : autonomousState.energy;
            auto neutralSignal = heardSignal * audibilityEnergy
                * (0.10f + audioDensity * 0.11f);
            // the synthesis device: this organism's voice, played by its
            // notes, heard through the same water; skipped at SYNTHESIS 0
            float synthSample = 0.0f;
            if (synthMix > 0.0f)
            {
                const auto& noteVoice = autonomousEnergy.voice(organism);
                const auto held = liquid > 0.0f ? autonomousEnergy.noteHeld(organism) : noteVoice.energy > 0.05f;
                auto& synth = synthVoices[organism];
                if (updateSpatialTargets)
                {
                    // the ecosystem's signals for the modulation matrix, 0..1
                    const auto& music = autonomousEnergy.ecologicalMusicDirector().telemetry();
                    const auto& body = ecosystem.organism(organism);
                    SynthVoice::Ecology eco;
                    eco.energy = std::clamp(noteVoice.energy * 1.9f, 0.0f, 1.0f);  // measured: it peaks near 0.53
                    const auto v = body.velocity;
                    eco.speed = std::clamp(std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z) / 1.2f, 0.0f, 1.0f);
                    float bond = 0.0f;
                    for (std::size_t other = 0; other < Ecosystem::organismCount; ++other)
                        if (other != organism)
                            bond = std::max(bond, music.bond[organism][other]);
                    eco.relation = std::clamp(bond, 0.0f, 1.0f);
                    eco.tension = std::clamp(music.tension, 0.0f, 1.0f);
                    eco.distance = std::clamp(ecosystem.waterPathToListener(organism) / 8.0f, 0.0f, 1.0f);  // the path is 0 .. two radii (8)
                    eco.channel = SoundChannel::trapping(body.position.y, music.channelAxis);
                    eco.temperature = std::clamp(music.temperatureC / 30.0f, 0.0f, 1.0f);
                    eco.hunger = std::clamp(1.0f - music.energy[organism] * 3.0f, 0.0f, 1.0f);
                    eco.age = std::clamp(music.age[organism], 0.0f, 1.0f);
                    eco.prey = std::clamp(music.prey / 2.0f, 0.0f, 1.0f);
                    eco.habitat = music.habitat.measured ? music.habitat.health : 0.5f;
                    eco.room = music.room.strength;
                    synth.setEcology(eco);
                }
                if (held && noteVoice.lifeEvents != synthEvents[organism])
                {
                    synthEvents[organism] = noteVoice.lifeEvents;
                    synth.noteOn(noteVoice.geometricAccent);
                }
                else if (!held && synth.gate())
                    synth.noteOff();
                synthSample = synth.render(noteVoice.frequency, acSignal) * synthLevel;
                if (liquid > 0.0f)
                {
                    synthLiquid[organism] += (synthSample - synthLiquid[organism]) * liquidCoefficient[organism];
                    synthSample += (synthLiquid[organism] - synthSample) * liquid;
                }
                neutralSignal += (synthSample * (0.10f + audioDensity * 0.11f) - neutralSignal) * synthMix;
            }
            const auto organismPan = organismPanPosition(organism) * 0.52f
                + organismDrift[organism] * 0.12f;
            const auto pan = std::clamp(organismPan, -1.0f, 1.0f);
            const auto neutralLeft = neutralSignal * (1.0f - pan) * 0.5f;
            const auto neutralRight = neutralSignal * (1.0f + pan) * 0.5f;

            auto activeSignal = heardSignal * audibilityEnergy
                * (0.16f + audioDensity * 0.16f);
            if (synthMix > 0.0f)
                activeSignal += (synthSample * (0.16f + audioDensity * 0.16f) - activeSignal) * synthMix;
            const auto directLeftGain = ecologicalMix.left * 1.41421356f;
            const auto directRightGain = ecologicalMix.right * 1.41421356f;
            if (updateSpatialTargets)
            {
                autonomousVoice.spatializer[organism].setGainPair(
                    directLeftGain, directRightGain, spatialDepth(ecologicalMix),
                    std::clamp(1.0f - crackStress * 0.72f, 0.28f, 1.0f));
                if (binauralMix > 0.0f || binauralTarget > 0.0f)
                {
                    const auto direction = ecosystem.directionFromListener(organism);
                    binaural[organism].setDirection(direction.azimuth, direction.elevation);
                    midiBinaural[organism].setDirection(direction.azimuth, direction.elevation);
                }
                if (ambisonicActive)
                {
                    const auto direction = ecosystem.directionFromListener(organism);
                    ambisonicPanners[organism].setDirection(direction.azimuth, direction.elevation);
                }
            }
            const auto spatial = autonomousVoice.spatializer[organism].tick(activeSignal);
            auto activeLeft = spatializerEnabled
                ? spatial.left : activeSignal * directLeftGain;
            auto activeRight = spatializerEnabled
                ? spatial.right : activeSignal * directRightGain;
            if (ambisonicActive)
                ambisonicOrganisms += ambisonicPanners[organism].tick(
                    morphValue(neutralLeft + neutralRight, activeLeft + activeRight, ecosystemBlend)
                    * 0.70710678f);
            // headphones: the same signal and level, from the organism's
            // direction through a head; crossfaded with the stereo mix
            if (binauralMix > 0.0f)
            {
                const auto mono = (activeLeft + activeRight) * binauralLevel;
                const auto heard = binaural[organism].tick(mono);
                activeLeft += (heard.left - activeLeft) * binauralMix;
                activeRight += (heard.right - activeRight) * binauralMix;
            }
            const auto addedLeft = morphValue(neutralLeft, activeLeft, ecosystemBlend);
            const auto addedRight = morphValue(neutralRight, activeRight, ecosystemBlend);
            l += addedLeft;
            r += addedRight;
            organismsAddedLeft += addedLeft;
            organismsAddedRight += addedRight;
        }
        const auto desire = ecosystem.desireCurrent();
        const auto environment = ecosystem.environmentState();
        const auto integrity = ecosystem.integrityState();
        const auto lens = ecosystem.lensState();
        const auto& primaryOrganism = ecosystem.organism(0u);
        const auto& granularOrganism = ecosystem.organism(1u);
        const auto composition = autonomousEnergy.compositionPlan();
        // the stems: the organisms' sum, before the module bank
        const auto organismsLeft = l, organismsRight = r;
        const auto bank = moduleBank->process(l, r, {
            desire.density, desire.danger, environment.dayPhase,
            environment.seasonPhase, integrity.crackStress,
            autonomousEnergy.voice(0).frequency,
            granularOrganism.strength, granularOrganism.agility,
            granularOrganism.intelligence, finalOutputFeedbackSample,
            environment.temperature, primaryOrganism.turbidity,
            primaryOrganism.position.y / ecosystem.aquariumRadius(),
            lens.surfacePosition.x, lens.surfacePosition.y,
            composition.influence, composition.pulseWeight,
            composition.tonalWeight, composition.textureWeight,
            composition.movementWeight, lens.heldByObserver >= 0
        });
        l += bank.left;
        r += bank.right;
        const auto filteredLeft = schoolSpectralLeft.tick(l).mix;
        const auto filteredRight = schoolSpectralRight.tick(r).mix;
        const auto ambience = ecologicalNoise.tick();
        if (currentSpectralEnvironment.influence > 0.0f)
        {
            const auto filterAmount = currentSpectralEnvironment.influence
                * currentSpectralEnvironment.filterMix;
            l = morphValue(l, filteredLeft, filterAmount);
            r = morphValue(r, filteredRight, filterAmount);
            const auto ambientAmount = currentSpectralEnvironment.influence
                * currentSpectralEnvironment.ambience;
            l += ambience.left * ambientAmount;
            r += ambience.right * ambientAmount;
        }
        // the water itself, bubbling, in the space (BubbleField); skipped
        // entirely without the ecological music: bit-identical
        if (bubblesActive)
        {
            const auto bubble = bubbleField.render();
            l += bubble.left * bubbleLevel;
            r += bubble.right * bubbleLevel;
        }
        const auto waterLeft = l - organismsLeft - bank.left, waterRight = r - organismsRight - bank.right;
        const auto sharedReflection = multiTapSpace->tick(l, r);
        // +6 dB on the reflections, outside the feedback loop: the space was
        // ~16 dB under the dry mix even at full RETURN, so SIZE/SPREAD/TONE/
        // MEMORY were barely audible (2026-10-20 review)
        constexpr float spaceWetMakeup = 2.0f;
        const auto spaceReturn = advanceMultiTapSpaceReturn() * spaceWetMakeup;
        const auto earlyLeft = multiTapSpaceEnabled
            ? l + sharedReflection.left * spaceReturn : l;
        const auto earlyRight = multiTapSpaceEnabled
            ? r + sharedReflection.right * spaceReturn : r;
        const auto tail = syntheticTail->tick(earlyLeft, earlyRight);
        const auto outputLeft = earlyLeft
            + tail.left * currentSyntheticTail.influence;
        const auto outputRight = earlyRight
            + tail.right * currentSyntheticTail.influence;
        const auto creativeOutput = masterChain->process(
            outputLeft * outputCalibrationGain,
            outputRight * outputCalibrationGain);
        const auto safeOutput = outputStage->process(
            creativeOutput.left, creativeOutput.right);
        if (stems.capacity > sample && stems.left[0] != nullptr)
        {
            const auto g = outputCalibrationGain;
            const std::array<float, stemCount> partsLeft {
                organismsLeft * g, bank.left * g, waterLeft * g,
                (earlyLeft - l) * g, (outputLeft - earlyLeft) * g };
            const std::array<float, stemCount> partsRight {
                organismsRight * g, bank.right * g, waterRight * g,
                (earlyRight - r) * g, (outputRight - earlyRight) * g };
            for (std::size_t stem = 0; stem < stemCount; ++stem)
                if (stems.left[stem] != nullptr && stems.right[stem] != nullptr)
                {
                    stems.left[stem][sample] = partsLeft[stem];
                    stems.right[stem][sample] = partsRight[stem];
                }
        }
        if (ambisonicActive)
        {
            // the rest of the mix (all but the organisms' own contributions)
            // as the stereo pair; the organisms from their directions
            const auto g = outputCalibrationGain;
            auto field = ambisonicOrganisms;
            field += FoaPanner::encodeStereo(outputLeft - organismsAddedLeft, outputRight - organismsAddedRight);
            ambisonics.channel[0][sample] = field.w * g;
            ambisonics.channel[1][sample] = field.y * g;
            ambisonics.channel[2][sample] = field.z * g;
            ambisonics.channel[3][sample] = field.x * g;
        }
        if (recordTapLeft != nullptr && recordTapRight != nullptr && sample < recordTapCapacity)
        {
            const auto pre = recordTap == RecordTap::preMasterCreative;
            recordTapLeft[sample] = pre ? outputLeft * outputCalibrationGain : creativeOutput.left;
            recordTapRight[sample] = pre ? outputRight * outputCalibrationGain : creativeOutput.right;
        }
        left[sample] = safeOutput.left;
        right[sample] = safeOutput.right;
        // the habitat listens to itself only while the ecological music plays.
        // It and the module bank's feedback read the creative MASTER's output,
        // before the trim and the safety limiter (SAIDA_AUDIO_COMUM §4): read
        // after them, the OUTPUT TRIM - the volume - changed the music itself
        // (2026-10-22: 0 vs -18 dB diverged within 66 ms)
        if (autonomousEnergy.ecologicalMusicDirector().performance().influence > 0.0f)
            habitat->push((creativeOutput.left + creativeOutput.right) * 0.5f);
        // the room: the microphone against what the aquarium plays
        if (roomEnabled && roomInput != nullptr && sample < roomInputCount)
            room.push(roomInput[sample], (safeOutput.left + safeOutput.right) * 0.5f);
        finalOutputFeedbackSample = std::clamp(
            (creativeOutput.left + creativeOutput.right) * 0.5f, -1.0f, 1.0f);
        // the published taps: envelopes, bounded
        {
            const auto follow = [this](float& tap, float value) {
                const auto x = std::isfinite(value) ? std::min(value, 1.0f) : 0.0f;
                tap += (x - tap) * (x > tap ? tapAttack : tapRelease);
            };
            const auto g = outputCalibrationGain;
            follow(taps.preMaster, std::max(std::abs(outputLeft * g), std::abs(outputRight * g)));
            follow(taps.preSafety, std::max(std::abs(creativeOutput.left), std::abs(creativeOutput.right)));
            const auto limiter = outputStage->limiterGainNow();
            follow(taps.safetyReduction, limiter < 1.0f ? -20.0f * std::log10(std::max(limiter, 1.0e-6f)) / 12.0f : 0.0f);
            follow(taps.postSafety, std::max(std::abs(safeOutput.left), std::abs(safeOutput.right)));
        }
        // Homeostasis observes the authored internal balance. Output
        // calibration is an acoustic gain stage and must not rewrite ecology.
        lastOutputEnergy = std::sqrt((l * l + r * r) * 0.5f) * 4.0f;
        ecosystem.setOutputEnergy(outputFeedbackEnabled ? lastOutputEnergy : 0.0f);
    }
    roomInput = nullptr;  // valid for this block only
    roomInputCount = 0;
    recordTapLeft = recordTapRight = nullptr;  // the taps too
    recordTapCapacity = 0;
    stems = {};
    ambisonics = {};
}

void BiomaEngine::setSupportPresence(float gain) noexcept
{
    supportTarget = std::isfinite(gain) ? std::clamp(gain, 0.0f, 16.0f) : 1.0f;
}

void BiomaEngine::setSynthMix(float amount) noexcept
{
    synthMixTarget = std::isfinite(amount) ? std::clamp(amount, 0.0f, 1.0f) : 0.0f;
}

void BiomaEngine::setSynthPatch(std::size_t organism, const SynthVoice::Patch& patch) noexcept
{
    if (organism < synthVoices.size())
        synthVoices[organism].setPatch(patch);
}

const SynthVoice::Patch& BiomaEngine::synthPatch(std::size_t organism) const noexcept
{
    return synthVoices[std::min(organism, synthVoices.size() - 1)].currentPatch();
}

void BiomaEngine::setRoomCouplingEnabled(bool enabled) noexcept
{
    if (enabled == roomEnabled)
        return;
    roomEnabled = enabled;
    room.reset();
}
}
