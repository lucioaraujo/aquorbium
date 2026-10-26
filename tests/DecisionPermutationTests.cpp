// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/BiomaRelabi.h"
#include "core/BranchingStepRouter.h"
#include "core/NotePermutationMatrix.h"

#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <utility>

namespace
{
using Matrix = aquorbium::NotePermutationMatrix;
using Router = aquorbium::BranchingStepRouter;

bool branchingTreeSelectsTwoLayers()
{
    static_assert(noexcept(std::declval<Router&>().select(0.0f, 0.0f)));
    Router router;
    router.setShape(3, 4);
    const auto first = router.select(0.34f, 0.76f);
    const auto held = router.select(0.34f, 0.76f);
    const auto next = router.select(0.90f, 0.10f);
    return router.leaves() == 12
        && first.category == 1 && first.variant == 3 && first.index == 7
        && first.changed && first.transitionCount == 1
        && !held.changed && held.transitionCount == 1
        && next.category == 2 && next.variant == 0 && next.index == 8
        && next.changed && next.transitionCount == 2;
}

bool branchingThresholdsAreIndependentAndBounded()
{
    Router router;
    router.setShape(100, 100);
    if (router.categories() != Router::maximumCategories
        || router.variants() != Router::maximumVariants
        || router.leaves() != Router::maximumLeaves)
        return false;

    std::array<float, aquorbium::ThresholdStepSelector::maximumSteps> broad {};
    broad[0] = 0.10f;
    broad[1] = 0.90f;
    broad[2] = 1.0f;
    std::array<float, aquorbium::ThresholdStepSelector::maximumSteps> detail {};
    detail[0] = 0.80f;
    detail[1] = 1.0f;
    if (!router.setCategoryThresholds(broad, 3)
        || !router.setVariantThresholds(detail, 2))
        return false;
    const auto selected = router.select(0.50f, 0.90f);
    if (selected.category != 1 || selected.variant != 1 || selected.index != 3
        || router.leaves() != 6)
        return false;
    broad[1] = 0.05f;
    return !router.setCategoryThresholds(broad, 3)
        && router.categories() == 3;
}

bool permutationRowsPreserveSharedNotes()
{
    static_assert(noexcept(std::declval<Matrix&>().advance()));
    Matrix matrix;
    std::array<Matrix::Note, Matrix::maximumNotes> notes {};
    notes[0] = { 0, 0, 0.2f };
    notes[1] = { 2, 0, 0.4f };
    notes[2] = { 4, 1, 0.6f };
    notes[3] = { 7, -1, 0.8f };
    if (!matrix.setNotes(notes, 4))
        return false;
    std::array<std::uint8_t, Matrix::maximumNotes> reverse {};
    reverse[0] = 3;
    reverse[1] = 2;
    reverse[2] = 1;
    reverse[3] = 0;
    if (!matrix.setRow(1, reverse) || !matrix.selectRow(1))
        return false;

    const auto first = matrix.advance(1, 0.5f);
    const auto second = matrix.advance(1, 0.5f);
    const auto third = matrix.advance(1, 0.5f);
    const auto fourth = matrix.advance(1, 0.5f);
    return first.sourceIndex == 3 && first.note.degree == 7
        && first.note.octave == 0 && first.note.velocity == 0.4f
        && second.sourceIndex == 2 && second.note.degree == 4
        && second.note.octave == 2 && second.note.velocity == 0.3f
        && third.sourceIndex == 1 && fourth.sourceIndex == 0
        && fourth.wrapped && fourth.event == 3;
}

bool invalidPermutationDoesNotReplaceRow()
{
    Matrix matrix;
    std::array<Matrix::Note, Matrix::maximumNotes> notes {};
    notes[0] = { 100, -100, std::numeric_limits<float>::quiet_NaN() };
    notes[1] = { -100, 100, 2.0f };
    if (!matrix.setNotes(notes, 2))
        return false;
    std::array<std::uint8_t, Matrix::maximumNotes> reverse {};
    reverse[0] = 1;
    reverse[1] = 0;
    if (!matrix.setRow(2, reverse))
        return false;
    auto duplicate = reverse;
    duplicate[1] = 1;
    if (matrix.setRow(2, duplicate) || !matrix.selectRow(2)
        || matrix.selectRow(Matrix::maximumRows))
        return false;
    const auto first = matrix.advance(20, std::numeric_limits<float>::infinity());
    const auto second = matrix.advance(-20, 2.0f);
    return first.note.degree == -64 && first.note.octave == 8
        && first.note.velocity == 1.0f
        && second.note.degree == 64 && second.note.octave == -8
        && second.note.velocity == 1.0f;
}

bool relabiIsDeterministicAndResettable()
{
    aquorbium::BiomaRelabi first;
    aquorbium::BiomaRelabi second;
    first.prepare(200.0);
    second.prepare(200.0);
    std::array<std::uint32_t, 2048> reference {};
    float minimum = 1.0f;
    float maximum = -1.0f;
    for (auto& bits : reference)
    {
        const auto a = first.tick();
        const auto b = second.tick();
        if (std::bit_cast<std::uint32_t>(a) != std::bit_cast<std::uint32_t>(b)
            || !std::isfinite(a) || a < -1.0f || a > 1.0f)
            return false;
        bits = std::bit_cast<std::uint32_t>(a);
        minimum = std::min(minimum, a);
        maximum = std::max(maximum, a);
    }
    if (maximum - minimum < 0.5f)
        return false;
    first.reset();
    for (const auto bits : reference)
        if (std::bit_cast<std::uint32_t>(first.tick()) != bits)
            return false;

    first.prepare(std::numeric_limits<double>::quiet_NaN());
    for (int sample = 0; sample < 4096; ++sample)
        if (!std::isfinite(first.tick()))
            return false;
    return true;
}
}

int main()
{
    const std::pair<const char*, bool (*)()> cases[] {
        { "branching tree", branchingTreeSelectsTwoLayers },
        { "branching thresholds", branchingThresholdsAreIndependentAndBounded },
        { "permutation rows", permutationRowsPreserveSharedNotes },
        { "permutation limits", invalidPermutationDoesNotReplaceRow },
        { "relabi", relabiIsDeterministicAndResettable },
    };
    for (const auto& [name, test] : cases)
    {
        if (!test())
        {
            std::cerr << name << " failed\n";
            return EXIT_FAILURE;
        }
    }
    std::cout << "Branching, permutations and Relabi passed\n";
}
