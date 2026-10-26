// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

// The room as an organism (2026-10-21): the aquarium's own sound, heard back
// through the speakers, is not the room; an external sound is, with its level
// and its register.
#include "core/RoomCoupling.h"

#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <iostream>

namespace
{
using aquorbium::RoomCoupling;
int failures = 0;
void check(bool condition, const char* what)
{
    if (!condition)
    {
        std::cerr << "FAILED: " << what << '\n';
        ++failures;
    }
}
constexpr double pi = 3.14159265358979323846;
std::uint32_t state = 0x1234567u;
float noise()
{
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return static_cast<float>(state) / 4294967296.0f * 2.0f - 1.0f;
}
// the aquarium: a few tones that come and go
float aquarium(int n)
{
    const auto t = n / 48000.0;
    const auto gate = ((n / 9000) % 3) != 0 ? 1.0 : 0.3;
    return static_cast<float>(gate * 0.2 * (std::sin(2.0 * pi * 220.0 * t) + 0.5 * std::sin(2.0 * pi * 1320.0 * t)));
}
RoomCoupling::Reading run(const std::function<float(int, float)>& microphone, int seconds)
{
    RoomCoupling room;
    room.prepare(48000.0);
    for (int n = 0; n < 48000 * seconds; ++n)
    {
        const auto out = aquarium(n);
        room.push(microphone(n, out), out);
    }
    return room.reading();
}
}

int main()
{
    const auto silent = run([](int, float) { return 0.0f; }, 5);
    check(!silent.active, "a silent microphone: no room");

    // the microphone hears only the aquarium, 12 dB down: that is not the room
    const auto self = run([](int, float out) { return 0.25f * out; }, 20);
    std::cout << "self only: room " << self.levelDb << " dB, coupling " << self.coupling << '\n';
    check(!self.active || self.strength < 0.1f, "the aquarium's own sound is not taken for the room");
    check(std::abs(self.coupling - 0.25f) < 0.05f, "the coupling is learned");

    // a low hum in the room (100 Hz at -26 dBFS), over the coupling
    const auto hum = run([](int n, float out) {
        return 0.25f * out + 0.05f * static_cast<float>(std::sin(2.0 * pi * 100.0 * n / 48000.0));
    }, 20);
    // hiss in the room (white noise at about -26 dBFS)
    const auto hiss = run([](int, float out) { return 0.25f * out + 0.087f * noise(); }, 20);
    std::cout << "hum: " << hum.levelDb << " dB at MIDI " << hum.midi << " (coupling " << hum.coupling << "); hiss: " << hiss.levelDb << " dB at MIDI "
              << hiss.midi << '\n';
    check(hum.active && std::abs(hum.levelDb - (-29.0f)) < 3.0f, "an external hum is heard at its level");
    check(hiss.active && hiss.strength > 0.5f, "an external hiss is heard");
    check(hum.midi < 60.0f && hiss.midi > 85.0f, "the room's register: a hum is low, a hiss high");

    // a silent aquarium (paused, muted) in a loud room: everything heard is
    // the room, within a second (it never was before 2026-10-22), and it
    // stops being heard when the room falls silent
    {
        RoomCoupling room;
        room.prepare(48000.0);
        for (int n = 0; n < 48000; ++n)
            room.push(0.087f * noise(), 0.0f);
        const auto loud = room.reading();
        for (int n = 0; n < 48000 * 3; ++n)
            room.push(0.0f, 0.0f);
        const auto quiet = room.reading();
        std::cout << "silent aquarium, loud room: " << loud.levelDb << " dB (active " << loud.active << ")\n";
        check(loud.active && loud.strength > 0.5f && std::abs(loud.levelDb - (-26.0f)) < 3.0f,  // 0.087 x uniform noise: RMS 0.050, -26 dBFS
              "a silent aquarium hears a loud room within a second, at its level");
        check(!quiet.active, "and no longer when the room falls silent");
    }

    if (failures == 0)
        std::cout << "room coupling: all checks passed\n";
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
