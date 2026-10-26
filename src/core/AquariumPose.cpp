// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/AquariumPose.h"

#include <cmath>

namespace aquorbium
{
namespace
{
bool finite(SpatialVector3 value) noexcept
{
    return std::isfinite(value.x) && std::isfinite(value.y) && std::isfinite(value.z);
}

bool finite(AquariumPose::Quaternion value) noexcept
{
    return std::isfinite(value.w) && std::isfinite(value.x)
        && std::isfinite(value.y) && std::isfinite(value.z);
}

AquariumPose::Quaternion multiply(AquariumPose::Quaternion left,
                                  AquariumPose::Quaternion right) noexcept
{
    return {
        left.w * right.w - left.x * right.x - left.y * right.y - left.z * right.z,
        left.w * right.x + left.x * right.w + left.y * right.z - left.z * right.y,
        left.w * right.y - left.x * right.z + left.y * right.w + left.z * right.x,
        left.w * right.z + left.x * right.y - left.y * right.x + left.z * right.w
    };
}

SpatialVector3 rotate(SpatialVector3 value, AquariumPose::Quaternion quaternion) noexcept
{
    // Unit-quaternion vector rotation without temporary quaternion products.
    const SpatialVector3 q { quaternion.x, quaternion.y, quaternion.z };
    const SpatialVector3 twiceCross {
        2.0f * (q.y * value.z - q.z * value.y),
        2.0f * (q.z * value.x - q.x * value.z),
        2.0f * (q.x * value.y - q.y * value.x)
    };
    const SpatialVector3 secondCross {
        q.y * twiceCross.z - q.z * twiceCross.y,
        q.z * twiceCross.x - q.x * twiceCross.z,
        q.x * twiceCross.y - q.y * twiceCross.x
    };
    return { value.x + quaternion.w * twiceCross.x + secondCross.x,
             value.y + quaternion.w * twiceCross.y + secondCross.y,
             value.z + quaternion.w * twiceCross.z + secondCross.z };
}
}

AquariumPose::AquariumPose(SpatialVector3 translation, Quaternion orientation) noexcept
{
    (void) set(translation, orientation);
}

AquariumPose AquariumPose::fromYawPitchRoll(
    SpatialVector3 translation, float yaw, float pitch, float roll) noexcept
{
    if (!finite(translation) || !std::isfinite(yaw)
        || !std::isfinite(pitch) || !std::isfinite(roll))
        return {};

    const auto halfYaw = yaw * 0.5f;
    const auto halfPitch = pitch * 0.5f;
    const auto halfRoll = roll * 0.5f;
    const Quaternion yawRotation { std::cos(halfYaw), 0.0f, std::sin(halfYaw), 0.0f };
    const Quaternion pitchRotation { std::cos(halfPitch), std::sin(halfPitch), 0.0f, 0.0f };
    const Quaternion rollRotation { std::cos(halfRoll), 0.0f, 0.0f, std::sin(halfRoll) };
    return { translation, multiply(multiply(yawRotation, pitchRotation), rollRotation) };
}

bool AquariumPose::set(SpatialVector3 translation, Quaternion orientation) noexcept
{
    if (!finite(translation) || !finite(orientation))
        return false;
    const auto normSquared = orientation.w * orientation.w + orientation.x * orientation.x
        + orientation.y * orientation.y + orientation.z * orientation.z;
    if (!std::isfinite(normSquared) || normSquared < 1.0e-12f)
        return false;
    const auto inverseNorm = 1.0f / std::sqrt(normSquared);
    position = translation;
    rotation = { orientation.w * inverseNorm, orientation.x * inverseNorm,
                 orientation.y * inverseNorm, orientation.z * inverseNorm };
    return true;
}

SpatialVector3 AquariumPose::localToWorld(SpatialVector3 local) const noexcept
{
    const auto direction = localDirectionToWorld(local);
    return { direction.x + position.x, direction.y + position.y, direction.z + position.z };
}

SpatialVector3 AquariumPose::worldToLocal(SpatialVector3 world) const noexcept
{
    return worldDirectionToLocal(
        { world.x - position.x, world.y - position.y, world.z - position.z });
}

SpatialVector3 AquariumPose::localDirectionToWorld(SpatialVector3 local) const noexcept
{
    return rotate(local, rotation);
}

SpatialVector3 AquariumPose::worldDirectionToLocal(SpatialVector3 world) const noexcept
{
    return rotate(world, { rotation.w, -rotation.x, -rotation.y, -rotation.z });
}
}
