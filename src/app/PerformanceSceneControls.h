// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include "AudioControlFrame.h"
#include "PerformanceParameterState.h"

namespace aquorbium::app
{
// GUI adapter. No ValueTree is retained by the resulting plain audio frame.
[[nodiscard]] inline AudioControlFrame sceneControls(
    const PerformanceParameterState& state, AudioControlFrame transients = {})
{
    using Key = EcologyParameterState::Key;
    EcologyParameterState ecology;
    static_cast<void>(ecology.restore(state.ecologySnapshot()));
    auto frame = transients;
    frame.density = static_cast<float>(ecology.get(Key::density));
    frame.drift = static_cast<float>(1.0 - ecology.get(Key::calm));
    frame.cohesion = static_cast<float>(ecology.get(Key::cohesion));
    frame.danger = static_cast<float>(ecology.get(Key::danger));
    frame.sparseFocus = static_cast<float>(ecology.get(Key::sparseFocus));
    frame.energyMemory = static_cast<float>(ecology.get(Key::energyMemory));
    frame.geometricCycle = static_cast<float>(ecology.get(Key::geometricCycle));
    frame.collectiveDrift = static_cast<float>(ecology.get(Key::collectiveDrift));
    constexpr std::array<float AudioControlFrame::*, 37> members {
        &AudioControlFrame::ecosystemMorph, &AudioControlFrame::geometryInfluence,
        &AudioControlFrame::rhythmDensity, &AudioControlFrame::orbitRotation,
        &AudioControlFrame::intervalBias, &AudioControlFrame::geometryMutation,
        &AudioControlFrame::strikerInfluence, &AudioControlFrame::strikerMatter,
        &AudioControlFrame::strikerPopulation, &AudioControlFrame::strikerScatter,
        &AudioControlFrame::spaceReturn, &AudioControlFrame::spaceGesture,
        &AudioControlFrame::spaceSize, &AudioControlFrame::spaceSpread,
        &AudioControlFrame::spaceTone, &AudioControlFrame::spaceMemory,
        &AudioControlFrame::environmentInfluence,
        &AudioControlFrame::environmentFilterMix,
        &AudioControlFrame::environmentCentre,
        &AudioControlFrame::environmentSpan,
        &AudioControlFrame::environmentResonance,
        &AudioControlFrame::environmentAmbience,
        &AudioControlFrame::tailInfluence,
        &AudioControlFrame::tailDecay,
        &AudioControlFrame::tailDamping,
        &AudioControlFrame::tailNightmare,
        &AudioControlFrame::formInfluence,
        &AudioControlFrame::formContext,
        &AudioControlFrame::formInertia,
        &AudioControlFrame::formMutation,
        &AudioControlFrame::masterTrim,
        &AudioControlFrame::masterCeiling,
        &AudioControlFrame::masterRelease,
        &AudioControlFrame::structureInfluence,
        &AudioControlFrame::structureMemory,
        &AudioControlFrame::structureHarmony,
        &AudioControlFrame::structurePhase
    };
    constexpr std::array<float AudioControlFrame::*, 4> music {
        &AudioControlFrame::musicInfluence, &AudioControlFrame::musicReaction,
        &AudioControlFrame::musicMemory, &AudioControlFrame::musicWater
    };
    static_assert(members.size() + IntegratedModuleBank::controlCount + 1 + music.size() + 16 + 5 + 1 + 1 + 1 + 12  // + SUPPORT, + EQ order, + the four cards
                  == PerformanceParameterState::parameters.size());
    for (std::size_t index = 0; index < members.size(); ++index)
        frame.*members[index] = static_cast<float>(state.get(PerformanceParameterState::parameters[index].id));
    for (std::size_t index = 0; index < frame.moduleBank.size(); ++index)
        frame.moduleBank[index] = static_cast<float>(state.get(
            PerformanceParameterState::parameters[members.size() + index].id));
    frame.creativeMaster = static_cast<float>(state.get(
        PerformanceParameterState::parameters[
            members.size() + IntegratedModuleBank::controlCount].id));
    for (std::size_t index = 0; index < music.size(); ++index)
        frame.*music[index] = static_cast<float>(state.get(PerformanceParameterState::parameters[
            PerformanceParameterState::preEcologicalMusicParameterCount + index].id));
    constexpr auto firstTimbre = PerformanceParameterState::preTimbreParameterCount;
    for (std::size_t i = 0; i < 5; ++i)
    {
        frame.timbreBody[i] = static_cast<float>(state.get(PerformanceParameterState::parameters[
            PerformanceParameterState::preTimbreKindParameterCount + i].id));
        frame.timbreMatter[i] = static_cast<float>(state.get(PerformanceParameterState::parameters[firstTimbre + 5 + i].id));
        frame.timbreMix[i] = static_cast<float>(state.get(PerformanceParameterState::parameters[firstTimbre + 10 + i].id));
    }
    frame.timbreEcology = static_cast<float>(state.get(PerformanceParameterState::parameters[firstTimbre + 15].id));
    frame.musicSalinity = static_cast<float>(state.get(
        PerformanceParameterState::parameters[PerformanceParameterState::preSalinityParameterCount].id));
    frame.supportPresence = static_cast<float>(state.get(
        PerformanceParameterState::parameters[PerformanceParameterState::preSupportParameterCount].id));
    frame.masterEqAfterDynamics = state.get(
        PerformanceParameterState::parameters[PerformanceParameterState::preEqOrderParameterCount].id) >= 0.5;
    for (std::size_t i = 0; i < frame.cards.size(); ++i)
        frame.cards[i] = static_cast<float>(state.get(
            PerformanceParameterState::parameters[PerformanceParameterState::preCardsParameterCount + i].id));
    frame.autonomous = state.autonomous();
    return frame;
}
}
