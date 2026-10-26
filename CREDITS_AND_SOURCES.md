# Aquorbium — créditos, fontes e licenças

Documento voltado à publicação: o que alguém precisa ler antes de usar,
redistribuir ou derivar do Aquorbium. Segue o formato do Trioio, do Rasgo
Modular e do Antitotem.

## Autoria e licença

**Lúcio de Araújo**, 2026. Código sob **GNU AGPL-3.0-or-later**: ver
[`LICENSE-AGPL-3.0.txt`](LICENSE-AGPL-3.0.txt). Cada arquivo-fonte começa com
o cabeçalho SPDX, conferido pelo teste `aquorbium_license_headers`.

A aplicação usa o **JUCE** sob **AGPLv3**, a opção livre do JUCE, sem licença
comercial. O JUCE não é versionado neste repositório: é um checkout apontado
por `-DAQUORBIUM_JUCE_PATH` (na CI, fixado numa tag).

O núcleo (`src/core/`) não depende do JUCE nem de nenhuma outra biblioteca.

## Código de terceiros incorporado

**Nenhum.** Não há código, imagem, áudio, fonte tipográfica ou texto de
terceiros neste repositório.

Quando algum componente externo entrar, ele ganha uma linha aqui com autor,
origem, versão, licença e local de uso, e a mesma triagem por cor dos
projetos irmãos:

- **verde:** domínio público, licença livre compatível ou código do autor;
- **amarelo:** copyleft compatível, com crédito e decisão registrados;
- **vermelho:** proprietário ou não comercial, que serve só de referência
  conceitual.

**Ferramenta de terceiros no caminho do release:** o
[linuxdeploy](https://github.com/linuxdeploy/linuxdeploy) (MIT), só na CI,
para montar o AppImage, fixado por versão e SHA-256 em
`packaging/linux/empacotar.sh`. Não fica no repositório.

## Código próprio do RASGO (verde: o mesmo autor)

Cada instrumento RASGO tem código próprio; o reaproveitamento é uma cópia
fixada, com proveniência, adaptada aos conceitos do instrumento.

| O quê | De onde | Onde aqui |
|---|---|---|
| detector de true peak (ITU-R BS.1770-4, anexo 2) | Navalha 2, `b5343c4` (GPL-3.0-or-later, o mesmo autor) | `src/core/TruePeakDetector.*` |
| os núcleos de síntese: oscilador PolyBLEP, filtro de estado variável TPT, operadores FM, wavetable, aditiva, formantes | Rasgo Modular `4ce898f` (AGPLv3, o mesmo autor), com checksums no código | `src/core/SynthVoice.h`, `src/core/SynthSources.*` |
| os scripts Linux e o fluxo da CI | Trioio `383c009` (vindos do Antitotem `6c88ab4`) | `packaging/`, `.github/workflows/` |

**Conceitos reescritos, sem código copiado:**

- a semente como hipótese e o Motion Engine v3, a "mão caótica" do VARIA
  (Rasgo Modular, `dossies/ESTUDO_seed_composicao_generativa.md`): em
  `src/core/OpeningSeed.h` e `src/core/ControlMotion.h`;
- as linguagens harmônicas da máquina de harmonia do Trioio
  (`docs/MAQUINA_DE_HARMONIA.md`): em `src/core/HarmonyPlan.h`.

## Fontes teóricas e técnicas

As equações e os modelos vêm da literatura pública; cada módulo cita as suas
no código e no seu documento de desenho (`*_DESIGN.md`). As principais:

- **medição de loudness e true peak:** ITU-R BS.1770-4; EBU R 128; EBU Tech
  3341 e 3342;
- **escuta binaural:** C. P. Brown e R. O. Duda, "A structural model for
  binaural sound synthesis" (1998); R. S. Woodworth (o atraso entre as
  orelhas);
- **Ambisonics:** M. Gerzon; o formato AmbiX;
- **ecologia:** as equações de Lotka-Volterra; R. Murray Schafer, *The
  Tuning of the World* (1977); B. Krause, a ecologia acústica e a hipótese do
  nicho;
- **síntese:** o PolyBLEP (V. Välimäki e A. Huovilainen; M. Finke); o filtro
  de estado variável TPT (A. Simper); Karplus-Strong (a corda); os osciladores de Duffing e de Lorenz
  (o caos); o atrator de Thomas (R. Thomas, 1999), na mão do VARIA;
- **harmonia e forma:** H. Riemann (as funções); W. Piston, *Harmony* (1941);
  R. Cohn, *Audacious Euphony* (2012); D. Milhaud (a politonalidade);
  A. Schoenberg, *Fundamentals of Musical Composition* (1967); G. Russell, o
  conceito de escala-acorde.

## Identidade visual

O símbolo do sonar, o lettering e os ícones são do autor (criados com o
Codex e aprovados em 2026-10-25): [`assets/identity/IDENTIDADE.md`](assets/identity/IDENTIDADE.md).
O lettering é desenhado em traços, sem fonte tipográfica.

## Contato

**rasgo.instruments@gmail.com** · https://github.com/lucioaraujo/aquorbium
