#!/usr/bin/env python3
"""Gera as quatro páginas do site do Aquorbium (index.html em português, a
canônica, e en/fr/es.html) a partir de um modelo único e dos textos de cada
idioma, para as quatro nunca divergirem na estrutura. Depois, `python3
seo.py` injeta os metadados (o bloco SEO não é escrito aqui).

    python3 gerar_paginas.py && python3 seo.py

Editar os TEXTOS aqui, não nos .html (eles são derivados). O formato segue o
do site do Trioio (lucioaraujo/trioio, commit 383c009, website/); o modelo e
os textos são do Aquorbium, e o sonar vivo e a semente da visita
(assets/sonar.js) são próprios desta página.
"""

import html
import json
import pathlib

AQUI = pathlib.Path(__file__).parent
PORTAL = "https://rasgoinstruments.arquiviagem.net/"
REPO = "https://github.com/lucioaraujo/aquorbium"
BLOB = "https://github.com/lucioaraujo/aquorbium/blob/main"
# a versão e os pacotes do release (a CI: .github/workflows/package.yml)
VERSAO = "0.1.0"
# enquanto a v0.1.0 não sai: noindex e o selo "em preparação"; os botões
# levam à página de releases. No lançamento: PUBLICADO = True.
PUBLICADO = True
RELEASES = REPO + "/releases"
DL = REPO + "/releases/download/v" + VERSAO + "/"
PACOTES = [
    ("Windows", [("aquorbium-%s-win64.exe" % VERSAO, "exe"), ("aquorbium-%s-win64.zip" % VERSAO, "zip")]),
    ("macOS", [("aquorbium-%s-Darwin.dmg" % VERSAO, "dmg")]),
    ("Linux", [("aquorbium-%s-Linux.deb" % VERSAO, "deb"), ("aquorbium-%s-linux-x86_64.AppImage" % VERSAO, "AppImage"),
               ("aquorbium-%s-linux-x86_64.tar.gz" % VERSAO, "tar.gz")]),
]

LINGUAS = [("pt-BR", "PT"), ("en", "EN"), ("fr", "FR"), ("es", "ES")]
# cada página nas quatro línguas (o menu e a troca de língua levam à mesma
# página na outra língua; seo.py tem os mesmos grupos)
ARQUIVOS = {
    "sobre": {"pt-BR": "index.html", "en": "en.html", "fr": "fr.html", "es": "es.html"},
    "tutorial": {"pt-BR": "tutorial.html", "en": "tutorial-en.html", "fr": "tutorial-fr.html", "es": "tutorial-es.html"},
    "instalacao": {"pt-BR": "instalacao.html", "en": "instalacao-en.html", "fr": "instalacao-fr.html", "es": "instalacao-es.html"},
    "guia": {"pt-BR": "como-tocar.html", "en": "como-tocar-en.html", "fr": "como-tocar-fr.html", "es": "como-tocar-es.html"},
    "ideia": {"pt-BR": "ideia.html", "en": "ideia-en.html", "fr": "ideia-fr.html", "es": "ideia-es.html"},
}
MENU = {
    "pt-BR": {"guia": "Como tocar", "ideia": "A ideia", "sobre": "Sobre", "tutorial": "Tutorial", "instalacao": "Instalação", "baixar": "Baixar", "nav": "Páginas"},
    "en": {"guia": "How to play", "ideia": "The idea", "sobre": "About", "tutorial": "Tutorial", "instalacao": "Installation", "baixar": "Download", "nav": "Pages"},
    "fr": {"guia": "Comment en jouer", "ideia": "L'idée", "sobre": "À propos", "tutorial": "Tutoriel", "instalacao": "Installation", "baixar": "Télécharger", "nav": "Pages"},
    "es": {"guia": "Cómo tocarlo", "ideia": "La idea", "sobre": "Acerca de", "tutorial": "Tutorial", "instalacao": "Instalación", "baixar": "Descargar", "nav": "Páginas"},
}
# ---- os textos das páginas "Como tocar" e "A ideia" --------------------------
GUIA = {
    "pt-BR": {
        "title": "Aquorbium — como tocar", "desc": "Como tocar o Aquorbium, cartão por cartão: a esfera, a faixa de baixo, o cérebro, a geometria, o timbre e o dispositivo de síntese, o espaço, o ambiente, a cauda, o master, os módulos, a lupa e os medidores.",
        "h1": "Como tocar", "lead": "Um passeio pela tela, cartão por cartão. Cada parte tem uma imagem do app tocando, o que ela faz e o que cada controle faz. Os textos são os mesmos do LEARN, dentro do app.",
        "controls": "Os controles", "try": "Experimente:", "screen_h": "A tela",
        "screen": ["No centro, a esfera de água com os cinco organismos; em volta, os cartões, numerados de 01 a 11; embaixo, a faixa do desejo; em cima, o cabeçalho.",
                   "O primeiro controle de cada cartão é a porta: a quantidade daquele módulo na mistura. Em zero, o módulo está desligado e os outros controles ficam cinzas. Suba a porta primeiro, depois mexa nos outros. Tudo é lento de propósito: dê um minuto a cada gesto."],
        "extra": {
            "editor": ("O dispositivo de síntese", "Uma voz que você constrói, para cada organismo", "TIMBRE → EDITAR abre um sintetizador por organismo: duas fontes (oscilador, ruído, FM, wavetable, aditiva, pulsar, corda, matéria, caos, realimentação ou um dos corpos), RING e FM entre elas, um filtro, dois envelopes e dois LFOs. O ecossistema toca as notas; na MODULAÇÃO, a vida do bioma - fome, tensão, distância, a temperatura da água - move os parâmetros que você liga.", "PROPOR TODOS, e SÍNTESE pela metade; depois mude uma via de cada vez e escute o que o ecossistema faz com ela. O tutorial tem cinco lições sobre o dispositivo."),
            "lens": ("Lupa", "A lupa no vidro", "Pressione e mantenha sobre a esfera para encostar a lupa no vidro e ouvir de perto os seres que passam ali; o resto fica fora de foco. Os quatro botões dão a forma da lupa.", "Siga um organismo com a lupa enquanto ele atravessa a esfera."),
            "meters": ("Medidores", "A saída, medida", "Loudness (LUFS momentâneo, curto e integrado), o true peak, a redução do limitador de segurança, a correlação estéreo e as três tomadas que o ecossistema pode ouvir: antes do MASTER CRIATIVO, antes do limitador e depois dele.", ""),
            "header": ("O cabeçalho", "Gravar, silenciar, ouvir e aprender", "O volume de saída, o REC, SILENCIAR e PAUSA, CAIXAS ou FONES, a escolha do ÁUDIO, o LEARN, o idioma, o TUTORIAL e o SOBRE. A linha de status mostra o mundo da sessão e a semente.", "Ligue o LEARN e passe o mouse sobre qualquer controle."),
        },
    },
    "en": {
        "title": "Aquorbium — how to play", "desc": "How to play the Aquorbium, card by card: the sphere, the bottom strip, the brain, geometry, timbre and the synthesis device, space, environment, tail, master, the modules, the lens and the meters.",
        "h1": "How to play", "lead": "A walk across the screen, card by card. Each part has a picture of the app playing, what it does and what each control does. The texts are the same as LEARN, inside the app.",
        "controls": "The controls", "try": "Try:", "screen_h": "The screen",
        "screen": ["In the centre, the sphere of water with the five organisms; around it, the cards, numbered 01 to 11; below, the strip of desire; above, the header.",
                   "The first control of each card is its door: that module's amount in the mix. At zero the module is off and the other controls are greyed out. Raise the door first, then move the others. Everything is slow on purpose: give each gesture a minute."],
        "extra": {
            "editor": ("The synthesis device", "A voice you build, for each organism", "TIMBRE → EDIT opens a synthesiser per organism: two sources (oscillator, noise, FM, wavetable, additive, pulsar, string, matter, chaos, feedback or one of the bodies), RING and FM between them, a filter, two envelopes and two LFOs. The ecosystem plays the notes; in MODULATION, the biome's life - hunger, tension, distance, the water's temperature - moves the parameters you route.", "PROPOSE ALL, and SYNTHESIS at half; then change one route at a time and listen to what the ecosystem does with it. The tutorial has five lessons on the device."),
            "lens": ("Lens", "The lens on the glass", "Press and hold on the sphere to put the lens against the glass and listen closely to the creatures passing there; the rest goes out of focus. The four knobs give the lens its shape.", "Follow an organism with the lens as it crosses the sphere."),
            "meters": ("Meters", "The output, measured", "Loudness (momentary, short-term and integrated LUFS), the true peak, the safety limiter's reduction, the stereo correlation and the three taps the ecosystem can listen to: before the CREATIVE MASTER, before the limiter and after it.", ""),
            "header": ("The header", "Record, mute, listen and learn", "The output volume, REC, MUTE and PAUSE, SPEAKERS or HEADPHONES, the AUDIO device, LEARN, the language, TUTORIAL and ABOUT. The status line shows the session's world and its seed.", "Turn LEARN on and hover over any control."),
        },
    },
    "fr": {
        "title": "Aquorbium — comment en jouer", "desc": "Comment jouer de l'Aquorbium, carte par carte : la sphère, la bande du bas, le cerveau, la géométrie, le timbre et le dispositif de synthèse, l'espace, l'environnement, la queue, le master, les modules, la loupe et les mesures.",
        "h1": "Comment en jouer", "lead": "Une promenade sur l'écran, carte par carte. Chaque partie a une image de l'app en jeu, ce qu'elle fait et ce que fait chaque commande. Les textes sont ceux de LEARN, dans l'app.",
        "controls": "Les commandes", "try": "Essayez :", "screen_h": "L'écran",
        "screen": ["Au centre, la sphère d'eau avec les cinq organismes ; autour, les cartes, numérotées de 01 à 11 ; en bas, la bande du désir ; en haut, l'en-tête.",
                   "La première commande de chaque carte est sa porte : la part de ce module dans le mélange. À zéro, le module est éteint et les autres commandes sont grisées. Montez d'abord la porte, puis touchez les autres. Tout est lent à dessein : accordez une minute à chaque geste."],
        "extra": {
            "editor": ("Le dispositif de synthèse", "Une voix que vous construisez, pour chaque organisme", "TIMBRE → ÉDITER ouvre un synthétiseur par organisme : deux sources (oscillateur, bruit, FM, table d'ondes, additive, pulsar, corde, matière, chaos, rétroaction ou l'un des corps), RING et FM entre elles, un filtre, deux enveloppes et deux LFO. L'écosystème joue les notes ; dans MODULATION, la vie du biome - la faim, la tension, la distance, la température de l'eau - bouge les paramètres que vous reliez.", "PROPOSER TOUS, et SYNTHÈSE à moitié ; puis changez une voie à la fois et écoutez ce que l'écosystème en fait. Le tutoriel a cinq leçons sur le dispositif."),
            "lens": ("Loupe", "La loupe sur la vitre", "Maintenez appuyé sur la sphère pour poser la loupe contre la vitre et écouter de près les êtres qui passent là ; le reste devient flou. Les quatre boutons donnent la forme de la loupe.", "Suivez un organisme avec la loupe pendant qu'il traverse la sphère."),
            "meters": ("Mesures", "La sortie, mesurée", "Le loudness (LUFS momentané, court terme et intégré), le true peak, la réduction du limiteur de sécurité, la corrélation stéréo et les trois prises que l'écosystème peut écouter : avant le MASTER CRÉATIF, avant le limiteur et après lui.", ""),
            "header": ("L'en-tête", "Enregistrer, couper, écouter et apprendre", "Le volume de sortie, REC, MUET et PAUSE, ENCEINTES ou CASQUE, le choix de l'AUDIO, LEARN, la langue, le TUTORIEL et À PROPOS. La ligne d'état montre le monde de la session et sa graine.", "Activez LEARN et passez la souris sur n'importe quelle commande."),
        },
    },
    "es": {
        "title": "Aquorbium — cómo tocarlo", "desc": "Cómo tocar el Aquorbium, tarjeta por tarjeta: la esfera, la franja de abajo, el cerebro, la geometría, el timbre y el dispositivo de síntesis, el espacio, el ambiente, la cola, el master, los módulos, la lupa y los medidores.",
        "h1": "Cómo tocarlo", "lead": "Un paseo por la pantalla, tarjeta por tarjeta. Cada parte tiene una imagen de la app sonando, lo que hace y lo que hace cada control. Los textos son los mismos de LEARN, dentro de la app.",
        "controls": "Los controles", "try": "Pruebe:", "screen_h": "La pantalla",
        "screen": ["En el centro, la esfera de agua con los cinco organismos; alrededor, las tarjetas, numeradas de 01 a 11; abajo, la franja del deseo; arriba, la cabecera.",
                   "El primer control de cada tarjeta es su puerta: la cantidad de ese módulo en la mezcla. En cero, el módulo está apagado y los demás controles quedan en gris. Suba primero la puerta, después mueva los demás. Todo es lento a propósito: dé un minuto a cada gesto."],
        "extra": {
            "editor": ("El dispositivo de síntesis", "Una voz que usted construye, para cada organismo", "TIMBRE → EDITAR abre un sintetizador por organismo: dos fuentes (oscilador, ruido, FM, tabla de ondas, aditiva, pulsar, cuerda, materia, caos, realimentación o uno de los cuerpos), RING y FM entre ellas, un filtro, dos envolventes y dos LFO. El ecosistema toca las notas; en MODULACIÓN, la vida del bioma - el hambre, la tensión, la distancia, la temperatura del agua - mueve los parámetros que conecte.", "PROPONER TODOS, y SÍNTESIS a la mitad; luego cambie una vía a la vez y escuche lo que el ecosistema hace con ella. El tutorial tiene cinco lecciones sobre el dispositivo."),
            "lens": ("Lupa", "La lupa en el vidrio", "Mantenga pulsado sobre la esfera para apoyar la lupa en el vidrio y escuchar de cerca a los seres que pasan por allí; el resto queda desenfocado. Los cuatro controles dan la forma de la lupa.", "Siga a un organismo con la lupa mientras cruza la esfera."),
            "meters": ("Medidores", "La salida, medida", "El loudness (LUFS momentáneo, corto e integrado), el true peak, la reducción del limitador de seguridad, la correlación estéreo y las tres tomas que el ecosistema puede escuchar: antes del MASTER CREATIVO, antes del limitador y después de él.", ""),
            "header": ("La cabecera", "Grabar, silenciar, escuchar y aprender", "El volumen de salida, REC, SILENCIAR y PAUSA, ALTAVOCES o AURICULARES, la elección del AUDIO, LEARN, el idioma, el TUTORIAL y ACERCA DE. La línea de estado muestra el mundo de la sesión y su semilla.", "Active LEARN y pase el ratón sobre cualquier control."),
        },
    },
}

IDEIA = {
    "pt-BR": {
        "title": "Aquorbium — a ideia", "desc": "Como o Aquorbium foi concebido: o bioma como conceito, a perspectiva parcial, a composição generativa a partir da ecologia, as referências e a licença.",
        "h1": "A ideia", "lead": "Do brainstorm ao instrumento: de onde o Aquorbium veio, o que ele pensa, como compõe, a quem ele deve e com que liberdade ele é seu.",
        "sections": [
            ("origem", "De onde veio", [
                "O Aquorbium começou como uma conversa de pesquisa sobre a ideia de um bioma: um instrumento que fosse um ecossistema vivo, em que os módulos se influenciam uns aos outros, em vez de um caminho em linha reta de uma nota até o som. As primeiras linhas de código são de 9 de agosto de 2026: um núcleo de bioma determinístico, um padrão de excelência para o instrumento e um laboratório para cada módulo.",
                "O nome também foi pesquisado. Passaram pela mesa Aquarium, Spherium, Squarium, Aquarical, Orbarium e Orquarium, cada um descartado por ser genérico, por colidir com um produto existente ou com outra palavra. Aquorbium - aqu- (água) + orb (a esfera) + -ium - ficou limpo e descreve o que o motor faz por dentro: um aquário esférico, com seres, observadores e o próprio vidro. A forma curta é Aquorb.",
                "Dali em diante o instrumento cresceu por coortes de módulos, cada um com pesquisa, protótipo isolado, testes e escuta antes de entrar no motor: o cérebro de incerteza, o sequenciador geométrico, o Striker granular, o espaço, a cauda, o master, a música ecológica, os corpos, o dispositivo de síntese, a gênese da sessão. Mais de cem commits, uma suíte de mais de oitenta testes e uma regra: nenhum código, som ou texto de terceiros, só ideias reescritas, com a fonte citada.",
            ]),
            ("conceitos", "O que ele pensa", [
                "A coluna vertebral do Aquorbium é uma frase: nenhuma entidade tem visão total. O organismo vê e ouve só o que a água deixa passar; quem escuta de fora tem o viés do seu modo de escuta; e com a lupa, ver mais de perto num lugar é ver pior em outro.",
                "Disso saem quatro consequências. Desejo sem controle direto: você muda tendências, não escreve acontecimentos. Tempo narrativo: o aquário tem manhã e noite, estações, épocas de caça e de corte, e a forma nasce dessa lógica, sem um sequenciador dizendo que entra a seção B. Consequência: sustentar a tensão tem custo, e o vidro pode trincar. E foco com custo: a lupa aproxima uma região e desfoca o resto.",
            ]),
            ("generativa", "Composição generativa", [
                "Na composição generativa, o compositor não escreve as notas uma a uma: escreve as regras, os seres e o mundo de onde as notas nascem, e deixa a música acontecer. É uma tradição que vem dos jogos de dados musicais do século XVIII e passa por Xenakis, pelas peças de Eno, pela vida artificial de McCormack, pela música de enxame de Blackwell e pela ecossistêmica audível de Di Scipio.",
                "No Aquorbium, as regras são ecológicas. Uma caça vira um cânone cujo atraso é a viagem do som pela água; rivais respondem em hoquetus; um casal em corte copia o canto do outro; um conflito acumula tensão harmônica e a sua dissolução vira cadência. Os cantos passam de organismo a organismo, como os das baleias, e às vezes uma revolução de canto troca o repertório do recife inteiro.",
                "Cada abertura é uma peça nova: a semente sorteia uma tonalidade e um modo, uma linguagem harmônica que modula, um arco de tensão de quatro a oito minutos e um bioma - a água, a luz, a população, quais módulos acordam. A semente não é um sorteio cego de botões: é uma hipótese coerente, um lugar de onde começar. E o VARIA move os controles ao longo da música, como uma mão lenta. Você não é substituído: o instrumento propõe, você toca.",
            ]),
        ],
        "refs_h": "Referências e créditos",
        "license_h": "Licença",
        "license": ["O Aquorbium é software livre, sob a GNU Affero General Public License, versão 3 ou posterior (AGPL-3.0-or-later). Você pode usá-lo, estudá-lo, modificá-lo e redistribuí-lo, inclusive comercialmente, desde que as versões modificadas que você distribuir ou ofereça pela rede também sejam livres, sob a mesma licença, com o código-fonte disponível.",
                    "O JUCE é usado sob a sua opção AGPLv3. As fontes do site (DejaVu) são livres. A música que você fizer com o Aquorbium é sua."],
    },
    "en": {
        "title": "Aquorbium — the idea", "desc": "How the Aquorbium was conceived: the biome as a concept, partial perspective, generative composition from ecology, the references and the licence.",
        "h1": "The idea", "lead": "From brainstorm to instrument: where the Aquorbium came from, what it thinks, how it composes, whom it owes and how freely it is yours.",
        "sections": [
            ("origem", "Where it came from", [
                "The Aquorbium began as a research conversation around the idea of a biome: an instrument that would be a living ecosystem, whose modules influence each other, instead of a straight path from a note to a sound. The first lines of code date from 9 August 2026: a deterministic biome core, a standard of excellence for the instrument and a laboratory for each module.",
                "The name was researched too. Aquarium, Spherium, Squarium, Aquarical, Orbarium and Orquarium were all on the table, each dropped for being generic, for clashing with an existing product or with another word. Aquorbium - aqu- (water) + orb (the sphere) + -ium - came out clean and describes what the engine does inside: a spherical aquarium, with creatures, observers and the glass itself. The short form is Aquorb.",
                "From there the instrument grew in cohorts of modules, each with research, an isolated prototype, tests and listening before it entered the engine: the brain of uncertainty, the geometric sequencer, the granular Striker, space, tail, master, the ecological music, the bodies, the synthesis device, the session's genesis. Over a hundred commits, a suite of more than eighty tests and one rule: no third-party code, sound or text, only ideas rewritten, with their source cited.",
            ]),
            ("conceitos", "What it thinks", [
                "The Aquorbium's backbone is one sentence: no entity has the whole view. An organism sees and hears only what the water lets through; whoever listens from outside has the bias of their listening mode; and with the lens, seeing closer in one place means seeing worse in another.",
                "Four consequences follow. Desire without direct control: you change tendencies, you do not write events. Narrative time: the aquarium has morning and night, seasons, times of hunting and of courtship, and the form is born from that logic, with no sequencer announcing section B. Consequence: holding tension has a cost, and the glass can crack. And focus at a cost: the lens brings one region close and blurs the rest.",
            ]),
            ("generativa", "Generative composition", [
                "In generative composition the composer does not write the notes one by one: they write the rules, the beings and the world the notes are born from, and let the music happen. It is a tradition that runs from the musical dice games of the eighteenth century through Xenakis, Eno's pieces, McCormack's artificial life, Blackwell's swarm music and Di Scipio's audible ecosystemics.",
                "In the Aquorbium the rules are ecological. A hunt becomes a canon whose delay is the sound's travel through the water; rivals answer in hocket; a courting pair copies each other's song; a conflict builds harmonic tension and its dissolution becomes a cadence. Songs pass from organism to organism, like the songs of whales, and now and then a song revolution changes the whole reef's repertoire.",
                "Every opening is another piece: the seed draws a key and a mode, a harmonic language that modulates, an arc of tension of four to eight minutes and a biome - the water, the light, the population, which modules wake. The seed is not a blind shuffle of knobs: it is a coherent hypothesis, a place to start from. And VARY moves the controls over the music's time, like a slow hand. You are not replaced: the instrument proposes, you play.",
            ]),
        ],
        "refs_h": "References and credits",
        "license_h": "Licence",
        "license": ["The Aquorbium is free software, under the GNU Affero General Public License, version 3 or later (AGPL-3.0-or-later). You may use it, study it, modify it and redistribute it, commercially too, provided the modified versions you distribute or offer over a network are also free, under the same licence, with their source code available.",
                    "JUCE is used under its AGPLv3 option. The site's fonts (DejaVu) are free. The music you make with the Aquorbium is yours."],
    },
    "fr": {
        "title": "Aquorbium — l'idée", "desc": "Comment l'Aquorbium a été conçu : le biome comme concept, la perspective partielle, la composition générative à partir de l'écologie, les références et la licence.",
        "h1": "L'idée", "lead": "Du brainstorm à l'instrument : d'où vient l'Aquorbium, ce qu'il pense, comment il compose, à qui il doit et avec quelle liberté il est à vous.",
        "sections": [
            ("origem", "D'où il vient", [
                "L'Aquorbium a commencé par une conversation de recherche autour de l'idée d'un biome : un instrument qui serait un écosystème vivant, dont les modules s'influencent les uns les autres, au lieu d'un chemin en ligne droite d'une note jusqu'au son. Les premières lignes de code datent du 9 août 2026 : un noyau de biome déterministe, un standard d'excellence pour l'instrument et un laboratoire pour chaque module.",
                "Le nom aussi a été recherché. Aquarium, Spherium, Squarium, Aquarical, Orbarium et Orquarium ont été sur la table, chacun écarté pour être générique, pour entrer en collision avec un produit existant ou avec un autre mot. Aquorbium - aqu- (l'eau) + orb (la sphère) + -ium - est sorti propre et décrit ce que le moteur fait à l'intérieur : un aquarium sphérique, avec des êtres, des observateurs et la vitre elle-même. La forme courte est Aquorb.",
                "Dès lors, l'instrument a grandi par cohortes de modules, chacun avec recherche, prototype isolé, tests et écoute avant d'entrer dans le moteur : le cerveau d'incertitude, le séquenceur géométrique, le Striker granulaire, l'espace, la queue, le master, la musique écologique, les corps, le dispositif de synthèse, la genèse de la session. Plus de cent commits, une suite de plus de quatre-vingts tests et une règle : aucun code, son ou texte de tiers, seulement des idées réécrites, avec leur source citée.",
            ]),
            ("conceitos", "Ce qu'il pense", [
                "La colonne vertébrale de l'Aquorbium tient en une phrase : aucune entité n'a la vue totale. L'organisme ne voit et n'entend que ce que l'eau laisse passer ; qui écoute du dehors a le biais de son mode d'écoute ; et avec la loupe, voir de plus près à un endroit, c'est voir moins bien ailleurs.",
                "Quatre conséquences en découlent. Le désir sans contrôle direct : vous changez des tendances, vous n'écrivez pas d'événements. Le temps narratif : l'aquarium a matin et nuit, saisons, temps de chasse et de parade, et la forme naît de cette logique, sans séquenceur qui annonce la section B. La conséquence : tenir la tension a un coût, et la vitre peut se fendre. Et le focus a un prix : la loupe rapproche une région et rend le reste flou.",
            ]),
            ("generativa", "Composition générative", [
                "En composition générative, le compositeur n'écrit pas les notes une à une : il écrit les règles, les êtres et le monde d'où les notes naissent, et laisse la musique advenir. C'est une tradition qui va des jeux de dés musicaux du XVIIIe siècle à Xenakis, aux pièces d'Eno, à la vie artificielle de McCormack, à la musique d'essaim de Blackwell et à l'écosystémique audible de Di Scipio.",
                "Dans l'Aquorbium, les règles sont écologiques. Une chasse devient un canon dont le retard est le trajet du son dans l'eau ; les rivaux se répondent en hoquet ; un couple en parade copie le chant de l'autre ; un conflit accumule une tension harmonique et sa dissolution devient cadence. Les chants passent d'un organisme à l'autre, comme ceux des baleines, et parfois une révolution du chant change le répertoire de tout le récif.",
                "Chaque ouverture est une nouvelle pièce : la graine tire une tonalité et un mode, un langage harmonique qui module, un arc de tension de quatre à huit minutes et un biome - l'eau, la lumière, la population, les modules qui s'éveillent. La graine n'est pas un tirage aveugle de boutons : c'est une hypothèse cohérente, un lieu d'où partir. Et VARIE bouge les commandes au fil de la musique, comme une main lente. Vous n'êtes pas remplacé : l'instrument propose, vous jouez.",
            ]),
        ],
        "refs_h": "Références et crédits",
        "license_h": "Licence",
        "license": ["L'Aquorbium est un logiciel libre, sous la GNU Affero General Public License, version 3 ou ultérieure (AGPL-3.0-or-later). Vous pouvez l'utiliser, l'étudier, le modifier et le redistribuer, y compris commercialement, à condition que les versions modifiées que vous distribuez ou proposez par le réseau soient aussi libres, sous la même licence, avec leur code source disponible.",
                    "JUCE est utilisé sous son option AGPLv3. Les polices du site (DejaVu) sont libres. La musique que vous faites avec l'Aquorbium est à vous."],
    },
    "es": {
        "title": "Aquorbium — la idea", "desc": "Cómo se concibió el Aquorbium: el bioma como concepto, la perspectiva parcial, la composición generativa a partir de la ecología, las referencias y la licencia.",
        "h1": "La idea", "lead": "Del brainstorm al instrumento: de dónde vino el Aquorbium, qué piensa, cómo compone, a quién debe y con qué libertad es suyo.",
        "sections": [
            ("origem", "De dónde vino", [
                "El Aquorbium empezó como una conversación de investigación en torno a la idea de un bioma: un instrumento que fuera un ecosistema vivo, cuyos módulos se influyen unos a otros, en lugar de un camino en línea recta de una nota al sonido. Las primeras líneas de código son del 9 de agosto de 2026: un núcleo de bioma determinista, un estándar de excelencia para el instrumento y un laboratorio para cada módulo.",
                "El nombre también se investigó. Aquarium, Spherium, Squarium, Aquarical, Orbarium y Orquarium pasaron por la mesa, cada uno descartado por genérico, por chocar con un producto existente o con otra palabra. Aquorbium - aqu- (agua) + orb (la esfera) + -ium - quedó limpio y describe lo que el motor hace por dentro: un acuario esférico, con seres, observadores y el propio vidrio. La forma corta es Aquorb.",
                "A partir de ahí el instrumento creció por cohortes de módulos, cada uno con investigación, prototipo aislado, pruebas y escucha antes de entrar en el motor: el cerebro de incertidumbre, el secuenciador geométrico, el Striker granular, el espacio, la cola, el master, la música ecológica, los cuerpos, el dispositivo de síntesis, la génesis de la sesión. Más de cien commits, una batería de más de ochenta pruebas y una regla: ningún código, sonido ni texto de terceros, solo ideas reescritas, con la fuente citada.",
            ]),
            ("conceitos", "Qué piensa", [
                "La columna vertebral del Aquorbium es una frase: ninguna entidad tiene la visión total. El organismo ve y oye solo lo que el agua deja pasar; quien escucha desde fuera tiene el sesgo de su modo de escucha; y con la lupa, ver más de cerca en un lugar es ver peor en otro.",
                "De ahí salen cuatro consecuencias. Deseo sin control directo: usted cambia tendencias, no escribe acontecimientos. Tiempo narrativo: el acuario tiene mañana y noche, estaciones, épocas de caza y de cortejo, y la forma nace de esa lógica, sin un secuenciador que anuncie la sección B. Consecuencia: sostener la tensión tiene un costo, y el vidrio puede agrietarse. Y foco con costo: la lupa acerca una región y desenfoca el resto.",
            ]),
            ("generativa", "Composición generativa", [
                "En la composición generativa, el compositor no escribe las notas una a una: escribe las reglas, los seres y el mundo del que nacen las notas, y deja que la música suceda. Es una tradición que va de los juegos de dados musicales del siglo XVIII a Xenakis, a las piezas de Eno, a la vida artificial de McCormack, a la música de enjambre de Blackwell y a la ecosistémica audible de Di Scipio.",
                "En el Aquorbium las reglas son ecológicas. Una caza se vuelve un canon cuyo retardo es el viaje del sonido por el agua; los rivales se responden en hoquetus; una pareja en cortejo copia el canto del otro; un conflicto acumula tensión armónica y su disolución se vuelve cadencia. Los cantos pasan de organismo a organismo, como los de las ballenas, y a veces una revolución del canto cambia el repertorio de todo el arrecife.",
                "Cada apertura es una pieza nueva: la semilla sortea una tonalidad y un modo, un lenguaje armónico que modula, un arco de tensión de cuatro a ocho minutos y un bioma - el agua, la luz, la población, qué módulos despiertan. La semilla no es un sorteo ciego de botones: es una hipótesis coherente, un lugar desde donde empezar. Y VARÍA mueve los controles a lo largo de la música, como una mano lenta. Usted no es reemplazado: el instrumento propone, usted toca.",
            ]),
        ],
        "refs_h": "Referencias y créditos",
        "license_h": "Licencia",
        "license": ["El Aquorbium es software libre, bajo la GNU Affero General Public License, versión 3 o posterior (AGPL-3.0-or-later). Puede usarlo, estudiarlo, modificarlo y redistribuirlo, también comercialmente, siempre que las versiones modificadas que distribuya u ofrezca por la red también sean libres, bajo la misma licencia, con su código fuente disponible.",
                    "JUCE se usa bajo su opción AGPLv3. Las fuentes del sitio (DejaVu) son libres. La música que haga con el Aquorbium es suya."],
    },
}


# ---- os cinco seres: natureza, som, o que fazem e como explorá-los ----------
# (src/core/Ecosystem.cpp: espécies, papéis e estratégias; os corpos de cada
# espécie; a música ecológica, ECOLOGICAL_MUSIC_DESIGN.md)
SERES = {
    "pt-BR": {
        "labels": ("Natureza", "Som", "Na música", "Explore"),
        "MORPH": ("O caçador. Predador e territorial, de primeiro plano.",
                  "Um oscilador que se transforma: a forma de onda desliza entre timbres, do liso ao brilhante.",
                  "Persegue a presa que consegue ver através da água e a imita em cânone, com o atraso da viagem do som; os cliques de ecolocalização aceleram até o buzz terminal da captura.",
                  "Suba o PERIGO e acompanhe as linhas coral da caça. Dê-lhe o corpo de voz e ligue o TIMBRE ECOLÓGICO: a voz se aguça quando ele caça."),
        "GRAIN": ("O recife de partículas. Territorial, de sustentação.",
                  "Um Striker granular: impactos feitos de grãos, da poeira breve ao metal ressonante.",
                  "Defende o seu lugar e dá ao bioma textura e pulso, por baixo das vozes de primeiro plano.",
                  "No cartão SÍNTESE, suba o STRIKER e mova MATÉRIA e DISPERSÃO enquanto o bioma toca; experimente os corpos gota e matéria."),
        "MODAL": ("O viajante. Migrante, de sustentação.",
                  "Um ressonador de modos: madeira, metal e vidro golpeados, que soam como tigelas e sinos.",
                  "Faz uma volta larga pela esfera, de cerca de um minuto e meio, e muda de perspectiva: perto do vidro soa claro, longe soa mais escuro, porque a água absorve os agudos.",
                  "Siga-o com a LUPA durante a volta inteira. Dê-lhe o corpo de vidro ou de membrana."),
        "PULSAR": ("A presa. De primeiro plano.",
                   "Síntese pulsar (Curtis Roads): trens de pequenos grãos de som, cada um com o seu formante.",
                   "Foge do predador em diminuição - as notas encurtam -, ou se cala para não ser ouvida.",
                   "Acompanhe as linhas âmbar da fuga. No dispositivo de síntese, ligue presas à velocidade do LFO 2: quando o recife se enche de presas, a pulsação fica nervosa."),
        "STRING": ("A âncora. Séssil e territorial, de sustentação.",
                   "Uma corda física: dedilhada a cada nota, ou friccionada, como um arco.",
                   "Fica perto do seu ponto de âncora e volta sempre a ele; defende o território e sustenta a harmonia por baixo.",
                   "Dê-lhe o corpo de arco e suba a SUSTENTAÇÃO para trazê-la ao primeiro plano."),
    },
    "en": {
        "labels": ("Nature", "Sound", "In the music", "Explore"),
        "MORPH": ("The hunter. Predator and territorial, in the foreground.",
                  "An oscillator that transforms: its waveform glides between timbres, from smooth to bright.",
                  "It chases the prey it can see through the water and imitates it in canon, with the delay of the sound's travel; its echolocation clicks speed up into the terminal buzz of the capture.",
                  "Raise DANGER and follow the coral lines of the hunt. Give it the voice body and turn ECOLOGICAL TIMBRE up: its voice sharpens as it hunts."),
        "GRAIN": ("The reef of particles. Territorial, in support.",
                  "A granular Striker: impacts made of grains, from brief dust to resonant metal.",
                  "It defends its place and gives the biome texture and pulse, beneath the foreground voices.",
                  "In the SYNTHESIS card, raise STRIKER and move MATTER and SCATTER while the biome plays; try the drop and matter bodies."),
        "MODAL": ("The traveller. Migrant, in support.",
                  "A resonator of modes: struck wood, metal and glass, ringing like bowls and bells.",
                  "It makes a wide round of the sphere, about a minute and a half long, and changes perspective: near the glass it sounds clear, far away darker, because the water absorbs the highs.",
                  "Follow it with the LENS through its whole round. Give it the glass or membrane body."),
        "PULSAR": ("The prey. In the foreground.",
                   "Pulsar synthesis (Curtis Roads): trains of tiny grains of sound, each with its own formant.",
                   "It flees the predator in diminution - its notes shorten -, or falls silent so as not to be heard.",
                   "Follow the amber lines of flight. In the synthesis device, route prey to the LFO 2 rate: when the reef fills with prey, its pulsing grows nervous."),
        "STRING": ("The anchor. Sessile and territorial, in support.",
                   "A physical string: plucked at every note, or bowed.",
                   "It stays near its anchor point and always returns to it; it defends its territory and holds the harmony from below.",
                   "Give it the bow body and raise SUPPORT to bring it to the foreground."),
    },
    "fr": {
        "labels": ("Nature", "Son", "Dans la musique", "À explorer"),
        "MORPH": ("Le chasseur. Prédateur et territorial, au premier plan.",
                  "Un oscillateur qui se transforme : sa forme d'onde glisse entre les timbres, du lisse au brillant.",
                  "Il poursuit la proie qu'il voit à travers l'eau et l'imite en canon, avec le retard du trajet du son ; ses clics d'écholocation accélèrent jusqu'au buzz terminal de la capture.",
                  "Montez le DANGER et suivez les lignes corail de la chasse. Donnez-lui le corps voix et montez le TIMBRE ÉCOLOGIQUE : sa voix s'aiguise quand il chasse."),
        "GRAIN": ("Le récif de particules. Territorial, en soutien.",
                  "Un Striker granulaire : des impacts faits de grains, de la poussière brève au métal résonant.",
                  "Il défend sa place et donne au biome texture et pulsation, sous les voix du premier plan.",
                  "Dans la carte SYNTHÈSE, montez STRIKER et bougez MATIÈRE et DISPERSION pendant que le biome joue ; essayez les corps goutte et matière."),
        "MODAL": ("Le voyageur. Migrateur, en soutien.",
                  "Un résonateur de modes : bois, métal et verre frappés, qui sonnent comme des bols et des cloches.",
                  "Il fait un large tour de la sphère, d'environ une minute et demie, et change de perspective : près de la vitre il sonne clair, au loin plus sombre, car l'eau absorbe les aigus.",
                  "Suivez-le à la LOUPE pendant tout son tour. Donnez-lui le corps verre ou membrane."),
        "PULSAR": ("La proie. Au premier plan.",
                   "La synthèse pulsar (Curtis Roads) : des trains de minuscules grains de son, chacun avec son formant.",
                   "Elle fuit le prédateur en diminution - ses notes raccourcissent -, ou se tait pour ne pas être entendue.",
                   "Suivez les lignes ambre de la fuite. Dans le dispositif de synthèse, reliez les proies à la vitesse du LFO 2 : quand le récif se remplit de proies, sa pulsation devient nerveuse."),
        "STRING": ("L'ancre. Sessile et territoriale, en soutien.",
                   "Une corde physique : pincée à chaque note, ou frottée à l'archet.",
                   "Elle reste près de son point d'ancrage et y revient toujours ; elle défend son territoire et tient l'harmonie par en dessous.",
                   "Donnez-lui le corps archet et montez le SOUTIEN pour l'amener au premier plan."),
    },
    "es": {
        "labels": ("Naturaleza", "Sonido", "En la música", "Explore"),
        "MORPH": ("El cazador. Depredador y territorial, en primer plano.",
                  "Un oscilador que se transforma: su forma de onda se desliza entre timbres, de lo liso a lo brillante.",
                  "Persigue a la presa que puede ver a través del agua y la imita en canon, con el retardo del viaje del sonido; sus clics de ecolocalización se aceleran hasta el zumbido terminal de la captura.",
                  "Suba el PELIGRO y siga las líneas coral de la caza. Dele el cuerpo de voz y suba el TIMBRE ECOLÓGICO: su voz se afila cuando caza."),
        "GRAIN": ("El arrecife de partículas. Territorial, de apoyo.",
                  "Un Striker granular: impactos hechos de granos, del polvo breve al metal resonante.",
                  "Defiende su lugar y da al bioma textura y pulso, por debajo de las voces de primer plano.",
                  "En la tarjeta SÍNTESIS, suba STRIKER y mueva MATERIA y DISPERSIÓN mientras el bioma toca; pruebe los cuerpos gota y materia."),
        "MODAL": ("El viajero. Migrante, de apoyo.",
                  "Un resonador de modos: madera, metal y vidrio golpeados, que suenan como cuencos y campanas.",
                  "Da una vuelta amplia por la esfera, de cerca de un minuto y medio, y cambia de perspectiva: cerca del vidrio suena claro, lejos más oscuro, porque el agua absorbe los agudos.",
                  "Sígalo con la LUPA durante toda la vuelta. Dele el cuerpo de vidrio o de membrana."),
        "PULSAR": ("La presa. En primer plano.",
                   "Síntesis pulsar (Curtis Roads): trenes de diminutos granos de sonido, cada uno con su formante.",
                   "Huye del depredador en disminución - sus notas se acortan -, o calla para no ser oída.",
                   "Siga las líneas ámbar de la huida. En el dispositivo de síntesis, conecte las presas a la velocidad del LFO 2: cuando el arrecife se llena de presas, su pulso se vuelve nervioso."),
        "STRING": ("El ancla. Sésil y territorial, de apoyo.",
                   "Una cuerda física: punteada en cada nota, o frotada con arco.",
                   "Se queda cerca de su punto de anclaje y siempre vuelve a él; defiende su territorio y sostiene la armonía desde abajo.",
                   "Dele el cuerpo de arco y suba el APOYO para traerla al primer plano."),
    },
}


# os títulos e as descrições das páginas novas
OUTRAS = {
    "pt-BR": {
        "tutorial_title": "Aquorbium — tutorial", "tutorial_desc": "O tutorial do Aquorbium, capítulo por capítulo: o ecossistema, a água, os cantos, os corpos, o dispositivo de síntese e as cinco lições, o espaço, a vida e o REC.",
        "tutorial_h1": "Tutorial", "tutorial_lead": "O mesmo tutorial que está dentro do app (o botão TUTORIAL), aqui para ler antes de abrir ou ao lado dele. Os textos vêm do próprio app, nas quatro línguas.",
        "contents": "Sumário", "tip": "Dica",
        "install_title": "Aquorbium — instalação", "install_desc": "Como instalar o Aquorbium no Windows, no macOS e no Linux, passo a passo, os avisos de segurança da primeira abertura e as soluções de problemas.",
        "install_h1": "Instalação",
    },
    "en": {
        "tutorial_title": "Aquorbium — tutorial", "tutorial_desc": "The Aquorbium tutorial, chapter by chapter: the ecosystem, the water, the songs, the bodies, the synthesis device and its five lessons, space, life and REC.",
        "tutorial_h1": "Tutorial", "tutorial_lead": "The same tutorial that lives inside the app (the TUTORIAL button), here to read before you open it or beside it. The texts come from the app itself, in four languages.",
        "contents": "Contents", "tip": "Tip",
        "install_title": "Aquorbium — installation", "install_desc": "How to install the Aquorbium on Windows, macOS and Linux, step by step, the first-launch security warnings and troubleshooting.",
        "install_h1": "Installation",
    },
    "fr": {
        "tutorial_title": "Aquorbium — tutoriel", "tutorial_desc": "Le tutoriel de l'Aquorbium, chapitre par chapitre : l'écosystème, l'eau, les chants, les corps, le dispositif de synthèse et ses cinq leçons, l'espace, la vie et REC.",
        "tutorial_h1": "Tutoriel", "tutorial_lead": "Le même tutoriel que dans l'app (le bouton TUTORIEL), ici pour le lire avant de l'ouvrir ou à côté. Les textes viennent de l'app elle-même, en quatre langues.",
        "contents": "Sommaire", "tip": "Astuce",
        "install_title": "Aquorbium — installation", "install_desc": "Comment installer l'Aquorbium sur Windows, macOS et Linux, pas à pas, les avertissements de sécurité de la première ouverture et les solutions aux problèmes.",
        "install_h1": "Installation",
    },
    "es": {
        "tutorial_title": "Aquorbium — tutorial", "tutorial_desc": "El tutorial del Aquorbium, capítulo a capítulo: el ecosistema, el agua, los cantos, los cuerpos, el dispositivo de síntesis y sus cinco lecciones, el espacio, la vida y REC.",
        "tutorial_h1": "Tutorial", "tutorial_lead": "El mismo tutorial que está dentro de la app (el botón TUTORIAL), aquí para leerlo antes de abrirla o a su lado. Los textos vienen de la propia app, en cuatro idiomas.",
        "contents": "Sumario", "tip": "Consejo",
        "install_title": "Aquorbium — instalación", "install_desc": "Cómo instalar el Aquorbium en Windows, macOS y Linux, paso a paso, los avisos de seguridad de la primera apertura y las soluciones de problemas.",
        "install_h1": "Instalación",
    },
}

# os organismos: nome, cor do app (src/app/Main.cpp, Palette) e natureza
ORGANISMOS = [("MORPH", "cyan"), ("GRAIN", "amber"), ("MODAL", "coral"), ("PULSAR", "violet"), ("STRING", "green")]

T = {
    "pt-BR": {
        "title": "Aquorbium — um bioma audível",
        "desc": "Aquorbium: um instrumento ecológico da família RASGO. Cinco organismos sonoros vivem numa esfera de água, se ouvem, caçam, fogem, cortejam e cantam; cada abertura é uma peça nova.",
        "strip": "RASGO — família de instrumentos e pesquisa sonora",
        "skip": "Pular para o conteúdo",
        "langnav": "Idioma",
        "pill_pub": "Versão 0.1.0 — para Windows, macOS e Linux",
        "pill_prep": "Em preparação — versão 0.1.0 para Windows, macOS e Linux",
        "lead": "Cinco organismos sonoros vivem numa esfera de água. Eles se veem e se ouvem através da água, caçam, fogem, cortejam, defendem território, nascem e morrem, e cantam. Você não toca notas: toca o ambiente, e escuta o que a vida lá dentro faz com ele.",
        "sonar_label": "Um sonar desenhado ao vivo: o feixe gira e os cinco organismos nadam na esfera.",
        "s_org": "Quem vive ali",
        "org": {
            "MORPH": "predador e territorial — um oscilador que se transforma",
            "GRAIN": "territorial — impactos feitos de partículas",
            "MODAL": "migrante — um ressonador de modos",
            "PULSAR": "presa — trens de pulsos",
            "STRING": "séssil e territorial — uma corda física",
        },
        "org_note": "Cada um só vê e ouve o que a água deixa passar. As linhas entre eles são as relações: coral é caça, âmbar é fuga, ciano é cardume, rosa é corte.",
        "s_seed": "A semente desta visita",
        "seed_intro": "Cada vez que o Aquorbium abre, uma semente sorteia um mundo e um bioma. Esta página faz o mesmo, aqui no seu navegador, a cada visita: é uma ilustração da ideia, não o som do app.",
        "seed_again": "Outra semente",
        "seed_ui": {
            "seed": "semente", "world": "O mundo", "biome": "O bioma", "wake": "Acordam",
            "arc": "arco de tensão", "minutes": "min",
            "genes": ["temperatura", "luz", "turbidez", "correnteza", "população", "predação", "habitat"],
            "sea": "mar", "fresh": "água doce",
            "languages": ["funcional", "modal", "brasileira", "impressionista", "cromática"],
            "modes": ["dórico", "eólio", "mixolídio", "lídio", "jônio", "frígio", "pentatônica menor", "pentatônica maior", "menor harmônica", "hirajoshi"],
            "arcs": ["subir ao clímax e cair", "ondas", "crescimento lento", "calmaria entre duas agitações"],
            "modules": ["STRIKER", "ESPAÇO", "GEOMETRIA", "AMBIENTE", "CAUDA", "FORMA", "ESTRUTURA", "MÚSICA"],
            "polytonal": "com episódios politonais",
        },
        "s_music": "A música nasce das relações",
        "music": [
            ("Caça e fuga", "O caçador imita o que ouviu da presa, num cânone cujo atraso é a viagem do som pela água; os cliques de ecolocalização aceleram até o buzz terminal. A presa foge em diminuição, ou se cala."),
            ("Rivais e aliados", "Rivais se respondem em hoquetus; aliados cantam juntos; o par em corte copia o canto do outro. Os cantos passam de organismo a organismo, como os das baleias."),
            ("Harmonia e forma", "A sessão tem uma linguagem harmônica — funcional, modal, brasileira, impressionista, cromática —, modula, às vezes deixa as vozes agudas em outra tonalidade, e segue um arco de tensão até o clímax e o repouso."),
            ("Pergunta e resposta", "A melodia respira depois das cadências; outra voz lhe responde com o motivo transformado — imitado, invertido, ao contrário —, e o motivo volta mais tarde."),
        ],
        "s_water": "A água é a forma",
        "water": "A água tem cinco estados, e cada um é um jeito de tocar: o gelo é um cristal de alturas, seco; a água fria é viscosa e ligada; a temperada é laminar; a turbulência traz rubato e heterofonia; a fervura traz bolhas que soam na própria altura física. O mar absorve os agudos com a distância; a água doce fica límpida.",
        "states": ["GELO", "FRIA", "TEMPERADA", "TURBULENTA", "FERVURA"],
        "s_play": "Como se toca",
        "play": [
            ("O ambiente", "Densidade, deriva, coesão, perigo, a água e a memória mudam tendências; os acontecimentos vêm do ecossistema."),
            ("VARIA", "Os controles se movem sozinhos ao longo da música, em torno de onde a semente ou a sua mão os deixou — mais quentes no clímax, assentando no repouso."),
            ("A voz de síntese", "Cada organismo tem um sintetizador que você constrói; numa matriz, a vida do bioma — fome, tensão, distância, a temperatura da água — toca os parâmetros."),
            ("A lupa e os fones", "A lupa aproxima a escuta de uma região da esfera; com fones, cada organismo vem da sua direção, em volta da cabeça."),
        ],
        "s_shot": "A interface",
        "shot_alt": "O Aquorbium tocando: a esfera de água com os cinco organismos e as suas relações no centro, os cartões em volta e a faixa de baixo com densidade, deriva, morph e VARIA.",
        "shot_cap": "O app tocando, com o tutorial e o LEARN em quatro idiomas, de 960×600 a telas largas.",
        "s_dl": "Baixar",
        "dl_intro_pub": "Escolha o pacote do seu sistema.",
        "dl_intro_prep": "A versão 0.1.0 está em preparação. Quando sair, os pacotes estarão na página de releases do GitHub.",
        "dl_os": {"Windows": "Windows 10 ou 11 — instalador (.exe) ou sem instalar (.zip)", "macOS": "macOS 10.15 ou mais novo, Intel e Apple Silicon", "Linux": ".deb para Ubuntu, Debian e Mint; AppImage ou .tar.gz para as outras distribuições"},
        "dl_warn": "Na primeira abertura, o Windows e o macOS mostram um aviso de segurança: é esperado, porque o app não tem a assinatura paga das duas empresas. O guia de instalação diz onde clicar.",
        "dl_install": "Guia de instalação",
        "dl_all": "Página de releases",
        "s_credits": "Créditos e fontes",
        "credits": "Lúcio de Araújo — concepção, composição, escuta e código. Os conceitos vêm da ecoacústica e da bioacústica (Krause, Schafer, os estudos dos cantos de baleias e golfinhos), da física do som na água e da teoria musical; nenhum código, som ou texto dessas obras foi copiado. Licença GNU AGPLv3 ou posterior; JUCE sob a via AGPLv3. Fontes e créditos completos:",
        "credits_link": "CREDITS_AND_SOURCES.md",
        "s_state": "Estado",
        "state_pub": "A versão 0.1.0 é a primeira versão pública. Os pacotes de Windows e macOS são construídos e testados pela integração contínua; o instrumento continua em desenvolvimento, aberto.",
        "state_prep": "O Aquorbium está se preparando para a primeira versão pública. O código já está aberto no GitHub.",
        "contact": "Contato",
        "footer_credits": "Créditos",
        "footer_license": "Licença AGPLv3+",
        "dialog_title": "CONTATO",
        "dialog_copy_hint": "Copie o endereço de e-mail:",
        "dialog_copy": "Copiar e-mail",
        "dialog_close": "Fechar janela de contato",
        "copied": "E-mail copiado.",
        "fallback": "Selecione e copie o endereço acima.",
    },
    "en": {
        "title": "Aquorbium — an audible biome",
        "desc": "Aquorbium: an ecological instrument of the RASGO family. Five sound organisms live in a sphere of water, hear each other, hunt, flee, court and sing; every opening is another piece.",
        "strip": "RASGO — a family of instruments and sound research",
        "skip": "Skip to content",
        "langnav": "Language",
        "pill_pub": "Version 0.1.0 — for Windows, macOS and Linux",
        "pill_prep": "In preparation — version 0.1.0 for Windows, macOS and Linux",
        "lead": "Five sound organisms live in a sphere of water. They see and hear each other through the water, hunt, flee, court, defend territory, are born and die, and sing. You do not play notes: you play the environment, and listen to what the life inside makes of it.",
        "sonar_label": "A sonar drawn live: the beam turns and the five organisms swim in the sphere.",
        "s_org": "Who lives there",
        "org": {
            "MORPH": "predator and territorial — an oscillator that transforms",
            "GRAIN": "territorial — impacts made of particles",
            "MODAL": "migrant — a resonator of modes",
            "PULSAR": "prey — trains of pulses",
            "STRING": "sessile and territorial — a physical string",
        },
        "org_note": "Each one sees and hears only what the water lets through. The lines between them are their relations: coral is a hunt, amber a flight, cyan a school, pink a courtship.",
        "s_seed": "The seed of this visit",
        "seed_intro": "Every time the Aquorbium opens, a seed draws a world and a biome. This page does the same, here in your browser, at every visit: it illustrates the idea, it is not the app's sound.",
        "seed_again": "Another seed",
        "seed_ui": {
            "seed": "seed", "world": "The world", "biome": "The biome", "wake": "Waking",
            "arc": "arc of tension", "minutes": "min",
            "genes": ["temperature", "light", "turbidity", "current", "population", "predation", "habitat"],
            "sea": "sea", "fresh": "fresh water",
            "languages": ["functional", "modal", "Brazilian", "impressionist", "chromatic"],
            "modes": ["dorian", "aeolian", "mixolydian", "lydian", "ionian", "phrygian", "minor pentatonic", "major pentatonic", "harmonic minor", "hirajoshi"],
            "arcs": ["rise to a climax and fall", "waves", "slow build", "calm between two agitations"],
            "modules": ["STRIKER", "SPACE", "GEOMETRY", "ENVIRONMENT", "TAIL", "FORM", "STRUCTURE", "MUSIC"],
            "polytonal": "with polytonal episodes",
        },
        "s_music": "The music is born from the relations",
        "music": [
            ("Hunt and flight", "A hunter imitates what it heard of its prey, in a canon whose delay is the sound's travel through the water; its echolocation clicks speed up into the terminal buzz. The prey flees in diminution, or falls silent."),
            ("Rivals and allies", "Rivals answer each other in hocket; allies sing together; a courting pair copies each other's song. Songs pass from organism to organism, like the songs of whales."),
            ("Harmony and form", "The session has a harmonic language — functional, modal, Brazilian, impressionist, chromatic —, modulates, sometimes lets the high voices wander into another key, and follows an arc of tension to its climax and its rest."),
            ("Question and answer", "The melody breathes after the cadences; another voice answers with its motif transformed — imitated, inverted, backwards —, and the motif returns later."),
        ],
        "s_water": "The water is the form",
        "water": "The water has five states, and each is a way of playing: ice is a crystal of pitches, dry; cold water is viscous and legato; temperate water is laminar; turbulence brings rubato and heterophony; boiling brings bubbles that ring at their own physical pitch. The sea absorbs the highs with distance; fresh water stays clear.",
        "states": ["ICE", "COLD", "TEMPERATE", "TURBULENT", "BOILING"],
        "s_play": "How you play it",
        "play": [
            ("The environment", "Density, drift, cohesion, danger, the water and memory change tendencies; the events come from the ecosystem."),
            ("VARY", "The controls move by themselves over the music's time, around where the seed or your hand left them — warmer at the climax, settling at rest."),
            ("The synthesis voice", "Each organism has a synthesiser you build; in a matrix, the biome's life — hunger, tension, distance, the water's temperature — plays the parameters."),
            ("The lens and headphones", "The lens brings your listening close to a region of the sphere; with headphones, each organism comes from its own direction, around your head."),
        ],
        "s_shot": "The interface",
        "shot_alt": "The Aquorbium playing: the sphere of water with the five organisms and their relations in the centre, the cards around it and the bottom strip with density, drift, morph and VARY.",
        "shot_cap": "The app playing, with the tutorial and LEARN in four languages, from 960×600 to wide screens.",
        "s_dl": "Download",
        "dl_intro_pub": "Choose the package for your system.",
        "dl_intro_prep": "Version 0.1.0 is in preparation. When it is out, the packages will be on the GitHub releases page.",
        "dl_os": {"Windows": "Windows 10 or 11 — installer (.exe) or no install (.zip)", "macOS": "macOS 10.15 or later, Intel and Apple Silicon", "Linux": ".deb for Ubuntu, Debian and Mint; AppImage or .tar.gz for other distributions"},
        "dl_warn": "On first launch, Windows and macOS show a security warning: it is expected, because the app does not carry the two companies' paid signature. The installation guide says where to click.",
        "dl_install": "Installation guide",
        "dl_all": "Releases page",
        "s_credits": "Credits and sources",
        "credits": "Lúcio de Araújo — concept, composition, listening and code. The concepts come from ecoacoustics and bioacoustics (Krause, Schafer, the studies of whale and dolphin song), the physics of sound in water and music theory; no code, sound or text of these works was copied. Licence GNU AGPLv3 or later; JUCE under AGPLv3. Full sources and credits:",
        "credits_link": "CREDITS_AND_SOURCES.md",
        "s_state": "Status",
        "state_pub": "Version 0.1.0 is the first public version. The Windows and macOS packages are built and tested by continuous integration; the instrument is still in development, open.",
        "state_prep": "The Aquorbium is getting ready for its first public version. The code is already open on GitHub.",
        "contact": "Contact",
        "footer_credits": "Credits",
        "footer_license": "AGPLv3+ licence",
        "dialog_title": "CONTACT",
        "dialog_copy_hint": "Copy the email address:",
        "dialog_copy": "Copy email",
        "dialog_close": "Close contact window",
        "copied": "Email copied.",
        "fallback": "Select and copy the address above.",
    },
    "fr": {
        "title": "Aquorbium — un biome audible",
        "desc": "Aquorbium : un instrument écologique de la famille RASGO. Cinq organismes sonores vivent dans une sphère d'eau, s'entendent, chassent, fuient, paradent et chantent ; chaque ouverture est une nouvelle pièce.",
        "strip": "RASGO — famille d'instruments et de recherche sonore",
        "skip": "Aller au contenu",
        "langnav": "Langue",
        "pill_pub": "Version 0.1.0 — pour Windows, macOS et Linux",
        "pill_prep": "En préparation — version 0.1.0 pour Windows, macOS et Linux",
        "lead": "Cinq organismes sonores vivent dans une sphère d'eau. Ils se voient et s'entendent à travers l'eau, chassent, fuient, paradent, défendent un territoire, naissent et meurent, et chantent. Vous ne jouez pas de notes : vous jouez l'environnement, et écoutez ce que la vie à l'intérieur en fait.",
        "sonar_label": "Un sonar dessiné en direct : le faisceau tourne et les cinq organismes nagent dans la sphère.",
        "s_org": "Qui vit là",
        "org": {
            "MORPH": "prédateur et territorial — un oscillateur qui se transforme",
            "GRAIN": "territorial — des impacts faits de particules",
            "MODAL": "migrateur — un résonateur de modes",
            "PULSAR": "proie — des trains d'impulsions",
            "STRING": "sessile et territorial — une corde physique",
        },
        "org_note": "Chacun ne voit et n'entend que ce que l'eau laisse passer. Les lignes entre eux sont leurs relations : corail une chasse, ambre une fuite, cyan un banc, rose une parade.",
        "s_seed": "La graine de cette visite",
        "seed_intro": "Chaque fois que l'Aquorbium s'ouvre, une graine tire un monde et un biome. Cette page fait de même, ici dans votre navigateur, à chaque visite : c'est une illustration de l'idée, pas le son de l'app.",
        "seed_again": "Une autre graine",
        "seed_ui": {
            "seed": "graine", "world": "Le monde", "biome": "Le biome", "wake": "S'éveillent",
            "arc": "arc de tension", "minutes": "min",
            "genes": ["température", "lumière", "turbidité", "courant", "population", "prédation", "habitat"],
            "sea": "mer", "fresh": "eau douce",
            "languages": ["fonctionnel", "modal", "brésilien", "impressionniste", "chromatique"],
            "modes": ["dorien", "éolien", "mixolydien", "lydien", "ionien", "phrygien", "pentatonique mineure", "pentatonique majeure", "mineur harmonique", "hirajoshi"],
            "arcs": ["monter au climax et retomber", "vagues", "lente croissance", "calme entre deux agitations"],
            "modules": ["STRIKER", "ESPACE", "GÉOMÉTRIE", "ENVIRONNEMENT", "QUEUE", "FORME", "STRUCTURE", "MUSIQUE"],
            "polytonal": "avec des épisodes polytonaux",
        },
        "s_music": "La musique naît des relations",
        "music": [
            ("Chasse et fuite", "Le chasseur imite ce qu'il a entendu de sa proie, en un canon dont le retard est le trajet du son dans l'eau ; ses clics d'écholocation accélèrent jusqu'au buzz terminal. La proie fuit en diminution, ou se tait."),
            ("Rivaux et alliés", "Les rivaux se répondent en hoquet ; les alliés chantent ensemble ; le couple en parade copie le chant de l'autre. Les chants passent d'un organisme à l'autre, comme ceux des baleines."),
            ("Harmonie et forme", "La session a un langage harmonique — fonctionnel, modal, brésilien, impressionniste, chromatique —, module, laisse parfois les voix aiguës dans une autre tonalité, et suit un arc de tension jusqu'au climax et au repos."),
            ("Question et réponse", "La mélodie respire après les cadences ; une autre voix lui répond avec son motif transformé — imité, renversé, à rebours —, et le motif revient plus tard."),
        ],
        "s_water": "L'eau est la forme",
        "water": "L'eau a cinq états, et chacun est une manière de jouer : la glace est un cristal de hauteurs, sec ; l'eau froide est visqueuse et liée ; l'eau tempérée est laminaire ; la turbulence apporte rubato et hétérophonie ; l'ébullition apporte des bulles qui sonnent à leur propre hauteur physique. La mer absorbe les aigus avec la distance ; l'eau douce reste claire.",
        "states": ["GLACE", "FROIDE", "TEMPÉRÉE", "TURBULENTE", "ÉBULLITION"],
        "s_play": "Comment on en joue",
        "play": [
            ("L'environnement", "Densité, dérive, cohésion, danger, l'eau et la mémoire changent des tendances ; les événements viennent de l'écosystème."),
            ("VARIE", "Les commandes bougent toutes seules au fil de la musique, autour de là où la graine ou votre main les a laissées — plus chaudes au climax, apaisées au repos."),
            ("La voix de synthèse", "Chaque organisme a un synthétiseur que vous construisez ; dans une matrice, la vie du biome — la faim, la tension, la distance, la température de l'eau — joue les paramètres."),
            ("La loupe et le casque", "La loupe rapproche votre écoute d'une région de la sphère ; au casque, chaque organisme vient de sa direction, autour de votre tête."),
        ],
        "s_shot": "L'interface",
        "shot_alt": "L'Aquorbium en jeu : la sphère d'eau avec les cinq organismes et leurs relations au centre, les cartes autour et la bande du bas avec densité, dérive, morph et VARIE.",
        "shot_cap": "L'app en jeu, avec le tutoriel et LEARN en quatre langues, de 960×600 aux écrans larges.",
        "s_dl": "Télécharger",
        "dl_intro_pub": "Choisissez le paquet de votre système.",
        "dl_intro_prep": "La version 0.1.0 est en préparation. À sa sortie, les paquets seront sur la page des versions de GitHub.",
        "dl_os": {"Windows": "Windows 10 ou 11 — installateur (.exe) ou sans installation (.zip)", "macOS": "macOS 10.15 ou plus récent, Intel et Apple Silicon", "Linux": ".deb pour Ubuntu, Debian et Mint ; AppImage ou .tar.gz pour les autres distributions"},
        "dl_warn": "À la première ouverture, Windows et macOS affichent un avertissement de sécurité : c'est attendu, car l'app n'a pas la signature payante des deux entreprises. Le guide d'installation dit où cliquer.",
        "dl_install": "Guide d'installation",
        "dl_all": "Page des versions",
        "s_credits": "Crédits et sources",
        "credits": "Lúcio de Araújo — conception, composition, écoute et code. Les concepts viennent de l'écoacoustique et de la bioacoustique (Krause, Schafer, les études des chants de baleines et de dauphins), de la physique du son dans l'eau et de la théorie musicale ; aucun code, son ou texte de ces œuvres n'a été copié. Licence GNU AGPLv3 ou ultérieure ; JUCE sous AGPLv3. Sources et crédits complets :",
        "credits_link": "CREDITS_AND_SOURCES.md",
        "s_state": "État",
        "state_pub": "La version 0.1.0 est la première version publique. Les paquets Windows et macOS sont construits et testés par l'intégration continue ; l'instrument reste en développement, ouvert.",
        "state_prep": "L'Aquorbium se prépare à sa première version publique. Le code est déjà ouvert sur GitHub.",
        "contact": "Contact",
        "footer_credits": "Crédits",
        "footer_license": "Licence AGPLv3+",
        "dialog_title": "CONTACT",
        "dialog_copy_hint": "Copiez l'adresse e-mail :",
        "dialog_copy": "Copier l'e-mail",
        "dialog_close": "Fermer la fenêtre de contact",
        "copied": "E-mail copié.",
        "fallback": "Sélectionnez et copiez l'adresse ci-dessus.",
    },
    "es": {
        "title": "Aquorbium — un bioma audible",
        "desc": "Aquorbium: un instrumento ecológico de la familia RASGO. Cinco organismos sonoros viven en una esfera de agua, se oyen, cazan, huyen, cortejan y cantan; cada apertura es una pieza nueva.",
        "strip": "RASGO — familia de instrumentos e investigación sonora",
        "skip": "Saltar al contenido",
        "langnav": "Idioma",
        "pill_pub": "Versión 0.1.0 — para Windows, macOS y Linux",
        "pill_prep": "En preparación — versión 0.1.0 para Windows, macOS y Linux",
        "lead": "Cinco organismos sonoros viven en una esfera de agua. Se ven y se oyen a través del agua, cazan, huyen, cortejan, defienden territorio, nacen y mueren, y cantan. Usted no toca notas: toca el ambiente, y escucha lo que la vida de dentro hace con él.",
        "sonar_label": "Un sonar dibujado en vivo: el haz gira y los cinco organismos nadan en la esfera.",
        "s_org": "Quién vive allí",
        "org": {
            "MORPH": "depredador y territorial — un oscilador que se transforma",
            "GRAIN": "territorial — impactos hechos de partículas",
            "MODAL": "migrante — un resonador de modos",
            "PULSAR": "presa — trenes de pulsos",
            "STRING": "sésil y territorial — una cuerda física",
        },
        "org_note": "Cada uno solo ve y oye lo que el agua deja pasar. Las líneas entre ellos son sus relaciones: coral es caza, ámbar huida, cian cardumen, rosa cortejo.",
        "s_seed": "La semilla de esta visita",
        "seed_intro": "Cada vez que el Aquorbium se abre, una semilla sortea un mundo y un bioma. Esta página hace lo mismo, aquí en su navegador, en cada visita: es una ilustración de la idea, no el sonido de la app.",
        "seed_again": "Otra semilla",
        "seed_ui": {
            "seed": "semilla", "world": "El mundo", "biome": "El bioma", "wake": "Despiertan",
            "arc": "arco de tensión", "minutes": "min",
            "genes": ["temperatura", "luz", "turbidez", "corriente", "población", "depredación", "hábitat"],
            "sea": "mar", "fresh": "agua dulce",
            "languages": ["funcional", "modal", "brasileño", "impresionista", "cromático"],
            "modes": ["dórico", "eólico", "mixolidio", "lidio", "jónico", "frigio", "pentatónica menor", "pentatónica mayor", "menor armónica", "hirajoshi"],
            "arcs": ["subir al clímax y caer", "olas", "crecimiento lento", "calma entre dos agitaciones"],
            "modules": ["STRIKER", "ESPACIO", "GEOMETRÍA", "AMBIENTE", "COLA", "FORMA", "ESTRUCTURA", "MÚSICA"],
            "polytonal": "con episodios politonales",
        },
        "s_music": "La música nace de las relaciones",
        "music": [
            ("Caza y huida", "El cazador imita lo que oyó de su presa, en un canon cuyo retardo es el viaje del sonido por el agua; sus clics de ecolocalización se aceleran hasta el zumbido terminal. La presa huye en disminución, o calla."),
            ("Rivales y aliados", "Los rivales se responden en hoquetus; los aliados cantan juntos; la pareja en cortejo copia el canto del otro. Los cantos pasan de organismo a organismo, como los de las ballenas."),
            ("Armonía y forma", "La sesión tiene un lenguaje armónico — funcional, modal, brasileño, impresionista, cromático —, modula, a veces deja las voces agudas en otra tonalidad, y sigue un arco de tensión hasta el clímax y el reposo."),
            ("Pregunta y respuesta", "La melodía respira después de las cadencias; otra voz le responde con su motivo transformado — imitado, invertido, al revés —, y el motivo vuelve más tarde."),
        ],
        "s_water": "El agua es la forma",
        "water": "El agua tiene cinco estados, y cada uno es una manera de tocar: el hielo es un cristal de alturas, seco; el agua fría es viscosa y ligada; la templada es laminar; la turbulencia trae rubato y heterofonía; el hervor trae burbujas que suenan a su propia altura física. El mar absorbe los agudos con la distancia; el agua dulce queda clara.",
        "states": ["HIELO", "FRÍA", "TEMPLADA", "TURBULENTA", "HERVOR"],
        "s_play": "Cómo se toca",
        "play": [
            ("El ambiente", "Densidad, deriva, cohesión, peligro, el agua y la memoria cambian tendencias; los acontecimientos vienen del ecosistema."),
            ("VARÍA", "Los controles se mueven solos a lo largo de la música, alrededor de donde la semilla o su mano los dejó — más cálidos en el clímax, asentándose en el reposo."),
            ("La voz de síntesis", "Cada organismo tiene un sintetizador que usted construye; en una matriz, la vida del bioma — el hambre, la tensión, la distancia, la temperatura del agua — toca los parámetros."),
            ("La lupa y los auriculares", "La lupa acerca su escucha a una región de la esfera; con auriculares, cada organismo llega desde su dirección, alrededor de su cabeza."),
        ],
        "s_shot": "La interfaz",
        "shot_alt": "El Aquorbium sonando: la esfera de agua con los cinco organismos y sus relaciones en el centro, las tarjetas alrededor y la franja de abajo con densidad, deriva, morph y VARÍA.",
        "shot_cap": "La app sonando, con el tutorial y LEARN en cuatro idiomas, de 960×600 a pantallas anchas.",
        "s_dl": "Descargar",
        "dl_intro_pub": "Elija el paquete de su sistema.",
        "dl_intro_prep": "La versión 0.1.0 está en preparación. Cuando salga, los paquetes estarán en la página de versiones de GitHub.",
        "dl_os": {"Windows": "Windows 10 u 11 — instalador (.exe) o sin instalar (.zip)", "macOS": "macOS 10.15 o posterior, Intel y Apple Silicon", "Linux": ".deb para Ubuntu, Debian y Mint; AppImage o .tar.gz para las demás distribuciones"},
        "dl_warn": "En la primera apertura, Windows y macOS muestran un aviso de seguridad: es esperable, porque la app no tiene la firma de pago de las dos empresas. La guía de instalación dice dónde hacer clic.",
        "dl_install": "Guía de instalación",
        "dl_all": "Página de versiones",
        "s_credits": "Créditos y fuentes",
        "credits": "Lúcio de Araújo — concepción, composición, escucha y código. Los conceptos vienen de la ecoacústica y la bioacústica (Krause, Schafer, los estudios de los cantos de ballenas y delfines), de la física del sonido en el agua y de la teoría musical; no se copió ningún código, sonido ni texto de esas obras. Licencia GNU AGPLv3 o posterior; JUCE bajo AGPLv3. Fuentes y créditos completos:",
        "credits_link": "CREDITS_AND_SOURCES.md",
        "s_state": "Estado",
        "state_pub": "La versión 0.1.0 es la primera versión pública. Los paquetes de Windows y macOS los construye y prueba la integración continua; el instrumento sigue en desarrollo, abierto.",
        "state_prep": "El Aquorbium se prepara para su primera versión pública. El código ya está abierto en GitHub.",
        "contact": "Contacto",
        "footer_credits": "Créditos",
        "footer_license": "Licencia AGPLv3+",
        "dialog_title": "CONTACTO",
        "dialog_copy_hint": "Copie la dirección de correo:",
        "dialog_copy": "Copiar correo",
        "dialog_close": "Cerrar ventana de contacto",
        "copied": "Correo copiado.",
        "fallback": "Seleccione y copie la dirección de arriba.",
    },
}


import re
import unicodedata

e = lambda s: html.escape(s, quote=False)
a = lambda s: html.escape(s, quote=True)


def cabeca(lang, titulo, descricao, scripts):
    robots = "" if PUBLICADO else '<meta name="robots" content="noindex" />\n'
    js = "".join('<script src="assets/%s" defer></script>\n' % x for x in scripts)
    return f"""<!doctype html>
<html lang="{lang}">
<head>
<meta charset="utf-8" />
<meta name="viewport" content="width=device-width, initial-scale=1" />
<title>{e(titulo)}</title>
<meta name="description" content="{a(descricao)}" />
{robots}<link rel="icon" href="assets/identity/aquorbium-favicon.svg" type="image/svg+xml" />
<link rel="stylesheet" href="styles.css" />
{js}</head>
<body>
"""


def cabecalho(lang, pagina):
    t, m = T[lang], MENU[lang]
    menu = []
    for chave in ("sobre", "guia", "tutorial", "ideia", "instalacao"):
        atual = ' aria-current="page"' if chave == pagina else ""
        menu.append('      <a href="%s"%s>%s</a>' % (ARQUIVOS[chave][lang], atual, e(m[chave])))
    menu.append('      <a href="%s#baixar">%s</a>' % (ARQUIVOS["sobre"][lang], e(m["baixar"])))
    langs = "\n".join('      <a href="%s"%s>%s</a>' % (ARQUIVOS[pagina][l], ' aria-current="true"' if l == lang else "", c)
                      for l, c in LINGUAS)
    return f"""<div class="rasgo-strip"><a href="{PORTAL}">{e(t["strip"])}</a></div>
<a class="skip-link" href="#conteudo">{e(t["skip"])}</a>

<div class="site-header">
  <a class="brand" href="{ARQUIVOS["sobre"][lang]}" aria-label="Aquorbium">
    <img src="assets/identity/aquorbium-symbol.svg" alt="" width="40" height="40" />
    <img class="wordmark" src="assets/identity/aquorbium-wordmark.svg" alt="AQUORBIUM" width="196" height="25" />
  </a>
  <div class="header-nav">
    <nav class="page-nav" aria-label="{a(m["nav"])}">
{chr(10).join(menu)}
    </nav>
    <nav class="lang-switch" aria-label="{a(t["langnav"])}">
{langs}
    </nav>
  </div>
</div>

"""


def rodape(lang):
    t = T[lang]
    return f"""
<footer class="site-footer">
  <span>© Lúcio de Araújo · Aquorbium 2026 · RASGO</span>
  <nav>
    <a href="{REPO}">GitHub</a> ·
    <a href="{BLOB}/CREDITS_AND_SOURCES.md">{e(t["footer_credits"])}</a> ·
    <a href="{BLOB}/LICENSE-AGPL-3.0.txt">{e(t["footer_license"])}</a>
  </nav>
</footer>
<dialog class="contact-dialog" data-contact-dialog aria-labelledby="contact-title">
  <button class="dialog-close" type="button" data-contact-close aria-label="{a(t["dialog_close"])}">×</button>
  <h2 id="contact-title">{e(t["dialog_title"])}</h2>
  <p>{e(t["dialog_copy_hint"])}</p>
  <code data-contact-address></code>
  <button class="button button-primary" type="button" data-contact-copy>{e(t["dialog_copy"])}</button>
  <p class="contact-feedback" data-contact-feedback data-copied="{a(t["copied"])}" data-fallback="{a(t["fallback"])}" aria-live="polite"></p>
</dialog>
</body>
</html>
"""


def pagina_sobre(lang):
    t = T[lang]
    seres = SERES[lang]
    rotulos = seres["labels"]
    organismos = "\n".join(
        '    <li style="--c: var(--%s)"><h3>%s</h3>%s</li>' % (cor, nome, "".join(
            '<p><strong>%s.</strong> %s</p>' % (e(rotulo), e(texto)) for rotulo, texto in zip(rotulos, seres[nome])))
        for nome, cor in ORGANISMOS)
    musica = "\n".join('    <li><h3>%s</h3><p>%s</p></li>' % (e(h), e(p)) for h, p in t["music"])
    tocar = "\n".join('    <li><h3>%s</h3><p>%s</p></li>' % (e(h), e(p)) for h, p in t["play"])
    estados = "\n".join('    <li>%s</li>' % e(s) for s in t["states"])
    if PUBLICADO:
        pacotes = "\n".join(
            '    <li><h3>%s</h3><p>%s</p><p class="dl-links">%s</p></li>' % (
                e(so), e(t["dl_os"][so]), " ".join('<a class="button" href="%s%s">%s</a>' % (DL, a(f), e(rot)) for f, rot in arqs))
            for so, arqs in PACOTES)
    else:
        pacotes = "\n".join('    <li><h3>%s</h3><p>%s</p></li>' % (e(so), e(t["dl_os"][so])) for so, _ in PACOTES)
    sementes = json.dumps(t["seed_ui"], ensure_ascii=False)
    instalacao = ARQUIVOS["instalacao"][lang]
    return (cabeca(lang, t["title"], t["desc"], ["contact.js", "sonar.js"]) + cabecalho(lang, "sobre") + f"""<main id="conteudo">
<section class="hero" id="top">
  <div class="hero-text">
    <span class="status-pill">{e(t["pill_pub"] if PUBLICADO else t["pill_prep"])}</span>
    <h1>Aquorbium</h1>
    <p class="lead">{e(t["lead"])}</p>
    <div class="contact-row">
      <a class="button button-primary" href="#baixar">{e(t["s_dl"])}</a>
      <a class="button" href="#semente">{e(t["s_seed"])}</a>
    </div>
  </div>
  <canvas class="sonar" data-sonar role="img" aria-label="{a(t["sonar_label"])}"></canvas>
</section>

<section class="section">
  <h2>{e(t["s_org"])}</h2>
  <ul class="organisms">
{organismos}
  </ul>
  <p>{e(t["org_note"])}</p>
</section>

<section class="section section-dark" id="semente">
  <h2>{e(t["s_seed"])}</h2>
  <p>{e(t["seed_intro"])}</p>
  <div class="seed" data-seed aria-live="polite"></div>
  <div class="contact-row">
    <button type="button" class="button button-primary" data-seed-again>{e(t["seed_again"])}</button>
  </div>
  <script type="application/json" data-seed-text>{sementes}</script>
</section>

<section class="section">
  <h2>{e(t["s_music"])}</h2>
  <ul class="steps">
{musica}
  </ul>
</section>

<section class="section section-dark">
  <h2>{e(t["s_water"])}</h2>
  <ol class="water-states">
{estados}
  </ol>
  <p>{e(t["water"])}</p>
</section>

<section class="section">
  <h2>{e(t["s_play"])}</h2>
  <ul class="steps">
{tocar}
  </ul>
</section>

<section class="section section-dark">
  <h2>{e(t["s_shot"])}</h2>
  <figure class="shot">
    <img src="assets/interface.jpg" alt="{a(t["shot_alt"])}" width="1600" height="838" loading="lazy" />
    <figcaption>{e(t["shot_cap"])}</figcaption>
  </figure>
</section>

<section class="section download" id="baixar">
  <h2>{e(t["s_dl"])}</h2>
  <p>{e(t["dl_intro_pub"] if PUBLICADO else t["dl_intro_prep"])}</p>
  <ul class="steps downloads">
{pacotes}
  </ul>
  <p class="example">{e(t["dl_warn"])}</p>
  <div class="contact-row">
    <a class="button button-primary" href="{instalacao}">{e(t["dl_install"])}</a>
    <a class="button" href="{RELEASES}">{e(t["dl_all"])}</a>
  </div>
</section>

<section class="section section-dark">
  <h2>{e(t["s_credits"])}</h2>
  <p>{e(t["credits"])} <a href="{BLOB}/CREDITS_AND_SOURCES.md">{e(t["credits_link"])}</a>.</p>
</section>

<section class="section">
  <h2>{e(t["s_state"])}</h2>
  <p>{e(t["state_pub"] if PUBLICADO else t["state_prep"])}</p>
  <div class="contact-row">
    <button type="button" class="button button-primary" data-contact-trigger>{e(t["contact"])}</button>
    <a class="button" href="{REPO}">GitHub</a>
  </div>
</section>
</main>""" + rodape(lang))


# ---- o tutorial, lido do próprio app (src/app/Tutorial.h) -------------------
LIT = r'"(?:[^"\\]|\\.)*"(?:\s*"(?:[^"\\]|\\.)*")*'
L4 = re.compile(r'\{\s*(' + LIT + r')\s*,\s*(' + LIT + r')\s*,\s*(' + LIT + r')\s*,\s*(' + LIT + r')\s*\}')


def texto_c(literal):
    partes = re.findall(r'"((?:[^"\\]|\\.)*)"', literal)
    junto = "".join(partes)
    return junto.encode("utf-8").decode("unicode_escape").encode("latin-1").decode("utf-8")


def capitulos():
    fonte = (AQUI.parent / "src/app/Tutorial.h").read_text(encoding="utf-8")
    bloco = fonte[fonte.index("chapters {{"):fonte.index("}};", fonte.index("chapters {{"))]
    textos = [[texto_c(g) for g in m.groups()] for m in L4.finditer(bloco)]
    assert len(textos) % 3 == 0, "Tutorial.h: chapters must be title, body, tip"
    return [(textos[k], textos[k + 1], textos[k + 2]) for k in range(0, len(textos), 3)]


def paragrafos(texto):
    saida = []
    for bloco in texto.split("\n\n"):
        linhas = bloco.split("\n")
        if len(linhas) > 1 and all(re.match(r"\d+\. ", l) for l in linhas):
            saida.append("<ol>" + "".join("<li>%s</li>" % e(re.sub(r"^\d+\. ", "", l)) for l in linhas) + "</ol>")
        elif len(linhas) > 1 and re.match(r"\d+\. ", linhas[0]) is None and all(re.match(r"\d+\. ", l) for l in linhas[1:]):
            saida.append("<p>%s</p><ol>%s</ol>" % (e(linhas[0]), "".join("<li>%s</li>" % e(re.sub(r"^\d+\. ", "", l)) for l in linhas[1:])))
        else:
            saida.append("<p>%s</p>" % "<br>".join(e(l) for l in linhas))
    return "\n".join(saida)


def pagina_tutorial(lang):
    o = OUTRAS[lang]
    k = {"en": 0, "pt-BR": 1, "fr": 2, "es": 3}[lang]  # the order of L4 in the app
    caps = capitulos()
    sumario = "\n".join('      <li><a href="#cap%d">%s</a></li>' % (n + 1, e(c[0][k])) for n, c in enumerate(caps))
    corpo = "\n".join(
        f"""<section class="chapter" id="cap{n + 1}">
  <h2><span class="chapter-number">{n + 1}</span> {e(c[0][k])}</h2>
  {paragrafos(c[1][k])}
  <p class="tip"><strong>{e(o["tip"])}:</strong> {e(c[2][k])}</p>
</section>""" for n, c in enumerate(caps))
    return (cabeca(lang, o["tutorial_title"], o["tutorial_desc"], ["contact.js"]) + cabecalho(lang, "tutorial") + f"""<main id="conteudo" class="doc">
<section class="doc-head">
  <h1>{e(o["tutorial_h1"])}</h1>
  <p class="lead">{e(o["tutorial_lead"])}</p>
</section>
<div class="doc-body">
  <nav class="doc-toc" aria-label="{a(o["contents"])}">
    <h2>{e(o["contents"])}</h2>
    <ol>
{sumario}
    </ol>
  </nav>
  <div class="doc-text">
{corpo}
  </div>
</div>
</main>
""" + rodape(lang))


# ---- a instalação, lida do INSTALL.md ---------------------------------------
SECOES_INSTALL = {"en": "## English", "pt-BR": "## Português", "fr": "## Français", "es": "## Español"}


def ancora(titulo):
    plano = unicodedata.normalize("NFKD", titulo.lower()).encode("ascii", "ignore").decode()
    return re.sub(r"[^a-z0-9]+", "-", plano).strip("-") or "s"


def inline(texto):
    texto = e(texto)
    texto = re.sub(r"`([^`]+)`", r"<code>\1</code>", texto)
    texto = re.sub(r"\*\*([^*]+)\*\*", r"<strong>\1</strong>", texto)

    def link(m):
        rotulo, alvo = m.group(1), m.group(2)
        if alvo.startswith("#"):
            alvo = "#" + ancora(alvo[1:].rstrip("-0123456789").replace("-", " "))
        return '<a href="%s">%s</a>' % (alvo, rotulo)
    return re.sub(r"\[([^\]]+)\]\(([^)]+)\)", link, texto)


def markdown(md):
    linhas = md.split("\n")
    saida, i = [], 0
    while i < len(linhas):
        l = linhas[i]
        if l.startswith("```"):
            j = i + 1
            while not linhas[j].startswith("```"):
                j += 1
            saida.append("<pre><code>%s</code></pre>" % e("\n".join(x.strip() for x in linhas[i + 1:j])))
            i = j + 1
        elif l.strip().startswith("```"):
            j = i + 1
            while not linhas[j].strip().startswith("```"):
                j += 1
            saida.append("<pre><code>%s</code></pre>" % e("\n".join(x.strip() for x in linhas[i + 1:j])))
            i = j + 1
        elif l.startswith("#### ") or l.startswith("### "):
            nivel = "h3" if l.startswith("#### ") else "h2"
            titulo = l.split(" ", 1)[1]
            saida.append('<%s id="%s">%s</%s>' % (nivel, ancora(titulo), inline(titulo), nivel))
            i += 1
        elif l.startswith("|"):
            j = i
            while j < len(linhas) and linhas[j].startswith("|"):
                j += 1
            celulas = [[c.strip() for c in r.strip("|").split("|")] for r in linhas[i:j] if not re.match(r"^\|[-| ]+\|$", r)]
            saida.append("<table>" + "".join("<tr>" + "".join("<td>%s</td>" % inline(c) for c in r) + "</tr>" for r in celulas if any(r)) + "</table>")
            i = j
        elif re.match(r"^\s*(\d+\.|-) ", l):
            # a list, with its continuation lines and one level of sub-items
            ordenada = re.match(r"^\s*\d+\.", l) is not None
            itens, j = [], i
            while j < len(linhas) and (re.match(r"^\s*(\d+\.|-) ", linhas[j]) or (linhas[j].startswith("  ") and linhas[j].strip())):
                if linhas[j].strip().startswith("```"):
                    break
                itens.append(linhas[j])
                j += 1
            html_itens, atual, sub = [], None, []
            for item in itens:
                profundo = len(item) - len(item.lstrip()) >= 2
                m = re.match(r"^\s*(\d+\.|-) (.*)", item)
                if m and not profundo:
                    if atual is not None:
                        html_itens.append((atual, sub))
                    atual, sub = m.group(2), []
                elif m and profundo:
                    sub.append(m.group(2))
                elif sub:
                    sub[-1] += " " + item.strip()
                elif atual is not None:
                    atual += " " + item.strip()
            if atual is not None:
                html_itens.append((atual, sub))
            tag = "ol" if ordenada else "ul"
            saida.append("<%s>%s</%s>" % (tag, "".join(
                "<li>%s%s</li>" % (inline(x), ("<ul>" + "".join("<li>%s</li>" % inline(y) for y in ys) + "</ul>") if ys else "")
                for x, ys in html_itens), tag))
            i = j
        elif l.strip() == "":
            i += 1
        else:
            j = i
            while j < len(linhas) and linhas[j].strip() and not re.match(r"^(#|\||```|\s*(\d+\.|-) )", linhas[j]):
                j += 1
            saida.append("<p>%s</p>" % inline(" ".join(x.strip() for x in linhas[i:j])))
            i = j
    return "\n".join(saida)


def pagina_instalacao(lang):
    o = OUTRAS[lang]
    md = (AQUI.parent / "INSTALL.md").read_text(encoding="utf-8")
    inicio = md.index(SECOES_INSTALL[lang])
    seguintes = [md.index(h) for h in SECOES_INSTALL.values() if md.index(h) > inicio]
    trecho = md[inicio:min(seguintes) if seguintes else len(md)]
    trecho = trecho.split("\n", 1)[1]
    return (cabeca(lang, o["install_title"], o["install_desc"], ["contact.js"]) + cabecalho(lang, "instalacao") + f"""<main id="conteudo" class="doc">
<section class="doc-head">
  <h1>{e(o["install_h1"])}</h1>
</section>
<div class="doc-text install">
{markdown(trecho)}
</div>
</main>
""" + rodape(lang))



# ---- os textos do próprio app (src/app/UiLanguage.h, LearnCatalog.h) --------
# A página "Como tocar" é montada com o que o app diz no LEARN: o que cada
# cartão faz e o que cada controle faz, nas quatro línguas. Assim a página e o
# app nunca divergem. A ordem do L4 no app é EN, PT, FR, ES.
ORDEM_L4 = {"en": 0, "pt-BR": 1, "fr": 2, "es": 3}


def textos_do_app():
    ui = (AQUI.parent / "src/app/UiLanguage.h").read_text(encoding="utf-8")
    learn = (AQUI.parent / "src/app/LearnCatalog.h").read_text(encoding="utf-8")
    nomes = {}
    for m in re.finditer(r"inline const L4 (\w+) (\{\s*" + LIT + r"\s*,\s*" + LIT + r"\s*,\s*" + LIT + r"\s*,\s*" + LIT + r"\s*\})", ui):
        chave = m.group(1)
        while chave in nomes:
            chave += "#2"
        nomes[chave] = [texto_c(g) for g in L4.match(m.group(2)).groups()]
    controles = {}
    inicio = ui.index("controlTexts()")
    for m in re.finditer(r'\{ "(\w+)", (\{\s*' + LIT + r"(?:\s*,\s*" + LIT + r"){3}\s*\}),\s*(\{\s*" + LIT + r"(?:\s*,\s*" + LIT + r"){3}\s*\})", ui[inicio:]):
        controles[m.group(1)] = ([texto_c(g) for g in L4.match(m.group(2)).groups()],
                                 [texto_c(g) for g in L4.match(m.group(3)).groups()])
    cartoes = {}
    for m in re.finditer(r'\{ "(\w+)", ((?:\{\s*' + LIT + r"(?:\s*,\s*" + LIT + r"){3}\s*\},?\s*){4})", learn):
        quatro = [[texto_c(g) for g in x.groups()] for x in L4.finditer(m.group(2))]
        cartoes[m.group(1)] = quatro  # title, function, understand, explore
    bloco = ui[ui.index("moduleDetails()"):]
    detalhes = [[texto_c(g) for g in x.groups()] for x in L4.finditer(bloco[:bloco.index("}};")])]
    main = (AQUI.parent / "src/app/Main.cpp").read_text(encoding="utf-8")
    rotulos = re.findall(r'\{ "module\w+", "([^"]+)", "', main)
    tutorial = (AQUI.parent / "src/app/Tutorial.h").read_text(encoding="utf-8")
    sobre = tutorial[tutorial.index('"ABOUT THE AQUORBIUM"'):]
    corpos = [[texto_c(g) for g in x.groups()] for x in L4.finditer("{ " + sobre)]
    return nomes, controles, cartoes, list(zip(rotulos, detalhes)), corpos[1]


# cada seção: o cartão do LEARN (ou None), a imagem, os controles por id e os
# textos avulsos do app (nome, dica) que entram como controles
SECOES = [
    ("ecosystem", "ecosystem", ["cohesion", "danger", "sparseFocus", "energyMemory", "geometricCycle", "collectiveDrift"], []),
    ("strip", "strip", ["density", "drift", "ecosystemMorph", "geometryInfluence", "rhythmDensity", "orbitRotation", "intervalBias", "geometryMutation"],
     [("autonomous", "autonomousTip"), ("gate", "gateTip"), ("vary", "varyTip")]),
    ("brain", "brain", ["brainClock", "brainCoordination"], []),
    ("geometry", "geometry", ["formInfluence", "formContext", "formInertia", "formMutation", "structureInfluence", "structureMemory",
                              "structureHarmony", "structurePhase", "musicInfluence", "musicReaction", "musicMemory", "musicWater"], []),
    ("timbre", "timbre", [], [("body", "bodyTip"), ("matter", "matterTip"), ("mix", "mixTip#2"), ("timbreEcology", "timbreEcologyTip"),
                              ("support", "supportTip")]),
    ("editor", "editor", [], [("propose", "proposeTip"), ("mix", "mixTip")]),
    ("synthesis", "synthesis", ["strikerInfluence", "strikerMatter", "strikerPopulation", "strikerScatter"], []),
    ("space", "space", ["spaceReturn", "spaceGesture", "spaceSize", "spaceSpread", "spaceTone", "spaceMemory"], []),
    ("environment", "environment", ["environmentInfluence", "environmentFilterMix", "environmentCentre", "environmentSpan",
                                    "environmentResonance", "environmentAmbience"], []),
    ("tail", "tail", ["tailInfluence", "tailDecay", "tailDamping", "tailNightmare"], []),
    ("master", "master", ["creativeMaster", "masterTrim", "masterCeiling", "masterRelease", "masterEqOrder", "masterMultiband",
                          "masterGlue", "masterTilt", "masterWidth"], []),
    ("modules", "modules", [], []),
    ("lens", "lens", ["lensRadius", "lensReach", "lensMagnification", "lensBlur"], []),
    ("meters", "meters", [], []),
    ("header", "header", [], [("rec", "recTip"), ("mute", "muteTip"), ("pause", "pauseTip"), ("speakers", "listeningTip"),
                              ("audio", "audioTip"), ("learn", "learnTip"), ("tutorial", "tutorialTip"), ("about", "aboutTip")]),
]


def pagina_guia(lang):
    o, g = OUTRAS[lang], GUIA[lang]
    k = ORDEM_L4[lang]
    nomes, controles, cartoes, modulos, _ = textos_do_app()
    pref = {"pt-BR": "pt", "en": "en", "fr": "fr", "es": "es"}[lang]
    partes, sumario = [], []
    for chave, imagem, ids, avulsos in SECOES:
        if chave in cartoes:
            titulo, funcao, entender, explorar = (x[k] for x in cartoes[chave])
        else:
            titulo, funcao, entender, explorar = g["extra"][chave]
        linhas = []
        for cid in ids:
            nome, dica = controles[cid]
            linhas.append((nome[k], dica[k]))
        for nome_id, dica_id in avulsos:
            if nome_id == "rec":
                linhas.append(("REC", nomes[dica_id][k]))
            elif nome_id == "speakers":
                linhas.append((nomes["speakers"][k] + " / " + nomes["headphones"][k], nomes[dica_id][k]))
            else:
                linhas.append((nomes[nome_id][k], nomes[dica_id][k]))
        if chave == "modules":
            linhas = [(rot, det[k]) for rot, det in modulos]
        if chave == "meters":
            linhas = []
        tabela = "".join('<tr><th scope="row">%s</th><td>%s</td></tr>' % (e(n), e(d)) for n, d in linhas)
        tabela = ('<table class="controls"><caption>%s</caption>%s</table>' % (e(g["controls"]), tabela)) if linhas else ""
        tentar = ('<p class="tip"><strong>%s</strong> %s</p>' % (e(g["try"]), e(explorar))) if explorar else ""
        alt = "%s — %s" % (titulo, funcao)
        sumario.append('      <li><a href="#%s">%s</a></li>' % (chave, e(titulo)))
        partes.append(f"""<section class="chapter guide-card" id="{chave}">
  <h2>{e(titulo)}</h2>
  <p class="function">{e(funcao)}</p>
  <figure class="guide-shot guide-{chave}"><img src="assets/guia/{pref}-{imagem}.jpg" alt="{a(alt)}" loading="lazy" /></figure>
  <p>{e(entender)}</p>
  {tentar}
  {tabela}
</section>""")
    tela = f"""<section class="chapter" id="tela">
  <h2>{e(g["screen_h"])}</h2>
  <figure class="guide-shot"><img src="assets/interface.jpg" alt="{a(T[lang]["shot_alt"])}" loading="lazy" /></figure>
  {"".join("<p>%s</p>" % e(x) for x in g["screen"])}
</section>"""
    sumario.insert(0, '      <li><a href="#tela">%s</a></li>' % e(g["screen_h"]))
    return (cabeca(lang, g["title"], g["desc"], ["contact.js"]) + cabecalho(lang, "guia") + f"""<main id="conteudo" class="doc">
<section class="doc-head">
  <h1>{e(g["h1"])}</h1>
  <p class="lead">{e(g["lead"])}</p>
</section>
<div class="doc-body">
  <nav class="doc-toc" aria-label="{a(o["contents"])}">
    <h2>{e(o["contents"])}</h2>
    <ol>
{chr(10).join(sumario)}
    </ol>
  </nav>
  <div class="doc-text">
{tela}
{chr(10).join(partes)}
  </div>
</div>
</main>
""" + rodape(lang))


def pagina_ideia(lang):
    d = IDEIA[lang]
    k = ORDEM_L4[lang]
    sobre = textos_do_app()[4][k]
    blocos = "\n".join(
        '<section class="chapter" id="%s">\n  <h2>%s</h2>\n%s\n</section>' % (sid, e(h), "\n".join("  <p>%s</p>" % e(p) for p in ps))
        for sid, h, ps in d["sections"])
    return (cabeca(lang, d["title"], d["desc"], ["contact.js"]) + cabecalho(lang, "ideia") + f"""<main id="conteudo" class="doc">
<section class="doc-head">
  <h1>{e(d["h1"])}</h1>
  <p class="lead">{e(d["lead"])}</p>
</section>
<div class="doc-text install">
{blocos}
<section class="chapter" id="referencias">
  <h2>{e(d["refs_h"])}</h2>
  {paragrafos(sobre)}
  <p><a href="{BLOB}/CREDITS_AND_SOURCES.md">CREDITS_AND_SOURCES.md</a> · <a href="{BLOB}/docs/DESENVOLVIMENTO.md">docs/DESENVOLVIMENTO.md</a></p>
</section>
<section class="chapter" id="licenca">
  <h2>{e(d["license_h"])}</h2>
  {"".join("<p>%s</p>" % e(p) for p in d["license"])}
  <p><a class="button" href="{BLOB}/LICENSE-AGPL-3.0.txt">LICENSE-AGPL-3.0.txt</a> <a class="button" href="https://www.gnu.org/licenses/agpl-3.0.html">gnu.org</a></p>
</section>
</div>
</main>
""" + rodape(lang))


def main():
    n = 0
    for lang, _ in LINGUAS:
        for chave, gerar in (("sobre", pagina_sobre), ("guia", pagina_guia), ("tutorial", pagina_tutorial),
                             ("ideia", pagina_ideia), ("instalacao", pagina_instalacao)):
            (AQUI / ARQUIVOS[chave][lang]).write_text(gerar(lang), encoding="utf-8")
            n += 1
    print("gerar_paginas.py: %d páginas" % n)


if __name__ == "__main__":
    main()
