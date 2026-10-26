// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include <array>
#include <atomic>
#include <cstdint>
#include <type_traits>

namespace aquorbium
{
// Three ownership-separated buffers. Exactly one producer and one consumer.
// Intermediate frames may be superseded; the consumer gets a complete frame.
// No retries, waiting, locks, allocation or unbounded work on either side.
template <typename Value>
class FixedLatestSnapshot final
{
    static_assert(std::is_trivially_copyable_v<Value>);
    static_assert(std::is_nothrow_copy_assignable_v<Value>);
    static_assert(std::atomic<std::uint32_t>::is_always_lock_free);

public:
    void publish(const Value& value) noexcept
    {
        buffers[back] = value;
        back = middle.exchange(back | dirty, std::memory_order_acq_rel) & indexMask;
    }

    [[nodiscard]] bool consumeLatest(Value& destination) noexcept
    {
        if ((middle.load(std::memory_order_acquire) & dirty) == 0)
            return false;
        front = middle.exchange(front, std::memory_order_acq_rel) & indexMask;
        destination = buffers[front];
        return true;
    }

private:
    static constexpr std::uint32_t dirty = 4;
    static constexpr std::uint32_t indexMask = 3;
    std::array<Value, 3> buffers {};
    alignas(64) std::uint32_t back = 2; // producer-owned
    alignas(64) std::atomic<std::uint32_t> middle { 1 };
    alignas(64) std::uint32_t front = 0; // consumer-owned
};
}
