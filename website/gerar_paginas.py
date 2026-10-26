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
PUBLICADO = False
RELEASES = REPO + "/releases"
DL = REPO + "/releases/download/v" + VERSAO + "/"
PACOTES = [
    ("Windows", [("aquorbium-%s-win64.exe" % VERSAO, "exe"), ("aquorbium-%s-win64.zip" % VERSAO, "zip")]),
    ("macOS", [("aquorbium-%s-Darwin.dmg" % VERSAO, "dmg")]),
    ("Linux", [("aquorbium-%s-Linux.deb" % VERSAO, "deb"), ("aquorbium-%s-linux-x86_64.AppImage" % VERSAO, "AppImage"),
               ("aquorbium-%s-linux-x86_64.tar.gz" % VERSAO, "tar.gz")]),
]

PAGINAS = [("index.html", "pt-BR", "PT"), ("en.html", "en", "EN"), ("fr.html", "fr", "FR"), ("es.html", "es", "ES")]

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


def pagina(arquivo, lang):
    t = T[lang]
    e = lambda s: html.escape(s, quote=False)
    a = lambda s: html.escape(s, quote=True)
    langs = "\n".join('      <a href="%s"%s>%s</a>' % (f, ' aria-current="true"' if f == arquivo else "", c) for f, _, c in PAGINAS)
    organismos = "\n".join(
        '    <li style="--c: var(--%s)"><h3>%s</h3><p>%s</p></li>' % (cor, nome, e(t["org"][nome])) for nome, cor in ORGANISMOS)
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
    robots = "" if PUBLICADO else '<meta name="robots" content="noindex" />\n'
    sementes = json.dumps(t["seed_ui"], ensure_ascii=False)
    return f"""<!doctype html>
<html lang="{lang}">
<head>
<meta charset="utf-8" />
<meta name="viewport" content="width=device-width, initial-scale=1" />
<title>{e(t["title"])}</title>
<meta name="description" content="{a(t["desc"])}" />
{robots}<link rel="icon" href="assets/identity/aquorbium-favicon.svg" type="image/svg+xml" />
<link rel="stylesheet" href="styles.css" />
<script src="assets/contact.js" defer></script>
<script src="assets/sonar.js" defer></script>
</head>
<body>
<div class="rasgo-strip"><a href="{PORTAL}">{e(t["strip"])}</a></div>
<a class="skip-link" href="#conteudo">{e(t["skip"])}</a>

<div class="site-header">
  <a class="brand" href="#top" aria-label="Aquorbium">
    <img src="assets/identity/aquorbium-symbol.svg" alt="" width="40" height="40" />
    <img class="wordmark" src="assets/identity/aquorbium-wordmark.svg" alt="AQUORBIUM" width="196" height="25" />
  </a>
  <nav class="lang-switch" aria-label="{a(t["langnav"])}">
{langs}
  </nav>
</div>

<main id="conteudo">
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
    <a class="button button-primary" href="{BLOB}/INSTALL.md">{e(t["dl_install"])}</a>
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
</main>

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


def main():
    for arquivo, lang, _ in PAGINAS:
        (AQUI / arquivo).write_text(pagina(arquivo, lang), encoding="utf-8")
    print("gerar_paginas.py: %d páginas" % len(PAGINAS))


if __name__ == "__main__":
    main()
