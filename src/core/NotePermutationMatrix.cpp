// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#include "core/NotePermutationMatrix.h"

#include <algorithm>
#include <cmath>

namespace aquorbium
{
NotePermutationMatrix::NotePermutationMatrix() noexcept
{
    makeIdentityRows();
}

bool NotePermutationMatrix::setNotes(
    const std::array<Note, maximumNotes>& values, std::size_t count) noexcept
{
    if (count == 0 || count > maximumNotes)
        return false;
    for (std::size_t index = 0; index < count; ++index)
        notes[index] = sanitized(values[index]);
    noteCount = count;
    makeIdentityRows();
    selectedRow = 0;
    reset();
    return true;
}

bool NotePermutationMatrix::setRow(
    std::size_t row,
    const std::array<std::uint8_t, maximumNotes>& permutation) noexcept
{
    if (row >= maximumRows)
        return false;
    std::array<bool, maximumNotes> used {};
    for (std::size_t step = 0; step < noteCount; ++step)
    {
        const auto index = static_cast<std::size_t>(permutation[step]);
        if (index >= noteCount || used[index])
            return false;
        used[index] = true;
    }
    rows[row] = permutation;
    rowValid[row] = true;
    return true;
}

bool NotePermutationMatrix::selectRow(std::size_t row) noexcept
{
    if (row >= maximumRows || !rowValid[row])
        return false;
    selectedRow = row;
    reset();
    return true;
}

void NotePermutationMatrix::reset(std::size_t startStep) noexcept
{
    cursor = startStep % noteCount;
    eventCounter = 0;
}

NotePermutationMatrix::Output NotePermutationMatrix::advance(
    int octaveOffset, float velocityScale) noexcept
{
    const auto step = cursor;
    const auto sourceIndex = static_cast<std::size_t>(rows[selectedRow][step]);
    auto note = notes[sourceIndex];
    note.octave = std::clamp(note.octave + octaveOffset, -8, 8);
    const auto scale = std::clamp(
        std::isfinite(velocityScale) ? velocityScale : 1.0f, 0.0f, 2.0f);
    note.velocity = std::clamp(note.velocity * scale, 0.0f, 1.0f);

    cursor = (cursor + 1) % noteCount;
    return { note, sourceIndex, selectedRow, step, cursor == 0, eventCounter++ };
}

NotePermutationMatrix::Note NotePermutationMatrix::sanitized(
    const Note& value) noexcept
{
    return {
        std::clamp(value.degree, -64, 64),
        std::clamp(value.octave, -8, 8),
        std::clamp(std::isfinite(value.velocity) ? value.velocity : 1.0f,
                   0.0f, 1.0f)
    };
}

void NotePermutationMatrix::makeIdentityRows() noexcept
{
    for (std::size_t row = 0; row < maximumRows; ++row)
    {
        for (std::size_t index = 0; index < maximumNotes; ++index)
            rows[row][index] = static_cast<std::uint8_t>(index);
        rowValid[row] = true;
    }
}
}
