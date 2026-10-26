// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace aquorbium
{
// Original digital double-scroll attractor. Orbit 3 is a conceptual reference
// for the musical role (deterministic 3D chaos, complementary outputs and
// tame/wild regions), not a circuit or software implementation source.
class DoubleScrollAttractor final
{
public:
    struct Parameters
    {
        float rateHz = 0.7f;
        float separation = 0.5f;
        float distribution = 0.5f;
        float wild = 0.5f;
    };

    struct Output
    {
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
        float xComplement = 0.0f;
        float yComplement = 0.0f;
        float zComplement = 0.0f;
        bool positiveEquilibrium = false;
    };

    void prepare(double sampleRate) noexcept;
    void setSeed(std::uint32_t value) noexcept;
    void reset() noexcept;
    void setParameters(Parameters value) noexcept;
    [[nodiscard]] Parameters parameters() const noexcept { return controls; }
    [[nodiscard]] Output tick() noexcept;

private:
    struct State { double x = 0.1, y = 0.0, z = 0.0; };
    [[nodiscard]] State derivative(const State& value) const noexcept;
    [[nodiscard]] double nonlinearity(double value) const noexcept;

    Parameters controls {};
    double preparedRate = 48000.0;
    State state {};
    std::uint32_t seed = 1u;
};

class SemanticAccessRouter final
{
public:
    static constexpr std::size_t portCount = 8;
    static constexpr std::size_t routeCount = 24;

    enum class SourceKind : std::uint8_t { audio, control, event };
    enum class DestinationKind : std::uint8_t { audio, modulation, event };

    struct Port
    {
        float audio = 0.0f;
        float control = 0.0f;
        bool event = false;
    };

    struct Route
    {
        std::size_t source = 0;
        std::size_t destination = 0;
        SourceKind sourceKind = SourceKind::control;
        DestinationKind destinationKind = DestinationKind::modulation;
        float gain = 0.0f;
        float contextDepth = 0.0f;
        bool enabled = false;
    };

    struct OutputPort
    {
        float audio = 0.0f;
        float modulation = 0.0f;
        bool event = false;
    };

    using Inputs = std::array<Port, portCount>;
    using Outputs = std::array<OutputPort, portCount>;

    void prepare(double sampleRate) noexcept;
    void reset() noexcept;
    void clearRoutes() noexcept;
    [[nodiscard]] bool setRoute(std::size_t index, Route value) noexcept;
    [[nodiscard]] Route route(std::size_t index) const noexcept;
    [[nodiscard]] Outputs process(const Inputs& input, float context) noexcept;

private:
    std::array<Route, routeCount> routes {};
    std::array<float, portCount> audioEnvelope {};
    float envelopeAttack = 0.01f;
    float envelopeRelease = 0.001f;
};

class OpenRuleProgram final
{
public:
    static constexpr std::size_t maximumRules = 16;
    static constexpr std::size_t targetCount = 4;

    enum class Source : std::uint8_t
    {
        density, danger, day, season, crack, chaos
    };
    enum class Comparison : std::uint8_t { less, greater };
    enum class Target : std::uint8_t
    {
        branch, evolution, gesture, integrity
    };

    struct Inputs
    {
        float density = 0.0f;
        float danger = 0.0f;
        float day = 0.0f;
        float season = 0.0f;
        float crack = 0.0f;
        float chaos = 0.0f;
    };

    struct Result
    {
        std::array<float, targetCount> value {};
        std::array<bool, targetCount> active {};
        std::size_t matches = 0;
    };

    // Grammar: source ('<'|'>') number ':' target '=' number, separated by ';'.
    // Compilation is transactional and belongs outside the audio callback.
    [[nodiscard]] bool compile(std::string_view text) noexcept;
    void clear() noexcept;
    [[nodiscard]] Result evaluate(Inputs input) const noexcept;
    [[nodiscard]] std::size_t size() const noexcept { return ruleSize; }

private:
    struct Rule
    {
        Source source = Source::density;
        Comparison comparison = Comparison::greater;
        float threshold = 0.5f;
        Target target = Target::branch;
        float value = 0.0f;
    };

    std::array<Rule, maximumRules> rules {};
    std::size_t ruleSize = 0;
};

class ContactCrackleExciter final
{
public:
    struct Parameters
    {
        float amount = 0.5f;
        float sensitivity = 0.5f;
        float colour = 0.5f;
        float decay = 0.5f;
    };

    struct Contact
    {
        float x = 0.0f;
        float y = 0.0f;
        float pressure = 0.0f;
        float velocity = 0.0f;
        std::uint32_t entropy = 0;
    };

    struct Output { float left = 0.0f; float right = 0.0f; };

    void prepare(double sampleRate) noexcept;
    void setSeed(std::uint32_t value) noexcept;
    void reset() noexcept;
    void setParameters(Parameters value) noexcept;
    [[nodiscard]] Parameters parameters() const noexcept { return controls; }
    [[nodiscard]] bool trigger(Contact value) noexcept;
    [[nodiscard]] Output tick() noexcept;
    [[nodiscard]] float energy() const noexcept { return envelope; }

private:
    [[nodiscard]] float noise() noexcept;

    Parameters controls {};
    double preparedRate = 48000.0;
    float envelope = 0.0f;
    float envelopeDecay = 0.999f;
    float impulse = 0.0f;
    float impulseDecay = 0.95f;
    float phase = 0.0f;
    float frequency = 1200.0f;
    float pan = 0.0f;
    std::uint32_t seed = 1u;
    std::uint32_t randomState = 1u;
};
}
