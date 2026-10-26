// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>

namespace aquorbium
{
// VARIA - the controls move over the music's time (2026-10-24). The author:
// "e controles também podem ser alterados ao longo do tempo de música"; "como
// é feito no rasgo modular, com um botão de varia".
//
// The concept is the Rasgo Modular's Motion Engine v3, the chaotic hand that
// plays the patch (dossies/ESTUDO_seed_composicao_generativa.md §3.7;
// apps/panel/MotionField.hpp and MotionEngine.hpp at commit 4ce898f). The
// author's direction there holds here: no rulebook of intentions, a small
// chaotic system that never repeats and never stops, which every control
// projects in its own way - so the controls move in relation, each along its
// own path. Rewritten for the Aquorbium, no code shared (each RASGO
// instrument keeps its own code: GOVERNANCA_E_TRANSVERSALIDADE.md §7.0). What
// is taken from the Modular, and kept:
//   - the Thomas cyclically symmetric attractor (R. Thomas, "Deterministic
//     chaos seen in terms of feedback circuits", Int. J. Bifurcation and
//     Chaos 9, 1999), b = 0.1899, a slow step;
//   - a projection per control, seeded by (seed, control);
//   - a reach per control that only sets HOW FAR it may go, around the value
//     the seed or the musician left, never the whole range (the Modular's
//     2026-09-05 lesson);
//   - the boldness: now and then one control stretches its reach for a few
//     seconds, then relaxes;
//   - the musician's hand wins: a control moved by hand re-anchors there;
//   - intensity 0 freezes for real (nothing is written), 2 doubles the reach,
//     under a ceiling of 0.48 of the range;
//   - near the output's ceiling everything is pulled back to its centre.
// What is the Aquorbium's own:
//   - the hand's speed follows the aquarium's ACTIVITY (the organisms'
//     energy), as the Modular follows its sound's energy;
//   - a layer of the FORM over the hand (the Modular's "morceau" idea, §3.7.4):
//     the session's arc of tension (SessionGenesis.h) warms the water and
//     quickens the current at the climax - brighter tone, more population,
//     more scatter, a smaller and drier space - and lets it settle at rest;
//   - the reaches are named from the Aquorbium's controls: the bodies'
//     choice, the salinity and the output stage are never moved; the gates
//     (a module's amount in the mix) breathe a little and a closed gate
//     stays closed (the window is relative to the value).
class ControlMotion final
{
public:
    static constexpr std::size_t capacity = 128;
    enum class Reach : std::uint8_t { fixed, structural, level, hot, free };
    struct Control
    {
        std::string_view id;
        float value = 0.0f;  // 0..1
    };

    [[nodiscard]] static Reach reachOf(std::string_view id) noexcept
    {
        const auto has = [id](std::string_view w) { return id.find(w) != std::string_view::npos; };
        // the musician's own and the discrete choices: never moved
        if (id.starts_with("master") || id == "creativeMaster" || id == "supportPresence" || id.starts_with("lens")
            || id.starts_with("room") || id.starts_with("timbreBody") || id.starts_with("timbreKind")
            || id == "musicSalinity" || id == "autonomous")
            return Reach::fixed;
        // the music's structure: it barely moves, and slowly
        if (has("orbit") || has("interval") || has("Phase") || has("Context") || has("brain"))
            return Reach::structural;
        // a module's amount in the mix: it breathes, never sweeps
        if (has("Influence") || has("Mix") || id.starts_with("module") || id == "spaceGesture")
            return Reach::level;
        // strong near their extremes
        if (has("Resonance") || has("Nightmare") || has("Mutation") || id == "danger")
            return Reach::hot;
        return Reach::free;
    }

    // the fraction of the range each reach may travel, and its easing time (s)
    [[nodiscard]] static float amplitude(Reach r) noexcept
    {
        switch (r)
        {
            case Reach::fixed: return 0.0f;
            case Reach::structural: return 0.015f;
            case Reach::level: return 0.04f;
            case Reach::hot: return 0.04f;
            case Reach::free: return 0.15f;
        }
        return 0.0f;
    }
    [[nodiscard]] static float easing(Reach r) noexcept
    {
        switch (r)
        {
            case Reach::fixed: return 1.0f;
            case Reach::structural: return 8.0f;
            case Reach::level: return 4.0f;
            case Reach::hot: return 3.5f;
            case Reach::free: return 1.8f;
        }
        return 2.5f;
    }

    // how the arc of tension pulls a control: +1 up at the climax, -1 down,
    // 0 not at all (warmer water, a quicker current)
    [[nodiscard]] static float tensionPull(std::string_view id) noexcept
    {
        if (id == "spaceTone" || id == "environmentCentre" || id == "rhythmDensity" || id == "strikerPopulation"
            || id == "strikerScatter" || id == "density" || id == "musicReaction" || id == "geometryMutation")
            return 1.0f;
        if (id == "spaceSize" || id == "tailDecay" || id == "calm" || id == "environmentAmbience"
            || id == "spaceMemory")
            return -1.0f;
        return 0.0f;
    }

    // builds a fibre for every control, around its current value
    void inhabit(std::span<const Control> controls, std::uint32_t seed) noexcept
    {
        count = 0;
        for (const auto& c : controls)
        {
            if (count >= capacity)
                break;
            auto& f = fibres[count++];
            f = Fibre {};
            f.id = c.id;
            f.reach = reachOf(c.id);
            f.centre = std::clamp(c.value, 0.0f, 1.0f);
            f.value = f.centre;
            f.pull = tensionPull(c.id);
            // a projection seeded by (seed, id)
            std::uint64_t h = 1469598103934665603ull ^ (static_cast<std::uint64_t>(seed) * 0x9e3779b97f4a7c15ull);
            for (const auto ch : c.id)
            {
                h ^= static_cast<unsigned char>(ch);
                h *= 1099511628211ull;
            }
            const auto w = [&h]() {
                h ^= h >> 13;
                h *= 0xbf58476d1ce4e5b9ull;
                h ^= h >> 7;
                return 2.0f * static_cast<float>(h >> 40) / 16777216.0f - 1.0f;
            };
            f.wx = w();
            f.wy = w();
            f.wz = w();
            const auto n = std::sqrt(f.wx * f.wx + f.wy * f.wy + f.wz * f.wz);
            if (n > 1.0e-6f)
            {
                f.wx /= n;
                f.wy /= n;
                f.wz /= n;
            }
        }
        // the field and the boldness, from the seed too
        rng = 0x9e3779b97f4a7c15ull ^ (static_cast<std::uint64_t>(seed) * 0xd1b54a32d192ed03ull);
        x = 0.10 + 0.02 * static_cast<double>(nextBits() % 128);
        y = -0.05 + 0.02 * static_cast<double>(nextBits() % 128);
        z = 0.03 * static_cast<double>(nextBits() % 128) - 1.0;
        speed = 1.0;
        boldCountdown = 10.0f;
        boldLeft = 0.0f;
        boldFibre = capacity;
    }

    // 0 freezes, 1 is the Modular's historical depth, 2 doubles it
    void setIntensity(float value) noexcept { intensity = std::clamp(value, 0.0f, 2.0f); }
    [[nodiscard]] float currentIntensity() const noexcept { return intensity; }
    [[nodiscard]] std::size_t size() const noexcept { return count; }

    // Advances dt seconds. values: the controls' current values, in the order
    // given to inhabit(); a value moved away from what the hand wrote last is
    // the musician's, and the fibre re-anchors there. The new values are
    // written back. activity 0..1 (the organisms' energy) quickens the hand;
    // tension 0..1 (the session's arc) pulls the form; nearCeiling 0..1 (how
    // close the output is to its ceiling) brings everything home.
    void tick(std::span<float> values, float dt, float activity, float tension, float nearCeiling = 0.0f) noexcept
    {
        if (intensity <= 0.0f || count == 0 || !(dt > 0.0f))
            return;
        const auto target = 0.35 + 0.9 * static_cast<double>(std::clamp(activity, 0.0f, 1.0f));
        speed += (target - speed) * (1.0 - std::exp(-static_cast<double>(dt) * 0.2));
        const auto step = static_cast<double>(dt) * 0.45 * speed;
        constexpr double b = 0.1899;  // Thomas: chaotic below ~0.208
        const auto nx = x + step * (std::sin(y) - b * x);
        const auto ny = y + step * (std::sin(z) - b * y);
        const auto nz = z + step * (std::sin(x) - b * z);
        x = nx;
        y = ny;
        z = nz;
        if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z))
        {
            x = 0.1;
            y = 0.0;
            z = 0.0;
        }
        const auto fx = static_cast<float>(std::tanh(x * 0.42));
        const auto fy = static_cast<float>(std::tanh(y * 0.42));
        const auto fz = static_cast<float>(std::tanh(z * 0.42));

        // the boldness: one free control at a time, now and then
        boldCountdown -= dt;
        if (boldCountdown <= 0.0f)
        {
            boldCountdown = 22.0f + 34.0f * unit();
            const auto i = static_cast<std::size_t>(unit() * static_cast<float>(count)) % count;
            if (fibres[i].reach == Reach::free)
            {
                boldFibre = i;
                boldLeft = 4.0f + 7.0f * unit();
            }
        }
        if (boldLeft > 0.0f)
            boldLeft -= dt;
        const auto duck = std::clamp((nearCeiling - 0.82f) * 3.5f, 0.0f, 1.0f);
        const auto form = std::clamp(tension, 0.0f, 1.0f) - 0.5f;

        const auto n = std::min(count, values.size());
        for (std::size_t i = 0; i < n; ++i)
        {
            auto& f = fibres[i];
            if (f.reach == Reach::fixed)
                continue;
            if (std::abs(values[i] - f.value) > 0.012f)
            {
                // the musician's hand: breathe around the new value
                f.centre = std::clamp(values[i], 0.0f, 1.0f);
                f.value = f.centre;
            }
            // a control at zero - a closed gate, an off module - stays exactly
            // at zero: the cards read "> 0" as on
            if (f.centre <= 1.0e-4f)
                continue;
            const auto bold = boldLeft > 0.0f && boldFibre == i ? 2.0f : 1.0f;
            f.boldness += (bold - f.boldness) * (1.0f - std::exp(-dt / 2.5f));
            const auto raw = std::tanh(f.wx * fx + f.wy * fy + f.wz * fz);
            const auto reach = std::min(0.48f, amplitude(f.reach) * f.boldness * intensity);
            // relative to the value: a closed gate (0) stays closed
            const auto window = std::min(reach, std::max(f.centre, 1.0e-3f) * 4.0f * reach);
            auto goal = f.centre + raw * window + f.pull * form * 0.8f * window;
            goal = std::clamp(goal, f.centre - 1.4f * window, f.centre + 1.4f * window);
            const auto home = duck * (f.reach == Reach::hot || f.reach == Reach::level ? 1.0f : 0.55f);
            goal += (f.centre - goal) * home;
            goal = std::clamp(goal, 0.0f, 1.0f);
            const auto k = 1.0f - std::exp(-dt / std::max(0.05f, easing(f.reach)));
            f.value = std::clamp(f.value + (goal - f.value) * k, 0.0f, 1.0f);
            values[i] = f.value;
        }
    }

    [[nodiscard]] float centreOf(std::size_t index) const noexcept
    {
        return index < count ? fibres[index].centre : 0.0f;
    }

private:
    struct Fibre
    {
        std::string_view id;
        Reach reach = Reach::fixed;
        float centre = 0.0f, value = 0.0f, pull = 0.0f, boldness = 1.0f;
        float wx = 0.0f, wy = 0.0f, wz = 0.0f;
    };
    std::array<Fibre, capacity> fibres {};
    std::size_t count = 0;
    double x = 0.1, y = 0.0, z = 0.0, speed = 1.0;
    std::uint64_t rng = 1;
    float intensity = 1.0f;
    float boldCountdown = 10.0f, boldLeft = 0.0f;
    std::size_t boldFibre = capacity;

    std::uint64_t nextBits() noexcept
    {
        rng ^= rng << 13;
        rng ^= rng >> 7;
        rng ^= rng << 17;
        return rng;
    }
    float unit() noexcept { return static_cast<float>(nextBits() >> 40) / 16777216.0f; }
};
}
