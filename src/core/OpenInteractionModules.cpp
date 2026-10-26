// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/OpenInteractionModules.h"

#include "core/Numeric.h"

#include <algorithm>
#include <charconv>
#include <cmath>
#include <limits>

namespace aquorbium
{
namespace
{
constexpr float twoPi = 6.28318530717958647692f;

float safeBipolar(float value) noexcept
{
    return numeric::clampFinite(value, -1.0f, 1.0f, 0.0f);
}

void skipSpaces(std::string_view text, std::size_t& position) noexcept
{
    while (position < text.size()
           && (text[position] == ' ' || text[position] == '\t'
               || text[position] == '\n' || text[position] == '\r'))
        ++position;
}

std::string_view identifier(std::string_view text,
                            std::size_t& position) noexcept
{
    skipSpaces(text, position);
    const auto start = position;
    while (position < text.size())
    {
        const auto character = text[position];
        if (!((character >= 'a' && character <= 'z') || character == '_'))
            break;
        ++position;
    }
    return text.substr(start, position - start);
}

bool number(std::string_view text, std::size_t& position, float& value) noexcept
{
    skipSpaces(text, position);
    const auto* begin = text.data() + position;
    const auto* end = text.data() + text.size();
    const auto parsed = std::from_chars(begin, end, value);
    if (parsed.ec != std::errc {} || parsed.ptr == begin
        || !std::isfinite(value))
        return false;
    position = static_cast<std::size_t>(parsed.ptr - text.data());
    return true;
}
}

void DoubleScrollAttractor::prepare(double sampleRate) noexcept
{
    preparedRate = numeric::sampleRate(sampleRate);
    setParameters(controls);
    reset();
}

void DoubleScrollAttractor::setSeed(std::uint32_t value) noexcept
{
    seed = value == 0u ? 1u : value;
    reset();
}

void DoubleScrollAttractor::reset() noexcept
{
    auto random = seed;
    random ^= random << 13u;
    random ^= random >> 17u;
    random ^= random << 5u;
    const auto offset = static_cast<double>(random) / 4294967295.0 * 0.08;
    state = { 0.08 + offset, 0.005 - offset * 0.25, -0.01 + offset * 0.5 };
}

void DoubleScrollAttractor::setParameters(Parameters value) noexcept
{
    controls.rateHz = numeric::clampFinite(value.rateHz, 0.002f, 80.0f,
                                            controls.rateHz);
    controls.separation = numeric::unit(value.separation, controls.separation);
    controls.distribution = numeric::unit(value.distribution,
                                           controls.distribution);
    controls.wild = numeric::unit(value.wild, controls.wild);
}

double DoubleScrollAttractor::nonlinearity(double value) const noexcept
{
    const auto breakpoint = 0.72 + static_cast<double>(controls.separation)
        * 0.72;
    const auto innerSlope = -1.04 - static_cast<double>(controls.wild) * 0.17;
    const auto outerSlope = -0.62
        - static_cast<double>(controls.distribution) * 0.18;
    return outerSlope * value + 0.5 * (innerSlope - outerSlope)
        * (std::abs(value + breakpoint) - std::abs(value - breakpoint));
}

DoubleScrollAttractor::State DoubleScrollAttractor::derivative(
    const State& value) const noexcept
{
    const auto alpha = 10.0 + static_cast<double>(controls.wild) * 6.0;
    const auto beta = 14.0 + static_cast<double>(controls.wild) * 14.0;
    return {
        alpha * (value.y - value.x - nonlinearity(value.x)),
        value.x - value.y + value.z,
        -beta * value.y
    };
}

DoubleScrollAttractor::Output DoubleScrollAttractor::tick() noexcept
{
    const auto step = static_cast<double>(controls.rateHz) * 6.0 / preparedRate;
    const auto first = derivative(state);
    const State midpoint {
        state.x + first.x * step * 0.5,
        state.y + first.y * step * 0.5,
        state.z + first.z * step * 0.5
    };
    const auto second = derivative(midpoint);
    state.x += second.x * step;
    state.y += second.y * step;
    state.z += second.z * step;
    if (!std::isfinite(state.x) || !std::isfinite(state.y)
        || !std::isfinite(state.z) || std::abs(state.x) > 12.0
        || std::abs(state.y) > 12.0 || std::abs(state.z) > 12.0)
        reset();
    const auto x = static_cast<float>(std::tanh(state.x * 0.55));
    const auto y = static_cast<float>(std::tanh(state.y * 1.10));
    const auto z = static_cast<float>(std::tanh(state.z * 0.22));
    return { x, y, z, -x, -y, -z, state.x >= 0.0 };
}

void SemanticAccessRouter::prepare(double sampleRate) noexcept
{
    const auto rate = numeric::sampleRate(sampleRate);
    envelopeAttack = static_cast<float>(-std::expm1(-1.0 / (rate * 0.006)));
    envelopeRelease = static_cast<float>(-std::expm1(-1.0 / (rate * 0.090)));
    reset();
}

void SemanticAccessRouter::reset() noexcept
{
    audioEnvelope.fill(0.0f);
}

void SemanticAccessRouter::clearRoutes() noexcept
{
    routes = {};
}

bool SemanticAccessRouter::setRoute(std::size_t index, Route value) noexcept
{
    if (index >= routes.size() || value.source >= portCount
        || value.destination >= portCount
        || static_cast<unsigned>(value.sourceKind)
            > static_cast<unsigned>(SourceKind::event)
        || static_cast<unsigned>(value.destinationKind)
            > static_cast<unsigned>(DestinationKind::event))
        return false;
    value.gain = numeric::clampFinite(value.gain, -1.0f, 1.0f, 0.0f);
    value.contextDepth = numeric::clampFinite(value.contextDepth,
                                               -1.0f, 1.0f, 0.0f);
    routes[index] = value;
    return true;
}

SemanticAccessRouter::Route SemanticAccessRouter::route(
    std::size_t index) const noexcept
{
    return index < routes.size() ? routes[index] : Route {};
}

SemanticAccessRouter::Outputs SemanticAccessRouter::process(
    const Inputs& input, float context) noexcept
{
    const auto safeContext = numeric::unit(context, 0.5f) * 2.0f - 1.0f;
    for (std::size_t port = 0; port < input.size(); ++port)
    {
        const auto magnitude = std::abs(safeBipolar(input[port].audio));
        audioEnvelope[port] += (magnitude - audioEnvelope[port])
            * (magnitude > audioEnvelope[port]
                ? envelopeAttack : envelopeRelease);
    }
    Outputs output {};
    for (const auto& current : routes)
    {
        if (!current.enabled) continue;
        const auto& source = input[current.source];
        auto value = 0.0f;
        switch (current.sourceKind)
        {
            case SourceKind::audio:
                value = current.destinationKind == DestinationKind::modulation
                    ? audioEnvelope[current.source]
                    : safeBipolar(source.audio);
                break;
            case SourceKind::control:
                value = safeBipolar(source.control);
                break;
            case SourceKind::event:
                value = source.event ? 1.0f : 0.0f;
                break;
        }
        const auto gain = std::clamp(current.gain
            + current.contextDepth * safeContext, -1.0f, 1.0f);
        auto& destination = output[current.destination];
        switch (current.destinationKind)
        {
            case DestinationKind::audio:
                destination.audio = std::clamp(destination.audio
                    + value * gain, -1.0f, 1.0f);
                break;
            case DestinationKind::modulation:
                destination.modulation = std::clamp(destination.modulation
                    + value * gain, -1.0f, 1.0f);
                break;
            case DestinationKind::event:
                destination.event = destination.event
                    || std::abs(value * gain) >= 0.5f;
                break;
        }
    }
    return output;
}

void OpenRuleProgram::clear() noexcept
{
    rules = {};
    ruleSize = 0;
}

bool OpenRuleProgram::compile(std::string_view text) noexcept
{
    std::array<Rule, maximumRules> parsedRules {};
    std::size_t parsedSize = 0;
    std::size_t position = 0;
    while (true)
    {
        skipSpaces(text, position);
        while (position < text.size() && text[position] == ';')
        {
            ++position;
            skipSpaces(text, position);
        }
        if (position == text.size()) break;
        if (parsedSize >= maximumRules) return false;

        Rule rule;
        const auto sourceName = identifier(text, position);
        if (sourceName == "density") rule.source = Source::density;
        else if (sourceName == "danger") rule.source = Source::danger;
        else if (sourceName == "day") rule.source = Source::day;
        else if (sourceName == "season") rule.source = Source::season;
        else if (sourceName == "crack") rule.source = Source::crack;
        else if (sourceName == "chaos") rule.source = Source::chaos;
        else return false;

        skipSpaces(text, position);
        if (position >= text.size()
            || (text[position] != '<' && text[position] != '>')) return false;
        rule.comparison = text[position++] == '<'
            ? Comparison::less : Comparison::greater;
        if (!number(text, position, rule.threshold)) return false;
        rule.threshold = numeric::unit(rule.threshold, 0.5f);
        skipSpaces(text, position);
        if (position >= text.size() || text[position++] != ':') return false;

        const auto targetName = identifier(text, position);
        if (targetName == "branch") rule.target = Target::branch;
        else if (targetName == "evolution") rule.target = Target::evolution;
        else if (targetName == "gesture") rule.target = Target::gesture;
        else if (targetName == "integrity") rule.target = Target::integrity;
        else return false;
        skipSpaces(text, position);
        if (position >= text.size() || text[position++] != '=') return false;
        if (!number(text, position, rule.value)) return false;
        rule.value = numeric::unit(rule.value, 0.0f);
        skipSpaces(text, position);
        if (position < text.size() && text[position] != ';') return false;
        parsedRules[parsedSize++] = rule;
    }
    if (parsedSize == 0) return false;
    rules = parsedRules;
    ruleSize = parsedSize;
    return true;
}

OpenRuleProgram::Result OpenRuleProgram::evaluate(Inputs input) const noexcept
{
    input.density = numeric::unit(input.density, 0.0f);
    input.danger = numeric::unit(input.danger, 0.0f);
    input.day = numeric::unit(input.day, 0.0f);
    input.season = numeric::unit(input.season, 0.0f);
    input.crack = numeric::unit(input.crack, 0.0f);
    input.chaos = numeric::unit(input.chaos, 0.0f);
    const std::array<float, 6> source {
        input.density, input.danger, input.day,
        input.season, input.crack, input.chaos
    };
    Result result;
    for (std::size_t index = 0; index < ruleSize; ++index)
    {
        const auto& rule = rules[index];
        const auto actual = source[static_cast<std::size_t>(rule.source)];
        const auto matches = rule.comparison == Comparison::less
            ? actual < rule.threshold : actual > rule.threshold;
        if (!matches) continue;
        const auto target = static_cast<std::size_t>(rule.target);
        result.value[target] = rule.value;
        result.active[target] = true;
        ++result.matches;
    }
    return result;
}

void ContactCrackleExciter::prepare(double sampleRate) noexcept
{
    preparedRate = numeric::sampleRate(sampleRate);
    setParameters(controls);
    reset();
}

void ContactCrackleExciter::setSeed(std::uint32_t value) noexcept
{
    seed = value == 0u ? 1u : value;
    reset();
}

void ContactCrackleExciter::reset() noexcept
{
    envelope = impulse = phase = pan = 0.0f;
    frequency = 1200.0f;
    randomState = seed;
}

void ContactCrackleExciter::setParameters(Parameters value) noexcept
{
    controls.amount = numeric::unit(value.amount, controls.amount);
    controls.sensitivity = numeric::unit(value.sensitivity,
                                          controls.sensitivity);
    controls.colour = numeric::unit(value.colour, controls.colour);
    controls.decay = numeric::unit(value.decay, controls.decay);
    envelopeDecay = static_cast<float>(std::exp(-1.0
        / (preparedRate * (0.018 + controls.decay * 0.420))));
    impulseDecay = static_cast<float>(std::exp(-1.0
        / (preparedRate * (0.001 + controls.colour * 0.022))));
}

bool ContactCrackleExciter::trigger(Contact value) noexcept
{
    value.x = safeBipolar(value.x);
    value.y = safeBipolar(value.y);
    value.pressure = numeric::unit(value.pressure, 0.0f);
    value.velocity = numeric::unit(value.velocity, 0.0f);
    const auto strength = value.pressure
        * (0.25f + controls.sensitivity * 0.75f);
    if (strength <= 0.0001f) return false;
    randomState ^= value.entropy + 0x9e3779b9u
        + (randomState << 6u) + (randomState >> 2u);
    if (randomState == 0u) randomState = seed;
    envelope = std::max(envelope, strength);
    impulse = std::max(impulse, value.velocity * 0.8f + strength * 0.2f);
    pan = value.x;
    const auto separation = std::abs(value.x - value.y) * 0.5f;
    frequency = std::clamp(180.0f + controls.colour * 6200.0f
        + separation * 3600.0f, 40.0f,
        static_cast<float>(preparedRate * 0.42));
    return true;
}

float ContactCrackleExciter::noise() noexcept
{
    auto value = randomState;
    value ^= value << 13u;
    value ^= value >> 17u;
    value ^= value << 5u;
    randomState = value == 0u ? seed : value;
    return static_cast<float>(randomState) / 4294967295.0f * 2.0f - 1.0f;
}

ContactCrackleExciter::Output ContactCrackleExciter::tick() noexcept
{
    phase += frequency / static_cast<float>(preparedRate);
    phase -= std::floor(phase);
    const auto random = noise();
    const auto threshold = 0.985f - controls.colour * 0.22f
        - envelope * 0.10f;
    if (std::abs(random) > threshold)
        impulse = std::max(impulse, std::abs(random));
    const auto body = std::sin(twoPi * phase) * 0.24f;
    const auto signal = std::tanh((body + random * impulse)
        * envelope * controls.amount * 2.4f);
    const auto leftGain = std::sqrt((1.0f - pan) * 0.5f);
    const auto rightGain = std::sqrt((1.0f + pan) * 0.5f);
    envelope *= envelopeDecay;
    impulse *= impulseDecay;
    if (envelope < 1.0e-8f) envelope = 0.0f;
    if (impulse < 1.0e-8f) impulse = 0.0f;
    return { signal * leftGain, signal * rightGain };
}
}
