// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace aquorbium
{
// Fixed-capacity, non-blocking queue for many producer threads and one
// consumer. It is intended for short control messages entering the audio
// thread. Capacity must be a power of two; no operation allocates or locks.
template <typename Value, std::size_t Capacity>
class FixedMpscQueue final
{
    static_assert(Capacity >= 2);
    static_assert((Capacity & (Capacity - 1)) == 0,
                  "FixedMpscQueue capacity must be a power of two");
    static_assert(std::is_trivially_copyable_v<Value>,
                  "FixedMpscQueue values must be trivially copyable");
    static_assert(std::atomic<std::size_t>::is_always_lock_free,
                  "FixedMpscQueue requires lock-free position atomics");

public:
    FixedMpscQueue() noexcept
    {
        for (std::size_t index = 0; index < Capacity; ++index)
            cells[index].sequence.store(index, std::memory_order_relaxed);
    }

    FixedMpscQueue(const FixedMpscQueue&) = delete;
    FixedMpscQueue& operator=(const FixedMpscQueue&) = delete;

    [[nodiscard]] bool tryPush(const Value& value) noexcept
    {
        auto position = enqueuePosition.load(std::memory_order_relaxed);
        for (;;)
        {
            auto& cell = cells[position & indexMask];
            const auto sequence = cell.sequence.load(std::memory_order_acquire);
            const auto difference = static_cast<std::intptr_t>(sequence)
                - static_cast<std::intptr_t>(position);

            if (difference == 0)
            {
                if (enqueuePosition.compare_exchange_weak(position, position + 1,
                                                          std::memory_order_relaxed,
                                                          std::memory_order_relaxed))
                {
                    cell.value = value;
                    cell.sequence.store(position + 1, std::memory_order_release);
                    return true;
                }
            }
            else if (difference < 0)
            {
                droppedPushes.fetch_add(1, std::memory_order_relaxed);
                return false;
            }
            else
            {
                position = enqueuePosition.load(std::memory_order_relaxed);
            }
        }
    }

    // Exactly one thread may call tryPop(). Producers may call tryPush()
    // concurrently from any number of threads.
    [[nodiscard]] bool tryPop(Value& destination) noexcept
    {
        auto& cell = cells[dequeuePosition & indexMask];
        const auto sequence = cell.sequence.load(std::memory_order_acquire);
        const auto difference = static_cast<std::intptr_t>(sequence)
            - static_cast<std::intptr_t>(dequeuePosition + 1);
        if (difference != 0)
            return false;

        destination = cell.value;
        cell.sequence.store(dequeuePosition + Capacity, std::memory_order_release);
        ++dequeuePosition;
        return true;
    }

    [[nodiscard]] std::uint64_t droppedCount() const noexcept
    {
        return droppedPushes.load(std::memory_order_relaxed);
    }

private:
    struct Cell
    {
        std::atomic<std::size_t> sequence { 0 };
        Value value {};
    };

    static constexpr std::size_t indexMask = Capacity - 1;
    std::array<Cell, Capacity> cells {};
    alignas(64) std::atomic<std::size_t> enqueuePosition { 0 };
    alignas(64) std::size_t dequeuePosition = 0;
    std::atomic<std::uint64_t> droppedPushes { 0 };
};
}
