// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

#include "PerformanceParameterState.h"

#include <cstring>

namespace aquorbium::app
{
// GUI-thread only. Files contain parameters, never audio, notes or DSP memory.
class PerformanceSceneFile
{
public:
    static constexpr int maximumBytes = 16384;
    static constexpr const char* extension = ".aquscene";

    struct WriteResult
    {
        juce::Result result;
        juce::File backup;
    };

    [[nodiscard]] static juce::Result read(const juce::File& file,
                                           PerformanceParameterState& destination)
    {
        if (!file.existsAsFile() || file.isSymbolicLink())
            return failure("Escolha um arquivo regular de cena");
        const auto size = file.getSize();
        if (size <= 0 || size > maximumBytes)
            return failure("Cena vazia ou maior que 16 KiB");
        juce::FileInputStream input(file);
        if (!input.openedOk())
            return failure("Falha ao abrir a cena");
        // A bounded read also protects against a file growing after getSize().
        std::array<char, maximumBytes + 1> bytes {};
        const auto count = input.read(bytes.data(), static_cast<int>(bytes.size()));
        if (count != size || count > maximumBytes || input.getStatus().failed())
            return failure("Falha de leitura ou arquivo alterado durante a leitura");
        if (std::memchr(bytes.data(), 0, static_cast<std::size_t>(count)) != nullptr
            || !juce::CharPointer_UTF8::isValidString(bytes.data(), count))
            return failure("A cena deve ser texto UTF-8 sem bytes nulos");
        return decode(juce::String::fromUTF8(bytes.data(), count), destination);
    }

    [[nodiscard]] static juce::Result decode(const juce::String& text,
                                             PerformanceParameterState& destination)
    {
        if (!text.trimEnd().endsWith("</AQUORBIUM_PERFORMANCE_V1>"))
            return failure("Formato de cena recusado");
        const auto xml = parseParameterXml(text, maximumBytes, 5);
        if (!xml)
            return failure("XML de cena inválido ou não permitido");
        const auto tree = juce::ValueTree::fromXml(*xml);
        PerformanceParameterState replacement;
        if (!replacement.restore(tree))
            return failure("Cena incompleta ou de versão desconhecida");
        // External files must use normalized values; don't silently clamp them.
        const auto normalized = [](const juce::ValueTree& node) {
            for (int index = 0; index < node.getNumProperties(); ++index)
            {
                const auto value = node.getProperty(node.getPropertyName(index)).toString().getDoubleValue();
                if (value < 0.0 || value > 1.0)
                    return false;
            }
            return true;
        };
        if (!normalized(tree) || !normalized(tree.getChild(0)))
            return failure("Valores da cena fora do intervalo 0 a 1");
        if (!destination.restore(replacement.snapshot()))
            return failure("Falha ao restaurar a cena");
        return juce::Result::ok();
    }

    [[nodiscard]] static WriteResult write(const juce::File& file,
                                            const PerformanceParameterState& state,
                                            bool allowReplacement = false)
    {
        const auto fail = [](const char* message, juce::File backup = {}) {
            return WriteResult { failure(message), backup };
        };
        if (!file.hasFileExtension(extension) || !file.getParentDirectory().isDirectory()
            || file.isDirectory() || file.isSymbolicLink())
            return fail("Destino inválido; use a extensão .aquscene");
        if (file.exists() && !allowReplacement)
            return fail("Arquivo existente: substituição não autorizada");
        const auto text = state.snapshot().toXmlString();
        PerformanceParameterState checked;
        if (decode(text, checked).failed())
            return fail("Estado inválido para exportação");

        juce::TemporaryFile temporary(file);
        {
            juce::FileOutputStream output(temporary.getFile());
            if (!output.openedOk() || !output.write(text.toRawUTF8(), text.getNumBytesAsUTF8()))
                return fail("Falha ao escrever o arquivo temporário");
            output.flush();
            if (output.getStatus().failed())
                return fail("Falha ao finalizar a escrita");
        }
        PerformanceParameterState staged;
        if (read(temporary.getFile(), staged).failed()
            || !staged.snapshot().isEquivalentTo(checked.snapshot()))
            return fail("Falha ao verificar o temporário; destino preservado");
        juce::File backup;
        // Recheck after staging: never replace a newly appeared target silently.
        if (file.exists())
        {
            if (!allowReplacement || !file.existsAsFile() || file.isSymbolicLink())
                return fail("Destino alterado; exportação cancelada");
            if (file.getSize() > maximumBytes)
                return fail("Destino maior que 16 KiB; escolha outro arquivo");
            backup = file.getSiblingFile(file.getFileName() + ".bak").getNonexistentSibling();
            if (!file.copyFileTo(backup))
                return fail("Falha ao preservar o arquivo anterior", backup);
        }
        // JUCE replaces from the same directory; retain backup even on failure.
        if (!temporary.overwriteTargetFileWithTemporary())
            return fail("Falha ao substituir o destino; backup preservado", backup);
        return { juce::Result::ok(), backup };
    }

private:
    static juce::Result failure(const char* text)
    {
        return juce::Result::fail(juce::String::fromUTF8(text));
    }
};
}
