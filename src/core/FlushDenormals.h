// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include <cstdint>

#if defined(__SSE__) || defined(_M_X64) || defined(_M_IX86_FP)
#include <xmmintrin.h>
#endif

namespace aquorbium
{
// Denormal numbers flushed to zero for as long as this lives (2026-10-22).
// Decaying filters, resonators and DC blockers reach the denormal range in
// silence, where the CPU is up to ~100x slower: measured, the whole engine
// with everything on and four MIDI voices took 3.1 ms per 256-sample block
// without it and 1.8 ms with it. The app's callback already flushed them
// (juce::ScopedNoDenormals); the engine now does it itself, so any host - a
// test, a future plugin - gets the same. The previous state is restored.
class ScopedFlushDenormals final
{
public:
    ScopedFlushDenormals() noexcept
    {
#if defined(__SSE__) || defined(_M_X64) || defined(_M_IX86_FP)
        previous = _mm_getcsr();
        _mm_setcsr(previous | 0x8040u);  // FTZ (bit 15) and DAZ (bit 6)
#elif defined(__aarch64__)
        std::uint64_t fpcr = 0;
        asm volatile("mrs %0, fpcr" : "=r"(fpcr));
        previous = fpcr;
        fpcr |= (1ull << 24);  // FZ
        asm volatile("msr fpcr, %0" : : "r"(fpcr));
#endif
    }
    ~ScopedFlushDenormals()
    {
#if defined(__SSE__) || defined(_M_X64) || defined(_M_IX86_FP)
        _mm_setcsr(static_cast<unsigned int>(previous));
#elif defined(__aarch64__)
        asm volatile("msr fpcr, %0" : : "r"(previous));
#endif
    }
    ScopedFlushDenormals(const ScopedFlushDenormals&) = delete;
    ScopedFlushDenormals& operator=(const ScopedFlushDenormals&) = delete;

private:
    std::uint64_t previous = 0;
};
}
