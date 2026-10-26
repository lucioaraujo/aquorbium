// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include "core/BiomaEngine.h"
#include "core/GranularPerformance.h"

#include <algorithm>
#include <cmath>

namespace aquorbium::app
{
struct AudioControlFrame
{
    float density = 0.45f;
    float drift = 0.20f;
    float cohesion = 0.50f;
    float danger = 0.50f;
    float ecosystemMorph = 1.0f;
    float sparseFocus = 0.25f;
    float energyMemory = 0.25f;
    float geometricCycle = 0.25f;
    float collectiveDrift = 0.25f;
    float geometryInfluence = 0.0f;
    float rhythmDensity = 0.50f;
    float orbitRotation = 0.50f;
    float intervalBias = 0.50f;
    float geometryMutation = 0.0f;
    float strikerInfluence = 0.0f;
    float strikerMatter = 0.50f;
    float strikerPopulation = 0.50f;
    float strikerScatter = 0.50f;
    float spaceReturn = 0.48f;
    float spaceGesture = 0.0f;
    float spaceSize = 0.50f;
    float spaceSpread = 1.0f;
    float spaceTone = 0.50f;
    float spaceMemory = 0.35f;
    float environmentInfluence = 0.0f;
    float environmentFilterMix = 0.5f;
    float environmentCentre = 0.5f;
    float environmentSpan = 0.5f;
    float environmentResonance = 0.45f;
    float environmentAmbience = 0.0f;
    float tailInfluence = 0.0f;
    float tailDecay = 0.45f;
    float tailDamping = 0.55f;
    float tailNightmare = 0.0f;
    float formInfluence = 0.0f;
    float formContext = 0.5f;
    float formInertia = 0.35f;
    float formMutation = 0.05f;
    float masterTrim = 0.75f;
    float masterCeiling = 0.8474576f;
    float masterRelease = 0.2222222f;
    float creativeMaster = 0.0f;
    float structureInfluence = 0.0f;
    float structureMemory = 0.5f;
    float structureHarmony = 0.5f;
    float structurePhase = 0.5f;
    float musicInfluence = 0.0f;
    float musicReaction = 0.6f;
    float musicMemory = 0.5f;
    float musicWater = 0.5f;
    float musicSalinity = 1.0f;
    std::array<float, 5> timbreBody {};   // body index / TimbreBody::kindScale
    std::array<float, 5> timbreMatter { 0.5f, 0.5f, 0.5f, 0.5f, 0.5f };
    std::array<float, 5> timbreMix {};
    float timbreEcology = 0.0f;
    // SUPPORT (2026-10-23): the support species' level, -6..+24 dB; 0.2 = 0 dB
    float supportPresence = 0.2f;
    // the creative MASTER's EQ after its dynamics (2026-10-23)
    bool masterEqAfterDynamics = false;
    // the LUPA, MASTER CRIATIVO, CÉREBRO and SALA cards (2026-10-24), in the
    // order of PerformanceParameterState: radius, reach, magnification,
    // blur, multiband, glue, tilt, width, clock, coordination, sensitivity,
    // reaction; 0.5 is classic (coordination 0)
    std::array<float, 12> cards { 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.0f, 0.5f, 0.5f };
    IntegratedModuleBank::Controls moduleBank {};
    float lensX = 1.0f;
    float lensY = 0.0f;
    float lensZ = 0.0f;
    bool lensHeld = false;
    bool autonomous = true;

    // Called only by the audio owner. Engine setters keep their own ramps.
    void apply(BiomaEngine& engine, bool immediateMorph = false) const noexcept
    {
        engine.setDensity(density);
        engine.setDrift(drift);
        engine.setCohesion(cohesion);
        engine.setDanger(danger);
        engine.setListeningBlend({ sparseFocus, energyMemory, geometricCycle, collectiveDrift });
        engine.setAutonomousEnabled(autonomous);
        if (immediateMorph)
            engine.setEcosystemBlendImmediately(ecosystemMorph);
        else
            engine.setEcosystemBlend(ecosystemMorph);
        engine.setLensGesture(lensHeld, { lensX, lensY, lensZ });
        engine.setGeometryPerformance({ geometryInfluence,
            rhythmDensity * 2.0f - 1.0f, orbitRotation * 2.0f - 1.0f,
            intervalBias * 2.0f - 1.0f, geometryMutation });
        engine.setFormMemoryPerformance({
            formInfluence, formContext, formInertia, formMutation
        });
        engine.setGranularStrikerPerformance(GranularPerformance::map(
            { strikerInfluence, strikerMatter, strikerPopulation, strikerScatter }));
        // SPACE MIX must change the wet presence as well as reflection shape.
        // The engine clamps the combined return to its established 0.75 cap.
        engine.setMultiTapSpaceReturn(spaceReturn * 0.75f * (1.0f + spaceGesture));
        engine.setMultiTapSpacePerformance(
            { spaceGesture, spaceSize, spaceSpread, spaceTone, spaceMemory });
        engine.setSpectralEnvironmentPerformance({
            environmentInfluence, environmentFilterMix, environmentCentre,
            environmentSpan, environmentResonance, environmentAmbience
        });
        engine.setSyntheticTailPerformance({
            tailInfluence, tailDecay, tailDamping, tailNightmare
        });
        engine.setOutputStageParameters({
            -18.0f + masterTrim * 24.0f,
            -6.0f + masterCeiling * 5.9f,
            50.0f + masterRelease * 450.0f,
            false
        });
        engine.setCreativeMasterInfluence(creativeMaster);
        engine.setCreativeMasterEqAfterDynamics(masterEqAfterDynamics);
        // each card's centre multiplies by 2^0 = 1 exactly: the classic value
        const auto c = [this](std::size_t i) { return std::clamp(cards[i], 0.0f, 1.0f); };
        engine.setLensShape(0.38f * std::exp2((c(0) - 0.5f) * 2.0f), 0.75f * std::exp2((c(1) - 0.5f) * 1.5f),
                            2.5f * std::exp2((c(2) - 0.5f) * 2.0f), 0.35f * std::exp2((0.5f - c(3)) * 2.0f));
        engine.setCreativeMasterShape({ c(4) - 0.5f, c(5) - 0.5f, (c(6) - 0.5f) * 2.0f, (c(7) - 0.5f) * 1.5f });
        engine.setBrainControls(std::exp2((c(8) - 0.5f) * 4.0f), c(9));
        engine.setRoomControls((c(10) - 0.5f) * 24.0f, std::exp2((c(11) - 0.5f) * 2.0f));
        engine.setStructureMemoryPerformance({
            structureInfluence, structureMemory, structureHarmony, structurePhase
        });
        engine.setEcologicalMusicPerformance({
            musicInfluence, musicReaction, musicMemory, musicWater, musicSalinity
        });
        BiomaEngine::TimbrePerformance timbre;
        for (std::size_t i = 0; i < timbre.body.size(); ++i)
        {
            const auto index = std::clamp(static_cast<int>(std::lround(timbreBody[i] * TimbreBody::kindScale)),
                                          0, static_cast<int>(TimbreBody::kindCount) - 1);
            timbre.body[i] = static_cast<TimbreBody::Kind>(index);
            timbre.matter[i] = timbreMatter[i];
            timbre.mix[i] = timbreMix[i];
        }
        timbre.ecology = timbreEcology;
        engine.setTimbrePerformance(timbre);
        // on top of the authored species gains, never instead of them
        engine.setSupportPresence(std::pow(10.0f, (-6.0f + 30.0f * std::clamp(supportPresence, 0.0f, 1.0f)) / 20.0f));
        engine.setIntegratedModuleBankControls(moduleBank);
    }
};
}
