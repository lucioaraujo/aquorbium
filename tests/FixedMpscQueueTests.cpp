// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/FixedMpscQueue.h"

#include <array>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <thread>
#include <vector>

namespace
{
struct Event
{
    std::uint32_t producer = 0;
    std::uint32_t ordinal = 0;
};

bool testCapacityAndOrder()
{
    aquorbium::FixedMpscQueue<Event, 8> queue;
    for (std::uint32_t value = 0; value < 8; ++value)
        if (!queue.tryPush({ 0, value })) return false;
    if (queue.tryPush({ 0, 8 }) || queue.droppedCount() != 1) return false;

    Event event;
    for (std::uint32_t value = 0; value < 8; ++value)
    {
        if (!queue.tryPop(event) || event.producer != 0 || event.ordinal != value)
            return false;
    }
    return !queue.tryPop(event);
}

bool testWrapAround()
{
    aquorbium::FixedMpscQueue<Event, 4> queue;
    for (std::uint32_t cycle = 0; cycle < 1000; ++cycle)
    {
        for (std::uint32_t index = 0; index < 4; ++index)
            if (!queue.tryPush({ cycle, index })) return false;
        for (std::uint32_t index = 0; index < 4; ++index)
        {
            Event event;
            if (!queue.tryPop(event) || event.producer != cycle || event.ordinal != index)
                return false;
        }
    }
    return queue.droppedCount() == 0;
}

bool testConcurrentProducers()
{
    constexpr std::uint32_t producerCount = 4;
    constexpr std::uint32_t eventsPerProducer = 20000;
    constexpr std::size_t totalEvents = producerCount * eventsPerProducer;
    aquorbium::FixedMpscQueue<Event, 1024> queue;
    std::atomic<bool> start { false };
    std::atomic<std::uint32_t> completedProducers { 0 };
    std::array<std::thread, producerCount> producers;

    for (std::uint32_t producer = 0; producer < producerCount; ++producer)
    {
        producers[producer] = std::thread([&, producer] {
            while (!start.load(std::memory_order_acquire))
                std::this_thread::yield();
            for (std::uint32_t ordinal = 0; ordinal < eventsPerProducer; ++ordinal)
            {
                while (!queue.tryPush({ producer, ordinal }))
                    std::this_thread::yield();
            }
            completedProducers.fetch_add(1, std::memory_order_release);
        });
    }

    start.store(true, std::memory_order_release);
    std::vector<bool> seen(totalEvents, false);
    std::array<std::uint32_t, producerCount> expectedOrdinal {};
    std::size_t received = 0;
    bool valid = true;
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(10);

    while (received < totalEvents && std::chrono::steady_clock::now() < deadline)
    {
        Event event;
        if (!queue.tryPop(event))
        {
            if (completedProducers.load(std::memory_order_acquire) == producerCount)
                std::this_thread::yield();
            continue;
        }

        if (event.producer >= producerCount || event.ordinal >= eventsPerProducer
            || event.ordinal != expectedOrdinal[event.producer])
        {
            valid = false;
            continue;
        }
        ++expectedOrdinal[event.producer];
        const auto key = static_cast<std::size_t>(event.producer) * eventsPerProducer
            + event.ordinal;
        if (seen[key]) valid = false;
        seen[key] = true;
        ++received;
    }

    for (auto& producer : producers)
        producer.join();

    if (received != totalEvents) valid = false;
    for (const auto count : expectedOrdinal)
        if (count != eventsPerProducer) valid = false;
    return valid;
}
}

int main()
{
    if (!testCapacityAndOrder())
    {
        std::cerr << "capacity/order test failed\n";
        return EXIT_FAILURE;
    }
    if (!testWrapAround())
    {
        std::cerr << "wrap-around test failed\n";
        return EXIT_FAILURE;
    }
    if (!testConcurrentProducers())
    {
        std::cerr << "concurrent producer test failed\n";
        return EXIT_FAILURE;
    }
    std::cout << "aquorbium fixed MPSC queue tests passed\n";
}
