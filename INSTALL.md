# Aquorbium — Installation Guide / Guia de Instalação / Guide d'installation / Guía de instalación

The installation steps for Windows and macOS follow the Trioio and Antitotem
guides (`lucioaraujo/trioio`, `lucioaraujo/antitotem`, `INSTALL.md`), checked
on real machines by the author; the requirements below were measured for the
Aquorbium itself.

- [English](#english)
- [Português](#português)
- [Français](#français)
- [Español](#español)

## English

### Installing, step by step

Packages for every system are on the
[releases page](https://github.com/lucioaraujo/aquorbium/releases); the
newest one is at the top. The security warnings below **are expected**. They
do not mean anything is wrong, and they only appear the first time.

#### Windows

1. Download `aquorbium-<version>-win64.exe`.
2. Open it. Windows shows a blue **"Windows protected your PC"** box: click
   **"More info"**, then **"Run anyway"**.
3. If Windows asks for administrator permission, click **"Yes"**.
4. Click **"Next"** to the end. The last page has **"Run Aquorbium"** already
   ticked. Aquorbium is then in the **Start menu** and on the desktop.

**Without installing:** unzip `aquorbium-<version>-win64.zip` and open
`Aquorbium.exe`.

#### macOS

1. Download `aquorbium-<version>-Darwin.dmg`. It works on Intel and on Apple
   Silicon Macs.
2. Open it and **drag Aquorbium into Applications**.
3. Open it. The first time, macOS **blocks** the app with a message saying
   that Apple could not confirm it is free of malicious software. You may see
   "cannot verify the developer" or "is damaged and can't be opened" instead.
   In every case:
   - This is **not a defect or a virus**: the app is signed ad hoc, without a
     paid Apple Developer ID.
   - Click **"OK"** or **"Done"**, **not** "Move to Trash".
4. Allow the app, once, in one of two ways:
   - **In System Settings:** go to **Privacy & Security**, scroll to the
     security section, click **"Open Anyway"**, confirm with your password or
     Touch ID, and click **"Open"**. On macOS 14 or earlier: right-click the
     app in Finder → **Open → Open**.
   - **In Terminal**, if the button does not appear:

     ```sh
     xattr -dr com.apple.quarantine "/Applications/Aquorbium.app"
     ```

     If nothing is printed, it worked. The command only removes the
     "quarantine mark" macOS puts on every downloaded file.

#### Linux

- **Ubuntu 22.04+, Debian 12+, Mint 21+:** double-click the `.deb`, or run
  `sudo apt install ./aquorbium-<version>-Linux.deb`.
- **Fedora, Arch, openSUSE and other distributions:**
  - **AppImage:** make it executable (Properties → allow executing, or
    `chmod +x`) and open it.
  - **`.tar.gz`:** unpack it and run `./install.sh`. It installs for your
    user only, without root, and `./install.sh --remove` undoes it.

### The first time you open it

1. **Every opening is another piece.** The session's seed proposes a biome -
   the water, the light, the population, the bodies, which modules wake - and
   a world of keys, harmony and form. With **VARY** on (bottom strip) the
   controls also move slowly by themselves. Play from there.
2. The **TUTORIAL** button explains the instrument step by step, and **LEARN**
   explains any control you point at, in English, Portuguese, French or
   Spanish (the language button: EN/PT/FR/ES).
3. **AUDIO** chooses the output device. **HEADPHONES** places each organism
   around your head: use it with headphones only.
4. The microphone (the **ROOM**) stays off until you turn it on.

### Troubleshooting

**Windows: "Windows protected your PC".** Click **"More info"**, then **"Run
anyway"**. If an antivirus moves `Aquorbium.exe` to quarantine, restore it and
allow it: the program is built by the public continuous integration of this
repository, from this source code.

**macOS: "damaged", "cannot verify the developer" or "Apple could not
confirm".** Follow step 4 of [macOS](#macos): **Open Anyway** in Privacy &
Security, or the `xattr` command in Terminal.

**Linux: the window never opens.** Install the `.deb` with `apt`, which also
installs the dependencies. With the AppImage or the `.tar.gz`, the system
needs ALSA, FreeType, Fontconfig, X11 and GLib, which any desktop has. If the
**AppImage** does not open, install `libfuse2` (Ubuntu 22.04+: `sudo apt
install libfuse2`), or start it with
`./aquorbium-<version>-linux-x86_64.AppImage --appimage-extract-and-run`.

**No sound.** Click **AUDIO** and choose the output. Also check the **OUTPUT
TRIM** in the header and that **MUTE** and **PAUSE** are off.

**Clicks or dropouts.** In **AUDIO**, choose a buffer of **512** samples. On
Linux the audio thread asks for real-time priority; the status line shows
**RT 70** when it gets it and **RT no** when it does not. To allow it, add
your user to the `audio` or `pipewire` group (depending on the distribution)
and restart the computer. Heavy programs running beside the Aquorbium
(compilers, video) also cause dropouts.

**Anything else:** write to **rasgo.instruments@gmail.com** with your system
and version, what you did, and the message you saw. You can also open an
[issue](https://github.com/lucioaraujo/aquorbium/issues).

### Requirements

| | |
|---|---|
| **Windows** | Windows 10 or 11, 64-bit |
| **macOS** | 10.15 or later; Intel and Apple Silicon (Universal 2) |
| **Linux** | x86-64 with glibc 2.35 or newer (2022+ distributions) |
| **Processor** | 64-bit, two cores or more. **Measured** on an Intel Core i5-6500 (2015, 4 cores) at 48 kHz with 512-sample blocks: the audio uses about 7 % of each block's time (at most 9 %) |
| **Memory** | about 55 MB while playing (measured) |
| **Disk** | about 15 MB |
| **Audio** | any output; a microphone only for the ROOM |

The processor figure was measured on the author's machine. Weaker machines
have not been tested.

### Building from source

```sh
git clone https://github.com/lucioaraujo/aquorbium.git
git clone --branch 9.0.3 https://github.com/juce-framework/JUCE.git
cmake -S aquorbium -B aquorbium/build -DCMAKE_BUILD_TYPE=Release -DAQUORBIUM_JUCE_PATH=$PWD/JUCE
cmake --build aquorbium/build --parallel 2
(cd aquorbium/build && ctest -LE slow --output-on-failure)
```

On Linux, the JUCE build needs the ALSA, FreeType, Fontconfig, X11 and GTK
development packages. The workflow `.github/workflows/package.yml` lists them.

### Status

Version 0.1.0 is the first public version. The Windows and macOS packages are
built, tested and checked by continuous integration: no dependency on the
Visual C++ Redistributable, and a sealed Universal 2 bundle on macOS. They
have not yet been opened by the author on real Windows and Mac machines.

### License

AGPL-3.0-or-later (`LICENSE-AGPL-3.0.txt`). Credits and sources:
`CREDITS_AND_SOURCES.md`. Problems: **rasgo.instruments@gmail.com**.

## Português

### Instalar, passo a passo

Os pacotes de cada sistema estão na
[página de releases](https://github.com/lucioaraujo/aquorbium/releases); o
mais novo fica no topo. Os avisos de segurança abaixo **são esperados**. Não
indicam nenhum problema e só aparecem na primeira vez.

#### Windows

1. Baixe `aquorbium-<versão>-win64.exe`.
2. Abra. O Windows mostra uma caixa azul **"O Windows protegeu o computador"**:
   clique em **"Mais informações"** e depois em **"Executar assim mesmo"**.
3. Se o Windows pedir permissão de administrador, clique em **"Sim"**.
4. Clique em **"Avançar"** até o fim. A última página já traz **"Executar
   Aquorbium"** marcado. O Aquorbium fica no **menu Iniciar** e na área de
   trabalho.

**Sem instalar:** descompacte `aquorbium-<versão>-win64.zip` e abra
`Aquorbium.exe`.

#### macOS

1. Baixe `aquorbium-<versão>-Darwin.dmg`. Funciona em Macs Intel e Apple
   Silicon.
2. Abra e **arraste o Aquorbium para Aplicativos**.
3. Abra. Na primeira vez, o macOS **bloqueia** o app com uma mensagem dizendo
   que a Apple não pôde confirmar que ele está livre de software malicioso.
   Pode aparecer também "não é possível verificar o desenvolvedor" ou "está
   danificado e não pode ser aberto". Em todos os casos:
   - **Não é defeito nem vírus**: o app é assinado ad hoc, sem um Developer ID
     pago da Apple.
   - Clique em **"OK"** ou **"Concluído"**, **não** em "Mover para o Lixo".
4. Permita o app, uma vez, de um destes jeitos:
   - **Nos Ajustes do Sistema:** vá em **Privacidade e Segurança**, desça até
     a seção de segurança, clique em **"Abrir Mesmo Assim"**, confirme com a
     senha ou o Touch ID e clique em **"Abrir"**. No macOS 14 ou anterior:
     clique com o botão direito no app, no Finder → **Abrir → Abrir**.
   - **No Terminal**, se o botão não aparecer:

     ```sh
     xattr -dr com.apple.quarantine "/Applications/Aquorbium.app"
     ```

     Se nada for impresso, deu certo. O comando só tira a "marca de
     quarentena" que o macOS põe em todo arquivo baixado.

#### Linux

- **Ubuntu 22.04+, Debian 12+, Mint 21+:** clique duas vezes no `.deb`, ou
  rode `sudo apt install ./aquorbium-<versão>-Linux.deb`.
- **Fedora, Arch, openSUSE e outras distribuições:**
  - **AppImage:** torne-o executável (Propriedades → permitir executar, ou
    `chmod +x`) e abra.
  - **`.tar.gz`:** descompacte e rode `./install.sh`. Instala só para o seu
    usuário, sem root; `./install.sh --remove` desfaz.

### Na primeira abertura

1. **Cada abertura é uma peça nova.** A semente da sessão propõe um bioma - a
   água, a luz, a população, os corpos, quais módulos acordam - e um mundo de
   tonalidades, harmonia e forma. Com o **VARIA** ligado (faixa de baixo), os
   controles também se movem devagar sozinhos. Toque a partir daí.
2. O botão **TUTORIAL** explica o instrumento passo a passo, e o **LEARN**
   explica qualquer controle que você apontar, em inglês, português, francês
   ou espanhol (o botão de idioma: EN/PT/FR/ES).
3. **ÁUDIO** escolhe a saída. **FONES** põe cada organismo em volta da sua
   cabeça: use só com fones.
4. O microfone (a **SALA**) fica desligado até você ligar.

### Problemas e soluções

**Windows: "O Windows protegeu o computador".** Clique em **"Mais
informações"** e em **"Executar assim mesmo"**. Se um antivírus puser o
`Aquorbium.exe` em quarentena, restaure e permita: o programa é construído
pela integração contínua pública deste repositório, a partir deste código.

**macOS: "danificado", "não é possível verificar o desenvolvedor" ou "a Apple
não pôde confirmar".** Siga o passo 4 do [macOS](#macos-1): **Abrir Mesmo
Assim** em Privacidade e Segurança, ou o comando `xattr` no Terminal.

**Linux: a janela não abre.** Instale o `.deb` com o `apt`, que também
instala as dependências. Com o AppImage ou o `.tar.gz`, o sistema precisa de
ALSA, FreeType, Fontconfig, X11 e GLib, que todo desktop tem. Se o
**AppImage** não abrir, instale o `libfuse2` (Ubuntu 22.04+: `sudo apt
install libfuse2`), ou abra com
`./aquorbium-<versão>-linux-x86_64.AppImage --appimage-extract-and-run`.

**Sem som.** Clique em **ÁUDIO** e escolha a saída. Confira também o **VOLUME
DE SAÍDA** no cabeçalho e se **SILENCIAR** e **PAUSA** estão desligados.

**Estalos ou falhas no som.** Em **ÁUDIO**, escolha um buffer de **512**
amostras. No Linux a thread de áudio pede prioridade de tempo real; a linha de
status mostra **RT 70** quando consegue e **RT não** quando não. Para
permitir, ponha o seu usuário no grupo `audio` ou `pipewire` (conforme a
distribuição) e reinicie o computador. Programas pesados ao lado do
Aquorbium (compiladores, vídeo) também causam falhas.

**Qualquer outra coisa:** escreva para **rasgo.instruments@gmail.com** com o
sistema e a versão, o que você fez e a mensagem que apareceu. Também pode
abrir uma [issue](https://github.com/lucioaraujo/aquorbium/issues).

### Requisitos

| | |
|---|---|
| **Windows** | Windows 10 ou 11, 64 bits |
| **macOS** | 10.15 ou mais novo; Intel e Apple Silicon (Universal 2) |
| **Linux** | x86-64 com glibc 2.35 ou mais nova (distribuições de 2022 em diante) |
| **Processador** | 64 bits, dois núcleos ou mais. **Medido** num Intel Core i5-6500 (2015, 4 núcleos), a 48 kHz e blocos de 512 amostras: o áudio usa cerca de 7 % do tempo de cada bloco (no máximo 9 %) |
| **Memória** | cerca de 55 MB tocando (medido) |
| **Disco** | cerca de 15 MB |
| **Áudio** | qualquer saída; microfone só para a SALA |

O valor do processador foi medido na máquina do autor. Máquinas mais fracas
não foram testadas.

### Compilar a partir do código

```sh
git clone https://github.com/lucioaraujo/aquorbium.git
git clone --branch 9.0.3 https://github.com/juce-framework/JUCE.git
cmake -S aquorbium -B aquorbium/build -DCMAKE_BUILD_TYPE=Release -DAQUORBIUM_JUCE_PATH=$PWD/JUCE
cmake --build aquorbium/build --parallel 2
(cd aquorbium/build && ctest -LE slow --output-on-failure)
```

No Linux, o build do JUCE precisa dos pacotes de desenvolvimento de ALSA,
FreeType, Fontconfig, X11 e GTK. O fluxo `.github/workflows/package.yml` traz
a lista.

### Estado

A versão 0.1.0 é a primeira versão pública. Os pacotes de Windows e macOS são
construídos, testados e conferidos pela integração contínua: sem dependência
do Visual C++ Redistributable, e um pacote Universal 2 selado no macOS. Ainda
não foram abertos pelo autor em Windows e Mac reais.

### Licença

AGPL-3.0-or-later (`LICENSE-AGPL-3.0.txt`). Créditos e fontes:
`CREDITS_AND_SOURCES.md`. Problemas: **rasgo.instruments@gmail.com**.

## Français

### Installer, pas à pas

Les paquets de chaque système sont sur la
[page des versions](https://github.com/lucioaraujo/aquorbium/releases) ; le
plus récent est en haut. Les avertissements de sécurité ci-dessous **sont
attendus**. Ils n'indiquent aucun problème et n'apparaissent que la première
fois.

#### Windows

1. Téléchargez `aquorbium-<version>-win64.exe`.
2. Ouvrez-le. Windows affiche une boîte bleue **« Windows a protégé votre
   ordinateur »** : cliquez sur **« Informations complémentaires »**, puis sur
   **« Exécuter quand même »**.
3. Si Windows demande l'autorisation d'administrateur, cliquez sur **« Oui »**.
4. Cliquez sur **« Suivant »** jusqu'au bout. La dernière page a déjà
   **« Lancer Aquorbium »** coché. Aquorbium est alors dans le **menu
   Démarrer** et sur le bureau.

**Sans installer :** décompressez `aquorbium-<version>-win64.zip` et ouvrez
`Aquorbium.exe`.

#### macOS

1. Téléchargez `aquorbium-<version>-Darwin.dmg`. Il fonctionne sur les Mac
   Intel et Apple Silicon.
2. Ouvrez-le et **glissez Aquorbium dans Applications**.
3. Ouvrez-le. La première fois, macOS **bloque** l'app avec un message disant
   qu'Apple n'a pas pu confirmer qu'elle est exempte de logiciel malveillant.
   Vous pouvez aussi voir « impossible de vérifier le développeur » ou « est
   endommagée et ne peut pas être ouverte ». Dans tous les cas :
   - Ce **n'est ni un défaut ni un virus** : l'app est signée ad hoc, sans
     Developer ID payant d'Apple.
   - Cliquez sur **« OK »** ou **« Terminé »**, **pas** sur « Placer dans la
     corbeille ».
4. Autorisez l'app, une fois, de l'une de ces façons :
   - **Dans Réglages Système :** allez dans **Confidentialité et sécurité**,
     descendez jusqu'à la section sécurité, cliquez sur **« Ouvrir quand
     même »**, confirmez avec votre mot de passe ou Touch ID, puis cliquez sur
     **« Ouvrir »**. Sous macOS 14 ou antérieur : clic droit sur l'app dans le
     Finder → **Ouvrir → Ouvrir**.
   - **Dans le Terminal**, si le bouton n'apparaît pas :

     ```sh
     xattr -dr com.apple.quarantine "/Applications/Aquorbium.app"
     ```

     Si rien ne s'affiche, c'est réussi. La commande retire seulement la
     « marque de quarantaine » que macOS pose sur tout fichier téléchargé.

#### Linux

- **Ubuntu 22.04+, Debian 12+, Mint 21+ :** double-cliquez sur le `.deb`, ou
  lancez `sudo apt install ./aquorbium-<version>-Linux.deb`.
- **Fedora, Arch, openSUSE et autres distributions :**
  - **AppImage :** rendez-le exécutable (Propriétés → autoriser l'exécution,
    ou `chmod +x`) et ouvrez-le.
  - **`.tar.gz` :** décompressez-le et lancez `./install.sh`. Il installe
    pour votre utilisateur seulement, sans root ; `./install.sh --remove`
    annule.

### À la première ouverture

1. **Chaque ouverture est une nouvelle pièce.** La graine de la session
   propose un biome - l'eau, la lumière, la population, les corps, les modules
   qui s'éveillent - et un monde de tonalités, d'harmonie et de forme. Avec
   **VARIE** allumé (bande du bas), les commandes bougent aussi lentement
   toutes seules. Jouez à partir de là.
2. Le bouton **TUTORIEL** explique l'instrument pas à pas, et **LEARN**
   explique toute commande que vous pointez, en anglais, portugais, français
   ou espagnol (le bouton de langue : EN/PT/FR/ES).
3. **AUDIO** choisit la sortie. **CASQUE** place chaque organisme autour de
   votre tête : au casque seulement.
4. Le microphone (la **SALLE**) reste éteint jusqu'à ce que vous l'allumiez.

### Problèmes et solutions

**Windows : « Windows a protégé votre ordinateur ».** Cliquez sur
**« Informations complémentaires »**, puis **« Exécuter quand même »**. Si un
antivirus met `Aquorbium.exe` en quarantaine, restaurez-le et autorisez-le :
le programme est construit par l'intégration continue publique de ce dépôt, à
partir de ce code.

**macOS : « endommagée », « impossible de vérifier le développeur » ou
« Apple n'a pas pu confirmer ».** Suivez l'étape 4 de [macOS](#macos-2) :
**Ouvrir quand même** dans Confidentialité et sécurité, ou la commande `xattr`
dans le Terminal.

**Linux : la fenêtre ne s'ouvre pas.** Installez le `.deb` avec `apt`, qui
installe aussi les dépendances. Avec l'AppImage ou le `.tar.gz`, le système a
besoin d'ALSA, FreeType, Fontconfig, X11 et GLib, présents sur tout bureau.
Si l'**AppImage** ne s'ouvre pas, installez `libfuse2` (Ubuntu 22.04+ : `sudo
apt install libfuse2`), ou lancez-la avec
`./aquorbium-<version>-linux-x86_64.AppImage --appimage-extract-and-run`.

**Pas de son.** Cliquez sur **AUDIO** et choisissez la sortie. Vérifiez aussi
le **NIVEAU DE SORTIE** dans l'en-tête, et que **MUET** et **PAUSE** sont
éteints.

**Clics ou coupures.** Dans **AUDIO**, choisissez un tampon de **512**
échantillons. Sous Linux, le fil audio demande la priorité temps réel ; la
ligne d'état affiche **RT 70** quand il l'obtient et **RT non** sinon. Pour
l'autoriser, ajoutez votre utilisateur au groupe `audio` ou `pipewire` (selon
la distribution) et redémarrez l'ordinateur. Des programmes lourds à côté de
l'Aquorbium (compilateurs, vidéo) causent aussi des coupures.

**Autre chose :** écrivez à **rasgo.instruments@gmail.com** avec votre système
et sa version, ce que vous avez fait et le message affiché. Vous pouvez aussi
ouvrir une [issue](https://github.com/lucioaraujo/aquorbium/issues).

### Configuration requise

| | |
|---|---|
| **Windows** | Windows 10 ou 11, 64 bits |
| **macOS** | 10.15 ou plus récent ; Intel et Apple Silicon (Universal 2) |
| **Linux** | x86-64 avec glibc 2.35 ou plus récente (distributions de 2022 et après) |
| **Processeur** | 64 bits, deux cœurs ou plus. **Mesuré** sur un Intel Core i5-6500 (2015, 4 cœurs), à 48 kHz et blocs de 512 échantillons : l'audio utilise environ 7 % du temps de chaque bloc (au plus 9 %) |
| **Mémoire** | environ 55 Mo en jeu (mesuré) |
| **Disque** | environ 15 Mo |
| **Audio** | toute sortie ; un microphone seulement pour la SALLE |

La valeur du processeur a été mesurée sur la machine de l'auteur. Des machines
plus faibles n'ont pas été testées.

### Compiler depuis le code

```sh
git clone https://github.com/lucioaraujo/aquorbium.git
git clone --branch 9.0.3 https://github.com/juce-framework/JUCE.git
cmake -S aquorbium -B aquorbium/build -DCMAKE_BUILD_TYPE=Release -DAQUORBIUM_JUCE_PATH=$PWD/JUCE
cmake --build aquorbium/build --parallel 2
(cd aquorbium/build && ctest -LE slow --output-on-failure)
```

Sous Linux, la compilation de JUCE demande les paquets de développement
d'ALSA, FreeType, Fontconfig, X11 et GTK. Le flux
`.github/workflows/package.yml` en donne la liste.

### État

La version 0.1.0 est la première version publique. Les paquets Windows et
macOS sont construits, testés et vérifiés par l'intégration continue : sans
dépendance au Visual C++ Redistributable, et un paquet Universal 2 scellé sur
macOS. Ils n'ont pas encore été ouverts par l'auteur sur de vrais Windows et
Mac.

### Licence

AGPL-3.0-or-later (`LICENSE-AGPL-3.0.txt`). Crédits et sources :
`CREDITS_AND_SOURCES.md`. Problèmes : **rasgo.instruments@gmail.com**.

## Español

### Instalar, paso a paso

Los paquetes de cada sistema están en la
[página de versiones](https://github.com/lucioaraujo/aquorbium/releases); el
más nuevo está arriba. Los avisos de seguridad de abajo **son esperables**. No
indican ningún problema y solo aparecen la primera vez.

#### Windows

1. Descargue `aquorbium-<versión>-win64.exe`.
2. Ábralo. Windows muestra un recuadro azul **«Windows protegió su PC»**: haga
   clic en **«Más información»** y luego en **«Ejecutar de todas formas»**.
3. Si Windows pide permiso de administrador, haga clic en **«Sí»**.
4. Haga clic en **«Siguiente»** hasta el final. La última página ya trae
   **«Ejecutar Aquorbium»** marcado. Aquorbium queda en el **menú Inicio** y
   en el escritorio.

**Sin instalar:** descomprima `aquorbium-<versión>-win64.zip` y abra
`Aquorbium.exe`.

#### macOS

1. Descargue `aquorbium-<versión>-Darwin.dmg`. Funciona en Mac Intel y Apple
   Silicon.
2. Ábralo y **arrastre Aquorbium a Aplicaciones**.
3. Ábralo. La primera vez, macOS **bloquea** la app con un mensaje que dice
   que Apple no pudo confirmar que está libre de software malicioso. También
   puede aparecer «no se puede verificar el desarrollador» o «está dañada y no
   se puede abrir». En todos los casos:
   - **No es un defecto ni un virus**: la app está firmada ad hoc, sin un
     Developer ID de pago de Apple.
   - Haga clic en **«OK»** o **«Listo»**, **no** en «Trasladar a la papelera».
4. Permita la app, una vez, de una de estas formas:
   - **En Ajustes del Sistema:** vaya a **Privacidad y seguridad**, baje hasta la
     sección de seguridad, haga clic en **«Abrir igualmente»**, confirme con su
     contraseña o Touch ID y haga clic en **«Abrir»**. En macOS 14 o anterior:
     clic derecho en la app en el Finder → **Abrir → Abrir**.
   - **En el Terminal**, si el botón no aparece:

     ```sh
     xattr -dr com.apple.quarantine "/Applications/Aquorbium.app"
     ```

     Si no se imprime nada, funcionó. El comando solo quita la «marca de
     cuarentena» que macOS pone en todo archivo descargado.

#### Linux

- **Ubuntu 22.04+, Debian 12+, Mint 21+:** haga doble clic en el `.deb`, o
  ejecute `sudo apt install ./aquorbium-<versión>-Linux.deb`.
- **Fedora, Arch, openSUSE y otras distribuciones:**
  - **AppImage:** hágalo ejecutable (Propiedades → permitir ejecutar, o
    `chmod +x`) y ábralo.
  - **`.tar.gz`:** descomprímalo y ejecute `./install.sh`. Instala solo para
    su usuario, sin root; `./install.sh --remove` lo deshace.

### La primera vez que lo abre

1. **Cada apertura es una pieza nueva.** La semilla de la sesión propone un
   bioma - el agua, la luz, la población, los cuerpos, qué módulos despiertan
   - y un mundo de tonalidades, armonía y forma. Con **VARÍA** encendido
   (franja de abajo), los controles también se mueven despacio solos. Toque a
   partir de ahí.
2. El botón **TUTORIAL** explica el instrumento paso a paso, y **LEARN**
   explica cualquier control que señale, en inglés, portugués, francés o
   español (el botón de idioma: EN/PT/FR/ES).
3. **AUDIO** elige la salida. **AURICULARES** coloca cada organismo alrededor
   de su cabeza: úselo solo con auriculares.
4. El micrófono (la **SALA**) queda apagado hasta que lo encienda.

### Problemas y soluciones

**Windows: «Windows protegió su PC».** Haga clic en **«Más información»** y en
**«Ejecutar de todas formas»**. Si un antivirus pone `Aquorbium.exe` en
cuarentena, restáurelo y permítalo: el programa lo construye la integración
continua pública de este repositorio, a partir de este código.

**macOS: «dañada», «no se puede verificar el desarrollador» o «Apple no pudo
confirmar».** Siga el paso 4 de [macOS](#macos-3): **Abrir igualmente** en
Privacidad y seguridad, o el comando `xattr` en el Terminal.

**Linux: la ventana no se abre.** Instale el `.deb` con `apt`, que también
instala las dependencias. Con el AppImage o el `.tar.gz`, el sistema necesita
ALSA, FreeType, Fontconfig, X11 y GLib, que todo escritorio tiene. Si el
**AppImage** no se abre, instale `libfuse2` (Ubuntu 22.04+: `sudo apt install
libfuse2`), o ábralo con
`./aquorbium-<versión>-linux-x86_64.AppImage --appimage-extract-and-run`.

**Sin sonido.** Haga clic en **AUDIO** y elija la salida. Revise también el
**NIVEL DE SALIDA** en la cabecera, y que **SILENCIAR** y **PAUSA** estén
apagados.

**Chasquidos o cortes.** En **AUDIO**, elija un búfer de **512** muestras. En
Linux el hilo de audio pide prioridad de tiempo real; la línea de estado
muestra **RT 70** cuando la obtiene y **RT no** cuando no. Para permitirla,
añada su usuario al grupo `audio` o `pipewire` (según la distribución) y
reinicie el ordenador. Programas pesados al lado del Aquorbium (compiladores,
vídeo) también causan cortes.

**Cualquier otra cosa:** escriba a **rasgo.instruments@gmail.com** con su
sistema y versión, lo que hizo y el mensaje que apareció. También puede
abrir una [issue](https://github.com/lucioaraujo/aquorbium/issues).

### Requisitos

| | |
|---|---|
| **Windows** | Windows 10 u 11, 64 bits |
| **macOS** | 10.15 o posterior; Intel y Apple Silicon (Universal 2) |
| **Linux** | x86-64 con glibc 2.35 o más nueva (distribuciones de 2022 en adelante) |
| **Procesador** | 64 bits, dos núcleos o más. **Medido** en un Intel Core i5-6500 (2015, 4 núcleos), a 48 kHz y bloques de 512 muestras: el audio usa cerca del 7 % del tiempo de cada bloque (como máximo 9 %) |
| **Memoria** | cerca de 55 MB sonando (medido) |
| **Disco** | cerca de 15 MB |
| **Audio** | cualquier salida; micrófono solo para la SALA |

El valor del procesador se midió en la máquina del autor. Máquinas más débiles
no se han probado.

### Compilar desde el código

```sh
git clone https://github.com/lucioaraujo/aquorbium.git
git clone --branch 9.0.3 https://github.com/juce-framework/JUCE.git
cmake -S aquorbium -B aquorbium/build -DCMAKE_BUILD_TYPE=Release -DAQUORBIUM_JUCE_PATH=$PWD/JUCE
cmake --build aquorbium/build --parallel 2
(cd aquorbium/build && ctest -LE slow --output-on-failure)
```

En Linux, la compilación de JUCE necesita los paquetes de desarrollo de ALSA,
FreeType, Fontconfig, X11 y GTK. El flujo `.github/workflows/package.yml`
trae la lista.

### Estado

La versión 0.1.0 es la primera versión pública. Los paquetes de Windows y
macOS los construye, prueba y verifica la integración continua: sin
dependencia del Visual C++ Redistributable, y un paquete Universal 2 sellado
en macOS. El autor aún no los ha abierto en Windows y Mac reales.

### Licencia

AGPL-3.0-or-later (`LICENSE-AGPL-3.0.txt`). Créditos y fuentes:
`CREDITS_AND_SOURCES.md`. Problemas: **rasgo.instruments@gmail.com**.
