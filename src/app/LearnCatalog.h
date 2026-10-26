// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo

#pragma once

// LEARN for the cards (the RASGO pattern of Rasgo Modular's Learn, rewritten
// for the Aquorbium): what a card is, how to understand it, what to explore.
// With LEARN on, pointing at a card or one of its controls shows its entry in
// the Learn panel; a control adds its own text (UiLanguage.h, ControlText).

#include "UiLanguage.h"

#include <array>

namespace aquorbium::app
{
struct CardText
{
    const char* id;
    L4 title;
    L4 function;   // one line under the title
    L4 understand; // the concept
    L4 explore;    // something to try
};

inline const std::array<CardText, 11>& cardTexts()
{
    static const std::array<CardText, 11> table {{
        { "brain", { "BRAIN", "CÉREBRO", "CERVEAU", "CEREBRO" },
          { "Uncertainty and state", "Incerteza e estado", "Incertitude et état", "Incertidumbre y estado" },
          { "The bio-energy and the internal decisions of the biome: it needs no external note to live. Its state feeds every other card.",
            "A bioenergia e as decisões internas do bioma: ele não precisa de nota externa para viver. Seu estado alimenta todos os outros cartões.",
            "La bio-énergie et les décisions internes du biome : il n'a besoin d'aucune note externe pour vivre. Son état nourrit toutes les autres cartes.",
            "La bioenergía y las decisiones internas del bioma: no necesita ninguna nota externa para vivir. Su estado alimenta todas las demás tarjetas." },
          { "Leave the biome alone for a few minutes and listen how it moves through day and season.",
            "Deixe o bioma sozinho alguns minutos e ouça como ele atravessa o dia e a estação.",
            "Laissez le biome seul quelques minutes et écoutez comment il traverse le jour et la saison.",
            "Deje el bioma solo unos minutos y escuche cómo atraviesa el día y la estación." } },
        { "geometry", { "GEOMETRY", "GEOMETRIA", "GÉOMÉTRIE", "GEOMETRÍA" },
          { "Form, structure and the ecological music", "Forma, estrutura e música ecológica", "Forme, structure et musique écologique", "Forma, estructura y música ecológica" },
          { "Three layers of composition: FORM (a Markov memory of sections), STRUCTURE (step memory and harmony) and the MUSIC of the ecosystem, where events are born from the web of relations between the organisms and from the state of the water.",
            "Três camadas de composição: FORMA (uma memória de Markov das seções), ESTRUTURA (memória de passos e harmonia) e a MÚSICA do ecossistema, em que os acontecimentos nascem da teia de relações entre os organismos e do estado da água.",
            "Trois couches de composition : FORME (une mémoire de Markov des sections), STRUCTURE (mémoire de pas et harmonie) et la MUSIQUE de l'écosystème, où les événements naissent du réseau de relations entre les organismes et de l'état de l'eau.",
            "Tres capas de composición: FORMA (una memoria de Markov de las secciones), ESTRUCTURA (memoria de pasos y armonía) y la MÚSICA del ecosistema, donde los acontecimientos nacen de la red de relaciones entre los organismos y del estado del agua." },
          { "Raise MUSIC to 1, then move WATER slowly to the left: listen to the music freeze into a crystal; then to the right, into turbulence and boiling.",
            "Suba MÚSICA até 1 e mova ÁGUA devagar para a esquerda: ouça a música congelar em cristal; depois para a direita, até a turbulência e a fervura.",
            "Montez MUSIQUE à 1, puis déplacez EAU lentement vers la gauche : écoutez la musique se figer en cristal ; puis vers la droite, jusqu'à la turbulence et l'ébullition.",
            "Suba MÚSICA a 1 y mueva AGUA despacio a la izquierda: escuche la música congelarse en cristal; luego a la derecha, hasta la turbulencia y el hervor." } },
        { "synthesis", { "SYNTHESIS", "SÍNTESE", "SYNTHÈSE", "SÍNTESIS" },
          { "Organic voices", "Vozes orgânicas", "Voix organiques", "Voces orgánicas" },
          { "The granular Striker: impacts made of particles, from dust to resonant metal.",
            "O Striker granular: impactos feitos de partículas, da poeira ao metal ressonante.",
            "Le Striker granulaire : des impacts faits de particules, de la poussière au métal résonant.",
            "El Striker granular: impactos hechos de partículas, del polvo al metal resonante." },
          { "Raise STRIKER MIX, then move MATTER and SCATTER while the biome plays.", "Suba STRIKER e mova MATÉRIA e DISPERSÃO enquanto o bioma toca.",
            "Montez STRIKER, puis bougez MATIÈRE et DISPERSION pendant que le biome joue.", "Suba STRIKER y mueva MATERIA y DISPERSIÓN mientras el bioma toca." } },
        { "space", { "SPACE", "ESPAÇO", "ESPACE", "ESPACIO" },
          { "Perspective and memory", "Perspectiva e memória", "Perspective et mémoire", "Perspectiva y memoria" },
          { "The shared space where the organisms' sounds reflect: early reflections whose size, opening and tone follow the aquarium.",
            "O espaço compartilhado onde os sons dos organismos refletem: primeiras reflexões cujo tamanho, abertura e tom seguem o aquário.",
            "L'espace partagé où les sons des organismes se réfléchissent : des premières réflexions dont la taille, l'ouverture et le ton suivent l'aquarium.",
            "El espacio compartido donde los sonidos de los organismos se reflejan: primeras reflexiones cuyo tamaño, apertura y tono siguen el acuario." },
          { "Raise SPACE MIX and SIZE for a large hall of water; lower TONE for a deeper, darker space.", "Suba ESPAÇO e TAMANHO para uma grande sala de água; baixe TOM para um espaço mais fundo e escuro.",
            "Montez ESPACE et TAILLE pour une grande salle d'eau ; baissez TON pour un espace plus profond et sombre.", "Suba ESPACIO y TAMAÑO para una gran sala de agua; baje TONO para un espacio más hondo y oscuro." } },
        { "environment", { "ENVIRONMENT", "AMBIENTE", "ENVIRONNEMENT", "AMBIENTE" },
          { "Signature and bed", "Assinatura e leito", "Signature et lit", "Firma y lecho" },
          { "An eight-band spectral signature of the biome and a bed of noise guided by season, water and the glass's integrity.",
            "Uma assinatura espectral de oito bandas do bioma e um leito de ruído guiado pela estação, pela água e pela integridade do vidro.",
            "Une signature spectrale à huit bandes du biome et un lit de bruit guidé par la saison, l'eau et l'intégrité du verre.",
            "Una firma espectral de ocho bandas del bioma y un lecho de ruido guiado por la estación, el agua y la integridad del vidrio." },
          { "Raise ENV MIX and AMBIENCE a little: the aquarium gains its own air.", "Suba AMBIENTE e AMBIÊNCIA um pouco: o aquário ganha um ar próprio.",
            "Montez ENVIRONNEMENT et AMBIANCE un peu : l'aquarium gagne son propre air.", "Suba AMBIENTE y AMBIENCIA un poco: el acuario gana su propio aire." } },
        { "tail", { "TAIL", "CAUDA", "QUEUE", "COLA" },
          { "Synthetic persistence", "Persistência sintética", "Persistance synthétique", "Persistencia sintética" },
          { "A late, diffuse reverberation after the first reflections.", "Uma reverberação tardia e difusa depois das primeiras reflexões.",
            "Une réverbération tardive et diffuse après les premières réflexions.", "Una reverberación tardía y difusa tras las primeras reflexiones." },
          { "With NIGHTMARE the tail twists on itself: use it sparingly, as an event.", "Com PESADELO a cauda se torce sobre si: use com parcimônia, como um acontecimento.",
            "Avec CAUCHEMAR la queue se tord sur elle-même : à utiliser avec parcimonie, comme un événement.", "Con PESADILLA la cola se retuerce: úsela con moderación, como un acontecimiento." } },
        { "master", { "MASTER", "MASTER", "MASTER", "MASTER" },
          { "Output and integrity", "Saída e integridade", "Sortie et intégrité", "Salida e integridad" },
          { "The final level, the safety limiter and an optional creative master (multiband dynamics, width, glue).",
            "O nível final, o limitador de segurança e um master criativo opcional (dinâmica multibanda, largura, cola).",
            "Le niveau final, le limiteur de sécurité et un master créatif optionnel (dynamique multibande, largeur, colle).",
            "El nivel final, el limitador de seguridad y un master creativo opcional (dinámica multibanda, anchura, cola)." },
          { "OUTPUT TRIM is the volume. MUTE and PAUSE are in the header.", "VOLUME DE SAÍDA é o volume. SILENCIAR e PAUSA estão no cabeçalho.",
            "NIVEAU DE SORTIE est le volume. MUET et PAUSE sont dans l'en-tête.", "NIVEL DE SALIDA es el volumen. SILENCIAR y PAUSA están en la cabecera." } },
        { "modules", { "MODULES", "MÓDULOS", "MODULES", "MÓDULOS" },
          { "Integrated bank", "Banco integrado", "Banc intégré", "Banco integrado" },
          { "Every coded prototype, gathered in fifteen playable families: a laboratory. LAB MIX at zero keeps the instrument exactly as it is.",
            "Todos os protótipos codificados, reunidos em quinze famílias tocáveis: um laboratório. LAB MIX em zero mantém o instrumento exatamente como está.",
            "Tous les prototypes codés, réunis en quinze familles jouables : un laboratoire. LAB MIX à zéro garde l'instrument exactement tel quel.",
            "Todos los prototipos codificados, reunidos en quince familias tocables: un laboratorio. LAB MIX en cero mantiene el instrumento tal cual." },
          { "Raise LAB MIX, then one family at a time, and listen what each adds.", "Suba LAB MIX e depois uma família por vez, ouvindo o que cada uma acrescenta.",
            "Montez LAB MIX, puis une famille à la fois, en écoutant ce que chacune ajoute.", "Suba LAB MIX y luego una familia a la vez, escuchando lo que añade cada una." } },
        { "timbre", { "TIMBRE", "TIMBRE", "TIMBRE", "TIMBRE" },
          { "The organisms' bodies", "Corpos dos organismos", "Les corps des organismes", "Los cuerpos de los organismos" },
          { "Each organism can sing with a second body - voice, matter, breath, string, drop, fold, membrane, glass, choir, bow or reed - mixed with its native body. ECOLOGICAL TIMBRE lets the water, the hunt, the flight, the courtship, vigour and health transform them. Below, SYNTHESIS and EDIT open the synthesis device: a voice per organism that you build, played by the ecosystem.",
            "Cada organismo pode cantar com um segundo corpo - voz, matéria, sopro, corda, gota, dobra, membrana, vidro, coro, arco ou palheta - misturado ao corpo nativo. O TIMBRE ECOLÓGICO deixa a água, a caça, a fuga, o acasalamento, o vigor e a saúde transformá-los. Embaixo, SÍNTESE e EDITAR abrem o dispositivo de síntese: uma voz por organismo que você constrói, tocada pelo ecossistema.",
            "Chaque organisme peut chanter avec un second corps - voix, matière, souffle, corde, goutte, pli, membrane, verre, chœur, archet ou anche - mêlé à son corps natif. Le TIMBRE ÉCOLOGIQUE laisse l'eau, la chasse, la fuite, la parade, la vigueur et la santé les transformer. En bas, SYNTHÈSE et ÉDITER ouvrent le dispositif de synthèse : une voix par organisme que vous construisez, jouée par l'écosystème.",
            "Cada organismo puede cantar con un segundo cuerpo - voz, materia, soplo, cuerda, gota, pliegue, membrana, vidrio, coro, arco o lengüeta - mezclado con su cuerpo nativo. El TIMBRE ECOLÓGICO deja que el agua, la caza, la huida, el cortejo, el vigor y la salud los transformen. Abajo, SÍNTESIS y EDITAR abren el dispositivo de síntesis: una voz por organismo que usted construye, tocada por el ecosistema." },
          { "Give MORPH the voice and PULSAR the drop, raise their MIX, then ECOLOGICAL TIMBRE: when the hunter chases, its voice sharpens; when the water boils, the drops thicken.",
            "Dê a voz ao MORPH e a gota ao PULSAR, suba a MISTURA e depois o TIMBRE ECOLÓGICO: quando o caçador persegue, a voz aguça; quando a água ferve, as gotas engrossam.",
            "Donnez la voix au MORPH et la goutte au PULSAR, montez leur MÉLANGE, puis le TIMBRE ÉCOLOGIQUE : quand le chasseur poursuit, sa voix s'aiguise ; quand l'eau bout, les gouttes s'épaississent.",
            "Dé la voz al MORPH y la gota al PULSAR, suba su MEZCLA y luego el TIMBRE ECOLÓGICO: cuando el cazador persigue, su voz se afila; cuando el agua hierve, las gotas se espesan." } },
        { "ecosystem", { "ECOSYSTEM", "ECOSSISTEMA", "ÉCOSYSTÈME", "ECOSISTEMA" },
          { "The aquarium, the web and the music", "O aquário, a teia e a música", "L'aquarium, le réseau et la musique", "El acuario, la red y la música" },
          { "Five organisms live in a sphere of water: they see, hear, hunt, flee, court and defend territory. The coloured lines are the web of relations; the panel below shows the state of the water, the tension, the key and the latest events. Press and hold on the sphere to move the lens and listen closer.",
            "Cinco organismos vivem numa esfera de água: veem, ouvem, caçam, fogem, cortejam e defendem território. As linhas coloridas são a teia de relações; o painel abaixo mostra o estado da água, a tensão, o tom e os últimos acontecimentos. Pressione e mantenha na esfera para mover a lupa e ouvir de perto.",
            "Cinq organismes vivent dans une sphère d'eau : ils voient, entendent, chassent, fuient, paradent et défendent un territoire. Les lignes colorées sont le réseau de relations ; le panneau du bas montre l'état de l'eau, la tension, la tonalité et les derniers événements. Maintenez appuyé sur la sphère pour déplacer la loupe et écouter de près.",
            "Cinco organismos viven en una esfera de agua: ven, oyen, cazan, huyen, cortejan y defienden territorio. Las líneas de colores son la red de relaciones; el panel inferior muestra el estado del agua, la tensión, la tonalidad y los últimos acontecimientos. Mantenga pulsado sobre la esfera para mover la lupa y escuchar de cerca." },
          { "Watch a coral line (a hunt) grow thick, then listen for the terminal buzz and the capture; after it, a cadence.",
            "Observe uma linha coral (uma caça) engrossar e escute o buzz terminal e a captura; depois, uma cadência.",
            "Regardez une ligne corail (une chasse) s'épaissir, puis écoutez le buzz terminal et la capture ; ensuite, une cadence.",
            "Observe una línea coral (una caza) engrosarse y escuche el zumbido terminal y la captura; después, una cadencia." } },
        { "strip", { "ECOLOGY", "ECOLOGIA", "ÉCOLOGIE", "ECOLOGÍA" },
          { "Desire and geometry", "Desejo e geometria", "Désir et géométrie", "Deseo y geometría" },
          { "The lower strip holds the desire of the ecosystem (density, drift, morph) and the geometric sequencer of the orbits. Desire changes tendencies, never writes events.",
            "A faixa de baixo reúne o desejo do ecossistema (densidade, deriva, morph) e o sequenciador geométrico das órbitas. O desejo altera tendências, nunca escreve acontecimentos.",
            "La bande du bas réunit le désir de l'écosystème (densité, dérive, morph) et le séquenceur géométrique des orbites. Le désir change des tendances, il n'écrit jamais d'événements.",
            "La franja inferior reúne el deseo del ecosistema (densidad, deriva, morph) y el secuenciador geométrico de las órbitas. El deseo cambia tendencias, nunca escribe acontecimientos." },
          { "ECO CONTROLS (in the aquarium) opens cohesion, danger and the listening modes, and the scenes.",
            "CONTROLES ECO (no aquário) abre coesão, perigo, os modos de escuta e as cenas.",
            "COMMANDES ÉCO (dans l'aquarium) ouvre cohésion, danger, les modes d'écoute et les scènes.",
            "CONTROLES ECO (en el acuario) abre cohesión, peligro, los modos de escucha y las escenas." } }
    }};
    return table;
}

inline const CardText* findCard(const juce::String& id)
{
    for (const auto& c : cardTexts())
        if (id == c.id)
            return &c;
    return nullptr;
}

inline const L4 learnEmpty { "LEARN · point at a control or a card", "LEARN · aponte para um controle ou cartão",
                             "LEARN · pointez une commande ou une carte", "LEARN · apunte a un control o una tarjeta" };
inline const L4 tryThis { "Try: ", "Experimente: ", "Essayez : ", "Pruebe: " };
}
