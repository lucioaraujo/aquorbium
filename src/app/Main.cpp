// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include <JuceHeader.h>
#include "BinaryData.h"
#include "AudioControlFrame.h"
#include "PerformancePresetBank.h"
#include "PerformanceSceneFile.h"
#include "PerformanceSceneControls.h"
#include "EcologyPanelLayout.h"
#include "MacroPanelLayout.h"
#include "OrganismLabelLayout.h"
#include "RuntimeAuditProtocol.h"
#include "AmbisonicCapture.h"
#include "RealtimeAudioThread.h"
#include "DiagnosticAudioCapture.h"
#include "EcologyParameterState.h"
#include "PerformanceParameterState.h"
#include "WindowLayoutPolicy.h"
#include "UiLanguage.h"
#include "LearnCatalog.h"
#include "Tutorial.h"
#include "core/BiomaEngine.h"
#include "core/ControlMotion.h"
#include "core/OpeningSeed.h"
#include "core/DeliveryProfiles.h"
#include "core/FixedMpscQueue.h"
#include "core/FixedLatestSnapshot.h"
#include "core/GranularPerformance.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <functional>
#include <mutex>
#include <utility>
#include <vector>

static_assert(std::atomic<float>::is_always_lock_free,
              "Aquorbium controls require lock-free float atomics");
static_assert(std::atomic<std::uint32_t>::is_always_lock_free,
              "Aquorbium status requires lock-free counter atomics");
static_assert(std::atomic<bool>::is_always_lock_free,
              "Aquorbium switches require lock-free bool atomics");
static_assert(std::atomic<aquorbium::app::DiagnosticAudioCapture*>::is_always_lock_free,
              "Aquorbium capture requires lock-free pointer atomics");

namespace
{
namespace Palette
{
const auto background = juce::Colour::fromRGB(5, 13, 18);
const auto panel = juce::Colour::fromRGB(10, 25, 31);
const auto panelRaised = juce::Colour::fromRGB(14, 34, 41);
const auto line = juce::Colour::fromRGB(47, 79, 86);
const auto text = juce::Colour::fromRGB(223, 235, 230);
const auto muted = juce::Colour::fromRGB(119, 148, 149);
const auto cyan = juce::Colour::fromRGB(80, 222, 213);
const auto amber = juce::Colour::fromRGB(243, 183, 91);
const auto coral = juce::Colour::fromRGB(239, 113, 95);
const auto violet = juce::Colour::fromRGB(158, 132, 244);
const auto green = juce::Colour::fromRGB(119, 210, 142);
}

juce::Font uiFont(float height, int style = juce::Font::plain)
{
    return juce::Font { juce::FontOptions { height, style } };
}

juce::String utf8(const char* text)
{
    return juce::String::fromUTF8(text);
}

bool needsPipeWireSafetyBuffer(const juce::String& outputName, int frames) noexcept
{
    // The ALSA/PipeWire route advertises very small periods (including 16),
    // but the interactive run reproduced XRUN/LATE with a 16-frame period.
    // Keep direct hardware and the dedicated audit routes untouched.
    return outputName.containsIgnoreCase("PipeWire") && frames > 0 && frames < 512;
}

struct ModuleBankDescriptor
{
    const char* id;
    const char* label;
    const char* detail;
};

constexpr std::array<ModuleBankDescriptor,
                     aquorbium::IntegratedModuleBank::controlCount> moduleBankDescriptors {{
    { "moduleBankMix", "LAB MIX", "Bypass mestre do banco: zero preserva exatamente o instrumento anterior" },
    { "moduleSynthBodies", "FM + HARMONICS", "FM, harmônicos, BiomaBass, cardume, vocoder e contágio" },
    { "moduleGranularMemory", "GRANULAR CAPTURE", "Granular, isolador, resíntese recente e foco lock-in" },
    { "modulePercussionPulses", "PERC + PULSE", "Percussão, máscara estocástica e HarmonicRippleVoice" },
    { "moduleFractalVector", "FRACTAL + VECTOR", "Fractal, vetor, polígono X/Y e FrequencyShifter" },
    { "moduleLorenzDrift", "LORENZ + DRIFT", "Lorenz, derivas e auto-feedback limitado" },
    { "moduleFluidPhase", "FLUID + PHASE", "Movimento, quadratura, inércia e binaural leve" },
    { "moduleGestureWaves", "GESTURE WAVES", "MotionGestureRecorder e banco de ondas de movimento" },
    { "moduleClockEvents", "CLOCK + EVENTS", "Relógio, eventos, ressonadores e L-system" },
    { "moduleTransposeSelect", "TRANSPOSE + SELECT", "AccumulatingTransposer e ThresholdStepSelector" },
    { "moduleBranchPermute", "BRANCH + PERMUTE", "Ramificação, limiares reativos, matriz e Spotlight" },
    { "moduleRotateLayout", "ROTATE + LAYOUT", "Rotação, layout e pools compartilhados/reconfiguráveis" },
    { "modulePatternLoop", "PATTERN + LOOP", "Bancos, RandomToLoop e autômato celular" },
    { "moduleShiftProbability", "SHIFT + PROB", "ShiftRegisterMemory e ProbabilityDistribution" },
    { "moduleEvolutionStages", "EVOLVE + STAGES", "Evolução, cross-feedback, limiares e cenas comportamentais" },
    { "moduleIntegrityMemory", "INTEGRITY + MEMORY", "Integridade, memória, fome e barramento sem entrada" }
}};

// The controls as the instruments of a submarine's control room (2026-10-20,
// the author: "pesquisa sobre painel de controles de submarinos. traga
// elementos visuais nos nossos controles baseados neles"). Each knob is a
// gauge: a metal bezel, a dark face with a graduated scale, a needle with a
// luminous tip, as on the depth gauges of a control room (black dials, light
// numerals, needles readable in low light); the value arc keeps the colour of
// its card. Buttons are panel switches whose edge lights when engaged.
class InstrumentLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float position,
                          float startAngle, float endAngle, juce::Slider& slider) override
    {
        const auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(10.0f);
        const auto radius = std::min(bounds.getWidth(), bounds.getHeight()) * 0.5f;
        const auto centre = bounds.getCentre();
        const auto enabled = slider.isEnabled();
        const auto tint = slider.findColour(juce::Slider::rotarySliderFillColourId);

        // the bezel: a ring of brushed metal
        const auto outer = juce::Rectangle<float>(radius * 2.0f, radius * 2.0f).withCentre(centre);
        juce::ColourGradient metal(Palette::text.withAlpha(0.55f), outer.getX(), outer.getY(),
                                   Palette::line.withAlpha(0.65f), outer.getRight(), outer.getBottom(), false);
        g.setGradientFill(metal);
        g.fillEllipse(outer);
        // the face
        const auto face = outer.reduced(radius * 0.11f);
        juce::ColourGradient deep(Palette::panelRaised, face.getCentreX(), face.getY(),
                                  Palette::background, face.getCentreX(), face.getBottom(), false);
        g.setGradientFill(deep);
        g.fillEllipse(face);
        const auto faceRadius = face.getWidth() * 0.5f;

        // the graduated scale: minor ticks, a major one every fifth
        constexpr int ticks = 20;
        for (int t = 0; t <= ticks; ++t)
        {
            const auto angle = startAngle + (endAngle - startAngle) * static_cast<float>(t) / ticks;
            const auto major = t % 5 == 0;
            const auto r1 = faceRadius * (major ? 0.70f : 0.80f);
            const auto r2 = faceRadius * 0.92f;
            const auto dx = std::sin(angle), dy = -std::cos(angle);
            g.setColour((major ? Palette::text : Palette::muted).withAlpha(enabled ? (major ? 0.85f : 0.55f) : 0.30f));
            g.drawLine(centre.x + dx * r1, centre.y + dy * r1, centre.x + dx * r2, centre.y + dy * r2,
                       major ? 1.4f : 0.8f);
        }
        // the value, an arc in the colour of the card, inside the bezel
        const auto valueAngle = startAngle + (endAngle - startAngle) * position;
        juce::Path arc;
        arc.addCentredArc(centre.x, centre.y, faceRadius * 0.98f, faceRadius * 0.98f, 0.0f, startAngle, valueAngle, true);
        g.setColour(enabled ? tint.withAlpha(0.95f) : Palette::muted.withAlpha(0.35f));
        g.strokePath(arc, juce::PathStrokeType(2.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // the needle, with a luminous tip, over a metal hub
        const auto dx = std::sin(valueAngle), dy = -std::cos(valueAngle);
        const auto tip = juce::Point<float>(centre.x + dx * faceRadius * 0.78f, centre.y + dy * faceRadius * 0.78f);
        const auto tail = juce::Point<float>(centre.x - dx * faceRadius * 0.16f, centre.y - dy * faceRadius * 0.16f);
        g.setColour(Palette::text.withAlpha(enabled ? 0.95f : 0.45f));
        g.drawLine({ tail, tip }, 2.0f);
        if (enabled)
        {
            g.setColour(tint.withAlpha(0.35f));
            g.fillEllipse(juce::Rectangle<float>(9.0f, 9.0f).withCentre(tip));
            g.setColour(tint);
            g.fillEllipse(juce::Rectangle<float>(4.5f, 4.5f).withCentre(tip));
        }
        const auto hub = juce::Rectangle<float>(faceRadius * 0.26f, faceRadius * 0.26f).withCentre(centre);
        g.setGradientFill(metal);
        g.fillEllipse(hub);
        g.setColour(Palette::background.withAlpha(0.8f));
        g.drawEllipse(hub, 0.8f);
    }

    void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour&,
                              bool highlighted, bool down) override
    {
        // a panel switch: a dark plate with a bevel; engaged, its edge lights
        auto bounds = button.getLocalBounds().toFloat().reduced(0.5f);
        const auto engaged = button.getToggleState();
        juce::ColourGradient plate(Palette::panelRaised.brighter(highlighted ? 0.18f : 0.08f), bounds.getX(), bounds.getY(),
                                   Palette::background, bounds.getX(), bounds.getBottom(), false);
        g.setGradientFill(plate);
        g.fillRoundedRectangle(bounds, 5.0f);
        if (engaged)
        {
            g.setColour(Palette::amber.withAlpha(0.18f));
            g.fillRoundedRectangle(bounds, 5.0f);
        }
        g.setColour(engaged ? Palette::amber.withAlpha(0.9f) : Palette::line.withAlpha(down ? 1.0f : 0.8f));
        g.drawRoundedRectangle(bounds, 5.0f, engaged ? 1.6f : 1.0f);
        g.setColour(Palette::text.withAlpha(0.10f));
        g.drawHorizontalLine(static_cast<int>(bounds.getY()) + 1, bounds.getX() + 4.0f, bounds.getRight() - 4.0f);
    }
};

class ModuleCard final : public juce::Component, public juce::SettableTooltipClient
{
public:
    ModuleCard(juce::String numberIn, juce::String titleIn,
               juce::String statusIn, juce::String functionIn,
               juce::String detailIn, juce::Colour accentIn,
               bool embeddedControlsIn = false)
        : number(std::move(numberIn)), title(std::move(titleIn)),
          status(std::move(statusIn)), function(std::move(functionIn)),
          detail(std::move(detailIn)), accent(accentIn),
          embeddedControls(embeddedControlsIn)
    {
        setInterceptsMouseClicks(false, false);
    }

    void paint(juce::Graphics& g) override
    {
        const auto bounds = getLocalBounds().toFloat().reduced(0.75f);
        const auto condensed = getHeight() < 145;
        g.setColour(Palette::panel);
        g.fillRoundedRectangle(bounds, 10.0f);
        g.setColour(accent.withAlpha(0.43f));
        g.drawRoundedRectangle(bounds, 10.0f, 1.25f);

        if (compactHeader)
        {
            // the wide surface: a one-line header, as an engraved nameplate
            // screwed to the panel, with the card's state as an annunciator
            // lamp (the description becomes a tooltip)
            auto plateArea = getLocalBounds().removeFromTop(40).reduced(7, 6).toFloat();
            drawNameplate(g, plateArea);
            auto line = plateArea.reduced(16.0f, 0.0f).toNearestInt();
            g.setFont(uiFont(10.0f, juce::Font::bold));
            g.setColour(accent);
            g.drawText(number, line.removeFromLeft(22), juce::Justification::centredLeft);
            const auto pillWidth = std::min(line.getWidth() / 2,
                static_cast<int>(status.length()) * 7 + 18);
            auto pill = line.removeFromRight(pillWidth).reduced(0, 3);
            g.setFont(uiFont(13.0f, juce::Font::bold));
            g.setColour(Palette::background);
            g.drawFittedText(title, line.translated(0, 1), juce::Justification::centredLeft, 1);
            g.setColour(Palette::text);
            g.drawFittedText(title, line, juce::Justification::centredLeft, 1);
            drawLamp(g, pill.toFloat(), 8.5f);
            return;
        }

        auto content = getLocalBounds().reduced(condensed ? 10 : 15,
                                                condensed ? 8 : 13);
        auto heading = content.removeFromTop(condensed ? 18 : 23);
        g.setFont(uiFont(condensed ? 9.5f : 11.0f, juce::Font::bold));
        g.setColour(accent);
        g.drawText(number, heading.removeFromLeft(condensed ? 23 : 28),
                   juce::Justification::centredLeft);
        g.setFont(uiFont(condensed ? 12.0f : 14.0f, juce::Font::bold));
        g.setColour(Palette::text);
        g.drawFittedText(title, heading, juce::Justification::centredLeft, 1);

        content.removeFromTop(condensed ? 5 : 9);
        auto pill = content.removeFromTop(condensed ? 17 : 20).removeFromLeft(
            std::max(condensed ? 58 : 68,
                     static_cast<int>(status.length()) * (condensed ? 7 : 8)
                         + (condensed ? 14 : 18)));
        drawLamp(g, pill.toFloat(), condensed ? 8.5f : 9.5f);

        content.removeFromTop(condensed ? 5 : 10);
        g.setColour(Palette::text.withAlpha(0.88f));
        g.setFont(uiFont(condensed ? 10.0f : 11.5f, juce::Font::bold));
        if (!embeddedControls && content.getHeight() > 12)
            g.drawFittedText(function, content.removeFromTop(condensed ? 22 : 28),
                             juce::Justification::topLeft, condensed ? 1 : 2);
        if (!embeddedControls && !condensed && content.getHeight() > 12)
        {
            content.removeFromTop(4);
            g.setColour(Palette::muted);
            g.setFont(uiFont(10.5f));
            g.drawFittedText(detail, content, juce::Justification::topLeft, 3);
        }
    }

    // an annunciator lamp: lit (with its glow) when the card acts, dark in bypass
    void setLit(bool value)
    {
        if (lit != value)
        {
            lit = value;
            repaint();
        }
    }

    void drawLamp(juce::Graphics& g, juce::Rectangle<float> lamp, float fontSize) const
    {
        if (lit)
        {
            g.setColour(accent.withAlpha(0.14f));
            g.fillRoundedRectangle(lamp.expanded(3.0f, 2.0f), 6.0f);
            juce::ColourGradient glow(accent.withAlpha(0.55f), lamp.getCentreX(), lamp.getY(),
                                      accent.withAlpha(0.22f), lamp.getCentreX(), lamp.getBottom(), false);
            g.setGradientFill(glow);
        }
        else
            g.setColour(Palette::background.withAlpha(0.85f));
        g.fillRoundedRectangle(lamp, 3.0f);
        g.setColour(lit ? accent : Palette::line.withAlpha(0.9f));
        g.drawRoundedRectangle(lamp, 3.0f, 1.0f);
        g.setColour(lit ? Palette::text : Palette::muted.withAlpha(0.8f));
        g.setFont(uiFont(fontSize, juce::Font::bold));
        g.drawFittedText(status, lamp.toNearestInt().reduced(4, 0), juce::Justification::centred, 1);
    }

    static void drawNameplate(juce::Graphics& g, juce::Rectangle<float> plate)
    {
        // a dark plate with an engraved edge and a screw at each end
        g.setColour(Palette::background.withAlpha(0.70f));
        g.fillRoundedRectangle(plate, 4.0f);
        g.setColour(Palette::line.withAlpha(0.75f));
        g.drawRoundedRectangle(plate, 4.0f, 1.0f);
        g.setColour(Palette::text.withAlpha(0.07f));
        g.drawHorizontalLine(static_cast<int>(plate.getY()) + 1, plate.getX() + 3.0f, plate.getRight() - 3.0f);
        for (const auto x : { plate.getX() + 7.0f, plate.getRight() - 7.0f })
        {
            const auto screw = juce::Rectangle<float>(5.0f, 5.0f).withCentre({ x, plate.getCentreY() });
            g.setColour(Palette::muted.withAlpha(0.55f));
            g.fillEllipse(screw);
            g.setColour(Palette::background);
            g.drawLine(screw.getX() + 1.0f, screw.getBottom() - 1.0f, screw.getRight() - 1.0f, screw.getY() + 1.0f, 0.8f);
        }
    }

    void setTexts(juce::String titleIn, juce::String functionIn, juce::String detailIn)
    {
        title = std::move(titleIn);
        function = std::move(functionIn);
        detail = std::move(detailIn);
        if (compactHeader)
            setTooltip(function + " - " + detail);
        repaint();
    }

    void setCompactHeader(bool value)
    {
        if (compactHeader != value)
        {
            compactHeader = value;
            setTooltip(value ? function + " - " + detail : juce::String());
            repaint();
        }
    }

    void setStatus(juce::String next)
    {
        if (status != next)
        {
            status = std::move(next);
            repaint();
        }
    }

private:
    juce::String number;
    juce::String title;
    juce::String status;
    juce::String function;
    juce::String detail;
    juce::Colour accent;
    bool embeddedControls = false;
    bool compactHeader = false;
    bool lit = true;
};

struct VisualFrame
{
    struct Organism
    {
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
        float energy = 0.5f;
        float autonomousEnergy = 0.0f;
        float autonomousPitch = 36.0f;
        int orbitStep = 0;
        int orbitLength = 1;
        int activeParticles = 0;
        bool geometricGate = false;
        float observation = 0.0f;
        float detail = 0.0f;
        float spatialPan = 0.0f;
        float spatialDepth = 0.0f;
    };

    std::array<Organism, aquorbium::Ecosystem::organismCount> organisms {};
    float ecosystemBlend = 1.0f;
    float outputEnergy = 0.0f;
    float momentaryLufs = aquorbium::LoudnessMeter::silenceLufs;
    // the master's true peak (4x, since the session started), gain reduction
    // and stereo correlation (2026-10-22)
    float outputTruePeak = 0.0f;
    float gainReductionDb = 0.0f;
    float correlation = 0.0f;
    float shortTermLufs = aquorbium::LoudnessMeter::silenceLufs;
    float integratedLufs = aquorbium::LoudnessMeter::silenceLufs;
    // the output's feedback taps, envelopes 0..1 (MEDIDORES, 2026-10-24)
    float tapPreMaster = 0.0f;
    float tapPreSafety = 0.0f;
    float tapSafetyReduction = 0.0f;
    float tapPostSafety = 0.0f;
    // the CÉREBRO and SALA cards (2026-10-24)
    int brainRegister = 0;
    float brainMood = 0.0f;
    bool roomEnabled = false;
    bool roomActive = false;
    float roomLevelDb = -120.0f;
    float roomStrength = 0.0f;
    float roomCoupling = 0.0f;
    std::uint64_t loudnessBlocks = 0;
    bool momentaryLoudnessReady = false;
    bool shortTermLoudnessReady = false;
    float daylight = 0.5f;
    float temperature = 0.5f;
    float integrity = 1.0f;
    float lensX = 1.0f;
    float lensY = 0.0f;
    float lensZ = 0.0f;
    float lensTargetX = 1.0f;
    float lensTargetY = 0.0f;
    float lensTargetZ = 0.0f;
    bool lensHeld = false;
    int lensFocusedOrganisms = 0;
    float autonomousEnergy = 0.0f;
    bool autonomousEnabled = false;
    int autonomousOrganisms = 0;
    int harmonicRoot = 36;
    std::uint32_t autonomousEvents = 0;
    int activeVoices = 0;
    std::uint64_t droppedEvents = 0;
    int musicPhase = 2;        // EcologicalMusicDirector::Phase
    float musicTension = 0.0f;
    float formTension = 0.5f;  // the session's arc (AutonomousEnergy), for the VARIA
    int musicAct = 0;          // EcologicalMusicDirector::Act
    float musicInfluence = 0.0f;
    float musicLatent = 0.0f;
    float musicCapacity = 0.0f;
    float musicChannelAxis = 0.0f;  // height of the deep sound channel
    float musicHabitat = -1.0f;     // the habitat's health, heard (-1 not yet measured)
    float musicRoomDb = 1.0f;       // the room's excess in dBFS (> 0: not heard)
    float musicPrey = 1.0f;       // Lotka-Volterra abundances, 1 = equilibrium
    float musicPredators = 1.0f;
    int musicEvents = 0;
    int musicCentre = 0;
    int musicMode = 3;
    float musicTempo = 0.0f;
    std::array<std::uint8_t, 25> relation {};  // [a * 5 + b]
    std::array<std::int8_t, 8> soundmark {};   // the place's motif (degrees)
    std::array<std::uint8_t, 8> soundmarkSteps {};
    int soundmarkLength = 0;
    bool soundmarkChosen = false;
    std::array<float, 25> bond {};
    std::array<std::uint32_t, aquorbium::EcologicalMusicDirector::actCount> actCounts {};
};

// The audio thread publishes only lock-free scalar atomics. The GUI creates its
// own value snapshot, so neither thread reads mutable state owned by the other.
class VisualTelemetryBridge final
{
public:
    void publish(const aquorbium::BiomaEngine::Telemetry& source) noexcept
    {
        for (std::size_t i = 0; i < source.organisms.size(); ++i)
        {
            x[i].store(source.organisms[i].position.x, std::memory_order_relaxed);
            y[i].store(source.organisms[i].position.y, std::memory_order_relaxed);
            z[i].store(source.organisms[i].position.z, std::memory_order_relaxed);
            energy[i].store(source.organisms[i].energy, std::memory_order_relaxed);
            auto visualEnergy = source.autonomy.voices[i].energy;
            if (source.organisms[i].species
                == aquorbium::Ecosystem::Species::GranularStriker)
                visualEnergy = std::max(visualEnergy,
                                        source.granularStriker.particleEnergy);
            else if (source.organisms[i].species
                     == aquorbium::Ecosystem::Species::Resonator)
                visualEnergy = std::max(visualEnergy, source.resonator.modalEnergy);
            else if (source.organisms[i].species
                     == aquorbium::Ecosystem::Species::PhysicalString)
                visualEnergy = std::max(visualEnergy,
                                        source.physicalString.stringEnergy);
            autonomousVoiceEnergy[i].store(visualEnergy,
                                           std::memory_order_relaxed);
            autonomousPitch[i].store(source.autonomy.voices[i].midiPitch,
                                     std::memory_order_relaxed);
            orbitStep[i].store(static_cast<int>(source.autonomy.voices[i].orbitStep),
                               std::memory_order_relaxed);
            orbitLength[i].store(static_cast<int>(source.autonomy.voices[i].orbitLength),
                                 std::memory_order_relaxed);
            activeParticles[i].store(
                static_cast<int>(source.sources[i].granularStriker.activeParticles),
                std::memory_order_relaxed);
            geometricGate[i].store(source.autonomy.voices[i].geometricGate,
                                   std::memory_order_relaxed);
            observation[i].store(source.mix[i].observation, std::memory_order_relaxed);
            detail[i].store(source.mix[i].detail, std::memory_order_relaxed);
            spatialPan[i].store(source.spatial[i].effectivePan, std::memory_order_relaxed);
            spatialDepth[i].store(source.spatial[i].target.depth, std::memory_order_relaxed);
        }
        blend.store(source.ecosystemBlendCurrent, std::memory_order_relaxed);
        outputEnergy.store(source.outputEnergy, std::memory_order_relaxed);
        momentaryLufs.store(source.output.momentaryLufs,
                            std::memory_order_relaxed);
        outputTruePeak.store(source.output.estimatedTruePeak, std::memory_order_relaxed);
        gainReductionDb.store(source.output.gainReductionDb, std::memory_order_relaxed);
        correlation.store(source.output.correlation, std::memory_order_relaxed);
        tapPreMaster.store(source.feedbackTaps.preMaster, std::memory_order_relaxed);
        tapPreSafety.store(source.feedbackTaps.preSafety, std::memory_order_relaxed);
        tapSafetyReduction.store(source.feedbackTaps.safetyReduction, std::memory_order_relaxed);
        tapPostSafety.store(source.feedbackTaps.postSafety, std::memory_order_relaxed);
        brainRegister.store(source.brainRegister, std::memory_order_relaxed);
        brainMood.store(source.brainMood, std::memory_order_relaxed);
        roomEnabled.store(source.roomEnabled, std::memory_order_relaxed);
        roomActive.store(source.room.active, std::memory_order_relaxed);
        roomLevelDb.store(source.room.levelDb, std::memory_order_relaxed);
        roomStrength.store(source.room.strength, std::memory_order_relaxed);
        roomCoupling.store(source.room.coupling, std::memory_order_relaxed);
        shortTermLufs.store(source.output.shortTermLufs,
                            std::memory_order_relaxed);
        integratedLufs.store(source.output.integratedLufs,
                             std::memory_order_relaxed);
        loudnessBlocks.store(source.output.loudnessBlocks,
                             std::memory_order_relaxed);
        momentaryLoudnessReady.store(source.output.momentaryLoudnessReady,
                                     std::memory_order_relaxed);
        shortTermLoudnessReady.store(source.output.shortTermLoudnessReady,
                                     std::memory_order_relaxed);
        daylight.store(source.environment.daylight, std::memory_order_relaxed);
        temperature.store(source.environment.temperature, std::memory_order_relaxed);
        integrity.store(source.integrity.health, std::memory_order_relaxed);
        lensX.store(source.lens.surfacePosition.x, std::memory_order_relaxed);
        lensY.store(source.lens.surfacePosition.y, std::memory_order_relaxed);
        lensZ.store(source.lens.surfacePosition.z, std::memory_order_relaxed);
        lensTargetX.store(source.lens.targetPosition.x, std::memory_order_relaxed);
        lensTargetY.store(source.lens.targetPosition.y, std::memory_order_relaxed);
        lensTargetZ.store(source.lens.targetPosition.z, std::memory_order_relaxed);
        lensHeld.store(source.lens.heldByObserver >= 0, std::memory_order_relaxed);
        lensFocusedOrganisms.store(static_cast<int>(source.lensFocusedOrganisms), std::memory_order_relaxed);
        autonomousTotalEnergy.store(source.autonomy.totalEnergy, std::memory_order_relaxed);
        autonomousEnabled.store(source.autonomousEnabled, std::memory_order_relaxed);
        autonomousOrganisms.store(static_cast<int>(source.autonomy.activeOrganisms),
                                  std::memory_order_relaxed);
        harmonicRoot.store(source.autonomy.harmonicRootMidi, std::memory_order_relaxed);
        autonomousEvents.store(static_cast<std::uint32_t>(source.autonomy.eventCount),
                               std::memory_order_relaxed);
        activeVoices.store(static_cast<int>(source.activeVoices), std::memory_order_relaxed);
        musicPhase.store(static_cast<int>(source.autonomy.music.phase), std::memory_order_relaxed);
        musicTension.store(source.autonomy.music.tension, std::memory_order_relaxed);
        formTension.store(source.autonomy.tension, std::memory_order_relaxed);
        musicAct.store(static_cast<int>(source.autonomy.music.act), std::memory_order_relaxed);
        const auto& music = source.autonomy.music;
        musicInfluence.store(source.autonomy.musicInfluence, std::memory_order_relaxed);
        musicLatent.store(music.latent, std::memory_order_relaxed);
        musicCapacity.store(music.capacity, std::memory_order_relaxed);
        musicPrey.store(music.prey, std::memory_order_relaxed);
        musicChannelAxis.store(music.channelAxis, std::memory_order_relaxed);
        musicHabitat.store(music.habitat.measured ? music.habitat.health : -1.0f, std::memory_order_relaxed);
        musicRoomDb.store(music.room.active ? music.room.levelDb : 1.0f, std::memory_order_relaxed);
        musicPredators.store(music.predators, std::memory_order_relaxed);
        musicEvents.store(static_cast<int>(music.eventsLastBar), std::memory_order_relaxed);
        musicCentre.store(music.centre, std::memory_order_relaxed);
        musicMode.store(music.mode, std::memory_order_relaxed);
        musicTempo.store(music.tempoBpm, std::memory_order_relaxed);
        for (std::size_t u = 0; u < soundmark.size(); ++u)
        {
            soundmark[u].store(music.soundmark.degrees[u], std::memory_order_relaxed);
            soundmarkSteps[u].store(music.soundmark.steps[u], std::memory_order_relaxed);
        }
        soundmarkLength.store(music.hasSoundmark ? music.soundmark.length : 0, std::memory_order_relaxed);
        soundmarkChosen.store(music.soundmarkChosen, std::memory_order_relaxed);
        for (std::size_t a = 0; a < 5; ++a)
            for (std::size_t b = 0; b < 5; ++b)
            {
                relation[a * 5 + b].store(static_cast<int>(music.relation[a][b]), std::memory_order_relaxed);
                bond[a * 5 + b].store(music.bond[a][b], std::memory_order_relaxed);
            }
        for (std::size_t k = 0; k < actCounts.size(); ++k)
            actCounts[k].store(music.actCounts[k], std::memory_order_relaxed);
    }

    [[nodiscard]] VisualFrame read() const noexcept
    {
        VisualFrame result;
        for (std::size_t i = 0; i < result.organisms.size(); ++i)
        {
            result.organisms[i].x = x[i].load(std::memory_order_relaxed);
            result.organisms[i].y = y[i].load(std::memory_order_relaxed);
            result.organisms[i].z = z[i].load(std::memory_order_relaxed);
            result.organisms[i].energy = energy[i].load(std::memory_order_relaxed);
            result.organisms[i].autonomousEnergy = autonomousVoiceEnergy[i].load(
                std::memory_order_relaxed);
            result.organisms[i].autonomousPitch = autonomousPitch[i].load(
                std::memory_order_relaxed);
            result.organisms[i].orbitStep = orbitStep[i].load(std::memory_order_relaxed);
            result.organisms[i].orbitLength = orbitLength[i].load(std::memory_order_relaxed);
            result.organisms[i].activeParticles = activeParticles[i].load(
                std::memory_order_relaxed);
            result.organisms[i].geometricGate = geometricGate[i].load(
                std::memory_order_relaxed);
            result.organisms[i].observation = observation[i].load(std::memory_order_relaxed);
            result.organisms[i].detail = detail[i].load(std::memory_order_relaxed);
            result.organisms[i].spatialPan = spatialPan[i].load(std::memory_order_relaxed);
            result.organisms[i].spatialDepth = spatialDepth[i].load(std::memory_order_relaxed);
        }
        result.ecosystemBlend = blend.load(std::memory_order_relaxed);
        result.outputEnergy = outputEnergy.load(std::memory_order_relaxed);
        result.momentaryLufs = momentaryLufs.load(std::memory_order_relaxed);
        result.outputTruePeak = outputTruePeak.load(std::memory_order_relaxed);
        result.gainReductionDb = gainReductionDb.load(std::memory_order_relaxed);
        result.correlation = correlation.load(std::memory_order_relaxed);
        result.tapPreMaster = tapPreMaster.load(std::memory_order_relaxed);
        result.tapPreSafety = tapPreSafety.load(std::memory_order_relaxed);
        result.tapSafetyReduction = tapSafetyReduction.load(std::memory_order_relaxed);
        result.tapPostSafety = tapPostSafety.load(std::memory_order_relaxed);
        result.brainRegister = brainRegister.load(std::memory_order_relaxed);
        result.brainMood = brainMood.load(std::memory_order_relaxed);
        result.roomEnabled = roomEnabled.load(std::memory_order_relaxed);
        result.roomActive = roomActive.load(std::memory_order_relaxed);
        result.roomLevelDb = roomLevelDb.load(std::memory_order_relaxed);
        result.roomStrength = roomStrength.load(std::memory_order_relaxed);
        result.roomCoupling = roomCoupling.load(std::memory_order_relaxed);
        result.shortTermLufs = shortTermLufs.load(std::memory_order_relaxed);
        result.integratedLufs = integratedLufs.load(std::memory_order_relaxed);
        result.loudnessBlocks = loudnessBlocks.load(std::memory_order_relaxed);
        result.momentaryLoudnessReady = momentaryLoudnessReady.load(
            std::memory_order_relaxed);
        result.shortTermLoudnessReady = shortTermLoudnessReady.load(
            std::memory_order_relaxed);
        result.daylight = daylight.load(std::memory_order_relaxed);
        result.temperature = temperature.load(std::memory_order_relaxed);
        result.integrity = integrity.load(std::memory_order_relaxed);
        result.lensX = lensX.load(std::memory_order_relaxed);
        result.lensY = lensY.load(std::memory_order_relaxed);
        result.lensZ = lensZ.load(std::memory_order_relaxed);
        result.lensTargetX = lensTargetX.load(std::memory_order_relaxed);
        result.lensTargetY = lensTargetY.load(std::memory_order_relaxed);
        result.lensTargetZ = lensTargetZ.load(std::memory_order_relaxed);
        result.lensHeld = lensHeld.load(std::memory_order_relaxed);
        result.lensFocusedOrganisms = lensFocusedOrganisms.load(std::memory_order_relaxed);
        result.autonomousEnergy = autonomousTotalEnergy.load(std::memory_order_relaxed);
        result.autonomousEnabled = autonomousEnabled.load(std::memory_order_relaxed);
        result.autonomousOrganisms = autonomousOrganisms.load(std::memory_order_relaxed);
        result.harmonicRoot = harmonicRoot.load(std::memory_order_relaxed);
        result.autonomousEvents = autonomousEvents.load(std::memory_order_relaxed);
        result.activeVoices = activeVoices.load(std::memory_order_relaxed);
        result.musicPhase = musicPhase.load(std::memory_order_relaxed);
        result.musicTension = musicTension.load(std::memory_order_relaxed);
        result.formTension = formTension.load(std::memory_order_relaxed);
        result.musicAct = musicAct.load(std::memory_order_relaxed);
        result.musicInfluence = musicInfluence.load(std::memory_order_relaxed);
        result.musicLatent = musicLatent.load(std::memory_order_relaxed);
        result.musicCapacity = musicCapacity.load(std::memory_order_relaxed);
        result.musicPrey = musicPrey.load(std::memory_order_relaxed);
        result.musicChannelAxis = musicChannelAxis.load(std::memory_order_relaxed);
        result.musicHabitat = musicHabitat.load(std::memory_order_relaxed);
        result.musicRoomDb = musicRoomDb.load(std::memory_order_relaxed);
        result.musicPredators = musicPredators.load(std::memory_order_relaxed);
        result.musicEvents = musicEvents.load(std::memory_order_relaxed);
        result.musicCentre = musicCentre.load(std::memory_order_relaxed);
        result.musicMode = musicMode.load(std::memory_order_relaxed);
        result.musicTempo = musicTempo.load(std::memory_order_relaxed);
        for (std::size_t u = 0; u < soundmark.size(); ++u)
        {
            result.soundmark[u] = static_cast<std::int8_t>(soundmark[u].load(std::memory_order_relaxed));
            result.soundmarkSteps[u] = static_cast<std::uint8_t>(soundmarkSteps[u].load(std::memory_order_relaxed));
        }
        result.soundmarkLength = soundmarkLength.load(std::memory_order_relaxed);
        result.soundmarkChosen = soundmarkChosen.load(std::memory_order_relaxed);
        for (std::size_t k = 0; k < 25; ++k)
        {
            result.relation[k] = static_cast<std::uint8_t>(relation[k].load(std::memory_order_relaxed));
            result.bond[k] = bond[k].load(std::memory_order_relaxed);
        }
        for (std::size_t k = 0; k < result.actCounts.size(); ++k)
            result.actCounts[k] = actCounts[k].load(std::memory_order_relaxed);
        return result;
    }

private:
    std::array<std::atomic<float>, aquorbium::Ecosystem::organismCount> x {};
    std::array<std::atomic<float>, aquorbium::Ecosystem::organismCount> y {};
    std::array<std::atomic<float>, aquorbium::Ecosystem::organismCount> z {};
    std::array<std::atomic<float>, aquorbium::Ecosystem::organismCount> energy {};
    std::array<std::atomic<float>, aquorbium::Ecosystem::organismCount> autonomousVoiceEnergy {};
    std::array<std::atomic<float>, aquorbium::Ecosystem::organismCount> autonomousPitch {};
    std::array<std::atomic<int>, aquorbium::Ecosystem::organismCount> orbitStep {};
    std::array<std::atomic<int>, aquorbium::Ecosystem::organismCount> orbitLength {};
    std::array<std::atomic<int>, aquorbium::Ecosystem::organismCount> activeParticles {};
    std::array<std::atomic<bool>, aquorbium::Ecosystem::organismCount> geometricGate {};
    std::array<std::atomic<float>, aquorbium::Ecosystem::organismCount> observation {};
    std::array<std::atomic<float>, aquorbium::Ecosystem::organismCount> detail {};
    std::array<std::atomic<float>, aquorbium::Ecosystem::organismCount> spatialPan {};
    std::array<std::atomic<float>, aquorbium::Ecosystem::organismCount> spatialDepth {};
    std::atomic<float> blend { 1.0f };
    std::atomic<float> outputEnergy { 0.0f };
    std::atomic<float> momentaryLufs { aquorbium::LoudnessMeter::silenceLufs };
    std::atomic<float> outputTruePeak { 0.0f };
    std::atomic<float> gainReductionDb { 0.0f };
    std::atomic<float> correlation { 0.0f };
    std::atomic<float> tapPreMaster { 0.0f }, tapPreSafety { 0.0f }, tapSafetyReduction { 0.0f }, tapPostSafety { 0.0f };
    std::atomic<int> brainRegister { 0 };
    std::atomic<float> brainMood { 0.0f }, roomLevelDb { -120.0f }, roomStrength { 0.0f }, roomCoupling { 0.0f };
    std::atomic<bool> roomEnabled { false }, roomActive { false };
    std::atomic<float> shortTermLufs { aquorbium::LoudnessMeter::silenceLufs };
    std::atomic<float> integratedLufs { aquorbium::LoudnessMeter::silenceLufs };
    std::atomic<std::uint64_t> loudnessBlocks { 0 };
    std::atomic<bool> momentaryLoudnessReady { false };
    std::atomic<bool> shortTermLoudnessReady { false };
    std::atomic<float> daylight { 0.5f };
    std::atomic<float> temperature { 0.5f };
    std::atomic<float> integrity { 1.0f };
    std::atomic<float> lensX { 1.0f };
    std::atomic<float> lensY { 0.0f };
    std::atomic<float> lensZ { 0.0f };
    std::atomic<float> lensTargetX { 1.0f };
    std::atomic<float> lensTargetY { 0.0f };
    std::atomic<float> lensTargetZ { 0.0f };
    std::atomic<bool> lensHeld { false };
    std::atomic<int> lensFocusedOrganisms { 0 };
    std::atomic<float> autonomousTotalEnergy { 0.0f };
    std::atomic<bool> autonomousEnabled { false };
    std::atomic<int> autonomousOrganisms { 0 };
    std::atomic<int> harmonicRoot { 36 };
    std::atomic<std::uint32_t> autonomousEvents { 0 };
    std::atomic<int> activeVoices { 0 };
    std::atomic<int> musicPhase { 2 };
    std::atomic<float> musicTension { 0.0f };
    std::atomic<float> formTension { 0.5f };
    std::atomic<int> musicAct { 0 };
    std::atomic<float> musicInfluence { 0.0f };
    std::atomic<float> musicLatent { 0.0f };
    std::atomic<float> musicCapacity { 0.0f };
    std::atomic<float> musicPrey { 1.0f };
    std::atomic<float> musicChannelAxis { 0.0f };
    std::atomic<float> musicHabitat { -1.0f };
    std::atomic<float> musicRoomDb { 1.0f };
    std::atomic<float> musicPredators { 1.0f };
    std::atomic<int> musicEvents { 0 };
    std::atomic<int> musicCentre { 0 };
    std::atomic<int> musicMode { 3 };
    std::atomic<float> musicTempo { 0.0f };
    std::array<std::atomic<int>, 25> relation {};
    std::array<std::atomic<int>, 8> soundmark {};
    std::array<std::atomic<int>, 8> soundmarkSteps {};
    std::atomic<int> soundmarkLength { 0 };
    std::atomic<bool> soundmarkChosen { false };
    std::array<std::atomic<float>, 25> bond {};
    std::array<std::atomic<std::uint32_t>, aquorbium::EcologicalMusicDirector::actCount> actCounts {};
};

// MEDIDORES (2026-10-24): the output measured, as bars with their values.
// It reads the GUI's VisualFrame copy only (never the audio thread) and
// repaints at most 15 times a second.
class MeterPanel final : public juce::Component, public juce::SettableTooltipClient
{
public:
    static constexpr int rowHeight = 16;
    static constexpr int rows = 9;
    static constexpr int preferredHeight = rows * rowHeight + 6;

    void setFrame(const VisualFrame& frame)
    {
        if (++tick % 2 != 0)
            return;
        readings = frame;
        repaint();
    }

    void paint(juce::Graphics& g) override
    {
        using aquorbium::app::T;
        namespace S = aquorbium::app::S;
        const auto db = [](float linear) { return 20.0f * std::log10(std::max(linear, 1.0e-6f)); };
        struct Row
        {
            float value;      // shown as text
            float position;   // 0..1 along the bar
            juce::String text;
            juce::Colour colour;
            bool centred;
        };
        const auto lufs = [](float v, bool ready) {
            const auto shown = ready && v > aquorbium::LoudnessMeter::silenceLufs + 1.0f;
            return Row { v, shown ? std::clamp((v + 60.0f) / 60.0f, 0.0f, 1.0f) : 0.0f,
                         shown ? juce::String(v, 1) + " LUFS" : juce::String("--"), Palette::cyan, false };
        };
        const auto tap = [&db](float v) {
            const auto d = db(v);
            return Row { d, std::clamp((d + 60.0f) / 60.0f, 0.0f, 1.0f),
                         v > 1.0e-5f ? juce::String(d, 1) + " dB" : juce::String("--"), Palette::violet, false };
        };
        const auto truePeak = db(readings.outputTruePeak);
        const auto reduction = readings.tapSafetyReduction * 12.0f;
        const std::array<Row, rows> values {{
            lufs(readings.momentaryLufs, readings.momentaryLoudnessReady),
            lufs(readings.shortTermLufs, readings.shortTermLoudnessReady),
            lufs(readings.integratedLufs, readings.loudnessBlocks > 0),
            { truePeak, std::clamp((truePeak + 60.0f) / 63.0f, 0.0f, 1.0f),
              readings.outputTruePeak > 1.0e-6f ? juce::String(truePeak, 1) + " dBTP" : juce::String("--"),
              truePeak > -1.0f ? Palette::coral : Palette::green, false },
            { reduction, std::clamp(reduction / 12.0f, 0.0f, 1.0f), juce::String(reduction, 1) + " dB",
              Palette::amber, false },
            { readings.correlation, std::clamp((readings.correlation + 1.0f) * 0.5f, 0.0f, 1.0f),
              juce::String(readings.correlation, 2), readings.correlation < 0.0f ? Palette::coral : Palette::green, true },
            tap(readings.tapPreMaster),
            tap(readings.tapPreSafety),
            tap(readings.tapPostSafety),
        }};
        auto area = getLocalBounds().reduced(2, 3);
        const auto labelWidth = std::min(104, area.getWidth() / 3);
        const auto valueWidth = std::min(76, area.getWidth() / 4);
        for (std::size_t i = 0; i < values.size(); ++i)
        {
            auto line = area.removeFromTop(rowHeight);
            g.setFont(uiFont(9.5f, juce::Font::bold));
            g.setColour(Palette::muted);
            g.drawFittedText(T(S::meterRows[i]), line.removeFromLeft(labelWidth), juce::Justification::centredLeft, 1);
            g.setColour(Palette::text);
            g.setFont(uiFont(10.0f));
            g.drawFittedText(values[i].text, line.removeFromRight(valueWidth), juce::Justification::centredRight, 1);
            auto bar = line.reduced(6, 5).toFloat();
            g.setColour(Palette::line.withAlpha(0.45f));
            g.fillRoundedRectangle(bar, 2.0f);
            g.setColour(values[i].colour.withAlpha(0.85f));
            if (values[i].centred)
            {
                const auto middle = bar.getCentreX();
                const auto x = bar.getX() + bar.getWidth() * values[i].position;
                g.fillRect(juce::Rectangle<float>::leftTopRightBottom(std::min(middle, x), bar.getY(),
                                                                       std::max(middle, x), bar.getBottom()));
                g.setColour(Palette::muted);
                g.drawVerticalLine(static_cast<int>(middle), bar.getY(), bar.getBottom());
            }
            else
                g.fillRoundedRectangle(bar.withWidth(bar.getWidth() * values[i].position), 2.0f);
        }
    }

private:
    VisualFrame readings {};
    unsigned tick = 0;
};

class EcosystemView final : public juce::Component
{
public:
    std::function<void(bool, aquorbium::Ecosystem::Vec3)> onLensGesture;
    std::function<void(bool)> onFreshWater;  // true: fresh water; false: sea
    std::function<void(bool)> onRoom;        // RoomCoupling on/off
    std::function<void()> onMark;            // the musician marks the soundmark

    EcosystemView()
    {
        // sea or fresh water, in the music panel's title line
        waterType.setClickingTogglesState(true);
        waterType.onClick = [this] {
            refreshTexts();
            if (onFreshWater)
                onFreshWater(waterType.getToggleState());
        };
        addChildComponent(waterType);
        roomButton.setClickingTogglesState(true);
        roomButton.onClick = [this] {
            if (onRoom)
                onRoom(roomButton.getToggleState());
        };
        addChildComponent(roomButton);
        markButton.onClick = [this] {
            if (onMark)
                onMark();
        };
        addChildComponent(markButton);
        refreshTexts();
    }

    void setRoom(bool on)
    {
        roomButton.setToggleState(on, juce::dontSendNotification);
    }

    void setFreshWater(bool fresh)
    {
        waterType.setToggleState(fresh, juce::dontSendNotification);
        refreshTexts();
    }

    void refreshTexts()
    {
        waterType.setButtonText(waterType.getToggleState() ? aquorbium::app::T(aquorbium::app::S::freshWater)
                                                           : aquorbium::app::T(aquorbium::app::S::sea));
        waterType.setTooltip(aquorbium::app::T(aquorbium::app::S::waterTypeTip));
        roomButton.setButtonText(aquorbium::app::T(aquorbium::app::S::room));
        roomButton.setTooltip(aquorbium::app::T(aquorbium::app::S::roomTip));
        markButton.setButtonText(aquorbium::app::T(aquorbium::app::S::mark));
        markButton.setTooltip(aquorbium::app::T(aquorbium::app::S::markTip));
    }

    void resized() override
    {
        const auto panel = computeAreas().panel;
        waterType.setVisible(!panel.isEmpty());
        roomButton.setVisible(!panel.isEmpty());
        markButton.setVisible(!panel.isEmpty());
        if (!panel.isEmpty())
        {
            waterType.setBounds(juce::Rectangle<int>(static_cast<int>(panel.getRight()) - 112 - 12,
                                                     static_cast<int>(panel.getY()) + 6, 112, 20));
            roomButton.setBounds(waterType.getBounds().translated(-(72 + 6), 0).withWidth(72));
            markButton.setBounds(roomButton.getBounds().translated(-(80 + 6), 0).withWidth(80));
        }
    }

    void setFrame(VisualFrame next) noexcept
    {
        // the latest acts of the ecological music, for the panel's log
        const auto now = juce::Time::getMillisecondCounterHiRes() / 1000.0;
        if (firstFrame)
            startSeconds = now;
        for (std::size_t k = 0; k < next.actCounts.size(); ++k)
            if (!firstFrame && next.actCounts[k] > frame.actCounts[k])
            {
                actLog[actLogWrite % actLog.size()] = { now - startSeconds, static_cast<int>(k) };
                ++actLogWrite;
                // a sonar PPI draws an expanding circle at each ping
                pings[pingWrite % pings.size()] = now;
                ++pingWrite;
            }
        // the sweep turns once every four seconds
        const auto elapsed = firstFrame ? 0.0 : juce::jlimit(0.0, 0.25, now - lastFrameSeconds);
        sweep = std::fmod(sweep + elapsed * juce::MathConstants<double>::twoPi / 4.0, juce::MathConstants<double>::twoPi);
        lastFrameSeconds = now;
        // the waterfall: twice a second, a row with the strength of each kind
        // of relation in the web (newest on top, as on a sonar's bearing-time
        // history)
        if (now - lastWaterfallSeconds >= 0.5)
        {
            lastWaterfallSeconds = now;
            auto& row = waterfall[waterfallWrite % waterfall.size()];
            row.fill(0.0f);
            for (std::size_t k = 0; k < 25; ++k)
            {
                const auto relation = static_cast<int>(next.relation[k]);
                if (relation >= 1 && relation <= 6)
                    row[static_cast<std::size_t>(relation - 1)] = std::min(1.0f, row[static_cast<std::size_t>(relation - 1)] + next.bond[k]);
            }
            ++waterfallWrite;
        }
        firstFrame = false;
        frame = next;
        repaint();
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(0.75f);
        g.setColour(Palette::panelRaised);
        g.fillRoundedRectangle(bounds, 14.0f);
        g.setColour(Palette::cyan.withAlpha(0.32f));
        g.drawRoundedRectangle(bounds, 14.0f, 1.25f);

        auto inner = bounds.reduced(18.0f);
        drawHeader(g, inner.removeFromTop(48.0f));
        drawFooter(g, inner.removeFromBottom(42.0f));

        const auto areas = computeAreas();
        drawSphere(g, areas.sphere);
        drawOrganisms(g, areas.sphere);
        if (!areas.panel.isEmpty())
            drawMusicPanel(g, areas.panel);
    }

    void mouseDown(const juce::MouseEvent& event) override
    {
        const auto sphere = sphereBounds();
        const auto radius = sphere.getWidth() * 0.44f;
        const auto delta = event.position - sphere.getCentre();
        if (delta.x * delta.x + delta.y * delta.y > radius * radius)
            return;
        draggingLens = true;
        publishLensGesture(event, true);
    }

    void mouseDrag(const juce::MouseEvent& event) override
    {
        if (draggingLens)
            publishLensGesture(event, true);
    }

    void mouseUp(const juce::MouseEvent& event) override
    {
        if (!draggingLens)
            return;
        draggingLens = false;
        publishLensGesture(event, false);
    }

private:
    struct Areas
    {
        juce::Rectangle<float> sphere;
        juce::Rectangle<float> panel;
    };

    // When the frame is taller than the sphere needs (the wide surface), the
    // spare height below it becomes the panel of the ecological music.
    [[nodiscard]] Areas computeAreas() const
    {
        auto inner = getLocalBounds().toFloat().reduced(0.75f).reduced(18.0f);
        inner.removeFromTop(48.0f);
        inner.removeFromBottom(42.0f);
        Areas result;
        const auto spare = inner.getHeight() - inner.getWidth();
        if (spare >= 150.0f)
            result.panel = inner.removeFromBottom(std::min(spare, 250.0f)).reduced(4.0f, 6.0f);
        const auto diameter = std::max(
            40.0f, std::min(inner.getWidth(), inner.getHeight()) - 14.0f);
        result.sphere = juce::Rectangle<float>(diameter, diameter).withCentre(inner.getCentre());
        return result;
    }

    [[nodiscard]] juce::Rectangle<float> sphereBounds() const { return computeAreas().sphere; }

    static juce::Colour relationColour(int relation) noexcept
    {
        switch (relation)
        {
            case 1: return Palette::coral;                 // hunts
            case 2: return Palette::amber;                 // flees
            case 3: return juce::Colour(0xffe8d04a);       // rivals
            case 4: return Palette::cyan;                  // allied
            case 5: return juce::Colour(0xffff8fc8);       // courts
            case 6: return Palette::violet;                // guards
            default: return Palette::muted;
        }
    }

    // The web of relations, drawn between the organisms: colour = relation,
    // thickness = bond; a dot marks whom it is aimed at.
    void drawWeb(juce::Graphics& g,
                 const std::array<juce::Point<float>, aquorbium::Ecosystem::organismCount>& points) const
    {
        if (frame.musicInfluence <= 0.0f)
            return;
        for (std::size_t a = 0; a < points.size(); ++a)
            for (std::size_t b = 0; b < points.size(); ++b)
            {
                if (a == b)
                    continue;
                const auto relation = static_cast<int>(frame.relation[a * 5 + b]);
                const auto bond = frame.bond[a * 5 + b];
                if (relation == 0 || bond < 0.18f)
                    continue;
                // a symmetric relation is drawn once, from the stronger side
                const auto back = frame.bond[b * 5 + a];
                if (static_cast<int>(frame.relation[b * 5 + a]) == relation && (back > bond || (!(back < bond) && b < a)))
                    continue;
                const auto colour = relationColour(relation);
                g.setColour(colour.withAlpha(juce::jlimit(0.25f, 0.95f, 0.25f + bond)));
                g.drawLine({ points[a], points[b] }, 1.0f + 3.5f * bond);
                const auto towards = points[a] + (points[b] - points[a]) * 0.82f;
                g.fillEllipse(juce::Rectangle<float>(5.0f + 3.0f * bond, 5.0f + 3.0f * bond).withCentre(towards));
            }
    }

    void drawMusicPanel(juce::Graphics& g, juce::Rectangle<float> area) const
    {
        g.setColour(Palette::panel.withAlpha(0.55f));
        g.fillRoundedRectangle(area, 8.0f);
        auto content = area.reduced(12.0f, 8.0f);
        g.setFont(uiFont(10.0f, juce::Font::bold));
        g.setColour(Palette::cyan);
        const auto titleText = aquorbium::app::T(aquorbium::app::S::musicPanel);
        const auto titleRight = content.getX() + juce::GlyphArrangement::getStringWidth(uiFont(10.0f, juce::Font::bold), titleText) + 6.0f;
        g.drawText(titleText, content.removeFromTop(16.0f), juce::Justification::centredLeft);
        // the soundmark's contour, between the title and MARCA: dots joined by
        // a line, amber when the musician chose it, muted when the place
        // found it; drawn only where there is room
        const auto markLeft = static_cast<float>(markButton.getX()) - 5.0f;
        const auto contourLeft = std::max(titleRight, markLeft - 72.0f);
        if (frame.musicInfluence > 0.0f && frame.soundmarkLength >= 2 && markButton.isVisible()
            && markLeft - contourLeft >= 16.0f)
        {
            const auto box = juce::Rectangle<float>(contourLeft, static_cast<float>(markButton.getY()),
                                                    markLeft - contourLeft, static_cast<float>(markButton.getHeight()))
                                 .reduced(1.0f, 2.0f);
            const auto dot = std::clamp(box.getWidth() / static_cast<float>(std::max(frame.soundmarkLength, 1)), 2.0f, 4.0f);
            const auto n = std::min(frame.soundmarkLength, 8);
            juce::Path contour;
            g.setColour((frame.soundmarkChosen ? Palette::amber : Palette::muted).withAlpha(0.9f));
            for (int u = 0; u < n; ++u)
            {
                const auto x = box.getX() + box.getWidth() * (static_cast<float>(u) + 0.5f) / static_cast<float>(n);
                const auto y = box.getBottom() - box.getHeight() * (static_cast<float>(frame.soundmark[static_cast<std::size_t>(u)]) + 5.0f) / 12.0f;
                if (u == 0) contour.startNewSubPath(x, y); else contour.lineTo(x, y);
                g.fillEllipse(juce::Rectangle<float>(dot, dot).withCentre({ x, y }));
            }
            g.strokePath(contour, juce::PathStrokeType(1.0f));
        }
        if (frame.musicInfluence <= 0.0f)
        {
            g.setFont(uiFont(10.5f));
            g.setColour(Palette::muted);
            g.drawFittedText(aquorbium::app::T(aquorbium::app::S::musicOff),
                             content.toNearestInt(), juce::Justification::topLeft, 4);
            return;
        }
        // the water's states as the lamps of a "Christmas tree" panel: the
        // current state lit, the others dark
        static const std::array<juce::Colour, 5> stateColours {
            juce::Colour(0xffbfe9ff), Palette::cyan, Palette::green, Palette::amber, Palette::coral };
        auto water = content.removeFromTop(30.0f);
        const auto chip = water.getWidth() / 5.0f;
        for (int k = 0; k < 5; ++k)
        {
            auto cell = water.removeFromLeft(chip);
            const auto current = k == frame.musicPhase;
            const auto colour = stateColours[static_cast<std::size_t>(k)];
            const auto lamp = juce::Rectangle<float>(12.0f, 12.0f).withCentre({ cell.getX() + 10.0f, cell.getCentreY() });
            if (current)
            {
                g.setColour(colour.withAlpha(0.25f));
                g.fillEllipse(lamp.expanded(5.0f));
            }
            g.setColour(current ? colour : Palette::background);
            g.fillEllipse(lamp);
            g.setColour(current ? colour.brighter(0.4f) : Palette::line);
            g.drawEllipse(lamp, 1.0f);
            g.setColour(current ? Palette::text : Palette::muted);
            g.setFont(uiFont(9.0f, juce::Font::bold));
            g.drawFittedText(aquorbium::app::T(aquorbium::app::S::waterStates[static_cast<std::size_t>(k)]),
                             cell.withTrimmedLeft(20.0f).toNearestInt(), juce::Justification::centredLeft, 1);
        }

        // the latent heat as a bubble inclinometer ("like a carpenter's
        // level"): the bubble leans towards the next state being paid for
        auto level = content.removeFromTop(16.0f).reduced(chip * 0.5f, 3.0f);
        g.setColour(Palette::background.withAlpha(0.9f));
        g.fillRoundedRectangle(level, level.getHeight() * 0.5f);
        g.setColour(Palette::line);
        g.drawRoundedRectangle(level, level.getHeight() * 0.5f, 1.0f);
        for (int t = -4; t <= 4; ++t)
        {
            const auto tx = level.getCentreX() + level.getWidth() * 0.45f * static_cast<float>(t) / 4.0f;
            g.setColour(Palette::muted.withAlpha(t == 0 ? 0.8f : 0.35f));
            g.drawVerticalLine(static_cast<int>(tx), level.getY() + 2.0f, level.getBottom() - 2.0f);
        }
        const auto paid = juce::jlimit(-1.0f, 1.0f, frame.musicLatent);
        const auto bubble = juce::Rectangle<float>(level.getHeight() * 1.8f, level.getHeight() - 4.0f)
            .withCentre({ level.getCentreX() + level.getWidth() * 0.42f * paid, level.getCentreY() });
        g.setColour((paid >= 0.0f ? Palette::coral : Palette::cyan).withAlpha(0.85f));
        g.fillEllipse(bubble);
        g.setColour(Palette::text.withAlpha(0.6f));
        g.fillEllipse(bubble.reduced(bubble.getWidth() * 0.3f, bubble.getHeight() * 0.3f).translated(-2.0f, -1.0f));
        content.removeFromTop(6.0f);

        // tension on a pressure gauge with a red zone; the key, tempo and niche
        static constexpr std::array<const char*, 12> pitchNames { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
        auto line = content.removeFromTop(40.0f);
        auto gaugeArea = line.removeFromLeft(74.0f);
        drawPressureGauge(g, gaugeArea, juce::jlimit(0.0f, 1.0f, frame.musicTension));
        g.setColour(Palette::muted);
        g.setFont(uiFont(10.0f, juce::Font::bold));
        g.drawText(aquorbium::app::T(aquorbium::app::S::tension), line.removeFromTop(16.0f).reduced(6.0f, 0.0f), juce::Justification::centredLeft);
        g.setColour(Palette::text);
        g.drawText(juce::String(pitchNames[static_cast<std::size_t>(((frame.musicCentre % 12) + 12) % 12)]) + " "
                       + aquorbium::app::T(aquorbium::app::S::modes[static_cast<std::size_t>(juce::jlimit(0, 5, frame.musicMode))]) + utf8(" · ")
                       + juce::String(juce::roundToInt(frame.musicTempo)) + " bpm" + aquorbium::app::T(aquorbium::app::S::niche)
                       + juce::String(frame.musicEvents) + "/" + juce::String(frame.musicCapacity, 1)
                       + (frame.musicHabitat >= 0.0f
                              ? aquorbium::app::T(aquorbium::app::S::habitat)
                                    + juce::String(juce::roundToInt(frame.musicHabitat * 100.0f)) + "%"
                              : juce::String())
                       + (frame.musicRoomDb <= 0.0f
                              ? aquorbium::app::T(aquorbium::app::S::roomLevel)
                                    + juce::String(juce::roundToInt(frame.musicRoomDb)) + " dB"
                              : juce::String()),
                   line.reduced(6.0f, 0.0f), juce::Justification::centredLeft);
        content.removeFromTop(2.0f);

        // the populations (Lotka-Volterra): two level bars around the
        // equilibrium mark - the prey fill the niche, the predators follow
        {
            auto populations = content.removeFromTop(12.0f).reduced(6.0f, 0.0f);
            const auto half = populations.getWidth() * 0.5f;
            const auto drawPopulation = [&](juce::Rectangle<float> cell, const juce::String& name, float abundance,
                                            juce::Colour colour) {
                g.setColour(Palette::muted);
                g.setFont(uiFont(8.5f, juce::Font::bold));
                g.drawText(name, cell.removeFromLeft(70.0f), juce::Justification::centredLeft);
                auto bar = cell.reduced(0.0f, 3.0f).withTrimmedRight(8.0f);
                g.setColour(Palette::muted.withAlpha(0.3f));
                g.fillRect(bar);
                const auto amount = juce::jlimit(0.0f, 1.0f, abundance / 2.0f);
                g.setColour(colour.withAlpha(0.85f));
                g.fillRect(bar.withWidth(bar.getWidth() * amount));
                g.setColour(Palette::text.withAlpha(0.7f));
                g.drawVerticalLine(static_cast<int>(bar.getCentreX()), bar.getY() - 2.0f, bar.getBottom() + 2.0f);
            };
            drawPopulation(populations.removeFromLeft(half), aquorbium::app::T(aquorbium::app::S::prey),
                           frame.musicPrey, Palette::cyan);
            drawPopulation(populations, aquorbium::app::T(aquorbium::app::S::predators), frame.musicPredators,
                           Palette::coral);
        }
        content.removeFromTop(4.0f);

        // legend of the web
        auto legend = content.removeFromTop(16.0f);
        const auto legendWidth = legend.getWidth() / 6.0f;
        g.setFont(uiFont(9.0f));
        for (int k = 0; k < 6; ++k)
        {
            auto cell = legend.removeFromLeft(legendWidth);
            g.setColour(relationColour(k + 1));
            g.fillRect(cell.removeFromLeft(10.0f).withSizeKeepingCentre(10.0f, 3.0f));
            g.setColour(Palette::muted);
            g.drawText(aquorbium::app::T(aquorbium::app::S::relations[static_cast<std::size_t>(k)]), cell.reduced(3.0f, 0.0f), juce::Justification::centredLeft);
        }
        content.removeFromTop(4.0f);

        // the latest acts on the left; the waterfall of the web on the right,
        // newest on top: each column a kind of relation, brightness its bond
        auto waterfallArea = content.removeFromRight(content.getWidth() * 0.42f).reduced(4.0f, 0.0f);
        g.setColour(Palette::background.withAlpha(0.9f));
        g.fillRect(waterfallArea);
        const auto rowsShown = std::min<std::size_t>(waterfall.size(), waterfallWrite);
        const auto rowHeight = waterfallArea.getHeight() / static_cast<float>(waterfall.size());
        const auto columnWidth = waterfallArea.getWidth() / 6.0f;
        for (std::size_t r = 0; r < rowsShown; ++r)
        {
            const auto& row = waterfall[(waterfallWrite - 1 - r) % waterfall.size()];
            for (std::size_t k = 0; k < 6; ++k)
                if (row[k] > 0.02f)
                {
                    g.setColour(relationColour(static_cast<int>(k) + 1).withAlpha(juce::jlimit(0.08f, 1.0f, row[k])));
                    g.fillRect(waterfallArea.getX() + columnWidth * static_cast<float>(k) + 1.0f,
                               waterfallArea.getY() + rowHeight * static_cast<float>(r),
                               columnWidth - 2.0f, std::max(1.0f, rowHeight));
                }
        }
        g.setColour(Palette::line);
        g.drawRect(waterfallArea, 1.0f);

        g.setFont(uiFont(10.0f));
        const auto rows = std::min<std::size_t>(actLogWrite, std::min<std::size_t>(actLog.size(),
            static_cast<std::size_t>(std::max(0.0f, content.getHeight() / 15.0f))));
        for (std::size_t r = 0; r < rows; ++r)
        {
            const auto& item = actLog[(actLogWrite - 1 - r) % actLog.size()];
            const auto seconds = static_cast<int>(item.seconds);
            const auto text = juce::String(seconds / 60).paddedLeft('0', 2) + ":" + juce::String(seconds % 60).paddedLeft('0', 2)
                + "  " + aquorbium::app::T(aquorbium::app::S::acts[static_cast<std::size_t>(juce::jlimit(0, static_cast<int>(aquorbium::EcologicalMusicDirector::actCount) - 1, item.act))]);
            g.setColour(r == 0 ? Palette::text : Palette::muted.withAlpha(0.9f - 0.15f * static_cast<float>(r)));
            g.drawText(text, content.removeFromTop(15.0f), juce::Justification::centredLeft);
        }
    }

    // a pressure gauge: an arc scale with a red zone near the top, a needle
    static void drawPressureGauge(juce::Graphics& g, juce::Rectangle<float> area, float value)
    {
        const auto radius = std::min(area.getWidth() * 0.5f, area.getHeight()) - 3.0f;
        const auto centre = juce::Point<float>(area.getCentreX(), area.getBottom() - 3.0f);
        const auto start = -juce::MathConstants<float>::halfPi, end = juce::MathConstants<float>::halfPi;
        juce::Path scale, red;
        scale.addCentredArc(centre.x, centre.y, radius, radius, 0.0f, start, end, true);
        red.addCentredArc(centre.x, centre.y, radius, radius, 0.0f, start + (end - start) * 0.75f, end, true);
        g.setColour(Palette::line);
        g.strokePath(scale, juce::PathStrokeType(3.0f));
        g.setColour(Palette::coral.withAlpha(0.85f));
        g.strokePath(red, juce::PathStrokeType(3.0f));
        for (int t = 0; t <= 8; ++t)
        {
            const auto a = start + (end - start) * static_cast<float>(t) / 8.0f;
            const auto dx = std::sin(a), dy = -std::cos(a);
            g.setColour(Palette::muted.withAlpha(0.7f));
            g.drawLine(centre.x + dx * radius * 0.78f, centre.y + dy * radius * 0.78f,
                       centre.x + dx * radius * 0.92f, centre.y + dy * radius * 0.92f, 0.8f);
        }
        const auto a = start + (end - start) * value;
        g.setColour(Palette::text);
        g.drawLine(centre.x, centre.y, centre.x + std::sin(a) * radius * 0.86f, centre.y - std::cos(a) * radius * 0.86f, 1.6f);
        g.fillEllipse(juce::Rectangle<float>(5.0f, 5.0f).withCentre(centre));
    }

    void publishLensGesture(const juce::MouseEvent& event, bool held)
    {
        if (!onLensGesture)
            return;
        const auto sphere = sphereBounds();
        const auto radius = sphere.getWidth() * 0.44f;
        const auto delta = event.position - sphere.getCentre();
        auto x = juce::jlimit(-1.0f, 1.0f, delta.x / radius);
        auto y = juce::jlimit(-1.0f, 1.0f, -delta.y / radius);
        const auto length = std::sqrt(x * x + y * y);
        if (length > 0.999f)
        {
            x *= 0.999f / length;
            y *= 0.999f / length;
        }
        const auto z = std::sqrt(std::max(0.0f, 1.0f - x * x - y * y))
            * (event.mods.isAltDown() ? -1.0f : 1.0f);
        onLensGesture(held, { x, y, z });
    }

    void drawHeader(juce::Graphics& g, juce::Rectangle<float> area) const
    {
        g.setFont(uiFont(10.0f, juce::Font::bold));
        g.setColour(Palette::cyan);
        g.drawText(utf8("BIOENERGY 100 HZ · LIVE ECOLOGY 200 HZ"), area.removeFromTop(15.0f),
                   juce::Justification::centred);
        g.setFont(uiFont(21.0f, juce::Font::bold));
        g.setColour(Palette::text);
        g.drawText(aquorbium::app::T(aquorbium::app::S::ecosystem), area, juce::Justification::centred);
    }

    void drawSphere(juce::Graphics& g, juce::Rectangle<float> sphere) const
    {
        juce::ColourGradient water(Palette::cyan.withAlpha(0.11f), sphere.getCentreX(), sphere.getY(),
                                   Palette::background.withAlpha(0.18f), sphere.getCentreX(), sphere.getBottom(), false);
        g.setGradientFill(water);
        g.fillEllipse(sphere);
        g.setColour(Palette::cyan.withAlpha(0.55f));
        g.drawEllipse(sphere, 1.4f);
        g.setColour(Palette::cyan.withAlpha(0.14f));
        g.drawEllipse(sphere.withSizeKeepingCentre(sphere.getWidth(), sphere.getHeight() * 0.34f), 1.0f);
        g.drawEllipse(sphere.withSizeKeepingCentre(sphere.getWidth() * 0.34f, sphere.getHeight()), 1.0f);
        g.drawLine(sphere.getX(), sphere.getCentreY(), sphere.getRight(), sphere.getCentreY(), 0.7f);
        g.drawLine(sphere.getCentreX(), sphere.getY(), sphere.getCentreX(), sphere.getBottom(), 0.7f);
        drawSonar(g, sphere);

        const auto lensSize = sphere.getWidth() * 0.11f;
        const auto lensCentre = juce::Point<float> {
            sphere.getCentreX() + frame.lensX * sphere.getWidth() * 0.44f,
            sphere.getCentreY() - frame.lensY * sphere.getHeight() * 0.44f
        };
        const auto lens = juce::Rectangle<float>(lensSize, lensSize)
            .withCentre(lensCentre);
        const auto lensOpacity = frame.lensZ < 0.0f ? 0.28f : 0.70f;
        g.setColour(Palette::amber.withAlpha(frame.lensHeld ? lensOpacity * 0.32f : 0.08f));
        g.fillEllipse(lens);
        g.setColour(Palette::amber.withAlpha(frame.lensHeld ? lensOpacity : 0.25f));
        g.drawEllipse(lens, 1.1f);
        if (frame.lensHeld)
        {
            const auto targetPoint = juce::Point<float> {
                sphere.getCentreX() + frame.lensTargetX * sphere.getWidth() * 0.44f,
                sphere.getCentreY() - frame.lensTargetY * sphere.getHeight() * 0.44f
            };
            g.setColour(Palette::amber.withAlpha(frame.lensTargetZ < 0.0f ? 0.30f : 0.65f));
            g.drawEllipse(juce::Rectangle<float>(7.0f, 7.0f).withCentre(targetPoint), 1.0f);
        }
    }

    // The sphere as a sonar plan-position indicator: range rings, bearings
    // around the rim (000-330), a sweep with the afterglow of a long-persistence
    // screen, and an expanding circle at every ping - here, at every act.
    void drawSonar(juce::Graphics& g, juce::Rectangle<float> sphere) const
    {
        const auto centre = sphere.getCentre();
        const auto radius = sphere.getWidth() * 0.5f;
        g.setColour(Palette::cyan.withAlpha(0.10f));
        for (const auto ring : { 0.25f, 0.5f, 0.75f })
            g.drawEllipse(juce::Rectangle<float>(radius * 2.0f * ring, radius * 2.0f * ring).withCentre(centre), 0.8f);
        g.setFont(uiFont(8.5f));
        for (int degrees = 0; degrees < 360; degrees += 10)
        {
            const auto angle = juce::degreesToRadians(static_cast<float>(degrees));
            const auto dx = std::sin(angle), dy = -std::cos(angle);
            const auto major = degrees % 30 == 0;
            const auto r1 = radius * (major ? 0.94f : 0.97f);
            g.setColour(Palette::cyan.withAlpha(major ? 0.45f : 0.25f));
            g.drawLine(centre.x + dx * r1, centre.y + dy * r1, centre.x + dx * radius, centre.y + dy * radius, major ? 1.1f : 0.7f);
            if (major)
            {
                const auto label = juce::Point<float>(centre.x + dx * (radius + 11.0f), centre.y + dy * (radius + 9.0f));
                g.setColour(Palette::cyan.withAlpha(0.45f));
                g.drawText(juce::String(degrees).paddedLeft('0', 3),
                           juce::Rectangle<float>(26.0f, 12.0f).withCentre(label), juce::Justification::centred);
            }
        }
        // the sweep and its afterglow
        if (frame.musicInfluence > 0.0f || frame.autonomousEnabled)
        {
            constexpr int trail = 28;
            for (int k = trail; k >= 0; --k)
            {
                const auto a0 = static_cast<float>(sweep) - 0.022f * static_cast<float>(k);
                juce::Path wedge;
                wedge.startNewSubPath(centre);
                wedge.addCentredArc(centre.x, centre.y, radius, radius, 0.0f, a0 - 0.022f, a0, false);
                wedge.closeSubPath();
                g.setColour(Palette::cyan.withAlpha(0.10f * (1.0f - static_cast<float>(k) / trail)));
                g.fillPath(wedge);
            }
            const auto dx = std::sin(static_cast<float>(sweep)), dy = -std::cos(static_cast<float>(sweep));
            g.setColour(Palette::cyan.withAlpha(0.55f));
            g.drawLine(centre.x, centre.y, centre.x + dx * radius, centre.y + dy * radius, 1.2f);
        }
        // the pings
        const auto now = juce::Time::getMillisecondCounterHiRes() / 1000.0;
        for (const auto started : pings)
        {
            const auto age = static_cast<float>(now - started);
            if (started <= 0.0 || age < 0.0f || age > 2.0f)
                continue;
            const auto r = radius * age / 2.0f;
            g.setColour(Palette::cyan.withAlpha(0.45f * (1.0f - age / 2.0f)));
            g.drawEllipse(juce::Rectangle<float>(r * 2.0f, r * 2.0f).withCentre(centre), 1.4f);
        }
    }

    void drawOrganisms(juce::Graphics& g, juce::Rectangle<float> sphere) const
    {
        static const std::array<juce::Colour, aquorbium::Ecosystem::organismCount> colours {
            Palette::cyan, Palette::amber, Palette::coral, Palette::violet,
            Palette::green
        };
        static const std::array<const char*, aquorbium::Ecosystem::organismCount> names {
            "MORPH", "GRAIN", "MODAL", "PULSAR", "STRING"
        };
        const auto radius = sphere.getWidth() * 0.44f;
        const auto centre = sphere.getCentre();
        // the deep sound channel (SoundChannel): a band across the sphere at
        // its axis height - along it sound travels far, across it falls into
        // shadow. It rises to the surface as the water cools.
        if (frame.musicInfluence > 0.0f)
        {
            juce::Graphics::ScopedSaveState clip(g);
            juce::Path globe;
            globe.addEllipse(centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f);
            g.reduceClipRegion(globe);
            const auto axisY = centre.y - juce::jlimit(-1.0f, 1.0f, frame.musicChannelAxis / 4.0f) * radius;
            const auto half = aquorbium::SoundChannel::halfWidth / 4.0f * radius;
            juce::ColourGradient band(Palette::cyan.withAlpha(0.0f), centre.x, axisY - half,
                                      Palette::cyan.withAlpha(0.0f), centre.x, axisY + half, false);
            band.addColour(0.5, Palette::cyan.withAlpha(0.10f));
            g.setGradientFill(band);
            g.fillRect(centre.x - radius, axisY - half, radius * 2.0f, half * 2.0f);
            g.setColour(Palette::cyan.withAlpha(0.35f));
            const float dashes[] { 6.0f, 5.0f };
            g.drawDashedLine({ centre.x - radius, axisY, centre.x + radius, axisY }, dashes, 2, 0.8f);
            g.setFont(uiFont(8.5f, juce::Font::bold));
            g.drawText("SOFAR", juce::Rectangle<float>(centre.x - radius * 0.62f, axisY - 13.0f, 60.0f, 12.0f),
                       juce::Justification::centredLeft);
        }
        std::array<juce::Point<float>, aquorbium::Ecosystem::organismCount> points;
        std::array<aquorbium::windowing::Bounds, aquorbium::Ecosystem::organismCount> bodies;
        for (std::size_t i = 0; i < frame.organisms.size(); ++i)
        {
            const auto& organism = frame.organisms[i];
            const auto depth = juce::jlimit(-1.0f, 1.0f, organism.z / 4.0f);
            points[i] = { centre.x + juce::jlimit(-1.0f, 1.0f, organism.x / 4.0f) * radius,
                          centre.y - juce::jlimit(-1.0f, 1.0f, organism.y / 4.0f) * radius
                              + depth * radius * 0.10f };
            const auto halo = 8.0f + 7.0f * juce::jlimit(0.0f, 1.0f, organism.energy)
                + 2.0f * depth + 7.0f + 11.0f * juce::jlimit(0.0f, 1.0f, organism.observation);
            const auto body = juce::Rectangle<float>(halo * 2.0f + 4.0f, halo * 2.0f + 4.0f)
                .withCentre(points[i]).getSmallestIntegerContainer();
            bodies[i] = { body.getX(), body.getY(), body.getWidth(), body.getHeight() };
        }
        auto labelArea = getLocalBounds().reduced(20);
        labelArea.removeFromTop(48);
        labelArea.removeFromBottom(42);
        if (const auto panel = computeAreas().panel; !panel.isEmpty())
            labelArea.setBottom(static_cast<int>(panel.getY()) - 4);
        const auto lensSize = sphere.getWidth() * 0.11f + 4.0f;
        const auto lensBounds = juce::Rectangle<float>(lensSize, lensSize).withCentre(
            { centre.x + frame.lensX * radius, centre.y - frame.lensY * radius })
            .getSmallestIntegerContainer();
        const auto labels = aquorbium::windowing::OrganismLabelLayout::calculate(
            { labelArea.getX(), labelArea.getY(), labelArea.getWidth(), labelArea.getHeight() }, bodies,
            { lensBounds.getX(), lensBounds.getY(), lensBounds.getWidth(), lensBounds.getHeight() });
        drawWeb(g, points);

        for (std::size_t i = 0; i < frame.organisms.size(); ++i)
        {
            const auto& organism = frame.organisms[i];
            const auto depth = juce::jlimit(-1.0f, 1.0f, organism.z / 4.0f);
            const auto point = points[i];
            const auto size = 8.0f + 7.0f * juce::jlimit(0.0f, 1.0f, organism.energy)
                + 2.0f * depth;
            const auto colour = colours[i];

            const auto observationRadius = size + 7.0f
                + 11.0f * juce::jlimit(0.0f, 1.0f, organism.observation);
            g.setColour(colour.withAlpha(juce::jlimit(
                0.0f, 1.0f, 0.10f + 0.18f * organism.detail)));
            g.fillEllipse(juce::Rectangle<float>(observationRadius * 2.0f, observationRadius * 2.0f)
                              .withCentre(point));
            g.setColour(colour.withAlpha(0.25f));
            g.drawLine(centre.x, centre.y, point.x, point.y, 0.7f);
            g.setColour(colour.withAlpha(0.92f));
            g.fillEllipse(juce::Rectangle<float>(size, size).withCentre(point));
            const auto energyRing = size + 4.0f
                + 13.0f * juce::jlimit(0.0f, 1.0f, organism.autonomousEnergy);
            g.setColour(colour.withAlpha(juce::jlimit(
                0.0f, 1.0f, 0.32f + organism.autonomousEnergy * 0.45f)));
            g.drawEllipse(juce::Rectangle<float>(energyRing, energyRing).withCentre(point), 1.2f);
            const auto spatialWidth = 24.0f;
            const auto spatialY = point.y + size * 0.72f + 5.0f;
            g.setColour(colour.withAlpha(juce::jlimit(
                0.0f, 1.0f,
                0.18f + (1.0f - organism.spatialDepth) * 0.26f)));
            g.drawLine(point.x - spatialWidth * 0.5f, spatialY,
                       point.x + spatialWidth * 0.5f, spatialY, 0.8f);
            const auto panMarker = point.x
                + juce::jlimit(-1.0f, 1.0f, organism.spatialPan) * spatialWidth * 0.5f;
            g.setColour(colour.withAlpha(0.82f));
            g.fillEllipse(panMarker - 1.7f, spatialY - 1.7f, 3.4f, 3.4f);
            g.setColour(Palette::text.withAlpha(0.92f));
            g.setFont(uiFont(11.0f));
            const auto organismLabel = juce::String(names[i]) + " "
                + juce::String(static_cast<int>(std::round(organism.autonomousPitch)))
                + utf8(" · ") + juce::String(organism.orbitStep + 1) + "/"
                + juce::String(organism.orbitLength)
                + (i == 1 && organism.activeParticles > 0
                       ? utf8(" · P") + juce::String(organism.activeParticles)
                       : juce::String())
                + (organism.geometricGate ? utf8(" •") : juce::String());
            if (labels[i].isValid())
            {
                const auto label = juce::Rectangle<int>(labels[i].x, labels[i].y,
                                                       labels[i].width, labels[i].height);
                g.setColour(colour.withAlpha(0.38f));
                g.drawLine(point.x, point.y, static_cast<float>(label.getX()),
                           static_cast<float>(label.getCentreY()), 0.7f);
                g.setColour(Palette::panelRaised.withAlpha(0.90f));
                g.fillRoundedRectangle(label.toFloat(), 3.0f);
                g.setColour(Palette::text.withAlpha(0.92f));
                g.drawText(organismLabel, label, juce::Justification::centredLeft);
            }
        }
    }

    void drawFooter(juce::Graphics& g, juce::Rectangle<float> area) const
    {
        g.setFont(uiFont(10.0f));
        g.setColour(frame.lensHeld ? Palette::cyan : Palette::muted);
        g.drawText(frame.lensHeld
                       ? aquorbium::app::T(aquorbium::app::S::lensActive) + juce::String(frame.lensFocusedOrganisms)
                           + aquorbium::app::T(aquorbium::app::S::lensDrag)
                       : aquorbium::app::T(aquorbium::app::S::lensFree),
                   area.removeFromTop(18.0f), juce::Justification::centred);
        const auto readout = [this](const juce::String& name, float value) {
            return name + " " + juce::String(static_cast<int>(std::round(value * 100.0f))) + "%";
        };
        g.setFont(uiFont(9.5f, juce::Font::bold));
        g.setColour(Palette::muted);
        const auto column = area.getWidth() / 5.0f;
        g.drawText(readout("MORPH", frame.ecosystemBlend), area.removeFromLeft(column), juce::Justification::centred);
        g.drawText(readout("ENERGY", frame.outputEnergy), area.removeFromLeft(column), juce::Justification::centred);
        g.drawText(readout("BIO", frame.autonomousEnergy), area.removeFromLeft(column), juce::Justification::centred);
        g.drawText("ROOT " + juce::String(frame.harmonicRoot) + utf8(" · ORG ")
                       + juce::String(frame.autonomousOrganisms),
                   area.removeFromLeft(column), juce::Justification::centred);
        const auto eventStatus = frame.droppedEvents == 0
            ? juce::String("EVENTS OK")
            : juce::String("DROPPED ") + juce::String(frame.droppedEvents);
        g.drawText("EXT " + juce::String(frame.activeVoices) + utf8(" · ") + eventStatus,
                   area, juce::Justification::centred);
    }

    struct LoggedAct
    {
        double seconds = 0.0;
        int act = 0;
    };
    VisualFrame frame;
    juce::TextButton waterType;
    juce::TextButton roomButton;
    juce::TextButton markButton;
    bool draggingLens = false;
    bool firstFrame = true;
    double startSeconds = 0.0;
    double lastFrameSeconds = 0.0;
    double sweep = 0.0;
    std::array<double, 6> pings {};
    std::size_t pingWrite = 0;
    std::array<std::array<float, 6>, 48> waterfall {};
    std::size_t waterfallWrite = 0;
    double lastWaterfallSeconds = 0.0;
    std::array<LoggedAct, 8> actLog {};
    std::size_t actLogWrite = 0;
};

class EcologyControlsBackdrop final : public juce::Component
{
public:
    void paint(juce::Graphics& g) override
    {
        const auto bounds = getLocalBounds().toFloat().reduced(0.75f);
        g.setColour(Palette::panelRaised);
        g.fillRoundedRectangle(bounds, 14.0f);
        g.setColour(Palette::green.withAlpha(0.55f));
        g.drawRoundedRectangle(bounds, 14.0f, 1.25f);
        g.setColour(Palette::text);
        g.setFont(uiFont(19.0f, juce::Font::bold));
        g.drawText("ECOLOGY", getLocalBounds().reduced(22).withTrimmedRight(145).removeFromTop(25),
                   juce::Justification::centredLeft);
    }
};
}

class SafeAudioSettingsComponent final : public juce::Component
{
public:
    explicit SafeAudioSettingsComponent(juce::AudioDeviceManager& managerIn)
        : manager(managerIn)
    {
        for (auto* component : { static_cast<juce::Component*>(&outputLabel),
                                 static_cast<juce::Component*>(&rateLabel),
                                 static_cast<juce::Component*>(&bufferLabel),
                                 static_cast<juce::Component*>(&output),
                                 static_cast<juce::Component*>(&rate),
                                 static_cast<juce::Component*>(&buffer),
                                 static_cast<juce::Component*>(&applyButton),
                                 static_cast<juce::Component*>(&status) })
            addAndMakeVisible(*component);
        outputLabel.setText(aquorbium::app::T(aquorbium::app::S::output), juce::dontSendNotification);
        rateLabel.setText(aquorbium::app::T(aquorbium::app::S::rate), juce::dontSendNotification);
        bufferLabel.setText(aquorbium::app::T(aquorbium::app::S::buffer), juce::dontSendNotification);
        for (auto* label : { &outputLabel, &rateLabel, &bufferLabel, &status })
        {
            label->setColour(juce::Label::textColourId, Palette::text);
            label->setFont(uiFont(13.0f));
        }
        for (auto* type : manager.getAvailableDeviceTypes())
        {
            type->scanForDevices();
            for (const auto& name : type->getDeviceNames(false))
            {
                outputs.push_back({ type->getTypeName(), name });
                output.addItem(type->getTypeName() + ": " + name,
                               static_cast<int>(outputs.size()));
            }
        }
        const auto setup = manager.getAudioDeviceSetup();
        for (std::size_t index = 0; index < outputs.size(); ++index)
            if (outputs[index].type == manager.getCurrentAudioDeviceType()
                && outputs[index].name == setup.outputDeviceName)
                output.setSelectedId(static_cast<int>(index + 1), juce::dontSendNotification);
        if (output.getSelectedId() == 0 && !outputs.empty())
            output.setSelectedId(1, juce::dontSendNotification);
        for (const auto hz : { 44100, 48000, 96000 })
            rate.addItem(juce::String(hz / 1000.0, 1) + " kHz", hz);
        const auto savedRate = static_cast<int>(setup.sampleRate);
        if (savedRate > 0 && rate.indexOfItemId(savedRate) < 0)
            rate.addItem(juce::String(savedRate) + " Hz", savedRate);
        rate.setSelectedId(savedRate > 0 ? savedRate : 48000, juce::dontSendNotification);
        output.onChange = [this] { updateBuffers(512); };
        updateBuffers(setup.bufferSize);
        applyButton.setButtonText(aquorbium::app::T(aquorbium::app::S::applyAudio));
        applyButton.onClick = [this] { applySelection(); };
        applyButton.setEnabled(!outputs.empty());
        status.setText(outputs.empty() ? utf8("Nenhuma saída de áudio encontrada")
                                       : utf8("PipeWire: mínimo 512 amostras"),
                       juce::dontSendNotification);
        status.setColour(juce::Label::textColourId, Palette::muted);
        setSize(520, 265);
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced(18);
        for (auto pair : { std::pair { &outputLabel, &output },
                           std::pair { &rateLabel, &rate },
                           std::pair { &bufferLabel, &buffer } })
        {
            auto row = area.removeFromTop(48);
            pair.first->setBounds(row.removeFromLeft(90));
            pair.second->setBounds(row.reduced(0, 6));
        }
        area.removeFromTop(8);
        applyButton.setBounds(area.removeFromTop(34).removeFromRight(170));
        area.removeFromTop(8);
        status.setBounds(area.removeFromTop(34));
    }

private:
    struct Output { juce::String type, name; };

    void updateBuffers(int preferred)
    {
        buffer.clear(juce::dontSendNotification);
        const auto selected = output.getSelectedId();
        const auto pipeWire = selected > 0
            && outputs[static_cast<std::size_t>(selected - 1)].name.containsIgnoreCase("PipeWire");
        if (!pipeWire)
            for (const auto size : { 128, 256 })
                buffer.addItem(juce::String(size) + aquorbium::app::T(aquorbium::app::S::samples), size);
        for (const auto size : { 512, 1024, 2048 })
            buffer.addItem(juce::String(size) + aquorbium::app::T(aquorbium::app::S::samples), size);
        buffer.setSelectedId(buffer.indexOfItemId(preferred) >= 0 ? preferred : 512,
                             juce::dontSendNotification);
        status.setText(pipeWire ? utf8("PipeWire: mínimo 512 amostras")
                                : utf8("Buffers menores são apenas para hardware direto"),
                       juce::dontSendNotification);
    }

    void applySelection()
    {
        const auto selected = output.getSelectedId();
        if (selected <= 0 || static_cast<std::size_t>(selected) > outputs.size()) return;
        const auto& destination = outputs[static_cast<std::size_t>(selected - 1)];
        auto previous = manager.createStateXml();
        juce::XmlElement requested("DEVICESETUP");
        requested.setAttribute("deviceType", destination.type);
        requested.setAttribute("audioOutputDeviceName", destination.name);
        requested.setAttribute("audioInputDeviceName", "");
        requested.setAttribute("audioDeviceRate", rate.getSelectedId());
        requested.setAttribute("audioDeviceBufferSize", buffer.getSelectedId());
        std::vector<juce::String> enabledMidi;
        const auto defaultMidiOutput = manager.getDefaultMidiOutputIdentifier();
        for (const auto& midi : juce::MidiInput::getAvailableDevices())
            if (manager.isMidiInputDeviceEnabled(midi.identifier))
                enabledMidi.push_back(midi.identifier);
        const auto error = manager.initialise(0, 2, &requested, false);
        if (error.isNotEmpty())
        {
            if (previous != nullptr)
            {
                if (needsPipeWireSafetyBuffer(
                        previous->getStringAttribute("audioOutputDeviceName"),
                        previous->getIntAttribute("audioDeviceBufferSize")))
                    previous->setAttribute("audioDeviceBufferSize", 512);
                static_cast<void>(manager.initialise(0, 2, previous.get(), false));
            }
            for (const auto& id : enabledMidi)
                manager.setMidiInputDeviceEnabled(id, true);
            if (defaultMidiOutput.isNotEmpty())
                manager.setDefaultMidiOutputDevice(defaultMidiOutput);
            status.setText(aquorbium::app::T(aquorbium::app::S::outputFailed) + error,
                           juce::dontSendNotification);
            status.setTooltip(error);
            return;
        }
        for (const auto& id : enabledMidi)
            manager.setMidiInputDeviceEnabled(id, true);
        if (defaultMidiOutput.isNotEmpty())
            manager.setDefaultMidiOutputDevice(defaultMidiOutput);
        status.setText(destination.name + "  " + juce::String(rate.getSelectedId())
            + " Hz / " + juce::String(buffer.getSelectedId()), juce::dontSendNotification);
    }

    juce::AudioDeviceManager& manager;
    std::vector<Output> outputs;
    juce::Label outputLabel, rateLabel, bufferLabel, status;
    juce::ComboBox output, rate, buffer;
    juce::TextButton applyButton;
};

// The TUTORIAL and ABOUT card (the RASGO pattern): contents on the left,
// the text with scrolling, the TIP in its own frame, previous/next. It opens
// in the instrument's language.
class GuideOverlay final : public juce::Component
{
public:
    enum class Kind { tutorial, about };
    std::function<void()> onClose;

    GuideOverlay()
    {
        heading.setFont(uiFont(20.0f, juce::Font::bold));
        heading.setColour(juce::Label::textColourId, Palette::cyan);
        addAndMakeVisible(heading);
        contentsHeading.setFont(uiFont(11.0f, juce::Font::bold));
        contentsHeading.setColour(juce::Label::textColourId, Palette::muted);
        addAndMakeVisible(contentsHeading);
        for (std::size_t i = 0; i < chapterButtons.size(); ++i)
        {
            chapterButtons[i].onClick = [this, i] { select(static_cast<int>(i)); };
            addAndMakeVisible(chapterButtons[i]);
        }
        body.setMultiLine(true, true);
        body.setReadOnly(true);
        body.setCaretVisible(false);
        body.setScrollbarsShown(true);
        body.setFont(uiFont(15.0f));
        body.setColour(juce::TextEditor::backgroundColourId, Palette::panel);
        body.setColour(juce::TextEditor::outlineColourId, Palette::panel);
        body.setColour(juce::TextEditor::textColourId, Palette::text);
        body.setIndents(14, 12);
        addAndMakeVisible(body);
        tipHeading.setFont(uiFont(12.0f, juce::Font::bold));
        tipHeading.setColour(juce::Label::textColourId, Palette::amber);
        addAndMakeVisible(tipHeading);
        tipBody.setFont(uiFont(14.0f));
        tipBody.setColour(juce::Label::textColourId, Palette::text);
        tipBody.setJustificationType(juce::Justification::topLeft);
        addAndMakeVisible(tipBody);
        previous.onClick = [this] { select(current - 1); };
        next.onClick = [this] { select(current + 1); };
        close.onClick = [this] { if (onClose) onClose(); };
        addAndMakeVisible(previous);
        addAndMakeVisible(next);
        addAndMakeVisible(close);
    }

    void show(Kind which)
    {
        kind = which;
        refresh();
        setVisible(true);
        toFront(true);
    }

    void refresh()
    {
        namespace Tu = aquorbium::app::tutorial;
        using aquorbium::app::T;
        const auto tutorial = kind == Kind::tutorial;
        heading.setText(tutorial ? T(aquorbium::app::S::tutorial) : T(aquorbium::app::about::title), juce::dontSendNotification);
        contentsHeading.setText(T(Tu::contents), juce::dontSendNotification);
        contentsHeading.setVisible(tutorial);
        for (std::size_t i = 0; i < chapterButtons.size(); ++i)
        {
            chapterButtons[i].setButtonText(juce::String(static_cast<int>(i) + 1) + ". " + T(Tu::chapters[i].title));
            chapterButtons[i].setVisible(tutorial);
            chapterButtons[i].setToggleState(static_cast<int>(i) == current, juce::dontSendNotification);
        }
        previous.setButtonText(T(Tu::previous));
        next.setButtonText(T(Tu::next));
        close.setButtonText(T(aquorbium::app::S::close));
        previous.setVisible(tutorial);
        next.setVisible(tutorial);
        tipHeading.setVisible(tutorial);
        tipBody.setVisible(tutorial);
        if (tutorial)
        {
            const auto& chapter = Tu::chapters[static_cast<std::size_t>(current)];
            body.setText(aquorbium::app::upper(T(chapter.title)) + "\n\n" + T(chapter.body), false);
            tipHeading.setText(T(Tu::tipHeading), juce::dontSendNotification);
            tipBody.setText(T(chapter.tip), juce::dontSendNotification);
            previous.setEnabled(current > 0);
            next.setEnabled(current + 1 < static_cast<int>(Tu::chapters.size()));
        }
        else
            body.setText(T(aquorbium::app::about::body) + "\n\n" + T(aquorbium::app::S::footerCredit)
                             + JUCE_APPLICATION_VERSION_STRING, false);
        body.moveCaretToTop(false);
        resized();
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(Palette::background.withAlpha(0.82f));
        g.setColour(Palette::panelRaised);
        g.fillRoundedRectangle(card.toFloat(), 14.0f);
        g.setColour(Palette::cyan.withAlpha(0.45f));
        g.drawRoundedRectangle(card.toFloat(), 14.0f, 1.3f);
    }

    void resized() override
    {
        card = getLocalBounds().withSizeKeepingCentre(std::min(1040, getWidth() - 40), std::min(660, getHeight() - 40));
        auto area = card.reduced(22);
        auto top = area.removeFromTop(36);
        close.setBounds(top.removeFromRight(110).reduced(0, 3));
        heading.setBounds(top);
        area.removeFromTop(10);
        const auto tutorial = kind == Kind::tutorial;
        if (tutorial)
        {
            auto left = area.removeFromLeft(250);
            area.removeFromLeft(16);
            contentsHeading.setBounds(left.removeFromTop(22));
            // up to 38 px each, fewer when the window is short (12 chapters
            // since the synthesis device: 2026-10-22)
            const auto buttonHeight = juce::jlimit(26, 38, left.getHeight() / static_cast<int>(chapterButtons.size()));
            for (auto& b : chapterButtons)
                b.setBounds(left.removeFromTop(buttonHeight).reduced(0, 3));
            auto bottom = area.removeFromBottom(36);
            next.setBounds(bottom.removeFromRight(130).reduced(0, 3));
            bottom.removeFromRight(8);
            previous.setBounds(bottom.removeFromRight(130).reduced(0, 3));
            area.removeFromBottom(8);
            auto tip = area.removeFromBottom(96);
            tipHeading.setBounds(tip.removeFromTop(20));
            tipBody.setBounds(tip);
            area.removeFromBottom(8);
        }
        body.setBounds(area);
    }

    // public for the layout audit
    [[nodiscard]] juce::Rectangle<int> cardBounds() const noexcept { return card; }
    void select(int index)
    {
        current = juce::jlimit(0, static_cast<int>(aquorbium::app::tutorial::chapters.size()) - 1, index);
        refresh();
    }

private:

    Kind kind = Kind::tutorial;
    int current = 0;
    juce::Rectangle<int> card;
    juce::Label heading, contentsHeading, tipHeading, tipBody;
    std::array<juce::TextButton, aquorbium::app::tutorial::chapters.size()> chapterButtons;
    juce::TextEditor body;
    juce::TextButton previous, next, close;
};

#include "app/SynthEditor.h"
#include "core/SynthPatchCodec.h"

// LEARN: the explanation of whatever the pointer rests on.
class LearnPanel final : public juce::Component
{
public:
    LearnPanel() { setInterceptsMouseClicks(false, false); }

    void setEntry(juce::String titleIn, juce::String bodyIn, juce::String exploreIn)
    {
        title = std::move(titleIn);
        body = std::move(bodyIn);
        explore = std::move(exploreIn);
        repaint();
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(0.5f);
        g.setColour(Palette::panelRaised.withAlpha(0.97f));
        g.fillRoundedRectangle(bounds, 10.0f);
        g.setColour(Palette::amber.withAlpha(0.6f));
        g.drawRoundedRectangle(bounds, 10.0f, 1.2f);
        auto area = getLocalBounds().reduced(14, 10);
        g.setColour(Palette::amber);
        g.setFont(uiFont(12.5f, juce::Font::bold));
        g.drawFittedText(title, area.removeFromTop(20), juce::Justification::centredLeft, 1);
        area.removeFromTop(4);
        auto exploreArea = explore.isEmpty() ? juce::Rectangle<int>() : area.removeFromBottom(34);
        g.setColour(Palette::text);
        g.setFont(uiFont(12.5f));
        g.drawFittedText(body, area, juce::Justification::topLeft, std::max(1, area.getHeight() / 15), 1.0f);
        if (!explore.isEmpty())
        {
            g.setColour(Palette::cyan);
            g.setFont(uiFont(11.5f, juce::Font::italic));
            g.drawFittedText(aquorbium::app::T(aquorbium::app::tryThis) + explore, exploreArea,
                             juce::Justification::topLeft, 2, 1.0f);
        }
    }

private:
    juce::String title, body, explore;
};

class MainComponent final : public juce::AudioAppComponent,
                            private juce::MidiInputCallback,
                            private juce::Timer
{
public:
    explicit MainComponent(juce::PropertiesFile& settingsIn, bool startAudio = true)
        : settings(settingsIn),
          brainCard("01", utf8("CÉREBRO"), utf8("AUTÔNOMO"), "Incerteza e estado",
                    utf8("Bioenergia e decisões internas; nenhuma nota externa é necessária."), Palette::cyan),
          geometryCard("02", "GEOMETRIA", "FORMA BYPASS", utf8("Órbitas generativas"),
                       utf8("Órbitas, memória de forma, passos e progressão harmônica."),
                       Palette::amber, true),
          synthesisCard("03", utf8("SÍNTESE"), "ATIVO", utf8("Vozes orgânicas"),
                        utf8("Morph, Striker, Modal, Pulsar e String físico."),
                        Palette::coral, true),
          spaceCard("04", utf8("ESPAÇO"), "ATIVO", utf8("Perspectiva e memória"),
                    utf8("Pan, profundidade e oito reflexos compartilhados que respondem ao Bioma."), Palette::violet, true),
          environmentCard("05", "AMBIENTE", "BYPASS", utf8("Assinatura e leito"),
                          utf8("Filtro espectral e atmosfera ecológica audível."),
                          Palette::cyan, true),
          tailCard("06", "CAUDA", "BYPASS", utf8("Persistência sintética"),
                   utf8("Reverberação tardia difusa após as primeiras reflexões."),
                   Palette::violet, true),
          masterCard("07", "MASTER", "SAFETY ACTIVE", utf8("Saída e integridade"),
                     utf8("Trim técnico, teto de segurança e recuperação do limitador."),
                     Palette::green, true),
          moduleBankCard("08", utf8("MÓDULOS"), "LAB BYPASS", utf8("Banco integrado"),
                         utf8("Todos os protótipos codificados, reunidos em quinze famílias operáveis."),
                         Palette::cyan, true),
          timbreCard("09", "TIMBRE", "NATIVO", utf8("Corpos dos organismos"),
                     utf8("Cada organismo pode cantar com um segundo corpo - voz, matéria, sopro, corda, gota ou dobra - misturado ao seu corpo nativo."),
                     Palette::coral, true),
          meterCard("10", "MEDIDORES", utf8("SAÍDA"), utf8("A saída medida"),
                    utf8("Loudness, true peak, limitador, correlação e os taps de realimentação."),
                    Palette::green, true),
          lensCard("11", "LUPA", "VIDRO", utf8("A lupa no vidro"),
                   utf8("Raio, alcance, ampliação e desfoque da lupa."), Palette::cyan, true),
          brainRoomCard("12", utf8("CÉREBRO · SALA"), "", utf8("Relógio e coordenação; o microfone"),
                        utf8("O Cérebro e a Sala, quando não cabem no painel de ecologia."), Palette::cyan, true)
    {
        setLookAndFeel(&lookAndFeel);
        for (auto* component : { static_cast<juce::Component*>(&brainCard),
                                 static_cast<juce::Component*>(&geometryCard),
                                 static_cast<juce::Component*>(&synthesisCard),
                                 static_cast<juce::Component*>(&spaceCard),
                                 static_cast<juce::Component*>(&environmentCard),
                                 static_cast<juce::Component*>(&tailCard),
                                 static_cast<juce::Component*>(&masterCard),
                                 static_cast<juce::Component*>(&moduleBankCard),
                                 static_cast<juce::Component*>(&ecosystemView),
                                 static_cast<juce::Component*>(&autonomy),
                                 static_cast<juce::Component*>(&gate),
                                 static_cast<juce::Component*>(&density),
                                 static_cast<juce::Component*>(&drift),
                                 static_cast<juce::Component*>(&ecosystemMorph),
                                 static_cast<juce::Component*>(&geometryInfluence),
                                 static_cast<juce::Component*>(&rhythmDensity),
                                 static_cast<juce::Component*>(&orbitRotation),
                                 static_cast<juce::Component*>(&intervalBias),
                                 static_cast<juce::Component*>(&geometryMutation),
                                 static_cast<juce::Component*>(&strikerInfluence),
                                 static_cast<juce::Component*>(&strikerMatter),
                                 static_cast<juce::Component*>(&strikerPopulation),
                                 static_cast<juce::Component*>(&strikerScatter),
                                 static_cast<juce::Component*>(&spaceReturn),
                                 static_cast<juce::Component*>(&spaceGesture),
                                 static_cast<juce::Component*>(&spaceSize),
                                 static_cast<juce::Component*>(&spaceSpread),
                                 static_cast<juce::Component*>(&spaceTone),
                                 static_cast<juce::Component*>(&spaceMemory),
                                 static_cast<juce::Component*>(&environmentInfluence),
                                 static_cast<juce::Component*>(&environmentFilterMix),
                                 static_cast<juce::Component*>(&environmentCentre),
                                 static_cast<juce::Component*>(&environmentSpan),
                                 static_cast<juce::Component*>(&environmentResonance),
                                 static_cast<juce::Component*>(&environmentAmbience),
                                 static_cast<juce::Component*>(&tailInfluence),
                                 static_cast<juce::Component*>(&tailDecay),
                                 static_cast<juce::Component*>(&tailDamping),
                                 static_cast<juce::Component*>(&tailNightmare),
                                 static_cast<juce::Component*>(&densityLabel),
                                 static_cast<juce::Component*>(&driftLabel),
                                 static_cast<juce::Component*>(&ecosystemMorphLabel),
                                 static_cast<juce::Component*>(&geometryInfluenceLabel),
                                 static_cast<juce::Component*>(&rhythmDensityLabel),
                                 static_cast<juce::Component*>(&orbitRotationLabel),
                                 static_cast<juce::Component*>(&intervalBiasLabel),
                                 static_cast<juce::Component*>(&geometryMutationLabel) })
            addAndMakeVisible(*component);

        addAndMakeVisible(geometryMemoryViewport);
        addAndMakeVisible(synthesisViewport);
        addAndMakeVisible(spaceViewport);
        addAndMakeVisible(environmentViewport);
        addAndMakeVisible(tailViewport);
        addAndMakeVisible(masterViewport);
        addAndMakeVisible(moduleBankViewport);
        addAndMakeVisible(lensViewport);
        lensViewport.setViewedComponent(&lensControls, false);
        geometryMemoryViewport.setViewedComponent(&geometryMemoryControls, false);
        synthesisViewport.setViewedComponent(&synthesisControls, false);
        spaceViewport.setViewedComponent(&spaceControls, false);
        environmentViewport.setViewedComponent(&environmentControls, false);
        tailViewport.setViewedComponent(&tailControls, false);
        masterViewport.setViewedComponent(&masterControls, false);
        moduleBankViewport.setViewedComponent(&moduleBankControls, false);
        for (auto* viewport : { &geometryMemoryViewport, &synthesisViewport, &spaceViewport,
                                &environmentViewport, &tailViewport, &masterViewport,
                                &moduleBankViewport, &lensViewport })
        {
            viewport->setScrollBarsShown(true, true);
            viewport->setScrollBarThickness(aquorbium::windowing::MacroPanelLayout::scrollBarWidth);
        }
        addAndMakeVisible(rightColumnViewport);
        rightColumnViewport.setViewedComponent(&rightColumnControls, false);
        rightColumnViewport.setScrollBarsShown(true, false);
        rightColumnViewport.setScrollBarThickness(12);
        for (auto* component : { static_cast<juce::Component*>(&synthesisCard),
                                 static_cast<juce::Component*>(&spaceCard),
                                 static_cast<juce::Component*>(&environmentCard),
                                 static_cast<juce::Component*>(&tailCard),
                                 static_cast<juce::Component*>(&masterCard),
                                 static_cast<juce::Component*>(&moduleBankCard),
                                 static_cast<juce::Component*>(&synthesisViewport),
                                 static_cast<juce::Component*>(&spaceViewport),
                                 static_cast<juce::Component*>(&environmentViewport),
                                 static_cast<juce::Component*>(&masterViewport),
                                 static_cast<juce::Component*>(&moduleBankViewport) })
            rightColumnControls.addAndMakeVisible(*component);
        rightColumnControls.addAndMakeVisible(tailViewport);
        rightColumnControls.addAndMakeVisible(timbreCard);
        rightColumnControls.addAndMakeVisible(timbreControls);
        configureTimbreControls();
        for (std::size_t index = 0; index < moduleBankSliders.size(); ++index)
        {
            moduleBankControls.addAndMakeVisible(moduleBankSliders[index]);
            moduleBankControls.addAndMakeVisible(moduleBankLabels[index]);
        }
        for (auto* component : { static_cast<juce::Component*>(&formInfluence),
                                 static_cast<juce::Component*>(&formContext),
                                 static_cast<juce::Component*>(&formInertia),
                                 static_cast<juce::Component*>(&formMutation),
                                 static_cast<juce::Component*>(&structureInfluence),
                                 static_cast<juce::Component*>(&structureMemory),
                                 static_cast<juce::Component*>(&structureHarmony),
                                 static_cast<juce::Component*>(&structurePhase),
                                 static_cast<juce::Component*>(&formInfluenceLabel),
                                 static_cast<juce::Component*>(&formContextLabel),
                                 static_cast<juce::Component*>(&formInertiaLabel),
                                 static_cast<juce::Component*>(&formMutationLabel),
                                 static_cast<juce::Component*>(&structureInfluenceLabel),
                                 static_cast<juce::Component*>(&structureMemoryLabel),
                                 static_cast<juce::Component*>(&structureHarmonyLabel),
                                 static_cast<juce::Component*>(&structurePhaseLabel),
                                 static_cast<juce::Component*>(&musicInfluence),
                                 static_cast<juce::Component*>(&musicReaction),
                                 static_cast<juce::Component*>(&musicMemory),
                                 static_cast<juce::Component*>(&musicWater),
                                 static_cast<juce::Component*>(&musicInfluenceLabel),
                                 static_cast<juce::Component*>(&musicReactionLabel),
                                 static_cast<juce::Component*>(&musicMemoryLabel),
                                 static_cast<juce::Component*>(&musicWaterLabel) })
            geometryMemoryControls.addAndMakeVisible(*component);
        for (auto* component : { static_cast<juce::Component*>(&strikerInfluence),
                                 static_cast<juce::Component*>(&strikerMatter),
                                 static_cast<juce::Component*>(&strikerPopulation),
                                 static_cast<juce::Component*>(&strikerScatter),
                                 static_cast<juce::Component*>(&strikerInfluenceLabel),
                                 static_cast<juce::Component*>(&strikerMatterLabel),
                                 static_cast<juce::Component*>(&strikerPopulationLabel),
                                 static_cast<juce::Component*>(&strikerScatterLabel) })
            synthesisControls.addAndMakeVisible(*component);
        for (auto* component : { static_cast<juce::Component*>(&spaceReturn),
                                 static_cast<juce::Component*>(&spaceGesture),
                                 static_cast<juce::Component*>(&spaceSize),
                                 static_cast<juce::Component*>(&spaceSpread),
                                 static_cast<juce::Component*>(&spaceTone),
                                 static_cast<juce::Component*>(&spaceMemory),
                                 static_cast<juce::Component*>(&spaceReturnLabel),
                                 static_cast<juce::Component*>(&spaceGestureLabel),
                                 static_cast<juce::Component*>(&spaceSizeLabel),
                                 static_cast<juce::Component*>(&spaceSpreadLabel),
                                 static_cast<juce::Component*>(&spaceToneLabel),
                                 static_cast<juce::Component*>(&spaceMemoryLabel) })
            spaceControls.addAndMakeVisible(*component);
        for (auto* component : { static_cast<juce::Component*>(&environmentInfluence),
                                 static_cast<juce::Component*>(&environmentFilterMix),
                                 static_cast<juce::Component*>(&environmentCentre),
                                 static_cast<juce::Component*>(&environmentSpan),
                                 static_cast<juce::Component*>(&environmentResonance),
                                 static_cast<juce::Component*>(&environmentAmbience),
                                 static_cast<juce::Component*>(&environmentInfluenceLabel),
                                 static_cast<juce::Component*>(&environmentFilterMixLabel),
                                 static_cast<juce::Component*>(&environmentCentreLabel),
                                 static_cast<juce::Component*>(&environmentSpanLabel),
                                 static_cast<juce::Component*>(&environmentResonanceLabel),
                                 static_cast<juce::Component*>(&environmentAmbienceLabel) })
            environmentControls.addAndMakeVisible(*component);
        for (auto* component : { static_cast<juce::Component*>(&tailInfluence),
                                 static_cast<juce::Component*>(&tailDecay),
                                 static_cast<juce::Component*>(&tailDamping),
                                 static_cast<juce::Component*>(&tailNightmare),
                                 static_cast<juce::Component*>(&tailInfluenceLabel),
                                 static_cast<juce::Component*>(&tailDecayLabel),
                                 static_cast<juce::Component*>(&tailDampingLabel),
                                 static_cast<juce::Component*>(&tailNightmareLabel) })
            tailControls.addAndMakeVisible(*component);
        for (auto* component : { static_cast<juce::Component*>(&creativeMaster),
                                 static_cast<juce::Component*>(&masterTrim),
                                 static_cast<juce::Component*>(&masterCeiling),
                                 static_cast<juce::Component*>(&masterRelease),
                                 static_cast<juce::Component*>(&masterEqOrder),
                                 static_cast<juce::Component*>(&creativeMasterLabel),
                                 static_cast<juce::Component*>(&masterTrimLabel),
                                 static_cast<juce::Component*>(&masterCeilingLabel),
                                 static_cast<juce::Component*>(&masterReleaseLabel),
                                 static_cast<juce::Component*>(&masterEqOrderLabel) })
            masterControls.addAndMakeVisible(*component);
        configureCardControls();

        for (std::size_t index = 0; index < moduleBankSliders.size(); ++index)
        {
            configureMacro(moduleBankSliders[index], moduleBankLabels[index],
                           moduleBankDescriptors[index].label, 0.0,
                           utf8(moduleBankDescriptors[index].detail), Palette::cyan);
            moduleBankTargets[index].store(0.0f, std::memory_order_relaxed);
        }

        addAndMakeVisible(audioSettingsButton);
        audioSettingsButton.setButtonText(utf8("ÁUDIO"));
        audioSettingsButton.setTooltip(utf8("Selecionar saída, taxa e buffer. Para diagnosticar estalos, tente 48 kHz / 512 amostras."));
        audioSettingsButton.onClick = [this] { showAudioSettings(); };
        // SILENCIAR: the aquarium keeps living, only the sound is closed.
        // PAUSA: the sound fades and the ecosystem's time stops until resumed.
        // Both ramp over 20 ms, so neither clicks.
        addAndMakeVisible(muteButton);
        muteButton.setClickingTogglesState(true);
        muteButton.setButtonText("SILENCIAR");
        muteButton.setTooltip(utf8("Fecha o som com uma rampa curta; o ecossistema continua vivendo."));
        muteButton.onClick = [this] {
            const auto on = muteButton.getToggleState();
            muteRequested.store(on, std::memory_order_relaxed);
            muteButton.setButtonText(on ? aquorbium::app::T(aquorbium::app::S::unmute) : aquorbium::app::T(aquorbium::app::S::mute));
        };
        addAndMakeVisible(pauseButton);
        pauseButton.setClickingTogglesState(true);
        pauseButton.setButtonText("PAUSA");
        pauseButton.setTooltip(utf8("Silencia e congela o tempo do ecossistema; CONTINUAR retoma de onde parou."));
        pauseButton.onClick = [this] {
            const auto on = pauseButton.getToggleState();
            pauseRequested.store(on, std::memory_order_relaxed);
            pauseButton.setButtonText(on ? aquorbium::app::T(aquorbium::app::S::resume) : aquorbium::app::T(aquorbium::app::S::pause));
        };

        // CAIXAS / FONES: the stereo mix, or each organism through a head
        addAndMakeVisible(listeningButton);
        listeningButton.setClickingTogglesState(true);
        listeningButton.onClick = [this] {
            const auto phones = listeningButton.getToggleState();
            headphonesRequested.store(phones, std::memory_order_relaxed);
            settings.setValue("audio.headphones", phones);
            listeningButton.setButtonText(aquorbium::app::T(phones ? aquorbium::app::S::headphones : aquorbium::app::S::speakers));
        };
        {
            const auto phones = settings.getBoolValue("audio.headphones", false);
            listeningButton.setToggleState(phones, juce::dontSendNotification);
            headphonesRequested.store(phones, std::memory_order_relaxed);
        }

        // REC: what is heard, to the 4 TB disk by default (Music/Aquorbium is
        // a link to AUDIOS_RASGO/Aquorbium on the author's machine)
        addAndMakeVisible(variaButton);
        variaButton.setButtonText("VARIA");
        variaButton.setColour(juce::ToggleButton::textColourId, Palette::text);
        variaButton.setColour(juce::ToggleButton::tickColourId, Palette::cyan);
        variaButton.setToggleState(settings.getBoolValue("varia.enabled", true), juce::dontSendNotification);
        variaButton.onClick = [this] {
            settings.setValue("varia.enabled", variaButton.getToggleState());
            if (variaButton.getToggleState())
                inhabitMotion();  // the hand starts from where everything is now
        };
        addAndMakeVisible(variaIntensity);
        variaIntensity.setSliderStyle(juce::Slider::LinearHorizontal);
        variaIntensity.setTextBoxStyle(juce::Slider::TextBoxRight, false, 52, 20);
        variaIntensity.setRange(0.0, 2.0, 0.01);
        variaIntensity.setDoubleClickReturnValue(true, 1.0);
        variaIntensity.setColour(juce::Slider::trackColourId, Palette::cyan.withAlpha(0.78f));
        variaIntensity.setColour(juce::Slider::backgroundColourId, Palette::line.withAlpha(0.46f));
        variaIntensity.setColour(juce::Slider::thumbColourId, Palette::text);
        variaIntensity.setColour(juce::Slider::textBoxTextColourId, Palette::text);
        variaIntensity.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        variaIntensity.setValue(juce::jlimit(0.0, 2.0, settings.getDoubleValue("varia.intensity", 1.0)),
                                juce::dontSendNotification);
        controlMotion.setIntensity(static_cast<float>(variaIntensity.getValue()));
        variaIntensity.onValueChange = [this] {
            controlMotion.setIntensity(static_cast<float>(variaIntensity.getValue()));
            settings.setValue("varia.intensity", variaIntensity.getValue());
        };

        addAndMakeVisible(recButton);
        recButton.setButtonText("REC");
        recButton.onClick = [this] {
            if (recorder == nullptr)
                startRecording();
            else
                stopRecording();
        };

        addAndMakeVisible(ecologyControlsBackdrop);
        addAndMakeVisible(ecologyControlsToggle);

        for (auto* component : { static_cast<juce::Component*>(&cohesion),
                                 static_cast<juce::Component*>(&danger),
                                 static_cast<juce::Component*>(&sparseFocus),
                                 static_cast<juce::Component*>(&energyMemory),
                                 static_cast<juce::Component*>(&geometricCycle),
                                 static_cast<juce::Component*>(&collectiveDrift),
                                 static_cast<juce::Component*>(&cohesionLabel),
                                 static_cast<juce::Component*>(&dangerLabel),
                                 static_cast<juce::Component*>(&sparseFocusLabel),
                                 static_cast<juce::Component*>(&energyMemoryLabel),
                                 static_cast<juce::Component*>(&geometricCycleLabel),
                                 static_cast<juce::Component*>(&collectiveDriftLabel) })
            addAndMakeVisible(*component);

        musicSalinity.setRange(0.0, 1.0, 0.0);
        musicSalinity.setValue(1.0, juce::dontSendNotification);
        ecosystemView.onFreshWater = [this](bool fresh) {
            musicSalinity.setValue(fresh ? 0.0 : 1.0, juce::sendNotificationSync);
        };
        // the musician's soundmark: the audio thread takes the phrase sounding now
        ecosystemView.onMark = [this] { soundmarkChoices.fetch_add(1, std::memory_order_relaxed); };
        // RoomCoupling: off by default; the input opens only when asked
        ecosystemView.onRoom = [this](bool on) {
            settings.setValue("room.enabled", on);
            openRoomInput(on);
        };
        ecosystemView.onLensGesture = [this](bool held, aquorbium::Ecosystem::Vec3 target) {
            lensTargetX.store(target.x, std::memory_order_relaxed);
            lensTargetY.store(target.y, std::memory_order_relaxed);
            lensTargetZ.store(target.z, std::memory_order_relaxed);
            lensHeldTarget.store(held, std::memory_order_relaxed);
            submitControlFrame();
        };

        ecologyControlsToggle.setButtonText("ECO CONTROLS");
        ecologyControlsToggle.setColour(juce::TextButton::buttonColourId, Palette::panelRaised);
        ecologyControlsToggle.setColour(juce::TextButton::textColourOffId, Palette::green);
        ecologyControlsToggle.onClick = [this] {
            showEcologyControls = !showEcologyControls;
            updateEcologySurface();
        };

        autonomy.setButtonText("AUTONOMOUS BIOMA");
        autonomy.setColour(juce::ToggleButton::textColourId, Palette::text);
        autonomy.setColour(juce::ToggleButton::tickColourId, Palette::cyan);
        autonomy.setToggleState(true, juce::dontSendNotification);
        autonomy.onClick = [this] {
            autonomyTarget.store(autonomy.getToggleState(), std::memory_order_relaxed);
            submitControlFrame();
        };

        gate.setButtonText("EXTERNAL ENERGY GATE");
        gate.setColour(juce::ToggleButton::textColourId, Palette::text);
        gate.setColour(juce::ToggleButton::tickColourId, Palette::cyan);
        gate.onClick = [this] { queueEvent({ gate.getToggleState(), 48, 0.8f }); };

        configureControl(density, densityLabel, "DENSITY", 0.45);
        configureControl(drift, driftLabel, "DRIFT", 0.20);
        configureControl(ecosystemMorph, ecosystemMorphLabel,
                         utf8("NEUTRAL  ←  ECOSYSTEM MORPH  →  ACTIVE"), 1.0);
        configureControl(cohesion, cohesionLabel, "COHESION", 0.5);
        configureControl(danger, dangerLabel, "DANGER", 0.5);
        configureControl(sparseFocus, sparseFocusLabel, "SPARSE FOCUS", 0.25);
        configureControl(energyMemory, energyMemoryLabel, "ENERGY MEMORY", 0.25);
        configureControl(geometricCycle, geometricCycleLabel, "GEOMETRIC CYCLE", 0.25);
        configureControl(collectiveDrift, collectiveDriftLabel, "COLLECTIVE DRIFT", 0.25);
        configureControl(geometryInfluence, geometryInfluenceLabel,
                         "GEOMETRY PERFORMANCE", 0.0);
        configureControl(rhythmDensity, rhythmDensityLabel, "RHYTHM DENSITY", 0.5);
        configureControl(orbitRotation, orbitRotationLabel, "ORBIT ROTATION", 0.5);
        configureControl(intervalBias, intervalBiasLabel, "INTERVAL BIAS", 0.5);
        configureControl(geometryMutation, geometryMutationLabel, "MUTATION", 0.0);
        configureMacro(formInfluence, formInfluenceLabel, "FORM MIX", 0.0,
                       utf8("Zero: memória formal transparente. Aumente para deslocar seção e contorno nos próximos eventos coletivos."),
                       Palette::amber);
        configureMacro(formContext, formContextLabel, "CONTEXT", 0.5,
                       utf8("Quanto o estado ecológico atrai a próxima região formal"),
                       Palette::amber);
        configureMacro(formInertia, formInertiaLabel, "INERTIA", 0.35,
                       utf8("Tendência de permanecer na região formal atual"),
                       Palette::amber);
        configureMacro(formMutation, formMutationLabel, "MUTATION", 0.05,
                       utf8("Probabilidade distribuída de saltos entre regiões formais"),
                       Palette::amber);
        configureMacro(structureInfluence, structureInfluenceLabel, "STRUCTURE MIX", 0.0,
                       utf8("Zero: caminho anterior intacto. Aumente para ativar memória de passos e progressão."),
                       Palette::amber);
        configureMacro(structureMemory, structureMemoryLabel, "STEP MEMORY", 0.5,
                       utf8("Contribuição da memória endereçável para altura e duração dos eventos"),
                       Palette::amber);
        configureMacro(structureHarmony, structureHarmonyLabel, "HARMONY", 0.5,
                       utf8("Contribuição dos acordes ponderados para o campo de alturas"),
                       Palette::amber);
        configureMacro(structurePhase, structurePhaseLabel, "PHASE PATH", 0.5,
                       utf8("Percurso ecológico: dia à esquerda, estação à direita"),
                       Palette::amber);
        configureMacro(musicInfluence, musicInfluenceLabel, utf8("MÚSICA"), 0.0,
                       utf8("Zero: rota anterior intacta. Aumente para que a música venha do ecossistema: caça, fuga, corte, território, a teia de relações."),
                       Palette::cyan);
        configureMacro(musicReaction, musicReactionLabel, utf8("REAÇÃO"), 0.6,
                       utf8("O quanto os organismos reagem ao que veem e ouvem através da água"),
                       Palette::cyan);
        configureMacro(musicMemory, musicMemoryLabel, utf8("MEMÓRIA"), 0.5,
                       utf8("Esquerda: os cantos se repetem (identidade). Direita: transformam-se e se copiam entre aliados."),
                       Palette::cyan);
        configureMacro(musicWater, musicWaterLabel, utf8("ÁGUA"), 0.5,
                       utf8("Centro: a água segue a estação. Esquerda esfria até o gelo; direita aquece até a fervura. A mudança de estado leva o tempo do calor latente."),
                       Palette::cyan);
        configureMacro(strikerInfluence, strikerInfluenceLabel, "STRIKER MIX", 0.0,
                       utf8("Zero: Striker autônomo. Aumente para ativar MATTER, POP e SCATTER; respondem nos próximos impactos."));
        configureMacro(strikerMatter, strikerMatterLabel, "MATTER", 0.5,
                       "Poeira curta para metal ressonante");
        configureMacro(strikerPopulation, strikerPopulationLabel, "POP", 0.5,
                       utf8("Quantidade de partículas por impacto"));
        configureMacro(strikerScatter, strikerScatterLabel, "SCATTER", 0.5,
                       utf8("Dispersão temporal e espectral"));
        configureMacro(spaceReturn, spaceReturnLabel, "RETURN", 0.48,
                       utf8("Quantidade audível do espaço compartilhado"),
                       Palette::violet);
        configureMacro(spaceGesture, spaceGestureLabel, "SPACE MIX", 0.0,
                       utf8("Zero: Espaço autônomo. Aumente para ativar SIZE, SPREAD, TONE e MEMORY. RETURN funciona independentemente."),
                       Palette::violet);
        configureMacro(spaceSize, spaceSizeLabel, "SIZE", 0.5,
                       utf8("Escala temporal dos reflexos"), Palette::violet);
        configureMacro(spaceSpread, spaceSpreadLabel, "SPREAD", 1.0,
                       utf8("Abertura lateral das reflexões"), Palette::violet);
        configureMacro(spaceTone, spaceToneLabel, "TONE", 0.5,
                       utf8("Abertura espectral dos reflexos"), Palette::violet);
        configureMacro(spaceMemory, spaceMemoryLabel, "MEMORY", 0.35,
                       utf8("Persistência do campo de reflexões"), Palette::violet);
        configureMacro(environmentInfluence, environmentInfluenceLabel, "ENV MIX", 0.0,
                       utf8("Zero: bypass transparente. Aumente para ativar filtro e atmosfera."),
                       Palette::cyan);
        configureMacro(environmentFilterMix, environmentFilterMixLabel, "FILTER", 0.5,
                       utf8("Mistura seca para assinatura espectral de oito bandas"), Palette::cyan);
        configureMacro(environmentCentre, environmentCentreLabel, "CENTER", 0.5,
                       utf8("Centro logarítmico do banco espectral"), Palette::cyan);
        configureMacro(environmentSpan, environmentSpanLabel, "SPAN", 0.5,
                       utf8("Dispersão em oitavas das oito bandas"), Palette::cyan);
        configureMacro(environmentResonance, environmentResonanceLabel, "RESONANCE", 0.45,
                       utf8("Seletividade das bandas espectrais"), Palette::cyan);
        configureMacro(environmentAmbience, environmentAmbienceLabel, "AMBIENCE", 0.0,
                       utf8("Leito de ruído guiado por estação, água e integridade"), Palette::cyan);
        configureMacro(tailInfluence, tailInfluenceLabel, "TAIL MIX", 0.0,
                       utf8("Zero: bypass transparente. Aumente para ouvir a cauda tardia."),
                       Palette::violet);
        configureMacro(tailDecay, tailDecayLabel, "DECAY", 0.45,
                       utf8("Duração da persistência sintética"), Palette::violet);
        configureMacro(tailDamping, tailDampingLabel, "DAMPING", 0.55,
                       utf8("Absorção progressiva das frequências altas"), Palette::violet);
        configureMacro(tailNightmare, tailNightmareLabel, "NIGHTMARE", 0.0,
                       utf8("Torção da matriz de realimentação"), Palette::violet);
        configureMacro(creativeMaster, creativeMasterLabel, "CREATIVE MASTER", 0.0,
                       utf8("Macro de crossover, dinâmica multibanda, largura e cola óptica. Zero é bypass exato."),
                       Palette::green);
        configureMacro(masterTrim, masterTrimLabel, "OUTPUT TRIM", 0.75,
                       utf8("Nível técnico final: -18 dB a +6 dB; 0,75 corresponde a 0 dB."),
                       Palette::green);
        configureMacro(masterCeiling, masterCeilingLabel, "CEILING", 5.0 / 5.9,
                       utf8("Teto do limitador: -6 dB a -0,1 dB; inicial em -1 dB."),
                       Palette::green);
        configureMacro(masterRelease, masterReleaseLabel, "RELEASE", 100.0 / 450.0,
                       utf8("Recuperação do limitador: 50 ms a 500 ms; inicial em 150 ms."),
                       Palette::green);
        configureMacro(masterEqOrder, masterEqOrderLabel, "EQ ORDER", 0.0,
                       utf8("Onde fica o EQ de inclinação do MASTER CRIATIVO: antes ou depois da dinâmica multibanda."),
                       Palette::green);
        for (auto* slider : { &creativeMaster, &masterTrim, &masterCeiling, &masterRelease })
            slider->setRange(0.0, 1.0, 0.0);
        // two positions, not a continuum: the order is a choice (principle 4ms LEQA)
        masterEqOrder.setRange(0.0, 1.0, 1.0);
        masterEqOrder.textFromValueFunction = [](double value) {
            return value >= 0.5 ? aquorbium::app::T(aquorbium::app::S::eqAfter)
                                : aquorbium::app::T(aquorbium::app::S::eqBefore);
        };
        masterEqOrder.valueFromTextFunction = [](const juce::String& text) {
            return text.containsIgnoreCase(aquorbium::app::T(aquorbium::app::S::eqAfter)) ? 1.0 : 0.0;
        };
        creativeMaster.textFromValueFunction = [](double value) {
            return juce::String(value, 2);
        };
        creativeMaster.valueFromTextFunction = [](const juce::String& text) {
            return juce::jlimit(0.0, 1.0, text.getDoubleValue());
        };
        masterTrim.textFromValueFunction = [](double value) {
            return juce::String(-18.0 + value * 24.0, 1) + " dB";
        };
        masterTrim.valueFromTextFunction = [](const juce::String& text) {
            return juce::jlimit(0.0, 1.0, (text.getDoubleValue() + 18.0) / 24.0);
        };
        masterCeiling.textFromValueFunction = [](double value) {
            return juce::String(-6.0 + value * 5.9, 1) + " dB";
        };
        masterCeiling.valueFromTextFunction = [](const juce::String& text) {
            return juce::jlimit(0.0, 1.0, (text.getDoubleValue() + 6.0) / 5.9);
        };
        masterRelease.textFromValueFunction = [](double value) {
            return juce::String(juce::roundToInt(50.0 + value * 450.0)) + " ms";
        };
        masterRelease.valueFromTextFunction = [](const juce::String& text) {
            return juce::jlimit(0.0, 1.0, (text.getDoubleValue() - 50.0) / 450.0);
        };
        creativeMaster.updateText();
        masterTrim.updateText();
        masterCeiling.updateText();
        masterRelease.updateText();
        masterEqOrder.updateText();
        for (const auto& binding : ecologyBindings())
            binding.slider->onValueChange = [this, binding] {
                const auto value = binding.slider->getValue();
                binding.target->store(static_cast<float>(value), std::memory_order_relaxed);
                static_cast<void>(ecologyState.set(binding.key,
                    binding.inverted ? 1.0 - value : value));
                submitControlFrame();
            };
        for (const auto& binding : performanceBindings())
            binding.slider->onValueChange = [this, binding] {
                binding.target->store(static_cast<float>(binding.slider->getValue()),
                                      std::memory_order_relaxed);
                submitControlFrame();
            };

        restorePerformanceState();
        // the header's volume: the same parameter as OUTPUT TRIM, both ways
        headerVolume.setSliderStyle(juce::Slider::LinearHorizontal);
        headerVolume.setTextBoxStyle(juce::Slider::TextBoxRight, false, 62, 22);
        headerVolume.setRange(0.0, 1.0, 0.0);
        headerVolume.setColour(juce::Slider::trackColourId, Palette::green.withAlpha(0.78f));
        headerVolume.setColour(juce::Slider::backgroundColourId, Palette::line.withAlpha(0.46f));
        headerVolume.setColour(juce::Slider::thumbColourId, Palette::text);
        headerVolume.setColour(juce::Slider::textBoxTextColourId, Palette::text);
        headerVolume.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        headerVolume.textFromValueFunction = masterTrim.textFromValueFunction;
        headerVolume.valueFromTextFunction = masterTrim.valueFromTextFunction;
        headerVolume.setDoubleClickReturnValue(true, 0.75);
        headerVolume.setValue(masterTrim.getValue(), juce::dontSendNotification);
        headerVolume.onValueChange = [this] {
            if (std::abs(masterTrim.getValue() - headerVolume.getValue()) > 1.0e-9)
                masterTrim.setValue(headerVolume.getValue(), juce::sendNotificationSync);
        };
        {
            auto previous = masterTrim.onValueChange;
            masterTrim.onValueChange = [this, previous] {
                if (previous)
                    previous();
                headerVolume.setValue(masterTrim.getValue(), juce::dontSendNotification);
            };
        }
        headerVolumeLabel.setFont(uiFont(9.5f, juce::Font::bold));
        headerVolumeLabel.setColour(juce::Label::textColourId, Palette::muted);
        headerVolumeLabel.setJustificationType(juce::Justification::centredLeft);
        addAndMakeVisible(headerVolume);
        addAndMakeVisible(headerVolumeLabel);
        updateControlAvailability();
        configurePresets();
        configureGuides();

        setSize(1280, 800);
        updateEcologySurface();
        // a new session, a new reef (the author, 2026-10-21: "essas sequências
        // de notas são sempre as mesmas"): the songs, signatures and themes
        // are drawn from this seed. "session.fixedSeed" (> 0) in the settings
        // replays a session; the last one is kept in "session.lastSeed" and
        // shown in the audio status. Audits and captures keep 42.
        if (startAudio)
        {
            const auto fixed = settings.getIntValue("session.fixedSeed", 0);
            sessionSeed = fixed > 0 ? static_cast<std::uint32_t>(fixed)
                                    : static_cast<std::uint32_t>(juce::Random::getSystemRandom().nextInt(0x7ffffffe)) + 1u;
            settings.setValue("session.lastSeed", static_cast<int>(sessionSeed));
        }
        synthEditor.seed = sessionSeed;  // PROPOSTA's first draw follows the session
        // the world the session opens in (SessionGenesis.h): a key and a path
        // of chords, a mode, registers, orbits, contour, pace, who opens, the
        // bodies' colours, the hour and season - from the session's seed, so
        // every opening is another piece; "session.genesis" = "0" keeps the
        // classic instrument. Set before the audio starts (prepareToPlay).
        sessionGenesis = settings.getBoolValue("session.genesis", true);
        if (sessionGenesis)
            engine.setSessionWorld(aquorbium::SessionWorld::fromSeed(sessionSeed));
        if (startAudio)
        {
            auto audioState = settings.getXmlValue("audio.device");
            if (audioState != nullptr && needsPipeWireSafetyBuffer(
                    audioState->getStringAttribute("audioOutputDeviceName"),
                    audioState->getIntAttribute("audioDeviceBufferSize")))
                audioState->setAttribute("audioDeviceBufferSize", 512);
            loadKeptSoundmark();
            loadSynth();
            // the opening seed (OpeningSeed.h): every opening another biome;
            // "opening.seeded" = 0 restores the saved controls, as before
            if (settings.getBoolValue("opening.seeded", true))
                applyOpeningSeed();
            variaAllowed = true;
            inhabitMotion();
            const auto room = settings.getBoolValue("room.enabled", false);
            ecosystemView.setRoom(room);
            setAudioChannels(room ? 1 : 0, 2, audioState.get());
            // a first opening (no saved device): JUCE's default ALSA route came
            // up at 8 kHz with 16-frame blocks (measured 2026-10-26: 4 XRUN in
            // 90 s). Ask the default device for 48 kHz and 512 frames instead
            if (audioState == nullptr)
            {
                auto setup = deviceManager.getAudioDeviceSetup();
                if (auto* device = deviceManager.getCurrentAudioDevice())
                {
                    if (device->getAvailableSampleRates().contains(48000.0))
                        setup.sampleRate = 48000.0;
                    if (device->getAvailableBufferSizes().contains(512))
                        setup.bufferSize = 512;
                    static_cast<void>(deviceManager.setAudioDeviceSetup(setup, true));
                }
            }
            roomRequested.store(room, std::memory_order_relaxed);
            for (const auto& device : juce::MidiInput::getAvailableDevices())
                deviceManager.setMidiInputDeviceEnabled(device.identifier, true);
            deviceManager.addMidiInputDeviceCallback({}, this);
        }
        startTimerHz(30);
    }

    ~MainComponent() override
    {
        stopTimer();
        // Persist device configuration on the GUI thread, before closing it.
        if (const auto state = deviceManager.createStateXml())
            settings.setValue("audio.device", state.get());
        audioSettingsWindow.reset();
        deviceManager.removeMidiInputDeviceCallback({}, this);
        if (diagnosticCapture != nullptr && !captureFinalized)
            captureFinalXruns = deviceManager.getXRunCount();
        recorderForAudio.store(nullptr, std::memory_order_release);
        shutdownAudio();
        if (recorder != nullptr)
            finishRecording();
        if (diagnosticCapture != nullptr && !captureFinalized)
            finishInteractiveCapture();
        savePerformanceState();
        setLookAndFeel(nullptr);
    }

    void prepareToPlay(int samplesPerBlockExpected, double rate) override
    {
        // a (re)started device runs a new audio thread: ask again for it
        realtimeAsked.store(false, std::memory_order_relaxed);
        realtimePriority.store(0, std::memory_order_relaxed);
        if (auto* capture = captureForAudio.load(std::memory_order_acquire);
            capture != nullptr && std::abs(rate - static_cast<double>(capture->rate())) > 0.5)
        {
            captureRateChanged.store(true, std::memory_order_release);
            capture->requestStop();
        }
        if (auto* rec = recorderForAudio.load(std::memory_order_acquire);
            rec != nullptr && std::abs(rate - static_cast<double>(rec->rate())) > 0.5)
            rec->requestStop();  // a WAV holds one rate; the GUI finalizes it
        // the microphone's samples for RoomCoupling: sized here, never in the callback
        roomScratch.assign(static_cast<std::size_t>(std::max(8192, samplesPerBlockExpected * 4)), 0.0f);
        // the REC's tap buffers (pre-safety / pre-master-criativo): sized here too
        tapLeft.assign(roomScratch.size(), 0.0f);
        tapRight.assign(roomScratch.size(), 0.0f);
        for (auto& v : stemLeft) v.assign(roomScratch.size(), 0.0f);
        for (auto& v : stemRight) v.assign(roomScratch.size(), 0.0f);
        for (auto& v : ambisonicBuffer) v.assign(roomScratch.size(), 0.0f);
        preparedRate = rate;
        outputGainStep = static_cast<float>(1.0 / std::max(1.0, rate * 0.020));
        telemetryInterval = static_cast<std::uint32_t>(std::max(1.0, std::round(rate / 30.0)));
        samplesUntilTelemetry = 0;
        engine.prepare(rate);
        engine.setSeed(sessionSeed);
        {
            // the soundmark kept from an earlier session (prepareToPlay is not
            // the audio callback: the lock only guards the GUI's copy)
            const std::lock_guard<std::mutex> lock(soundmarkKeptLock);
            if (soundmarkKept.length >= 2)
                engine.restoreSoundmark(soundmarkKept);
        }
        static_cast<void>(controlFrames.consumeLatest(audioControls));
        audioControls.apply(engine, true);
        static_cast<void>(synthFrames.consumeLatest(audioSynth));
        applySynthToEngine();
    }

    void getNextAudioBlock(const juce::AudioSourceChannelInfo& info) override
    {
        audioCallbackStartedCounter.fetch_add(1, std::memory_order_relaxed);
        // once per audio thread: ask for real-time scheduling (see
        // RealtimeAudioThread.h); a new device restarts the question
        if (!realtimeAsked.exchange(true, std::memory_order_relaxed))
            realtimePriority.store(aquorbium::app::promoteCurrentThreadToRealtime(), std::memory_order_relaxed);
        const juce::ScopedNoDenormals noDenormals;
        const auto started = juce::Time::getHighResolutionTicks();
        // RoomCoupling: the input arrives in the buffer; copy it before the
        // buffer is cleared - only descriptors are taken from it
        const auto room = roomRequested.load(std::memory_order_relaxed);
        engine.setRoomCouplingEnabled(room);
        engine.setHeadphones(headphonesRequested.load(std::memory_order_relaxed));
        if (room && info.buffer->getNumChannels() > 0 && !roomScratch.empty())
        {
            const auto count = std::min(static_cast<std::size_t>(info.numSamples), roomScratch.size());
            std::copy_n(info.buffer->getReadPointer(0, info.startSample), count, roomScratch.data());
            engine.setRoomInput(roomScratch.data(), count);
        }
        info.clearActiveBufferRegion();
        publishControlsToAudioEngine();
        consumeEvents();
        if (const auto choices = soundmarkChoices.load(std::memory_order_relaxed); choices != soundmarkChoicesSeen)
        {
            soundmarkChoicesSeen = choices;
            engine.chooseSoundmark();
        }
        auto* left = info.buffer->getWritePointer(0, info.startSample);
        auto* right = info.buffer->getNumChannels() > 1
            ? info.buffer->getWritePointer(1, info.startSample) : left;
        const auto paused = pauseRequested.load(std::memory_order_relaxed);
        const auto silent = paused || muteRequested.load(std::memory_order_relaxed);
        // paused and already silent: the engine is not called, its time stops
        // (the buffer was cleared above)
        // the REC listening before the final output (recording.tap)
        const auto tap = static_cast<aquorbium::BiomaEngine::RecordTap>(recorderTap.load(std::memory_order_relaxed));
        const auto tapping = tap != aquorbium::BiomaEngine::RecordTap::postSafety
            && recorderForAudio.load(std::memory_order_acquire) != nullptr
            && static_cast<std::size_t>(info.numSamples) <= tapLeft.size();
        if (tapping)
        {
            std::fill_n(tapLeft.data(), info.numSamples, 0.0f);
            std::fill_n(tapRight.data(), info.numSamples, 0.0f);
            engine.setRecordTap(tap, tapLeft.data(), tapRight.data(), tapLeft.size());
        }
        // the stems beside the take (recording.stems)
        const auto stemming = stemsForAudio.load(std::memory_order_acquire)
            && recorderForAudio.load(std::memory_order_acquire) != nullptr
            && static_cast<std::size_t>(info.numSamples) <= stemLeft[0].size();
        if (stemming)
        {
            aquorbium::BiomaEngine::StemBuffers buffers;
            for (std::size_t s = 0; s < buffers.left.size(); ++s)
            {
                std::fill_n(stemLeft[s].data(), info.numSamples, 0.0f);
                std::fill_n(stemRight[s].data(), info.numSamples, 0.0f);
                buffers.left[s] = stemLeft[s].data();
                buffers.right[s] = stemRight[s].data();
            }
            buffers.capacity = stemLeft[0].size();
            engine.setStems(buffers);
        }
        // the first-order Ambisonics beside the take (recording.ambisonics)
        auto* foaRecorder = ambisonicRecorderForAudio.load(std::memory_order_acquire);
        const auto ambisonic = foaRecorder != nullptr
            && static_cast<std::size_t>(info.numSamples) <= ambisonicBuffer[0].size();
        if (ambisonic)
        {
            aquorbium::BiomaEngine::AmbisonicBuffers buffers;
            for (std::size_t c = 0; c < buffers.channel.size(); ++c)
            {
                std::fill_n(ambisonicBuffer[c].data(), info.numSamples, 0.0f);
                buffers.channel[c] = ambisonicBuffer[c].data();
            }
            buffers.capacity = ambisonicBuffer[0].size();
            engine.setAmbisonics(buffers);
        }
        if (!(paused && outputGain <= 0.0f))
            engine.render(left, right, static_cast<std::size_t>(info.numSamples));
        if (ambisonic)
            foaRecorder->push(ambisonicBuffer[0].data(), ambisonicBuffer[1].data(), ambisonicBuffer[2].data(),
                                    ambisonicBuffer[3].data(), static_cast<std::size_t>(info.numSamples));
        if (silent || outputGain < 1.0f)
        {
            const auto target = silent ? 0.0f : 1.0f;
            for (int sample = 0; sample < info.numSamples; ++sample)
            {
                outputGain = target > outputGain ? std::min(target, outputGain + outputGainStep)
                                                 : std::max(target, outputGain - outputGainStep);
                left[sample] *= outputGain;
                if (right != left)
                    right[sample] *= outputGain;
            }
        }
        if (auto* capture = captureForAudio.load(std::memory_order_acquire))
            capture->push(left, right, static_cast<std::size_t>(info.numSamples));
        if (auto* rec = recorderForAudio.load(std::memory_order_acquire))
        {
            if (tapping)
                rec->push(tapLeft.data(), tapRight.data(), static_cast<std::size_t>(info.numSamples));
            else
                rec->push(left, right, static_cast<std::size_t>(info.numSamples));
            if (stemming)
                for (std::size_t s = 0; s < stemRecordersForAudio.size(); ++s)
                    if (auto* stem = stemRecordersForAudio[s].load(std::memory_order_acquire))
                        stem->push(stemLeft[s].data(), stemRight[s].data(), static_cast<std::size_t>(info.numSamples));
        }
        if (runtimeAuditEnabled)
        {
            // Fixed scalar accumulators, audio-owned until shutdownAudio joins
            // the callback. No file, allocation, lock or GUI access here.
            for (int sample = 0; sample < info.numSamples; ++sample)
            {
                const auto l = left[sample], r = right[sample];
                if (!std::isfinite(l) || !std::isfinite(r))
                    ++runtimeAuditInvalidSamples;
                else
                {
                    runtimeAuditPeak = std::max(runtimeAuditPeak, std::max(std::abs(l), std::abs(r)));
                    runtimeAuditSquared += static_cast<double>(l) * l + static_cast<double>(r) * r;
                    if (runtimeAuditSamples != 0)
                        runtimeAuditDelta = std::max(runtimeAuditDelta,
                            std::max(std::abs(l - runtimeAuditPreviousLeft), std::abs(r - runtimeAuditPreviousRight)));
                }
                runtimeAuditPreviousLeft = l;
                runtimeAuditPreviousRight = r;
                ++runtimeAuditSamples;
            }
        }
        // No point building expensive telemetry hundreds/thousands of times
        // per second for a GUI that reads it at 30 Hz.
        if (samplesUntilTelemetry <= static_cast<std::uint32_t>(info.numSamples))
        {
            visualTelemetry.publish(engine.telemetry());
            samplesUntilTelemetry = telemetryInterval;
        }
        else
            samplesUntilTelemetry -= static_cast<std::uint32_t>(info.numSamples);
        auto clipped = false;
        for (int sample = 0; sample < info.numSamples; ++sample)
            clipped = clipped || std::abs(left[sample]) >= 1.0f || std::abs(right[sample]) >= 1.0f;
        if (clipped)
            clippedBlockCounter.fetch_add(1, std::memory_order_relaxed);
        const auto elapsed = juce::Time::highResolutionTicksToSeconds(
            juce::Time::getHighResolutionTicks() - started);
        if (elapsed > static_cast<double>(info.numSamples) / preparedRate)
            overBudgetCounter.fetch_add(1, std::memory_order_relaxed);
        audioCallbackCounter.fetch_add(1, std::memory_order_relaxed);
    }

    void releaseResources() override {}

    bool startInteractiveCapture(const juce::File& directory,
                                 const juce::String& outputName, unsigned seconds)
    {
        // Interactive, explicit opt-in. Normal preferences and audio route are
        // untouched; the caller supplies a fresh directory and settings root.
        if (outputName.isEmpty() || seconds == 0
            || seconds > aquorbium::app::DiagnosticAudioCapture::maximumSeconds
            || directory.getChildFile("CAPTURE.wav").exists()
            || directory.getChildFile("REPORT.txt").exists())
            return false;
        captureDirectory = directory;
        juce::String available;
        std::unique_ptr<juce::XmlElement> requested;
        int matches = 0;
        for (auto* type : deviceManager.getAvailableDeviceTypes())
        {
            type->scanForDevices();
            for (const auto& name : type->getDeviceNames(false))
            {
                available += type->getTypeName() + ": " + name + "\n";
                if (name == outputName)
                {
                    ++matches;
                    requested = std::make_unique<juce::XmlElement>("DEVICESETUP");
                    requested->setAttribute("deviceType", type->getTypeName());
                    requested->setAttribute("audioOutputDeviceName", name);
                    requested->setAttribute("audioInputDeviceName", "");
                    requested->setAttribute("audioDeviceRate", 48000.0);
                    requested->setAttribute("audioDeviceBufferSize", 512);
                }
            }
        }
        if (matches != 1)
        {
            static_cast<void>(directory.getChildFile("REPORT.txt").replaceWithText(
                "NOT RUN: requested output must match exactly one device.\n" + available));
            return false;
        }
        setAudioChannels(0, 2, requested.get());
        auto* device = deviceManager.getCurrentAudioDevice();
        if (device == nullptr || !device->isPlaying()
            || deviceManager.getAudioDeviceSetup().outputDeviceName != outputName)
        {
            shutdownAudio();
            static_cast<void>(directory.getChildFile("REPORT.txt").replaceWithText(
                "NOT RUN: requested output is not running; fallback refused.\n" + available));
            return false;
        }
        captureDevice = device->getName();
        captureBackend = deviceManager.getCurrentAudioDeviceType();
        captureRate = device->getCurrentSampleRate();
        captureBuffer = device->getCurrentBufferSizeSamples();
        // Leave useful metadata even if the GUI later stops before finalizing
        // the detailed report. The capture writer finalizes its WAV independently.
        const auto pendingReport = "status=incomplete\ndevice=" + captureDevice
            + "\nbackend=" + captureBackend + "\nrate=" + juce::String(captureRate)
            + "\nbuffer=" + juce::String(captureBuffer)
            + "\nrequested_seconds=" + juce::String(seconds)
            + "\nThe process may have stopped before its GUI finalized REPORT.txt.\n";
        if (!directory.getChildFile("REPORT.txt").replaceWithText(pendingReport))
        {
            shutdownAudio();
            return false;
        }
        diagnosticCapture = std::make_unique<aquorbium::app::DiagnosticAudioCapture>(
            directory.getChildFile("CAPTURE.wav").getFullPathName().toStdString(),
            static_cast<unsigned>(captureRate), seconds);
        if (!diagnosticCapture->start())
        {
            diagnosticCapture.reset();
            shutdownAudio();
            static_cast<void>(directory.getChildFile("REPORT.txt").replaceWithText(
                "NOT RUN: cannot create exclusive WAV or start disk writer.\n"));
            return false;
        }
        captureStarted = juce::Time::getMillisecondCounterHiRes();
        captureNextReadingMs = captureStarted + 500.0;
        captureForAudio.store(diagnosticCapture.get(), std::memory_order_release);
        audioSettingsButton.setEnabled(false);
        for (const auto& midi : juce::MidiInput::getAvailableDevices())
            deviceManager.setMidiInputDeviceEnabled(midi.identifier, true);
        deviceManager.addMidiInputDeviceCallback({}, this);
        lastGuiTick = 0;
        return true;
    }

    bool startRuntimeAudit(const juce::File& directory, const juce::String& outputName, bool stress)
    {
        // Explicit opt-in diagnostic. All paths and device work are GUI-owned.
        if (directory.getChildFile("REPORT.txt").exists() || directory.createDirectory().failed())
            return false;
        runtimeAuditDirectory = directory;
        runtimeAuditStress = stress;
        if (stress)
        {
            // Existing scene boundary, isolated diagnostic bank only.
            aquorbium::app::PerformanceParameterState calm, active;
            static_cast<void>(active.set("strikerInfluence", 1.0));
            static_cast<void>(active.set("strikerMatter", 0.9));
            static_cast<void>(active.set("strikerPopulation", 0.9));
            static_cast<void>(active.set("geometryInfluence", 1.0));
            static_cast<void>(active.set("rhythmDensity", 0.9));
            static_cast<void>(active.set("spaceGesture", 1.0));
            static_cast<void>(active.set("spaceMemory", 0.9));
            aquorbium::app::EcologyParameterState ecology;
            static_cast<void>(ecology.set(aquorbium::app::EcologyParameterState::Key::density, 0.9));
            static_cast<void>(ecology.set(aquorbium::app::EcologyParameterState::Key::calm, 0.1));
            static_cast<void>(active.setEcology(ecology.snapshot()));
            // the active scene carries the synthesis device (PROPOSTA, seed
            // 42, SYNTHESIS 1) and the calm one none: each recall switches
            // the five synthesis voices in and out under audio (2026-10-22)
            std::array<juce::String, aquorbium::app::PerformanceParameterState::synthPatchCount> proposed;
            for (std::size_t i = 0; i < proposed.size(); ++i)
                proposed[i] = juce::String(aquorbium::SynthPatchCodec::encode(aquorbium::SynthProposal::propose(i, 42u)));
            if (!active.setSynth(1.0, proposed))
                return false;
            if (!presetBank.store(0, calm, "Audit calm") || !presetBank.store(1, active, "Audit active"))
                return false;
        }
        juce::String available;
        std::unique_ptr<juce::XmlElement> requested;
        int matches = 0;
        for (auto* type : deviceManager.getAvailableDeviceTypes())
        {
            type->scanForDevices();
            for (const auto& name : type->getDeviceNames(false))
            {
                available += type->getTypeName() + ": " + name + "\n";
                if (outputName.isNotEmpty() && name == outputName)
                {
                    ++matches;
                    requested = std::make_unique<juce::XmlElement>("DEVICESETUP");
                    requested->setAttribute("deviceType", type->getTypeName());
                    requested->setAttribute("audioOutputDeviceName", name);
                    requested->setAttribute("audioInputDeviceName", "");
                    requested->setAttribute("audioDeviceRate", 48000.0);
                    requested->setAttribute("audioDeviceBufferSize", 512);
                }
            }
        }
        if (outputName.isNotEmpty() && matches != 1)
        {
            static_cast<void>(directory.getChildFile("REPORT.txt").replaceWithText(
                "NOT RUN: requested output must match exactly one device.\n" + available));
            return false;
        }
        runtimeAuditEnabled = true; // published before opening the callback
        setAudioChannels(0, 2, requested.get());
        auto* device = deviceManager.getCurrentAudioDevice();
        if (device == nullptr || !device->isPlaying()
            || (outputName.isNotEmpty() && deviceManager.getAudioDeviceSetup().outputDeviceName != outputName))
        {
            shutdownAudio();
            runtimeAuditEnabled = false;
            static_cast<void>(directory.getChildFile("REPORT.txt").replaceWithText(
                "NOT RUN: no running requested output; fallback is not accepted.\n" + available));
            return false;
        }
        runtimeAuditDevice = device->getName();
        runtimeAuditBackend = deviceManager.getCurrentAudioDeviceType();
        runtimeAuditRate = device->getCurrentSampleRate();
        runtimeAuditBuffer = device->getCurrentBufferSizeSamples();
        runtimeAuditStarted = juce::Time::getMillisecondCounterHiRes();
        runtimeAuditActive = true;
        audioSettingsButton.setEnabled(false); // keep the measured route fixed
        lastGuiTick = 0;
        return true;
    }

    bool auditLayout(const juce::File& directory)
    {
        // Offline visual audit: no device, no normal user preferences and no
        // overwrite. Generate both central pages and all right-column views.
        stopTimer();
        if (directory.getChildFile("REPORT.txt").exists()) return false;
        juce::String report;
        // why the audit failed, in FAILED.txt (2026-10-22: it failed silently)
        const auto fail = [&directory](const juce::String& why) {
            directory.getChildFile("FAILED.txt").appendText(why + "\n");
            return false;
        };
        for (const auto size : { juce::Point<int> { 1920, 1006 }, { 1280, 800 },
                                 { 1024, 650 }, { 960, 600 } })
        {
            setSize(size.x, size.y);
            for (auto* slider : { &strikerInfluence, &strikerMatter, &strikerPopulation,
                                  &strikerScatter, &spaceReturn, &spaceGesture, &spaceSize,
                                  &spaceSpread, &spaceTone, &spaceMemory,
                                  &environmentInfluence, &environmentFilterMix,
                                  &environmentCentre, &environmentSpan,
                                  &environmentResonance, &environmentAmbience,
                                  &tailInfluence, &tailDecay,
                                  &tailDamping, &tailNightmare,
                                  &formInfluence, &formContext,
                                  &formInertia, &formMutation,
                                  &structureInfluence, &structureMemory,
                                  &structureHarmony, &structurePhase,
                                  &creativeMaster, &masterTrim,
                                  &masterCeiling, &masterRelease, &masterEqOrder })
            {
                const auto geometry = slider->getLookAndFeel().getSliderLayout(*slider);
                const auto diameter = std::min(geometry.sliderBounds.getWidth(),
                                              geometry.sliderBounds.getHeight()) - 20;
                if (slider->getWidth() < 80 || slider->getHeight() < 92 || diameter < 52)
                    return fail(juce::String(size.x) + "x" + juce::String(size.y) + ": a card knob too small, "
                                + slider->getBounds().toString() + " diameter " + juce::String(diameter));
            }
            for (auto& slider : moduleBankSliders)
            {
                const auto geometry = slider.getLookAndFeel().getSliderLayout(slider);
                const auto diameter = std::min(geometry.sliderBounds.getWidth(),
                                               geometry.sliderBounds.getHeight()) - 20;
                if (slider.getWidth() < 80 || slider.getHeight() < 92 || diameter < 52)
                    return fail(juce::String(size.x) + "x" + juce::String(size.y) + ": a module knob too small, "
                                + slider.getBounds().toString());
            }
            // two rows of knobs; the wide surface (1920) lays SYNTHESIS and
            // TAIL out in one row of four by design, so one row there
            // (the 224 predates the wide surface: the audit had failed
            // silently since, found 2026-10-22)
            const auto wideSurface = surfaceMode == aquorbium::windowing::SurfaceMode::wide;
            const auto oneRow = wideSurface ? 104 : 224;
            if (synthesisViewport.getHeight() < oneRow || spaceViewport.getHeight() < 224
                || environmentViewport.getHeight() < 224 || tailViewport.getHeight() < oneRow)
                return fail(juce::String(size.x) + "x" + juce::String(size.y) + ": a card viewport under 224 px (synthesis "
                            + juce::String(synthesisViewport.getHeight()) + ", space " + juce::String(spaceViewport.getHeight())
                            + ", environment " + juce::String(environmentViewport.getHeight()) + ", tail " + juce::String(tailViewport.getHeight()) + ")");
            if (strikerMatter.isEnabled() || spaceSize.isEnabled()
                || environmentCentre.isEnabled() || tailDecay.isEnabled()
                || rhythmDensity.isEnabled() || formContext.isEnabled()
                || structureMemory.isEnabled() || moduleBankSliders[1].isEnabled())
                return fail("a gated control enabled with fresh settings"); // all manual mixes are zero
            for (int view = 0; view < 19; ++view)  // 18: the cards added on 2026-10-24
            {
                showEcologyControls = view == 9;
                updateEcologySurface();
                geometryMemoryViewport.setViewPosition(0,
                    view >= 4 && view <= 7
                        ? aquorbium::windowing::MacroPanelLayout::rowHeight * (view - 4)
                        : 0);
                moduleBankViewport.setViewPosition(0,
                    view >= 10 && view < 18
                        ? aquorbium::windowing::MacroPanelLayout::rowHeight * (view - 10)
                        : 0);
                rightColumnViewport.setViewPosition(0, view == 0 ? 0
                    : view == 1 ? spaceCard.getY()
                    : view == 2 ? environmentCard.getY()
                    : (view >= 3 && view <= 7) ? tailCard.getY()
                    : view <= 9 ? masterCard.getY()
                    : view == 18 ? lensCard.getY()
                                : moduleBankCard.getY());
                const auto file = directory.getChildFile(juce::String(size.x) + "x"
                    + juce::String(size.y) + (view == 0 ? "-synthesis.png"
                                              : view == 1 ? "-space.png"
                                              : view == 2 ? "-environment.png"
                                              : view == 3 ? "-tail.png"
                                              : view == 4 ? "-geometry-form-a.png"
                                              : view == 5 ? "-geometry-form-b.png"
                                              : view == 6 ? "-geometry-structure-a.png"
                                              : view == 7 ? "-geometry-structure-b.png"
                                              : view == 8 ? "-master.png"
                                              : view == 9 ? "-ecology.png"
                                              : view == 18 ? "-new-cards.png"
                                              : "-modules-" + juce::String(view - 9) + ".png"));
                if (file.exists()) return false;
                juce::Image image(juce::Image::ARGB, getWidth(), getHeight(), true);
                juce::Graphics graphics(image);
                paintEntireComponent(graphics, false);
                auto stream = file.createOutputStream();
                if (stream == nullptr || !juce::PNGImageFormat().writeImageToStream(image, *stream))
                    return false;
            }
            // the synthesis editor, with the five proposals of a fixed seed
            // (2026-10-22), in each of the four languages
            {
                const auto languageBefore = aquorbium::app::currentLang();
                for (std::size_t i = 0; i < SynthEditor::organisms; ++i)
                    synthEditor.patches[i] = aquorbium::SynthProposal::propose(i, 42u);
                synthEditor.mix = 0.5f;
                synthEditorViewport.setBounds(getLocalBounds());
                synthEditor.setSize(std::max(getWidth(), SynthEditor::minimumWidth), std::max(getHeight(), SynthEditor::minimumHeight));
                synthEditorViewport.setVisible(true);
                synthEditorViewport.toFront(false);
                synthEditor.show();
                for (int language = 0; language < 4; ++language)
                {
                    aquorbium::app::currentLang() = static_cast<aquorbium::app::Lang>(language);
                    synthEditor.refreshTexts();
                    synthEditor.resized();
                    const auto problems = synthEditor.auditLayout();
                    if (problems.isNotEmpty())
                    {
                        directory.getChildFile("SYNTH_EDITOR_PROBLEMS.txt").appendText(
                            juce::String(size.x) + "x" + juce::String(size.y) + " language " + juce::String(language) + "\n" + problems);
                        return false;
                    }
                    const auto file = directory.getChildFile(juce::String(size.x) + "x" + juce::String(size.y)
                                                             + "-synth-editor-" + juce::String(language) + ".png");
                    if (file.exists()) return false;
                    juce::Image image(juce::Image::ARGB, getWidth(), getHeight(), true);
                    juce::Graphics graphics(image);
                    paintEntireComponent(graphics, false);
                    auto stream = file.createOutputStream();
                    if (stream == nullptr || !juce::PNGImageFormat().writeImageToStream(image, *stream))
                        return false;
                }
                // the modes the seed may not show (2026-10-22): MORPH through
                // FORMANT (TRACT, VOWEL) and MODAL on FM 4-OP (its algorithm
                // list and ratios C/D), in PT at 1280 x 800
                if (size.x == 1280)
                {
                    aquorbium::app::currentLang() = aquorbium::app::Lang::pt;
                    synthEditor.refreshTexts();
                    synthEditor.patches[0].filterMode = aquorbium::SynthVoice::FilterMode::formant;
                    synthEditor.patches[2].sourceA = aquorbium::SynthVoice::Source::fm4;
                    synthEditor.patches[2].algorithmA = 6;
                    for (const std::size_t organism : { std::size_t { 0 }, std::size_t { 2 } })
                    {
                        synthEditor.selectOrganism(organism);
                        synthEditor.resized();
                        const auto problems = synthEditor.auditLayout();
                        if (problems.isNotEmpty())
                            return fail("1280x800 demo organism " + juce::String(static_cast<int>(organism)) + ": " + problems);
                        const auto file = directory.getChildFile("1280x800-synth-editor-demo-" + juce::String(static_cast<int>(organism)) + ".png");
                        juce::Image image(juce::Image::ARGB, getWidth(), getHeight(), true);
                        juce::Graphics graphics(image);
                        paintEntireComponent(graphics, false);
                        auto stream = file.createOutputStream();
                        if (stream == nullptr || !juce::PNGImageFormat().writeImageToStream(image, *stream))
                            return false;
                    }
                    synthEditor.selectOrganism(0);
                }
                aquorbium::app::currentLang() = languageBefore;
                synthEditorViewport.setVisible(false);
            }
            // the TUTORIAL's synthesis chapter and ABOUT (2026-10-22), in PT:
            // every chapter button inside the card
            {
                const auto languageBefore = aquorbium::app::currentLang();
                aquorbium::app::currentLang() = aquorbium::app::Lang::pt;
                guide.setBounds(getLocalBounds());
                // chapter 8 (the synthesis device), lesson 5 (the longest) and ABOUT
                for (const auto page : { 7, 12, -1 })
                {
                    const auto kind = page < 0 ? GuideOverlay::Kind::about : GuideOverlay::Kind::tutorial;
                    guide.show(kind);
                    guide.toFront(false);
                    if (page >= 0)
                        guide.select(page);
                    guide.resized();
                    for (auto* child : guide.getChildren())
                        if (child->isVisible() && !guide.cardBounds().contains(child->getBounds()))
                            return fail(juce::String(size.x) + "x" + juce::String(size.y) + ": a guide control outside its card, "
                                        + child->getBounds().toString());
                    const auto file = directory.getChildFile(juce::String(size.x) + "x" + juce::String(size.y)
                        + (page == 7 ? "-tutorial-synthesis.png" : page == 12 ? "-tutorial-lesson5.png" : "-about.png"));
                    if (file.exists()) return false;
                    juce::Image image(juce::Image::ARGB, getWidth(), getHeight(), true);
                    juce::Graphics graphics(image);
                    paintEntireComponent(graphics, false);
                    auto stream = file.createOutputStream();
                    if (stream == nullptr || !juce::PNGImageFormat().writeImageToStream(image, *stream))
                        return false;
                }
                guide.setVisible(false);
                aquorbium::app::currentLang() = languageBefore;
            }
            report += juce::String(size.x) + "x" + juce::String(size.y)
                + ": knobs >=52px, hit areas >=80x92px; separate labels/values; gated controls disabled; 18 snapshots;"
                  " synthesis editor (PROPOSTA seed 42) inside its card in 4 languages\n";
        }
        return directory.getChildFile("REPORT.txt").replaceWithText(report);
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(Palette::background);
        const auto wide = surfaceMode == aquorbium::windowing::SurfaceMode::wide;
        const auto full = surfaceMode == aquorbium::windowing::SurfaceMode::full || wide;
        const auto margin = wide ? 16 : full ? 24 : 14;
        auto header = getLocalBounds().removeFromTop(wide ? 64 : full ? 74 : 58).reduced(margin, 0);
        header.setRight(std::max(header.getX(), headerButtonsLeft));
        // the brand takes only the room the header's buttons leave (on a
        // narrow window the buttons reached over it); too little, none
        auto brand = header.removeFromLeft(std::min(brandWidth(), header.getWidth()));
        const auto brandFits = brand.getWidth() >= 120;
        // the identity approved on 2026-10-25 (assets/identity/IDENTIDADE.md):
        // the sonar symbol, and beside it the original vector lettering
        if (logo == nullptr)
            logo = juce::Drawable::createFromImageData(BinaryData::aquorbiumsymbol_svg,
                                                        static_cast<std::size_t>(BinaryData::aquorbiumsymbol_svgSize));
        if (wordmark == nullptr)
            wordmark = juce::Drawable::createFromImageData(BinaryData::aquorbiumwordmark_svg,
                                                            static_cast<std::size_t>(BinaryData::aquorbiumwordmark_svgSize));
        if (logo != nullptr && brandFits)
        {
            const auto size = full ? 44.0f : 36.0f;
            logo->drawWithin(g, juce::Rectangle<float>(static_cast<float>(brand.getX()), static_cast<float>(brand.getY()) + 6.0f, size, size),
                             juce::RectanglePlacement::centred, 1.0f);
            brand.removeFromLeft(static_cast<int>(size) + 10);
        }
        const auto title = brand.removeFromTop(full ? 43 : 34);
        if (brandFits && wordmark != nullptr)
            wordmark->drawWithin(g, title.toFloat().reduced(0.0f, full ? 8.0f : 7.0f),
                                 juce::RectanglePlacement::xLeft | juce::RectanglePlacement::yMid, 1.0f);
        else if (brandFits)
        {
            g.setColour(Palette::text);
            g.setFont(uiFont(full ? 26.0f : 22.0f, juce::Font::bold));
            g.drawText("AQUORBIUM", title, juce::Justification::centredLeft);
        }
        g.setColour(Palette::muted);
        g.setFont(uiFont(full ? 10.5f : 9.5f, juce::Font::bold));
        if (brandFits)
            g.drawFittedText(aquorbium::app::T(aquorbium::app::S::subtitle), brand,
                             juce::Justification::centredLeft, 1);
        g.setColour(audioEngineOnline ? Palette::cyan : Palette::amber);
        const auto statusLine = audioStatus.upToFirstOccurrenceOf("  CPU ", false, false);
        const auto metricsLine = audioStatus.fromFirstOccurrenceOf("  CPU ", false, false);
        g.setFont(uiFont(12.0f, juce::Font::bold));
        g.drawFittedText((audioEngineOnline ? aquorbium::app::T(aquorbium::app::S::audioOnline) : aquorbium::app::T(aquorbium::app::S::audioWaiting)) + "  "
                         + statusLine, header.removeFromTop(24),
                         juce::Justification::centredRight, 1);
        g.setFont(uiFont(11.0f));
        g.drawFittedText(metricsLine, header.removeFromTop(19),
                         juce::Justification::centredRight, 1);

        const auto footerHeight = wide ? 134 : full ? 224 : 192;
        const auto footer = getLocalBounds().removeFromBottom(footerHeight).toFloat();
        g.setColour(Palette::panel.withAlpha(0.72f));
        g.fillRect(footer);
        g.setColour(Palette::line.withAlpha(0.65f));
        g.drawHorizontalLine(static_cast<int>(footer.getY()), footer.getX(), footer.getRight());
        // the credit line, in the free bottom margin
        g.setColour(Palette::muted.withAlpha(0.85f));
        g.setFont(uiFont(10.0f));
        g.drawText(aquorbium::app::T(aquorbium::app::S::footerCredit) + JUCE_APPLICATION_VERSION_STRING,
                   getLocalBounds().removeFromBottom(margin).reduced(margin, 0),
                   juce::Justification::centredRight);
    }

    void resized() override
    {
        {
            // the RASGO header (as in the Trioio): language, TUTORIAL and
            // ABOUT in the right corner; LEARN beside them; then the sound
            // (MUTE, PAUSE, AUDIO). Widths follow the language.
            using aquorbium::app::Lang;
            const auto lang = aquorbium::app::currentLang();
            const auto aboutWidth = lang == Lang::es ? 112 : lang == Lang::fr ? 104 : 92;
            const auto tutorialWidth = lang == Lang::fr ? 104 : 100;
            auto x = getWidth() - 16;
            const auto place = [&x](juce::Button& button, int width, int gapAfter) {
                x -= width;
                button.setBounds(x, 20, width, 28);
                x -= gapAfter;
            };
            place(aboutButton, aboutWidth, 6);
            place(tutorialButton, tutorialWidth, 6);
            place(languageButton, 48, 6);
            place(learnButton, 76, 18);
            place(audioSettingsButton, 82, 6);
            place(listeningButton, lang == Lang::en ? 112 : (lang == Lang::es ? 118 : 96), 6);
            place(pauseButton, lang == Lang::fr ? 120 : 112, 6);
            place(muteButton, 112, 6);
            place(recButton, 104, 12);
            // the output volume, in the header (the author, 2026-10-24): as
            // wide as the room left for the status allows, hidden if too narrow
            {
                const auto room = x - 16 - brandWidth() - 420;  // the status lines keep ~420 px
                const auto width = std::min(220, room);
                headerVolume.setVisible(width >= 110);
                headerVolumeLabel.setVisible(width >= 110);
                if (width >= 110)
                {
                    x -= width;
                    headerVolumeLabel.setBounds(x, 4, width, 16);
                    headerVolume.setBounds(x, 20, width, 28);
                    x -= 12;
                }
            }
            headerButtonsLeft = x;
        }
        guide.setBounds(getLocalBounds());
        synthEditorViewport.setBounds(getLocalBounds());
        synthEditor.setSize(std::max(getWidth(), SynthEditor::minimumWidth), std::max(getHeight(), SynthEditor::minimumHeight));
        surfaceMode = aquorbium::windowing::WindowLayoutPolicy::classifySurface(
            getWidth(), getHeight());
        const auto wide = surfaceMode == aquorbium::windowing::SurfaceMode::wide;
        for (auto* card : { &brainCard, &geometryCard, &synthesisCard, &spaceCard,
                            &environmentCard, &tailCard, &masterCard, &moduleBankCard, &timbreCard, &meterCard, &lensCard, &brainRoomCard })
            card->setCompactHeader(wide);
        if (wide)
            layoutWideSurface();
        else if (surfaceMode == aquorbium::windowing::SurfaceMode::full)
            layoutFullSurface();
        else
            layoutCompactSurface();
        layoutLearnPanel();
        if (guide.isVisible())
            guide.toFront(false);
        keepSynthEditorOnTop();
        repaint();
    }

    // the layout re-adopts the sound cards and raises the ECO toggle above
    // every sibling: an open synthesis editor goes back on top (2026-10-22)
    void keepSynthEditorOnTop()
    {
        if (synthEditorViewport.isVisible())
            synthEditorViewport.toFront(false);
    }

private:

    // ---- tutorial, learn, about and languages (2026-10-20) ---------------------
    struct ControlWidget
    {
        const char* id;
        juce::Slider* slider;
        juce::Label* label;
        const char* card;
    };

    [[nodiscard]] std::array<ControlWidget, 63> controlWidgets()
    {
        std::array<ControlWidget, 63> all {};
        const auto base = controlWidgetsBase();
        std::copy(base.begin(), base.end(), all.begin());
        static constexpr std::array<const char*, 12> cards { "lens", "lens", "lens", "lens", "master", "master",
                                                             "master", "master", "brain", "brain", "room", "room" };
        for (std::size_t i = 0; i < cardSliders.size(); ++i)
            all[base.size() + i] = { cardIds[i], &cardSliders[i], &cardLabels[i], cards[i] };
        return all;
    }

    [[nodiscard]] std::array<ControlWidget, 51> controlWidgetsBase()
    {
        return {{
            { "density", &density, &densityLabel, "strip" },
            { "drift", &drift, &driftLabel, "strip" },
            { "ecosystemMorph", &ecosystemMorph, &ecosystemMorphLabel, "strip" },
            { "cohesion", &cohesion, &cohesionLabel, "strip" },
            { "danger", &danger, &dangerLabel, "strip" },
            { "sparseFocus", &sparseFocus, &sparseFocusLabel, "strip" },
            { "energyMemory", &energyMemory, &energyMemoryLabel, "strip" },
            { "geometricCycle", &geometricCycle, &geometricCycleLabel, "strip" },
            { "collectiveDrift", &collectiveDrift, &collectiveDriftLabel, "strip" },
            { "geometryInfluence", &geometryInfluence, &geometryInfluenceLabel, "strip" },
            { "rhythmDensity", &rhythmDensity, &rhythmDensityLabel, "strip" },
            { "orbitRotation", &orbitRotation, &orbitRotationLabel, "strip" },
            { "intervalBias", &intervalBias, &intervalBiasLabel, "strip" },
            { "geometryMutation", &geometryMutation, &geometryMutationLabel, "strip" },
            { "formInfluence", &formInfluence, &formInfluenceLabel, "geometry" },
            { "formContext", &formContext, &formContextLabel, "geometry" },
            { "formInertia", &formInertia, &formInertiaLabel, "geometry" },
            { "formMutation", &formMutation, &formMutationLabel, "geometry" },
            { "structureInfluence", &structureInfluence, &structureInfluenceLabel, "geometry" },
            { "structureMemory", &structureMemory, &structureMemoryLabel, "geometry" },
            { "structureHarmony", &structureHarmony, &structureHarmonyLabel, "geometry" },
            { "structurePhase", &structurePhase, &structurePhaseLabel, "geometry" },
            { "musicInfluence", &musicInfluence, &musicInfluenceLabel, "geometry" },
            { "musicReaction", &musicReaction, &musicReactionLabel, "geometry" },
            { "musicMemory", &musicMemory, &musicMemoryLabel, "geometry" },
            { "musicWater", &musicWater, &musicWaterLabel, "geometry" },
            { "strikerInfluence", &strikerInfluence, &strikerInfluenceLabel, "synthesis" },
            { "strikerMatter", &strikerMatter, &strikerMatterLabel, "synthesis" },
            { "strikerPopulation", &strikerPopulation, &strikerPopulationLabel, "synthesis" },
            { "strikerScatter", &strikerScatter, &strikerScatterLabel, "synthesis" },
            { "spaceReturn", &spaceReturn, &spaceReturnLabel, "space" },
            { "spaceGesture", &spaceGesture, &spaceGestureLabel, "space" },
            { "spaceSize", &spaceSize, &spaceSizeLabel, "space" },
            { "spaceSpread", &spaceSpread, &spaceSpreadLabel, "space" },
            { "spaceTone", &spaceTone, &spaceToneLabel, "space" },
            { "spaceMemory", &spaceMemory, &spaceMemoryLabel, "space" },
            { "environmentInfluence", &environmentInfluence, &environmentInfluenceLabel, "environment" },
            { "environmentFilterMix", &environmentFilterMix, &environmentFilterMixLabel, "environment" },
            { "environmentCentre", &environmentCentre, &environmentCentreLabel, "environment" },
            { "environmentSpan", &environmentSpan, &environmentSpanLabel, "environment" },
            { "environmentResonance", &environmentResonance, &environmentResonanceLabel, "environment" },
            { "environmentAmbience", &environmentAmbience, &environmentAmbienceLabel, "environment" },
            { "tailInfluence", &tailInfluence, &tailInfluenceLabel, "tail" },
            { "tailDecay", &tailDecay, &tailDecayLabel, "tail" },
            { "tailDamping", &tailDamping, &tailDampingLabel, "tail" },
            { "tailNightmare", &tailNightmare, &tailNightmareLabel, "tail" },
            { "creativeMaster", &creativeMaster, &creativeMasterLabel, "master" },
            { "masterTrim", &masterTrim, &masterTrimLabel, "master" },
            { "masterCeiling", &masterCeiling, &masterCeilingLabel, "master" },
            { "masterRelease", &masterRelease, &masterReleaseLabel, "master" },
            { "masterEqOrder", &masterEqOrder, &masterEqOrderLabel, "master" }
        }};
    }

    struct CardWidget
    {
        ModuleCard* card;
        const char* id;
    };

    [[nodiscard]] std::array<CardWidget, 9> cardWidgets()
    {
        return {{ { &brainCard, "brain" }, { &geometryCard, "geometry" }, { &synthesisCard, "synthesis" },
                  { &spaceCard, "space" }, { &environmentCard, "environment" }, { &tailCard, "tail" },
                  { &masterCard, "master" }, { &moduleBankCard, "modules" }, { &timbreCard, "timbre" } }};
    }

    void configureGuides()
    {
        using namespace aquorbium::app;
        // the language: the saved one, or the system's on the first opening
        currentLang() = settings.containsKey("ui.language")
            ? langFromCode(settings.getValue("ui.language"))
            : langFromCode(juce::SystemStats::getUserLanguage());
        for (auto* button : { &tutorialButton, &learnButton, &aboutButton, &languageButton })
            addAndMakeVisible(*button);
        learnButton.setClickingTogglesState(true);
        tutorialButton.onClick = [this] { guide.show(GuideOverlay::Kind::tutorial); };
        aboutButton.onClick = [this] { guide.show(GuideOverlay::Kind::about); };
        learnButton.onClick = [this] {
            learnMode = learnButton.getToggleState();
            lastLearnKey.clear();
            learnPanel.setEntry(T(learnEmpty), T(S::learnTip), {});
            learnPanel.setVisible(learnMode);
            if (learnMode)
                learnPanel.toFront(false);
        };
        languageButton.onClick = [this] {
            currentLang() = nextLang(currentLang());
            settings.setValue("ui.language", langCode(currentLang()));
            applyLanguage();
        };
        addChildComponent(guide);
        // the editor scrolls on small screens rather than shrink its knobs
        synthEditorViewport.setViewedComponent(&synthEditor, false);
        synthEditorViewport.setScrollBarsShown(true, true, false, false);
        synthEditor.setVisible(true);
        addChildComponent(synthEditorViewport);
        synthEditor.onClose = [this] { synthEditorViewport.setVisible(false); };
        synthEditor.onChange = [this] {
            synthMixCard.setValue(synthEditor.mix, juce::dontSendNotification);
            synthChanged();
        };
        addChildComponent(learnPanel);
        guide.onClose = [this] { guide.setVisible(false); };

        // what each component explains under LEARN
        for (const auto& w : controlWidgets())
            for (auto* c : { static_cast<juce::Component*>(w.slider), static_cast<juce::Component*>(w.label) })
            {
                c->getProperties().set("learn", w.id);
                c->getProperties().set("card", w.card);
            }
        const auto mark = [](juce::Component& c, const juce::String& key) { c.getProperties().set("learn", key); };
        mark(geometryMemoryViewport, "card:geometry");
        mark(synthesisViewport, "card:synthesis");
        mark(spaceViewport, "card:space");
        mark(environmentViewport, "card:environment");
        mark(tailViewport, "card:tail");
        mark(masterViewport, "card:master");
        mark(moduleBankViewport, "card:modules");
        mark(timbreControls, "card:timbre");
        mark(ecosystemView, "card:ecosystem");
        for (std::size_t i = 0; i < moduleBankSliders.size(); ++i)
        {
            mark(moduleBankSliders[i], "module:" + juce::String(static_cast<int>(i)));
            mark(moduleBankLabels[i], "module:" + juce::String(static_cast<int>(i)));
        }
        for (std::size_t i = 0; i < 5; ++i)
        {
            mark(timbreBodySelect[i], "timbre.body");
            mark(timbreMatter[i], "timbre.matter");
            mark(timbreMix[i], "timbre.mix");
        }
        mark(timbreEcology, "timbre.ecology");
        mark(timbreEcologyLabel, "timbre.ecology");
        mark(supportPresence, "timbre.support");
        mark(supportPresenceLabel, "timbre.support");
        // the synthesis device's row (phase 6, 2026-10-22)
        mark(synthMixCard, "timbre.synth");
        mark(synthMixCardLabel, "timbre.synth");
        mark(synthEditButton, "timbre.edit");
        mark(tutorialButton, "header:tutorial");
        mark(learnButton, "header:learn");
        mark(aboutButton, "header:about");
        mark(languageButton, "header:language");
        mark(muteButton, "header:mute");
        mark(recButton, "header:rec");
        mark(variaButton, "header:varia");
        mark(variaIntensity, "header:variaIntensity");
        mark(listeningButton, "header:listening");
        mark(pauseButton, "header:pause");
        mark(audioSettingsButton, "header:audio");
        mark(autonomy, "header:autonomous");
        mark(gate, "header:gate");
        addMouseListener(this, true);
        applyLanguage();
    }

    void mouseMove(const juce::MouseEvent& event) override { updateLearn(event); }
    void mouseEnter(const juce::MouseEvent& event) override { updateLearn(event); }

    void updateLearn(const juce::MouseEvent& event)
    {
        if (!learnMode)
            return;
        juce::String key;
        for (auto* c = event.eventComponent; c != nullptr && c != this; c = c->getParentComponent())
            if (const auto* value = c->getProperties().getVarPointer("learn"))
            {
                key = value->toString();
                break;
            }
        if (key.isEmpty())
            for (const auto& w : cardWidgets())
                if (w.card->isShowing() && w.card->getScreenBounds().contains(event.getScreenPosition()))
                {
                    key = juce::String("card:") + w.id;
                    break;
                }
        if (key.isEmpty() || key == lastLearnKey)
            return;
        lastLearnKey = key;
        showLearn(key, event.eventComponent);
    }

    void showLearn(const juce::String& key, juce::Component* source)
    {
        using namespace aquorbium::app;
        const auto cardEntry = [this](const juce::String& id) {
            if (const auto* c = findCard(id))
                learnPanel.setEntry(T(c->title) + utf8(" · ") + T(c->function), T(c->understand), T(c->explore));
        };
        if (key.startsWith("card:"))
            return cardEntry(key.fromFirstOccurrenceOf("card:", false, false));
        if (key.startsWith("header:"))
        {
            const auto what = key.fromFirstOccurrenceOf("header:", false, false);
            const auto* button = dynamic_cast<juce::Button*>(source);
            const auto title = button != nullptr ? button->getButtonText() : what.toUpperCase();
            const auto tip = what == "tutorial" ? T(S::tutorialTip) : what == "learn" ? T(S::learnTip)
                : what == "about" ? T(S::aboutTip) : what == "language" ? T(S::languageTip)
                : what == "mute" ? T(S::muteTip) : what == "pause" ? T(S::pauseTip)
                : what == "audio" ? T(S::audioTip) : what == "autonomous" ? T(S::autonomousTip)
                : what == "rec" ? T(S::recTip) : what == "listening" ? T(S::listeningTip)
                : what == "varia" ? T(S::varyTip) : what == "variaIntensity" ? T(S::varyIntensityTip) : T(S::gateTip);
            return learnPanel.setEntry(title, tip, {});
        }
        if (key.startsWith("timbre."))
        {
            const auto* c = findCard("timbre");
            const auto what = key.fromFirstOccurrenceOf("timbre.", false, false);
            const auto title = what == "body" ? T(S::body) : what == "matter" ? T(S::matter)
                : what == "mix" ? T(S::mix) : what == "synth" ? T(S::synth::title)
                : what == "edit" ? T(S::synth::edit) : what == "support" ? T(S::support) : T(S::timbreEcology);
            const auto tip = what == "body" ? T(S::bodyTip) : what == "matter" ? T(S::matterTip)
                : what == "mix" ? T(S::mixTip) : what == "synth" ? T(S::synth::mixTip)
                : what == "edit" ? T(S::synth::editTip) : what == "support" ? T(S::supportTip) : T(S::timbreEcologyTip);
            return learnPanel.setEntry(title, tip, c != nullptr ? T(c->explore) : juce::String());
        }
        if (key.startsWith("module:"))
        {
            const auto index = static_cast<std::size_t>(juce::jlimit(0, 15, key.fromFirstOccurrenceOf("module:", false, false).getIntValue()));
            const auto* c = findCard("modules");
            return learnPanel.setEntry(moduleBankDescriptors[index].label, T(moduleDetails()[index]),
                                       c != nullptr ? T(c->explore) : juce::String());
        }
        if (const auto* control = findControl(key))
        {
            auto body = T(control->tip);
            if (control->learn.en != nullptr)
                body += "\n" + T(control->learn);
            juce::String explore;
            if (source != nullptr)
                if (const auto* card = findCard(source->getProperties()["card"].toString()))
                    explore = T(card->explore);
            learnPanel.setEntry(T(control->name), body, explore);
        }
    }

    // every visible text, in the chosen language
    void applyLanguage()
    {
        using namespace aquorbium::app;
        synthEditor.refreshTexts();
        synthMixCardLabel.setText(T(S::synth::title), juce::dontSendNotification);
        synthEditButton.setButtonText(T(S::synth::edit));
        synthMixCard.setTooltip(T(S::synth::mixTip));
        synthEditButton.setTooltip(T(S::synth::editTip));
        tutorialButton.setButtonText(T(S::tutorial));
        learnButton.setButtonText(T(S::learn));
        aboutButton.setButtonText(T(S::about));
        languageButton.setButtonText(langCode(currentLang()).toUpperCase());
        tutorialButton.setTooltip(T(S::tutorialTip));
        learnButton.setTooltip(T(S::learnTip));
        aboutButton.setTooltip(T(S::aboutTip));
        languageButton.setTooltip(T(S::languageTip));
        audioSettingsButton.setButtonText(T(S::audio));
        audioSettingsButton.setTooltip(T(S::audioTip));
        muteButton.setButtonText(muteButton.getToggleState() ? T(S::unmute) : T(S::mute));
        muteButton.setTooltip(T(S::muteTip));
        pauseButton.setButtonText(pauseButton.getToggleState() ? T(S::resume) : T(S::pause));
        pauseButton.setTooltip(T(S::pauseTip));
        recButton.setTooltip(T(S::recTip));
        variaButton.setButtonText(T(S::vary));
        variaButton.setTooltip(T(S::varyTip));
        variaIntensity.setTooltip(T(S::varyIntensityTip));
        listeningButton.setButtonText(T(listeningButton.getToggleState() ? S::headphones : S::speakers));
        listeningButton.setTooltip(T(S::listeningTip));
        brainCard.setStatus(T(S::autonomousState));
        autonomy.setButtonText(T(S::autonomous));
        autonomy.setTooltip(T(S::autonomousTip));
        gate.setButtonText(T(S::gate));
        gate.setTooltip(T(S::gateTip));
        for (const auto& w : controlWidgets())
            if (const auto* text = findControl(w.id))
            {
                w.label->setText(T(text->name), juce::dontSendNotification);
                w.slider->setName(T(text->name));
                w.slider->setTitle(T(text->tip));
                w.slider->setTooltip(T(text->tip));
            }
        masterEqOrder.updateText();  // BEFORE / AFTER in the new language
        for (std::size_t i = 0; i < moduleBankSliders.size(); ++i)
        {
            moduleBankSliders[i].setTooltip(T(moduleDetails()[i]));
            moduleBankSliders[i].setTitle(T(moduleDetails()[i]));
        }
        for (const auto& w : cardWidgets())
            if (const auto* text = findCard(w.id))
                w.card->setTexts(T(text->title), T(text->function), T(text->understand));
        meterCard.setTexts(T(S::meters), T(S::metersStatus), T(S::metersTip));
        lensCard.setTexts(T(S::lens), T(S::lensFunction), T(S::lensTip));
        if (const auto* trim = findControl("masterTrim"))
        {
            headerVolumeLabel.setText(T(trim->name), juce::dontSendNotification);
            headerVolume.setTooltip(T(trim->tip));
        }
        lensCard.setStatus(T(S::glass));
        brainRoomCard.setTexts(T(S::brainHeading) + utf8(" · ") + T(S::roomHeading), T(S::brainHeading),
                               T(S::brainHeading) + utf8(" · ") + T(S::roomHeading));
        brainRoomCard.setStatus(T(S::microphone));
        lensCard.setTooltip(T(S::lensTip));
        brainHeading.setText(T(S::brainHeading), juce::dontSendNotification);
        roomHeading.setText(T(S::roomHeading), juce::dontSendNotification);
        meterCard.setStatus(T(S::metersStatus));
        meterCard.setTooltip(T(S::metersTip));
        meterPanel.setTooltip(T(S::metersTip));
        // timbre card
        timbreHeadings[0].setText(T(S::body), juce::dontSendNotification);
        timbreHeadings[1].setText(T(S::matter), juce::dontSendNotification);
        timbreHeadings[2].setText(T(S::mix), juce::dontSendNotification);
        timbreEcologyLabel.setText(T(S::timbreEcology), juce::dontSendNotification);
        timbreEcology.setTooltip(T(S::timbreEcologyTip));
        supportPresenceLabel.setText(T(S::support), juce::dontSendNotification);
        supportPresence.setTooltip(T(S::supportTip));
        for (std::size_t i = 0; i < 5; ++i)
        {
            const auto selected = timbreBodySelect[i].getSelectedId();
            timbreBodySelect[i].clear(juce::dontSendNotification);
            for (int k = 0; k < static_cast<int>(aquorbium::TimbreBody::kindCount); ++k)
                timbreBodySelect[i].addItem(T(S::bodies[static_cast<std::size_t>(k)]), k + 1);
            timbreBodySelect[i].setSelectedId(std::max(1, selected), juce::dontSendNotification);
            timbreBodySelect[i].setTooltip(T(S::bodyTip));
            timbreMatter[i].setTooltip(T(S::matterTip));
            timbreMix[i].setTooltip(T(S::mixTip));
        }
        // scenes
        presetStore.setButtonText(T(S::store));
        presetRecall.setButtonText(T(S::recall));
        presetDefault.setButtonText(T(S::initial));
        presetImport.setButtonText(T(S::importScene));
        presetExport.setButtonText(T(S::exportScene));
        presetRename.setButtonText(T(S::rename));
        presetUndo.setButtonText(T(S::undo));
        presetName.setTextToShowWhenEmpty(T(S::sceneNameHint), Palette::muted);
        presetName.setTooltip(T(S::nameTip));
        updatePresetStatus();
        updateEcologySurface();
        updateControlAvailability();
        guide.refresh();
        if (learnMode)
        {
            lastLearnKey.clear();
            learnPanel.setEntry(T(learnEmpty), T(S::learnTip), {});
        }
        ecosystemView.refreshTexts();
        ecosystemView.repaint();
        resized();  // the header buttons' widths follow the language
    }

    // TIMBRE: per organism a second body (a selector), its MATTER and its MIX
    // with the native body; ECOLOGY lets the ecosystem move matter and
    // presence. The selector is backed by a hidden 0..1 slider, the persisted
    // parameter (index / 6), so scenes and presets treat it like the others.
    void configureTimbreControls()
    {
        static constexpr std::array<const char*, 5> organisms { "MORPH", "GRAIN", "MODAL", "PULSAR", "STRING" };
        static constexpr std::array<const char*, 3> headings { "CORPO", "MATÉRIA", "MISTURA" };
        for (std::size_t h = 0; h < headings.size(); ++h)
        {
            timbreHeadings[h].setText(utf8(headings[h]), juce::dontSendNotification);
            timbreHeadings[h].setFont(uiFont(9.5f, juce::Font::bold));
            timbreHeadings[h].setColour(juce::Label::textColourId, Palette::muted);
            timbreHeadings[h].setJustificationType(juce::Justification::centredLeft);
            timbreControls.addAndMakeVisible(timbreHeadings[h]);
        }
        const auto setupBar = [](juce::Slider& slider, double initial, const juce::String& tip) {
            slider.setRange(0.0, 1.0, 0.01);
            slider.setSliderStyle(juce::Slider::LinearHorizontal);
            slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
            slider.setPopupDisplayEnabled(true, true, nullptr);
            slider.setColour(juce::Slider::trackColourId, Palette::coral.withAlpha(0.72f));
            slider.setColour(juce::Slider::backgroundColourId, Palette::line.withAlpha(0.46f));
            slider.setColour(juce::Slider::thumbColourId, Palette::text);
            slider.setTooltip(tip);
            slider.setScrollWheelEnabled(false);
            slider.setValue(initial, juce::dontSendNotification);
        };
        for (std::size_t i = 0; i < 5; ++i)
        {
            timbreNames[i].setText(organisms[i], juce::dontSendNotification);
            timbreNames[i].setFont(uiFont(10.0f, juce::Font::bold));
            timbreNames[i].setColour(juce::Label::textColourId, Palette::text);
            timbreControls.addAndMakeVisible(timbreNames[i]);
            for (int k = 0; k < static_cast<int>(aquorbium::TimbreBody::kindCount); ++k)
                timbreBodySelect[i].addItem(utf8(aquorbium::TimbreBody::name(
                    static_cast<aquorbium::TimbreBody::Kind>(k))), k + 1);
            timbreBodySelect[i].setSelectedId(1, juce::dontSendNotification);
            timbreBodySelect[i].setTooltip(utf8("O segundo corpo deste organismo: voz (canto formante), matéria (modos harmônicos → membrana), sopro, corda, gota d'água, dobra, membrana (tambor d'água → pele), vidro friccionado ou coro."));
            timbreBodySelect[i].onChange = [this, i] {
                const auto index = timbreBodySelect[i].getSelectedId() - 1;
                timbreBodyValue[i].setValue(static_cast<double>(std::max(0, index)) / aquorbium::TimbreBody::kindScale,
                                            juce::sendNotificationSync);
            };
            timbreControls.addAndMakeVisible(timbreBodySelect[i]);
            timbreBodyValue[i].setRange(0.0, 1.0, 0.0);
            timbreLegacyBody[i].setRange(0.0, 1.0, 0.0);
            setupBar(timbreMatter[i], 0.5, utf8("MATÉRIA: o caráter dentro do corpo (as vogais da voz e do coro; harmônico → membrana; o ar do sopro; dedilhada → friccionada; gota → chuva; a dobra; tambor solto → pele esticada; o brilho e o batimento do vidro)."));
            setupBar(timbreMix[i], 0.0, utf8("MISTURA: quanto do segundo corpo soa junto ao corpo nativo."));
            timbreControls.addAndMakeVisible(timbreMatter[i]);
            timbreControls.addAndMakeVisible(timbreMix[i]);
            timbreMatterTarget[i].store(0.5f, std::memory_order_relaxed);
        }
        setupBar(timbreEcology, 0.0, utf8("TIMBRE ECOLÓGICO: o quanto a vida do ecossistema transforma a matéria e a presença dos corpos (a água, a caça, a fuga, o acasalamento, o vigor, a saúde)."));
        timbreEcologyLabel.setText(utf8("TIMBRE ECOLÓGICO"), juce::dontSendNotification);
        timbreEcologyLabel.setFont(uiFont(9.5f, juce::Font::bold));
        timbreEcologyLabel.setColour(juce::Label::textColourId, Palette::muted);
        timbreControls.addAndMakeVisible(timbreEcology);
        timbreControls.addAndMakeVisible(timbreEcologyLabel);
        // SUPPORT (2026-10-23): the support species' level, 0.2 = 0 dB
        setupBar(supportPresence, 0.2, utf8("SUSTENTAÇÃO: o nível de GRAIN, MODAL e STRING diante de MORPH e PULSAR, de -6 a +24 dB."));
        supportPresence.textFromValueFunction = [](double v) {
            return juce::String(-6.0 + 30.0 * v, 1) + " dB";
        };
        supportPresence.updateText();
        supportPresenceLabel.setFont(uiFont(9.5f, juce::Font::bold));
        supportPresenceLabel.setColour(juce::Label::textColourId, Palette::muted);
        timbreControls.addAndMakeVisible(supportPresence);
        timbreControls.addAndMakeVisible(supportPresenceLabel);
        // the synthesis device: its mix and the editor
        setupBar(synthMixCard, 0.0, aquorbium::app::T(aquorbium::app::S::synth::mixTip));
        synthMixCard.onValueChange = [this] {
            synthEditor.mix = static_cast<float>(synthMixCard.getValue());
            synthEditor.loadControls();
            synthChanged();
        };
        synthMixCardLabel.setFont(uiFont(9.5f, juce::Font::bold));
        synthMixCardLabel.setColour(juce::Label::textColourId, Palette::cyan);
        synthEditButton.onClick = [this] {
            synthEditorViewport.setVisible(true);
            synthEditorViewport.toFront(false);
            synthEditor.show();
        };
        timbreControls.addAndMakeVisible(synthMixCard);
        timbreControls.addAndMakeVisible(synthMixCardLabel);
        timbreControls.addAndMakeVisible(synthEditButton);
    }

    static constexpr int timbreCardHeight(bool wide) noexcept
    {
        // the wide surface packs the rows (organisms 36, controls 34) so that
        // SUPPORT (2026-10-23) fits in the same height as before it
        return wide ? 40 + 18 + 5 * 36 + 3 * 34 + 8
                    : 68 + 18 + 5 * 40 + 40 + 40 + 40 + 8;
    }

    void layoutTimbrePerformance()
    {
        const auto wide = surfaceMode == aquorbium::windowing::SurfaceMode::wide;
        auto area = timbreCard.getBounds().reduced(10, 0);
        area.removeFromTop(wide ? 40 : 68);
        area.removeFromBottom(6);
        timbreControls.setBounds(area);
        auto inner = timbreControls.getLocalBounds();
        const auto nameWidth = 58;
        const auto bodyWidth = juce::jlimit(80, 110, inner.getWidth() / 3);
        auto heading = inner.removeFromTop(18);
        heading.removeFromLeft(nameWidth);
        timbreHeadings[0].setBounds(heading.removeFromLeft(bodyWidth));
        const auto barWidth = heading.getWidth() / 2;
        timbreHeadings[1].setBounds(heading.removeFromLeft(barWidth));
        timbreHeadings[2].setBounds(heading);
        const auto organismRow = wide ? 36 : 40, controlRow = wide ? 34 : 40;
        for (std::size_t i = 0; i < 5; ++i)
        {
            auto row = inner.removeFromTop(organismRow).reduced(0, wide ? 5 : 6);
            timbreNames[i].setBounds(row.removeFromLeft(nameWidth));
            timbreBodySelect[i].setBounds(row.removeFromLeft(bodyWidth).reduced(0, 1));
            row.removeFromLeft(6);
            timbreMatter[i].setBounds(row.removeFromLeft(row.getWidth() / 2).reduced(2, 0));
            timbreMix[i].setBounds(row.reduced(2, 0));
        }
        auto ecology = inner.removeFromTop(controlRow).reduced(0, wide ? 6 : 8);
        timbreEcologyLabel.setBounds(ecology.removeFromLeft(nameWidth + bodyWidth));
        timbreEcology.setBounds(ecology.reduced(2, 0));
        auto supportRow = inner.removeFromTop(controlRow).reduced(0, wide ? 6 : 8);
        supportPresenceLabel.setBounds(supportRow.removeFromLeft(nameWidth + bodyWidth));
        supportPresence.setBounds(supportRow.reduced(2, 0));
        auto synthRow = inner.removeFromTop(controlRow).reduced(0, wide ? 4 : 6);
        synthMixCardLabel.setBounds(synthRow.removeFromLeft(nameWidth));
        synthEditButton.setBounds(synthRow.removeFromRight(90).reduced(0, 1));
        synthRow.removeFromRight(6);
        synthMixCard.setBounds(synthRow.reduced(2, 2));
    }

    [[nodiscard]] int brandWidth() const noexcept
    {
        return getWidth() >= 1200 ? 300 : 230;
    }
    int headerButtonsLeft = 0;
    mutable std::unique_ptr<juce::Drawable> logo;
    mutable std::unique_ptr<juce::Drawable> wordmark;

    void layoutLearnPanel()
    {
        const auto view = ecosystemView.getBounds();
        learnPanel.setBounds(view.getX() + 10, view.getBottom() - 176, view.getWidth() - 20, 166);
        if (learnMode)
            learnPanel.toFront(false);
    }

    // The cards that live in the scrolling right column on smaller screens.
    // On the wide surface they become children of this component, placed
    // around the aquarium; elsewhere they return to the scrolling column.
    void adoptSoundCards(bool wide)
    {
        juce::Component& parent = wide ? static_cast<juce::Component&>(*this)
                                       : static_cast<juce::Component&>(rightColumnControls);
        for (auto* component : { static_cast<juce::Component*>(&synthesisCard),
                                 static_cast<juce::Component*>(&spaceCard),
                                 static_cast<juce::Component*>(&environmentCard),
                                 static_cast<juce::Component*>(&tailCard),
                                 static_cast<juce::Component*>(&masterCard),
                                 static_cast<juce::Component*>(&moduleBankCard),
                                 static_cast<juce::Component*>(&synthesisViewport),
                                 static_cast<juce::Component*>(&spaceViewport),
                                 static_cast<juce::Component*>(&environmentViewport),
                                 static_cast<juce::Component*>(&tailViewport),
                                 static_cast<juce::Component*>(&masterViewport),
                                 static_cast<juce::Component*>(&moduleBankViewport),
                                 static_cast<juce::Component*>(&timbreCard),
                                 static_cast<juce::Component*>(&timbreControls),
                                 static_cast<juce::Component*>(&meterCard),
                                 static_cast<juce::Component*>(&meterPanel),
                                 static_cast<juce::Component*>(&lensCard),
                                 static_cast<juce::Component*>(&lensViewport),
                                 static_cast<juce::Component*>(&brainRoomCard),
                                 static_cast<juce::Component*>(&brainRoomControls) })
            if (component->getParentComponent() != &parent)
                parent.addAndMakeVisible(*component);
        rightColumnViewport.setVisible(!wide);
    }

    // Every control visible, the aquarium at the centre and the modules
    // around it (the author, 2026-10-20: "prefiro quando os controles são
    // visíveis"; "o quadro do ecossistema ocupa muito da tela"; "o ecossistema
    // deve continuar no centro e os módulos ao redor dele"). Two columns of
    // cards on each side, four knobs per row; a low strip for the sliders.
    void layoutWideSurface()
    {
        using Panel = aquorbium::windowing::MacroPanelLayout;
        constexpr int gap = 10;
        constexpr int columnWidth = 336;
        adoptSoundCards(true);
        auto area = getLocalBounds().reduced(16);
        area.removeFromTop(48);
        auto controls = area.removeFromBottom(112);
        area.removeFromBottom(gap);

        auto composition = area.removeFromLeft(columnWidth);
        area.removeFromLeft(gap);
        auto sound = area.removeFromLeft(columnWidth);
        area.removeFromLeft(gap);
        auto lab = area.removeFromRight(columnWidth);
        area.removeFromRight(gap);
        auto space = area.removeFromRight(columnWidth);
        area.removeFromRight(gap);
        ecosystemView.setBounds(area);
        // the wide surface has room for CÉREBRO and SALA in the ecology panel
        brainRoomInCard = false;
        brainRoomCard.setVisible(false);
        brainRoomControls.setVisible(false);
        layoutEcologySurface();

        auto placeIn = [&](juce::Rectangle<int>& column, ModuleCard& card, std::size_t count) {
            card.setBounds(column.removeFromTop(Panel::wideCardHeight(count)));
            column.removeFromTop(gap);
        };
        brainCard.setBounds(composition.removeFromTop(Panel::wideHeaderHeight));
        composition.removeFromTop(gap);
        placeIn(composition, geometryCard, 12);
        timbreCard.setBounds(composition.removeFromTop(timbreCardHeight(true)));
        placeIn(sound, synthesisCard, 4);
        placeIn(sound, tailCard, 4);
        placeIn(sound, masterCard, 9);
        placeIn(space, spaceCard, 6);
        placeIn(space, environmentCard, 6);
        if (space.getHeight() >= Panel::wideCardHeight(4))
        {
            placeIn(space, lensCard, 4);
            lensCard.setVisible(true);
            lensViewport.setVisible(true);
        }
        else
        {
            lensCard.setVisible(false);
            lensViewport.setVisible(false);
        }
        placeIn(lab, moduleBankCard, 16);
        // MEDIDORES below the modules, if the column has the room
        if (lab.getHeight() >= Panel::wideHeaderHeight + MeterPanel::preferredHeight + 8)
        {
            meterCard.setBounds(lab.removeFromTop(Panel::wideHeaderHeight + MeterPanel::preferredHeight + 8));
            meterPanel.setBounds(meterCard.getBounds().withTrimmedTop(Panel::wideHeaderHeight).reduced(10, 2));
            meterCard.setVisible(true);
            meterPanel.setVisible(true);
        }
        else
        {
            meterCard.setVisible(false);
            meterPanel.setVisible(false);
        }
        layoutFormMemoryPerformance();
        layoutSynthesisPerformance();
        layoutSpacePerformance();
        layoutEnvironmentPerformance();
        layoutTailPerformance();
        layoutMasterPerformance();
        layoutLensPerformance();
        layoutModuleBankPerformance();
        layoutTimbrePerformance();

        // the low strip: switches | ecology | geometry (two sub-columns)
        auto switches = controls.removeFromLeft(columnWidth).reduced(8, 6);
        // three equal rows: the autonomous bioma, the gate, the VARIA
        const auto switchRow = std::min(40, switches.getHeight() / 3);
        autonomy.setBounds(switches.removeFromTop(switchRow));
        gate.setBounds(switches.removeFromTop(switchRow));
        layoutVaria(switches.removeFromTop(switchRow));
        controls.removeFromLeft(gap);
        const auto third = (controls.getWidth() - gap * 2) / 3;
        auto ecology = controls.removeFromLeft(third);
        controls.removeFromLeft(gap);
        auto geometryA = controls.removeFromLeft(third);
        controls.removeFromLeft(gap);
        auto geometryB = controls;
        layoutControl(ecology, densityLabel, density, 36, 170);
        layoutControl(ecology, driftLabel, drift, 36, 170);
        layoutControl(ecology, ecosystemMorphLabel, ecosystemMorph, 36, 170);
        layoutControl(geometryA, geometryInfluenceLabel, geometryInfluence, 36, 170);
        layoutControl(geometryA, rhythmDensityLabel, rhythmDensity, 36, 170);
        layoutControl(geometryA, orbitRotationLabel, orbitRotation, 36, 170);
        layoutControl(geometryB, intervalBiasLabel, intervalBias, 36, 170);
        layoutControl(geometryB, geometryMutationLabel, geometryMutation, 36, 170);
    }

    void layoutFullSurface()
    {
        auto area = getLocalBounds().reduced(24);
        area.removeFromTop(64);
        auto controls = area.removeFromBottom(202);
        area.removeFromBottom(14);

        constexpr int gap = 14;
        const auto sideWidth = juce::jlimit(280, 320, area.getWidth() / 5);
        auto left = area.removeFromLeft(sideWidth);
        area.removeFromLeft(gap);
        auto right = area.removeFromRight(sideWidth);
        area.removeFromRight(gap);
        ecosystemView.setBounds(area);
        layoutEcologySurface();

        auto leftTop = left.removeFromTop((left.getHeight() - gap) / 2);
        left.removeFromTop(gap);
        brainCard.setBounds(leftTop);
        geometryCard.setBounds(left);
        layoutFormMemoryPerformance();

        layoutRightColumn(right, gap);

        auto performanceControls = controls.removeFromLeft(sideWidth).reduced(8, 25);
        autonomy.setBounds(performanceControls.removeFromTop(42));
        gate.setBounds(performanceControls.removeFromTop(42));
        layoutVaria(performanceControls.removeFromTop(42));
        controls.removeFromLeft(gap + 10);
        auto ecologyControls = controls.removeFromLeft((controls.getWidth() - gap) / 2);
        controls.removeFromLeft(gap);
        layoutControl(ecologyControls, densityLabel, density, 50, 190);
        layoutControl(ecologyControls, driftLabel, drift, 50, 190);
        layoutControl(ecologyControls, ecosystemMorphLabel, ecosystemMorph, 50, 190);
        layoutControl(controls, geometryInfluenceLabel, geometryInfluence, 38, 170);
        layoutControl(controls, rhythmDensityLabel, rhythmDensity, 38, 170);
        layoutControl(controls, orbitRotationLabel, orbitRotation, 38, 170);
        layoutControl(controls, intervalBiasLabel, intervalBias, 38, 170);
        layoutControl(controls, geometryMutationLabel, geometryMutation, 38, 170);
    }

    void layoutCompactSurface()
    {
        const auto emergency = surfaceMode == aquorbium::windowing::SurfaceMode::emergency;
        const auto margin = emergency ? 10 : 14;
        const auto gap = emergency ? 10 : 12;
        auto area = getLocalBounds().reduced(margin);
        area.removeFromTop(emergency ? 48 : 52);
        auto controls = area.removeFromBottom(emergency ? 174 : 180);
        area.removeFromBottom(gap);

        const auto sideWidth = juce::jlimit(188, 205, area.getWidth() / 5);
        auto left = area.removeFromLeft(sideWidth);
        area.removeFromLeft(gap);
        auto right = area.removeFromRight(sideWidth);
        area.removeFromRight(gap);
        ecosystemView.setBounds(area);
        layoutEcologySurface();

        auto leftTop = left.removeFromTop((left.getHeight() - gap) / 2);
        left.removeFromTop(gap);
        brainCard.setBounds(leftTop);
        geometryCard.setBounds(left);
        layoutFormMemoryPerformance();

        layoutRightColumn(right, gap);

        const auto columnWidth = (controls.getWidth() - gap * 2) / 3;
        auto performance = controls.removeFromLeft(columnWidth);
        controls.removeFromLeft(gap);
        auto ecology = controls.removeFromLeft(columnWidth);
        controls.removeFromLeft(gap);
        auto geometry = controls;

        // the compact surface has no room for a third row: the VARIA shares
        // the autonomous bioma's, and its reach is set on the larger surfaces
        {
            auto first = performance.removeFromTop(34);
            variaButton.setBounds(first.removeFromRight(std::min(110, first.getWidth() / 3)));
            autonomy.setBounds(first);
            variaIntensity.setVisible(false);
        }
        gate.setBounds(performance.removeFromTop(34));
        performance.removeFromTop(2);
        layoutStackedControl(performance, densityLabel, density, 48);
        layoutStackedControl(performance, driftLabel, drift, 48);

        layoutStackedControl(ecology, ecosystemMorphLabel, ecosystemMorph, 56);
        layoutStackedControl(ecology, geometryInfluenceLabel, geometryInfluence, 56);
        layoutStackedControl(ecology, rhythmDensityLabel, rhythmDensity, 56);

        layoutStackedControl(geometry, orbitRotationLabel, orbitRotation, 56);
        layoutStackedControl(geometry, intervalBiasLabel, intervalBias, 56);
        layoutStackedControl(geometry, geometryMutationLabel, geometryMutation, 56);
    }

    void layoutRightColumn(juce::Rectangle<int> bounds, int gap)
    {
        adoptSoundCards(false);

        rightColumnViewport.setBounds(bounds);
        const auto width = bounds.getWidth() - 12;
        const auto synthesisHeight = 76 + aquorbium::windowing::MacroPanelLayout::rowHeight * 2;
        const auto spaceHeight = 76 + aquorbium::windowing::MacroPanelLayout::rowHeight
            * (width >= 268 ? 2 : 3);
        const auto environmentHeight = 76 + aquorbium::windowing::MacroPanelLayout::rowHeight
            * (width >= 268 ? 2 : 3);
        const auto tailHeight = 76 + aquorbium::windowing::MacroPanelLayout::rowHeight * 2;
        // nine controls since the creative master's insides (2026-10-24):
        // five rows of two
        const auto masterHeight = 76 + aquorbium::windowing::MacroPanelLayout::rowHeight
            * 5;
        const auto moduleBankHeight = 76
            + aquorbium::windowing::MacroPanelLayout::rowHeight * 3;
        const auto meterHeight = 76 + MeterPanel::preferredHeight;
        const auto lensHeight = 76 + aquorbium::windowing::MacroPanelLayout::rowHeight * 2;
        const auto brainRoomHeight = 76 + 6 * 30 + 8;
        const auto minimumHeight = synthesisHeight + spaceHeight + environmentHeight
            + tailHeight + masterHeight + moduleBankHeight + timbreCardHeight(false) + meterHeight
            + lensHeight + brainRoomHeight + gap * 9;
        const auto extra = std::max(0, bounds.getHeight() - minimumHeight);
        rightColumnControls.setSize(width, minimumHeight + extra);
        synthesisCard.setBounds(0, 0, width, synthesisHeight + extra / 4);
        spaceCard.setBounds(0, synthesisCard.getBottom() + gap, width,
                            spaceHeight + extra / 4);
        environmentCard.setBounds(0, spaceCard.getBottom() + gap, width,
                                  environmentHeight + extra / 4);
        tailCard.setBounds(0, environmentCard.getBottom() + gap, width,
                           tailHeight + extra - extra * 3 / 4);
        masterCard.setBounds(0, tailCard.getBottom() + gap, width, masterHeight);
        moduleBankCard.setBounds(0, masterCard.getBottom() + gap,
                                 width, moduleBankHeight);
        timbreCard.setBounds(0, moduleBankCard.getBottom() + gap, width, timbreCardHeight(false));
        meterCard.setBounds(0, timbreCard.getBottom() + gap, width, meterHeight);
        meterPanel.setBounds(meterCard.getBounds().withTrimmedTop(68).reduced(10, 2));
        meterCard.setVisible(true);
        meterPanel.setVisible(true);
        lensCard.setBounds(0, meterCard.getBottom() + gap, width, lensHeight);
        lensCard.setVisible(true);
        lensViewport.setVisible(true);
        layoutLensPerformance();
        // the CÉREBRO and SALA rows, in their card
        brainRoomInCard = true;
        brainRoomCard.setBounds(0, lensCard.getBottom() + gap, width, brainRoomHeight);
        brainRoomCard.setVisible(true);
        brainRoomControls.setBounds(brainRoomCard.getBounds().withTrimmedTop(68).reduced(12, 0));
        brainRoomControls.setVisible(true);
        placeBrainRoomRows(brainRoomControls, brainRoomControls.getLocalBounds(),
                           std::min(130, brainRoomControls.getWidth() / 3), 30, 28);
        updateEcologySurface();
        layoutTimbrePerformance();
        layoutSynthesisPerformance();
        layoutSpacePerformance();
        layoutEnvironmentPerformance();
        layoutTailPerformance();
        layoutMasterPerformance();
        layoutModuleBankPerformance();
    }

    void layoutSynthesisPerformance()
    {
        std::array<juce::Label*, 4> labels {
            &strikerInfluenceLabel, &strikerMatterLabel,
            &strikerPopulationLabel, &strikerScatterLabel
        };
        std::array<juce::Slider*, 4> sliders {
            &strikerInfluence, &strikerMatter, &strikerPopulation, &strikerScatter
        };
        layoutMacroPanel(synthesisCard, synthesisViewport, synthesisControls, labels, sliders);
    }

    void layoutFormMemoryPerformance()
    {
        std::array<juce::Label*, 12> labels {
            &formInfluenceLabel, &formContextLabel,
            &formInertiaLabel, &formMutationLabel,
            &structureInfluenceLabel, &structureMemoryLabel,
            &structureHarmonyLabel, &structurePhaseLabel,
            &musicInfluenceLabel, &musicReactionLabel,
            &musicMemoryLabel, &musicWaterLabel
        };
        std::array<juce::Slider*, 12> sliders {
            &formInfluence, &formContext, &formInertia, &formMutation,
            &structureInfluence, &structureMemory, &structureHarmony, &structurePhase,
            &musicInfluence, &musicReaction, &musicMemory, &musicWater
        };
        layoutMacroPanel(geometryCard, geometryMemoryViewport,
                         geometryMemoryControls, labels, sliders);
    }

    void layoutEcologySurface()
    {
        const auto bounds = ecosystemView.getBounds();
        ecologyControlsBackdrop.setBounds(bounds);
        const auto layout = aquorbium::windowing::EcologyPanelLayout::calculate(
            { bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight() });
        const auto rectangle = [](aquorbium::windowing::Bounds item) {
            return juce::Rectangle<int>(item.x, item.y, item.width, item.height);
        };
        ecologyControlsToggle.setBounds(rectangle(layout.toggle));
        presetStatus.setBounds(rectangle(layout.presetStatus));
        presetName.setBounds(rectangle(layout.nameEditor));
        presetRename.setBounds(rectangle(layout.renameButton));
        presetUndo.setBounds(rectangle(layout.undoButton));
        std::size_t buttonIndex = 0;
        for (auto* component : { static_cast<juce::Component*>(&presetSlot),
                                 static_cast<juce::Component*>(&presetStore),
                                 static_cast<juce::Component*>(&presetRecall),
                                 static_cast<juce::Component*>(&presetDefault) })
        {
            component->setBounds(rectangle(layout.presetButtons[buttonIndex++]));
        }
        presetImport.setBounds(rectangle(layout.transferButtons[0]));
        presetExport.setBounds(rectangle(layout.transferButtons[1]));
        std::array<juce::Slider*, 6> sliders {
            &cohesion, &danger, &sparseFocus, &energyMemory,
            &geometricCycle, &collectiveDrift
        };
        std::array<juce::Label*, 6> labels {
            &cohesionLabel, &dangerLabel, &sparseFocusLabel, &energyMemoryLabel,
            &geometricCycleLabel, &collectiveDriftLabel
        };
        for (std::size_t i = 0; i < sliders.size(); ++i)
        {
            auto row = rectangle(layout.rows[i]);
            labels[i]->setBounds(row.removeFromLeft(layout.labelWidth));
            sliders[i]->setBounds(row);
        }
        // the CÉREBRO and SALA rows (2026-10-24), in the room between the
        // ecology rows and the scene buttons, at the same pitch if it fits
        {
            const auto last = rectangle(layout.rows[sliders.size() - 1]);
            const auto top = last.getBottom() + 10;
            const auto bottom = rectangle(layout.presetButtons[0]).getY() - 10;
            const auto pitch = std::min(last.getHeight() + 8, std::max(0, (bottom - top) / 6));
            cardRowsFit = pitch >= 22 && !brainRoomInCard;
            if (cardRowsFit)
                placeBrainRoomRows(*this, { last.getX(), top, last.getWidth(), bottom - top },
                                   layout.labelWidth, pitch, std::min(pitch, last.getHeight()));
            updateEcologySurface();
        }
        ecologyControlsToggle.toFront(false);
        keepSynthEditorOnTop();
    }

    void updateEcologySurface()
    {
        ecosystemView.setVisible(!showEcologyControls);
        ecologyControlsBackdrop.setVisible(showEcologyControls);
        for (auto* component : { static_cast<juce::Component*>(&cohesion),
                                 static_cast<juce::Component*>(&danger),
                                 static_cast<juce::Component*>(&sparseFocus),
                                 static_cast<juce::Component*>(&energyMemory),
                                 static_cast<juce::Component*>(&geometricCycle),
                                 static_cast<juce::Component*>(&collectiveDrift),
                                 static_cast<juce::Component*>(&cohesionLabel),
                                 static_cast<juce::Component*>(&dangerLabel),
                                 static_cast<juce::Component*>(&sparseFocusLabel),
                                 static_cast<juce::Component*>(&energyMemoryLabel),
                                 static_cast<juce::Component*>(&geometricCycleLabel),
                                 static_cast<juce::Component*>(&collectiveDriftLabel),
                                 static_cast<juce::Component*>(&presetSlot),
                                 static_cast<juce::Component*>(&presetStore),
                                 static_cast<juce::Component*>(&presetRecall),
                                 static_cast<juce::Component*>(&presetDefault),
                                 static_cast<juce::Component*>(&presetImport),
                                 static_cast<juce::Component*>(&presetExport),
                                 static_cast<juce::Component*>(&presetName),
                                 static_cast<juce::Component*>(&presetRename),
                                 static_cast<juce::Component*>(&presetUndo),
                                 static_cast<juce::Component*>(&presetStatus) })
            component->setVisible(showEcologyControls);
        for (auto* component : { static_cast<juce::Component*>(&cardSliders[8]), static_cast<juce::Component*>(&cardSliders[9]),
                                 static_cast<juce::Component*>(&cardSliders[10]), static_cast<juce::Component*>(&cardSliders[11]),
                                 static_cast<juce::Component*>(&cardLabels[8]), static_cast<juce::Component*>(&cardLabels[9]),
                                 static_cast<juce::Component*>(&cardLabels[10]), static_cast<juce::Component*>(&cardLabels[11]),
                                 static_cast<juce::Component*>(&brainHeading), static_cast<juce::Component*>(&brainStatus),
                                 static_cast<juce::Component*>(&roomHeading), static_cast<juce::Component*>(&roomStatus) })
        {
            component->setVisible(brainRoomInCard || (showEcologyControls && cardRowsFit));
            if (component->isVisible() && !brainRoomInCard)
                component->toFront(false);  // above the ecology backdrop
        }
        ecologyControlsToggle.setButtonText(showEcologyControls
                                                ? aquorbium::app::T(aquorbium::app::S::aquariumView) : aquorbium::app::T(aquorbium::app::S::ecoControls));
        ecologyControlsToggle.toFront(false);
        keepSynthEditorOnTop();
    }

    void layoutSpacePerformance()
    {
        std::array<juce::Label*, 6> labels {
            &spaceReturnLabel, &spaceGestureLabel, &spaceSizeLabel,
            &spaceSpreadLabel, &spaceToneLabel, &spaceMemoryLabel
        };
        std::array<juce::Slider*, 6> sliders {
            &spaceReturn, &spaceGesture, &spaceSize,
            &spaceSpread, &spaceTone, &spaceMemory
        };
        layoutMacroPanel(spaceCard, spaceViewport, spaceControls, labels, sliders);
    }

    void layoutEnvironmentPerformance()
    {
        std::array<juce::Label*, 6> labels {
            &environmentInfluenceLabel, &environmentFilterMixLabel,
            &environmentCentreLabel, &environmentSpanLabel,
            &environmentResonanceLabel, &environmentAmbienceLabel
        };
        std::array<juce::Slider*, 6> sliders {
            &environmentInfluence, &environmentFilterMix,
            &environmentCentre, &environmentSpan,
            &environmentResonance, &environmentAmbience
        };
        layoutMacroPanel(environmentCard, environmentViewport,
                         environmentControls, labels, sliders);
    }

    void layoutTailPerformance()
    {
        std::array<juce::Label*, 4> labels {
            &tailInfluenceLabel, &tailDecayLabel,
            &tailDampingLabel, &tailNightmareLabel
        };
        std::array<juce::Slider*, 4> sliders {
            &tailInfluence, &tailDecay, &tailDamping, &tailNightmare
        };
        layoutMacroPanel(tailCard, tailViewport, tailControls, labels, sliders);
    }

    void layoutLensPerformance()
    {
        std::array<juce::Label*, 4> labels { &cardLabels[0], &cardLabels[1], &cardLabels[2], &cardLabels[3] };
        std::array<juce::Slider*, 4> sliders { &cardSliders[0], &cardSliders[1], &cardSliders[2], &cardSliders[3] };
        layoutMacroPanel(lensCard, lensViewport, lensControls, labels, sliders);
    }

    void layoutMasterPerformance()
    {
        std::array<juce::Label*, 9> labels {
            &creativeMasterLabel, &masterTrimLabel,
            &masterCeilingLabel, &masterReleaseLabel, &masterEqOrderLabel,
            &cardLabels[4], &cardLabels[5], &cardLabels[6], &cardLabels[7]
        };
        std::array<juce::Slider*, 9> sliders {
            &creativeMaster, &masterTrim, &masterCeiling, &masterRelease, &masterEqOrder,
            &cardSliders[4], &cardSliders[5], &cardSliders[6], &cardSliders[7]
        };
        layoutMacroPanel(masterCard, masterViewport, masterControls, labels, sliders);
    }

    void layoutModuleBankPerformance()
    {
        std::array<juce::Label*, aquorbium::IntegratedModuleBank::controlCount> labels {};
        std::array<juce::Slider*, aquorbium::IntegratedModuleBank::controlCount> sliders {};
        for (std::size_t index = 0; index < labels.size(); ++index)
        {
            labels[index] = &moduleBankLabels[index];
            sliders[index] = &moduleBankSliders[index];
        }
        layoutMacroPanel(moduleBankCard, moduleBankViewport,
                         moduleBankControls, labels, sliders);
    }

    template <std::size_t Count>
    void layoutMacroPanel(ModuleCard& card, juce::Viewport& viewport,
                          juce::Component& content,
                          const std::array<juce::Label*, Count>& labels,
                          const std::array<juce::Slider*, Count>& sliders)
    {
        const auto bounds = card.getBounds();
        const aquorbium::windowing::Bounds cardBounds {
            bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight() };
        const auto layout = surfaceMode == aquorbium::windowing::SurfaceMode::wide
            ? aquorbium::windowing::MacroPanelLayout::calculateWide(cardBounds, Count)
            : aquorbium::windowing::MacroPanelLayout::calculate(cardBounds, Count);
        const auto rectangle = [](aquorbium::windowing::Bounds item) {
            return juce::Rectangle<int>(item.x, item.y, item.width, item.height);
        };
        viewport.setBounds(rectangle(layout.viewport));
        content.setSize(layout.content.width, layout.content.height);
        for (std::size_t index = 0; index < Count; ++index)
        {
            labels[index]->setBounds(rectangle(layout.labels[index]));
            sliders[index]->setBounds(rectangle(layout.sliders[index]));
        }
    }

    void updateControlAvailability()
    {
        const auto strikerActive = strikerInfluence.getValue() > 0.0;
        const auto spaceActive = spaceGesture.getValue() > 0.0;
        const auto geometryActive = geometryInfluence.getValue() > 0.0;
        const auto environmentActive = environmentInfluence.getValue() > 0.0;
        const auto tailActive = tailInfluence.getValue() > 0.0;
        const auto formActive = formInfluence.getValue() > 0.0;
        const auto structureActive = structureInfluence.getValue() > 0.0;
        const auto musicActive = musicInfluence.getValue() > 0.0;
        ecosystemView.setFreshWater(musicSalinity.getValue() < 0.5);
        int chosenBodies = 0;
        for (std::size_t i = 0; i < 5; ++i)
        {
            const auto index = juce::jlimit(0, static_cast<int>(aquorbium::TimbreBody::kindCount) - 1,
                juce::roundToInt(timbreBodyValue[i].getValue() * aquorbium::TimbreBody::kindScale));
            if (timbreBodySelect[i].getSelectedId() != index + 1)
                timbreBodySelect[i].setSelectedId(index + 1, juce::dontSendNotification);
            timbreMatter[i].setEnabled(index > 0);
            timbreMix[i].setEnabled(index > 0);
            chosenBodies += index > 0 ? 1 : 0;
        }
        timbreCard.setStatus(chosenBodies == 0 ? aquorbium::app::T(aquorbium::app::S::native)
            : juce::String(chosenBodies) + (chosenBodies == 1 ? aquorbium::app::T(aquorbium::app::S::bodyOne) : aquorbium::app::T(aquorbium::app::S::bodyMany))
                + (timbreEcology.getValue() > 0.0 ? aquorbium::app::T(aquorbium::app::S::ecological) : juce::String()));
        for (auto* component : { static_cast<juce::Component*>(&musicReaction),
                                 static_cast<juce::Component*>(&musicMemory),
                                 static_cast<juce::Component*>(&musicWater),
                                 static_cast<juce::Component*>(&musicReactionLabel),
                                 static_cast<juce::Component*>(&musicMemoryLabel),
                                 static_cast<juce::Component*>(&musicWaterLabel) })
            component->setEnabled(musicActive);
        const auto moduleBankActive = moduleBankSliders[
            aquorbium::IntegratedModuleBank::mix].getValue() > 0.0;
        std::size_t activeModuleFamilies = 0;
        for (auto* component : { static_cast<juce::Component*>(&strikerMatter),
                                 static_cast<juce::Component*>(&strikerPopulation),
                                 static_cast<juce::Component*>(&strikerScatter),
                                 static_cast<juce::Component*>(&strikerMatterLabel),
                                 static_cast<juce::Component*>(&strikerPopulationLabel),
                                 static_cast<juce::Component*>(&strikerScatterLabel) })
            component->setEnabled(strikerActive);
        for (auto* component : { static_cast<juce::Component*>(&spaceSize),
                                 static_cast<juce::Component*>(&spaceSpread),
                                 static_cast<juce::Component*>(&spaceTone),
                                 static_cast<juce::Component*>(&spaceMemory),
                                 static_cast<juce::Component*>(&spaceSizeLabel),
                                 static_cast<juce::Component*>(&spaceSpreadLabel),
                                 static_cast<juce::Component*>(&spaceToneLabel),
                                 static_cast<juce::Component*>(&spaceMemoryLabel) })
            component->setEnabled(spaceActive);
        for (auto* slider : { &rhythmDensity, &orbitRotation, &intervalBias, &geometryMutation })
            slider->setEnabled(geometryActive);
        for (auto* component : { static_cast<juce::Component*>(&environmentFilterMix),
                                 static_cast<juce::Component*>(&environmentCentre),
                                 static_cast<juce::Component*>(&environmentSpan),
                                 static_cast<juce::Component*>(&environmentResonance),
                                 static_cast<juce::Component*>(&environmentAmbience),
                                 static_cast<juce::Component*>(&environmentFilterMixLabel),
                                 static_cast<juce::Component*>(&environmentCentreLabel),
                                 static_cast<juce::Component*>(&environmentSpanLabel),
                                 static_cast<juce::Component*>(&environmentResonanceLabel),
                                 static_cast<juce::Component*>(&environmentAmbienceLabel) })
            component->setEnabled(environmentActive);
        for (auto* component : { static_cast<juce::Component*>(&tailDecay),
                                 static_cast<juce::Component*>(&tailDamping),
                                 static_cast<juce::Component*>(&tailNightmare),
                                 static_cast<juce::Component*>(&tailDecayLabel),
                                 static_cast<juce::Component*>(&tailDampingLabel),
                                 static_cast<juce::Component*>(&tailNightmareLabel) })
            component->setEnabled(tailActive);
        for (auto* component : { static_cast<juce::Component*>(&formContext),
                                 static_cast<juce::Component*>(&formInertia),
                                 static_cast<juce::Component*>(&formMutation),
                                 static_cast<juce::Component*>(&formContextLabel),
                                 static_cast<juce::Component*>(&formInertiaLabel),
                                 static_cast<juce::Component*>(&formMutationLabel) })
            component->setEnabled(formActive);
        for (auto* component : { static_cast<juce::Component*>(&structureMemory),
                                 static_cast<juce::Component*>(&structureHarmony),
                                 static_cast<juce::Component*>(&structurePhase),
                                 static_cast<juce::Component*>(&structureMemoryLabel),
                                 static_cast<juce::Component*>(&structureHarmonyLabel),
                                 static_cast<juce::Component*>(&structurePhaseLabel) })
            component->setEnabled(structureActive);
        for (std::size_t index = 1; index < moduleBankSliders.size(); ++index)
        {
            moduleBankSliders[index].setEnabled(moduleBankActive);
            moduleBankLabels[index].setEnabled(moduleBankActive);
            activeModuleFamilies += moduleBankSliders[index].getValue() > 0.0 ? 1u : 0u;
        }
        synthesisCard.setStatus(strikerActive ? aquorbium::app::T(aquorbium::app::S::strikerManual) : aquorbium::app::T(aquorbium::app::S::strikerAutonomous));
        spaceCard.setStatus(spaceActive ? aquorbium::app::T(aquorbium::app::S::manual) : aquorbium::app::T(aquorbium::app::S::autonomousState));
        environmentCard.setStatus(environmentActive ? aquorbium::app::T(aquorbium::app::S::active) : aquorbium::app::T(aquorbium::app::S::bypass));
        tailCard.setStatus(tailActive ? aquorbium::app::T(aquorbium::app::S::active) : aquorbium::app::T(aquorbium::app::S::bypass));
        masterCard.setStatus(creativeMaster.getValue() > 0.0 ? aquorbium::app::T(aquorbium::app::S::creativeSafety) : aquorbium::app::T(aquorbium::app::S::safety));
        if (!musicActive)
            geometryCard.setStatus(structureActive ? aquorbium::app::T(aquorbium::app::S::structureActive)
                : formActive ? aquorbium::app::T(aquorbium::app::S::memoryActive) : aquorbium::app::T(aquorbium::app::S::formBypass));
        synthesisCard.setLit(strikerActive);
        spaceCard.setLit(spaceActive);
        environmentCard.setLit(environmentActive);
        tailCard.setLit(tailActive);
        masterCard.setLit(true);
        geometryCard.setLit(musicActive || structureActive || formActive);
        moduleBankCard.setLit(moduleBankActive);
        timbreCard.setLit(chosenBodies > 0);
        moduleBankCard.setStatus(moduleBankActive
            ? aquorbium::app::T(aquorbium::app::S::labActive) + juce::String(activeModuleFamilies) + "/15"
            : aquorbium::app::T(aquorbium::app::S::labBypass));
    }

    void showAudioSettings()
    {
        if (audioSettingsWindow != nullptr)
        {
            audioSettingsWindow->toFront(true);
            audioSettingsWindow->setVisible(true);
            return;
        }
        auto selector = std::make_unique<SafeAudioSettingsComponent>(deviceManager);
        juce::DialogWindow::LaunchOptions options;
        options.content.setOwned(selector.release());
        options.dialogTitle = utf8("Aquorbium — saída e buffer de áudio");
        options.dialogBackgroundColour = Palette::panelRaised;
        options.escapeKeyTriggersCloseButton = true;
        options.useNativeTitleBar = true;
        options.resizable = false;
        // Non-modal: no nested GUI event loop. Owned here, deleted before manager.
        audioSettingsWindow.reset(options.create());
        audioSettingsWindow->centreAroundComponent(this, 520, 265);
        audioSettingsWindow->setVisible(true);
    }

    struct NoteEvent
    {
        bool noteOn = false;
        int note = 0;
        float velocity = 0.0f;
    };

    bool tickRuntimeAudit()
    {
        const auto seconds = (juce::Time::getMillisecondCounterHiRes() - runtimeAuditStarted) / 1000.0;
        using Protocol = aquorbium::app::RuntimeAuditProtocol;
        using Phase = aquorbium::app::AuditPhase;
        const auto phase = Protocol::phase(seconds, runtimeAuditStress);
        const auto step = Protocol::step(seconds, runtimeAuditStress);
        if (phase != Phase::finished)
        {
            if (step != runtimeAuditStep)
            {
                runtimeAuditStep = step;
                const auto value = (step % 2 == 0) ? 0.1 : 0.9;
                const auto set = [](juce::Slider& slider, double target) {
                    slider.setValue(target, juce::sendNotificationSync);
                };
                if (phase == Phase::macros)
                {
                    set(strikerInfluence, 1.0); set(spaceGesture, 1.0);
                    set(environmentInfluence, 1.0);
                    set(tailInfluence, 1.0);
                    set(density, value); set(drift, 1.0 - value);
                    set(strikerMatter, value); set(strikerPopulation, value);
                    set(strikerScatter, 1.0 - value); set(spaceSize, value);
                    set(spaceSpread, value); set(spaceTone, value);
                    set(spaceMemory, value);
                    set(environmentFilterMix, value);
                    set(environmentCentre, value);
                    set(environmentSpan, 1.0 - value);
                    set(environmentResonance, value);
                    set(environmentAmbience, 1.0 - value);
                    set(tailDecay, value);
                    set(tailDamping, 1.0 - value);
                    set(tailNightmare, value);
                    const auto view = step % 4;
                    rightColumnViewport.setViewPosition(0, view == 0 ? 0
                        : view == 1 ? spaceCard.getY()
                        : view == 2 ? environmentCard.getY() : tailCard.getY());
                }
                else if (phase == Phase::geometry)
                {
                    set(geometryInfluence, 1.0); set(rhythmDensity, value);
                    set(orbitRotation, value); set(intervalBias, value);
                    set(geometryMutation, value);
                    showEcologyControls = step % 2 == 0;
                    updateEcologySurface();
                }
                else if (phase == Phase::lens)
                {
                    showEcologyControls = false;
                    updateEcologySurface();
                    lensHeldTarget.store(true, std::memory_order_relaxed);
                    lensTargetX.store(static_cast<float>(value * 2.0 - 1.0), std::memory_order_relaxed);
                    lensTargetY.store(0.3f, std::memory_order_relaxed);
                    lensTargetZ.store(0.8f, std::memory_order_relaxed);
                    submitControlFrame();
                }
                else if (phase == Phase::scenes)
                {
                    const auto second = step / 4;
                    if (second != runtimeAuditSceneSecond)
                    {
                        runtimeAuditSceneSecond = second;
                        aquorbium::app::PerformanceParameterState recalled;
                        if (!presetBank.recall(static_cast<std::size_t>(second % 2), recalled))
                            runtimeAuditSceneFailure = true;
                        else
                        {
                            applyPerformancePreset(recalled);
                            applySceneSynth(recalled);
                            ++runtimeAuditSceneRecalls;
                        }
                    }
                }
                else if (phase == Phase::autonomy)
                {
                    const auto enabled = (step / 4) % 2 == 0;
                    if (autonomy.getToggleState() != enabled)
                        ++runtimeAuditAutonomySwitches;
                    autonomy.setToggleState(enabled, juce::dontSendNotification);
                    autonomyTarget.store(enabled, std::memory_order_relaxed);
                    submitControlFrame();
                }
                else if (phase == Phase::notes)
                {
                    autonomy.setToggleState(true, juce::dontSendNotification);
                    autonomyTarget.store(true, std::memory_order_relaxed);
                    set(density, 0.95); set(drift, 0.9);
                    for (int note = 48; note < 52; ++note)
                    {
                        queueEvent({ false, note, 0.0f });
                        queueEvent({ true, note, 0.6f });
                        runtimeAuditGeneratedEvents += 2;
                    }
                    submitControlFrame();
                }
                else if (phase == Phase::release)
                {
                    lensHeldTarget.store(false, std::memory_order_relaxed);
                    if (seconds >= 58.0 && !runtimeAuditNotesReleased)
                    {
                        for (int note = 48; note < 52; ++note)
                        {
                            queueEvent({ false, note, 0.0f });
                            ++runtimeAuditGeneratedEvents;
                        }
                        runtimeAuditNotesReleased = true;
                    }
                    submitControlFrame();
                }
                if (runtimeAuditRows < runtimeAuditReadings.size())
                {
                    auto& row = runtimeAuditReadings[runtimeAuditRows++];
                    row.seconds = seconds;
                    row.cpu = deviceManager.getCpuUsage() * 100.0;
                    row.xruns = deviceManager.getXRunCount();
                    row.callbacks = audioCallbackCounter.load(std::memory_order_relaxed);
                    row.late = overBudgetCounter.load(std::memory_order_relaxed);
                    row.clip = clippedBlockCounter.load(std::memory_order_relaxed);
                    row.gui = guiPauseCounter;
                }
            }
            return false;
        }

        runtimeAuditActive = false;
        stopTimer();
        const auto finalXruns = deviceManager.getXRunCount();
        const auto dropped = pendingEvents.droppedCount();
        const auto finalTelemetry = visualTelemetry.read();
        shutdownAudio(); // wait for audio ownership before reading accumulators
        runtimeAuditEnabled = false;
        const auto callbacks = audioCallbackCounter.load(std::memory_order_relaxed);
        const auto late = overBudgetCounter.load(std::memory_order_relaxed);
        const auto clip = clippedBlockCounter.load(std::memory_order_relaxed);
        const auto completed = callbacks > 0 && runtimeAuditSamples > 0 && runtimeAuditInvalidSamples == 0
            && !runtimeAuditSceneFailure;
        juce::String report = "Aquorbium runtime diagnostic (not perceptual approval)\n";
        report += "device=" + runtimeAuditDevice + "\nbackend=" + runtimeAuditBackend;
        report += "\nrate=" + juce::String(runtimeAuditRate) + "\nbuffer=" + juce::String(runtimeAuditBuffer);
        report += "\nwall_seconds=" + juce::String(seconds, 3);
        report += "\naudio_seconds=" + juce::String(static_cast<double>(runtimeAuditSamples) / runtimeAuditRate, 3);
        report += "\ncallbacks=" + juce::String(callbacks) + "\nxruns=" + juce::String(finalXruns);
        report += "\nlate=" + juce::String(late) + "\nclipped_blocks=" + juce::String(clip);
        report += "\ngui_pauses_gt_250ms=" + juce::String(guiPauseCounter) + "\ndropped_events=" + juce::String(dropped);
        report += "\ninvalid_sample_pairs=" + juce::String(runtimeAuditInvalidSamples);
        report += "\npeak=" + juce::String(runtimeAuditPeak, 8) + "\nmax_sample_delta=" + juce::String(runtimeAuditDelta, 8);
        report += "\nrms=" + juce::String(runtimeAuditSamples > 0
            ? std::sqrt(runtimeAuditSquared / (2.0 * static_cast<double>(runtimeAuditSamples))) : 0.0, 8);
        report += "\nlufs_momentary=" + juce::String(finalTelemetry.momentaryLufs, 2);
        report += "\nlufs_short_term=" + juce::String(finalTelemetry.shortTermLufs, 2);
        report += "\nlufs_integrated=" + juce::String(finalTelemetry.integratedLufs, 2);
        report += "\nloudness_blocks=" + juce::String(finalTelemetry.loudnessBlocks);
        report += "\ncompleted=" + juce::String(completed ? 1 : 0);
        report += "\nstress=" + juce::String(runtimeAuditStress ? 1 : 0);
        report += "\nscene_recalls=" + juce::String(runtimeAuditSceneRecalls);
        report += "\nautonomy_switches=" + juce::String(runtimeAuditAutonomySwitches);
        report += "\ngenerated_note_events=" + juce::String(runtimeAuditGeneratedEvents);
        report += "\nProtocol: 0-4s baseline; 4-9s macros; 9-14s geometry/pages; 14-18s held lens; 18-20s release.\n";
        if (runtimeAuditStress)
            report += "Stress extension: 20-35s scene recalls; 35-45s autonomy; 45-58s four voices; 58-60s release.\n";
        report += "No physical mouse/MIDI input or perceptual evaluation. Delta is not a click detector.\n";
        report += "seconds\tcpu_percent\txruns\tcallbacks\tlate\tclip\tgui\n";
        for (std::size_t index = 0; index < runtimeAuditRows; ++index)
        {
            const auto& row = runtimeAuditReadings[index];
            report += juce::String(row.seconds, 3) + "\t" + juce::String(row.cpu, 2)
                + "\t" + juce::String(row.xruns) + "\t" + juce::String(row.callbacks)
                + "\t" + juce::String(row.late) + "\t" + juce::String(row.clip) + "\t" + juce::String(row.gui) + "\n";
        }
        const auto written = runtimeAuditDirectory.getChildFile("REPORT.txt").replaceWithText(report);
        juce::JUCEApplication::getInstance()->setApplicationReturnValue(completed && written ? 0 : 1);
        juce::JUCEApplication::getInstance()->quit();
        return true;
    }

    void finishInteractiveCapture()
    {
        if (diagnosticCapture == nullptr || captureFinalized) return;
        diagnosticCapture->stop(); // GUI only, after callback is done or writer has finished
        captureFinalized = true;
        audioSettingsButton.setEnabled(true);
        const auto elapsed = juce::Time::getMillisecondCounterHiRes() - captureStarted;
        const auto capturedFrames = diagnosticCapture->captured();
        juce::String report = "Aquorbium interactive capture (not perceptual approval)\n";
        report += "device=" + captureDevice + "\nbackend=" + captureBackend;
        report += "\nrate=" + juce::String(captureRate) + "\nbuffer=" + juce::String(captureBuffer);
        report += "\nrequested_seconds=" + juce::String(diagnosticCapture->requestedSeconds());
        report += "\nwall_seconds=" + juce::String(elapsed / 1000.0, 3);
        report += "\nwritten_frames=" + juce::String(static_cast<juce::int64>(capturedFrames));
        report += "\naudio_seconds=" + juce::String(static_cast<double>(capturedFrames) / captureRate, 3);
        report += "\ndropped_frames=" + juce::String(static_cast<juce::int64>(diagnosticCapture->dropped()));
        report += "\nwrite_failed=" + juce::String(diagnosticCapture->failed() ? 1 : 0);
        report += "\nwall_deadline_reached="
            + juce::String(diagnosticCapture->stoppedByWallDeadline() ? 1 : 0);
        report += "\nrate_changed=" + juce::String(captureRateChanged.load(std::memory_order_acquire) ? 1 : 0);
        report += "\ncallbacks=" + juce::String(audioCallbackCounter.load(std::memory_order_relaxed));
        report += "\ncallbacks_started="
            + juce::String(audioCallbackStartedCounter.load(std::memory_order_relaxed));
        report += "\nxruns=" + juce::String(captureFinalXruns);
        report += "\nlate=" + juce::String(overBudgetCounter.load(std::memory_order_relaxed));
        report += "\nclipped_blocks=" + juce::String(clippedBlockCounter.load(std::memory_order_relaxed));
        report += "\ngui_pauses_gt_250ms=" + juce::String(guiPauseCounter);
        report += "\nManual controls/MIDI are permitted; no scripted gestures were used.\n";
        report += "WAV records Aquorbium samples before the audio device. A driver glitch may be audible but absent from this file.\n";
        report += "seconds\tcpu_percent\txruns\tcallbacks\tlate\tclip\tgui\tdensity\tdrift\tgeometry_mix\tstriker_mix\tspace_mix\n";
        for (std::size_t index = 0; index < captureRows; ++index)
        {
            const auto& row = captureReadings[index];
            report += juce::String(row.seconds, 3) + "\t" + juce::String(row.cpu, 2)
                + "\t" + juce::String(row.xruns) + "\t" + juce::String(row.callbacks)
                + "\t" + juce::String(row.late) + "\t" + juce::String(row.clip)
                + "\t" + juce::String(row.gui) + "\t" + juce::String(row.density, 2)
                + "\t" + juce::String(row.drift, 2) + "\t" + juce::String(row.geometry, 2)
                + "\t" + juce::String(row.striker, 2) + "\t" + juce::String(row.space, 2) + "\n";
        }
        static_cast<void>(captureDirectory.getChildFile("REPORT.txt").replaceWithText(report));
    }

    // The card reads the water's state, the tension of the web and the latest
    // act, so the performer sees what the aquarium is living
    static juce::String ecologicalMusicStatus(const VisualFrame& frame)
    {
        const auto phase = aquorbium::app::T(aquorbium::app::S::waterStates[static_cast<std::size_t>(juce::jlimit(0, 4, frame.musicPhase))]);
        const auto act = aquorbium::app::T(aquorbium::app::S::acts[static_cast<std::size_t>(juce::jlimit(0, static_cast<int>(aquorbium::EcologicalMusicDirector::actCount) - 1, frame.musicAct))]);
        return aquorbium::app::T(aquorbium::app::S::water) + phase + utf8(" · ") + act + " "
            + juce::String(juce::roundToInt(frame.musicTension * 100.0f)) + "%";
    }

    void timerCallback() override
    {
        const auto guiTick = juce::Time::getHighResolutionTicks();
        if (lastGuiTick != 0 && juce::Time::highResolutionTicksToSeconds(guiTick - lastGuiTick) > 0.250)
            ++guiPauseCounter;
        lastGuiTick = guiTick;
        if (runtimeAuditActive && tickRuntimeAudit())
            return;
        tickRecording();
        if (diagnosticCapture != nullptr && !captureFinalized)
        {
            captureFinalXruns = deviceManager.getXRunCount();
            const auto now = juce::Time::getMillisecondCounterHiRes();
            if (now >= captureNextReadingMs && captureRows < captureReadings.size())
            {
                captureNextReadingMs = now + 500.0;
                auto& row = captureReadings[captureRows++];
                row.seconds = (now - captureStarted) / 1000.0;
                row.cpu = deviceManager.getCpuUsage() * 100.0;
                row.xruns = captureFinalXruns;
                row.callbacks = audioCallbackCounter.load(std::memory_order_relaxed);
                row.late = overBudgetCounter.load(std::memory_order_relaxed);
                row.clip = clippedBlockCounter.load(std::memory_order_relaxed);
                row.gui = guiPauseCounter;
                row.density = density.getValue();
                row.drift = drift.getValue();
                row.geometry = geometryInfluence.getValue();
                row.striker = strikerInfluence.getValue();
                row.space = spaceGesture.getValue();
            }
            if (diagnosticCapture->done()) finishInteractiveCapture();
        }
        auto frame = visualTelemetry.read();
        frame.droppedEvents = pendingEvents.droppedCount();
        tickMotion(frame);
        ecosystemView.setFrame(frame);
        if (meterPanel.isShowing())
            meterPanel.setFrame(frame);
        if (brainStatus.isShowing() && statusTimerTicks % 5 == 0)
        {
            juce::String bits;
            for (int b = 7; b >= 0; --b)
                bits << (((frame.brainRegister >> b) & 1) != 0 ? "1" : "0");
            brainStatus.setText(bits + utf8("  ·  humor ") + juce::String(frame.brainMood, 2), juce::dontSendNotification);
            roomStatus.setText(!frame.roomEnabled ? utf8("SALA desligada")
                               : juce::String(frame.roomLevelDb, 0) + utf8(" dB  ·  força ") + juce::String(frame.roomStrength, 2)
                                     + utf8("  ·  acoplamento ") + juce::String(frame.roomCoupling, 2),
                               juce::dontSendNotification);
        }
        if (musicInfluence.getValue() > 0.0)
            geometryCard.setStatus(ecologicalMusicStatus(frame));
        keepChosenSoundmark(frame);
        if (++statusTimerTicks >= 15)
        {
            statusTimerTicks = 0;
            if (auto* device = deviceManager.getCurrentAudioDevice())
            {
                audioStatus = juce::String(device->getCurrentSampleRate() / 1000.0, 1)
                    + " kHz / " + juce::String(device->getCurrentBufferSizeSamples())
                    + "  CPU " + juce::String(deviceManager.getCpuUsage() * 100.0, 0)
                    + "%  XRUN " + juce::String(deviceManager.getXRunCount())
                    + "  LATE " + juce::String(overBudgetCounter.load(std::memory_order_relaxed))
                    + (realtimePriority.load(std::memory_order_relaxed) > 0
                           ? "  RT " + juce::String(realtimePriority.load(std::memory_order_relaxed))
                           : (realtimeAsked.load(std::memory_order_relaxed) ? utf8("  RT não") : juce::String()))
                    + "  CLIP " + juce::String(clippedBlockCounter.load(std::memory_order_relaxed))
                    + "  GUI " + juce::String(guiPauseCounter)
                    + "  SEED " + juce::String(static_cast<juce::int64>(sessionSeed))
                    + (sessionGenesis ? "  " + sessionWorldText() : juce::String());
                const auto setup = deviceManager.getAudioDeviceSetup();
                if (needsPipeWireSafetyBuffer(setup.outputDeviceName, setup.bufferSize))
                    audioStatus = aquorbium::app::T(aquorbium::app::S::unsafeBuffer) + audioStatus;
                const auto momentaryText = frame.momentaryLoudnessReady
                    ? juce::String(frame.momentaryLufs, 1) : "--";
                const auto shortTermText = frame.shortTermLoudnessReady
                    ? juce::String(frame.shortTermLufs, 1) : "--";
                audioSettingsButton.setTooltip(device->getName()
                    + "\n" + audioStatus
                    + "\nLUFS M/S/I: " + momentaryText + " / "
                    + shortTermText + " / " + juce::String(frame.integratedLufs, 1)
                    + "\nTRUE PEAK " + juce::String(20.0f * std::log10(std::max(frame.outputTruePeak, 1.0e-6f)), 1)
                    + " dBTP  GR " + juce::String(frame.gainReductionDb, 1)
                    + " dB  CORR " + juce::String(frame.correlation, 2)
                    + "\ncallbacks started/completed: "
                    + juce::String(audioCallbackStartedCounter.load(std::memory_order_relaxed))
                    + "/" + juce::String(audioCallbackCounter.load(std::memory_order_relaxed))
                    + utf8("\nSelecionar saída e buffer; diagnóstico inicial: 48 kHz / 512 amostras. LATE = callback acima do orçamento; CLIP = blocos acima de 0 dBFS; GUI = intervalos de interface acima de 250 ms (inclui suspensão do sistema)."));
            }
            else
                audioStatus = utf8("SEM DISPOSITIVO — abra ÁUDIO");
            repaint();
        }

        const auto callbackCount = audioCallbackCounter.load(std::memory_order_relaxed);
        if (callbackCount != previousAudioCallbackCount)
        {
            previousAudioCallbackCount = callbackCount;
            missedAudioTimerTicks = 0;
            if (!audioEngineOnline)
            {
                audioEngineOnline = true;
                repaint();
            }
        }
        else if (++missedAudioTimerTicks >= 15 && audioEngineOnline)
        {
            audioEngineOnline = false;
            repaint();
        }
    }

    void handleIncomingMidiMessage(juce::MidiInput*, const juce::MidiMessage& message) override
    {
        if (message.isNoteOn())
            queueEvent({ true, message.getNoteNumber(), message.getFloatVelocity() });
        else if (message.isNoteOff())
            queueEvent({ false, message.getNoteNumber(), 0.0f });
    }

    void queueEvent(NoteEvent event) noexcept
    {
        static_cast<void>(pendingEvents.tryPush(event));
    }

    void consumeEvents() noexcept
    {
        // Bound work per callback even if a controller floods the input.
        constexpr int maximumEventsPerBlock = 64;
        NoteEvent event;
        for (int count = 0; count < maximumEventsPerBlock && pendingEvents.tryPop(event); ++count)
        {
            if (event.noteOn)
                engine.noteOn(event.note, event.velocity);
            else
                engine.noteOff(event.note);
        }
    }

    void submitControlFrame() noexcept
    {
        updateControlAvailability();
        // GUI producer only. Preset restore calls this once after all controls.
        aquorbium::app::AudioControlFrame frame;
        frame.density = densityTarget.load(std::memory_order_relaxed);
        frame.drift = driftTarget.load(std::memory_order_relaxed);
        frame.cohesion = cohesionTarget.load(std::memory_order_relaxed);
        frame.danger = dangerTarget.load(std::memory_order_relaxed);
        frame.ecosystemMorph = ecosystemMorphTarget.load(std::memory_order_relaxed);
        frame.sparseFocus = sparseFocusTarget.load(std::memory_order_relaxed);
        frame.energyMemory = energyMemoryTarget.load(std::memory_order_relaxed);
        frame.geometricCycle = geometricCycleTarget.load(std::memory_order_relaxed);
        frame.collectiveDrift = collectiveDriftTarget.load(std::memory_order_relaxed);
        frame.geometryInfluence = geometryInfluenceTarget.load(std::memory_order_relaxed);
        frame.rhythmDensity = rhythmDensityTarget.load(std::memory_order_relaxed);
        frame.orbitRotation = orbitRotationTarget.load(std::memory_order_relaxed);
        frame.intervalBias = intervalBiasTarget.load(std::memory_order_relaxed);
        frame.geometryMutation = geometryMutationTarget.load(std::memory_order_relaxed);
        frame.strikerInfluence = strikerInfluenceTarget.load(std::memory_order_relaxed);
        frame.strikerMatter = strikerMatterTarget.load(std::memory_order_relaxed);
        frame.strikerPopulation = strikerPopulationTarget.load(std::memory_order_relaxed);
        frame.strikerScatter = strikerScatterTarget.load(std::memory_order_relaxed);
        frame.spaceReturn = spaceReturnTarget.load(std::memory_order_relaxed);
        frame.spaceGesture = spaceGestureTarget.load(std::memory_order_relaxed);
        frame.spaceSize = spaceSizeTarget.load(std::memory_order_relaxed);
        frame.spaceSpread = spaceSpreadTarget.load(std::memory_order_relaxed);
        frame.spaceTone = spaceToneTarget.load(std::memory_order_relaxed);
        frame.spaceMemory = spaceMemoryTarget.load(std::memory_order_relaxed);
        frame.environmentInfluence = environmentInfluenceTarget.load(std::memory_order_relaxed);
        frame.environmentFilterMix = environmentFilterMixTarget.load(std::memory_order_relaxed);
        frame.environmentCentre = environmentCentreTarget.load(std::memory_order_relaxed);
        frame.environmentSpan = environmentSpanTarget.load(std::memory_order_relaxed);
        frame.environmentResonance = environmentResonanceTarget.load(std::memory_order_relaxed);
        frame.environmentAmbience = environmentAmbienceTarget.load(std::memory_order_relaxed);
        frame.tailInfluence = tailInfluenceTarget.load(std::memory_order_relaxed);
        frame.tailDecay = tailDecayTarget.load(std::memory_order_relaxed);
        frame.tailDamping = tailDampingTarget.load(std::memory_order_relaxed);
        frame.tailNightmare = tailNightmareTarget.load(std::memory_order_relaxed);
        frame.formInfluence = formInfluenceTarget.load(std::memory_order_relaxed);
        frame.formContext = formContextTarget.load(std::memory_order_relaxed);
        frame.formInertia = formInertiaTarget.load(std::memory_order_relaxed);
        frame.formMutation = formMutationTarget.load(std::memory_order_relaxed);
        frame.masterTrim = masterTrimTarget.load(std::memory_order_relaxed);
        frame.masterCeiling = masterCeilingTarget.load(std::memory_order_relaxed);
        frame.masterRelease = masterReleaseTarget.load(std::memory_order_relaxed);
        frame.creativeMaster = creativeMasterTarget.load(std::memory_order_relaxed);
        frame.masterEqAfterDynamics = masterEqOrderTarget.load(std::memory_order_relaxed) >= 0.5f;
        for (std::size_t i = 0; i < frame.cards.size(); ++i)
            frame.cards[i] = cardTargets[i].load(std::memory_order_relaxed);
        frame.structureInfluence = structureInfluenceTarget.load(std::memory_order_relaxed);
        frame.structureMemory = structureMemoryTarget.load(std::memory_order_relaxed);
        frame.structureHarmony = structureHarmonyTarget.load(std::memory_order_relaxed);
        frame.structurePhase = structurePhaseTarget.load(std::memory_order_relaxed);
        frame.musicInfluence = musicInfluenceTarget.load(std::memory_order_relaxed);
        frame.musicReaction = musicReactionTarget.load(std::memory_order_relaxed);
        frame.musicMemory = musicMemoryTarget.load(std::memory_order_relaxed);
        frame.musicWater = musicWaterTarget.load(std::memory_order_relaxed);
        frame.musicSalinity = musicSalinityTarget.load(std::memory_order_relaxed);
        for (std::size_t i = 0; i < 5; ++i)
        {
            frame.timbreBody[i] = timbreBodyTarget[i].load(std::memory_order_relaxed);
            frame.timbreMatter[i] = timbreMatterTarget[i].load(std::memory_order_relaxed);
            frame.timbreMix[i] = timbreMixTarget[i].load(std::memory_order_relaxed);
        }
        frame.timbreEcology = timbreEcologyTarget.load(std::memory_order_relaxed);
        frame.supportPresence = supportPresenceTarget.load(std::memory_order_relaxed);
        for (std::size_t index = 0; index < frame.moduleBank.size(); ++index)
            frame.moduleBank[index] = moduleBankTargets[index].load(std::memory_order_relaxed);
        frame.lensX = lensTargetX.load(std::memory_order_relaxed);
        frame.lensY = lensTargetY.load(std::memory_order_relaxed);
        frame.lensZ = lensTargetZ.load(std::memory_order_relaxed);
        frame.lensHeld = lensHeldTarget.load(std::memory_order_relaxed);
        frame.autonomous = autonomyTarget.load(std::memory_order_relaxed);
        controlFrames.publish(frame);
    }

    void publishControlsToAudioEngine() noexcept
    {
        if (controlFrames.consumeLatest(audioControls))
            audioControls.apply(engine);
        if (synthFrames.consumeLatest(audioSynth))
            applySynthToEngine();
    }

    // ---- the synthesis device: GUI -> audio (lock-free), settings ----------
    struct SynthFrame
    {
        std::array<aquorbium::SynthVoice::Patch, SynthEditor::organisms> patches {};
        float mix = 0.0f;
    };

    void applySynthToEngine() noexcept
    {
        engine.setSynthMix(audioSynth.mix);
        for (std::size_t i = 0; i < SynthEditor::organisms; ++i)
            engine.setSynthPatch(i, audioSynth.patches[i]);
    }

    void synthChanged()
    {
        SynthFrame frame;
        frame.patches = synthEditor.patches;
        frame.mix = synthEditor.mix;
        synthFrames.publish(frame);
        settings.setValue("synth.mix", synthEditor.mix);
        for (std::size_t i = 0; i < SynthEditor::organisms; ++i)
            settings.setValue("synth.patch." + juce::String(static_cast<int>(i)),
                              juce::String(aquorbium::SynthPatchCodec::encode(synthEditor.patches[i])));
    }

    void loadSynth()
    {
        synthEditor.mix = static_cast<float>(juce::jlimit(0.0, 1.0, settings.getDoubleValue("synth.mix", 0.0)));
        for (std::size_t i = 0; i < SynthEditor::organisms; ++i)
        {
            const auto text = settings.getValue("synth.patch." + juce::String(static_cast<int>(i)));
            synthEditor.patches[i] = aquorbium::SynthPatchCodec::decode(text.toStdString());
        }
        synthMixCard.setValue(synthEditor.mix, juce::dontSendNotification);
        synthEditor.loadControls();
        SynthFrame frame;
        frame.patches = synthEditor.patches;
        frame.mix = synthEditor.mix;
        synthFrames.publish(frame);
    }

    [[nodiscard]] bool applyEcologyState(const juce::ValueTree& snapshot)
    {
        if (!ecologyState.restore(snapshot))
            return false;
        for (const auto& binding : ecologyBindings())
        {
            const auto value = ecologyState.get(binding.key);
            binding.slider->setValue(binding.inverted ? 1.0 - value : value,
                                     juce::dontSendNotification);
            const auto displayed = binding.slider->getValue();
            static_cast<void>(ecologyState.set(binding.key,
                binding.inverted ? 1.0 - displayed : displayed));
            binding.target->store(static_cast<float>(displayed), std::memory_order_relaxed);
        }
        return true;
    }
    struct EcologyBinding
    {
        aquorbium::app::EcologyParameterState::Key key;
        juce::Slider* slider;
        std::atomic<float>* target;
        bool inverted = false;
    };

    [[nodiscard]] std::array<EcologyBinding, 8> ecologyBindings()
    {
        using Key = aquorbium::app::EcologyParameterState::Key;
        return {{
            { Key::density, &density, &densityTarget },
            { Key::calm, &drift, &driftTarget, true },
            { Key::cohesion, &cohesion, &cohesionTarget },
            { Key::danger, &danger, &dangerTarget },
            { Key::sparseFocus, &sparseFocus, &sparseFocusTarget },
            { Key::energyMemory, &energyMemory, &energyMemoryTarget },
            { Key::geometricCycle, &geometricCycle, &geometricCycleTarget },
            { Key::collectiveDrift, &collectiveDrift, &collectiveDriftTarget }
        }};
    }

    struct PerformanceBinding
    {
        const char* id;
        juce::Slider* slider;
        std::atomic<float>* target;
    };

    [[nodiscard]] std::array<PerformanceBinding,
                             aquorbium::app::PerformanceParameterState::parameters.size()>
    performanceBindings()
    {
        std::array<PerformanceBinding,
                   aquorbium::app::PerformanceParameterState::parameters.size()> bindings {{
            { "ecosystemMorph", &ecosystemMorph, &ecosystemMorphTarget },
            { "geometryInfluence", &geometryInfluence, &geometryInfluenceTarget },
            { "rhythmDensity", &rhythmDensity, &rhythmDensityTarget },
            { "orbitRotation", &orbitRotation, &orbitRotationTarget },
            { "intervalBias", &intervalBias, &intervalBiasTarget },
            { "geometryMutation", &geometryMutation, &geometryMutationTarget },
            { "strikerInfluence", &strikerInfluence, &strikerInfluenceTarget },
            { "strikerMatter", &strikerMatter, &strikerMatterTarget },
            { "strikerPopulation", &strikerPopulation, &strikerPopulationTarget },
            { "strikerScatter", &strikerScatter, &strikerScatterTarget },
            { "spaceReturn", &spaceReturn, &spaceReturnTarget },
            { "spaceGesture", &spaceGesture, &spaceGestureTarget },
            { "spaceSize", &spaceSize, &spaceSizeTarget },
            { "spaceSpread", &spaceSpread, &spaceSpreadTarget },
            { "spaceTone", &spaceTone, &spaceToneTarget },
            { "spaceMemory", &spaceMemory, &spaceMemoryTarget },
            { "environmentInfluence", &environmentInfluence, &environmentInfluenceTarget },
            { "environmentFilterMix", &environmentFilterMix, &environmentFilterMixTarget },
            { "environmentCentre", &environmentCentre, &environmentCentreTarget },
            { "environmentSpan", &environmentSpan, &environmentSpanTarget },
            { "environmentResonance", &environmentResonance, &environmentResonanceTarget },
            { "environmentAmbience", &environmentAmbience, &environmentAmbienceTarget },
            { "tailInfluence", &tailInfluence, &tailInfluenceTarget },
            { "tailDecay", &tailDecay, &tailDecayTarget },
            { "tailDamping", &tailDamping, &tailDampingTarget },
            { "tailNightmare", &tailNightmare, &tailNightmareTarget },
            { "formInfluence", &formInfluence, &formInfluenceTarget },
            { "formContext", &formContext, &formContextTarget },
            { "formInertia", &formInertia, &formInertiaTarget },
            { "formMutation", &formMutation, &formMutationTarget },
            { "masterTrim", &masterTrim, &masterTrimTarget },
            { "masterCeiling", &masterCeiling, &masterCeilingTarget },
            { "masterRelease", &masterRelease, &masterReleaseTarget },
            { "structureInfluence", &structureInfluence, &structureInfluenceTarget },
            { "structureMemory", &structureMemory, &structureMemoryTarget },
            { "structureHarmony", &structureHarmony, &structureHarmonyTarget },
            { "structurePhase", &structurePhase, &structurePhaseTarget }
        }};
        constexpr std::size_t firstModule =
            aquorbium::app::PerformanceParameterState::structureMemoryParameterCount;
        for (std::size_t index = 0; index < moduleBankDescriptors.size(); ++index)
            bindings[firstModule + index] = {
                moduleBankDescriptors[index].id,
                &moduleBankSliders[index],
                &moduleBankTargets[index]
            };
        bindings[aquorbium::app::PerformanceParameterState::preCreativeMasterParameterCount] = {
            "creativeMaster", &creativeMaster, &creativeMasterTarget
        };
        constexpr std::size_t firstMusic =
            aquorbium::app::PerformanceParameterState::preEcologicalMusicParameterCount;
        bindings[firstMusic] = { "musicInfluence", &musicInfluence, &musicInfluenceTarget };
        bindings[firstMusic + 1] = { "musicReaction", &musicReaction, &musicReactionTarget };
        bindings[firstMusic + 2] = { "musicMemory", &musicMemory, &musicMemoryTarget };
        bindings[firstMusic + 3] = { "musicWater", &musicWater, &musicWaterTarget };
        constexpr std::size_t firstTimbre =
            aquorbium::app::PerformanceParameterState::preTimbreParameterCount;
        constexpr std::size_t firstKind =
            aquorbium::app::PerformanceParameterState::preTimbreKindParameterCount;
        for (std::size_t i = 0; i < 5; ++i)
        {
            bindings[firstTimbre + i] = { aquorbium::app::PerformanceParameterState::parameters[firstTimbre + i].id,
                                          &timbreLegacyBody[i], &timbreLegacyTarget[i] };
            bindings[firstKind + i] = { aquorbium::app::PerformanceParameterState::parameters[firstKind + i].id,
                                        &timbreBodyValue[i], &timbreBodyTarget[i] };
            bindings[firstTimbre + 5 + i] = { aquorbium::app::PerformanceParameterState::parameters[firstTimbre + 5 + i].id,
                                              &timbreMatter[i], &timbreMatterTarget[i] };
            bindings[firstTimbre + 10 + i] = { aquorbium::app::PerformanceParameterState::parameters[firstTimbre + 10 + i].id,
                                               &timbreMix[i], &timbreMixTarget[i] };
        }
        bindings[firstTimbre + 15] = { "timbreEcology", &timbreEcology, &timbreEcologyTarget };
        bindings[aquorbium::app::PerformanceParameterState::preSalinityParameterCount] = {
            "musicSalinity", &musicSalinity, &musicSalinityTarget };
        bindings[aquorbium::app::PerformanceParameterState::preSupportParameterCount] = {
            "supportPresence", &supportPresence, &supportPresenceTarget };
        bindings[aquorbium::app::PerformanceParameterState::preEqOrderParameterCount] = {
            "masterEqOrder", &masterEqOrder, &masterEqOrderTarget };
        for (std::size_t i = 0; i < cardSliders.size(); ++i)
            bindings[aquorbium::app::PerformanceParameterState::preCardsParameterCount + i] = {
                cardIds[i], &cardSliders[i], &cardTargets[i] };
        return bindings;
    }

    // ---- the musician's soundmark, kept in the settings ("music.soundmark":
    // "degrees|steps", comma-separated) ------------------------------------------
    void loadKeptSoundmark()
    {
        const auto text = settings.getValue("music.soundmark");
        const auto degrees = juce::StringArray::fromTokens(text.upToFirstOccurrenceOf("|", false, false), ",", "");
        const auto steps = juce::StringArray::fromTokens(text.fromFirstOccurrenceOf("|", false, false), ",", "");
        aquorbium::SongForm::Phrase motif {};
        const auto n = std::min({ degrees.size(), steps.size(), static_cast<int>(aquorbium::SongForm::capacity) });
        for (int u = 0; u < n; ++u)
        {
            motif.degrees[static_cast<std::size_t>(u)] = static_cast<std::int8_t>(juce::jlimit(-5, 7, degrees[u].getIntValue()));
            motif.steps[static_cast<std::size_t>(u)] = static_cast<std::uint8_t>(juce::jlimit(1, 16, steps[u].getIntValue()));
        }
        motif.length = static_cast<std::uint8_t>(n >= 2 ? n : 0);
        const std::lock_guard<std::mutex> lock(soundmarkKeptLock);
        soundmarkKept = motif;
    }

    void keepChosenSoundmark(const VisualFrame& frame)
    {
        if (!frame.soundmarkChosen || frame.soundmarkLength < 2)
            return;
        aquorbium::SongForm::Phrase motif {};
        motif.length = static_cast<std::uint8_t>(std::min(frame.soundmarkLength, 8));
        for (std::size_t u = 0; u < motif.length; ++u)
        {
            motif.degrees[u] = frame.soundmark[u];
            motif.steps[u] = frame.soundmarkSteps[u];
        }
        {
            const std::lock_guard<std::mutex> lock(soundmarkKeptLock);
            if (motif.length == soundmarkKept.length && motif.degrees == soundmarkKept.degrees
                && motif.steps == soundmarkKept.steps)
                return;
            soundmarkKept = motif;
        }
        juce::StringArray degrees, steps;
        for (std::size_t u = 0; u < motif.length; ++u)
        {
            degrees.add(juce::String(static_cast<int>(motif.degrees[u])));
            steps.add(juce::String(static_cast<int>(motif.steps[u])));
        }
        settings.setValue("music.soundmark", degrees.joinIntoString(",") + "|" + steps.joinIntoString(","));
    }

    // ---- RoomCoupling: open or close the input (the device is reopened) ------
    void openRoomInput(bool on)
    {
        roomRequested.store(false, std::memory_order_relaxed);
        const auto state = deviceManager.createStateXml();
        setAudioChannels(on ? 1 : 0, 2, state.get());
        roomRequested.store(on, std::memory_order_relaxed);
    }

    // ---- REC ------------------------------------------------------------------
    // The audio thread only copies into the capture's bounded ring; the file
    // is written by its own thread. Stopping first detaches the audio pointer,
    // and the object is destroyed a few GUI ticks later, after the writer has
    // finished - never while a callback may still hold it.
    void startRecording()
    {
        if (recorder != nullptr || preparedRate < 8000.0)
            return;
        const auto root = juce::File(settings.getValue("recording.directory",
            juce::File::getSpecialLocation(juce::File::userMusicDirectory).getChildFile("Aquorbium").getFullPathName()));
        const auto now = juce::Time::getCurrentTime();
        const auto day = root.getChildFile(now.formatted("%Y-%m-%d"));
        if (day.createDirectory().failed() || day.getBytesFreeOnVolume() < (2ll << 30))
        {
            recordingNotice(aquorbium::app::T(aquorbium::app::S::recFailed));
            return;
        }
        const auto file = day.getNonexistentChildFile(
            "aquorbium_" + now.formatted("%H-%M-%S") + "_seed" + juce::String(static_cast<juce::int64>(sessionSeed)),
            ".wav", false);
        auto rec = std::make_unique<aquorbium::app::DiagnosticAudioCapture>(
            file.getFullPathName().toStdString(), static_cast<unsigned>(std::lround(preparedRate)),
            aquorbium::app::DiagnosticAudioCapture::recordingSeconds,
            aquorbium::app::DiagnosticAudioCapture::recordingSeconds);
        // measured like Navalha's masters, tagged like Antitotem's takes; the
        // artist is whoever plays ("recording.artist" in the settings), never
        // assumed
        rec->enableAnalysis();
        // "recording.format" = "pcm24" or "pcm16" writes PCM with TPDF dither
        // at that depth; the default is the 32-bit float master
        // "recording.tap": "post-safety" (the default, what is heard),
        // "pre-safety" (after the creative MASTER, before the safety limiter)
        // or "pre-master-criativo" (the mix before the creative MASTER)
        const auto tapName = settings.getValue("recording.tap", "post-safety");
        const auto tapChoice = tapName == "pre-safety" ? aquorbium::BiomaEngine::RecordTap::preSafety
            : tapName == "pre-master-criativo" ? aquorbium::BiomaEngine::RecordTap::preMasterCreative
                                                : aquorbium::BiomaEngine::RecordTap::postSafety;
        recordingTapName = tapChoice == aquorbium::BiomaEngine::RecordTap::postSafety ? juce::String("post-safety") : tapName;
        if (settings.getValue("recording.format") == "pcm24")
            rec->usePcm24();
        else if (settings.getValue("recording.format") == "pcm16")
            rec->usePcm16();
        rec->addInfo("INAM", ("Aquorbium " + now.formatted("%Y-%m-%d %H:%M:%S")).toStdString());
        rec->addInfo("ICRD", now.formatted("%Y-%m-%d").toStdString());
        rec->addInfo("ISFT", juce::String("Aquorbium " JUCE_APPLICATION_VERSION_STRING).toStdString());
        rec->addInfo("ICMT", ("session seed " + juce::String(static_cast<juce::int64>(sessionSeed))
                              + (rec->bitsPerSample() == 32 ? juce::String("; 32-bit float")
                                                             : "; " + juce::String(rec->bitsPerSample()) + "-bit PCM, TPDF dither")
                              + "; tap " + recordingTapName).toStdString());
        if (const auto artist = settings.getValue("recording.artist"); artist.isNotEmpty())
            rec->addInfo("IART", artist.toStdString());
        if (!rec->start())
        {
            recordingNotice(aquorbium::app::T(aquorbium::app::S::recFailed));
            return;
        }
        // "recording.stems" = "true": five more WAVs beside the take, whose
        // sum is the pre-master mix (organisms, lab, water, space, tail)
        stemFiles.clear();
        if (settings.getBoolValue("recording.stems", false))
        {
            static constexpr std::array<const char*, aquorbium::BiomaEngine::stemCount> stemNames {
                "organisms", "lab", "water", "space", "tail" };
            for (std::size_t s = 0; s < stemNames.size(); ++s)
            {
                const auto stemFile = file.getSiblingFile(file.getFileNameWithoutExtension()
                                                          + "_stem-" + stemNames[s] + ".wav");
                auto stem = std::make_unique<aquorbium::app::DiagnosticAudioCapture>(
                    stemFile.getFullPathName().toStdString(), static_cast<unsigned>(std::lround(preparedRate)),
                    aquorbium::app::DiagnosticAudioCapture::recordingSeconds,
                    aquorbium::app::DiagnosticAudioCapture::recordingSeconds);
                stem->addInfo("INAM", ("Aquorbium stem " + juce::String(stemNames[s])).toStdString());
                stem->addInfo("ICMT", "a stem of " + file.getFileName().toStdString()
                                          + "; the five stems sum to the mix before the creative MASTER");
                if (stem->start())
                {
                    stemFiles.add(stemFile.getFileName());
                    stemRecorders[s] = std::move(stem);
                    stemRecordersForAudio[s].store(stemRecorders[s].get(), std::memory_order_release);
                }
            }
            stemsForAudio.store(true, std::memory_order_release);
        }
        // "recording.ambisonics" = "true": a first-order Ambisonics file
        // beside the take (AmbiX: ACN W Y Z X, SN3D, float), the organisms
        // from their directions to the listener, of the mix before the
        // creative MASTER - for any Ambisonics decoder, speakers or binaural
        ambisonicFile.clear();
        if (settings.getBoolValue("recording.ambisonics", false))
        {
            const auto foaFile = file.getSiblingFile(file.getFileNameWithoutExtension() + "_ambix.wav");
            auto foa = std::make_unique<aquorbium::app::AmbisonicCapture>(
                foaFile.getFullPathName().toStdString(), static_cast<unsigned>(std::lround(preparedRate)),
                aquorbium::app::DiagnosticAudioCapture::recordingSeconds);
            if (foa->start())
            {
                ambisonicFile = foaFile.getFileName();
                ambisonicRecorder = std::move(foa);
                ambisonicRecorderForAudio.store(ambisonicRecorder.get(), std::memory_order_release);
            }
        }
        recordingFile = file;
        recordingStart = now;
        recordingControls = capturePerformanceState().snapshot().toXmlString();
        recordingBits = rec->bitsPerSample();
        recordingStopTicks = 0;
        recorder = std::move(rec);
        recorderTap.store(static_cast<int>(tapChoice), std::memory_order_relaxed);
        recorderForAudio.store(recorder.get(), std::memory_order_release);
        recButton.setColour(juce::TextButton::buttonColourId, Palette::coral.withAlpha(0.55f));
    }

    void stopRecording()
    {
        if (recorder == nullptr || recordingStopTicks > 0)
            return;
        recorderForAudio.store(nullptr, std::memory_order_release);
        stemsForAudio.store(false, std::memory_order_release);
        for (auto& pointer : stemRecordersForAudio)
            pointer.store(nullptr, std::memory_order_release);
        for (auto& stem : stemRecorders)
            if (stem != nullptr)
                stem->requestStop();
        ambisonicRecorderForAudio.store(nullptr, std::memory_order_release);
        recorder->requestStop();
        recordingStopTicks = 1;
    }

    void tickRecording()
    {
        if (recorder == nullptr)
            return;
        if (recordingStopTicks == 0)
        {
            if (recorder->done())
                stopRecording();  // the hour is over, the disk failed, or the rate changed
            else
            {
                const auto seconds = static_cast<int>(recorder->elapsed() / std::max(1u, recorder->rate()));
                recButton.setButtonText(utf8("● ") + juce::String(seconds / 60).paddedLeft('0', 2) + ":"
                                        + juce::String(seconds % 60).paddedLeft('0', 2));
            }
            return;
        }
        // a few ticks after detaching (about 100 ms: longer than any audio
        // block), once the writer has closed the file
        if (++recordingStopTicks >= 4 && recorder->done())
            finishRecording();
    }

    void finishRecording()
    {
        recorder->stop();
        auto ambisonicFailed = false;
        if (ambisonicRecorder != nullptr)
        {
            ambisonicRecorder->stop();
            ambisonicFailed = ambisonicRecorder->failed();
            ambisonicRecorder.reset();
        }
        for (auto& stem : stemRecorders)
            if (stem != nullptr)
            {
                stem->stop();
                stem.reset();
            }
        const auto frames = recorder->captured();
        const auto rate = std::max(1u, recorder->rate());
        const auto failed = recorder->failed();
        const auto dropped = recorder->dropped();
        const auto measured = recorder->analysis();
        const auto encoded = recorder->encodedAnalysis();
        recorder.reset();
        recButton.setButtonText("REC");
        recButton.removeColour(juce::TextButton::buttonColourId);
        // provenance beside the sound: what made it, and how to replay it
        juce::String text;
        text << "Aquorbium " << JUCE_APPLICATION_VERSION_STRING << "\n"
             << "file=" << recordingFile.getFileName() << "\n"
             << "started=" << recordingStart.toISO8601(true) << "\n"
             << "seconds=" << juce::String(static_cast<double>(frames) / static_cast<double>(rate), 2) << "\n"
             << "sample_rate=" << static_cast<int>(rate) << "\n"
             << "format=" << (recordingBits == 32 ? juce::String("WAV IEEE float 32-bit stereo")
                                                  : "WAV PCM " + juce::String(recordingBits) + "-bit stereo, TPDF dither")
             << "\n"
             << "tap=" << recordingTapName
             << (recordingTapName == "post-safety" ? "  (what is heard: after the creative MASTER, the safety limiter and MUTE)"
                 : recordingTapName == "pre-safety" ? "  (after the creative MASTER, before the safety limiter; MUTE not applied)"
                                                    : "  (the mix before the creative MASTER; MUTE not applied)")
             << "\n"
             << "dropped_frames=" << static_cast<juce::int64>(dropped) << "\n"
             << "stems=" << (stemFiles.isEmpty() ? juce::String("none")
                                                 : stemFiles.joinIntoString(", ")
                                                       + "  (their sum is the mix before the creative MASTER)") << "\n"
             << "ambisonics=" << (ambisonicFile.isEmpty() ? juce::String("none")
                                                          : ambisonicFile + "  (AmbiX first order: ACN W Y Z X, SN3D, float;"
                                                                " the mix before the creative MASTER"
                                                                + juce::String(ambisonicFailed ? "; WRITE FAILED" : "") + ")")
             << "\n"
             << "write_failed=" << (failed ? 1 : 0) << "\n"
             << "session_seed=" << static_cast<juce::int64>(sessionSeed)
             << "  (to replay the session: session.fixedSeed in the settings)\n"
             << "session_world=" << (sessionGenesis ? sessionWorldText() : juce::String("classic"))
             << "  (session.genesis; the world drawn from the seed)\n"
             << "\n[measurement: BS.1770 loudness and 4x true peak, on the whole take]\n"
             << "integrated_lufs=" << juce::String(measured.integratedLufs, 1) << "\n"
             << "max_short_term_lufs=" << juce::String(measured.maxShortTermLufs, 1) << "\n"
             << "loudness_range_lu=" << (measured.loudnessRangeReady ? juce::String(measured.loudnessRange, 1)
                                                                     : juce::String("n/a")) << "\n"
             << "true_peak_dbtp=" << juce::String(measured.truePeakDb, 2) << "\n"
             << "sample_peak_dbfs=" << juce::String(measured.samplePeakDb, 2) << "\n"
             << "rms_dbfs=" << juce::String(measured.rmsDb, 1) << "\n"
             << "dc_offset=" << juce::String(measured.dcLeft, 6) << " " << juce::String(measured.dcRight, 6) << "\n"
             << "samples_at_or_over_full_scale=" << static_cast<juce::int64>(measured.overFullScale) << "\n"
             << "non_finite_samples_written_as_silence=" << static_cast<juce::int64>(measured.nonFinite) << "\n"
             << "true_peak_within_-1_dbtp=" << (measured.truePeakDb <= -1.0f ? "yes" : "NO") << "\n"
             << "\n[the file as written: the same measurement on the encoded samples]\n"
             << "encoded_integrated_lufs=" << juce::String(encoded.integratedLufs, 1) << "\n"
             << "encoded_true_peak_dbtp=" << juce::String(encoded.truePeakDb, 2) << "\n"
             << "encoded_sample_peak_dbfs=" << juce::String(encoded.samplePeakDb, 2) << "\n"
             << "encoded_samples_at_full_scale=" << static_cast<juce::int64>(encoded.overFullScale) << "\n"
             << "encoded_true_peak_within_-1_dbtp=" << (encoded.truePeakDb <= -1.0f ? "yes" : "NO") << "\n"
             << "\n[delivery profiles: a report of what each destination would do to the file; nothing is applied]\n"
             << "spotify_master_ceiling_dbtp=" << juce::String(aquorbium::spotifyMasterCeilingDbtp(encoded.integratedLufs), 1)
             << "  (Spotify's advice: -2 dBTP when louder than -14 LUFS) -> "
             << (encoded.truePeakDb <= aquorbium::spotifyMasterCeilingDbtp(encoded.integratedLufs) ? "within" : "OVER") << "\n";
        for (const auto& profile : aquorbium::deliveryProfiles)
        {
            const auto verdict = aquorbium::judgeDelivery(profile, encoded.integratedLufs, encoded.truePeakDb);
            text << profile.key << ": target " << juce::String(profile.targetLufs, 0) << " LUFS / ceiling "
                 << juce::String(profile.ceilingDbtp, 0) << " dBTP; gain to target "
                 << juce::String(verdict.gainToTargetDb, 1) << " dB, applied " << juce::String(verdict.appliedGainDb, 1)
                 << " dB -> " << juce::String(verdict.resultingLufs, 1) << " LUFS, "
                 << juce::String(verdict.resultingTruePeakDb, 1) << " dBTP"
                 << (verdict.reachesTarget ? "" : "  (stays under the target: no headroom)")
                 << (verdict.truePeakWithinCeiling ? "" : "  (true peak OVER the ceiling)") << "\n";
        }
        text
             << "\ncontrols_at_start=\n" << recordingControls << "\n";
        static_cast<void>(recordingFile.withFileExtension("txt").replaceWithText(text));
        recordingNotice(failed ? aquorbium::app::T(aquorbium::app::S::recFailed)
                               : aquorbium::app::T(aquorbium::app::S::recSaved) + recordingFile.getFileName() + utf8(" · ")
                                     + juce::String(measured.integratedLufs, 1) + " LUFS" + utf8(" · ")
                                     + juce::String(measured.truePeakDb, 1) + " dBTP");
        recordingStopTicks = 0;
    }

    // a message in the audio status line, held for about eight seconds
    void recordingNotice(const juce::String& notice)
    {
        audioStatus = notice;
        statusTimerTicks = -240;
        repaint();
    }

    // the opening seed (OpeningSeed.h, 2026-10-24): the seed proposes the
    // sound too - the gates, the cards' controls, the bodies, the ecology and
    // the synthesis device's patches - from the session's biome. The output
    // stage stays the musician's. The state the musician left is kept in
    // "opening.previousStateV1" (a scene snapshot, synthesis included).
    void applyOpeningSeed()
    {
        auto state = capturePerformanceState();
        settings.setValue("opening.previousStateV1", state.snapshot().toXmlString());
        const auto& world = engine.currentSessionWorld();
        const auto biome = aquorbium::OpeningHypothesis::fromSeed(sessionSeed, world.dayPhase, world.seasonPhase);
        for (std::size_t i = 0; i < biome.count; ++i)
        {
            const auto id = std::string(biome.settings[i].id);
            if (!state.set(id.c_str(), biome.settings[i].value))
            {
                DBG("opening seed: unknown control " << id);
            }
        }
        using Key = aquorbium::app::EcologyParameterState::Key;
        auto ecology = ecologyState;
        static_cast<void>(ecology.set(Key::density, biome.density));
        static_cast<void>(ecology.set(Key::calm, biome.calm));
        static_cast<void>(ecology.set(Key::cohesion, biome.cohesion));
        static_cast<void>(ecology.set(Key::danger, biome.danger));
        static_cast<void>(ecology.set(Key::sparseFocus, biome.behaviours[0]));
        static_cast<void>(ecology.set(Key::energyMemory, biome.behaviours[1]));
        static_cast<void>(ecology.set(Key::geometricCycle, biome.behaviours[2]));
        static_cast<void>(ecology.set(Key::collectiveDrift, biome.behaviours[3]));
        static_cast<void>(state.setEcology(ecology.snapshot()));
        state.setAutonomous(true);
        applyPerformancePreset(state);
        // the synthesis device: the PROPOSTA with the session's seed
        synthEditor.mix = biome.synthMix;
        for (std::size_t i = 0; i < SynthEditor::organisms; ++i)
            synthEditor.patches[i] = aquorbium::SynthProposal::propose(i, sessionSeed);
        synthMixCard.setValue(synthEditor.mix, juce::dontSendNotification);
        synthEditor.loadControls();
        synthChanged();
        savePerformanceState();
    }

    // the VARIA's hand takes every control where it is now
    void inhabitMotion()
    {
        motionControls.clear();
        for (const auto& binding : performanceBindings())
            motionControls.push_back({ binding.id, static_cast<float>(binding.slider->getValue()) });
        for (const auto& binding : ecologyBindings())
            motionControls.push_back({ aquorbium::app::EcologyParameterState::idOf(binding.key),
                                       static_cast<float>(ecologyState.get(binding.key)) });
        motionValues.assign(motionControls.size(), 0.0f);
        controlMotion.inhabit(motionControls, sessionSeed);
    }

    // a step of the VARIA, on the message thread (30 Hz); a control the
    // musician is holding is left alone (the hand re-anchors on it after)
    void tickMotion(const VisualFrame& frame)
    {
        if (!variaAllowed || !variaButton.getToggleState() || motionControls.empty()
            || pauseRequested.load(std::memory_order_relaxed))
            return;
        const auto bindings = performanceBindings();
        const auto ecology = ecologyBindings();
        std::size_t k = 0;
        for (const auto& binding : bindings)
            motionValues[k++] = static_cast<float>(binding.slider->getValue());
        for (const auto& binding : ecology)
            motionValues[k++] = static_cast<float>(ecologyState.get(binding.key));
        const auto before = motionValues;
        controlMotion.tick(motionValues, 1.0f / 30.0f, frame.autonomousEnergy, frame.formTension,
                           juce::jlimit(0.0f, 1.0f, frame.outputTruePeak));
        auto changed = false;
        k = 0;
        for (const auto& binding : bindings)
        {
            const auto i = k++;
            if (std::abs(motionValues[i] - before[i]) < 1.0e-7f || binding.slider->isMouseButtonDown())
                continue;
            binding.slider->setValue(motionValues[i], juce::dontSendNotification);
            binding.target->store(static_cast<float>(binding.slider->getValue()), std::memory_order_relaxed);
            changed = true;
        }
        for (const auto& binding : ecology)
        {
            const auto i = k++;
            if (std::abs(motionValues[i] - before[i]) < 1.0e-7f || binding.slider->isMouseButtonDown())
                continue;
            binding.slider->setValue(binding.inverted ? 1.0 - motionValues[i] : motionValues[i], juce::dontSendNotification);
            const auto displayed = binding.slider->getValue();
            static_cast<void>(ecologyState.set(binding.key, binding.inverted ? 1.0 - displayed : displayed));
            binding.target->store(static_cast<float>(displayed), std::memory_order_relaxed);
            changed = true;
        }
        if (changed)
            submitControlFrame();
    }

    void restorePerformanceState()
    {
        aquorbium::app::PerformanceParameterState state;
        state.loadPreferences(settings);
        applyPerformancePreset(state);
    }
    void savePerformanceState()
    {
        const auto state = capturePerformanceState();
        // without the synthesis (its own settings): builds from before the
        // device still read the performance state
        settings.setValue("parameter.performanceStateV1", state.snapshotWithoutSynth().toXmlString());
        if (presetBankWritable)
            presetBank.savePreferences(settings);
        settings.setValue("parameter.ecologyStateV1", ecologyState.snapshot().toXmlString());
        settings.setValue("parameter.density", density.getValue());
        settings.setValue("parameter.drift", drift.getValue());
        settings.setValue("parameter.ecosystemMorph", ecosystemMorph.getValue());
        settings.setValue("parameter.cohesion", cohesion.getValue());
        settings.setValue("parameter.danger", danger.getValue());
        settings.setValue("parameter.blendSparseFocus", sparseFocus.getValue());
        settings.setValue("parameter.blendEnergyMemory", energyMemory.getValue());
        settings.setValue("parameter.blendGeometricCycle", geometricCycle.getValue());
        settings.setValue("parameter.blendCollectiveDrift", collectiveDrift.getValue());
        settings.setValue("parameter.geometryInfluence", geometryInfluence.getValue());
        settings.setValue("parameter.rhythmDensity", rhythmDensity.getValue());
        settings.setValue("parameter.orbitRotation", orbitRotation.getValue());
        settings.setValue("parameter.intervalBias", intervalBias.getValue());
        settings.setValue("parameter.geometryMutation", geometryMutation.getValue());
        settings.setValue("parameter.strikerInfluence", strikerInfluence.getValue());
        settings.setValue("parameter.strikerMatter", strikerMatter.getValue());
        settings.setValue("parameter.strikerPopulation", strikerPopulation.getValue());
        settings.setValue("parameter.strikerScatter", strikerScatter.getValue());
        settings.setValue("parameter.spaceReturn", spaceReturn.getValue());
        settings.setValue("parameter.spaceGesture", spaceGesture.getValue());
        settings.setValue("parameter.spaceSize", spaceSize.getValue());
        settings.setValue("parameter.spaceSpread", spaceSpread.getValue());
        settings.setValue("parameter.spaceTone", spaceTone.getValue());
        settings.setValue("parameter.spaceMemory", spaceMemory.getValue());
        settings.setValue("parameter.environmentInfluence", environmentInfluence.getValue());
        settings.setValue("parameter.environmentFilterMix", environmentFilterMix.getValue());
        settings.setValue("parameter.environmentCentre", environmentCentre.getValue());
        settings.setValue("parameter.environmentSpan", environmentSpan.getValue());
        settings.setValue("parameter.environmentResonance", environmentResonance.getValue());
        settings.setValue("parameter.environmentAmbience", environmentAmbience.getValue());
        settings.setValue("parameter.tailInfluence", tailInfluence.getValue());
        settings.setValue("parameter.tailDecay", tailDecay.getValue());
        settings.setValue("parameter.tailDamping", tailDamping.getValue());
        settings.setValue("parameter.tailNightmare", tailNightmare.getValue());
        settings.setValue("parameter.formInfluence", formInfluence.getValue());
        settings.setValue("parameter.formContext", formContext.getValue());
        settings.setValue("parameter.formInertia", formInertia.getValue());
        settings.setValue("parameter.formMutation", formMutation.getValue());
        settings.setValue("parameter.masterTrim", masterTrim.getValue());
        settings.setValue("parameter.masterCeiling", masterCeiling.getValue());
        settings.setValue("parameter.masterRelease", masterRelease.getValue());
        settings.setValue("parameter.creativeMaster", creativeMaster.getValue());
        settings.setValue("parameter.masterEqOrder", masterEqOrder.getValue());
        for (std::size_t i = 0; i < cardSliders.size(); ++i)
            settings.setValue("parameter." + juce::String(cardIds[i]), cardSliders[i].getValue());
        settings.setValue("parameter.structureInfluence", structureInfluence.getValue());
        settings.setValue("parameter.structureMemory", structureMemory.getValue());
        settings.setValue("parameter.structureHarmony", structureHarmony.getValue());
        settings.setValue("parameter.structurePhase", structurePhase.getValue());
        settings.setValue("parameter.musicInfluence", musicInfluence.getValue());
        settings.setValue("parameter.musicReaction", musicReaction.getValue());
        settings.setValue("parameter.musicMemory", musicMemory.getValue());
        settings.setValue("parameter.musicWater", musicWater.getValue());
        for (std::size_t i = 0; i < 5; ++i)
        {
            const auto n = juce::String(static_cast<int>(i));
            settings.setValue("parameter.timbreKind" + n, timbreBodyValue[i].getValue());
            settings.setValue("parameter.timbreMatter" + n, timbreMatter[i].getValue());
            settings.setValue("parameter.timbreMix" + n, timbreMix[i].getValue());
        }
        settings.setValue("parameter.timbreEcology", timbreEcology.getValue());
        settings.setValue("parameter.supportPresence", supportPresence.getValue());
        settings.setValue("parameter.musicSalinity", musicSalinity.getValue());
        for (std::size_t index = 0; index < moduleBankDescriptors.size(); ++index)
            settings.setValue("parameter." + juce::String(moduleBankDescriptors[index].id),
                              moduleBankSliders[index].getValue());
        settings.setValue("parameter.autonomous", autonomy.getToggleState());
        static_cast<void>(settings.saveIfNeeded());
    }

    [[nodiscard]] aquorbium::app::PerformanceParameterState capturePerformanceState()
    {
        aquorbium::app::PerformanceParameterState state;
        static_cast<void>(state.setEcology(ecologyState.snapshot()));
        for (const auto& binding : performanceBindings())
            static_cast<void>(state.set(binding.id, binding.slider->getValue()));
        state.setAutonomous(autonomy.getToggleState());
        // the synthesis device travels with the scenes (2026-10-22)
        std::array<juce::String, aquorbium::app::PerformanceParameterState::synthPatchCount> patches;
        for (std::size_t i = 0; i < patches.size(); ++i)
            patches[i] = juce::String(aquorbium::SynthPatchCodec::encode(synthEditor.patches[i]));
        static_cast<void>(state.setSynth(juce::jlimit(0.0, 1.0, static_cast<double>(synthEditor.mix)), patches));
        return state;
    }

    // a recalled scene's synthesis: its mix and patches; a scene from before
    // the device sounds as it was saved (SYNTHESIS 0) and keeps the patches
    // being edited. Not applied at startup: the synthesis has its own
    // settings (synth.mix, synth.patch.N).
    void applySceneSynth(const aquorbium::app::PerformanceParameterState& state)
    {
        if (state.hasSynth())
        {
            synthEditor.mix = static_cast<float>(juce::jlimit(0.0, 1.0, state.synthMix()));
            for (std::size_t i = 0; i < SynthEditor::organisms; ++i)
                synthEditor.patches[i] = aquorbium::SynthPatchCodec::decode(state.synthPatch(i).toStdString());
        }
        else
            synthEditor.mix = 0.0f;
        synthMixCard.setValue(synthEditor.mix, juce::dontSendNotification);
        synthEditor.loadControls();
        synthChanged();
    }

    void applyPerformancePreset(const aquorbium::app::PerformanceParameterState& state)
    {
        // No notifications: one coherent publication after updating all widgets.
        if (!applyEcologyState(state.ecologySnapshot()))
            return;
        for (const auto& binding : performanceBindings())
        {
            binding.slider->setValue(state.get(binding.id), juce::dontSendNotification);
            binding.target->store(static_cast<float>(binding.slider->getValue()),
                                  std::memory_order_relaxed);
        }
        autonomy.setToggleState(state.autonomous(), juce::dontSendNotification);
        autonomyTarget.store(state.autonomous(), std::memory_order_relaxed);
        aquorbium::app::AudioControlFrame transients;
        transients.lensX = lensTargetX.load(std::memory_order_relaxed);
        transients.lensY = lensTargetY.load(std::memory_order_relaxed);
        transients.lensZ = lensTargetZ.load(std::memory_order_relaxed);
        transients.lensHeld = lensHeldTarget.load(std::memory_order_relaxed);
        // Capture displayed/quantized values, not unquantized imported values.
        controlFrames.publish(aquorbium::app::sceneControls(capturePerformanceState(), transients));
        updateControlAvailability();
    }

    [[nodiscard]] std::size_t selectedPresetSlot() const
    {
        return static_cast<std::size_t>(std::max(0, presetSlot.getSelectedId() - 1));
    }

    void updatePresetStatus()
    {
        const auto selected = selectedPresetSlot();
        const auto occupied = presetBank.occupied(selected);
        for (std::size_t slot = 0; slot < aquorbium::app::PerformancePresetBank::capacity; ++slot)
        {
            auto label = aquorbium::app::T(aquorbium::app::S::scene) + juce::String(static_cast<int>(slot) + 1);
            if (presetBank.name(slot).isNotEmpty()) label += utf8(" · ") + presetBank.name(slot);
            presetSlot.changeItemText(static_cast<int>(slot) + 1, label);
        }
        presetSlot.setSelectedId(static_cast<int>(selected) + 1, juce::dontSendNotification);
        presetSlot.setTooltip(presetSlot.getText());
        presetName.setText(presetBank.name(selected), false);
        updatePresetAvailability();
        presetStatus.setText(occupied ? aquorbium::app::T(aquorbium::app::S::sceneSaved) : aquorbium::app::T(aquorbium::app::S::sceneEmpty), juce::dontSendNotification);
        presetStatus.setTooltip(presetStatus.getText());
    }

    void configurePresets()
    {
        const auto valid = presetBank.loadPreferences(settings);
        presetBankWritable = valid; // Keep invalid on-disk evidence until explicit store.
        for (int slot = 1; slot <= 4; ++slot)
            presetSlot.addItem(aquorbium::app::T(aquorbium::app::S::scene) + juce::String(slot), slot);
        presetSlot.setSelectedId(1, juce::dontSendNotification);
        presetSlot.onChange = [this] { updatePresetStatus(); };
        presetStore.setButtonText("GRAVAR");
        presetRecall.setButtonText("RECUPERAR");
        presetDefault.setButtonText("INICIAL");
        presetImport.setButtonText("IMPORTAR CENA");
        presetExport.setButtonText("EXPORTAR CENA");
        presetRename.setButtonText("RENOMEAR");
        presetUndo.setButtonText("DESFAZER");
        presetName.setInputRestrictions(aquorbium::app::PerformancePresetBank::maximumNameCharacters);
        presetName.setTextToShowWhenEmpty("Nome da cena (opcional)", Palette::muted);
        presetName.setFont(uiFont(11.0f));
        presetName.setColour(juce::TextEditor::backgroundColourId, Palette::panel);
        presetName.setColour(juce::TextEditor::textColourId, Palette::text);
        presetName.setTooltip(utf8("Até 32 caracteres; Enter ou RENOMEAR confirma. GRAVAR inclui o nome digitado."));
        presetRename.setTitle(utf8("Renomear a cena salva sem alterar o som; nome vazio remove o nome"));
        presetUndo.setTitle(utf8("Desfazer a última gravação, importação ou renomeação; não altera o som atual"));
        presetRename.onClick = [this] {
            if (!presetBank.rename(selectedPresetSlot(), presetName.getText()))
            {
                finishSceneTransfer(utf8("Nome inválido ou cena vazia; banco preservado"));
                return;
            }
            completePresetMutation("Cena renomeada; som preservado");
        };
        presetName.onReturnKey = [this] {
            if (presetRename.isEnabled()) presetRename.onClick();
        };
        presetUndo.onClick = [this] {
            const auto slot = presetBank.undoSlot();
            if (!presetBank.undo()) return;
            presetSlot.setSelectedId(static_cast<int>(slot) + 1, juce::dontSendNotification);
            completePresetMutation(utf8("Última alteração desfeita; som preservado"));
        };
        presetImport.setTitle(utf8("Importar arquivo .aquscene para o slot sem alterar o som"));
        presetExport.setTitle(utf8("Exportar a cena salva no slot; grave antes para incluir os controles atuais"));
        presetImport.onClick = [this] { chooseSceneFile(false); };
        presetExport.onClick = [this] { chooseSceneFile(true); };
        presetSlot.setTitle(utf8("Escolher uma das quatro cenas locais"));
        presetStore.setTitle(utf8("Gravar os 31 valores atuais na cena escolhida, substituindo a anterior"));
        presetRecall.setTitle(utf8("Recuperar a cena sem reiniciar notas, sementes ou memórias DSP"));
        presetDefault.setTitle(utf8("Restaurar valores iniciais sem apagar as cenas"));
        presetStatus.setColour(juce::Label::textColourId, Palette::muted);
        presetStatus.setFont(uiFont(10.0f));
        presetStore.onClick = [this] {
            if (!presetBank.store(selectedPresetSlot(), capturePerformanceState(), presetName.getText()))
                finishSceneTransfer(utf8("Nome inválido; cena anterior preservada"));
            else completePresetMutation("Cena salva");
        };
        presetRecall.onClick = [this] {
            aquorbium::app::PerformanceParameterState state;
            if (presetBank.recall(selectedPresetSlot(), state))
            {
                applyPerformancePreset(state);
                applySceneSynth(state);
                presetStatus.setText(aquorbium::app::T(aquorbium::app::S::sceneRecalled), juce::dontSendNotification);
            }
        };
        presetDefault.onClick = [this] {
            applyPerformancePreset(aquorbium::app::PerformanceParameterState {});
            applySceneSynth(aquorbium::app::PerformanceParameterState {});  // SYNTHESIS 0, patches kept
            presetStatus.setText(aquorbium::app::T(aquorbium::app::S::sceneDefaults), juce::dontSendNotification);
        };
        for (auto* component : { static_cast<juce::Component*>(&presetSlot),
                                 static_cast<juce::Component*>(&presetStore),
                                 static_cast<juce::Component*>(&presetRecall),
                                 static_cast<juce::Component*>(&presetDefault),
                                 static_cast<juce::Component*>(&presetImport),
                                 static_cast<juce::Component*>(&presetExport),
                                 static_cast<juce::Component*>(&presetName),
                                 static_cast<juce::Component*>(&presetRename),
                                 static_cast<juce::Component*>(&presetUndo),
                                 static_cast<juce::Component*>(&presetStatus) })
            addAndMakeVisible(*component);
        updatePresetStatus();
        if (!valid)
            presetStatus.setText(utf8("Banco inválido; cenas indisponíveis"), juce::dontSendNotification);
    }

    void chooseSceneFile(bool exporting)
    {
        if (sceneFileChooser || sceneTransferPending)
            return;
        const auto slot = selectedPresetSlot();
        aquorbium::app::PerformanceParameterState state;
        if (exporting && !presetBank.recall(slot, state))
            return;
        // Freeze slot and scene at dialog opening; subsequent GUI changes aren't exported.
        const auto snapshot = state.snapshot();
        sceneTransferPending = true;
        sceneFileChooser = std::make_unique<juce::FileChooser>(
            exporting ? "Exportar cena salva" : "Importar cena sem alterar o som",
            juce::File {}, "*.aquscene", true, false, this);
        updatePresetAvailability();
        const juce::Component::SafePointer<MainComponent> safe(this);
        const auto chooserFlags = juce::FileBrowserComponent::canSelectFiles
            | (exporting ? juce::FileBrowserComponent::saveMode : juce::FileBrowserComponent::openMode);
        sceneFileChooser->launchAsync(chooserFlags, [safe, exporting, slot, snapshot](const juce::FileChooser& chooser) {
            if (!safe)
                return;
            auto file = chooser.getResult();
            // Don't destroy the chooser inside its own callback. Re-enable later.
            juce::MessageManager::callAsync([safe] {
                if (safe)
                {
                    safe->sceneFileChooser.reset();
                    safe->updatePresetAvailability();
                }
            });
            if (file == juce::File {})
            {
                safe->finishSceneTransfer(utf8("Transferência cancelada"));
                return;
            }
            if (exporting)
            {
                file = file.withFileExtension(aquorbium::app::PerformanceSceneFile::extension);
                const auto write = [safe, file, snapshot](bool replace) {
                    if (!safe)
                        return;
                    aquorbium::app::PerformanceParameterState frozen;
                    if (!frozen.restore(snapshot))
                    {
                        safe->finishSceneTransfer(utf8("Cena inválida; exportação cancelada"));
                        return;
                    }
                    const auto outcome = aquorbium::app::PerformanceSceneFile::write(file, frozen, replace);
                    safe->finishSceneTransfer(outcome.result.failed() ? outcome.result.getErrorMessage()
                        : outcome.backup == juce::File {} ? "Cena exportada"
                                                        : "Cena exportada; anterior em .bak",
                        file.getFullPathName() + (outcome.backup == juce::File {} ? juce::String {}
                            : "\nBackup: " + outcome.backup.getFullPathName()));
                };
                if (!file.exists())
                    write(false);
                else
                    juce::AlertWindow::showAsync(juce::MessageBoxOptions()
                        .withIconType(juce::MessageBoxIconType::WarningIcon)
                        .withTitle("Substituir arquivo de cena?")
                        .withMessage(file.getFullPathName() + utf8("\nUma cópia .bak do arquivo anterior será preservada."))
                        .withButton("Substituir").withButton("Cancelar"),
                        [safe, write](int result) {
                            if (result == 1) write(true);
                            else if (safe) safe->finishSceneTransfer(utf8("Exportação cancelada"));
                        });
            }
            else
            {
                aquorbium::app::PerformanceParameterState imported;
                const auto outcome = aquorbium::app::PerformanceSceneFile::read(file, imported);
                if (outcome.failed())
                {
                    safe->finishSceneTransfer(outcome.getErrorMessage());
                    return;
                }
                const auto importedSnapshot = imported.snapshot();
                const auto store = [safe, slot, importedSnapshot](int result) {
                    if (!safe)
                        return;
                    if (result != 1)
                    {
                        safe->finishSceneTransfer(utf8("Importação cancelada"));
                        return;
                    }
                    aquorbium::app::PerformanceParameterState validated;
                    if (!validated.restore(importedSnapshot) || !safe->presetBank.store(slot, validated))
                    {
                        safe->finishSceneTransfer(utf8("Cena inválida; importação cancelada"));
                        return;
                    }
                    safe->presetBankWritable = true;
                    safe->presetBank.savePreferences(safe->settings);
                    const auto saved = safe->settings.saveIfNeeded();
                    safe->updatePresetStatus();
                    safe->finishSceneTransfer(saved ? "Cena importada; clique RECUPERAR para aplicar"
                                                    : utf8("Cena importada em memória; falha ao salvar"));
                };
                if (!safe->presetBank.occupied(slot))
                    store(1);
                else
                    juce::AlertWindow::showAsync(juce::MessageBoxOptions()
                        .withIconType(juce::MessageBoxIconType::WarningIcon)
                        .withTitle("Substituir cena salva?")
                        .withMessage("A cena " + juce::String(static_cast<int>(slot) + 1)
                            + utf8(" será substituída. O som atual não será alterado."))
                        .withButton("Substituir").withButton("Cancelar"), store);
            }
        });
    }

    void updatePresetAvailability()
    {
        const auto occupied = presetBank.occupied(selectedPresetSlot());
        presetRecall.setEnabled(occupied);
        const auto available = !sceneFileChooser && !sceneTransferPending;
        presetExport.setEnabled(occupied && available);
        presetImport.setEnabled(available);
        presetSlot.setEnabled(available);
        presetStore.setEnabled(available);
        presetName.setEnabled(available);
        presetRename.setEnabled(occupied && available);
        presetUndo.setEnabled(presetBank.canUndo() && available);
    }

    void completePresetMutation(const juce::String& message)
    {
        presetBankWritable = true;
        presetBank.savePreferences(settings);
        const auto saved = settings.saveIfNeeded();
        updatePresetStatus();
        finishSceneTransfer(saved ? message : utf8("Banco em memória; falha ao salvar"));
    }

    void finishSceneTransfer(const juce::String& message, const juce::String& detail = {})
    {
        sceneTransferPending = false;
        updatePresetAvailability();
        presetStatus.setText(message, juce::dontSendNotification);
        presetStatus.setTooltip(detail.isEmpty() ? message : message + "\n" + detail);
    }

    // the LUPA, MASTER CRIATIVO, CÉREBRO and SALA controls (2026-10-24)
    void configureCardControls()
    {
        for (std::size_t i = 0; i < cardSliders.size(); ++i)
        {
            auto& slider = cardSliders[i];
            auto& label = cardLabels[i];
            const auto initial = static_cast<double>(cardTargets[i].load(std::memory_order_relaxed));
            if (i < 8)
            {
                configureMacro(slider, label, cardIds[i], initial, juce::String(), i < 4 ? Palette::cyan : Palette::green);
                slider.setRange(0.0, 1.0, 0.0);
                (i < 4 ? lensControls : masterControls).addAndMakeVisible(slider);
                (i < 4 ? lensControls : masterControls).addAndMakeVisible(label);
            }
            else
            {
                configureControl(slider, label, cardIds[i], initial);
                addChildComponent(slider);
                addChildComponent(label);
            }
        }
        for (auto& slider : cardSliders)
            slider.textFromValueFunction = [](double v) { return juce::String(v, 2); };
        // the values in their own units
        cardSliders[8].textFromValueFunction = [](double v) {
            return juce::String(0.23 * std::exp2((v - 0.5) * 4.0), 2) + " Hz";
        };
        cardSliders[10].textFromValueFunction = [](double v) {
            const auto db = (v - 0.5) * 24.0;
            return (db >= 0.0 ? "+" : "") + juce::String(db, 1) + " dB";
        };
        for (auto* l : { &brainHeading, &roomHeading })
        {
            l->setFont(uiFont(11.0f, juce::Font::bold));
            l->setColour(juce::Label::textColourId, Palette::cyan);
            addChildComponent(*l);
        }
        for (auto* l : { &brainStatus, &roomStatus })
        {
            l->setFont(uiFont(10.0f));
            l->setColour(juce::Label::textColourId, Palette::muted);
            l->setJustificationType(juce::Justification::centredRight);
            addChildComponent(*l);
        }
        for (auto& slider : cardSliders)
            slider.updateText();
    }

    static void configureControl(juce::Slider& slider, juce::Label& label,
                                 const juce::String& name, double initial)
    {
        slider.setRange(0.0, 1.0, 0.01);
        slider.setSliderStyle(juce::Slider::LinearHorizontal);
        slider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 58, 22);
        slider.setColour(juce::Slider::trackColourId, Palette::cyan.withAlpha(0.72f));
        slider.setColour(juce::Slider::backgroundColourId, Palette::line.withAlpha(0.46f));
        slider.setColour(juce::Slider::thumbColourId, Palette::text);
        slider.setColour(juce::Slider::textBoxTextColourId, Palette::text);
        slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        slider.setValue(initial);
        label.setText(name, juce::dontSendNotification);
        label.setColour(juce::Label::textColourId, Palette::muted);
        label.setFont(uiFont(10.0f, juce::Font::bold));
        label.setJustificationType(juce::Justification::centredLeft);
    }

    static void configureMacro(juce::Slider& slider, juce::Label& label,
                               const juce::String& name, double initial,
                               const juce::String& description,
                               juce::Colour tint = Palette::coral)
    {
        slider.setRange(0.0, 1.0, 0.01);
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 58, 20);
        slider.setColour(juce::Slider::rotarySliderFillColourId,
                         tint.withAlpha(0.82f));
        slider.setColour(juce::Slider::rotarySliderOutlineColourId,
                         Palette::line.withAlpha(0.48f));
        slider.setColour(juce::Slider::thumbColourId, Palette::text);
        slider.setColour(juce::Slider::textBoxTextColourId, Palette::text);
        slider.setColour(juce::Slider::textBoxOutlineColourId,
                         juce::Colours::transparentBlack);
        slider.setName(name);
        slider.setTitle(description);
        slider.setTooltip(description);
        slider.setViewportIgnoreDragFlag(true);
        slider.setScrollWheelEnabled(false);
        slider.setValue(initial, juce::dontSendNotification);
        label.setText(name, juce::dontSendNotification);
        label.setColour(juce::Label::textColourId, Palette::muted);
        label.setFont(uiFont(11.0f));
        label.setJustificationType(juce::Justification::centred);
        label.setInterceptsMouseClicks(false, false);
    }

    // the VARIA's row: the switch, then how far its hand travels
    void layoutVaria(juce::Rectangle<int> row)
    {
        variaButton.setBounds(row.removeFromLeft(std::min(120, row.getWidth() / 2)));
        variaIntensity.setVisible(row.getWidth() >= 90);
        variaIntensity.setBounds(row.reduced(0, 6));
    }

    static void layoutControl(juce::Rectangle<int>& area, juce::Label& label,
                              juce::Slider& slider, int rowHeight = 43,
                              int labelWidth = 270)
    {
        auto row = area.removeFromTop(rowHeight);
        label.setBounds(row.removeFromLeft(labelWidth));
        slider.setBounds(row);
    }

    static void layoutStackedControl(juce::Rectangle<int>& area, juce::Label& label,
                                     juce::Slider& slider, int rowHeight)
    {
        auto row = area.removeFromTop(rowHeight);
        label.setBounds(row.removeFromTop(16));
        slider.setBounds(row);
    }

    juce::PropertiesFile& settings;
    std::uint32_t sessionSeed = 42;  // set before the audio starts, then constant
    bool sessionGenesis = true;      // the session's world drawn from its seed
    // the world in a few words: tonic, mode, orbit
    [[nodiscard]] juce::String sessionWorldText() const
    {
        static constexpr std::array<const char*, 12> names { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
        const auto& world = engine.currentSessionWorld();
        const auto tonic = ((36 + world.rootOffset) % 12 + 12) % 12;
        static constexpr std::array<const char*, 5> languages { "funcional", "modal", "brasileira", "impressionista", "cromática" };
        const auto language = world.harmonic ? utf8(" · ") + utf8(languages[static_cast<std::size_t>(world.harmony.language)])
                                                   + (world.harmony.polytonal ? utf8(" + politonal") : juce::String())
                                             : juce::String();
        return juce::String(names[static_cast<std::size_t>(tonic)]) + " " + world.modeName + language
            + utf8(" · órbita ") + juce::String(static_cast<int>(world.orbitSteps));
    }
    aquorbium::app::EcologyParameterState ecologyState;
    aquorbium::app::PerformancePresetBank presetBank;
    bool presetBankWritable = true;
    juce::ComboBox presetSlot;
    juce::TextButton presetStore;
    juce::TextButton presetRecall;
    juce::TextButton presetDefault;
    juce::TextButton presetImport;
    juce::TextButton presetExport;
    juce::TextEditor presetName;
    juce::TextButton presetRename;
    juce::TextButton presetUndo;
    std::unique_ptr<juce::FileChooser> sceneFileChooser;
    bool sceneTransferPending = false;
    juce::Label presetStatus;
    aquorbium::BiomaEngine engine;
    VisualTelemetryBridge visualTelemetry;
    InstrumentLookAndFeel lookAndFeel;
    juce::TooltipWindow tooltips { this, 700 };
    juce::TextButton audioSettingsButton;
    juce::TextButton muteButton;
    juce::TextButton pauseButton;
    juce::TextButton recButton;
    juce::TextButton listeningButton;
    std::atomic<bool> headphonesRequested { false };
    std::vector<float> roomScratch;              // sized in prepareToPlay
    std::vector<float> tapLeft, tapRight;        // the REC taps, sized in prepareToPlay
    // which tap the take listens to (BiomaEngine::RecordTap), set when it starts
    std::atomic<int> recorderTap { 0 };
    std::atomic<bool> roomRequested { false };
    std::atomic<std::uint32_t> soundmarkChoices { 0 };
    std::uint32_t soundmarkChoicesSeen = 0;           // audio thread
    std::mutex soundmarkKeptLock;
    aquorbium::SongForm::Phrase soundmarkKept {};     // "music.soundmark" in the settings
    std::unique_ptr<aquorbium::app::DiagnosticAudioCapture> recorder;
    // the stems beside the take (2026-10-22)
    std::array<std::unique_ptr<aquorbium::app::DiagnosticAudioCapture>, aquorbium::BiomaEngine::stemCount> stemRecorders;
    std::array<std::atomic<aquorbium::app::DiagnosticAudioCapture*>, aquorbium::BiomaEngine::stemCount> stemRecordersForAudio {};
    std::atomic<bool> stemsForAudio { false };
    std::unique_ptr<aquorbium::app::AmbisonicCapture> ambisonicRecorder;
    std::atomic<aquorbium::app::AmbisonicCapture*> ambisonicRecorderForAudio { nullptr };
    std::array<std::vector<float>, 4> ambisonicBuffer;  // W Y Z X, sized in prepareToPlay
    juce::String ambisonicFile;
    std::array<std::vector<float>, aquorbium::BiomaEngine::stemCount> stemLeft, stemRight;  // sized in prepareToPlay
    juce::StringArray stemFiles;
    std::atomic<aquorbium::app::DiagnosticAudioCapture*> recorderForAudio { nullptr };
    juce::File recordingFile;
    juce::Time recordingStart;
    juce::String recordingControls;
    unsigned recordingBits = 32;
    juce::String recordingTapName { "post-safety" };
    int recordingStopTicks = 0;
    std::atomic<bool> muteRequested { false };
    std::atomic<bool> pauseRequested { false };
    float outputGain = 1.0f;      // audio owner only: the mute/pause ramp
    float outputGainStep = 1.0f / 960.0f;
    std::unique_ptr<juce::DialogWindow> audioSettingsWindow;
    juce::String audioStatus;
    int statusTimerTicks = 0;
    juce::int64 lastGuiTick = 0;
    std::uint32_t guiPauseCounter = 0; // GUI-owned; no access from audio
    // GUI-independent diagnostic state. Values below are audio-owned, except
    // for the explicitly atomic counters consumed by the GUI.
    double preparedRate = 48000.0;
    std::uint32_t telemetryInterval = 1600;
    std::uint32_t samplesUntilTelemetry = 0;
    std::atomic<std::uint32_t> overBudgetCounter { 0 };
    std::atomic<bool> realtimeAsked { false };   // set by the audio thread's first callback
    std::atomic<int> realtimePriority { 0 };     // SCHED_RR priority obtained, 0 = none
    std::atomic<std::uint32_t> clippedBlockCounter { 0 };
    std::atomic<std::uint32_t> audioCallbackStartedCounter { 0 };
    struct CaptureReading
    {
        double seconds = 0.0, cpu = 0.0, density = 0.0, drift = 0.0;
        double geometry = 0.0, striker = 0.0, space = 0.0;
        int xruns = 0;
        std::uint32_t callbacks = 0, late = 0, clip = 0, gui = 0;
    };
    std::unique_ptr<aquorbium::app::DiagnosticAudioCapture> diagnosticCapture;
    std::atomic<aquorbium::app::DiagnosticAudioCapture*> captureForAudio { nullptr };
    std::atomic<bool> captureRateChanged { false };
    std::array<CaptureReading, 256> captureReadings {};
    std::size_t captureRows = 0;
    juce::File captureDirectory;
    juce::String captureDevice, captureBackend;
    double captureStarted = 0.0, captureNextReadingMs = 0.0, captureRate = 0.0;
    int captureBuffer = 0, captureFinalXruns = 0;
    bool captureFinalized = false;
    struct RuntimeAuditReading
    {
        double seconds = 0.0, cpu = 0.0;
        int xruns = 0;
        std::uint32_t callbacks = 0, late = 0, clip = 0, gui = 0;
    };
    std::array<RuntimeAuditReading, 256> runtimeAuditReadings {};
    std::size_t runtimeAuditRows = 0;
    juce::File runtimeAuditDirectory;
    juce::String runtimeAuditDevice, runtimeAuditBackend;
    double runtimeAuditStarted = 0.0, runtimeAuditRate = 48000.0;
    int runtimeAuditStep = -1, runtimeAuditBuffer = 0;
    int runtimeAuditSceneSecond = -1;
    std::uint32_t runtimeAuditSceneRecalls = 0, runtimeAuditAutonomySwitches = 0, runtimeAuditGeneratedEvents = 0;
    bool runtimeAuditStress = false, runtimeAuditSceneFailure = false, runtimeAuditNotesReleased = false;
    bool runtimeAuditActive = false; // GUI-owned
    bool runtimeAuditEnabled = false; // immutable while callback is running
    // Audio-owned; read on GUI only after shutdownAudio().
    std::uint64_t runtimeAuditSamples = 0, runtimeAuditInvalidSamples = 0;
    double runtimeAuditSquared = 0.0;
    float runtimeAuditPeak = 0.0f, runtimeAuditDelta = 0.0f;
    float runtimeAuditPreviousLeft = 0.0f, runtimeAuditPreviousRight = 0.0f;
    ModuleCard brainCard;
    ModuleCard geometryCard;
    ModuleCard synthesisCard;
    ModuleCard spaceCard;
    ModuleCard environmentCard;
    ModuleCard tailCard;
    ModuleCard masterCard;
    ModuleCard moduleBankCard;
    ModuleCard timbreCard;
    ModuleCard meterCard;   // MEDIDORES (2026-10-24)
    MeterPanel meterPanel;
    // the LUPA, MASTER CRIATIVO, CÉREBRO and SALA controls (2026-10-24), in
    // the order of PerformanceParameterState: 0-3 lens, 4-7 master, 8-9
    // brain, 10-11 room; 0.5 is classic (coordination 0)
    static constexpr std::array<const char*, 12> cardIds { "lensRadius", "lensReach", "lensMagnification", "lensBlur",
        "masterMultiband", "masterGlue", "masterTilt", "masterWidth", "brainClock", "brainCoordination",
        "roomSensitivity", "roomReaction" };
    std::array<juce::Slider, 12> cardSliders;
    std::array<juce::Label, 12> cardLabels;
    std::array<std::atomic<float>, 12> cardTargets { 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.0f, 0.5f, 0.5f };
    ModuleCard lensCard;
    juce::Viewport lensViewport;
    juce::Component lensControls;
    juce::Label brainHeading, brainStatus, roomHeading, roomStatus;
    bool cardRowsFit = false;
    // on smaller screens the CÉREBRO and SALA rows live in a card of the
    // right column instead of the ecology panel
    ModuleCard brainRoomCard;
    juce::Component brainRoomControls;
    bool brainRoomInCard = false;
    // the six lines (heading + 2 rows, twice) into a parent, from the top
    void placeBrainRoomRows(juce::Component& parent, juce::Rectangle<int> area, int labelWidth, int pitch, int rowHeight)
    {
        auto y = area.getY();
        const auto place = [&](juce::Label& heading, juce::Label& status, std::size_t first) {
            for (auto* c : { static_cast<juce::Component*>(&heading), static_cast<juce::Component*>(&status),
                             static_cast<juce::Component*>(&cardLabels[first]), static_cast<juce::Component*>(&cardSliders[first]),
                             static_cast<juce::Component*>(&cardLabels[first + 1]), static_cast<juce::Component*>(&cardSliders[first + 1]) })
                if (c->getParentComponent() != &parent)
                    parent.addChildComponent(*c);
            auto line = juce::Rectangle<int>(area.getX(), y, area.getWidth(), pitch);
            heading.setBounds(line.removeFromLeft(labelWidth));
            status.setBounds(line);
            y += pitch;
            for (std::size_t k = first; k < first + 2; ++k)
            {
                auto row = juce::Rectangle<int>(area.getX(), y, area.getWidth(), rowHeight);
                cardLabels[k].setBounds(row.removeFromLeft(labelWidth));
                cardSliders[k].setBounds(row);
                y += pitch;
            }
        };
        place(brainHeading, brainStatus, 8);
        place(roomHeading, roomStatus, 10);
    }
    juce::Component timbreControls;
    std::array<juce::Label, 5> timbreNames;
    std::array<juce::Label, 3> timbreHeadings;
    std::array<juce::ComboBox, 5> timbreBodySelect;
    std::array<juce::Slider, 5> timbreBodyValue;   // the persisted parameter behind each selector (timbreKind*)
    std::array<juce::Slider, 5> timbreLegacyBody;  // timbreBody* (index / 6), kept only for old scenes
    std::array<std::atomic<float>, 5> timbreLegacyTarget {};
    std::array<juce::Slider, 5> timbreMatter;
    std::array<juce::Slider, 5> timbreMix;
    juce::Slider timbreEcology;
    juce::Label timbreEcologyLabel;
    juce::Slider supportPresence;
    juce::Label supportPresenceLabel;
    std::atomic<float> supportPresenceTarget { 0.2f };
    std::array<std::atomic<float>, 5> timbreBodyTarget {};
    std::array<std::atomic<float>, 5> timbreMatterTarget {};
    std::array<std::atomic<float>, 5> timbreMixTarget {};
    std::atomic<float> timbreEcologyTarget { 0.0f };
    juce::Slider musicSalinity;  // the persisted parameter behind the sea/fresh switch
    std::atomic<float> musicSalinityTarget { 1.0f };
    // tutorial, learn, about and languages (2026-10-20)
    juce::TextButton tutorialButton;
    juce::TextButton learnButton;
    juce::TextButton aboutButton;
    juce::TextButton languageButton;
    GuideOverlay guide;
    SynthEditor synthEditor;
    juce::Viewport synthEditorViewport;
    juce::Slider synthMixCard;
    juce::Label synthMixCardLabel;
    juce::TextButton synthEditButton;
    LearnPanel learnPanel;
    bool learnMode = false;
    juce::String lastLearnKey;
    juce::Component rightColumnControls;
    juce::Viewport rightColumnViewport;
    juce::Component geometryMemoryControls;
    juce::Component synthesisControls;
    juce::Component spaceControls;
    juce::Component environmentControls;
    juce::Component tailControls;
    juce::Component masterControls;
    juce::Component moduleBankControls;
    juce::Viewport geometryMemoryViewport;
    juce::Viewport synthesisViewport;
    juce::Viewport spaceViewport;
    juce::Viewport environmentViewport;
    juce::Viewport tailViewport;
    juce::Viewport masterViewport;
    juce::Viewport moduleBankViewport;
    EcosystemView ecosystemView;
    EcologyControlsBackdrop ecologyControlsBackdrop;
    juce::TextButton ecologyControlsToggle;
    juce::ToggleButton autonomy;
    juce::ToggleButton gate;
    juce::Slider density;
    juce::Slider drift;
    juce::Slider ecosystemMorph;
    juce::Slider cohesion;
    juce::Slider danger;
    juce::Slider sparseFocus;
    juce::Slider energyMemory;
    juce::Slider geometricCycle;
    juce::Slider collectiveDrift;
    juce::Slider geometryInfluence;
    juce::Slider rhythmDensity;
    juce::Slider orbitRotation;
    juce::Slider intervalBias;
    juce::Slider geometryMutation;
    juce::Slider formInfluence;
    juce::Slider formContext;
    juce::Slider formInertia;
    juce::Slider formMutation;
    juce::Slider strikerInfluence;
    juce::Slider strikerMatter;
    juce::Slider strikerPopulation;
    juce::Slider strikerScatter;
    juce::Slider spaceReturn;
    juce::Slider spaceGesture;
    juce::Slider spaceSize;
    juce::Slider spaceSpread;
    juce::Slider spaceTone;
    juce::Slider spaceMemory;
    juce::Slider environmentInfluence;
    juce::Slider environmentFilterMix;
    juce::Slider environmentCentre;
    juce::Slider environmentSpan;
    juce::Slider environmentResonance;
    juce::Slider environmentAmbience;
    juce::Slider tailInfluence;
    juce::Slider tailDecay;
    juce::Slider tailDamping;
    juce::Slider tailNightmare;
    juce::Slider creativeMaster;
    juce::Slider masterTrim;
    juce::Slider masterCeiling;
    juce::Slider masterRelease;
    juce::Slider masterEqOrder;
    juce::Slider headerVolume;     // OUTPUT TRIM again, in the header (2026-10-24)
    juce::Label headerVolumeLabel;
    // the VARIA (ControlMotion.h, 2026-10-24): the controls move over the
    // music's time; motionControls lists the performance bindings, then the
    // ecology, in the order the hand was given them
    juce::ToggleButton variaButton;
    juce::Slider variaIntensity;
    aquorbium::ControlMotion controlMotion;
    std::vector<aquorbium::ControlMotion::Control> motionControls;
    std::vector<float> motionValues;
    bool variaAllowed = false;  // only in a live session: audits and captures stay still
    juce::Slider structureInfluence;
    juce::Slider structureMemory;
    juce::Slider structureHarmony;
    juce::Slider structurePhase;
    juce::Slider musicInfluence;
    juce::Slider musicReaction;
    juce::Slider musicMemory;
    juce::Slider musicWater;
    std::array<juce::Slider, aquorbium::IntegratedModuleBank::controlCount>
        moduleBankSliders;
    juce::Label densityLabel;
    juce::Label driftLabel;
    juce::Label ecosystemMorphLabel;
    juce::Label cohesionLabel;
    juce::Label dangerLabel;
    juce::Label sparseFocusLabel;
    juce::Label energyMemoryLabel;
    juce::Label geometricCycleLabel;
    juce::Label collectiveDriftLabel;
    juce::Label geometryInfluenceLabel;
    juce::Label rhythmDensityLabel;
    juce::Label orbitRotationLabel;
    juce::Label intervalBiasLabel;
    juce::Label geometryMutationLabel;
    juce::Label formInfluenceLabel;
    juce::Label formContextLabel;
    juce::Label formInertiaLabel;
    juce::Label formMutationLabel;
    juce::Label strikerInfluenceLabel;
    juce::Label strikerMatterLabel;
    juce::Label strikerPopulationLabel;
    juce::Label strikerScatterLabel;
    juce::Label spaceReturnLabel;
    juce::Label spaceGestureLabel;
    juce::Label spaceSizeLabel;
    juce::Label spaceSpreadLabel;
    juce::Label spaceToneLabel;
    juce::Label spaceMemoryLabel;
    juce::Label environmentInfluenceLabel;
    juce::Label environmentFilterMixLabel;
    juce::Label environmentCentreLabel;
    juce::Label environmentSpanLabel;
    juce::Label environmentResonanceLabel;
    juce::Label environmentAmbienceLabel;
    juce::Label tailInfluenceLabel;
    juce::Label tailDecayLabel;
    juce::Label tailDampingLabel;
    juce::Label tailNightmareLabel;
    juce::Label creativeMasterLabel;
    juce::Label masterTrimLabel;
    juce::Label masterCeilingLabel;
    juce::Label masterReleaseLabel;
    juce::Label masterEqOrderLabel;
    juce::Label structureInfluenceLabel;
    juce::Label structureMemoryLabel;
    juce::Label structureHarmonyLabel;
    juce::Label structurePhaseLabel;
    juce::Label musicInfluenceLabel;
    juce::Label musicReactionLabel;
    juce::Label musicMemoryLabel;
    juce::Label musicWaterLabel;
    std::array<juce::Label, aquorbium::IntegratedModuleBank::controlCount>
        moduleBankLabels;
    std::atomic<float> densityTarget { 0.45f };
    std::atomic<float> driftTarget { 0.20f };
    std::atomic<float> ecosystemMorphTarget { 1.0f };
    std::atomic<float> cohesionTarget { 0.5f };
    std::atomic<float> dangerTarget { 0.5f };
    std::atomic<float> sparseFocusTarget { 0.25f };
    std::atomic<float> energyMemoryTarget { 0.25f };
    std::atomic<float> geometricCycleTarget { 0.25f };
    std::atomic<float> collectiveDriftTarget { 0.25f };
    std::atomic<float> geometryInfluenceTarget { 0.0f };
    std::atomic<float> rhythmDensityTarget { 0.5f };
    std::atomic<float> orbitRotationTarget { 0.5f };
    std::atomic<float> intervalBiasTarget { 0.5f };
    std::atomic<float> geometryMutationTarget { 0.0f };
    std::atomic<float> formInfluenceTarget { 0.0f };
    std::atomic<float> formContextTarget { 0.5f };
    std::atomic<float> formInertiaTarget { 0.35f };
    std::atomic<float> formMutationTarget { 0.05f };
    std::atomic<float> strikerInfluenceTarget { 0.0f };
    std::atomic<float> strikerMatterTarget { 0.5f };
    std::atomic<float> strikerPopulationTarget { 0.5f };
    std::atomic<float> strikerScatterTarget { 0.5f };
    std::atomic<float> spaceReturnTarget { 0.48f };
    std::atomic<float> spaceGestureTarget { 0.0f };
    std::atomic<float> spaceSizeTarget { 0.5f };
    std::atomic<float> spaceSpreadTarget { 1.0f };
    std::atomic<float> spaceToneTarget { 0.5f };
    std::atomic<float> spaceMemoryTarget { 0.35f };
    std::atomic<float> environmentInfluenceTarget { 0.0f };
    std::atomic<float> environmentFilterMixTarget { 0.5f };
    std::atomic<float> environmentCentreTarget { 0.5f };
    std::atomic<float> environmentSpanTarget { 0.5f };
    std::atomic<float> environmentResonanceTarget { 0.45f };
    std::atomic<float> environmentAmbienceTarget { 0.0f };
    std::atomic<float> tailInfluenceTarget { 0.0f };
    std::atomic<float> tailDecayTarget { 0.45f };
    std::atomic<float> tailDampingTarget { 0.55f };
    std::atomic<float> tailNightmareTarget { 0.0f };
    std::atomic<float> creativeMasterTarget { 0.0f };
    std::atomic<float> masterTrimTarget { 0.75f };
    std::atomic<float> masterCeilingTarget { 5.0f / 5.9f };
    std::atomic<float> masterReleaseTarget { 100.0f / 450.0f };
    std::atomic<float> masterEqOrderTarget { 0.0f };
    std::atomic<float> structureInfluenceTarget { 0.0f };
    std::atomic<float> structureMemoryTarget { 0.5f };
    std::atomic<float> structureHarmonyTarget { 0.5f };
    std::atomic<float> structurePhaseTarget { 0.5f };
    std::atomic<float> musicInfluenceTarget { 0.0f };
    std::atomic<float> musicReactionTarget { 0.6f };
    std::atomic<float> musicMemoryTarget { 0.5f };
    std::atomic<float> musicWaterTarget { 0.5f };
    std::array<std::atomic<float>, aquorbium::IntegratedModuleBank::controlCount>
        moduleBankTargets {};
    std::atomic<float> lensTargetX { 1.0f };
    std::atomic<float> lensTargetY { 0.0f };
    std::atomic<float> lensTargetZ { 0.0f };
    std::atomic<bool> lensHeldTarget { false };
    std::atomic<bool> autonomyTarget { true };
    std::atomic<std::uint32_t> audioCallbackCounter { 0 };
    std::uint32_t previousAudioCallbackCount = 0;
    int missedAudioTimerTicks = 15;
    bool audioEngineOnline = false;
    bool showEcologyControls = false;
    aquorbium::windowing::SurfaceMode surfaceMode {
        aquorbium::windowing::SurfaceMode::full
    };
    aquorbium::FixedMpscQueue<NoteEvent, 256> pendingEvents;
    aquorbium::FixedLatestSnapshot<aquorbium::app::AudioControlFrame> controlFrames;
    aquorbium::FixedLatestSnapshot<SynthFrame> synthFrames;  // the synthesis device, GUI -> audio
    SynthFrame audioSynth {};                                 // audio-owned copy
    aquorbium::app::AudioControlFrame audioControls; // audio owner only
};

class ResponsiveShell final : public juce::Component
{
public:
    explicit ResponsiveShell(juce::PropertiesFile& settings, bool startAudio = true)
        : mainComponent(settings, startAudio)
    {
        addAndMakeVisible(viewport);
        viewport.setViewedComponent(&mainComponent, false);
        viewport.setScrollBarsShown(true, true, true, false);
        viewport.setScrollBarThickness(11);
        setSize(aquorbium::windowing::WindowLayoutPolicy::preferredWidth,
                aquorbium::windowing::WindowLayoutPolicy::preferredHeight);
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(Palette::background);
    }

    MainComponent& getMainComponent() noexcept { return mainComponent; }

    void resized() override
    {
        viewport.setBounds(getLocalBounds());
        mainComponent.setSize(
            std::max(getWidth(), aquorbium::windowing::WindowLayoutPolicy::emergencyWidth),
            std::max(getHeight(), aquorbium::windowing::WindowLayoutPolicy::emergencyHeight));
    }

private:
    MainComponent mainComponent;
    juce::Viewport viewport;
};

class AquorbiumWindow final : public juce::DocumentWindow,
                              private juce::Timer
{
public:
    explicit AquorbiumWindow(juce::PropertiesFile& settingsIn, bool startAudio = true)
        : DocumentWindow("Aquorbium", Palette::background, allButtons),
          settings(settingsIn)
    {
        setUsingNativeTitleBar(true);
        setResizable(true, true);
        setResizeLimits(720, 480, 7680, 4320);
        setContentOwned(new ResponsiveShell(settings, startAudio), true);
        lastNormalBounds = resolveStartupBounds();
        setBounds(lastNormalBounds);
        setVisible(true);
        // the logo as the window's and taskbar's icon (after the peer exists)
        if (auto* peer = getPeer())
            peer->setIcon(juce::ImageCache::getFromMemory(BinaryData::aquorbium256_png, BinaryData::aquorbium256_pngSize));
        if (settings.getBoolValue("window.fullScreen", false))
            setFullScreen(true);
        startTimer(1500);
    }

    ~AquorbiumWindow() override
    {
        stopTimer();
        saveWindowState();
    }

    void closeButtonPressed() override
    {
        saveWindowState();
        juce::JUCEApplication::getInstance()->systemRequestedQuit();
    }

private:
    using PolicyBounds = aquorbium::windowing::Bounds;
    using Policy = aquorbium::windowing::WindowLayoutPolicy;

    [[nodiscard]] static PolicyBounds fromJuce(juce::Rectangle<int> bounds) noexcept
    {
        return { bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight() };
    }

    [[nodiscard]] static juce::Rectangle<int> toJuce(PolicyBounds bounds) noexcept
    {
        return { bounds.x, bounds.y, bounds.width, bounds.height };
    }

    [[nodiscard]] static PolicyBounds userBoundsFor(
        const juce::Displays::Display& display) noexcept
    {
        return fromJuce(display.userBounds.getSmallestIntegerContainer());
    }

    [[nodiscard]] juce::Rectangle<int> resolveStartupBounds() const
    {
        const auto& displays = juce::Desktop::getInstance().getDisplays();
        // window.layoutVersion 2 (2026-10-20): the wide surface. Bounds saved
        // by an older version (the 1280x800 default) are not reused once, so
        // a large screen opens on its whole work area with every control in view
        const auto hasSavedBounds = settings.getIntValue("window.layoutVersion", 1) >= 2
            && settings.containsKey("window.width")
            && settings.containsKey("window.height");
        const PolicyBounds saved {
            settings.getIntValue("window.x"),
            settings.getIntValue("window.y"),
            settings.getIntValue("window.width"),
            settings.getIntValue("window.height")
        };
        const auto savedJuce = toJuce(saved);
        const auto* display = hasSavedBounds
            ? displays.getDisplayForRect(savedJuce)
            : displays.getPrimaryDisplay();

        if (display == nullptr)
            return toJuce(hasSavedBounds ? saved
                                         : Policy::initialBounds({ 0, 0, 1280, 800 }));

        const auto userArea = userBoundsFor(*display);
        return toJuce(hasSavedBounds ? Policy::restoreBounds(saved, userArea)
                                     : Policy::initialBounds(userArea));
    }

    void timerCallback() override
    {
        if (isMinimised() || juce::Component::isMouseButtonDownAnywhere())
            return;

        const auto current = fromJuce(getBounds());
        const auto& displays = juce::Desktop::getInstance().getDisplays();
        const auto* display = displays.getDisplayForRect(getBounds());
        if (display == nullptr)
            return;

        const auto userArea = userBoundsFor(*display);
        if (!Policy::isRecoverable(current, userArea))
        {
            const auto recovered = toJuce(Policy::restoreBounds(current, userArea));
            setFullScreen(false);
            setBounds(recovered);
            lastNormalBounds = recovered;
            return;
        }

        if (!isFullScreen())
            lastNormalBounds = getBounds();
    }

    void saveWindowState()
    {
        if (!isFullScreen() && getBounds().getWidth() > 0 && getBounds().getHeight() > 0)
            lastNormalBounds = getBounds();

        settings.setValue("window.x", lastNormalBounds.getX());
        settings.setValue("window.y", lastNormalBounds.getY());
        settings.setValue("window.width", lastNormalBounds.getWidth());
        settings.setValue("window.height", lastNormalBounds.getHeight());
        settings.setValue("window.fullScreen", isFullScreen());
        settings.setValue("window.layoutVersion", 2);
        static_cast<void>(settings.saveIfNeeded());
    }

    juce::PropertiesFile& settings;
    juce::Rectangle<int> lastNormalBounds;
};

class AquorbiumApplication final : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override { return "Aquorbium"; }
    const juce::String getApplicationVersion() override { return JUCE_APPLICATION_VERSION_STRING; }

    void initialise(const juce::String& commandLine) override
    {
        juce::PropertiesFile::Options options;
        options.applicationName = "Aquorbium";
        options.filenameSuffix = ".settings";
        options.folderName = "RASGO/Aquorbium";
        constexpr auto validationPrefix = "--settings-dir=";
        juce::String auditDirectory;
        juce::String runtimeDirectory, runtimeOutput;
        juce::String captureDirectory, captureOutput;
        unsigned captureSeconds = 120;
        bool captureSecondsProvided = false;
        bool runtimeStress = false;
        for (const auto& rawArgument : juce::StringArray::fromTokens(commandLine, true))
        {
            const auto argument = rawArgument.unquoted();
            if (argument.startsWith(validationPrefix))
                options.folderName = argument.substring(
                    static_cast<int>(std::char_traits<char>::length(validationPrefix)));
            if (argument.startsWith("--layout-audit="))
                auditDirectory = argument.fromFirstOccurrenceOf("=", false, false).unquoted();
            if (argument.startsWith("--runtime-audit="))
                runtimeDirectory = argument.fromFirstOccurrenceOf("=", false, false).unquoted();
            if (argument.startsWith("--runtime-output="))
                runtimeOutput = argument.fromFirstOccurrenceOf("=", false, false).unquoted();
            if (argument == "--runtime-stress")
                runtimeStress = true;
            if (argument.startsWith("--capture-audio="))
                captureDirectory = argument.fromFirstOccurrenceOf("=", false, false).unquoted();
            if (argument.startsWith("--capture-output="))
                captureOutput = argument.fromFirstOccurrenceOf("=", false, false).unquoted();
            if (argument.startsWith("--capture-seconds="))
            {
                const auto value = argument.fromFirstOccurrenceOf("=", false, false).unquoted();
                captureSecondsProvided = true;
                if (value.isEmpty() || !value.containsOnly("0123456789"))
                { setApplicationReturnValue(1); quit(); return; }
                captureSeconds = static_cast<unsigned>(value.getIntValue());
            }
        }
        if (runtimeStress && runtimeDirectory.isEmpty())
        { setApplicationReturnValue(1); quit(); return; }
        if ((captureOutput.isNotEmpty() || captureSecondsProvided) && captureDirectory.isEmpty())
        { setApplicationReturnValue(1); quit(); return; }
        if (captureDirectory.isNotEmpty())
        {
            const juce::File directory(captureDirectory);
            if (auditDirectory.isNotEmpty() || runtimeDirectory.isNotEmpty()
                || captureOutput.isEmpty() || captureSeconds == 0
                || captureSeconds > aquorbium::app::DiagnosticAudioCapture::maximumSeconds
                || !juce::File::isAbsolutePath(captureDirectory)
                || directory.exists() || directory.createDirectory().failed())
            { setApplicationReturnValue(1); quit(); return; }
            options.folderName = directory.getChildFile("settings").getFullPathName();
        }
        if (runtimeDirectory.isNotEmpty())
        {
            const juce::File directory(runtimeDirectory);
            if (auditDirectory.isNotEmpty() || !juce::File::isAbsolutePath(runtimeDirectory)
                || directory.exists() || directory.createDirectory().failed())
            { setApplicationReturnValue(1); quit(); return; }
            options.folderName = directory.getChildFile("settings").getFullPathName();
        }
        if (auditDirectory.isNotEmpty())
            options.folderName = juce::File(auditDirectory).getChildFile("settings").getFullPathName();
        options.osxLibrarySubFolder = "Application Support";
        options.millisecondsBeforeSaving = 2000;
        applicationProperties.setStorageParameters(options);
        if (auditDirectory.isNotEmpty())
        {
            const juce::File directory(auditDirectory);
            if (directory.createDirectory().failed()) { setApplicationReturnValue(1); quit(); return; }
            auto audit = std::make_unique<MainComponent>(*applicationProperties.getUserSettings(), false);
            const auto passed = audit->auditLayout(directory);
            audit.reset();
            setApplicationReturnValue(passed ? 0 : 1);
            quit();
            return;
        }
        window = std::make_unique<AquorbiumWindow>(
            *applicationProperties.getUserSettings(),
            runtimeDirectory.isEmpty() && captureDirectory.isEmpty());
        if (runtimeDirectory.isNotEmpty())
        {
            auto* shell = dynamic_cast<ResponsiveShell*>(window->getContentComponent());
            if (shell == nullptr || !shell->getMainComponent().startRuntimeAudit(
                    juce::File(runtimeDirectory), runtimeOutput, runtimeStress))
            { setApplicationReturnValue(1); quit(); }
        }
        if (captureDirectory.isNotEmpty())
        {
            auto* shell = dynamic_cast<ResponsiveShell*>(window->getContentComponent());
            if (shell == nullptr || !shell->getMainComponent().startInteractiveCapture(
                    juce::File(captureDirectory), captureOutput, captureSeconds))
            { setApplicationReturnValue(1); quit(); }
        }
    }

    void shutdown() override
    {
        window.reset();
        static_cast<void>(applicationProperties.saveIfNeeded());
        applicationProperties.closeFiles();
    }

private:
    juce::ApplicationProperties applicationProperties;
    std::unique_ptr<AquorbiumWindow> window;
};

START_JUCE_APPLICATION(AquorbiumApplication)
