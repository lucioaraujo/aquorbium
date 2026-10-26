// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include <algorithm>
#include <array>

namespace aquorbium
{
// Delivery profiles (aquorbium-tarefas.md, phase 7a/7b, 2026-10-23): a REPORT
// beside each REC take of what each destination would do to it - never a
// correction applied to the sound, and never a single target imposed on the
// live performance. Each profile is a loudness target and a true-peak
// ceiling, taken from its primary source:
//
// - Spotify, "Loudness normalization on Spotify" (support.spotify.com,
//   artists article, read 2026-10-23): Normal -14 LUFS, Quiet -19 LUFS; when
//   a softer master is turned up, Spotify "leave[s] 1 dB headroom", so the
//   positive gain stops where the true peak reaches -1 dBTP (its example: a
//   -20 LUFS track peaking at -5 dBTP is raised only to -16 LUFS). Its Loud
//   mode (-11 LUFS) uses Spotify's own limiter and is not simulated here.
//   Mastering advice from the same page: true peak under -1 dBTP, or under
//   -2 dBTP when the master is louder than -14 LUFS.
// - EBU R 128 (broadcast): -23 LUFS, maximum permitted true peak -1 dBTP;
//   the gain is applied in full, so the report says whether the take would
//   then pass the ceiling.
struct DeliveryProfile
{
    const char* key;
    float targetLufs;
    float ceilingDbtp;
    bool positiveGainLimitedByCeiling;  // the platform stops turning up at the ceiling
};

inline constexpr std::array<DeliveryProfile, 3> deliveryProfiles {{
    { "spotify_normal", -14.0f, -1.0f, true },
    { "spotify_quiet", -19.0f, -1.0f, true },
    { "ebu_r128", -23.0f, -1.0f, false },
}};

struct DeliveryVerdict
{
    float gainToTargetDb = 0.0f;   // what it would take to reach the target
    float appliedGainDb = 0.0f;    // what the destination would apply
    float resultingLufs = 0.0f;
    float resultingTruePeakDb = 0.0f;
    bool reachesTarget = false;    // within 0.05 LU
    bool truePeakWithinCeiling = false;
};

[[nodiscard]] inline DeliveryVerdict judgeDelivery(const DeliveryProfile& profile,
                                                   float integratedLufs, float truePeakDb) noexcept
{
    DeliveryVerdict v;
    v.gainToTargetDb = profile.targetLufs - integratedLufs;
    v.appliedGainDb = v.gainToTargetDb;
    if (profile.positiveGainLimitedByCeiling && v.appliedGainDb > 0.0f)
        v.appliedGainDb = std::max(0.0f, std::min(v.appliedGainDb, profile.ceilingDbtp - truePeakDb));
    v.resultingLufs = integratedLufs + v.appliedGainDb;
    v.resultingTruePeakDb = truePeakDb + v.appliedGainDb;
    v.reachesTarget = v.resultingLufs >= profile.targetLufs - 0.05f && v.resultingLufs <= profile.targetLufs + 0.05f;
    v.truePeakWithinCeiling = v.resultingTruePeakDb <= profile.ceilingDbtp + 1.0e-4f;
    return v;
}

// Spotify's mastering advice for the file itself, before any normalisation
[[nodiscard]] inline float spotifyMasterCeilingDbtp(float integratedLufs) noexcept
{
    return integratedLufs > -14.0f ? -2.0f : -1.0f;
}
}
