// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace aquorbium
{
// Eight fixed permutation rows over a shared pool of up to sixteen notes.
// Rows change order without duplicating note data or allocating at event time.
class NotePermutationMatrix final
{
public:
    static constexpr std::size_t maximumNotes = 16;
    static constexpr std::size_t maximumRows = 8;

    struct Note
    {
        int degree = 0;
        int octave = 0;
        float velocity = 1.0f;
    };

    struct Output
    {
        Note note;
        std::size_t sourceIndex = 0;
        std::size_t row = 0;
        std::size_t step = 0;
        bool wrapped = false;
        std::uint64_t event = 0;
    };

    NotePermutationMatrix() noexcept;

    [[nodiscard]] bool setNotes(
        const std::array<Note, maximumNotes>& values,
        std::size_t count) noexcept;
    [[nodiscard]] bool setRow(
        std::size_t row,
        const std::array<std::uint8_t, maximumNotes>& permutation) noexcept;
    [[nodiscard]] bool selectRow(std::size_t row) noexcept;
    void reset(std::size_t startStep = 0) noexcept;
    [[nodiscard]] Output advance(int octaveOffset = 0,
                                 float velocityScale = 1.0f) noexcept;

    [[nodiscard]] std::size_t size() const noexcept { return noteCount; }
    [[nodiscard]] std::size_t activeRow() const noexcept { return selectedRow; }

private:
    [[nodiscard]] static Note sanitized(const Note& value) noexcept;
    void makeIdentityRows() noexcept;

    std::array<Note, maximumNotes> notes {};
    std::array<std::array<std::uint8_t, maximumNotes>, maximumRows> rows {};
    std::array<bool, maximumRows> rowValid {};
    std::size_t noteCount = 1;
    std::size_t selectedRow = 0;
    std::size_t cursor = 0;
    std::uint64_t eventCounter = 0;
};
}
