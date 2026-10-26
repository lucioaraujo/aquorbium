// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/BinauralSource.h"

#include <algorithm>
#include <cmath>

namespace aquorbium
{
namespace
{
constexpr float pi = 3.14159265358979323846f;
constexpr float minimumAlpha = 0.1f;
constexpr float minimumTheta = 150.0f * pi / 180.0f;
constexpr std::array<float, 5> rho { 0.5f, -1.0f, 0.5f, -0.25f, 0.25f };
constexpr std::array<float, 5> echoA { 1.0f, 5.0f, 5.0f, 5.0f, 5.0f };
constexpr std::array<float, 5> echoB { 2.0f, 4.0f, 7.0f, 11.0f, 13.0f };
constexpr std::array<float, 5> echoD { 1.0f, 0.5f, 0.5f, 0.5f, 0.5f };
}

float BinauralSource::interauralDelaySeconds(float earAngle) noexcept
{
    // Woodworth's spherical head, made non-negative by adding a / c
    const auto theta = std::clamp(earAngle, 0.0f, pi);
    const auto t = theta < 0.5f * pi ? -std::cos(theta) : theta - 0.5f * pi;
    return headRadius / soundSpeed * (t + 1.0f);
}

float BinauralSource::shadowAlpha(float earAngle) noexcept
{
    const auto theta = std::clamp(earAngle, 0.0f, pi);
    return (1.0f + 0.5f * minimumAlpha) + (1.0f - 0.5f * minimumAlpha) * std::cos(theta / minimumTheta * pi);
}

void BinauralSource::Shadow::set(float alpha, float sampleRate) noexcept
{
    // bilinear transform of (1 + alpha s / 2w0) / (1 + s / 2w0)
    const auto omega0 = soundSpeed / headRadius;
    const auto k = sampleRate / omega0;
    const auto norm = 1.0f / (k + 1.0f);
    b0 = (alpha * k + 1.0f) * norm;
    b1 = (1.0f - alpha * k) * norm;
    a1 = (1.0f - k) * norm;
}

float BinauralSource::Shadow::process(float x) noexcept
{
    const auto y = b0 * x + b1 * x1 - a1 * y1;
    x1 = x;
    y1 = y;
    return y;
}

void BinauralSource::prepare(double sampleRate) noexcept
{
    rate = std::isfinite(sampleRate) && sampleRate >= 8000.0 ? static_cast<float>(sampleRate) : 48000.0f;
    smoothing = 1.0f - std::exp(-1.0f / (0.02f * rate));  // ~20 ms
    reset();
}

void BinauralSource::reset() noexcept
{
    input.fill(0.0f);
    pinnaOut.fill(0.0f);
    write = 0;
    leftShadow.x1 = leftShadow.y1 = rightShadow.x1 = rightShadow.y1 = 0.0f;
    setDirection(0.0f, 0.0f);
    echo = echoTarget;
    leftDelay = leftDelayTarget;
    rightDelay = rightDelayTarget;
}

void BinauralSource::setDirection(float azimuth, float elevation) noexcept
{
    const auto az = std::isfinite(azimuth) ? azimuth : 0.0f;
    const auto el = std::clamp(std::isfinite(elevation) ? elevation : 0.0f, -0.5f * pi, 0.5f * pi);
    // the angle between the source and each ear's axis (the ears at +-90 degrees)
    const auto lateral = std::cos(el) * std::sin(az);  // + to the right
    const auto rightAngle = std::acos(std::clamp(lateral, -1.0f, 1.0f));
    const auto leftAngle = std::acos(std::clamp(-lateral, -1.0f, 1.0f));
    leftDelayTarget = interauralDelaySeconds(leftAngle) * rate;
    rightDelayTarget = interauralDelaySeconds(rightAngle) * rate;
    leftShadow.set(shadowAlpha(leftAngle), rate);
    rightShadow.set(shadowAlpha(rightAngle), rate);
    // the pinna's echoes (the paper's delays are in samples at 44.1 kHz)
    const auto scale = rate / 44100.0f;
    const auto front = std::cos(0.5f * std::remainder(az, 2.0f * pi));
    for (std::size_t k = 0; k < echoTarget.size(); ++k)
        echoTarget[k] = std::max(1.0f, (echoA[k] * front * std::sin(echoD[k] * (0.5f * pi - el)) + echoB[k]) * scale);
}

float BinauralSource::read(const std::array<float, lineSize>& line, std::size_t writeIndex, float delay) noexcept
{
    const auto d = std::clamp(delay, 0.0f, static_cast<float>(lineSize - 2));
    auto position = static_cast<float>(writeIndex) - d;
    if (position < 0.0f)
        position += static_cast<float>(lineSize);
    const auto i0 = static_cast<std::size_t>(position) % lineSize;
    const auto i1 = (i0 + 1) % lineSize;
    const auto fraction = position - std::floor(position);
    return line[i0] + (line[i1] - line[i0]) * fraction;
}

BinauralSource::Sample BinauralSource::tick(float x) noexcept
{
    const auto value = std::isfinite(x) ? x : 0.0f;
    input[write] = value;
    // the pinna: the direct sound and its five echoes
    auto pinna = value;
    for (std::size_t k = 0; k < echo.size(); ++k)
    {
        echo[k] += (echoTarget[k] - echo[k]) * smoothing;
        pinna += rho[k] * read(input, write, echo[k]);
    }
    pinnaOut[write] = pinna;
    // the ears: each its own delay, then the head's shadow
    leftDelay += (leftDelayTarget - leftDelay) * smoothing;
    rightDelay += (rightDelayTarget - rightDelay) * smoothing;
    Sample out;
    out.left = leftShadow.process(read(pinnaOut, write, leftDelay));
    out.right = rightShadow.process(read(pinnaOut, write, rightDelay));
    write = (write + 1) % lineSize;
    return out;
}
}
