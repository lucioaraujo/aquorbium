// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/OpenInteractionModules.h"

#include <array>
#include <bit>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>

namespace
{
bool doubleScrollIsDeterministicBoundedAndBistable()
{
    aquorbium::DoubleScrollAttractor first, second;
    for (auto* attractor : { &first, &second })
    {
        attractor->prepare(48000.0);
        attractor->setSeed(0x71a92u);
        attractor->setParameters({ 42.0f, 0.58f, 0.62f, 0.74f });
    }
    auto positive = false;
    auto negative = false;
    auto movement = 0.0;
    auto previous = first.tick();
    static_cast<void>(second.tick());
    for (int sample = 1; sample < 48000 * 4; ++sample)
    {
        const auto a = first.tick();
        const auto b = second.tick();
        if (std::bit_cast<std::uint32_t>(a.x)
                != std::bit_cast<std::uint32_t>(b.x)
            || std::bit_cast<std::uint32_t>(a.y)
                != std::bit_cast<std::uint32_t>(b.y)
            || !std::isfinite(a.x) || !std::isfinite(a.y)
            || !std::isfinite(a.z) || std::abs(a.x) > 1.0f
            || std::abs(a.y) > 1.0f || std::abs(a.z) > 1.0f
            || a.xComplement != -a.x || a.yComplement != -a.y
            || a.zComplement != -a.z)
            return false;
        movement += std::abs(static_cast<double>(a.x - previous.x));
        positive = positive || a.positiveEquilibrium;
        negative = negative || !a.positiveEquilibrium;
        previous = a;
    }
    return movement > 1.0 && positive && negative;
}

bool semanticRouterConvertsDomainsAndBoundsSums()
{
    using Router = aquorbium::SemanticAccessRouter;
    Router router;
    router.prepare(48000.0);
    if (!router.setRoute(0, { 0, 2, Router::SourceKind::audio,
                              Router::DestinationKind::audio,
                              0.75f, 0.0f, true })
        || !router.setRoute(1, { 1, 2, Router::SourceKind::control,
                                 Router::DestinationKind::modulation,
                                 0.5f, 0.25f, true })
        || !router.setRoute(2, { 3, 4, Router::SourceKind::event,
                                 Router::DestinationKind::event,
                                 1.0f, 0.0f, true })
        || router.setRoute(Router::routeCount, {}))
        return false;
    Router::Inputs input {};
    input[0].audio = 0.8f;
    input[1].control = -0.5f;
    input[3].event = true;
    const auto output = router.process(input, 1.0f);
    return std::abs(output[2].audio - 0.6f) < 1.0e-6f
        && std::abs(output[2].modulation + 0.375f) < 1.0e-6f
        && output[4].event;
}

bool openRulesCompileTransactionallyAndEvaluate()
{
    aquorbium::OpenRuleProgram program;
    if (!program.compile(
            "danger>0.6:evolution=0.9; day<0.2:gesture=0.75;"
            "crack>0.5:integrity=1; chaos>0.7:branch=0.8")
        || program.size() != 4u)
        return false;
    const auto result = program.evaluate({ 0.4f, 0.8f, 0.1f,
                                            0.5f, 0.7f, 0.75f });
    if (result.matches != 4u || !result.active[0] || !result.active[1]
        || !result.active[2] || !result.active[3]
        || std::abs(result.value[0] - 0.8f) > 1.0e-6f
        || std::abs(result.value[1] - 0.9f) > 1.0e-6f)
        return false;
    if (program.compile("danger?:branch=1") || program.size() != 4u)
        return false;
    const auto sanitized = program.evaluate({
        std::numeric_limits<float>::quiet_NaN(),
        std::numeric_limits<float>::infinity(), 0.1f, 0.5f, 0.7f, 0.75f
    });
    return sanitized.matches == 3u;
}

bool crackleIsDeterministicBoundedAndHasExactZeroAmount()
{
    aquorbium::ContactCrackleExciter first, second, bypassed;
    for (auto* exciter : { &first, &second, &bypassed })
    {
        exciter->prepare(48000.0);
        exciter->setSeed(0x91ab3u);
    }
    first.setParameters({ 0.8f, 0.7f, 0.6f, 0.4f });
    second.setParameters({ 0.8f, 0.7f, 0.6f, 0.4f });
    bypassed.setParameters({ 0.0f, 0.7f, 0.6f, 0.4f });
    const aquorbium::ContactCrackleExciter::Contact contact {
        -0.6f, 0.7f, 0.9f, 0.8f, 0x12345678u
    };
    if (!first.trigger(contact) || !second.trigger(contact)
        || !bypassed.trigger(contact)) return false;
    auto energy = 0.0;
    for (int sample = 0; sample < 48000; ++sample)
    {
        const auto a = first.tick();
        const auto b = second.tick();
        const auto dry = bypassed.tick();
        if (std::bit_cast<std::uint32_t>(a.left)
                != std::bit_cast<std::uint32_t>(b.left)
            || std::bit_cast<std::uint32_t>(a.right)
                != std::bit_cast<std::uint32_t>(b.right)
            || !std::isfinite(a.left) || !std::isfinite(a.right)
            || std::abs(a.left) > 1.0f || std::abs(a.right) > 1.0f
            || dry.left != 0.0f || dry.right != 0.0f)
            return false;
        energy += static_cast<double>(a.left) * a.left
            + static_cast<double>(a.right) * a.right;
    }
    return energy > 0.01 && first.energy() < 0.01f;
}
}

int main()
{
    struct Case { const char* name; bool (*run)(); };
    const std::array cases {
        Case { "double scroll", doubleScrollIsDeterministicBoundedAndBistable },
        Case { "semantic access", semanticRouterConvertsDomainsAndBoundsSums },
        Case { "open rules", openRulesCompileTransactionallyAndEvaluate },
        Case { "contact crackle", crackleIsDeterministicBoundedAndHasExactZeroAmount }
    };
    for (const auto& test : cases)
        if (!test.run())
        {
            std::cerr << "Failed: " << test.name << '\n';
            return EXIT_FAILURE;
        }
    std::cout << "Open interaction cohort tests passed\n";
    return EXIT_SUCCESS;
}
