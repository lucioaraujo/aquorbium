// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/FixedLatestSnapshot.h"

#include <array>
#include <atomic>
#include <cstdlib>
#include <iostream>
#include <thread>

#define REQUIRE(condition) do { if (!(condition)) { \
    std::cerr << "requirement failed at line " << __LINE__ << ": " #condition "\n"; \
    return EXIT_FAILURE; } } while (false)

struct Frame
{
    std::uint64_t sequence = 0;
    std::array<std::uint64_t, 32> values {};
};

Frame makeFrame(std::uint64_t sequence)
{
    Frame frame;
    frame.sequence = sequence;
    for (std::size_t index = 0; index < frame.values.size(); ++index)
        frame.values[index] = sequence ^ (index * 0xB10A5EEDu);
    return frame;
}

bool coherent(const Frame& frame)
{
    for (std::size_t index = 0; index < frame.values.size(); ++index)
        if (frame.values[index] != (frame.sequence ^ (index * 0xB10A5EEDu)))
            return false;
    return true;
}

int main()
{
    aquorbium::FixedLatestSnapshot<Frame> bridge;
    Frame frame = makeFrame(99);
    REQUIRE(!bridge.consumeLatest(frame));
    REQUIRE(frame.sequence == 99);
    bridge.publish(makeFrame(1));
    bridge.publish(makeFrame(2));
    REQUIRE(bridge.consumeLatest(frame));
    REQUIRE(frame.sequence == 2 && coherent(frame));
    REQUIRE(!bridge.consumeLatest(frame));

    constexpr std::uint64_t finalSequence = 100000;
    std::atomic<bool> finished { false };
    std::thread producer([&] {
        for (std::uint64_t sequence = 3; sequence <= finalSequence; ++sequence)
            bridge.publish(makeFrame(sequence));
        finished.store(true, std::memory_order_release);
    });
    bool valid = true;
    std::uint64_t lastSequence = 2;
    for (int iteration = 0; iteration < 1000000; ++iteration)
    {
        if (bridge.consumeLatest(frame))
        {
            valid = valid && coherent(frame) && frame.sequence > lastSequence;
            lastSequence = frame.sequence;
        }
        if (finished.load(std::memory_order_acquire))
            break;
        if (iteration % 1000 == 0)
            std::this_thread::yield();
    }
    producer.join();
    if (bridge.consumeLatest(frame))
    {
        valid = valid && coherent(frame) && frame.sequence > lastSequence;
        lastSequence = frame.sequence;
    }
    REQUIRE(valid);
    REQUIRE(lastSequence == finalSequence);
    REQUIRE(!bridge.consumeLatest(frame));
    return EXIT_SUCCESS;
}
