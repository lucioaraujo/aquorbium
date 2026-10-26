// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

namespace aquorbium
{
// A single phase generator for envelopes, LFOs and audio oscillation. Mode
// changes alter lifecycle, not the underlying curve implementation.
class VectorGenerator final
{
public:
    enum class Mode
    {
        loop,
        oneShot
    };

    struct Shape
    {
        float skew = 0.5f;
        float curve = 0.0f;
        bool bipolar = false;
    };

    void prepare(double newSampleRate) noexcept;
    void reset(float phase = 0.0f) noexcept;
    void setMode(Mode value) noexcept;
    void setRateHz(float value) noexcept;
    void setShape(Shape value) noexcept;
    void trigger() noexcept;
    [[nodiscard]] float tick() noexcept;

    [[nodiscard]] bool active() const noexcept { return running; }
    [[nodiscard]] float currentPhase() const noexcept;
    [[nodiscard]] Mode mode() const noexcept { return lifecycle; }
    [[nodiscard]] Shape shape() const noexcept { return controls; }

private:
    [[nodiscard]] float evaluate(float value) const noexcept;

    Shape controls {};
    Mode lifecycle = Mode::loop;
    double sampleRate = 48000.0;
    double phase = 0.0;
    float rateHz = 1.0f;
    bool running = true;
};
}
