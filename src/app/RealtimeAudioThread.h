// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#if defined(__linux__)
 #include <pthread.h>
 #include <sched.h>
 #include <sys/resource.h>
#endif

#include <algorithm>

namespace aquorbium::app
{
// The audio thread asks for real-time scheduling (2026-10-24). On Linux JUCE
// starts its ALSA thread as an ordinary SCHED_OTHER thread
// (juce_ALSA_linux.cpp: startThread (Priority::high)), so on a loaded machine
// the audio callback waits behind other processes and arrives late - an XRUN
// (the author's session: XRUN 190 = LATE 190 while the engine itself used
// ~12 % of the block). Called once, from the audio thread itself, at its
// first callback: one system call, no lock, no allocation. It succeeds only
// where the user may use real-time priorities (RLIMIT_RTPRIO > 0, e.g. the
// "pipewire" or "audio" group); otherwise nothing changes and the result says
// so. Returns the SCHED_RR priority obtained, or 0.
inline int promoteCurrentThreadToRealtime() noexcept
{
#if defined(__linux__)
    rlimit limit {};
    if (getrlimit(RLIMIT_RTPRIO, &limit) != 0 || limit.rlim_cur == 0)
        return 0;
    // below PipeWire's own threads (server ~88, client data loop ~83)
    const auto ceiling = static_cast<int>(std::min<rlim_t>(limit.rlim_cur, 70));
    const auto priority = std::clamp(ceiling, sched_get_priority_min(SCHED_RR), sched_get_priority_max(SCHED_RR));
    sched_param parameter {};
    parameter.sched_priority = priority;
    return pthread_setschedparam(pthread_self(), SCHED_RR, &parameter) == 0 ? priority : 0;
#else
    return 0;
#endif
}
}
