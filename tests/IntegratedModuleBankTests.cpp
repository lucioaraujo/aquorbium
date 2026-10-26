// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/IntegratedModuleBank.h"

#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstdlib>
#include <iostream>

namespace
{
using Bank = aquorbium::IntegratedModuleBank;

float renderEnergy(Bank::Controls controls, std::uint32_t seed)
{
    Bank bank;
    bank.setControls(controls);
    bank.prepare(48000.0);
    bank.setSeed(seed);
    auto sum = 0.0;
    for (int sample = 0; sample < 48000; ++sample)
    {
        const auto phase = static_cast<float>(sample % 48000) / 48000.0f;
        const auto input = std::sin(phase * 6.28318530717958647692f * 97.0f) * 0.08f;
        const auto output = bank.process(input, input * 0.5f, {
            0.62f, 0.48f, phase,
            std::fmod(phase * 0.17f, 1.0f), 0.35f, 82.41f
        });
        if (!std::isfinite(output.left) || !std::isfinite(output.right)
            || std::abs(output.left) > 0.65001f
            || std::abs(output.right) > 0.65001f)
            return -1.0f;
        sum += static_cast<double>(output.left) * output.left
            + static_cast<double>(output.right) * output.right;
    }
    return static_cast<float>(sum / 96000.0);
}

bool bypassIsExact()
{
    Bank::Controls controls {};
    for (std::size_t index = 1; index < controls.size(); ++index)
        controls[index] = 1.0f;
    return renderEnergy(controls, 77u) == 0.0f;
}

bool everyFamilyHasBoundedOutput()
{
    for (std::size_t family = 1; family < Bank::controlCount; ++family)
    {
        Bank::Controls controls {};
        controls[Bank::mix] = 1.0f;
        controls[family] = 1.0f;
        const auto energy = renderEnergy(controls, 811u);
        if (!(energy > 1.0e-10f))
        {
            std::cerr << "family " << family << " energy=" << energy << '\n';
            return false;
        }
    }
    return true;
}

bool deterministicForSameSeed()
{
    Bank first, second;
    Bank::Controls controls {};
    controls.fill(0.72f);
    controls[Bank::mix] = 0.8f;
    for (auto* bank : { &first, &second })
    {
        bank->setControls(controls);
        bank->prepare(48000.0);
        bank->setSeed(9137u);
    }
    for (int sample = 0; sample < 24000; ++sample)
    {
        const auto phase = static_cast<float>(sample) / 24000.0f;
        const auto a = first.process(0.05f, -0.03f,
                                     { 0.6f, 0.4f, phase, phase * 0.5f,
                                       0.3f, 98.0f });
        const auto b = second.process(0.05f, -0.03f,
                                      { 0.6f, 0.4f, phase, phase * 0.5f,
                                        0.3f, 98.0f });
        if (std::bit_cast<std::uint32_t>(a.left)
                != std::bit_cast<std::uint32_t>(b.left)
            || std::bit_cast<std::uint32_t>(a.right)
                != std::bit_cast<std::uint32_t>(b.right))
            return false;
    }
    const auto telemetry = first.telemetry();
    return telemetry.events > 0 && telemetry.activeFamilies == 15
        && telemetry.outputEnergy > 0.0f;
}

bool ecologicalIntegrationIsActive()
{
    Bank bank;
    Bank::Controls controls {};
    controls[Bank::mix] = 1.0f;
    controls[Bank::synthBodies] = 0.8f;
    controls[Bank::granularMemory] = 0.75f;
    controls[Bank::evolutionStages] = 0.7f;
    bank.setControls(controls);
    bank.prepare(48000.0);
    bank.setSeed(0x91b2u);

    Bank::Context context;
    context.density = 0.1f;
    context.danger = 0.2f;
    context.baseFrequencyHz = 73.42f;
    context.strength = 0.9f;
    context.agility = 0.8f;
    context.intelligence = 0.75f;
    for (int sample = 0; sample < 24000; ++sample)
    {
        context.dayPhase = static_cast<float>(sample) / 24000.0f;
        context.feedbackAudio = 0.72f;
        static_cast<void>(bank.process(0.08f, -0.03f, context));
    }
    auto telemetry = bank.telemetry();
    if (telemetry.events == 0 || telemetry.feedbackModulation == 0.0f
        || telemetry.quantizedSubSemitones > -5.0f
        || telemetry.quantizedSubSemitones < -36.0f
        || telemetry.grainParameters.gain < 0.55f)
        return false;

    context.density = 1.0f;
    static_cast<void>(bank.process(0.08f, -0.03f, context));
    telemetry = bank.telemetry();
    if (!(telemetry.smoothedContext.density > 0.1f
          && telemetry.smoothedContext.density < 0.2f))
        return false;
    for (int sample = 0; sample < 12000; ++sample)
        static_cast<void>(bank.process(0.08f, -0.03f, context));
    return bank.telemetry().smoothedContext.density > 0.95f;
}

bool openInteractionCohortIsObservable()
{
    Bank bank;
    Bank::Controls controls {};
    controls[Bank::mix] = 1.0f;
    controls[Bank::lorenzDrift] = 1.0f;
    controls[Bank::gestureWaves] = 1.0f;
    controls[Bank::branchPermute] = 1.0f;
    controls[Bank::evolutionStages] = 1.0f;
    bank.setControls(controls);
    bank.prepare(48000.0);
    bank.setSeed(0x64c9u);

    auto scrollPeak = 0.0f;
    auto accessPeak = 0.0f;
    auto rulePeak = 0.0f;
    auto contactPeak = 0.0f;
    Bank::Context context;
    context.density = 0.72f;
    context.danger = 0.88f;
    context.crackStress = 0.91f;
    context.agility = 0.76f;
    for (int sample = 0; sample < 12000; ++sample)
    {
        context.dayPhase = static_cast<float>(sample) / 12000.0f;
        const auto impulse = sample >= 8 && sample < 96 ? 0.8f : 0.0f;
        static_cast<void>(bank.process(impulse, impulse, context));
        const auto telemetry = bank.telemetry();
        scrollPeak = std::max(scrollPeak, std::abs(telemetry.doubleScrollX));
        accessPeak = std::max(accessPeak,
                              std::abs(telemetry.accessModulation));
        rulePeak = std::max(rulePeak, telemetry.openRuleInfluence);
        contactPeak = std::max(contactPeak, telemetry.contactEnergy);
    }
    return scrollPeak > 0.001f && accessPeak > 0.001f
        && rulePeak > 0.5f && contactPeak > 0.05f;
}

bool refinementCohortIsObservable()
{
    Bank bank;
    Bank::Controls controls {};
    controls[Bank::mix] = 1.0f;
    controls[Bank::fluidPhase] = 1.0f;
    controls[Bank::gestureWaves] = 1.0f;
    bank.setControls(controls);
    bank.prepare(48000.0);
    bank.setSeed(0x349au);

    Bank::Context context;
    context.verticalPosition = -1.0f;
    context.turbidity = 0.8f;
    context.temperature = 0.45f;
    context.lensHeld = true;
    for (int sample = 0; sample < 1200; ++sample)
    {
        const auto phase = static_cast<float>(sample) / 1200.0f;
        context.lensX = phase * 2.0f - 1.0f;
        context.lensY = std::sin(phase * 6.28318530717958647692f);
        static_cast<void>(bank.process(0.0f, 0.0f, context));
    }
    if (bank.telemetry().mediumWaterMix < 0.95f) return false;

    context.lensHeld = false;
    auto playbackObserved = false;
    for (int sample = 0; sample < 1200; ++sample)
    {
        static_cast<void>(bank.process(0.0f, 0.0f, context));
        playbackObserved = playbackObserved
            || bank.telemetry().lensGestureActive;
    }
    return playbackObserved;
}
}

int main()
{
    if (!bypassIsExact())
    {
        std::cerr << "module bank bypass failed\n";
        return EXIT_FAILURE;
    }
    if (!everyFamilyHasBoundedOutput())
    {
        std::cerr << "module bank family response failed\n";
        return EXIT_FAILURE;
    }
    if (!deterministicForSameSeed())
    {
        std::cerr << "module bank determinism failed\n";
        return EXIT_FAILURE;
    }
    if (!ecologicalIntegrationIsActive())
    {
        std::cerr << "module bank ecological integration failed\n";
        return EXIT_FAILURE;
    }
    if (!openInteractionCohortIsObservable())
    {
        std::cerr << "open interaction cohort observability failed\n";
        return EXIT_FAILURE;
    }
    if (!refinementCohortIsObservable())
    {
        std::cerr << "refinement cohort observability failed\n";
        return EXIT_FAILURE;
    }
    std::cout << "Integrated module bank: bypass, 15 families, determinism, ecology and both cohort observability checks passed\n";
}
