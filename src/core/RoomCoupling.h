// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include <array>
#include <cstddef>

namespace aquorbium
{
// The room as one more organism (CONCEITOS_VERIFICACAO_E_APROFUNDAMENTO.md
// §2.3; 2026-10-21). Di Scipio's Audible Eco-Systemic Interface (2003,
// Organised Sound 8:269) closes the loop through the room: the system hears
// the space it sounds in and adjusts itself. Here, optionally and off by
// default, the microphone becomes perception: the reef hears the room's noise
// as an organism more - the Lombard effect answers it and the voices move
// their register away from the band it occupies.
//
// The microphone also hears the aquarium itself through the speakers; if that
// counted, the Lombard effect would push itself to its ceiling. So the module
// learns the coupling between what the aquarium plays and what the
// microphone hears: microphone power = k^2 x output power + room power, and
// since the aquarium's level varies independently of the room, the slope of
// a running regression (exponential moments, ~10 s) recovers k^2. The room is
// the EXCESS over k^2 times the output - voices, instruments, traffic. Until
// the coupling is learned (a few seconds of varying output) the room is not
// reported.
//
// Only descriptors leave this class (a level and a register); the samples
// are never stored, recorded or sent, and never reach the output. Fixed
// state, no allocation: safe on the audio thread.
class RoomCoupling final
{
public:
    struct Reading
    {
        bool active = false;      // the room is louder than the floor
        float levelDb = -120.0f;  // the room's excess, dBFS
        float strength = 0.0f;    // 0 (quiet room) .. 1 (loud room, -20 dBFS and above)
        float midi = 60.0f;       // where in the register the room sounds (band centroid)
        float coupling = 0.0f;    // the learned speaker-to-microphone gain
    };

    static constexpr float floorDb = -60.0f;

    void prepare(double sampleRate) noexcept;
    void reset() noexcept;
    // one sample of the microphone and of the aquarium's own output
    void push(float microphone, float output) noexcept;
    [[nodiscard]] Reading reading() const noexcept { return last; }

private:
    void update() noexcept;

    // three bands: low (< 300 Hz), middle (300 Hz .. 2 kHz), high (> 2 kHz),
    // for the microphone and the output; one-pole splits
    struct Bands
    {
        float lowState = 0.0f, midState = 0.0f;
        std::array<float, 3> power {};
    };
    void split(Bands& bands, float x) noexcept;

    double rate = 48000.0;
    float lowCoefficient = 0.04f, midCoefficient = 0.23f;
    float powerCoefficient = 1.0e-4f;  // ~300 ms
    Bands mic {}, out {};
    // running moments of output power (x) and microphone power (y), 100 Hz
    double meanX = 0.0, meanY = 0.0, meanXX = 0.0, meanXY = 0.0;
    double weight = 0.0;  // the moments' accumulated weight (bias correction)
    double momentCoefficient = 0.001;  // ~10 s at 100 Hz
    float coupling2 = 0.0f;            // k^2
    std::size_t learnedUpdates = 0;
    std::size_t settledUpdates = 0;  // updates since the microphone powers settled (silent output)
    std::size_t countdown = 0;
    std::size_t interval = 480;        // descriptors at 100 Hz
    Reading last {};
};
}
