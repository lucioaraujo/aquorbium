// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

// The synthesis device's editor (SYNTH_DEVICE_DESIGN.md, phase 4;
// 2026-10-21). An overlay, as TUTORIAL and ABOUT (the author's choice), with
// an organism selector, the SYNTHESIS mix, the voice's sections and the
// modulation matrix. Included by Main.cpp after Palette, uiFont and the
// language helpers; it owns no audio state - it edits the five patches and
// the mix, and tells its owner when they change (the owner publishes them to
// the audio thread and keeps them in the settings).

#include "core/SynthProposal.h"
#include "core/SynthVoice.h"

#include <array>
#include <functional>
#include <memory>
#include <vector>

class SynthEditor final : public juce::Component
{
public:
    using Patch = aquorbium::SynthVoice::Patch;
    static constexpr std::size_t organisms = 5;
    // below this the knobs shrink to dots: the owner scrolls the editor
    // inside a viewport instead (2026-10-22)
    static constexpr int minimumWidth = 1280, minimumHeight = 800;

    std::function<void()> onClose;
    std::function<void()> onChange;

    std::array<Patch, organisms> patches {};
    float mix = 0.0f;
    // the session's seed (Main sets it): PROPOSTA's first draw follows it
    std::uint32_t seed = 1u;

    SynthEditor()
    {
        using namespace aquorbium::app;
        heading.setFont(uiFont(20.0f, juce::Font::bold));
        heading.setColour(juce::Label::textColourId, Palette::cyan);
        addAndMakeVisible(heading);
        intro.setFont(uiFont(12.0f));
        intro.setColour(juce::Label::textColourId, Palette::muted);
        intro.setJustificationType(juce::Justification::topLeft);
        addAndMakeVisible(intro);
        close.onClick = [this] {
            setVisible(false);
            if (onClose)
                onClose();
        };
        addAndMakeVisible(close);
        copyAll.onClick = [this] {
            for (std::size_t i = 0; i < organisms; ++i)
                patches[i] = patches[current];
            changed();
        };
        addAndMakeVisible(copyAll);
        // PROPOSTA (phase 5): this organism, or all five
        propose.onClick = [this] {
            patches[current] = aquorbium::SynthProposal::propose(current, nextDraw());
            proposed();
        };
        addAndMakeVisible(propose);
        proposeAll.onClick = [this] {
            const auto draw = nextDraw();
            for (std::size_t i = 0; i < organisms; ++i)
                patches[i] = aquorbium::SynthProposal::propose(i, draw);
            proposed();
        };
        addAndMakeVisible(proposeAll);

        static const std::array<juce::Colour, organisms> colours {
            Palette::cyan, Palette::amber, Palette::coral, Palette::violet, Palette::green };
        static const std::array<const char*, organisms> names { "MORPH", "GRAIN", "MODAL", "PULSAR", "STRING" };
        for (std::size_t i = 0; i < organisms; ++i)
        {
            auto& b = organismButtons[i];
            b.setButtonText(names[i]);
            b.setClickingTogglesState(true);
            b.setRadioGroupId(4711);
            b.setColour(juce::TextButton::buttonOnColourId, colours[i].withAlpha(0.55f));
            b.onClick = [this, i] {
                current = i;
                loadControls();
            };
            addAndMakeVisible(b);
        }
        organismButtons[0].setToggleState(true, juce::dontSendNotification);

        mixSlider.setSliderStyle(juce::Slider::LinearBar);
        mixSlider.setRange(0.0, 1.0, 0.0);
        mixSlider.setNumDecimalPlacesToDisplay(2);
        mixSlider.onValueChange = [this] {
            mix = static_cast<float>(mixSlider.getValue());
            changed();
        };
        addAndMakeVisible(mixSlider);
        mixLabel.setFont(uiFont(11.0f, juce::Font::bold));
        mixLabel.setColour(juce::Label::textColourId, Palette::text);
        addAndMakeVisible(mixLabel);

        buildSections();
        refreshTexts();
        loadControls();
    }

    // choose the organism being edited (the buttons, and the layout audit)
    void selectOrganism(std::size_t organism)
    {
        current = std::min(organism, organisms - 1);
        organismButtons[current].setToggleState(true, juce::dontSendNotification);
        loadControls();
    }

    void show()
    {
        loadControls();
        setVisible(true);
        toFront(true);
    }

    // the owner changed the patches or the mix (settings, scenes)
    void loadControls()
    {
        loading = true;
        mixSlider.setValue(mix, juce::dontSendNotification);
        auto& p = patches[current];
        for (auto& knob : knobs)
            knob->slider.setValue(knob->field(p), juce::dontSendNotification);
        for (auto& choice : choices)
            choice->box.setSelectedId(choice->get(p) + 1 - choice->offset, juce::dontSendNotification);
        for (std::size_t r = 0; r < aquorbium::SynthVoice::routeCount; ++r)
        {
            routeRows[r].source.setSelectedId(static_cast<int>(p.routes[r].source) + 1, juce::dontSendNotification);
            routeRows[r].destination.setSelectedId(static_cast<int>(p.routes[r].destination) + 1, juce::dontSendNotification);
            routeRows[r].amount.setValue(p.routes[r].amount, juce::dontSendNotification);
        }
        loading = false;
        updateMacroLabels();
    }

    // the layout audit (Main's --layout-audit, 2026-10-22): every control
    // inside the card with a usable size, the top buttons apart from each
    // other and from the texts, and every button's text fitting its width
    [[nodiscard]] juce::String auditLayout() const
    {
        juce::String problems;
        for (auto* child : getChildren())
        {
            if (!child->isVisible())
                continue;
            const auto b = child->getBounds();
            if (b.isEmpty() || !card.contains(b))
                problems << "outside the card or empty: " << child->getName() << " " << b.toString() << "\n";
        }
        const std::array<const juce::Component*, 6> row { &heading, &mixSlider, &intro, &propose, &proposeAll, &copyAll };
        for (std::size_t i = 0; i < row.size(); ++i)
            for (std::size_t j = i + 1; j < row.size(); ++j)
                if (row[i]->getBounds().intersects(row[j]->getBounds()))
                    problems << "overlap " << juce::String(static_cast<int>(i)) << "/" << juce::String(static_cast<int>(j)) << "\n";
        for (const auto* button : { &close, &copyAll, &propose, &proposeAll })
        {
            const auto font = button->getLookAndFeel().getTextButtonFont(const_cast<juce::TextButton&>(*button), button->getHeight());
            const auto width = juce::GlyphArrangement::getStringWidth(font, button->getButtonText());
            if (width > static_cast<float>(button->getWidth() - 8))
                problems << "text too wide: " << button->getButtonText() << "\n";
        }
        return problems;
    }

    void refreshTexts()
    {
        using namespace aquorbium::app;
        namespace Y = S::synth;
        heading.setText(T(Y::title), juce::dontSendNotification);
        intro.setText(T(Y::intro), juce::dontSendNotification);
        close.setButtonText(T(S::close));
        copyAll.setButtonText(T(Y::copyAll));
        propose.setButtonText(T(Y::propose));
        propose.setTooltip(T(Y::proposeTip));
        proposeAll.setButtonText(T(Y::proposeAll));
        proposeAll.setTooltip(T(Y::proposeAllTip));
        mixLabel.setText(T(Y::title), juce::dontSendNotification);
        mixSlider.setTooltip(T(Y::mixTip));
        for (auto& section : sections)
        {
            section->title.setText(T(*section->name), juce::dontSendNotification);
            // what the section does (phase 6): the sources explain themselves
            // on their choice box instead
            const auto* n = section->name;
            const auto* tip = n == &Y::mixSection ? &Y::mixSectionTip
                            : n == &Y::filter ? &Y::filterTip
                            : (n == &Y::ampEnvelope || n == &Y::modEnvelope) ? &Y::envelopeTip
                            : (n == &Y::lfo1 || n == &Y::lfo2) ? &Y::lfoTip : nullptr;
            section->title.setTooltip(tip != nullptr ? T(*tip) : juce::String());
        }
        matrixTitle.setTooltip(T(Y::matrixTip));
        for (auto& knob : knobs)
            knob->label.setText(T(*knob->name), juce::dontSendNotification);
        for (auto& choice : choices)
        {
            const auto selected = choice->box.getSelectedId();
            choice->box.clear(juce::dontSendNotification);
            for (std::size_t k = 0; k < choice->count; ++k)
                choice->box.addItem(T(choice->items[k]), static_cast<int>(k) + 1);
            choice->box.setSelectedId(selected, juce::dontSendNotification);
        }
        for (auto& row : routeRows)
        {
            const auto source = row.source.getSelectedId(), destination = row.destination.getSelectedId();
            row.source.clear(juce::dontSendNotification);
            row.destination.clear(juce::dontSendNotification);
            for (std::size_t k = 0; k < Y::modSources.size(); ++k)
                row.source.addItem(T(Y::modSources[k]), static_cast<int>(k) + 1);
            for (std::size_t k = 0; k < Y::modDestinations.size(); ++k)
                row.destination.addItem(T(Y::modDestinations[k]), static_cast<int>(k) + 1);
            row.source.setSelectedId(std::max(1, source), juce::dontSendNotification);
            row.destination.setSelectedId(std::max(1, destination), juce::dontSendNotification);
        }
        matrixTitle.setText(T(Y::matrix), juce::dontSendNotification);
        updateMacroLabels();
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(Palette::background.withAlpha(0.86f));
        g.setColour(Palette::panelRaised);
        g.fillRoundedRectangle(card.toFloat(), 14.0f);
        g.setColour(Palette::cyan.withAlpha(0.45f));
        g.drawRoundedRectangle(card.toFloat(), 14.0f, 1.3f);
        g.setColour(Palette::panel);
        for (auto& section : sections)
            g.fillRoundedRectangle(section->area.toFloat(), 8.0f);
        g.fillRoundedRectangle(matrixArea.toFloat(), 8.0f);
    }

    void resized() override
    {
        card = getLocalBounds().withSizeKeepingCentre(std::min(1320, getWidth() - 24), std::min(820, getHeight() - 24));
        auto area = card.reduced(18, 14);
        auto top = area.removeFromTop(32);
        close.setBounds(top.removeFromRight(110).reduced(0, 2));
        top.removeFromRight(8);
        copyAll.setBounds(top.removeFromRight(170).reduced(0, 2));
        heading.setBounds(top.removeFromLeft(150));
        for (auto& b : organismButtons)
        {
            b.setBounds(top.removeFromLeft(86).reduced(2, 2));
        }
        top.removeFromLeft(12);
        mixLabel.setBounds(top.removeFromLeft(80));
        mixSlider.setBounds(top.reduced(0, 6));
        auto introRow = area.removeFromTop(28);
        proposeAll.setBounds(introRow.removeFromRight(170).reduced(0, 2));
        introRow.removeFromRight(8);
        propose.setBounds(introRow.removeFromRight(130).reduced(0, 2));
        introRow.removeFromRight(12);
        intro.setBounds(introRow);
        area.removeFromTop(4);

        // two rows of sections, then the matrix
        auto matrixBlock = area.removeFromBottom(std::min(196, area.getHeight() / 3));
        const auto rowHeight = area.getHeight() / 2;
        auto first = area.removeFromTop(rowHeight).reduced(0, 3);
        auto second = area.reduced(0, 3);
        // first row: source A, source B, mix
        layoutRow(first, { 0, 1, 2 }, { 3.0f, 3.0f, 2.2f });
        // second row: filter, amp envelope, mod envelope, LFO 1, LFO 2
        layoutRow(second, { 3, 4, 5, 6, 7 }, { 2.4f, 2.0f, 2.0f, 1.3f, 1.3f });

        matrixArea = matrixBlock.reduced(0, 3);
        auto m = matrixArea.reduced(10, 6);
        matrixTitle.setBounds(m.removeFromTop(20));
        const auto half = m.getWidth() / 2;
        for (std::size_t r = 0; r < routeRows.size(); ++r)
        {
            const auto column = static_cast<int>(r / 4);
            const auto rowIndex = static_cast<int>(r % 4);
            auto row = juce::Rectangle<int>(m.getX() + column * half, m.getY() + rowIndex * (m.getHeight() / 4),
                                            half - 12, m.getHeight() / 4).reduced(0, 3);
            routeRows[r].source.setBounds(row.removeFromLeft(row.getWidth() * 2 / 5).reduced(2, 0));
            routeRows[r].destination.setBounds(row.removeFromLeft(row.getWidth() / 2).reduced(2, 0));
            routeRows[r].amount.setBounds(row.reduced(2, 2));
        }
    }

private:
    struct Knob
    {
        juce::Slider slider;
        juce::Label label;
        std::function<float&(Patch&)> field;
        const aquorbium::app::L4* name = nullptr;
    };
    struct Choice
    {
        juce::ComboBox box;
        std::function<int(const Patch&)> get;
        std::function<void(Patch&, int)> set;
        const aquorbium::app::L4* items = nullptr;  // the item names
        std::size_t count = 0;
        int offset = 0;                              // value = item index + offset
        bool sharesRow = false;                      // drawn on the previous choice's row (one of them shows)
    };
    struct Section
    {
        juce::Label title;
        juce::Rectangle<int> area;
        std::vector<Choice*> choices;
        std::vector<Knob*> knobs;
        const aquorbium::app::L4* name = nullptr;
    };
    struct RouteRow
    {
        juce::ComboBox source, destination;
        juce::Slider amount;
    };

    void changed()
    {
        if (!loading && onChange)
            onChange();
    }

    Knob& knob(Section& section, const aquorbium::app::L4& name, double lo, double hi, double centre,
               std::function<float&(Patch&)> field, const char* suffix = "")
    {
        auto k = std::make_unique<Knob>();
        k->name = &name;
        k->field = std::move(field);
        k->slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        k->slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 64, 15);
        k->slider.setRange(lo, hi, 0.0);
        if (centre > lo && centre < hi)
            k->slider.setSkewFactorFromMidPoint(centre);
        k->slider.setTextValueSuffix(suffix);
        k->slider.setNumDecimalPlacesToDisplay(hi - lo > 50.0 ? 0 : 2);
        auto* raw = k.get();
        k->slider.onValueChange = [this, raw] {
            if (loading)
                return;
            raw->field(patches[current]) = static_cast<float>(raw->slider.getValue());
            changed();
        };
        k->label.setFont(uiFont(10.0f, juce::Font::bold));
        k->label.setColour(juce::Label::textColourId, Palette::muted);
        k->label.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(k->slider);
        addAndMakeVisible(k->label);
        section.knobs.push_back(raw);
        knobs.push_back(std::move(k));
        return *raw;
    }

    template <std::size_t N>
    Choice& choice(Section& section, const std::array<aquorbium::app::L4, N>& items,
                   std::function<int(const Patch&)> get, std::function<void(Patch&, int)> set, int offset = 0,
                   std::size_t skip = 0)
    {
        auto c = std::make_unique<Choice>();
        c->items = items.data() + skip;
        c->count = N - skip;
        c->offset = offset;
        c->get = std::move(get);
        c->set = std::move(set);
        auto* raw = c.get();
        c->box.onChange = [this, raw] {
            if (loading)
                return;
            raw->set(patches[current], raw->box.getSelectedId() - 1 + raw->offset);
            updateMacroLabels();
            changed();
        };
        addAndMakeVisible(c->box);
        section.choices.push_back(raw);
        choices.push_back(std::move(c));
        return *raw;
    }

    Section& section(const aquorbium::app::L4& name)
    {
        auto s = std::make_unique<Section>();
        s->name = &name;
        s->title.setFont(uiFont(11.0f, juce::Font::bold));
        s->title.setColour(juce::Label::textColourId, Palette::cyan);
        addAndMakeVisible(s->title);
        auto* raw = s.get();
        sections.push_back(std::move(s));
        return *raw;
    }

    void buildSections()
    {
        using namespace aquorbium::app;
        namespace Y = S::synth;
        using Src = aquorbium::SynthVoice::Source;
        using Mode = aquorbium::SynthVoice::FilterMode;
        using Shape = aquorbium::SynthVoice::LfoShape;
        for (int side = 0; side < 2; ++side)
        {
            auto& s = section(side == 0 ? Y::sourceA : Y::sourceB);
            if (side == 0)
            {
                sourceChoice[0] = &choice(s, Y::sources, [](const Patch& p) { return static_cast<int>(p.sourceA); },
                       [](Patch& p, int v) { p.sourceA = static_cast<Src>(std::clamp(v, 0, static_cast<int>(aquorbium::SynthVoice::sourceCount) - 1)); });
                // which body, for the BODY source (the bodies list without "native")
                bodyChoice[0] = &choice(s, S::bodies, [](const Patch& p) { return static_cast<int>(p.bodyA); },
                       [](Patch& p, int v) { p.bodyA = static_cast<std::uint8_t>(std::clamp(v, 1, 11)); }, 1, 1);
                // FM 4-OP's algorithm, on the same row (the body list or this one shows)
                algorithmChoice[0] = &choice(s, Y::algorithms, [](const Patch& p) { return static_cast<int>(p.algorithmA); },
                       [](Patch& p, int v) { p.algorithmA = static_cast<std::uint8_t>(std::clamp(v, 0, 7)); });
                algorithmChoice[0]->sharesRow = true;
                macro[0][0] = &knob(s, Y::shape, 0, 1, -1, [](Patch& p) -> float& { return p.shapeA; });
                macro[0][1] = &knob(s, Y::width, 0, 1, -1, [](Patch& p) -> float& { return p.widthA; });
                knob(s, Y::tune, -24, 24, -1, [](Patch& p) -> float& { return p.tuneA; }, " st");
                knob(s, Y::fine, -50, 50, -1, [](Patch& p) -> float& { return p.fineA; }, " c");
                knob(s, Y::sub, 0, 1, -1, [](Patch& p) -> float& { return p.subA; });
                macro[0][2] = &knob(s, Y::colour, 0, 1, -1, [](Patch& p) -> float& { return p.colourA; });
                knob(s, Y::level, 0, 1, -1, [](Patch& p) -> float& { return p.levelA; });
                ratioKnob[0][0] = &knob(s, Y::ratioC, 0, 1, -1, [](Patch& p) -> float& { return p.ratioCA; });
                ratioKnob[0][1] = &knob(s, Y::ratioD, 0, 1, -1, [](Patch& p) -> float& { return p.ratioDA; });
            }
            else
            {
                sourceChoice[1] = &choice(s, Y::sources, [](const Patch& p) { return static_cast<int>(p.sourceB); },
                       [](Patch& p, int v) { p.sourceB = static_cast<Src>(std::clamp(v, 0, static_cast<int>(aquorbium::SynthVoice::sourceCount) - 1)); });
                // which body, for the BODY source (the bodies list without "native")
                bodyChoice[1] = &choice(s, S::bodies, [](const Patch& p) { return static_cast<int>(p.bodyB); },
                       [](Patch& p, int v) { p.bodyB = static_cast<std::uint8_t>(std::clamp(v, 1, 11)); }, 1, 1);
                // FM 4-OP's algorithm, on the same row (the body list or this one shows)
                algorithmChoice[1] = &choice(s, Y::algorithms, [](const Patch& p) { return static_cast<int>(p.algorithmB); },
                       [](Patch& p, int v) { p.algorithmB = static_cast<std::uint8_t>(std::clamp(v, 0, 7)); });
                algorithmChoice[1]->sharesRow = true;
                macro[1][0] = &knob(s, Y::shape, 0, 1, -1, [](Patch& p) -> float& { return p.shapeB; });
                macro[1][1] = &knob(s, Y::width, 0, 1, -1, [](Patch& p) -> float& { return p.widthB; });
                knob(s, Y::tune, -24, 24, -1, [](Patch& p) -> float& { return p.tuneB; }, " st");
                knob(s, Y::fine, -50, 50, -1, [](Patch& p) -> float& { return p.fineB; }, " c");
                knob(s, Y::sub, 0, 1, -1, [](Patch& p) -> float& { return p.subB; });
                macro[1][2] = &knob(s, Y::colour, 0, 1, -1, [](Patch& p) -> float& { return p.colourB; });
                knob(s, Y::level, 0, 1, -1, [](Patch& p) -> float& { return p.levelB; });
                ratioKnob[1][0] = &knob(s, Y::ratioC, 0, 1, -1, [](Patch& p) -> float& { return p.ratioCB; });
                ratioKnob[1][1] = &knob(s, Y::ratioD, 0, 1, -1, [](Patch& p) -> float& { return p.ratioDB; });
            }
        }
        {
            auto& s = section(Y::mixSection);
            knob(s, Y::ring, 0, 1, -1, [](Patch& p) -> float& { return p.ring; });
            knob(s, Y::fm, 0, 1, -1, [](Patch& p) -> float& { return p.fm; });
            knob(s, Y::drift, 0, 1, -1, [](Patch& p) -> float& { return p.drift; });
            knob(s, Y::accent, 0, 1, -1, [](Patch& p) -> float& { return p.accentToLevel; });
            knob(s, Y::level, 0, 1.5, -1, [](Patch& p) -> float& { return p.level; });
        }
        {
            auto& s = section(Y::filter);
            choice(s, Y::filterModes, [](const Patch& p) { return static_cast<int>(p.filterMode); },
                   [](Patch& p, int v) { p.filterMode = static_cast<Mode>(std::clamp(v, 0, static_cast<int>(aquorbium::SynthVoice::filterModeCount) - 1)); });
            cutoffKnob = &knob(s, Y::cutoff, 20, 20000, 800, [](Patch& p) -> float& { return p.cutoff; }, " Hz");
            knob(s, Y::resonance, 0, 1, -1, [](Patch& p) -> float& { return p.resonance; });
            knob(s, Y::keytrack, 0, 1, -1, [](Patch& p) -> float& { return p.keytrack; });
            knob(s, Y::envelope, -4, 4, -1, [](Patch& p) -> float& { return p.filterEnv; }, " oct");
            knob(s, Y::drive, 0, 1, -1, [](Patch& p) -> float& { return p.drive; });
            // FORMANT (2026-10-22): the vowel, A 0 .. U 1
            vowelKnob = &knob(s, Y::vowel, 0, 1, -1, [](Patch& p) -> float& { return p.vowel; });
        }
        for (int which = 0; which < 2; ++which)
        {
            auto& s = section(which == 0 ? Y::ampEnvelope : Y::modEnvelope);
            const auto pick = [which](Patch& p) -> aquorbium::SynthVoice::Envelope& { return which == 0 ? p.amp : p.mod; };
            knob(s, Y::attack, 0.001, 5, 0.2, [pick](Patch& p) -> float& { return pick(p).attack; }, " s");
            knob(s, Y::decay, 0.001, 5, 0.4, [pick](Patch& p) -> float& { return pick(p).decay; }, " s");
            knob(s, Y::sustain, 0, 1, -1, [pick](Patch& p) -> float& { return pick(p).sustain; });
            knob(s, Y::release, 0.001, 8, 0.5, [pick](Patch& p) -> float& { return pick(p).release; }, " s");
            knob(s, Y::curve, 0, 1, -1, [pick](Patch& p) -> float& { return pick(p).curve; });
        }
        {
            auto& s = section(Y::lfo1);
            choice(s, Y::lfoShapes, [](const Patch& p) { return static_cast<int>(p.lfo1.shape); },
                   [](Patch& p, int v) { p.lfo1.shape = static_cast<Shape>(std::clamp(v, 0, 3)); });
            knob(s, Y::rate, 0.01, 20, 2, [](Patch& p) -> float& { return p.lfo1.rate; }, " Hz");
            knob(s, Y::vibrato, 0, 100, -1, [](Patch& p) -> float& { return p.vibrato; }, " c");
        }
        {
            auto& s = section(Y::lfo2);
            choice(s, Y::lfoShapes, [](const Patch& p) { return static_cast<int>(p.lfo2.shape); },
                   [](Patch& p, int v) { p.lfo2.shape = static_cast<Shape>(std::clamp(v, 0, 3)); });
            knob(s, Y::rate, 0.01, 20, 2, [](Patch& p) -> float& { return p.lfo2.rate; }, " Hz");
            knob(s, Y::wah, 0, 4, -1, [](Patch& p) -> float& { return p.wah; }, " oct");
        }
        matrixTitle.setFont(uiFont(11.0f, juce::Font::bold));
        matrixTitle.setColour(juce::Label::textColourId, Palette::cyan);
        addAndMakeVisible(matrixTitle);
        for (std::size_t r = 0; r < routeRows.size(); ++r)
        {
            auto& row = routeRows[r];
            const auto update = [this, r] {
                if (loading)
                    return;
                auto& route = patches[current].routes[r];
                route.source = static_cast<aquorbium::SynthVoice::ModSource>(std::max(0, routeRows[r].source.getSelectedId() - 1));
                route.destination = static_cast<aquorbium::SynthVoice::ModDestination>(
                    std::max(0, routeRows[r].destination.getSelectedId() - 1));
                route.amount = static_cast<float>(routeRows[r].amount.getValue());
                updateRouteTips();
                changed();
            };
            row.source.onChange = update;
            row.destination.onChange = update;
            row.amount.setSliderStyle(juce::Slider::LinearBar);
            row.amount.setRange(-1.0, 1.0, 0.0);
            row.amount.setNumDecimalPlacesToDisplay(2);
            row.amount.onValueChange = update;
            addAndMakeVisible(row.source);
            addAndMakeVisible(row.destination);
            addAndMakeVisible(row.amount);
        }
    }

    void layoutRow(juce::Rectangle<int> row, std::initializer_list<int> indices, std::initializer_list<float> weights)
    {
        float total = 0.0f;
        for (const auto w : weights)
            total += w;
        auto weight = weights.begin();
        const auto fullWidth = row.getWidth();
        for (const auto index : indices)
        {
            auto& s = *sections[static_cast<std::size_t>(index)];
            const auto width = static_cast<int>(static_cast<float>(fullWidth) * *weight / total);
            ++weight;
            s.area = row.removeFromLeft(width).reduced(3, 0);
            auto inner = s.area.reduced(8, 6);
            s.title.setBounds(inner.removeFromTop(18));
            juce::Rectangle<int> previous;
            for (auto* c : s.choices)
            {
                if (!c->sharesRow)
                    previous = inner.removeFromTop(24).reduced(0, 1);
                c->box.setBounds(previous);
            }
            inner.removeFromTop(2);
            // the knobs in a grid: as many columns as fit at ~64 px
            const auto columns = std::max(1, inner.getWidth() / 64);
            const auto count = static_cast<int>(s.knobs.size());
            const auto rows = std::max(1, (count + columns - 1) / columns);
            const auto cellW = inner.getWidth() / columns;
            const auto cellH = inner.getHeight() / rows;
            for (int k = 0; k < count; ++k)
            {
                auto cell = juce::Rectangle<int>(inner.getX() + (k % columns) * cellW, inner.getY() + (k / columns) * cellH,
                                                 cellW, cellH).reduced(2, 1);
                s.knobs[static_cast<std::size_t>(k)]->label.setBounds(cell.removeFromTop(14));
                s.knobs[static_cast<std::size_t>(k)]->slider.setBounds(cell);
            }
        }
    }

    // FORMA / LARGURA / COR take the names of what they do for each source
    void updateMacroLabels()
    {
        using namespace aquorbium::app;
        const auto& p = patches[current];
        const std::array<int, 2> source { static_cast<int>(p.sourceA), static_cast<int>(p.sourceB) };
        for (std::size_t side = 0; side < 2; ++side)
        {
            const auto& names = S::synth::macroNames[static_cast<std::size_t>(
                std::clamp(source[side], 0, static_cast<int>(S::synth::macroNames.size()) - 1))];
            for (std::size_t m = 0; m < 3; ++m)
                if (macro[side][m] != nullptr)
                {
                    macro[side][m]->label.setText(T(names[m]), juce::dontSendNotification);
                    macro[side][m]->slider.setEnabled(T(names[m]) != "-");
                }
            // the body list matters only for the BODY source
            if (bodyChoice[side] != nullptr)
                bodyChoice[side]->box.setEnabled(source[side] == static_cast<int>(aquorbium::SynthVoice::Source::body));
            // FM 4-OP: its algorithm in the body list's row, its ratios C/D
            const auto fm4 = source[side] == static_cast<int>(aquorbium::SynthVoice::Source::fm4);
            // the ratios read as ratios (2.0x), FORMA too when it is FM's ratio B
            const auto fmRatio = fm4 || source[side] == static_cast<int>(aquorbium::SynthVoice::Source::fm);
            if (macro[side][0] != nullptr)
                showAsRatio(macro[side][0]->slider, fmRatio);
            for (auto* r : ratioKnob[side])
                if (r != nullptr)
                    showAsRatio(r->slider, true);
            if (bodyChoice[side] != nullptr && algorithmChoice[side] != nullptr)
            {
                bodyChoice[side]->box.setVisible(!fm4);
                algorithmChoice[side]->box.setVisible(fm4);
            }
            for (auto* r : ratioKnob[side])
                if (r != nullptr)
                    r->slider.setEnabled(fm4);
            // the source explains itself (phase 6)
            if (sourceChoice[side] != nullptr)
                sourceChoice[side]->box.setTooltip(T(S::synth::sourceTips[static_cast<std::size_t>(
                    std::clamp(source[side], 0, static_cast<int>(S::synth::sourceTips.size()) - 1))]));
        }
        // FORMANT: CUTOFF is the vocal tract, and VOWEL only acts there
        const auto formantMode = p.filterMode == aquorbium::SynthVoice::FilterMode::formant;
        if (cutoffKnob != nullptr)
            cutoffKnob->label.setText(T(formantMode ? S::synth::tract : S::synth::cutoff), juce::dontSendNotification);
        if (vowelKnob != nullptr)
            vowelKnob->slider.setEnabled(formantMode);
        updateRouteTips();
    }

    // each route's source box says what that signal is
    void updateRouteTips()
    {
        using namespace aquorbium::app;
        const auto& p = patches[current];
        for (std::size_t r = 0; r < aquorbium::SynthVoice::routeCount; ++r)
        {
            const auto i = std::min(static_cast<std::size_t>(p.routes[r].source), S::synth::modSourceTips.size() - 1);
            routeRows[r].source.setTooltip(T(S::synth::modSourceTips[i]));
            routeRows[r].destination.setTooltip(T(S::synth::matrixTip));
            routeRows[r].amount.setTooltip(T(S::synth::matrixTip));
        }
    }

    std::array<std::array<Knob*, 3>, 2> macro {};
    std::array<Choice*, 2> bodyChoice {}, sourceChoice {};
    // a 0..1 macro shown as the musical ratio it selects (0.5x .. 9x)
    static void showAsRatio(juce::Slider& slider, bool asRatio)
    {
        if (asRatio)
        {
            slider.textFromValueFunction = [](double v) {
                return juce::String(aquorbium::Fm4Source::ratioOf(static_cast<float>(v)), 1) + "x";
            };
            slider.valueFromTextFunction = [](const juce::String& text) {
                // the nearest step of the table
                const auto wanted = text.retainCharacters("0123456789.").getDoubleValue();
                double best = 0.0, distance = 1.0e9;
                for (int i = 0; i <= 9; ++i)
                {
                    const auto r = static_cast<double>(aquorbium::Fm4Source::ratioOf(static_cast<float>(i) / 9.0f));
                    if (std::abs(r - wanted) < distance) { distance = std::abs(r - wanted); best = i / 9.0; }
                }
                return best;
            };
        }
        else
        {
            slider.textFromValueFunction = nullptr;
            slider.valueFromTextFunction = nullptr;
        }
        slider.updateText();
    }

    std::array<Choice*, 2> algorithmChoice {};
    std::array<std::array<Knob*, 2>, 2> ratioKnob {};
    Knob* cutoffKnob = nullptr;
    Knob* vowelKnob = nullptr;
    std::size_t current = 0;
    bool loading = false;
    juce::Rectangle<int> card, matrixArea;
    juce::Label heading, intro, mixLabel, matrixTitle;
    juce::TextButton close, copyAll, propose, proposeAll;
    std::uint32_t draws = 0;
    // the n-th draw of the session: the first is the session's seed itself
    std::uint32_t nextDraw() noexcept { return seed ^ (0x9e3779b9u * draws++); }
    // a proposal that cannot be heard is no proposal: with SYNTHESIS at
    // zero, it is raised to the middle
    void proposed()
    {
        if (mix < 0.01f)
            mix = 0.5f;
        loadControls();
        changed();
    }
    std::array<juce::TextButton, organisms> organismButtons;
    juce::Slider mixSlider;
    std::vector<std::unique_ptr<Section>> sections;
    std::vector<std::unique_ptr<Knob>> knobs;
    std::vector<std::unique_ptr<Choice>> choices;
    std::array<RouteRow, aquorbium::SynthVoice::routeCount> routeRows;
};
