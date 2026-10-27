// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

// The four languages of the RASGO family (the convention of the Antitotem,
// Navalha 2, Rasgo Modular and Trioio, rewritten here for the Aquorbium):
// English, Portuguese, French and Spanish. On the first opening, the system
// language (any other falls back to English); afterwards, the one chosen with
// the EN/PT/FR/ES button. Every visible text comes from here - control names,
// tips, the Learn texts, cards, states and the ecosystem panel.
// (2026-10-20, the author: "avance os tutoriais, sobre, learn, idiomas".)

#include <juce_core/juce_core.h>

#include <array>
#include <cstddef>

namespace aquorbium::app
{
enum class Lang { en = 0, pt, fr, es };

struct L4
{
    const char* en;
    const char* pt;
    const char* fr;
    const char* es;
};

inline juce::String tr(const L4& value, Lang language)
{
    const std::array<const char*, 4> all { value.en, value.pt, value.fr, value.es };
    const auto* text = all[static_cast<std::size_t>(language)];
    return juce::String::fromUTF8(text != nullptr ? text : "");
}

inline Lang langFromCode(const juce::String& code)
{
    const auto c = code.substring(0, 2).toLowerCase();
    if (c == "pt") return Lang::pt;
    if (c == "fr") return Lang::fr;
    if (c == "es") return Lang::es;
    return Lang::en;
}
inline juce::String langCode(Lang l) { return std::array<const char*, 4> { "en", "pt", "fr", "es" }[static_cast<std::size_t>(l)]; }
inline Lang nextLang(Lang l) { return static_cast<Lang>((static_cast<int>(l) + 1) % 4); }

// the interface's language: GUI thread only (the audio thread never reads it)
inline Lang& currentLang() noexcept
{
    static Lang language = Lang::en;
    return language;
}
inline juce::String T(const L4& value) { return tr(value, currentLang()); }

// upper case for the four languages (2026-10-22): juce::String::toUpperCase
// follows the C locale and leaves the accented letters lower ("A áGUA").
// Latin-1 (à..þ, ÿ) and œ cover EN, PT, FR and ES; the rest goes to JUCE.
inline juce::juce_wchar upperLetter(juce::juce_wchar c) noexcept
{
    if (c >= 0xe0 && c <= 0xfe && c != 0xf7) return c - 0x20;
    if (c == 0xff) return 0x178;
    if (c == 0x153) return 0x152;
    return juce::CharacterFunctions::toUpperCase(c);
}
inline juce::String upper(const juce::String& text)
{
    juce::String out;
    out.preallocateBytes(text.getNumBytesAsUTF8() + 8);
    for (auto p = text.getCharPointer(); !p.isEmpty();)
        out += juce::String::charToString(upperLetter(p.getAndAdvance()));
    return out;
}

namespace S
{
// ---- header -------------------------------------------------------------------
inline const L4 subtitle { "AUDIBLE BIOME · ECOLOGICAL INSTRUMENT", "BIOMA AUDÍVEL · INSTRUMENTO ECOLÓGICO",
                           "BIOME AUDIBLE · INSTRUMENT ÉCOLOGIQUE", "BIOMA AUDIBLE · INSTRUMENTO ECOLÓGICO" };
inline const L4 tutorial { "TUTORIAL", "TUTORIAL", "TUTORIEL", "TUTORIAL" };
inline const L4 learn { "LEARN", "LEARN", "LEARN", "LEARN" };
inline const L4 about { "ABOUT", "SOBRE", "À PROPOS", "ACERCA DE" };
inline const L4 languageTip { "Interface language: EN, PT, FR, ES", "Idioma da interface: EN, PT, FR, ES",
                              "Langue de l'interface : EN, PT, FR, ES", "Idioma de la interfaz: EN, PT, FR, ES" };
inline const L4 tutorialTip { "A guided tour of the instrument, chapter by chapter", "Uma visita guiada ao instrumento, capítulo a capítulo",
                              "Une visite guidée de l'instrument, chapitre par chapitre", "Una visita guiada al instrumento, capítulo a capítulo" };
inline const L4 learnTip { "LEARN on: point at any control or card and its explanation appears below",
                           "LEARN ligado: aponte para qualquer controle ou cartão e a explicação aparece embaixo",
                           "LEARN actif : pointez une commande ou une carte et son explication apparaît en bas",
                           "LEARN activo: apunte a cualquier control o tarjeta y su explicación aparece abajo" };
inline const L4 aboutTip { "What the Aquorbium is, its concepts, credits and licence", "O que é o Aquorbium, seus conceitos, créditos e licença",
                           "Ce qu'est l'Aquorbium, ses concepts, crédits et licence", "Qué es el Aquorbium, sus conceptos, créditos y licencia" };
inline const L4 audio { "AUDIO", "ÁUDIO", "AUDIO", "AUDIO" };
inline const L4 audioTip { "Choose output, rate and buffer. To diagnose clicks, try 48 kHz / 512 samples.",
                           "Selecionar saída, taxa e buffer. Para diagnosticar estalos, tente 48 kHz / 512 amostras.",
                           "Choisir sortie, fréquence et tampon. Pour diagnostiquer des clics, essayez 48 kHz / 512 échantillons.",
                           "Elegir salida, frecuencia y búfer. Para diagnosticar chasquidos, pruebe 48 kHz / 512 muestras." };
inline const L4 mute { "MUTE", "SILENCIAR", "MUET", "SILENCIAR" };
inline const L4 unmute { "SOUND", "SOM", "SON", "SONIDO" };
inline const L4 muteTip { "Closes the sound with a short ramp; the ecosystem keeps living.",
                          "Fecha o som com uma rampa curta; o ecossistema continua vivendo.",
                          "Coupe le son avec une rampe courte ; l'écosystème continue de vivre.",
                          "Cierra el sonido con una rampa corta; el ecosistema sigue viviendo." };
inline const L4 pause { "PAUSE", "PAUSA", "PAUSE", "PAUSA" };
inline const L4 resume { "RESUME", "CONTINUAR", "REPRENDRE", "CONTINUAR" };
inline const L4 pauseTip { "Silences and freezes the ecosystem's time; RESUME continues where it stopped.",
                           "Silencia e congela o tempo do ecossistema; CONTINUAR retoma de onde parou.",
                           "Coupe le son et fige le temps de l'écosystème ; REPRENDRE continue où il s'est arrêté.",
                           "Silencia y congela el tiempo del ecosistema; CONTINUAR sigue donde se detuvo." };
inline const L4 close { "CLOSE", "FECHAR", "FERMER", "CERRAR" };
// the listening mode (2026-10-21): loudspeakers (stereo) or headphones (binaural)
// ---- the synthesis device's editor (SYNTH_DEVICE_DESIGN.md, phase 4; 2026-10-21) ----
namespace synth
{
inline const L4 title { "SYNTHESIS", "SÍNTESE", "SYNTHÈSE", "SÍNTESIS" };
inline const L4 edit { "EDIT", "EDITAR", "ÉDITER", "EDITAR" };
inline const L4 copyAll { "COPY TO ALL", "COPIAR PARA TODOS", "COPIER À TOUS", "COPIAR A TODOS" };
// PROPOSTA (phase 5, 2026-10-22): starting patches from the species and the seed
inline const L4 propose { "PROPOSE", "PROPOSTA", "PROPOSER", "PROPUESTA" };
inline const L4 proposeAll { "PROPOSE ALL", "PROPOR TODOS", "PROPOSER TOUS", "PROPONER TODOS" };
inline const L4 proposeTip { "PROPOSE: a starting patch for this organism, drawn from its species (sources, gesture, register) and its ecological strategy (what reaches the matrix), and levelled with the others. Each press draws another one; the first of a session follows its seed. Edit from there.",
                             "PROPOSTA: um patch de partida para este organismo, tirado da espécie (fontes, gesto, registro) e da estratégia ecológica (o que chega à matriz), e nivelado com os outros. Cada clique tira outro; o primeiro da sessão segue a semente. Edite a partir dele.",
                             "PROPOSER : un patch de départ pour cet organisme, tiré de son espèce (sources, geste, registre) et de sa stratégie écologique (ce qui atteint la matrice), et nivelé avec les autres. Chaque clic en tire un autre ; le premier d'une session suit sa graine. Éditez à partir de là.",
                             "PROPUESTA: un patch de partida para este organismo, sacado de su especie (fuentes, gesto, registro) y de su estrategia ecológica (lo que llega a la matriz), y nivelado con los demás. Cada clic saca otro; el primero de una sesión sigue su semilla. Edite a partir de ahí." };
inline const L4 proposeAllTip { "PROPOSE ALL: a starting patch for each of the five organisms at once (replaces all five patches).",
                                "PROPOR TODOS: um patch de partida para cada um dos cinco organismos de uma vez (substitui os cinco patches).",
                                "PROPOSER TOUS : un patch de départ pour chacun des cinq organismes à la fois (remplace les cinq patches).",
                                "PROPONER TODOS: un patch de partida para cada uno de los cinco organismos a la vez (reemplaza los cinco patches)." };
inline const L4 mixTip { "SYNTHESIS: how much of each organism's synthesis voice sounds instead of its own body. At zero the instrument sounds exactly as before.",
                         "SÍNTESE: quanto da voz de síntese de cada organismo soa no lugar do seu próprio corpo. Em zero o instrumento soa exatamente como antes.",
                         "SYNTHÈSE : quelle part de la voix de synthèse de chaque organisme sonne à la place de son propre corps. À zéro l'instrument sonne exactement comme avant.",
                         "SÍNTESIS: cuánto de la voz de síntesis de cada organismo suena en lugar de su propio cuerpo. En cero el instrumento suena exactamente como antes." };
inline const L4 intro { "Each organism plays its own synthesis voice with the ecosystem's notes. Build the sound here; in MODULATION the ecosystem moves the parameters you route.",
                        "Cada organismo toca a sua própria voz de síntese com as notas do ecossistema. Construa o som aqui; em MODULAÇÃO o ecossistema move os parâmetros que você ligar.",
                        "Chaque organisme joue sa propre voix de synthèse avec les notes de l'écosystème. Construisez le son ici ; dans MODULATION l'écosystème bouge les paramètres que vous reliez.",
                        "Cada organismo toca su propia voz de síntesis con las notas del ecosistema. Construya el sonido aquí; en MODULACIÓN el ecosistema mueve los parámetros que conecte." };
inline const L4 sourceA { "SOURCE A", "FONTE A", "SOURCE A", "FUENTE A" };
inline const L4 sourceB { "SOURCE B", "FONTE B", "SOURCE B", "FUENTE B" };
inline const L4 mixSection { "MIX", "MISTURA", "MÉLANGE", "MEZCLA" };
inline const L4 filter { "FILTER", "FILTRO", "FILTRE", "FILTRO" };
inline const L4 ampEnvelope { "AMP ENVELOPE", "ENVELOPE AMP", "ENVELOPPE AMP", "ENVOLVENTE AMP" };
inline const L4 modEnvelope { "MOD ENVELOPE", "ENVELOPE MOD", "ENVELOPPE MOD", "ENVOLVENTE MOD" };
inline const L4 lfo1 { "LFO 1 - VIBRATO", "LFO 1 - VIBRATO", "LFO 1 - VIBRATO", "LFO 1 - VIBRATO" };
inline const L4 lfo2 { "LFO 2 - WAH", "LFO 2 - WAH", "LFO 2 - WAH", "LFO 2 - WAH" };
inline const L4 matrix { "MODULATION - the ecosystem plays the parameters", "MODULAÇÃO - o ecossistema toca os parâmetros",
                         "MODULATION - l'écosystème joue les paramètres", "MODULACIÓN - el ecosistema toca los parámetros" };
inline const std::array<L4, 14> sources {{
    { "off", "desligada", "éteinte", "apagada" },
    { "oscillator", "oscilador", "oscillateur", "oscilador" },
    { "noise", "ruído", "bruit", "ruido" },
    { "native body", "corpo nativo", "corps natif", "cuerpo nativo" },
    { "FM", "FM", "FM", "FM" },
    { "wavetable", "wavetable", "table d'ondes", "tabla de ondas" },
    { "additive", "aditiva", "additive", "aditiva" },
    { "pulsar", "pulsar", "pulsar", "pulsar" },
    { "string", "corda", "corde", "cuerda" },
    { "matter (modal)", "matéria (modal)", "matière (modale)", "materia (modal)" },
    { "a body", "um corpo", "un corps", "un cuerpo" },
    { "FM 4-op", "FM 4 operadores", "FM 4 opérateurs", "FM 4 operadores" },
    { "chaos (Duffing)", "caos (Duffing)", "chaos (Duffing)", "caos (Duffing)" },
    { "feedback (no input)", "realimentação (sem entrada)", "rétroaction (sans entrée)", "realimentación (sin entrada)" } }};
inline const L4 bodyChoice { "BODY", "CORPO", "CORPS", "CUERPO" };
// what FORMA / LARGURA / COR mean for each source (by Source index)
inline const std::array<std::array<L4, 3>, 14> macroNames {{
    {{ { "SHAPE", "FORMA", "FORME", "FORMA" }, { "WIDTH", "LARGURA", "LARGEUR", "ANCHO" }, { "-", "-", "-", "-" } }},
    {{ { "SHAPE", "FORMA", "FORME", "FORMA" }, { "WIDTH", "LARGURA", "LARGEUR", "ANCHO" }, { "-", "-", "-", "-" } }},
    {{ { "-", "-", "-", "-" }, { "-", "-", "-", "-" }, { "COLOUR", "COR", "COULEUR", "COLOR" } }},
    {{ { "-", "-", "-", "-" }, { "-", "-", "-", "-" }, { "-", "-", "-", "-" } }},
    {{ { "RATIO", "RAZÃO", "RAPPORT", "RAZÓN" }, { "INDEX", "ÍNDICE", "INDICE", "ÍNDICE" }, { "FEEDBACK", "REALIM.", "RÉTROACT.", "REALIM." } }},
    {{ { "POSITION", "POSIÇÃO", "POSITION", "POSICIÓN" }, { "WARP", "WARP", "WARP", "WARP" }, { "DRIFT", "DERIVA", "DÉRIVE", "DERIVA" } }},
    {{ { "TILT", "INCLINAÇÃO", "PENTE", "PENDIENTE" }, { "ODD/EVEN", "ÍMPAR/PAR", "IMPAIR/PAIR", "IMPAR/PAR" }, { "STRETCH", "ESTICAR", "ÉTIREMENT", "ESTIRAR" } }},
    {{ { "FORMANT", "FORMANTE", "FORMANT", "FORMANTE" }, { "PULSARET", "PULSARET", "PULSARET", "PULSARET" }, { "WINDOW", "JANELA", "FENÊTRE", "VENTANA" } }},
    {{ { "DAMPING", "AMORTEC.", "AMORTISS.", "AMORTIG." }, { "POSITION", "POSIÇÃO", "POSITION", "POSICIÓN" }, { "SUSTAIN", "SUSTENTO", "MAINTIEN", "SOSTÉN" } }},
    {{ { "STRUCTURE", "ESTRUTURA", "STRUCTURE", "ESTRUCTURA" }, { "BRIGHTNESS", "BRILHO", "BRILLANCE", "BRILLO" }, { "DAMPING", "AMORTEC.", "AMORTISS.", "AMORTIG." } }},
    {{ { "MATTER", "MATÉRIA", "MATIÈRE", "MATERIA" }, { "-", "-", "-", "-" }, { "-", "-", "-", "-" } }},
    {{ { "RATIO B", "RAZÃO B", "RAPPORT B", "RAZÓN B" }, { "INDEX", "ÍNDICE", "INDICE", "ÍNDICE" }, { "FEEDBACK", "REALIM.", "RÉTROACT.", "REALIM." } }},
    {{ { "FORCING", "FORÇA", "FORÇAGE", "FUERZA" }, { "DAMPING", "AMORTEC.", "AMORTISS.", "AMORTIG." }, { "COLOUR", "COR", "COULEUR", "COLOR" } }},
    {{ { "GAIN", "GANHO", "GAIN", "GANANCIA" }, { "DETUNE", "DESAFINAR", "DÉSACCORD", "DESAFINAR" }, { "COLOUR", "COR", "COULEUR", "COLOR" } }} }};
// FM 4-OP (2026-10-22): the ratios of C and D, and the eight algorithms
// (who modulates whom: ">" modulates, "+" sums carriers)
inline const L4 ratioC { "RATIO C", "RAZÃO C", "RAPPORT C", "RAZÓN C" };
inline const L4 ratioD { "RATIO D", "RAZÃO D", "RAPPORT D", "RAZÓN D" };
inline const std::array<L4, 8> algorithms {{
    { "1  A>B>C>D", "1  A>B>C>D", "1  A>B>C>D", "1  A>B>C>D" },
    { "2  A>B>C + D", "2  A>B>C + D", "2  A>B>C + D", "2  A>B>C + D" },
    { "3  A>B + C>D", "3  A>B + C>D", "3  A>B + C>D", "3  A>B + C>D" },
    { "4  A>C, B>C>D", "4  A>C, B>C>D", "4  A>C, B>C>D", "4  A>C, B>C>D" },
    { "5  A, B, C > D", "5  A, B, C > D", "5  A, B, C > D", "5  A, B, C > D" },
    { "6  A>B + C + D", "6  A>B + C + D", "6  A>B + C + D", "6  A>B + C + D" },
    { "7  A > B, C, D", "7  A > B, C, D", "7  A > B, C, D", "7  A > B, C, D" },
    { "8  A + B + C + D (additive)", "8  A + B + C + D (aditivo)", "8  A + B + C + D (additif)", "8  A + B + C + D (aditivo)" } }};
inline const std::array<L4, 5> filterModes {{
    { "low-pass", "passa-baixas", "passe-bas", "paso bajo" },
    { "band-pass", "passa-banda", "passe-bande", "paso banda" },
    { "high-pass", "passa-altas", "passe-haut", "paso alto" },
    { "notch", "rejeita-faixa", "réjecteur", "rechazo de banda" },
    { "formant (vowels)", "formante (vogais)", "formant (voyelles)", "formante (vocales)" } }};
inline const std::array<L4, 4> lfoShapes {{
    { "sine", "seno", "sinus", "seno" },
    { "triangle", "triângulo", "triangle", "triángulo" },
    { "square", "quadrada", "carré", "cuadrada" },
    { "wander", "deriva", "dérive", "deriva" } }};
inline const std::array<L4, 18> modSources {{
    { "-", "-", "-", "-" },
    { "amp envelope", "envelope amp", "enveloppe amp", "envolvente amp" },
    { "mod envelope", "envelope mod", "enveloppe mod", "envolvente mod" },
    { "LFO 1", "LFO 1", "LFO 1", "LFO 1" },
    { "LFO 2", "LFO 2", "LFO 2", "LFO 2" },
    { "note accent", "acento da nota", "accent de la note", "acento de la nota" },
    { "energy", "energia", "énergie", "energía" },
    { "speed", "velocidade", "vitesse", "velocidad" },
    { "relation", "relação", "relation", "relación" },
    { "tension", "tensão", "tension", "tensión" },
    { "distance", "distância", "distance", "distancia" },
    { "SOFAR channel", "canal SOFAR", "canal SOFAR", "canal SOFAR" },
    { "temperature", "temperatura", "température", "temperatura" },
    { "hunger", "fome", "faim", "hambre" },
    { "age", "idade", "âge", "edad" },
    { "prey", "presas", "proies", "presas" },
    { "habitat", "habitat", "habitat", "hábitat" },
    { "room", "sala", "salle", "sala" } }};
inline const std::array<L4, 19> modDestinations {{
    { "-", "-", "-", "-" },
    { "pitch", "altura", "hauteur", "altura" },
    { "cutoff", "corte", "coupure", "corte" },
    { "resonance", "ressonância", "résonance", "resonancia" },
    { "drive", "drive", "drive", "drive" },
    { "shape A", "forma A", "forme A", "forma A" },
    { "shape B", "forma B", "forme B", "forma B" },
    { "width A", "largura A", "largeur A", "ancho A" },
    { "width B", "largura B", "largeur B", "ancho B" },
    { "level A", "nível A", "niveau A", "nivel A" },
    { "level B", "nível B", "niveau B", "nivel B" },
    { "ring", "ring", "ring", "ring" },
    { "FM", "FM", "FM", "FM" },
    { "colour A", "cor A", "couleur A", "color A" },
    { "colour B", "cor B", "couleur B", "color B" },
    { "level", "nível", "niveau", "nivel" },
    { "LFO 1 rate", "veloc. LFO 1", "vitesse LFO 1", "veloc. LFO 1" },
    { "LFO 2 rate", "veloc. LFO 2", "vitesse LFO 2", "veloc. LFO 2" },
    { "vowel", "vogal", "voyelle", "vocal" } }};
// knob labels
inline const L4 shape { "SHAPE", "FORMA", "FORME", "FORMA" };
inline const L4 width { "WIDTH", "LARGURA", "LARGEUR", "ANCHO" };
inline const L4 tune { "TUNE", "AFINAR", "ACCORD", "AFINAR" };
inline const L4 fine { "FINE", "FINO", "FIN", "FINO" };
inline const L4 sub { "SUB", "SUB", "SUB", "SUB" };
inline const L4 colour { "COLOUR", "COR", "COULEUR", "COLOR" };
inline const L4 level { "LEVEL", "NÍVEL", "NIVEAU", "NIVEL" };
inline const L4 ring { "RING", "RING", "RING", "RING" };
inline const L4 fm { "FM B>A", "FM B>A", "FM B>A", "FM B>A" };
inline const L4 drift { "DRIFT", "DERIVA", "DÉRIVE", "DERIVA" };
inline const L4 accent { "ACCENT", "ACENTO", "ACCENT", "ACENTO" };
inline const L4 cutoff { "CUTOFF", "CORTE", "COUPURE", "CORTE" };
inline const L4 tract { "TRACT", "TRATO", "CONDUIT", "TRACTO" };
inline const L4 vowel { "VOWEL", "VOGAL", "VOYELLE", "VOCAL" };
inline const L4 resonance { "RESO", "RESSON.", "RÉSON.", "RESON." };
inline const L4 keytrack { "KEY", "NOTA", "NOTE", "NOTA" };
inline const L4 envelope { "ENV", "ENV", "ENV", "ENV" };
inline const L4 drive { "DRIVE", "DRIVE", "DRIVE", "DRIVE" };
inline const L4 attack { "ATTACK", "ATAQUE", "ATTAQUE", "ATAQUE" };
inline const L4 decay { "DECAY", "QUEDA", "DÉCLIN", "CAÍDA" };
inline const L4 sustain { "SUSTAIN", "SUSTENTO", "MAINTIEN", "SOSTÉN" };
inline const L4 release { "RELEASE", "SOLTURA", "RELÂCHEM.", "LIBERACIÓN" };
inline const L4 curve { "CURVE", "CURVA", "COURBE", "CURVA" };
inline const L4 rate { "RATE", "VELOC.", "VITESSE", "VELOC." };
inline const L4 vibrato { "VIBRATO", "VIBRATO", "VIBRATO", "VIBRATO" };
inline const L4 wah { "WAH", "WAH", "WAH", "WAH" };
inline const L4 amount { "AMOUNT", "QUANTIDADE", "QUANTITÉ", "CANTIDAD" };

// ---- help inside the editor (phase 6, 2026-10-22): tooltips that follow the choice ----
inline const L4 editTip { "EDIT: opens the synthesis device - for each organism, two sources, a filter, two envelopes, two LFOs and a matrix where the ecosystem plays the parameters. PROPOSE gives a starting point.",
                          "EDITAR: abre o dispositivo de síntese - para cada organismo, duas fontes, um filtro, dois envelopes, dois LFOs e uma matriz em que o ecossistema toca os parâmetros. PROPOSTA dá um ponto de partida.",
                          "ÉDITER : ouvre le dispositif de synthèse - pour chaque organisme, deux sources, un filtre, deux enveloppes, deux LFO et une matrice où l'écosystème joue les paramètres. PROPOSER donne un point de départ.",
                          "EDITAR: abre el dispositivo de síntesis - para cada organismo, dos fuentes, un filtro, dos envolventes, dos LFO y una matriz donde el ecosistema toca los parámetros. PROPUESTA da un punto de partida." };
// what each source is (by Source index), shown on its choice box
inline const std::array<L4, 14> sourceTips {{
    { "Off: this slot is silent.", "Desligada: este slot fica em silêncio.", "Éteinte : ce slot est muet.", "Apagada: este slot queda en silencio." },
    { "Oscillator: SHAPE morphs sine > triangle > saw > pulse (band-limited), WIDTH is the pulse width, SUB adds the octave below.",
      "Oscilador: FORMA vai de seno a triângulo, dente de serra e pulso (banda limitada), LARGURA é a largura do pulso, SUB soma a oitava de baixo.",
      "Oscillateur : FORME passe du sinus au triangle, à la dent de scie et à l'impulsion (bande limitée), LARGEUR est la largeur d'impulsion, SUB ajoute l'octave inférieure.",
      "Oscilador: FORMA va de seno a triángulo, diente de sierra y pulso (banda limitada), ANCHO es el ancho de pulso, SUB suma la octava inferior." },
    { "Noise: COLOUR goes from white to pink to brown (darker). Pitchless: the filter gives it a register.",
      "Ruído: COR vai do branco ao rosa e ao marrom (mais escuro). Sem altura: o filtro lhe dá um registro.",
      "Bruit : COULEUR va du blanc au rose puis au brun (plus sombre). Sans hauteur : le filtre lui donne un registre.",
      "Ruido: COLOR va del blanco al rosa y al marrón (más oscuro). Sin altura: el filtro le da un registro." },
    { "Native body: the organism's own sound, passed through the device's filter and envelopes.",
      "Corpo nativo: o som próprio do organismo, passado pelo filtro e pelos envelopes do dispositivo.",
      "Corps natif : le son propre de l'organisme, passé par le filtre et les enveloppes du dispositif.",
      "Cuerpo nativo: el sonido propio del organismo, pasado por el filtro y las envolventes del dispositivo." },
    { "FM: two operators, B modulating A. RATIO picks B's ratio from a musical table, INDEX the brightness (0 = a pure sine), FEEDBACK makes A rougher.",
      "FM: dois operadores, B modulando A. RAZÃO escolhe a razão de B numa tabela musical, ÍNDICE o brilho (0 = seno puro), REALIM. deixa A mais áspero.",
      "FM : deux opérateurs, B module A. RAPPORT choisit le rapport de B dans une table musicale, INDICE la brillance (0 = sinus pur), RÉTROACT. rend A plus rugueux.",
      "FM: dos operadores, B modula A. RAZÓN elige la razón de B en una tabla musical, ÍNDICE el brillo (0 = seno puro), REALIM. hace A más áspero." },
    { "Wavetable: POSITION scans 16 frames (saw > square > formant > sine), WARP bends the phase, DRIFT lets the position wander by itself.",
      "Wavetable: POSIÇÃO percorre 16 quadros (dente de serra > quadrada > formante > seno), WARP dobra a fase, DERIVA deixa a posição vagar sozinha.",
      "Table d'ondes : POSITION parcourt 16 trames (dent de scie > carré > formant > sinus), WARP courbe la phase, DÉRIVE laisse la position errer d'elle-même.",
      "Tabla de ondas: POSICIÓN recorre 16 cuadros (diente de sierra > cuadrada > formante > seno), WARP dobla la fase, DERIVA deja que la posición vague sola." },
    { "Additive: up to 64 partials. TILT from dark to bright, ODD/EVEN from only even to only odd (a clarinet-like hollowness), STRETCH makes the partials inharmonic, towards a bell.",
      "Aditiva: até 64 parciais. INCLINAÇÃO do escuro ao claro, ÍMPAR/PAR de só pares a só ímpares (um oco de clarinete), ESTICAR torna as parciais inarmônicas, rumo a um sino.",
      "Additive : jusqu'à 64 partiels. PENTE du sombre au brillant, IMPAIR/PAIR de seulement pairs à seulement impairs (un creux de clarinette), ÉTIREMENT rend les partiels inharmoniques, vers une cloche.",
      "Aditiva: hasta 64 parciales. PENDIENTE de oscuro a brillante, IMPAR/PAR de solo pares a solo impares (un hueco de clarinete), ESTIRAR vuelve inarmónicos los parciales, hacia una campana." },
    { "Pulsar (Curtis Roads): a train of short grains at the note's rate. FORMANT sets the grain's own frequency (0.1x to 8x the note), PULSARET its shape, WINDOW its envelope.",
      "Pulsar (Curtis Roads): um trem de grãos curtos na frequência da nota. FORMANTE define a frequência do próprio grão (0,1x a 8x a nota), PULSARET a forma, JANELA o envelope do grão.",
      "Pulsar (Curtis Roads) : un train de grains courts au rythme de la note. FORMANT règle la fréquence du grain (0,1x à 8x la note), PULSARET sa forme, FENÊTRE son enveloppe.",
      "Pulsar (Curtis Roads): un tren de granos cortos a la frecuencia de la nota. FORMANTE fija la frecuencia del grano (0,1x a 8x la nota), PULSARET su forma, VENTANA su envolvente." },
    { "String (Karplus-Strong): plucked at every note. DAMPING darkens it, POSITION is where it is plucked, SUSTAIN how long it rings.",
      "Corda (Karplus-Strong): dedilhada a cada nota. AMORTEC. a escurece, POSIÇÃO é onde é tocada, SUSTENTO quanto tempo ela soa.",
      "Corde (Karplus-Strong) : pincée à chaque note. AMORTISS. l'assombrit, POSITION est l'endroit pincé, MAINTIEN la durée de résonance.",
      "Cuerda (Karplus-Strong): punteada en cada nota. AMORTIG. la oscurece, POSICIÓN es dónde se puntea, SOSTÉN cuánto tiempo suena." },
    { "Matter: 24 resonant modes struck at every note. STRUCTURE goes from harmonic to stretched (wood, metal, glass), BRIGHTNESS lights the high modes, DAMPING shortens them.",
      "Matéria: 24 modos ressonantes golpeados a cada nota. ESTRUTURA vai do harmônico ao estirado (madeira, metal, vidro), BRILHO acende os modos agudos, AMORTEC. os encurta.",
      "Matière : 24 modes résonants frappés à chaque note. STRUCTURE va de l'harmonique à l'étiré (bois, métal, verre), BRILLANCE allume les modes aigus, AMORTISS. les raccourcit.",
      "Materia: 24 modos resonantes golpeados en cada nota. ESTRUCTURA va de lo armónico a lo estirado (madera, metal, vidrio), BRILLO enciende los modos agudos, AMORTIG. los acorta." },
    { "A body: one of the instrument's bodies (voice, matter, breath, string, drop, fold, membrane, glass, choir, bow, reed), chosen in the list below. MATTER moves its character.",
      "Um corpo: um dos corpos do instrumento (voz, matéria, sopro, corda, gota, dobra, membrana, vidro, coro, arco, palheta), escolhido na lista de baixo. MATÉRIA move o seu caráter.",
      "Un corps : l'un des corps de l'instrument (voix, matière, souffle, corde, goutte, pli, membrane, verre, chœur, archet, anche), choisi dans la liste du dessous. MATIÈRE déplace son caractère.",
      "Un cuerpo: uno de los cuerpos del instrumento (voz, materia, soplo, cuerda, gota, pliegue, membrana, vidrio, coro, arco, lengüeta), elegido en la lista de abajo. MATERIA mueve su carácter." },
    { "FM 4-op: four sine operators A B C D in one of eight algorithms (the list below: '>' modulates, '+' sums the carriers). RATIO B, RATIO C and RATIO D from a musical table (0.5x to 9x), INDEX the depth (0 = plain sines), FEEDBACK makes A rougher.",
      "FM 4 operadores: quatro operadores senoidais A B C D num de oito algoritmos (a lista de baixo: '>' modula, '+' soma as portadoras). RAZÃO B, RAZÃO C e RAZÃO D numa tabela musical (0,5x a 9x), ÍNDICE a profundidade (0 = senos puros), REALIM. deixa A mais áspero.",
      "FM 4 opérateurs : quatre opérateurs sinusoïdaux A B C D dans l'un de huit algorithmes (la liste du dessous : '>' module, '+' additionne les porteuses). RAPPORT B, RAPPORT C et RAPPORT D dans une table musicale (0,5x à 9x), INDICE la profondeur (0 = sinus purs), RÉTROACT. rend A plus rugueux.",
      "FM 4 operadores: cuatro operadores senoidales A B C D en uno de ocho algoritmos (la lista de abajo: '>' modula, '+' suma las portadoras). RAZÓN B, RAZÓN C y RAZÓN D en una tabla musical (0,5x a 9x), ÍNDICE la profundidad (0 = senos puros), REALIM. hace A más áspero." },
    { "Chaos: a forced Duffing oscillator (a ball between two wells, pushed at the note's frequency). FORCING goes from a small tone in one well, through chaos, to a large orbit across both wells; DAMPING moves those borders; COLOUR from the position to the brighter velocity. One loudness in every regime.",
      "Caos: um oscilador de Duffing forçado (uma bola entre dois vales, empurrada na frequência da nota). FORÇA vai de um tom pequeno num vale só, pelo caos, a uma órbita grande pelos dois vales; AMORTEC. desloca essas fronteiras; COR vai da posição à velocidade, mais brilhante. O mesmo volume em todos os regimes.",
      "Chaos : un oscillateur de Duffing forcé (une bille entre deux vallées, poussée à la fréquence de la note). FORÇAGE va d'un petit son dans une seule vallée, par le chaos, à une grande orbite à travers les deux ; AMORTISS. déplace ces frontières ; COULEUR va de la position à la vitesse, plus brillante. Le même volume dans tous les régimes.",
      "Caos: un oscilador de Duffing forzado (una bola entre dos valles, empujada a la frecuencia de la nota). FUERZA va de un tono pequeño en un solo valle, por el caos, a una órbita grande por los dos; AMORTIG. desplaza esas fronteras; COLOR va de la posición a la velocidad, más brillante. El mismo volumen en todos los regímenes." },
    { "Feedback: a loop with no input that sings by itself, like a no-input mixer - a delay tuned to the note, a low-pass and a soft clip. GAIN from barely singing to saturated; DETUNE adds a second tap off the period, and the loop jumps to other modes; COLOUR opens the low-pass.",
      "Realimentação: um laço sem entrada que canta sozinho, como uma mesa sem entrada - um atraso afinado na nota, um passa-baixas e uma saturação suave. GANHO vai de mal cantando a saturado; DESAFINAR soma um segundo tap fora do período, e o laço salta para outros modos; COR abre o passa-baixas.",
      "Rétroaction : une boucle sans entrée qui chante d'elle-même, comme une console sans entrée - un retard accordé sur la note, un passe-bas et une saturation douce. GAIN va d'à peine chantant à saturé ; DÉSACCORD ajoute une seconde prise hors période, et la boucle saute vers d'autres modes ; COULEUR ouvre le passe-bas.",
      "Realimentación: un lazo sin entrada que canta solo, como una mesa sin entrada - un retardo afinado en la nota, un paso bajo y una saturación suave. GANANCIA va de apenas cantando a saturado; DESAFINAR suma una segunda toma fuera del período, y el lazo salta a otros modos; COLOR abre el paso bajo." } }};
// the sections
inline const L4 mixSectionTip { "RING multiplies A by B (metallic sidebands). FM B>A makes B modulate A's pitch - it acts on the oscillating sources only: STRING, MATTER and BODY keep their tuning (a resonator retuned every sample is unstable). DRIFT lets the tuning breathe; ACCENT is how much the note's accent sets the level.",
                                "RING multiplica A por B (bandas laterais metálicas). FM B>A faz B modular a altura de A - age só nas fontes osciladoras: CORDA, MATÉRIA e CORPO mantêm a afinação (um ressonador reafinado a cada amostra é instável). DERIVA deixa a afinação respirar; ACENTO é quanto o acento da nota define o nível.",
                                "RING multiplie A par B (bandes latérales métalliques). FM B>A fait moduler la hauteur de A par B - seulement sur les sources oscillantes : CORDE, MATIÈRE et CORPS gardent leur accord (un résonateur réaccordé à chaque échantillon est instable). DÉRIVE fait respirer l'accord ; ACCENT règle combien l'accent de la note fixe le niveau.",
                                "RING multiplica A por B (bandas laterales metálicas). FM B>A hace que B module la altura de A - solo en las fuentes osciladoras: CUERDA, MATERIA y CUERPO mantienen su afinación (un resonador reafinado en cada muestra es inestable). DERIVA hace respirar la afinación; ACENTO es cuánto el acento de la nota fija el nivel." };
inline const L4 filterTip { "A state-variable filter (Simper): low-pass, band-pass, high-pass or notch. KEY makes the cutoff follow the note, ENV opens it with the mod envelope (in octaves, + or -), DRIVE saturates before it. FORMANT: five vowel formants (A E I O U, Fant's sung-vowel table); VOWEL walks them, CUTOFF becomes TRACT, the size of the vocal tract (1000 Hz = a bass voice), and RESO narrows them.",
                            "Um filtro de estado variável (Simper): passa-baixas, passa-banda, passa-altas ou rejeita-faixa. NOTA faz o corte seguir a nota, ENV o abre com o envelope mod (em oitavas, + ou -), DRIVE satura antes dele. FORMANTE: cinco formantes de vogais (A E I O U, a tabela de vogais cantadas de Fant); VOGAL as percorre, CORTE vira TRATO, o tamanho do trato vocal (1000 Hz = uma voz de baixo), e RESSON. as estreita.",
                            "Un filtre à variables d'état (Simper) : passe-bas, passe-bande, passe-haut ou réjecteur. NOTE fait suivre la note à la coupure, ENV l'ouvre avec l'enveloppe mod (en octaves, + ou -), DRIVE sature avant lui. FORMANT : cinq formants de voyelles (A E I O U, la table des voyelles chantées de Fant) ; VOYELLE les parcourt, COUPURE devient CONDUIT, la taille du conduit vocal (1000 Hz = une voix de basse), et RÉSON. les resserre.",
                            "Un filtro de variables de estado (Simper): paso bajo, paso banda, paso alto o rechazo de banda. NOTA hace que el corte siga la nota, ENV lo abre con la envolvente mod (en octavas, + o -), DRIVE satura antes de él. FORMANTE: cinco formantes de vocales (A E I O U, la tabla de vocales cantadas de Fant); VOCAL las recorre, CORTE se vuelve TRACTO, el tamaño del tracto vocal (1000 Hz = una voz de bajo), y RESON. las estrecha." };
inline const L4 envelopeTip { "Envelopes are opened by the ecosystem's notes. AMP shapes the volume; MOD shapes the filter (ENV) and anything routed to it in MODULATION. CURVE goes from convex to concave.",
                              "Os envelopes são abertos pelas notas do ecossistema. AMP dá forma ao volume; MOD dá forma ao filtro (ENV) e ao que for ligado a ele em MODULAÇÃO. CURVA vai do convexo ao côncavo.",
                              "Les enveloppes sont ouvertes par les notes de l'écosystème. AMP façonne le volume ; MOD façonne le filtre (ENV) et ce qui lui est relié dans MODULATION. COURBE va du convexe au concave.",
                              "Las envolventes las abren las notas del ecosistema. AMP da forma al volumen; MOD da forma al filtro (ENV) y a lo que se conecte a ella en MODULACIÓN. CURVA va de convexa a cóncava." };
inline const L4 lfoTip { "Two slow oscillators: LFO 1 goes to the pitch (VIBRATO, in cents), LFO 2 to the cutoff (WAH, in octaves). WANDER is a smooth random walk. Both can also be routed in MODULATION.",
                         "Dois osciladores lentos: o LFO 1 vai para a altura (VIBRATO, em cents), o LFO 2 para o corte (WAH, em oitavas). DERIVA é um passeio aleatório suave. Os dois também podem ser ligados em MODULAÇÃO.",
                         "Deux oscillateurs lents : le LFO 1 va à la hauteur (VIBRATO, en cents), le LFO 2 à la coupure (WAH, en octaves). DÉRIVE est une marche aléatoire douce. Les deux peuvent aussi être reliés dans MODULATION.",
                         "Dos osciladores lentos: el LFO 1 va a la altura (VIBRATO, en cents), el LFO 2 al corte (WAH, en octavas). DERIVA es un paseo aleatorio suave. Ambos pueden conectarse también en MODULACIÓN." };
inline const L4 matrixTip { "Eight routes: a SOURCE (the voice's envelopes and LFOs, or a signal of the ecosystem), a DESTINATION and an AMOUNT (- to +). Pitch moves up to 12 semitones, cutoff 4 octaves, rates x8, the rest the whole range. This is where the ecosystem plays the sound.",
                            "Oito vias: uma FONTE (os envelopes e LFOs da voz, ou um sinal do ecossistema), um DESTINO e uma QUANTIDADE (- a +). A altura move até 12 semitons, o corte 4 oitavas, as velocidades x8, o resto a faixa inteira. É aqui que o ecossistema toca o som.",
                            "Huit voies : une SOURCE (les enveloppes et LFO de la voix, ou un signal de l'écosystème), une DESTINATION et une QUANTITÉ (- à +). La hauteur bouge jusqu'à 12 demi-tons, la coupure 4 octaves, les vitesses x8, le reste toute la plage. C'est ici que l'écosystème joue le son.",
                            "Ocho vías: una FUENTE (las envolventes y LFO de la voz, o una señal del ecosistema), un DESTINO y una CANTIDAD (- a +). La altura se mueve hasta 12 semitonos, el corte 4 octavas, las velocidades x8, el resto todo el rango. Aquí es donde el ecosistema toca el sonido." };
// what each modulation source is (by ModSource index), shown on the route's source box
inline const std::array<L4, 18> modSourceTips {{
    { "No source: the route is off.", "Sem fonte: a via está desligada.", "Pas de source : la voie est coupée.", "Sin fuente: la vía está apagada." },
    { "The amp envelope of each note.", "O envelope amp de cada nota.", "L'enveloppe amp de chaque note.", "La envolvente amp de cada nota." },
    { "The mod envelope of each note.", "O envelope mod de cada nota.", "L'enveloppe mod de chaque note.", "La envolvente mod de cada nota." },
    { "LFO 1, at its own rate and shape.", "O LFO 1, na sua velocidade e forma.", "Le LFO 1, à sa vitesse et sa forme.", "El LFO 1, a su velocidad y forma." },
    { "LFO 2, at its own rate and shape.", "O LFO 2, na sua velocidade e forma.", "Le LFO 2, à sa vitesse et sa forme.", "El LFO 2, a su velocidad y forma." },
    { "The accent of the ecosystem's note: strong notes give more.", "O acento da nota do ecossistema: notas fortes dão mais.", "L'accent de la note de l'écosystème : les notes fortes donnent plus.", "El acento de la nota del ecosistema: las notas fuertes dan más." },
    { "The organism's energy: how alive and active it is now.", "A energia do organismo: o quanto está vivo e ativo agora.", "L'énergie de l'organisme : à quel point il est vivant et actif maintenant.", "La energía del organismo: cuán vivo y activo está ahora." },
    { "How fast the organism swims.", "A velocidade com que o organismo nada.", "La vitesse à laquelle l'organisme nage.", "La velocidad a la que nada el organismo." },
    { "How strongly it is related now (hunt, flight, school, courtship).", "A força da relação em que está agora (caça, fuga, cardume, corte).", "La force de sa relation actuelle (chasse, fuite, banc, parade).", "La fuerza de su relación actual (caza, huida, cardumen, cortejo)." },
    { "The harmonic tension of the whole ecosystem.", "A tensão harmônica do ecossistema inteiro.", "La tension harmonique de tout l'écosystème.", "La tensión armónica de todo el ecosistema." },
    { "Its distance from the centre of the sphere.", "A distância dele ao centro da esfera.", "Sa distance au centre de la sphère.", "Su distancia al centro de la esfera." },
    { "How close it is to the SOFAR channel, where sound carries far.", "O quanto está perto do canal SOFAR, onde o som vai longe.", "Sa proximité du canal SOFAR, où le son porte loin.", "Cuán cerca está del canal SOFAR, donde el sonido llega lejos." },
    { "The water's temperature: from ice to boiling.", "A temperatura da água: do gelo à fervura.", "La température de l'eau : de la glace à l'ébullition.", "La temperatura del agua: del hielo al hervor." },
    { "Its hunger: high when its energy is low.", "A fome: alta quando a energia está baixa.", "Sa faim : haute quand son énergie est basse.", "Su hambre: alta cuando su energía es baja." },
    { "Its age in its life cycle.", "A idade no seu ciclo de vida.", "Son âge dans son cycle de vie.", "Su edad en su ciclo de vida." },
    { "The prey population of the reef (the slow Lotka-Volterra cycles).", "A população de presas do recife (os ciclos lentos de Lotka-Volterra).", "La population de proies du récif (les cycles lents de Lotka-Volterra).", "La población de presas del arrecife (los ciclos lentos de Lotka-Volterra)." },
    { "The health of the habitat, measured on the sound itself (acoustic indices).", "A saúde do habitat, medida no próprio som (índices acústicos).", "La santé de l'habitat, mesurée sur le son lui-même (indices acoustiques).", "La salud del hábitat, medida en el propio sonido (índices acústicos)." },
    { "The room: what the microphone hears when ROOM is on.", "A sala: o que o microfone ouve quando SALA está ligada.", "La salle : ce que le micro entend quand SALLE est active.", "La sala: lo que oye el micrófono cuando SALA está activa." } }};
}

inline const L4 speakers { "SPEAKERS", "CAIXAS", "ENCEINTES", "ALTAVOCES" };
inline const L4 headphones { "HEADPHONES", "FONES", "CASQUE", "AURICULARES" };
inline const L4 listeningTip { "SPEAKERS: the stereo mix. HEADPHONES: each organism heard from where it is in the sphere - left and right, front and back, above and below - through a head (Brown & Duda's binaural model: the head's shadow, the delay between the ears, the pinna). Use it with headphones only.",
                               "CAIXAS: a mistura estéreo. FONES: cada organismo ouvido de onde está na esfera - esquerda e direita, frente e trás, acima e abaixo - através de uma cabeça (o modelo binaural de Brown & Duda: a sombra da cabeça, o atraso entre as orelhas, a orelha externa). Use só com fones.",
                               "ENCEINTES : le mélange stéréo. CASQUE : chaque organisme entendu d'où il est dans la sphère - gauche et droite, devant et derrière, en haut et en bas - à travers une tête (le modèle binaural de Brown & Duda : l'ombre de la tête, le délai entre les oreilles, le pavillon). À utiliser seulement au casque.",
                               "ALTAVOCES: la mezcla estéreo. AURICULARES: cada organismo oído desde donde está en la esfera - izquierda y derecha, delante y detrás, arriba y abajo - a través de una cabeza (el modelo binaural de Brown & Duda: la sombra de la cabeza, el retardo entre los oídos, el pabellón). Úselo solo con auriculares." };
// the REC button (2026-10-21)
inline const L4 recTip { "Records what you hear (after MASTER and MUTE) to a 32-bit float WAV in Music/Aquorbium/<date>/, with a text file beside it: the session seed and the controls. Up to one hour.",
                         "Grava o que você ouve (depois do MASTER e do SILENCIAR) num WAV float de 32 bits em Música/Aquorbium/<data>/, com um texto ao lado: a semente da sessão e os controles. Até uma hora.",
                         "Enregistre ce que vous entendez (après MASTER et MUET) en WAV flottant 32 bits dans Musique/Aquorbium/<date>/, avec un texte à côté : la graine de la session et les commandes. Jusqu'à une heure.",
                         "Graba lo que escucha (después de MASTER y SILENCIAR) en un WAV flotante de 32 bits en Música/Aquorbium/<fecha>/, con un texto al lado: la semilla de la sesión y los controles. Hasta una hora." };
inline const L4 recSaved { "SAVED ", "GRAVADO ", "ENREGISTRÉ ", "GRABADO " };
inline const L4 recFailed { "REC FAILED - check the folder and the free space",
                            "A GRAVAÇÃO FALHOU - confira a pasta e o espaço livre",
                            "L'ENREGISTREMENT A ÉCHOUÉ - vérifiez le dossier et l'espace libre",
                            "LA GRABACIÓN FALLÓ - revise la carpeta y el espacio libre" };
// the status when the audio thread did not get real-time priority (2026-10-26;
// it was "RT não" in every language)
inline const L4 realtimeRefused { "RT no", "RT não", "RT non", "RT no" };
// the session's world in the status (2026-10-26)
inline const std::array<L4, 5> harmonyLanguages {{
    { "functional", "funcional", "fonctionnel", "funcional" },
    { "modal", "modal", "modal", "modal" },
    { "Brazilian", "brasileira", "brésilien", "brasileño" },
    { "impressionist", "impressionista", "impressionniste", "impresionista" },
    { "chromatic", "cromática", "chromatique", "cromático" } }};
// the session's modes (SessionGenesis.h), by their core name
struct ModeName { const char* core; L4 name; };
inline const std::array<ModeName, 10> sessionModes {{
    { "dorian", { "dorian", "dórico", "dorien", "dórico" } },
    { "aeolian", { "aeolian", "eólio", "éolien", "eólico" } },
    { "mixolydian", { "mixolydian", "mixolídio", "mixolydien", "mixolidio" } },
    { "lydian", { "lydian", "lídio", "lydien", "lidio" } },
    { "ionian", { "ionian", "jônio", "ionien", "jónico" } },
    { "phrygian", { "phrygian", "frígio", "phrygien", "frigio" } },
    { "minor pentatonic", { "minor pentatonic", "pentatônica menor", "pentatonique mineure", "pentatónica menor" } },
    { "major pentatonic", { "major pentatonic", "pentatônica maior", "pentatonique majeure", "pentatónica mayor" } },
    { "harmonic minor", { "harmonic minor", "menor harmônica", "mineur harmonique", "menor armónica" } },
    { "hirajoshi", { "hirajoshi", "hirajoshi", "hirajoshi", "hirajoshi" } } }};
inline const L4 polytonal { "polytonal", "politonal", "polytonal", "politonal" };
inline const L4 orbit { "orbit", "órbita", "orbite", "órbita" };
// the VARIA and the opening seed (2026-10-24)
inline const L4 vary { "VARY", "VARIA", "VARIE", "VARÍA" };
inline const L4 varyTip { "VARY: the controls move by themselves over the music's time, slowly and in relation, around where the seed or your hand left them - warmer and quicker at the climax of the piece, settling at rest. The output stage never moves. Move any control and it breathes around your value. Off: everything stays where it is. Each opening also proposes another biome - water, light, population, bodies, which modules wake; the state you left is kept as opening.previousStateV1 in the settings.",
                          "VARIA: os controles se movem sozinhos ao longo da música, devagar e em relação, em torno de onde a semente ou a sua mão os deixou - mais quentes e rápidos no clímax da peça, assentando no repouso. O estágio de saída nunca se move. Mexa num controle e ele passa a respirar em torno do seu valor. Desligado: tudo fica onde está. Cada abertura também propõe outro bioma - água, luz, população, corpos, quais módulos acordam; o estado que você deixou fica guardado como opening.previousStateV1 nas preferências.",
                          "VARIE : les commandes bougent toutes seules au fil de la musique, lentement et en relation, autour de là où la graine ou votre main les a laissées - plus chaudes et rapides au climax de la pièce, apaisées au repos. L'étage de sortie ne bouge jamais. Touchez une commande et elle respire autour de votre valeur. Éteint : tout reste en place. Chaque ouverture propose aussi un autre biome - eau, lumière, population, corps, quels modules s'éveillent ; l'état que vous avez laissé est gardé comme opening.previousStateV1 dans les préférences.",
                          "VARÍA: los controles se mueven solos a lo largo de la música, despacio y en relación, alrededor de donde la semilla o su mano los dejó - más cálidos y rápidos en el clímax de la pieza, asentándose en el reposo. La etapa de salida nunca se mueve. Mueva un control y respirará alrededor de su valor. Apagado: todo queda donde está. Cada apertura también propone otro bioma - agua, luz, población, cuerpos, qué módulos despiertan; el estado que usted dejó queda guardado como opening.previousStateV1 en las preferencias." };
inline const L4 varyIntensityTip { "How far the VARY hand travels: at the left it holds still, in the middle (default) it breathes, at the right it reaches twice as far.",
                                   "Quanto a mão do VARIA anda: à esquerda fica parada, no meio (o padrão) respira, à direita vai duas vezes mais longe.",
                                   "Jusqu'où va la main de VARIE : à gauche elle s'arrête, au milieu (par défaut) elle respire, à droite elle va deux fois plus loin.",
                                   "Cuánto se mueve la mano de VARÍA: a la izquierda queda quieta, en el medio (por defecto) respira, a la derecha llega el doble de lejos." };
// the footer credit (the RASGO family convention, as in the Trioio)
inline const L4 footerCredit { "© LÚCIO DE ARAÚJO  ·  AQUORBIUM 2026  ·  AGPLv3+ LICENSE  ·  v",
                               "© LÚCIO DE ARAÚJO  ·  AQUORBIUM 2026  ·  LICENÇA AGPLv3+  ·  v",
                               "© LÚCIO DE ARAÚJO  ·  AQUORBIUM 2026  ·  LICENCE AGPLv3+  ·  v",
                               "© LÚCIO DE ARAÚJO  ·  AQUORBIUM 2026  ·  LICENCIA AGPLv3+  ·  v" };
inline const L4 audioOnline { "AUDIO ONLINE", "ÁUDIO ATIVO", "AUDIO ACTIF", "AUDIO ACTIVO" };
inline const L4 audioWaiting { "AUDIO WAITING", "ÁUDIO AGUARDANDO", "AUDIO EN ATTENTE", "AUDIO EN ESPERA" };
inline const L4 unsafeBuffer { "UNSAFE BUFFER ", "BUFFER INSEGURO ", "TAMPON RISQUÉ ", "BÚFER INSEGURO " };

// ---- bottom strip and switches ------------------------------------------------
inline const L4 autonomous { "AUTONOMOUS BIOME", "BIOMA AUTÔNOMO", "BIOME AUTONOME", "BIOMA AUTÓNOMO" };
inline const L4 autonomousTip { "The biome lives and sounds by itself; turn off to play it only from outside.",
                                "O bioma vive e soa sozinho; desligue para tocá-lo só de fora.",
                                "Le biome vit et sonne seul ; désactivez pour le jouer seulement de l'extérieur.",
                                "El bioma vive y suena solo; apáguelo para tocarlo solo desde fuera." };
inline const L4 gate { "EXTERNAL ENERGY GATE", "PORTÃO DE ENERGIA EXTERNA", "PORTE D'ÉNERGIE EXTERNE", "PUERTA DE ENERGÍA EXTERNA" };
inline const L4 gateTip { "External notes (MIDI, keyboard) gate the biome's energy.",
                          "Notas externas (MIDI, teclado) liberam a energia do bioma.",
                          "Les notes externes (MIDI, clavier) ouvrent l'énergie du biome.",
                          "Las notas externas (MIDI, teclado) liberan la energía del bioma." };
inline const L4 ecoControls { "ECO CONTROLS", "CONTROLES ECO", "COMMANDES ÉCO", "CONTROLES ECO" };
// the BRAIN nameplate on the wide surface, where its controls live in ECO
// CONTROLS (the author's choice, 2026-10-27)
inline const L4 brainNoteWhat { "CLOCK · COORDINATION", "RELÓGIO · COORDENAÇÃO", "HORLOGE · COORDINATION", "RELOJ · COORDINACIÓN" };
inline const L4 brainNoteWhere { "in ECO CONTROLS ›", "em CONTROLES ECO ›", "dans COMMANDES ÉCO ›", "en CONTROLES ECO ›" };
inline const L4 brainNoteTip { "The Brain's CLOCK and COORDINATION are in ECO CONTROLS: click to open them.", "O RELÓGIO e a COORDENAÇÃO do Cérebro ficam em CONTROLES ECO: clique para abri-los.", "L'HORLOGE et la COORDINATION du Cerveau sont dans COMMANDES ÉCO : cliquez pour les ouvrir.", "El RELOJ y la COORDINACIÓN del Cerebro están en CONTROLES ECO: haga clic para abrirlos." };
inline const L4 aquariumView { "AQUARIUM VIEW", "VER AQUÁRIO", "VUE DE L'AQUARIUM", "VER ACUARIO" };
inline const L4 sceneNameHint { "Scene name (optional)", "Nome da cena (opcional)", "Nom de la scène (facultatif)", "Nombre de la escena (opcional)" };

// ---- scenes -------------------------------------------------------------------
inline const L4 scene { "Scene ", "Cena ", "Scène ", "Escena " };
inline const L4 sceneSaved { "Scene saved", "Cena salva", "Scène enregistrée", "Escena guardada" };
inline const L4 sceneEmpty { "Empty scene", "Cena vazia", "Scène vide", "Escena vacía" };
inline const L4 sceneRecalled { "Scene recalled", "Cena recuperada", "Scène rappelée", "Escena recuperada" };
inline const L4 sceneDefaults { "Initial values; scenes preserved", "Valores iniciais; cenas preservadas",
                                "Valeurs initiales ; scènes préservées", "Valores iniciales; escenas conservadas" };
inline const L4 store { "STORE", "SALVAR", "ENREGISTRER", "GUARDAR" };
inline const L4 recall { "RECALL", "RECUPERAR", "RAPPELER", "RECUPERAR" };
inline const L4 initial { "INITIAL", "INICIAL", "INITIAL", "INICIAL" };
inline const L4 importScene { "IMPORT SCENE", "IMPORTAR CENA", "IMPORTER SCÈNE", "IMPORTAR ESCENA" };
inline const L4 exportScene { "EXPORT SCENE", "EXPORTAR CENA", "EXPORTER SCÈNE", "EXPORTAR ESCENA" };
inline const L4 rename { "RENAME", "RENOMEAR", "RENOMMER", "RENOMBRAR" };
inline const L4 undo { "UNDO", "DESFAZER", "ANNULER", "DESHACER" };
inline const L4 nameTip { "Up to 32 characters; Enter or RENAME confirms. STORE includes the typed name.",
                          "Até 32 caracteres; Enter ou RENOMEAR confirma. SALVAR inclui o nome digitado.",
                          "Jusqu'à 32 caractères ; Entrée ou RENOMMER confirme. ENREGISTRER inclut le nom saisi.",
                          "Hasta 32 caracteres; Enter o RENOMBRAR confirma. GUARDAR incluye el nombre escrito." };

// ---- card states ----------------------------------------------------------------
inline const L4 active { "ACTIVE", "ATIVO", "ACTIF", "ACTIVO" };
inline const L4 bypass { "BYPASS", "BYPASS", "BYPASS", "BYPASS" };
inline const L4 manual { "MANUAL", "MANUAL", "MANUEL", "MANUAL" };
inline const L4 autonomousState { "AUTONOMOUS", "AUTÔNOMO", "AUTONOME", "AUTÓNOMO" };
inline const L4 strikerManual { "STRIKER MANUAL", "STRIKER MANUAL", "STRIKER MANUEL", "STRIKER MANUAL" };
inline const L4 strikerAutonomous { "STRIKER AUTONOMOUS", "STRIKER AUTÔNOMO", "STRIKER AUTONOME", "STRIKER AUTÓNOMO" };
inline const L4 creativeSafety { "CREATIVE + SAFETY", "CRIATIVO + SEGURANÇA", "CRÉATIF + SÉCURITÉ", "CREATIVO + SEGURIDAD" };
inline const L4 safety { "SAFETY ACTIVE", "SEGURANÇA ATIVA", "SÉCURITÉ ACTIVE", "SEGURIDAD ACTIVA" };
inline const L4 structureActive { "STRUCTURE ACTIVE", "ESTRUTURA ATIVA", "STRUCTURE ACTIVE", "ESTRUCTURA ACTIVA" };
inline const L4 memoryActive { "MEMORY ACTIVE", "MEMÓRIA ATIVA", "MÉMOIRE ACTIVE", "MEMORIA ACTIVA" };
inline const L4 formBypass { "FORM BYPASS", "FORMA BYPASS", "FORME BYPASS", "FORMA BYPASS" };
inline const L4 labActive { "LAB ACTIVE ", "LAB ATIVO ", "LAB ACTIF ", "LAB ACTIVO " };
inline const L4 labBypass { "LAB BYPASS", "LAB BYPASS", "LAB BYPASS", "LAB BYPASS" };
inline const L4 native { "NATIVE", "NATIVO", "NATIF", "NATIVO" };
inline const L4 bodyOne { " BODY", " CORPO", " CORPS", " CUERPO" };
inline const L4 bodyMany { " BODIES", " CORPOS", " CORPS", " CUERPOS" };
inline const L4 ecological { " · ECOLOGICAL", " · ECOLÓGICO", " · ÉCOLOGIQUE", " · ECOLÓGICO" };
inline const L4 water { "WATER ", "ÁGUA ", "EAU ", "AGUA " };

// ---- timbre card ------------------------------------------------------------------
inline const L4 body { "BODY", "CORPO", "CORPS", "CUERPO" };
inline const L4 matter { "MATTER", "MATÉRIA", "MATIÈRE", "MATERIA" };
inline const L4 mix { "MIX", "MISTURA", "MÉLANGE", "MEZCLA" };
inline const L4 timbreEcology { "ECOLOGICAL TIMBRE", "TIMBRE ECOLÓGICO", "TIMBRE ÉCOLOGIQUE", "TIMBRE ECOLÓGICO" };
// SUPPORT (2026-10-23): the level of the support species
inline const L4 support { "SUPPORT", "SUSTENTAÇÃO", "SOUTIEN", "APOYO" };
inline const L4 supportTip { "SUPPORT: the level of the three support organisms - GRAIN, MODAL, STRING - against MORPH and PULSAR, from -6 to +24 dB. They sit about 22-26 dB under the foreground; raise it to bring them forward. 0 dB at the start.",
                             "SUSTENTAÇÃO: o nível dos três organismos de sustentação - GRAIN, MODAL, STRING - diante de MORPH e PULSAR, de -6 a +24 dB. Eles ficam uns 22-26 dB abaixo do primeiro plano; suba para trazê-los à frente. 0 dB no início.",
                             "SOUTIEN : le niveau des trois organismes de soutien - GRAIN, MODAL, STRING - face à MORPH et PULSAR, de -6 à +24 dB. Ils sont environ 22-26 dB sous le premier plan ; montez pour les amener devant. 0 dB au départ.",
                             "APOYO: el nivel de los tres organismos de apoyo - GRAIN, MODAL, STRING - frente a MORPH y PULSAR, de -6 a +24 dB. Quedan unos 22-26 dB por debajo del primer plano; súbalo para traerlos al frente. 0 dB al inicio." };
// MEDIDORES, the meters card (2026-10-24)
inline const L4 meters { "METERS", "MEDIDORES", "MESURES", "MEDIDORES" };
inline const L4 metersStatus { "OUTPUT", "SAÍDA", "SORTIE", "SALIDA" };
inline const L4 metersTip { "The output, measured: loudness (BS.1770, momentary, short-term, integrated), the 4x true peak, the safety limiter's reduction, the stereo correlation, and the three feedback taps that the ecosystem can listen to - before the creative MASTER, before the safety limiter, after it.",
                            "A saída, medida: loudness (BS.1770, momentâneo, curto, integrado), o true peak 4x, a redução do limitador de segurança, a correlação estéreo e os três taps de realimentação que o ecossistema pode ouvir - antes do MASTER CRIATIVO, antes do limitador, depois dele.",
                            "La sortie, mesurée : loudness (BS.1770, momentané, court terme, intégré), le true peak 4x, la réduction du limiteur de sécurité, la corrélation stéréo et les trois prises de rétroaction que l'écosystème peut écouter - avant le MASTER CRÉATIF, avant le limiteur, après lui.",
                            "La salida, medida: loudness (BS.1770, momentáneo, corto, integrado), el true peak 4x, la reducción del limitador de seguridad, la correlación estéreo y las tres tomas de realimentación que el ecosistema puede escuchar - antes del MASTER CREATIVO, antes del limitador, después de él." };
inline const std::array<L4, 9> meterRows {{
    { "LUFS M", "LUFS M", "LUFS M", "LUFS M" },
    { "LUFS S", "LUFS S", "LUFS S", "LUFS S" },
    { "LUFS I", "LUFS I", "LUFS I", "LUFS I" },
    { "TRUE PEAK", "TRUE PEAK", "TRUE PEAK", "TRUE PEAK" },
    { "LIMITER", "LIMITADOR", "LIMITEUR", "LIMITADOR" },
    { "CORRELATION", "CORRELAÇÃO", "CORRÉLATION", "CORRELACIÓN" },
    { "PRE-MASTER", "PRÉ-MASTER", "PRÉ-MASTER", "PRE-MÁSTER" },
    { "PRE-SAFETY", "PRÉ-SEGURANÇA", "PRÉ-SÉCURITÉ", "PRE-SEGURIDAD" },
    { "POST-SAFETY", "PÓS-SEGURANÇA", "POST-SÉCURITÉ", "POST-SEGURIDAD" } }};
// the creative MASTER's EQ order (2026-10-23)
// the CÉREBRO, SALA and LUPA cards (2026-10-24)
inline const L4 brainHeading { "BRAIN", "CÉREBRO", "CERVEAU", "CEREBRO" };
inline const L4 roomHeading { "ROOM", "SALA", "SALLE", "SALA" };
inline const L4 lens { "LENS", "LUPA", "LOUPE", "LUPA" };
inline const L4 glass { "GLASS", "VIDRO", "VITRE", "VIDRIO" };
inline const L4 microphone { "MIC", "MIC", "MICRO", "MIC" };
inline const L4 lensFunction { "The lens on the glass", "A lupa no vidro", "La loupe sur la vitre", "La lupa en el vidrio" };
inline const L4 lensTip { "LENS: the shape of the lens you hold on the glass - its radius, its reach into the water, how much it brings forward and how much it blurs the rest. The centre of each knob is the classic lens.",
                          "LUPA: a forma da lupa que você segura no vidro - o raio, o alcance na água, quanto ela traz à frente e quanto desfoca o resto. O centro de cada botão é a lupa clássica.",
                          "LOUPE : la forme de la loupe tenue sur la vitre - son rayon, sa portée dans l'eau, combien elle avance et combien elle floute le reste. Le centre de chaque bouton est la loupe classique.",
                          "LUPA: la forma de la lupa que sostiene en el vidrio - el radio, el alcance en el agua, cuánto adelanta y cuánto desenfoca el resto. El centro de cada control es la lupa clásica." };
inline const L4 eqBefore { "BEFORE", "ANTES", "AVANT", "ANTES" };
inline const L4 eqAfter { "AFTER", "DEPOIS", "APRÈS", "DESPUÉS" };
inline const L4 bodyTip { "This organism's second body: voice (formant song), matter (harmonic → membrane modes), breath, string, water drop, fold, membrane (water drum → skin), rubbed glass, choir, bow (a bowed string) or reed (a clarinet's tube).",
                          "O segundo corpo deste organismo: voz (canto formante), matéria (modos harmônicos → membrana), sopro, corda, gota d'água, dobra, membrana (tambor d'água → pele), vidro friccionado, coro, arco (corda friccionada) ou palheta (o tubo de um clarinete).",
                          "Le second corps de cet organisme : voix (chant formantique), matière (modes harmoniques → membrane), souffle, corde, goutte d'eau, pli, membrane (tambour d'eau → peau), verre frotté, chœur, archet (corde frottée) ou anche (le tube d'une clarinette).",
                          "El segundo cuerpo de este organismo: voz (canto formántico), materia (modos armónicos → membrana), soplo, cuerda, gota de agua, pliegue, membrana (tambor de agua → piel), vidrio frotado, coro, arco (cuerda frotada) o lengüeta (el tubo de un clarinete)." };
inline const L4 matterTip { "MATTER: the character inside the body (the vowels of voice and choir; harmonic → membrane; the air of breath; plucked → bowed; drops → rain; the fold; loose drum → taut skin; the brightness and beating of glass; the bow from sul tasto to sul ponticello; the reed's embouchure, dark to bright).",
                            "MATÉRIA: o caráter dentro do corpo (as vogais da voz e do coro; harmônico → membrana; o ar do sopro; dedilhada → friccionada; gota → chuva; a dobra; tambor solto → pele esticada; o brilho e o batimento do vidro; o arco de sul tasto a sul ponticello; a embocadura da palheta, escura a brilhante).",
                            "MATIÈRE : le caractère à l'intérieur du corps (les voyelles de la voix et du chœur ; harmonique → membrane ; l'air du souffle ; pincée → frottée ; gouttes → pluie ; le pli ; tambour lâche → peau tendue ; l'éclat et le battement du verre ; l'archet de sul tasto à sul ponticello ; l'embouchure de l'anche, sombre à brillante).",
                            "MATERIA: el carácter dentro del cuerpo (las vocales de la voz y del coro; armónico → membrana; el aire del soplo; punteada → frotada; gotas → lluvia; el pliegue; tambor flojo → piel tensa; el brillo y el batido del vidrio; el arco de sul tasto a sul ponticello; la embocadura de la lengüeta, oscura a brillante)." };
inline const L4 mixTip { "MIX: how much of the second body sounds together with the native body.",
                         "MISTURA: quanto do segundo corpo soa junto ao corpo nativo.",
                         "MÉLANGE : quelle part du second corps sonne avec le corps natif.",
                         "MEZCLA: cuánto del segundo cuerpo suena junto al cuerpo nativo." };
inline const L4 timbreEcologyTip { "ECOLOGICAL TIMBRE: how much the ecosystem's life transforms the matter and presence of the bodies (water, hunt, flight, courtship, vigour, health).",
                                   "TIMBRE ECOLÓGICO: o quanto a vida do ecossistema transforma a matéria e a presença dos corpos (a água, a caça, a fuga, a corte, o vigor, a saúde).",
                                   "TIMBRE ÉCOLOGIQUE : combien la vie de l'écosystème transforme la matière et la présence des corps (eau, chasse, fuite, parade, vigueur, santé).",
                                   "TIMBRE ECOLÓGICO: cuánto la vida del ecosistema transforma la materia y la presencia de los cuerpos (agua, caza, huida, cortejo, vigor, salud)." };
inline const std::array<L4, 12> bodies {{
    { "native", "nativo", "natif", "nativo" },
    { "voice", "voz", "voix", "voz" },
    { "matter", "matéria", "matière", "materia" },
    { "breath", "sopro", "souffle", "soplo" },
    { "string", "corda", "corde", "cuerda" },
    { "drop", "gota", "goutte", "gota" },
    { "fold", "dobra", "pli", "pliegue" },
    { "membrane", "membrana", "membrane", "membrana" },
    { "glass", "vidro", "verre", "vidrio" },
    { "choir", "coro", "chœur", "coro" },
    { "bow", "arco", "archet", "arco" },
    { "reed", "palheta", "anche", "lengüeta" }
}};

// ---- ecosystem view and its panel -----------------------------------------------------
inline const L4 ecosystem { "ECOSYSTEM", "ECOSSISTEMA", "ÉCOSYSTÈME", "ECOSISTEMA" };
inline const L4 lensFree { "FREE LENS · press and hold to hear the focus", "LUPA LIVRE · pressione e mantenha para ouvir o foco",
                           "LOUPE LIBRE · maintenez appuyé pour écouter le foyer", "LUPA LIBRE · mantenga pulsado para oír el foco" };
inline const L4 lensActive { "LENS ACTIVE · FOCUS ", "LUPA ATIVA · FOCO ", "LOUPE ACTIVE · FOYER ", "LUPA ACTIVA · FOCO " };
inline const L4 lensDrag { " · drag; Alt: behind", " · arraste; Alt: atrás", " · glissez ; Alt : derrière", " · arrastre; Alt: detrás" };
inline const L4 musicPanel { "MUSIC OF THE ECOSYSTEM", "MÚSICA DO ECOSSISTEMA", "MUSIQUE DE L'ÉCOSYSTÈME", "MÚSICA DEL ECOSISTEMA" };
inline const L4 musicOff { "At zero. Raise MUSIC in the GEOMETRY card: the events will be born from the web of relations - hunt, flight, rivalry, school, courtship, territory - and from the state of the water.",
                           "Em zero. Suba MÚSICA no cartão GEOMETRIA: os acontecimentos passam a nascer da teia de relações - caça, fuga, rivalidade, cardume, corte, território - e do estado da água.",
                           "À zéro. Montez MUSIQUE dans la carte GÉOMÉTRIE : les événements naîtront du réseau de relations - chasse, fuite, rivalité, banc, parade, territoire - et de l'état de l'eau.",
                           "En cero. Suba MÚSICA en la tarjeta GEOMETRÍA: los acontecimientos nacerán de la red de relaciones - caza, huida, rivalidad, cardumen, cortejo, territorio - y del estado del agua." };
inline const L4 tension { "TENSION", "TENSÃO", "TENSION", "TENSIÓN" };
inline const L4 sea { "SEA", "MAR", "MER", "MAR" };
inline const L4 freshWater { "FRESH WATER", "ÁGUA DOCE", "EAU DOUCE", "AGUA DULCE" };
inline const L4 waterTypeTip { "Sea or fresh water: the sea's chemistry (boric acid, magnesium sulphate) absorbs the highs, more as the water warms; fresh water stays clear (Ainslie & McColm).",
                               "Mar ou água doce: a química do mar (ácido bórico, sulfato de magnésio) absorve os agudos, mais quando a água esquenta; a água doce fica límpida (Ainslie & McColm).",
                               "Mer ou eau douce : la chimie de la mer (acide borique, sulfate de magnésium) absorbe les aigus, davantage quand l'eau se réchauffe ; l'eau douce reste claire (Ainslie & McColm).",
                               "Mar o agua dulce: la química del mar (ácido bórico, sulfato de magnesio) absorbe los agudos, más cuando el agua se calienta; el agua dulce queda clara (Ainslie & McColm)." };
inline const L4 niche { " · niche ", " · nicho ", " · niche ", " · nicho " };
// RoomCoupling (optional, off by default)
inline const L4 room { "ROOM", "SALA", "SALLE", "SALA" };
inline const L4 roomTip { "Optional: the microphone becomes the reef's hearing. The room's noise is heard as one more organism - the voices rise over it (Lombard) and leave its register. The aquarium's own sound coming back through the speakers is learned and discounted. Only a level and a register are measured: nothing is recorded, sent or played back.",
                          "Opcional: o microfone vira a escuta do recife. O barulho da sala é ouvido como um organismo a mais - as vozes sobem acima dele (Lombard) e deixam o seu registro. O som do próprio aquário que volta pelas caixas é aprendido e descontado. Só um nível e um registro são medidos: nada é gravado, enviado ou reproduzido.",
                          "Optionnel : le microphone devient l'écoute du récif. Le bruit de la salle est entendu comme un organisme de plus - les voix montent au-dessus (Lombard) et quittent son registre. Le son de l'aquarium qui revient par les enceintes est appris et décompté. Seuls un niveau et un registre sont mesurés : rien n'est enregistré, envoyé ni restitué.",
                          "Opcional: el micrófono se vuelve la escucha del arrecife. El ruido de la sala se oye como un organismo más - las voces suben por encima (Lombard) y dejan su registro. El sonido del propio acuario que vuelve por los altavoces se aprende y se descuenta. Solo se miden un nivel y un registro: nada se graba, envía ni reproduce." };
// the musician's soundmark (Schafer)
inline const L4 mark { "MARK", "MARCA", "EMBLÈME", "MARCA" };
inline const L4 markTip { "Takes the phrase sounding now as this place's SOUNDMARK: it returns after cadences and changes of the water, is not replaced by song revolutions, and is kept for the next sessions. Press again to mark another phrase.",
                          "Toma a frase que soa agora como a MARCA SONORA deste lugar: ela volta depois das cadências e das mudanças da água, não é trocada pelas revoluções de canto e fica guardada para as próximas sessões. Aperte de novo para marcar outra frase.",
                          "Prend la phrase qui sonne maintenant comme l'EMBLÈME SONORE de ce lieu : il revient après les cadences et les changements de l'eau, n'est pas remplacé par les révolutions du chant et reste gardé pour les sessions suivantes. Appuyez de nouveau pour marquer une autre phrase.",
                          "Toma la frase que suena ahora como la MARCA SONORA de este lugar: vuelve tras las cadencias y los cambios del agua, no la reemplazan las revoluciones del canto y queda guardada para las próximas sesiones. Pulse de nuevo para marcar otra frase." };
inline const L4 roomLevel { " · room ", " · sala ", " · salle ", " · sala " };
// the habitat's health, heard (ADI and ACI)
inline const L4 habitat { " · habitat ", " · habitat ", " · habitat ", " · hábitat " };
// Lotka-Volterra abundances on the music panel
inline const L4 prey { "PREY", "PRESAS", "PROIES", "PRESAS" };
inline const L4 predators { "PREDATORS", "PREDADORES", "PRÉDATEURS", "DEPREDADORES" };
inline const std::array<L4, 5> waterStates {{
    { "ICE", "GELO", "GLACE", "HIELO" },
    { "COLD", "FRIA", "FROIDE", "FRÍA" },
    { "TEMPERATE", "TEMPERADA", "TEMPÉRÉE", "TEMPLADA" },
    { "TURBULENT", "TURBULENTA", "TURBULENTE", "TURBULENTA" },
    { "BOILING", "FERVURA", "ÉBULLITION", "HERVOR" }
}};
inline const std::array<L4, 6> modes {{
    { "phrygian", "frígio", "phrygien", "frigio" },
    { "aeolian", "eólio", "éolien", "eólico" },
    { "dorian", "dórico", "dorien", "dórico" },
    { "mixolydian", "mixolídio", "mixolydien", "mixolidio" },
    { "ionian", "jônio", "ionien", "jónico" },
    { "lydian", "lídio", "lydien", "lidio" }
}};
inline const std::array<L4, 6> relations {{
    { "hunt", "caça", "chasse", "caza" },
    { "flight", "fuga", "fuite", "huida" },
    { "rivalry", "rivalidade", "rivalité", "rivalidad" },
    { "school", "cardume", "banc", "cardumen" },
    { "courtship", "corte", "parade", "cortejo" },
    { "territory", "território", "territoire", "territorio" }
}};
inline const std::array<L4, 22> acts {{
    { "grazing", "pastejo", "pâture", "pastoreo" },
    { "hunt", "caça", "chasse", "caza" },
    { "terminal buzz", "buzz terminal", "buzz terminal", "zumbido terminal" },
    { "capture", "captura", "capture", "captura" },
    { "flight", "fuga", "fuite", "huida" },
    { "the prey falls silent", "a presa se cala", "la proie se tait", "la presa calla" },
    { "rivalry", "rivalidade", "rivalité", "rivalidad" },
    { "school", "cardume", "banc", "cardumen" },
    { "duet", "dueto", "duo", "dúo" },
    { "territory defence", "defesa do território", "défense du territoire", "defensa del territorio" },
    { "modulation", "modulação", "modulation", "modulación" },
    { "cadence", "cadência", "cadence", "cadencia" },
    { "change of state", "mudança de estado", "changement d'état", "cambio de estado" },
    { "the glass cracks", "o vidro trinca", "le verre se fend", "el vidrio se agrieta" },
    { "bubbles", "bolhas", "bulles", "burbujas" },
    { "departure", "partida", "départ", "partida" },
    { "bubble net", "rede de bolhas", "filet de bulles", "red de burbujas" },
    { "song revolution", "revolução de canto", "révolution du chant", "revolución del canto" },
    { "the song begins again", "o canto recomeça", "le chant recommence", "el canto recomienza" },
    { "a voice dies", "uma voz morre", "une voix meurt", "una voz muere" },
    { "a voice is born", "uma voz nasce", "une voix naît", "una voz nace" },
    { "the soundmark returns", "a marca sonora volta", "l'emblème sonore revient", "la marca sonora vuelve" }
}};

// ---- audio settings dialog ----------------------------------------------------------
inline const L4 output { "Output", "Saída", "Sortie", "Salida" };
inline const L4 rate { "Rate", "Taxa", "Fréquence", "Frecuencia" };
inline const L4 buffer { "Buffer", "Buffer", "Tampon", "Búfer" };
inline const L4 applyAudio { "APPLY AUDIO", "APLICAR ÁUDIO", "APPLIQUER L'AUDIO", "APLICAR AUDIO" };
inline const L4 samples { " samples", " amostras", " échantillons", " muestras" };
inline const L4 outputFailed { "Could not open the output: ", "Falha ao abrir saída: ", "Impossible d'ouvrir la sortie : ", "No se pudo abrir la salida: " };
}

// ---- every control: name, tip and a deeper Learn text ---------------------------------
struct ControlText
{
    const char* id;
    L4 name;
    L4 tip;
    L4 learn;  // empty fields fall back to the tip
};

inline const std::array<ControlText, 63>& controlTexts()
{
    static const std::array<ControlText, 63> table {{
        { "density", { "DENSITY", "DENSIDADE", "DENSITÉ", "DENSIDAD" },
          { "How much happens: the ecosystem's desire for density.", "Quanto acontece: o desejo de densidade do ecossistema.", "Combien de choses se passent : le désir de densité de l'écosystème.", "Cuánto sucede: el deseo de densidad del ecosistema." },
          { "A tendency, not a command: the organisms move and sing more, the niche admits more events per bar. Combine with DRIFT: dense and calm is a slow crowd; dense and agitated is a storm.",
            "Uma tendência, não uma ordem: os organismos se movem e cantam mais, o nicho admite mais eventos por compasso. Combine com DERIVA: denso e calmo é uma multidão lenta; denso e agitado é uma tempestade.",
            "Une tendance, pas un ordre : les organismes bougent et chantent davantage, la niche admet plus d'événements par mesure. Combinez avec DÉRIVE : dense et calme, une foule lente ; dense et agité, une tempête.",
            "Una tendencia, no una orden: los organismos se mueven y cantan más, el nicho admite más eventos por compás. Combine con DERIVA: denso y calmo es una multitud lenta; denso y agitado, una tormenta." } },
        { "drift", { "DRIFT", "DERIVA", "DÉRIVE", "DERIVA" },
          { "Agitation: left is calm water, right is restless.", "Agitação: à esquerda água calma, à direita inquieta.", "Agitation : à gauche eau calme, à droite agitée.", "Agitación: a la izquierda agua calma, a la derecha inquieta." },
          { "The inverse of calm. Calm slows the tempo, lengthens rests and glides; agitation raises the heat of the water and can push it towards turbulence.",
            "O inverso da calma. A calma desacelera o andamento, alonga pausas e glissandos; a agitação aquece a água e pode levá-la à turbulência.",
            "L'inverse du calme. Le calme ralentit le tempo, allonge silences et glissandos ; l'agitation chauffe l'eau et peut la pousser vers la turbulence.",
            "Lo inverso de la calma. La calma ralentiza el tempo, alarga pausas y glisandos; la agitación calienta el agua y puede llevarla a la turbulencia." } },
        { "ecosystemMorph", { "NEUTRAL ← ECOSYSTEM MORPH → ACTIVE", "NEUTRO ← MORPH DO ECOSSISTEMA → ATIVO", "NEUTRE ← MORPH DE L'ÉCOSYSTÈME → ACTIF", "NEUTRO ← MORPH DEL ECOSISTEMA → ACTIVO" },
          { "From a neutral mix to the ecosystem's full spatial and timbral influence.", "De uma mistura neutra à influência espacial e timbral plena do ecossistema.", "D'un mélange neutre à l'influence spatiale et timbrale pleine de l'écosystème.", "De una mezcla neutra a la plena influencia espacial y tímbrica del ecosistema." }, {} },
        { "cohesion", { "COHESION", "COESÃO", "COHÉSION", "COHESIÓN" },
          { "How tightly the school swims and sings together.", "O quanto o cardume nada e canta junto.", "À quel point le banc nage et chante ensemble.", "Cuánto nada y canta junto el cardumen." },
          { "A cohesive school sings in homorhythm; a loose one in heterophony. It also weighs the alliance bonds in the web.",
            "Um cardume coeso canta em homorritmia; um disperso, em heterofonia. Também pesa os laços de aliança na teia.",
            "Un banc cohésif chante en homorythmie ; un banc lâche en hétérophonie. Elle pèse aussi les liens d'alliance du réseau.",
            "Un cardumen cohesionado canta en homorritmia; uno disperso, en heterofonía. También pesa los lazos de alianza en la red." } },
        { "danger", { "DANGER", "PERIGO", "DANGER", "PELIGRO" },
          { "Fear in the water: sharper accents, more ornaments, more flights.", "Medo na água: acentos mais fortes, mais ornamentos, mais fugas.", "Peur dans l'eau : accents plus forts, plus d'ornements, plus de fuites.", "Miedo en el agua: acentos más fuertes, más ornamentos, más huidas." }, {} },
        { "sparseFocus", { "SPARSE FOCUS", "FOCO ESPARSO", "FOCUS ÉPARS", "FOCO DISPERSO" },
          { "Listening mode: few, focused events.", "Modo de escuta: poucos eventos, focados.", "Mode d'écoute : peu d'événements, focalisés.", "Modo de escucha: pocos eventos, enfocados." }, {} },
        { "energyMemory", { "ENERGY MEMORY", "MEMÓRIA DE ENERGIA", "MÉMOIRE D'ÉNERGIE", "MEMORIA DE ENERGÍA" },
          { "Listening mode: energy remembered and released slowly.", "Modo de escuta: a energia lembrada e liberada devagar.", "Mode d'écoute : l'énergie mémorisée et relâchée lentement.", "Modo de escucha: la energía recordada y liberada despacio." }, {} },
        { "geometricCycle", { "GEOMETRIC CYCLE", "CICLO GEOMÉTRICO", "CYCLE GÉOMÉTRIQUE", "CICLO GEOMÉTRICO" },
          { "Listening mode: the orbits' cycles lead.", "Modo de escuta: os ciclos das órbitas conduzem.", "Mode d'écoute : les cycles des orbites mènent.", "Modo de escucha: los ciclos de las órbitas conducen." }, {} },
        { "collectiveDrift", { "COLLECTIVE DRIFT", "DERIVA COLETIVA", "DÉRIVE COLLECTIVE", "DERIVA COLECTIVA" },
          { "Listening mode: the group drifts together.", "Modo de escuta: o grupo deriva junto.", "Mode d'écoute : le groupe dérive ensemble.", "Modo de escucha: el grupo deriva junto." }, {} },
        { "geometryInfluence", { "GEOMETRY PERFORMANCE", "GEOMETRIA (PERFORMANCE)", "GÉOMÉTRIE (JEU)", "GEOMETRÍA (INTERPRETACIÓN)" },
          { "Zero: autonomous geometry. Raise to enable rhythm, rotation, intervals and mutation; changes appear at the next events.",
            "Zero: geometria autônoma. Aumente para ativar ritmo, rotação, intervalos e mutação; mudanças surgem nos próximos eventos.",
            "Zéro : géométrie autonome. Montez pour activer rythme, rotation, intervalles et mutation ; les changements apparaissent aux prochains événements.",
            "Cero: geometría autónoma. Súbalo para activar ritmo, rotación, intervalos y mutación; los cambios aparecen en los próximos eventos." }, {} },
        { "rhythmDensity", { "RHYTHM DENSITY", "DENSIDADE RÍTMICA", "DENSITÉ RYTHMIQUE", "DENSIDAD RÍTMICA" },
          { "How many steps of each orbit sound.", "Quantos passos de cada órbita soam.", "Combien de pas de chaque orbite sonnent.", "Cuántos pasos de cada órbita suenan." }, {} },
        { "orbitRotation", { "ORBIT ROTATION", "ROTAÇÃO DAS ÓRBITAS", "ROTATION DES ORBITES", "ROTACIÓN DE LAS ÓRBITAS" },
          { "Rotates where each orbit's pattern begins.", "Gira onde começa o padrão de cada órbita.", "Fait tourner le début du motif de chaque orbite.", "Gira dónde empieza el patrón de cada órbita." }, {} },
        { "intervalBias", { "INTERVAL BIAS", "TENDÊNCIA DE INTERVALO", "BIAIS D'INTERVALLE", "SESGO DE INTERVALO" },
          { "Leans the melodic steps towards smaller or wider intervals.", "Inclina os passos melódicos para intervalos menores ou maiores.", "Penche les pas mélodiques vers des intervalles plus petits ou plus larges.", "Inclina los pasos melódicos hacia intervalos menores o mayores." }, {} },
        { "geometryMutation", { "MUTATION", "MUTAÇÃO", "MUTATION", "MUTACIÓN" },
          { "How often the orbits change their shape.", "Com que frequência as órbitas mudam de forma.", "À quelle fréquence les orbites changent de forme.", "Con qué frecuencia las órbitas cambian de forma." }, {} },
        { "formInfluence", { "FORM MIX", "FORMA", "FORME", "FORMA" },
          { "Zero: formal memory transparent. Raise to move section and contour at the next collective events.",
            "Zero: memória formal transparente. Aumente para deslocar seção e contorno nos próximos eventos coletivos.",
            "Zéro : mémoire formelle transparente. Montez pour déplacer section et contour aux prochains événements collectifs.",
            "Cero: memoria formal transparente. Súbalo para desplazar sección y contorno en los próximos eventos colectivos." },
          { "A Markov memory of formal regions: the piece remembers where it has been and tends to return or leave, as a form that breathes.",
            "Uma memória de Markov das regiões formais: a peça lembra onde esteve e tende a voltar ou partir, como uma forma que respira.",
            "Une mémoire de Markov des régions formelles : la pièce se souvient où elle a été et tend à y revenir ou à partir, comme une forme qui respire.",
            "Una memoria de Markov de las regiones formales: la pieza recuerda dónde estuvo y tiende a volver o a partir, como una forma que respira." } },
        { "formContext", { "CONTEXT", "CONTEXTO", "CONTEXTE", "CONTEXTO" },
          { "How much the ecological state attracts the next formal region.", "Quanto o estado ecológico atrai a próxima região formal.", "Combien l'état écologique attire la prochaine région formelle.", "Cuánto el estado ecológico atrae la próxima región formal." }, {} },
        { "formInertia", { "INERTIA", "INÉRCIA", "INERTIE", "INERCIA" },
          { "Tendency to stay in the current formal region.", "Tendência de permanecer na região formal atual.", "Tendance à rester dans la région formelle actuelle.", "Tendencia a permanecer en la región formal actual." }, {} },
        { "formMutation", { "MUTATION", "MUTAÇÃO", "MUTATION", "MUTACIÓN" },
          { "Distributed probability of jumps between formal regions.", "Probabilidade distribuída de saltos entre regiões formais.", "Probabilité distribuée de sauts entre régions formelles.", "Probabilidad distribuida de saltos entre regiones formales." }, {} },
        { "structureInfluence", { "STRUCTURE MIX", "ESTRUTURA", "STRUCTURE", "ESTRUCTURA" },
          { "Zero: previous path intact. Raise to enable step memory and progression.", "Zero: caminho anterior intacto. Aumente para ativar memória de passos e progressão.",
            "Zéro : chemin précédent intact. Montez pour activer la mémoire de pas et la progression.", "Cero: camino anterior intacto. Súbalo para activar la memoria de pasos y la progresión." }, {} },
        { "structureMemory", { "STEP MEMORY", "MEMÓRIA DE PASSOS", "MÉMOIRE DE PAS", "MEMORIA DE PASOS" },
          { "Contribution of the addressable memory to pitch and duration of events.", "Contribuição da memória endereçável para altura e duração dos eventos.",
            "Contribution de la mémoire adressable à la hauteur et à la durée des événements.", "Contribución de la memoria direccionable a la altura y duración de los eventos." }, {} },
        { "structureHarmony", { "HARMONY", "HARMONIA", "HARMONIE", "ARMONÍA" },
          { "Contribution of the weighted chords to the pitch field.", "Contribuição dos acordes ponderados para o campo de alturas.", "Contribution des accords pondérés au champ des hauteurs.", "Contribución de los acordes ponderados al campo de alturas." }, {} },
        { "structurePhase", { "PHASE PATH", "PERCURSO DE FASE", "PARCOURS DE PHASE", "RECORRIDO DE FASE" },
          { "Ecological path: day on the left, season on the right.", "Percurso ecológico: dia à esquerda, estação à direita.", "Parcours écologique : le jour à gauche, la saison à droite.", "Recorrido ecológico: día a la izquierda, estación a la derecha." }, {} },
        { "musicInfluence", { "MUSIC", "MÚSICA", "MUSIQUE", "MÚSICA" },
          { "Zero: previous route intact. Raise so that the music comes from the ecosystem: hunt, flight, courtship, territory, the web of relations.",
            "Zero: rota anterior intacta. Aumente para que a música venha do ecossistema: caça, fuga, corte, território, a teia de relações.",
            "Zéro : route précédente intacte. Montez pour que la musique vienne de l'écosystème : chasse, fuite, parade, territoire, le réseau de relations.",
            "Cero: ruta anterior intacta. Súbalo para que la música venga del ecosistema: caza, huida, cortejo, territorio, la red de relaciones." },
          { "Events are born from what the organisms perceive: a hunter imitates what it heard of its prey, in a canon whose delay is the propagation through the water; the prey flees in diminution; rivals answer in hocket; allies sing together; courting pairs copy each other's song. Compatible relations sound consonant, incompatible ones rub; when a conflict dissolves, the music cadences.",
            "Os acontecimentos nascem do que os organismos percebem: o caçador imita o que ouviu da presa, num cânone cujo atraso é a propagação na água; a presa foge em diminuição; rivais respondem em hoquetus; aliados cantam juntos; o par em corte copia o canto um do outro. Relações compatíveis soam consonantes, incompatíveis friccionam; quando um conflito se dissolve, a música chega a uma cadência.",
            "Les événements naissent de ce que perçoivent les organismes : le chasseur imite ce qu'il a entendu de sa proie, en un canon dont le retard est la propagation dans l'eau ; la proie fuit en diminution ; les rivaux répondent en hoquet ; les alliés chantent ensemble ; le couple en parade copie le chant de l'autre. Les relations compatibles sonnent consonantes, les incompatibles frottent ; quand un conflit se dissout, la musique conclut sur une cadence.",
            "Los acontecimientos nacen de lo que perciben los organismos: el cazador imita lo que oyó de su presa, en un canon cuyo retardo es la propagación en el agua; la presa huye en disminución; los rivales responden en hoquetus; los aliados cantan juntos; la pareja en cortejo copia el canto del otro. Las relaciones compatibles suenan consonantes, las incompatibles friccionan; cuando un conflicto se disuelve, la música cierra en una cadencia." } },
        { "musicReaction", { "REACTION", "REAÇÃO", "RÉACTION", "REACCIÓN" },
          { "How strongly the organisms react to what they see and hear through the water.", "O quanto os organismos reagem ao que veem e ouvem através da água.",
            "La force avec laquelle les organismes réagissent à ce qu'ils voient et entendent à travers l'eau.", "Cuánto reaccionan los organismos a lo que ven y oyen a través del agua." },
          { "Lowers the threshold of perception: more relations form, the web gets busier and tenser.",
            "Baixa o limiar de percepção: mais relações se formam, a teia fica mais movimentada e tensa.",
            "Abaisse le seuil de perception : plus de relations se forment, le réseau devient plus actif et plus tendu.",
            "Baja el umbral de percepción: se forman más relaciones, la red se vuelve más activa y tensa." } },
        { "musicMemory", { "MEMORY", "MEMÓRIA", "MÉMOIRE", "MEMORIA" },
          { "Left: songs repeat (identity). Right: they transform and are copied among allies.", "Esquerda: os cantos se repetem (identidade). Direita: transformam-se e se copiam entre aliados.",
            "Gauche : les chants se répètent (identité). Droite : ils se transforment et se copient entre alliés.", "Izquierda: los cantos se repiten (identidad). Derecha: se transforman y se copian entre aliados." },
          { "Each organism has a signature - a coda and a whistle. Memory decides whether it is repeated or developed (transposed, inverted, fragmented), how fast allies copy units of each other's song, and how often a song revolution sweeps the reef.",
            "Cada organismo tem uma assinatura - uma coda e um assobio. A memória decide se ela se repete ou se desenvolve (transposta, invertida, fragmentada), a velocidade com que aliados copiam unidades do canto um do outro e a frequência das revoluções de canto.",
            "Chaque organisme a une signature - une coda et un sifflement. La mémoire décide si elle se répète ou se développe (transposée, renversée, fragmentée), la vitesse à laquelle les alliés copient des unités du chant de l'autre, et la fréquence des révolutions du chant.",
            "Cada organismo tiene una firma - una coda y un silbido. La memoria decide si se repite o se desarrolla (transpuesta, invertida, fragmentada), la rapidez con que los aliados copian unidades del canto del otro y la frecuencia de las revoluciones del canto." } },
        { "musicWater", { "WATER", "ÁGUA", "EAU", "AGUA" },
          { "Centre: the water follows the season. Left cools towards ice; right heats towards boiling. The change of state takes the time of latent heat.",
            "Centro: a água segue a estação. Esquerda esfria até o gelo; direita aquece até a fervura. A mudança de estado leva o tempo do calor latente.",
            "Centre : l'eau suit la saison. À gauche elle refroidit vers la glace ; à droite elle chauffe vers l'ébullition. Le changement d'état prend le temps de la chaleur latente.",
            "Centro: el agua sigue la estación. A la izquierda se enfría hacia el hielo; a la derecha se calienta hacia el hervor. El cambio de estado lleva el tiempo del calor latente." },
          { "The liquid state is the form: ice is a crystal lattice of pitches, dry and clear; cold water is viscous and legato; temperate water is laminar; turbulence brings rubato and heterophony; boiling brings Minnaert bubbles and a slow, opaque sound. The water also absorbs the highs with distance.",
            "O estado líquido é a forma: o gelo é uma rede cristalina de alturas, seca e clara; a água fria é viscosa e ligada; a temperada é laminar; a turbulência traz rubato e heterofonia; a fervura traz bolhas de Minnaert e um som lento e opaco. A água também absorve os agudos com a distância.",
            "L'état liquide est la forme : la glace est un réseau cristallin de hauteurs, sec et clair ; l'eau froide est visqueuse et liée ; l'eau tempérée est laminaire ; la turbulence apporte rubato et hétérophonie ; l'ébullition apporte les bulles de Minnaert et un son lent et opaque. L'eau absorbe aussi les aigus avec la distance.",
            "El estado líquido es la forma: el hielo es una red cristalina de alturas, seca y clara; el agua fría es viscosa y ligada; la templada es laminar; la turbulencia trae rubato y heterofonía; el hervor trae burbujas de Minnaert y un sonido lento y opaco. El agua también absorbe los agudos con la distancia." } },
        { "strikerInfluence", { "STRIKER MIX", "STRIKER", "STRIKER", "STRIKER" },
          { "Zero: autonomous Striker. Raise to enable MATTER, POP and SCATTER; they answer at the next impacts.", "Zero: Striker autônomo. Aumente para ativar MATÉRIA, POP e DISPERSÃO; respondem nos próximos impactos.",
            "Zéro : Striker autonome. Montez pour activer MATIÈRE, POP et DISPERSION ; ils répondent aux prochains impacts.", "Cero: Striker autónomo. Súbalo para activar MATERIA, POP y DISPERSIÓN; responden en los próximos impactos." }, {} },
        { "strikerMatter", { "MATTER", "MATÉRIA", "MATIÈRE", "MATERIA" },
          { "From short dust to resonant metal.", "Da poeira breve ao metal ressonante.", "De la poussière brève au métal résonant.", "De polvo breve a metal resonante." }, {} },
        { "strikerPopulation", { "POP", "POP", "POP", "POP" },
          { "Number of particles per impact.", "Quantidade de partículas por impacto.", "Nombre de particules par impact.", "Cantidad de partículas por impacto." }, {} },
        { "strikerScatter", { "SCATTER", "DISPERSÃO", "DISPERSION", "DISPERSIÓN" },
          { "Temporal and spectral scatter.", "Dispersão temporal e espectral.", "Dispersion temporelle et spectrale.", "Dispersión temporal y espectral." }, {} },
        { "spaceReturn", { "RETURN", "RETORNO", "RETOUR", "RETORNO" },
          { "Audible amount of the shared space.", "Quantidade audível do espaço compartilhado.", "Quantité audible de l'espace partagé.", "Cantidad audible del espacio compartido." }, {} },
        { "spaceGesture", { "SPACE MIX", "ESPAÇO", "ESPACE", "ESPACIO" },
          { "Zero: autonomous Space. Raise to enable SIZE, SPREAD, TONE and MEMORY. RETURN works independently.", "Zero: Espaço autônomo. Aumente para ativar TAMANHO, ABERTURA, TOM e MEMÓRIA. RETORNO funciona independentemente.",
            "Zéro : Espace autonome. Montez pour activer TAILLE, OUVERTURE, TON et MÉMOIRE. RETOUR fonctionne indépendamment.", "Cero: Espacio autónomo. Súbalo para activar TAMAÑO, APERTURA, TONO y MEMORIA. RETORNO funciona por separado." }, {} },
        { "spaceSize", { "SIZE", "TAMANHO", "TAILLE", "TAMAÑO" },
          { "Time scale of the reflections.", "Escala temporal das reflexões.", "Échelle temporelle des réflexions.", "Escala temporal de los reflejos." }, {} },
        { "spaceSpread", { "SPREAD", "ABERTURA", "OUVERTURE", "APERTURA" },
          { "Lateral opening of the reflections.", "Abertura lateral das reflexões.", "Ouverture latérale des réflexions.", "Apertura lateral de los reflejos." }, {} },
        { "spaceTone", { "TONE", "TOM", "TON", "TONO" },
          { "Spectral opening of the reflections.", "Abertura espectral das reflexões.", "Ouverture spectrale des réflexions.", "Apertura espectral de los reflejos." }, {} },
        { "spaceMemory", { "MEMORY", "MEMÓRIA", "MÉMOIRE", "MEMORIA" },
          { "Persistence of the reflection field.", "Persistência do campo de reflexões.", "Persistance du champ de réflexions.", "Persistencia del campo de reflejos." }, {} },
        { "environmentInfluence", { "ENV MIX", "AMBIENTE", "ENVIRONNEMENT", "AMBIENTE" },
          { "Zero: transparent bypass. Raise to enable filter and atmosphere.", "Zero: bypass transparente. Aumente para ativar filtro e atmosfera.",
            "Zéro : bypass transparent. Montez pour activer filtre et atmosphère.", "Cero: bypass transparente. Súbalo para activar filtro y atmósfera." }, {} },
        { "environmentFilterMix", { "FILTER", "FILTRO", "FILTRE", "FILTRO" },
          { "Dry mix of the eight-band spectral signature.", "Mistura seca da assinatura espectral de oito bandas.", "Mélange sec de la signature spectrale à huit bandes.", "Mezcla seca de la firma espectral de ocho bandas." }, {} },
        { "environmentCentre", { "CENTRE", "CENTRO", "CENTRE", "CENTRO" },
          { "Logarithmic centre of the spectral bank.", "Centro logarítmico do banco espectral.", "Centre logarithmique du banc spectral.", "Centro logarítmico del banco espectral." }, {} },
        { "environmentSpan", { "SPAN", "EXTENSÃO", "ÉTENDUE", "EXTENSIÓN" },
          { "Spread in octaves of the eight bands.", "Extensão em oitavas das oito bandas.", "Étendue en octaves des huit bandes.", "Extensión en octavas de las ocho bandas." }, {} },
        { "environmentResonance", { "RESONANCE", "RESSONÂNCIA", "RÉSONANCE", "RESONANCIA" },
          { "Selectivity of the spectral bands.", "Seletividade das bandas espectrais.", "Sélectivité des bandes spectrales.", "Selectividad de las bandas espectrales." }, {} },
        { "environmentAmbience", { "AMBIENCE", "AMBIÊNCIA", "AMBIANCE", "AMBIENCIA" },
          { "Noise bed guided by season, water and integrity.", "Leito de ruído guiado por estação, água e integridade.", "Lit de bruit guidé par la saison, l'eau et l'intégrité.", "Lecho de ruido guiado por estación, agua e integridad." }, {} },
        { "tailInfluence", { "TAIL MIX", "CAUDA", "QUEUE", "COLA" },
          { "Zero: transparent bypass. Raise to hear the late tail.", "Zero: bypass transparente. Aumente para ouvir a cauda tardia.", "Zéro : bypass transparent. Montez pour entendre la queue tardive.", "Cero: bypass transparente. Súbalo para oír la cola tardía." }, {} },
        { "tailDecay", { "DECAY", "DECAIMENTO", "DÉCROISSANCE", "DECAIMIENTO" },
          { "Length of the synthetic persistence.", "Duração da persistência sintética.", "Durée de la persistance synthétique.", "Duración de la persistencia sintética." }, {} },
        { "tailDamping", { "DAMPING", "AMORTECIMENTO", "AMORTISSEMENT", "AMORTIGUACIÓN" },
          { "Progressive absorption of the high frequencies.", "Absorção progressiva das frequências altas.", "Absorption progressive des hautes fréquences.", "Absorción progresiva de las frecuencias altas." }, {} },
        { "tailNightmare", { "NIGHTMARE", "PESADELO", "CAUCHEMAR", "PESADILLA" },
          { "Twist of the feedback matrix.", "Torção da matriz de realimentação.", "Torsion de la matrice de rétroaction.", "Torsión de la matriz de realimentación." }, {} },
        { "creativeMaster", { "CREATIVE MASTER", "MASTER CRIATIVO", "MASTER CRÉATIF", "MASTER CREATIVO" },
          { "Macro of crossover, multiband dynamics, width and optical glue. Zero is an exact bypass.", "Macro de crossover, dinâmica multibanda, largura e cola óptica. Zero é bypass exato.",
            "Macro de crossover, dynamique multibande, largeur et colle optique. Zéro est un bypass exact.", "Macro de crossover, dinámica multibanda, anchura y pegamento óptico. Cero es un bypass exacto." }, {} },
        { "masterTrim", { "OUTPUT TRIM", "VOLUME DE SAÍDA", "NIVEAU DE SORTIE", "NIVEL DE SALIDA" },
          { "Final technical level: -18 dB to +6 dB; 0.75 is 0 dB.", "Nível técnico final: -18 dB a +6 dB; 0,75 corresponde a 0 dB.", "Niveau technique final : -18 dB à +6 dB ; 0,75 vaut 0 dB.", "Nivel técnico final: -18 dB a +6 dB; 0,75 equivale a 0 dB." }, {} },
        { "masterCeiling", { "CEILING", "TETO", "PLAFOND", "TECHO" },
          { "Limiter ceiling: -6 dB to -0.1 dB; initially -1 dB.", "Teto do limitador: -6 dB a -0,1 dB; inicial em -1 dB.", "Plafond du limiteur : -6 dB à -0,1 dB ; initial -1 dB.", "Techo del limitador: -6 dB a -0,1 dB; inicial -1 dB." }, {} },
        { "masterRelease", { "RELEASE", "RECUPERAÇÃO", "RELÂCHEMENT", "RECUPERACIÓN" },
          { "Limiter recovery: 50 ms to 500 ms; initially 150 ms.", "Recuperação do limitador: 50 ms a 500 ms; inicial em 150 ms.", "Relâchement du limiteur : 50 ms à 500 ms ; initial 150 ms.", "Recuperación del limitador: 50 ms a 500 ms; inicial 150 ms." }, {} },
        { "lensRadius", { "RADIUS", "RAIO", "RAYON", "RADIO" },
          { "LENS: the size of the focused spot on the glass. Centre = as before.", "LUPA: o tamanho do ponto em foco no vidro. Centro = como antes.", "LOUPE : la taille du point net sur la vitre. Centre = comme avant.", "LUPA: el tamaño del punto enfocado en el vidrio. Centro = como antes." }, {} },
        { "lensReach", { "REACH", "ALCANCE", "PORTÉE", "ALCANCE" },
          { "LENS: how deep into the water the lens reaches from the glass. Centre = as before.", "LUPA: quão fundo na água a lupa alcança a partir do vidro. Centro = como antes.", "LOUPE : jusqu'où dans l'eau la loupe porte depuis la vitre. Centre = comme avant.", "LUPA: cuán hondo en el agua llega la lupa desde el vidrio. Centro = como antes." }, {} },
        { "lensMagnification", { "ZOOM", "AMPLIAÇÃO", "GROSSISSEMENT", "AUMENTO" },
          { "LENS: how much the focused organisms come forward. Centre = as before.", "LUPA: quanto os organismos em foco vêm à frente. Centro = como antes.", "LOUPE : combien les organismes nets passent devant. Centre = comme avant.", "LUPA: cuánto avanzan los organismos enfocados. Centro = como antes." }, {} },
        { "lensBlur", { "BLUR", "DESFOQUE", "FLOU", "DESENFOQUE" },
          { "LENS: how far the organisms out of focus fall back. Centre = as before.", "LUPA: quanto os organismos fora de foco recuam. Centro = como antes.", "LOUPE : combien les organismes flous reculent. Centre = comme avant.", "LUPA: cuánto retroceden los organismos desenfocados. Centro = como antes." }, {} },
        { "masterMultiband", { "MULTIBAND", "MULTIBANDA", "MULTIBANDE", "MULTIBANDA" },
          { "CREATIVE MASTER: the multiband dynamics, less or more than the ecology sets. Centre = the ecology alone.", "MASTER CRIATIVO: a dinâmica multibanda, menos ou mais do que a ecologia define. Centro = só a ecologia.", "MASTER CRÉATIF : la dynamique multibande, moins ou plus que l'écologie. Centre = l'écologie seule.", "MASTER CREATIVO: la dinámica multibanda, menos o más que la ecología. Centro = solo la ecología." }, {} },
        { "masterGlue", { "GLUE", "COLA", "COLLE", "PEGAMENTO" },
          { "CREATIVE MASTER: the slow glue compressor, less or more. Centre = the ecology alone.", "MASTER CRIATIVO: a cola lenta, menos ou mais. Centro = só a ecologia.", "MASTER CRÉATIF : la colle lente, moins ou plus. Centre = l'écologie seule.", "MASTER CREATIVO: el pegamento lento, menos o más. Centro = solo la ecología." }, {} },
        { "masterTilt", { "TILT", "INCLINAÇÃO", "INCLINAISON", "INCLINACIÓN" },
          { "CREATIVE MASTER: darker (left) or brighter (right) than the daylight sets. Centre = the ecology alone.", "MASTER CRIATIVO: mais escuro (esquerda) ou mais claro (direita) do que a luz do dia define. Centro = só a ecologia.", "MASTER CRÉATIF : plus sombre (gauche) ou plus clair (droite) que la lumière du jour. Centre = l'écologie seule.", "MASTER CREATIVO: más oscuro (izquierda) o más claro (derecha) que la luz del día. Centro = solo la ecología." }, {} },
        { "masterWidth", { "WIDTH", "LARGURA", "LARGEUR", "ANCHO" },
          { "CREATIVE MASTER: narrower or wider than the cohesion sets (the bass stays mono). Centre = the ecology alone.", "MASTER CRIATIVO: mais estreito ou mais largo do que a coesão define (o grave fica mono). Centro = só a ecologia.", "MASTER CRÉATIF : plus étroit ou plus large que la cohésion (le grave reste mono). Centre = l'écologie seule.", "MASTER CREATIVO: más estrecho o más ancho que la cohesión (el grave queda mono). Centro = solo la ecología." }, {} },
        { "brainClock", { "CLOCK", "RELÓGIO", "HORLOGE", "RELOJ" },
          { "BRAIN: how often the Brain's register turns (0.06 to 0.9 Hz). Centre = as before.", "CÉREBRO: com que frequência o registrador do Cérebro gira (0,06 a 0,9 Hz). Centro = como antes.", "CERVEAU : à quelle fréquence le registre du Cerveau tourne (0,06 à 0,9 Hz). Centre = comme avant.", "CEREBRO: con qué frecuencia gira el registro del Cerebro (0,06 a 0,9 Hz). Centro = como antes." }, {} },
        { "brainCoordination", { "COORDINATION", "COORDENAÇÃO", "COORDINATION", "COORDINACIÓN" },
          { "BRAIN: how much the organisms share one leaning - at 0 each its own, at 1 a school. Zero = as before.", "CÉREBRO: quanto os organismos compartilham uma inclinação - em 0 cada um a sua, em 1 um cardume. Zero = como antes.", "CERVEAU : combien les organismes partagent une inclinaison - à 0 chacun la sienne, à 1 un banc. Zéro = comme avant.", "CEREBRO: cuánto comparten los organismos una inclinación - en 0 cada uno la suya, en 1 un cardumen. Cero = como antes." }, {} },
        { "roomSensitivity", { "SENSITIVITY", "SENSIBILIDADE", "SENSIBILITÉ", "SENSIBILIDAD" },
          { "ROOM: the microphone heard softer or louder, -12 to +12 dB. Centre = as before. Acts with ROOM on.", "SALA: o microfone ouvido mais baixo ou mais alto, -12 a +12 dB. Centro = como antes. Age com a SALA ligada.", "SALLE : le micro entendu plus bas ou plus fort, -12 à +12 dB. Centre = comme avant. Agit avec la SALLE allumée.", "SALA: el micrófono oído más bajo o más alto, -12 a +12 dB. Centro = como antes. Actúa con la SALA encendida." }, {} },
        { "roomReaction", { "REACTION", "REAÇÃO", "RÉACTION", "REACCIÓN" },
          { "ROOM: how strongly the music answers the room. Centre = as before. Acts with ROOM on.", "SALA: com que força a música responde à sala. Centro = como antes. Age com a SALA ligada.", "SALLE : avec quelle force la musique répond à la salle. Centre = comme avant. Agit avec la SALLE allumée.", "SALA: con qué fuerza la música responde a la sala. Centro = como antes. Actúa con la SALA encendida." }, {} },
        { "masterEqOrder", { "EQ ORDER", "ORDEM DO EQ", "ORDRE EQ", "ORDEN EQ" },
          { "Where the CREATIVE MASTER's tilt EQ sits: BEFORE the multiband dynamics (the dynamics react to the tilted sound) or AFTER them (the tilt colours what the dynamics left). It acts only with CREATIVE MASTER above zero.",
            "Onde fica o EQ de inclinação do MASTER CRIATIVO: ANTES da dinâmica multibanda (a dinâmica reage ao som já inclinado) ou DEPOIS dela (a inclinação colore o que a dinâmica deixou). Só age com o MASTER CRIATIVO acima de zero.",
            "Où se place l'égaliseur d'inclinaison du MASTER CRÉATIF : AVANT la dynamique multibande (elle réagit au son déjà incliné) ou APRÈS (l'inclinaison colore ce que la dynamique a laissé). N'agit qu'avec le MASTER CRÉATIF au-dessus de zéro.",
            "Dónde va el EQ de inclinación del MASTER CREATIVO: ANTES de la dinámica multibanda (la dinámica reacciona al sonido ya inclinado) o DESPUÉS (la inclinación colorea lo que la dinámica dejó). Solo actúa con el MASTER CREATIVO por encima de cero." }, {} }
    }};
    return table;
}

inline const ControlText* findControl(const juce::String& id)
{
    for (const auto& c : controlTexts())
        if (id == c.id)
            return &c;
    return nullptr;
}

// the module bank's families: names stay as module names; the details translate
inline const std::array<L4, 16>& moduleDetails()
{
    static const std::array<L4, 16> table {{
        { "Master bypass of the bank: zero keeps the previous instrument exactly", "Bypass mestre do banco: zero preserva exatamente o instrumento anterior", "Bypass maître du banc : zéro conserve exactement l'instrument précédent", "Bypass maestro del banco: cero conserva exactamente el instrumento anterior" },
        { "FM, harmonics, BiomaBass, school, vocoder and contagion", "FM, harmônicos, BiomaBass, cardume, vocoder e contágio", "FM, harmoniques, BiomaBass, banc, vocodeur et contagion", "FM, armónicos, BiomaBass, cardumen, vocoder y contagio" },
        { "Granular, isolator, recent resynthesis and lock-in focus", "Granular, isolador, resíntese recente e foco lock-in", "Granulaire, isolateur, resynthèse récente et focus lock-in", "Granular, aislador, resíntesis reciente y foco lock-in" },
        { "Percussion, stochastic mask and HarmonicRippleVoice", "Percussão, máscara estocástica e HarmonicRippleVoice", "Percussion, masque stochastique et HarmonicRippleVoice", "Percusión, máscara estocástica y HarmonicRippleVoice" },
        { "Fractal, vector, X/Y polygon and FrequencyShifter", "Fractal, vetor, polígono X/Y e FrequencyShifter", "Fractale, vecteur, polygone X/Y et FrequencyShifter", "Fractal, vector, polígono X/Y y FrequencyShifter" },
        { "Lorenz, drifts and bounded self-feedback", "Lorenz, derivas e autorrealimentação limitada", "Lorenz, dérives et auto-rétroaction bornée", "Lorenz, derivas y autorrealimentación acotada" },
        { "Motion, quadrature, inertia and light binaural", "Movimento, quadratura, inércia e binaural leve", "Mouvement, quadrature, inertie et binaural léger", "Movimiento, cuadratura, inercia y binaural ligero" },
        { "MotionGestureRecorder and bank of motion waves", "MotionGestureRecorder e banco de ondas de movimento", "MotionGestureRecorder et banc d'ondes de mouvement", "MotionGestureRecorder y banco de ondas de movimiento" },
        { "Clock, events, resonators and L-system", "Relógio, eventos, ressonadores e L-system", "Horloge, événements, résonateurs et L-system", "Reloj, eventos, resonadores y L-system" },
        { "AccumulatingTransposer and ThresholdStepSelector", "AccumulatingTransposer e ThresholdStepSelector", "AccumulatingTransposer et ThresholdStepSelector", "AccumulatingTransposer y ThresholdStepSelector" },
        { "Branching, reactive thresholds, matrix and Spotlight", "Ramificação, limiares reativos, matriz e Spotlight", "Ramification, seuils réactifs, matrice et Spotlight", "Ramificación, umbrales reactivos, matriz y Spotlight" },
        { "Rotation, layout and shared/reconfigurable pools", "Rotação, layout e pools compartilhados/reconfiguráveis", "Rotation, disposition et pools partagés/reconfigurables", "Rotación, disposición y pools compartidos/reconfigurables" },
        { "Banks, RandomToLoop and cellular automaton", "Bancos, RandomToLoop e autômato celular", "Banques, RandomToLoop et automate cellulaire", "Bancos, RandomToLoop y autómata celular" },
        { "ShiftRegisterMemory and ProbabilityDistribution", "ShiftRegisterMemory e ProbabilityDistribution", "ShiftRegisterMemory et ProbabilityDistribution", "ShiftRegisterMemory y ProbabilityDistribution" },
        { "Evolution, cross-feedback, thresholds and behavioural scenes", "Evolução, realimentação cruzada, limiares e cenas comportamentais", "Évolution, rétroaction croisée, seuils et scènes comportementales", "Evolución, realimentación cruzada, umbrales y escenas de comportamiento" },
        { "Integrity, memory, hunger and an input-less bus", "Integridade, memória, fome e barramento sem entrada", "Intégrité, mémoire, faim et bus sans entrée", "Integridad, memoria, hambre y bus sin entrada" }
    }};
    return table;
}
}
