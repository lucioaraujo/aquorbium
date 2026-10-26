// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

namespace aquorbium
{
struct SpatialVector3
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

// Rigid transform between Aquarium-local coordinates and the world occupied by
// external observers. Scale is deliberately absent: changing the visual size
// of the Aquarium must not silently change its physical boundary.
class AquariumPose final
{
public:
    struct Quaternion
    {
        float w = 1.0f;
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
    };

    AquariumPose() noexcept = default;
    AquariumPose(SpatialVector3 translation, Quaternion orientation) noexcept;

    [[nodiscard]] static AquariumPose fromYawPitchRoll(
        SpatialVector3 translation, float yaw, float pitch, float roll) noexcept;

    // Invalid or degenerate input is rejected and the last valid pose remains.
    [[nodiscard]] bool set(SpatialVector3 translation, Quaternion orientation) noexcept;

    [[nodiscard]] SpatialVector3 localToWorld(SpatialVector3 local) const noexcept;
    [[nodiscard]] SpatialVector3 worldToLocal(SpatialVector3 world) const noexcept;
    [[nodiscard]] SpatialVector3 localDirectionToWorld(SpatialVector3 local) const noexcept;
    [[nodiscard]] SpatialVector3 worldDirectionToLocal(SpatialVector3 world) const noexcept;

    [[nodiscard]] SpatialVector3 translation() const noexcept { return position; }
    [[nodiscard]] Quaternion orientation() const noexcept { return rotation; }

private:
    SpatialVector3 position {};
    Quaternion rotation {};
};
}
