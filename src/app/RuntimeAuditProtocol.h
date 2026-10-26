// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include <cmath>

namespace aquorbium::app
{
enum class AuditPhase { baseline, macros, geometry, lens, release, scenes, autonomy, notes, finished };

struct RuntimeAuditProtocol
{
    static constexpr double duration(bool stress) noexcept { return stress ? 60.0 : 20.0; }
    static AuditPhase phase(double seconds, bool stress) noexcept
    {
        if (!std::isfinite(seconds) || seconds < 0.0 || seconds >= duration(stress))
            return AuditPhase::finished;
        if (seconds < 4.0) return AuditPhase::baseline;
        if (seconds < 9.0) return AuditPhase::macros;
        if (seconds < 14.0) return AuditPhase::geometry;
        if (seconds < 18.0) return AuditPhase::lens;
        if (seconds < 20.0) return AuditPhase::release;
        if (seconds < 35.0) return AuditPhase::scenes;
        if (seconds < 45.0) return AuditPhase::autonomy;
        if (seconds < 58.0) return AuditPhase::notes;
        return AuditPhase::release;
    }
    static int step(double seconds, bool stress) noexcept
    {
        return phase(seconds, stress) == AuditPhase::finished ? -1 : static_cast<int>(seconds * 4.0);
    }
};
}
