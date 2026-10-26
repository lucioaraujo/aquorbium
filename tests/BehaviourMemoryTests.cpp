// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/BehaviourMemory.h"

#include <array>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <utility>

namespace
{
bool historyCapturesAndLoopsWindow()
{
    aquorbium::DriftHistoryLoop history;
    for (int value = 0; value < 8; ++value)
        (void)history.tick(static_cast<float>(value) / 7.0f);
    history.setWindow(1.0f, 0.5f);
    if (!history.beginLoop() || history.loopLength() != 5)
        return false;
    const std::array<float, 5> expected {
        3.0f / 7.0f, 4.0f / 7.0f, 5.0f / 7.0f, 6.0f / 7.0f, 1.0f
    };
    for (int loop = 0; loop < 3; ++loop)
        for (const auto value : expected)
        {
            const auto output = history.tick(-1.0f);
            if (!output.looping || output.value != value)
                return false;
        }
    history.endLoop();
    return history.tick(-0.25f).value == -0.25f;
}

bool historyWrapsAtFixedCapacity()
{
    aquorbium::DriftHistoryLoop history;
    for (std::size_t index = 0; index < history.capacity + 32; ++index)
        (void)history.tick(static_cast<float>(index % 101) / 100.0f);
    history.setWindow(0.0f, 0.0f);
    return history.captured() == history.capacity && history.beginLoop()
        && history.loopLength() == 2;
}

bool movementSignatureCanResistChange()
{
    aquorbium::MovementSignatureLock signature;
    signature.reset({ 0.2f, 0.4f, 0.6f });
    signature.setLocked(true);
    signature.update({ 0.8f, 0.9f, 0.1f });
    const auto held = signature.signature();
    if (held.smoothing != 0.2f || held.curveFamily != 0.4f
        || held.eventProbability != 0.6f)
        return false;
    signature.setLocked(false);
    signature.update({ 0.8f, 0.9f, 0.1f });
    const auto changed = signature.signature();
    return changed.smoothing == 0.8f && changed.curveFamily == 0.9f
        && changed.eventProbability == 0.1f;
}

bool presenceWaitsForRelevantEvent()
{
    using Latch = aquorbium::PresenceStateLatch;
    using Presence = aquorbium::Ecosystem::Presence;
    Latch latch;
    latch.reset(Presence::Present);
    latch.setRelevantEvents(static_cast<std::uint8_t>(Latch::Event::danger)
                            | static_cast<std::uint8_t>(Latch::Event::integrity));
    latch.request(Presence::Leaving);
    if (latch.trigger(Latch::Event::mating) || latch.current() != Presence::Present
        || !latch.hasPending())
        return false;
    return latch.trigger(Latch::Event::danger)
        && latch.current() == Presence::Leaving && !latch.hasPending();
}

bool chokeChoosesLowestProminentChannel()
{
    aquorbium::PriorityChoke choke;
    choke.setThreshold(0.6f);
    const std::array<float, 8> prominence {
        0.2f, 0.75f, 0.9f, 0.1f, 1.0f, 0.0f, 0.0f, 0.0f
    };
    const auto output = choke.process(prominence, 5);
    if (output.selected != 1 || output.gain[1] != 1.0f)
        return false;
    for (std::size_t index = 0; index < output.gain.size(); ++index)
        if (index != 1 && output.gain[index] != 0.0f)
            return false;
    return choke.process({}, 8).selected == -1;
}

bool invalidValuesAreSanitized()
{
    aquorbium::DriftHistoryLoop history;
    const auto live = history.tick(std::numeric_limits<float>::quiet_NaN());
    history.setWindow(std::numeric_limits<float>::infinity(), -3.0f);

    aquorbium::MovementSignatureLock signature;
    signature.reset({ 0.2f, 0.3f, 0.4f });
    signature.update({ std::numeric_limits<float>::quiet_NaN(), 4.0f, -2.0f });
    const auto values = signature.signature();

    aquorbium::PriorityChoke choke;
    choke.setThreshold(std::numeric_limits<float>::quiet_NaN());
    return live.value == 0.0f && values.smoothing == 0.2f
        && values.curveFamily == 1.0f && values.eventProbability == 0.0f
        && choke.threshold() == 0.5f;
}
}

int main()
{
    const std::pair<const char*, bool (*)()> cases[] {
        { "history loop", historyCapturesAndLoopsWindow },
        { "history capacity", historyWrapsAtFixedCapacity },
        { "signature lock", movementSignatureCanResistChange },
        { "presence latch", presenceWaitsForRelevantEvent },
        { "priority choke", chokeChoosesLowestProminentChannel },
        { "limits", invalidValuesAreSanitized },
    };
    for (const auto& [name, test] : cases)
    {
        if (!test())
        {
            std::cerr << name << " failed\n";
            return EXIT_FAILURE;
        }
    }
    std::cout << "Behaviour memory tests passed\n";
}
