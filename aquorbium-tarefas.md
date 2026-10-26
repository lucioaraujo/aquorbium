# Rasgo: Aquorbium — Lista de Tarefas

Atualização de organização: 2026-10-23. Esta lista reúne 337 caixas: 287
concluídas e 50 abertas, contando itens e subitens. Elas não representam 335
motores DSP. A arquitetura tem:
- cinco núcleos técnicos operantes;
- um Master criativo ativo;
- um estágio técnico de saída separado;
- o Ecossistema, com cinco organismos sonoros;
- o dispositivo de síntese, uma voz por organismo.

O banco final conserva hipóteses de implementação extraídas da pesquisa de
módulos externos.

Nesta lista, uma caixa marcada conclui somente a frase da tarefa. Em
especial, "prototipar" confirma o core, mas não confirma que o módulo está
disponível no aplicativo. O estado de cada família segue estas etapas:
pesquisa → protótipo de core → integração de áudio → controles/persistência →
validação técnica → aprovação musical. Só as etapas efetivamente realizadas
podem ser anunciadas.

## Publicação v0.1.0 — 2026-10-26

O autor decidiu: **repositório próprio e público** (`lucioaraujo/aquorbium`,
como o Trioio), a primeira versão pública é a **v0.1.0** (o padrão dos outros
instrumentos) e **nenhuma assinatura paga** ("não vou pagar nada"): o macOS
sai selado ad-hoc e Universal, o Windows sem assinatura, o Linux com
checksums, e o guia de instalação explica os avisos.

**Escuta que trava a v0.1.0** (uma sessão de 30–40 min):
- [ ] abrir o app 3 ou 4 vezes, 3–5 min cada: cada abertura soa diferente?
      alguma é desagradável ou vazia demais? (semente de abertura);
- [ ] numa delas, deixar o VARIA uns 5 min: o movimento é musical? demais ou
      de menos? (o slider ao lado ajusta);
- [ ] uma sessão de ~8 min sem mexer: a harmonia, o clímax e o repouso, a
      pergunta e resposta e o motivo que volta fazem sentido?
- [ ] o equilíbrio da mistura: o nível geral e a SUSTENTAÇÃO como padrão.

As outras escutas da lista A seguem depois da v0.1.0, como melhoria.

**Técnico, antes da tag** (padrão `RASGO_DOCUMENTATION/PADRAO_DISTRIBUICAO_MULTIPLATAFORMA.md`):
- [ ] o repositório aninhado com histórico limpo; o monorepo ignora a pasta;
- [x] instalação e CPack dos três sistemas (`1c4b7fb`); CRT estática no Windows e a
      conferência por `dumpbin`; Universal 2 e selo ad-hoc no macOS; AppImage
      e `.tar.gz` no Linux, com testes em containers;
- [x] `std::from_chars` de ponto flutuante (3 usos) trocado por
      `core/DecimalText.h` (macOS 10.15);
- [x] o fluxo da CI (só nas tags `v*`, JUCE 9.0.3), escrito, **ainda não
      rodado**: roda no primeiro push de uma tag (~160 min cobrados);
- [x] `CREDITS_AND_SOURCES.md` (com o JUCE e o linuxdeploy), `INSTALL.md`
      (4 línguas), `CHANGELOG.md`, o README público;
- [ ] a pendência do true peak normativo (README, estágio de saída).

**CI (2026-10-26/27, repositório público: gratuita):** Linux e macOS
passam (build, 83 testes, pacotes, Universal 2 selado). O Windows passou a
compilar (`EngineSoakTests` sem POSIX) e os testes que caíam por estouro
de pilha (o `BiomaEngine` tem ~870 KB) passaram com `/STACK` de 16 MB nos
executáveis do MSVC; resta o `aquorbium_ambisonic_tests`, que termina com
0xc0000409 na gravação do arquivo AmbiX (marcas finas em `9c232ea` para
achar a linha). Dois testes frágeis no macOS arm64 (FMA) foram tornados
robustos: o período pelas passagens do equilíbrio, o rubato em cinco
temporadas.

**Validado localmente (2026-10-26, `1c4b7fb`):** ctest 83/83; auditoria de
layout nos quatro tamanhos e o editor de síntese em FR e ES; o `.deb`, o
`.tar.gz` e o AppImage gerados e abertos (janela em 20 s, ~55 MB de
memória); o `install.sh` instala e remove. O `.deb` local pede glibc 2.38
porque foi feito nesta máquina; o da CI, no Ubuntu 22.04, pede 2.35.

**Site próprio** (o autor, 2026-10-26: "verificar também as páginas web;
páginas do github (no mesmo padrão dos outros instrumentos)"; "na página
web própria pode inovar"):
- [x] `website/` no formato do Trioio (gerador, `seo.py`, faixa do portal,
      fontes livres com a licença, contato sem `mailto`, 4 línguas) e o
      `deploy-site.yml`; o próprio desta página: o **sonar vivo** e a
      **semente desta visita** (`website/README.md`);
- [x] conferido no Chrome sem janela, no desktop (1440) e no celular (390);
- [x] o GitHub Pages ligado (2026-10-26) e o site no ar em
      <https://lucioaraujo.github.io/aquorbium/>; até o lançamento,
      `noindex` e "em preparação";
- [x] (2026-10-27, pedidos do autor) o menu Sobre | Como tocar | Tutorial |
      A ideia | Instalação | Baixar, 20 páginas em quatro línguas:
      - **Como tocar:** uma seção por área da tela, com a captura do app
        tocando em cada língua (`website/assets/guia/`), o texto do LEARN e
        os 95 controles explicados, lidos do próprio app;
      - **Tutorial:** os 18 capítulos, lidos do `Tutorial.h`;
      - **A ideia:** a origem, os conceitos, a composição generativa, as
        referências (o SOBRE do app) e a licença;
      - **Instalação:** o `INSTALL.md`, com os requisitos mínimos;
      - os cinco seres explicados (natureza, som, o que fazem, o que
        explorar), conferidos contra o `Ecosystem.cpp`;
- [ ] as capturas do "Como tocar" refeitas quando a interface mudar
      (`website/README.md`, como gerar);
- [ ] no lançamento: `PUBLICADO = True` no gerador, e o card no portal RASGO
      (`RASGO_WEBSITE/`; o envio do portal é do autor).

**Idiomas** (o autor, 2026-10-26: "há a questão dos idiomas também"):
- [x] a interface completa nas quatro línguas: 625 textos, nenhum vazio;
      só 8 iguais ao inglês, todos de propósito (cognatos e notação técnica:
      "STRUCTURE ACTIVE", "chaos (Duffing)", os diagramas do FM);
- [x] uma revisão do FR e do ES pelo Claude (2026-10-26), em 626 textos:
      cerca de 30 correções, nenhuma de texto faltando:
      - conflitos de nome no ES: GLUE "COLA" × o cartão COLA (agora
        PEGAMENTO), SUPPORT e SUSTAIN ambos "SOSTÉN" (SUPPORT agora APOYO),
        TILT e BRIGHTNESS ambos "BRILLO" (TILT agora PENDIENTE);
      - termos: FR "RELÂCHE" → "RELÂCHEM." (como no limitador); ES "FOCO
        ESCASO" → "FOCO DISPERSO"; "el vidrio se rompe" → "se agrieta"; "la
        musique cadence" / "la música cadencia" → "conclut sur une cadence" /
        "cierra en una cadencia"; "pasa-bajos" → "paso bajo";
      - o ES sempre de "usted", como o resto do app (REC, LUPA, VARIA);
      - o status "RT não" passou às quatro línguas;
- [x] a revisão do PT (~27 correções: "acasalamento" → "corte", "a música
      cadencia" → "chega a uma cadência", STORE "GRAVAR" → "SALVAR" para não
      confundir com o REC, TILT "BRILHO" → "INCLINAÇÃO", "reflexos" →
      "reflexões", "pastagem" → "pastejo") e do EN (gramática, "CENTER" →
      "CENTRE" no inglês britânico do resto) (2026-10-26);
- [x] o tutorial ganhou o capítulo 2, "Cada abertura, uma peça nova" (o
      mundo da sessão, a harmonia, o bioma, o VARIA), nas quatro línguas; a
      Lição 1 não supõe mais SÍNTESE em 0; o status nomeia os 10 modos na
      língua da interface; o sumário cabe com 18 capítulos em 960x600;
- [ ] a revisão de um falante nativo de FR e de ES continua desejável;
- [x] o `INSTALL.md` nas quatro línguas (EN, PT, FR, ES; o ES de "usted");
- [x] o README público curto nas quatro línguas, com uma imagem do app
      tocando (`74ae4ef`); o diário de desenvolvimento, que era o README,
      está em `docs/DESENVOLVIMENTO.md` (autorizado pelo autor em
      2026-10-26), com o guia rápido de "Como tocar";
- [x] o status nomeia o mundo da sessão na língua da interface ("B dorian ·
      modal · orbit 16"); o texto do REC guarda os nomes fixos em inglês, para
      a proveniência.
- [x] o `testar-arranque.sh` encerra o grupo de processos inteiro: o AppImage
      deixava o app tocando depois do teste (o defeito vem do Antitotem e
      está também no Trioio: `packaging/PROVENIENCIA.md`).

**Repositório** (2026-10-26): este diretório é agora o repositório próprio
(`19140e2`, histórico limpo; o anterior fica no `rasgo-instruments` até
`02292a1`; cópia de preservação em
`RASGO_ARQUIVO/.local-preservation-aquorbium-20261026/`). O `biome.odt`
(brainstorm com texto gerado por IA) fica só local. Os caminhos do disco do
autor saíram dos documentos. Falta criar `lucioaraujo/aquorbium` no GitHub
(o autor: a criação foi barrada pela permissão do modo automático) e o
push, quando o autor pedir.

## Semente de abertura, VARIA e identidade — 2026-10-24/26

- [x] **Semente de abertura** (o autor: "a cada vez que o instrumento for
      aberto soe diferente"; "levando em consideração os conceitos e
      particularidades de cada instrumento"): `src/core/OpeningSeed.h`. A
      semente sorteia um bioma (temperatura, luz, turbidez, correnteza,
      população, predação, habitat) e dele as portas, os controles, os
      corpos (afinidade por espécie), a ecologia e a PROPOSTA de síntese. O
      estágio de saída é do músico. Conceito: a semente como hipótese do
      Rasgo Modular (`dossies/ESTUDO_seed_composicao_generativa.md` §1.1),
      reescrito.
- [x] **VARIA** (o autor: "como é feito no rasgo modular, com um botão de
      varia"): `src/core/ControlMotion.h`, o atrator de Thomas do Motion
      Engine v3 do Modular (`apps/panel/MotionField.hpp` @4ce898f, conceito
      reescrito), com a camada da forma: a tensão da sessão puxa o clímax.
- [x] **Identidade aprovada** no cabeçalho: `aquorbium-symbol.svg` e
      `aquorbium-wordmark.svg`; a marca só ocupa o espaço que os botões
      deixam (no 960 px ela some em vez de ficar sob eles).
- [x] **Primeira abertura** sem preferências de áudio: pedia o ALSA padrão
      a 8 kHz e bloco 16 (4 XRUN em 90 s); agora 48 kHz, 0 XRUN em 60 s
      (o bloco de 512 não foi confirmado pelo `pw-top` nessa rota).
- **Validação:** ctest 82/82; `aquorbium_opening_seed_tests` (400 sementes,
  400 biomas, 102 constelações de módulos; luz 0,77 ao meio-dia, 0,22 à
  meia-noite; 10 min de VARIA); auditoria de layout nos quatro tamanhos;
  três aberturas reais em silêncio: biomas diferentes, RT 70, o nível RMS
  de −32 a −35 dBFS e pico até −10 dBFS, contra −33 a −42 e −19 a −24
  sem a semente; na tela, o VARIA moveu DENSIDADE 0,56 → 0,61 em 60 s.
- **Limites:** a escuta do autor; captura e auditoria seguem com a semente
  42 e sem a semente de abertura (de propósito).

## Pendências abertas — consolidado de 2026-10-22

Um mapa das 50 caixas abertas, agrupadas pelo que cada uma exige. O detalhe
fica no item original, na seção indicada entre parênteses.

### A. Dependem do autor: escuta, uso e decisões

- **Usar o dispositivo de síntese** (EDITAR, PROPOSTA, cenas) e dizer o que
  falta ou incomoda no fluxo. Não é aprovar timbres: é a usabilidade da
  ferramenta (`SYNTH_DEVICE_DESIGN.md`). Em 2026-10-23 o autor abriu o
  editor e pediu para aprender a tocar; o TUTORIAL ganhou cinco lições
  (capítulos 9 a 13). Se continuar cheio demais, há o modo SIMPLES a fazer.
- **Testar a SALA com o microfone real.** Desde 2026-10-22 a sala também é
  ouvida com o aquário em silêncio (roteiro, item 12).
- **Integrar a identidade visual aprovada** ao cabeçalho JUCE: usar o símbolo
  e o lettering vetorial em tamanho legível, sem alterar o layout do painel;
  conferir também o ícone da janela e os pacotes (Logo).
- **Revisar as traduções**, sobretudo FR e ES (Tutorial, Learn, Sobre e
  idiomas).
- **Modo noturno "rig for red"**, se o autor quiser (Visual de painel de
  submarino).
- **Escutas e aprovações musicais pendentes:**
  - os corpos novos (Corpos dos organismos);
  - o layout em uso (Layout com todos os controles visíveis);
  - MÚSICA > 0 e a aprovação musical (Música ecológica);
  - a variação composicional, com o tom e a memória do Espaço (Mais
    variação nas sonoridades);
  - o diretor de variação: ritmo, melodia, harmonia, timbre, eventos, forma
    (Implementação inicial em 2026-10-02);
  - a política de dia e estação na progressão (Fase 3);
  - o caráter ecológico da Cauda (Fase 6);
  - os pares A/B, o retorno inicial 0,36 e os seis controles do Espaço
    (Integrações e validações ainda necessárias);
  - a consequência sonora de `AquariumIntegrity::shattered` (idem);
  - o BiomaBass contra um Reese e um FM convencionais (Fase 2b);
  - o `BiomaRelabi` como decisão do cardume (Fase 4).
- **Mistura: a presença da sustentação** é agora um controle do músico:
  SUSTENTAÇÃO, no cartão TIMBRE, desde 2026-10-23.
  - GRAIN, MODAL e STRING ficam 22 a 26 LU abaixo do primeiro plano em
    0 dB.
  - Para aproximá-los, a faixa sugerida fica entre +10 e +15 dB.
  - Falta o autor achar o próprio valor e, se quiser, mudar o padrão.
- **Limiares do MASTER CRIATIVO:** no nível usual a dinâmica e a cola não
  comprimem nada (0,000 dB), e a ORDEM DO EQ só muda o som com o sinal alto.
  Fica com o autor decidir se os limiares devem acompanhar o nível do
  Aquorbium (`OUTPUT_STAGE_DESIGN.md`, achado de 2026-10-23).
- **Decisões de conceito:**
  - primeiro plano sonoro × estrutura que só sustenta (Fase 11);
  - os conceitos complementares, ar/água e densidades (Fase 11), e ar/água
    por organismo (Integrações);
  - se a Lupa (`heldByObserver`) pode ser automatizada pelo Cérebro, e a
    calibração da Lupa contra casos reais (Lupa).

### B. Técnicas, verificáveis sem escuta

- **Sentido musical (pedido do autor, 2026-10-24).** Feito
  (`COMPOSITIONAL_VARIATION_DESIGN.md`):
  - a gênese da sessão;
  - os encadeamentos harmônicos em cinco linguagens, com modulação e
    politonalismo;
  - o arco de tensão;
  - a respiração e os retardos;
  - a pergunta e resposta;
  - o motivo que volta;
  - a MÚSICA ecológica dentro da harmonia.

  Próximo: o banco e a máquina de escalas do Trioio no lugar dos 10 modos;
  e a escuta do autor.
- **Interface (2026-10-24).** Feitos os cartões MEDIDORES, LUPA,
  CÉREBRO · SALA e o MASTER CRIATIVO por dentro, mais o fader de volume no
  cabeçalho. Falta o quadro compacto da SÍNTESE: sem espaço livre, e o
  EDITAR já cobre.

- **Som e espaço:**
  - Ambisonics: saída multicanal ao vivo (o arquivo AmbiX do REC foi
    feito em 2026-10-23; o ao vivo depende de um sistema de caixas);
  - a HRTF medida e o ouvinte na posição de um `Observer` (Rodada
    SchneidersLaden, páginas 46–65).
- **Interface:** cartões técnicos como superfícies de controle e a camada
  "Shift" (Fase 9).

### C. Dependem de hardware ou de outro ambiente

- deadline e XRUN no dispositivo real, e soak test do app (Fase 10);
- ThreadSanitizer num ambiente em que o runtime funcione (Fase 10);
- DPI de 125 % a 200 % e dois monitores (Fase 9).

### D. Plugin e DAW (quando houver o plugin)

- `AudioProcessorValueTreeState` com todos os parâmetros, e automação
  testada na DAW (Fase 9);
- sincronia de tempo pelo `AudioPlayHead` (Fase 3);
- declarar a latência ao host e um perfil de baixa latência (Fase 7b);
- o adaptador opcional `juce::Synthesiser` com `findVoiceToSteal`
  determinístico (Fase 1).

### E. Exploração e pesquisa

- Westlicht PER|FORMER como referência (Fase 3).
- KOMA Sensei e Bela Gliss como entradas de `Environment`; um pedal físico
  por CV (Rodadas SchneidersLaden).
- **Microfone e mundo de fora:**
  - o microfone estressando `crackStress`;
  - a brecha externa da `AquariumIntegrity` em três controles;
  - o roteamento semântico no `AccessPort`;
  - o exciter por gesto de dois pontos no vidro;
  - o programa de regras editável ao vivo.
  (Rodada EMW e Rodadas SchneidersLaden.)

### F. Acervo (regras permanentes, não etapas)

- Atualizar `ARQ-AQU-001` numa release, entrega ou publicação; registrar no
  histórico global a cada marco; manter ficha, tutorial, imagens e
  demonstrações sincronizados (Continuidade de acervo e Fase 0).
- `biome.odt` é pesquisa bruta: não copiar o hard clip dele como limitador
  (Fase 0).

## Verificação dos conceitos e potenciais não explorados — 2026-10-20

`CONCEITOS_VERIFICACAO_E_APROFUNDAMENTO.md`: cada conceito do SOBRE e do
código conferido na fonte (fiel, simplificado ou errado), aprofundado e
transformado em proposta de módulo; e os projetos livres com potencial
(Atlas, GitHub, universidades).

- [x] Correções aplicadas:
      - as vogais [ɔ]/[ɛ];
      - a membrana pelos zeros de Jₙ;
      - a nota sobre as razões do vidro;
      - o DOI de Aguilar de Soto;
      - Noad et al. no SOBRE (quatro línguas).
- [x] O roteiro proposto (§4), completo em 2026-10-21:
      1. ~~`ChorusPhase`~~ feito em 2026-10-20: o nicho temporal como coro
         de osciladores acoplados pela escuta; cardume em fase, fuga e
         território em antifase, a caça como perseguição (medido);
      2. ~~`SongForm`~~ feito em 2026-10-20: unidade, frase, tema e canto
         (Payne & McVay); a deriva lenta pela MEMÓRIA; frase inteira ou a
         cabeça do tema; repetição de motivos de +46% a +105% (medido);
      3. ~~as correções de endereçamento, Lombard em dB e o medo que
         sincroniza~~ feitas em 2026-10-20 (medidas no documento §2);
      4. ~~`SpectralConsonance`~~ feito em 2026-10-20: consonância pelos
         parciais e pelo registro (Sethares); a escala nasce dos corpos
         inarmônicos (medido: com membrana, sexta menor ×2,2, quinta −46%);
      5. ~~a física da água~~ feita em 2026-10-20: a absorção de Ainslie &
         McColm (temperatura, sal, pH), a velocidade do som de Mackenzie e o
         seletor MAR / ÁGUA DOCE;
      6. ~~`ThermalCloud`~~ feito em 2026-10-20: glissandos com velocidades
         de Maxwell–Boltzmann pela temperatura da água (Xenakis); medido;
      7. ~~`PopulationCycles`~~ feito em 2026-10-21: ciclos de Lotka–Volterra
         de 2 a 5 min. As presas enchem o nicho (1,3–1,4× por estado da
         água), os predadores pesam os caçadores, e as capturas alimentam o
         modelo. As barras aparecem no painel;
      8. ~~`LifeCycle`~~ feito em 2026-10-21: energia como alimento (as presas
         do Lotka–Volterra, a captura), a fome que poupa, a idade que
         escurece, a morte que leva o canto ao recife e o nascimento com os
         cantos dos pais recombinados; 8 a 9 gerações de voz em 15 min;
      - feito em 2026-10-21, fora da lista: a semente nova a cada abertura
        (antes era sempre 42) e o REC medido. Depois: o LRA (EBU 3342), o
        PCM de 24 bits com TPDF e dois capítulos novos no tutorial (vida e
        lugar; canal, habitat, sala e REC), com as referências no Sobre;
      9. ~~`SoundChannel`~~ feito em 2026-10-21: o canal SOFAR de Munk, com
         o eixo pela temperatura (polar sob o gelo); sombra através dele
         (ouvido de 42% para 12%), espalhamento cilíndrico ao longo; faixa
         SOFAR na esfera;
      10. ~~soundmark e hi-fi/lo-fi~~ feitos em 2026-10-21: a marca sonora do
          lugar volta nas cadências e mudanças de estado; o nicho lotado
          perde definição (notas 2× mais longas, acentos achatados). A
          curva foi reajustada depois de medida no motor completo (antes,
          nunca se ligava); o centroide mudou só −2%: o lo-fi está na
          articulação. A marca escolhida pelo músico (MARCA) também está
          feita: guardada entre sessões e desenhada no painel;
      - `CodaExchange` (§2.6), além do roteiro, feito em 2026-10-21: o
        alfabeto de codas (6 ritmos × 3 andamentos), os clãs, o rubato que
        converge turno a turno e o ornamento que marca a mudança de assunto;
        medido no motor completo;
      - ARCO e PALHETA, corpos físicos novos, feitos em 2026-10-21: corda
        friccionada com atrito stick-slip e tubo de palheta com harmônicos
        ímpares; afinados, nivelados e com escutas em
        AUDIOS_RASGO/Aquorbium/escutas/2026-10-21-arco-palheta/; a v2
        (corpo, vibrato, breu, sopro) foi julgada pior pelo autor ("tudo
        rachado") e revertida para a v1. Próximo: uma mudança de cada vez,
        com escuta solo;
      - `BubbleField`, a água borbulhando, feito em 2026-10-21: Minnaert,
        a distribuição de Deane & Stokes e as bolhas acopladas; ~240/s na
        fervura, 6 a 10 dB sob os organismos; escuta em
        AUDIOS_RASGO/Aquorbium/escutas/2026-10-21-fervura/;
      - CAIXAS / FONES, feito em 2026-10-21: o binaural por fonte (Brown &
        Duda) nos fones, com o estéreo intacto nas caixas; níveis iguais;
        A/B em AUDIOS_RASGO/Aquorbium/escutas/2026-10-21-fones/. As vozes
        de MIDI entraram no binaural em 2026-10-22 (`MidiBinauralTests`: a
        correlação cai de 0,89 para 0,57, com o mesmo nível; nas caixas, a
        saída é idêntica à anterior). Ambisonics: desde 2026-10-23 o REC
        grava, com `recording.ambisonics`, um `_ambix.wav` de 1ª ordem
        (ACN/SN3D, float), com cada organismo vindo da sua direção ao ouvinte
        e o resto como o par estéreo a ±30°. Validado pelo ffprobe (4 canais,
        f32, layout desconhecido, o certo para AmbiX). Pendente: saída
        multicanal ao vivo, que depende de um sistema de caixas;
      11. ~~`HabitatHealth`~~ feito em 2026-10-21: ADI e ACI do próprio som
          (FFT 4096, limiar relativo de 30 dB), a saúde do habitat alarga ou
          estreita o nicho; "habitat N%" no painel;
      12. ~~`RoomCoupling`~~ feito em 2026-10-21: o botão SALA (desligado por
          padrão), o acoplamento das caixas aprendido e descontado, o
          Lombard e a fuga de registro diante da sala. Pendente: o autor
          testar com o microfone real.

## Identidade visual — 2026-10-25

- [x] Proposta de logo (o autor: "crie uma logo para o aquorbium"):
      - os arquivos e a proveniência estão em `assets/identity/`
        (`IDENTIDADE.md`);
      - no app, o símbolo fica no cabeçalho e é o ícone da janela.
- [x] O autor aprovou a nova identidade: lockup horizontal transparente, sem
      degradê nem caixa, com sonar circular e lettering vetorial original.
      Arquivo final: `assets/identity/aquorbium-logo-sonar-texto-compacta.svg`;
      símbolo, wordmark, favicon e ícones em vários tamanhos também foram
      gerados. Não restaurar as variantes rejeitadas.
- [ ] Integrar o símbolo e o wordmark finais ao cabeçalho JUCE. O app ainda
      carrega o `aquorbium-logo.svg` anterior e desenha o título com a fonte da
      interface; adaptar a composição ao espaço do cabeçalho e validar os
      ícones/pacotes.
- [x] **Dispositivo de síntese** (SYNTH_DEVICE_DESIGN.md; o autor, 2026-10-21:
      ferramentas de síntese em vez de timbres prontos). Feitas: fase 1 (a
      voz), fase 2 (a matriz ecológica), fase 4 (o editor, SÍNTESE +
      EDITAR no cartão TIMBRE) e fase 3 (2026-10-22: FM, WAVETABLE,
      ADITIVA, PULSAR, CORDA, MATÉRIA do Rasgo Modular e os onze corpos
      como fontes) e fase 5 (2026-10-22: PROPOSTA e PROPOR TODOS, que
      escutam e nivelam o que propõem); os patches nas CENAS (banco V3,
      `.aquscene`) e a fase 6 (tutorial, dicas no editor, LEARN e
      Sobre), o FORMANTE como modo de filtro (vogais, TRATO) e o FM 4
      operadores (8 algoritmos). Completo em 2026-10-22. Seguem abertos,
      fora desta caixa: o uso real pelo autor e a revisão nativa FR/ES
      (ver "Pendências abertas").

## Visual de painel de submarino — 2026-10-20

O autor: "faça uma pesquisa sobre painel de controles de submarinos. traga
elementos visuais nos nossos controles baseados neles".

**Pesquisa (fontes):**
- o painel de aberturas do casco, a "árvore de Natal": lâmpada vermelha
  para aberto, verde para fechado; "green board" quer dizer seguro para
  mergulhar (USS Cod, *Diving Procedure*; Fleet Type Submarine, cap. 18 —
  maritime.org);
- a sala de controle do USS Pampanito: medidores de profundidade rasa e
  funda, inclinômetro de bolha "como um nível de carpinteiro", indicadores
  dos lemes de profundidade (maritime.org/tour/cr.php);
- o sonar PPI: anéis de alcance, rumos de 0 a 360°, varredura, tela de
  longa persistência e, no sonar, um círculo que se expande a cada ping
  (Wikipedia, *Plan position indicator*);
- a cascata de rumo × tempo (BTH): o mais novo no topo, rolando para baixo
  (FAS, ES310 *Basic Sonar System*; The War Zone, sonaristas);
- a iluminação "rig for red" (e, hoje, azul) para a visão noturna (HMCS
  Ojibwa Museum; The Tides of History), registrada como possibilidade.

**No app:**
- [x] Botões giratórios como mostradores de instrumento: aro metálico, fundo
      escuro, escala graduada, ponteiro com ponta luminosa e o arco na cor
      do cartão (`InstrumentLookAndFeel`).
- [x] Cabeçalhos dos cartões como placas gravadas com parafusos; o estado
      como lâmpada anunciadora (acesa quando o cartão atua, apagada em
      bypass); os botões como chaves de painel, com a borda acesa quando
      engatadas.
- [x] A esfera como sonar PPI: anéis, rumos 000–330, varredura com
      persistência e um ping que se expande a cada acontecimento.
- [x] No painel da música:
      - os estados da água como lâmpadas da árvore de Natal;
      - o calor latente como inclinômetro de bolha;
      - a tensão num manômetro com zona vermelha;
      - a cascata da teia (seis colunas, uma por tipo de relação; o brilho
        é o vínculo; o mais novo no topo).
- [x] Conferido em captura com a música ligada; suíte 51/51.
- [ ] Possível: modo noturno "rig for red" (paleta vermelha), se o autor
      quiser.

## Tutorial, Learn, Sobre e idiomas — 2026-10-20

O autor: "avance os tutoriais, sobre, learn, idiomas". O padrão RASGO
(Antitotem, Navalha 2, Rasgo Modular, Trioio) foi reescrito para o
Aquorbium, sem código copiado.

- [x] Quatro idiomas (EN/PT/FR/ES) em toda a interface (`src/app/UiLanguage.h`):
      - os 50 controles, com nome, dica e um texto de Learn;
      - os cartões, os estados, o painel da música (estados da água, modos,
        relações, atos), os corpos, as cenas e o diálogo de áudio;
      - na primeira abertura vale o idioma do sistema, e o botão EN/PT/FR/ES
        guarda a escolha (`ui.language`).
      Ficam em inglês as leituras técnicas do quadro (BIOENERGY, ENERGY,
      ROOT) e os nomes dos módulos do laboratório.
- [x] TUTORIAL (`src/app/Tutorial.h`): nove capítulos com DICA, sumário e
      anterior/próximo, nos quatro idiomas.
- [x] SOBRE: o propósito, os conceitos, as pesquisas citadas, a licença e
      as fontes.
- [x] LEARN (`src/app/LearnCatalog.h`): ligado, mostra embaixo do aquário a
      explicação do que está sob o ponteiro (controle, cartão, corpo,
      módulo ou botão), com uma sugestão para experimentar.
- [x] Conferido em capturas automatizadas, numa instância isolada: o Learn
      sobre ÁGUA, o tutorial em inglês, a troca para francês e o Sobre em
      francês; suíte 51/51.
- [x] A frase de crédito no rodapé (o autor: "falta a frase de rodapé com
      os créditos"), no padrão do Trioio, nos quatro idiomas: "© LÚCIO DE
      ARAÚJO · AQUORBIUM 2026 · LICENÇA AGPLv3+ · v0.0.1". Fica na margem
      inferior direita e também no fim do SOBRE.
- [x] Os botões do cabeçalho no padrão RASGO (o autor: "prefiro com o
      padrão, com idioma, tutorial, sobre no canto direito"). Ordem:
      SILENCIAR, PAUSA, ÁUDIO · LEARN · idioma, TUTORIAL, SOBRE. As
      larguras seguem o idioma; o estado do áudio fica entre o nome e os
      botões.
- [ ] Revisão das traduções pelo autor (sobretudo FR e ES).

## Escuta do autor — 2026-10-20

Depois da articulação, dos corpos, do timbre ecológico, do layout e da teia
no quadro, o autor escreveu: "escutei, melhorou". A aprovação é parcial; o
ajuste fino continua aberto:
- o nível de cada corpo;
- o escurecimento pela água (`liquidCutoffHz`);
- a densidade por estado.

## Corpos dos organismos e timbre ecológico — 2026-10-20

Concepção e validação em `TIMBRE_BODIES_DESIGN.md`.

- [x] Seis segundos corpos por organismo (`TimbreBody`): voz formante,
      matéria modal, sopro, corda, gota (Minnaert) e dobra; cada um com
      MATÉRIA e MISTURA.
- [x] TIMBRE ECOLÓGICO: a água, a teia, o vigor e a saúde movem a matéria
      e a presença.
- [x] Cartão 09 TIMBRE no app; 16 parâmetros persistidos, com migração das
      cenas.
- [x] Testes (`TimbreBodyTests`); suíte 51/51; render
      `validation-output/timbres-2026-10-20/`.
- [x] Nivelamento dos corpos contra o nativo, no motor (±0,1 dB; a corda
      fica a −5 dB).
- [x] Mais três corpos: membrana, vidro friccionado e coro.
- [x] A escolha passa a `timbreKind*` (índice / 15), com migração das cenas.
- [ ] Escuta do autor dos corpos novos.

## Layout com todos os controles visíveis, SILENCIAR e PAUSA — 2026-10-20

Pedidos do autor:
- "os controles ficam escondidos e precisam deslocar a barra de rolagem.
  prefiro quando os controles são visíveis";
- "o quadro do ecossistema ocupa muito da tela";
- "o ecossistema deve continuar no centro e os módulos ao redor dele";
- "não há botão de mute ou pause, ou controle de master".

- [x] Superfície ampla (a partir de 1800×960):
      - o ecossistema no centro e duas colunas de cartões de cada lado
        (Geometria | Síntese, Cauda e Master | ecossistema | Espaço e
        Ambiente | Módulos), com quatro botões por linha e o cabeçalho
        compacto;
      - os 52 botões visíveis sem rolagem, com as mesmas áreas de toque;
      - nas telas menores, o layout anterior (com rolagem) continua como
        reserva.
- [x] A janela abre na área útil inteira numa tela grande. Os tamanhos
      salvos pela versão anterior são ignorados uma vez
      (`window.layoutVersion` 2).
- [x] SILENCIAR (o ecossistema segue vivendo) e PAUSA (o tempo para) no
      cabeçalho, com rampa de 20 ms; o MASTER fica sempre à vista.
- [x] Testes do layout amplo em `WindowLayoutPolicyTests`; suíte 50/50;
      captura de tela conferida em 1920×1006.
- [x] O quadro do ecossistema sem espaço vazio (o autor: "o quadro do
      ecossistema tem muito espaço vazio").
      - A teia de relações é desenhada na esfera: a cor é a relação, a
        espessura é o vínculo e um ponto marca o alvo.
      - Abaixo da esfera, o painel MÚSICA DO ECOSSISTEMA mostra:
        - os cinco estados da água e o calor latente pago;
        - a tensão;
        - o tom, o modo, o andamento e o nicho (eventos/capacidade);
        - a legenda;
        - os últimos acontecimentos, com a hora.
      - Com a MÚSICA em zero, o painel explica como ligá-la.
      - Conferido em captura após 30 s.
- [ ] Escuta e uso do autor.

## Música ecológica — a teia, os estados líquidos e o cartão no app — 2026-10-20

Concepção, pesquisa e medições em `ECOLOGICAL_MUSIC_DESIGN.md` (§§ 1–6).

- [x] Diagnóstico medido: o "sino de vento", ~33 eventos/30 s, sem papéis
      nem arco.
- [x] `EcologicalMusicDirector` em cinco camadas:
      - os estados líquidos com calor latente;
      - a propagação: atraso, absorção ∝ f², Doppler e eco;
      - a teia de relações 5×5;
      - a harmonia por compatibilidade, com tensão e cadência;
      - a densidade como capacidade logística do nicho.
- [x] Testes do diretor e render de uma estação, antes/depois, com roteiro
      de escuta (`validation-output/ecological-music-2026-10-20/`).
- [x] Cartão no app (02 GEOMETRIA): MÚSICA, REAÇÃO, MEMÓRIA e ÁGUA, com a
      leitura ao vivo no status (estado da água · ato · tensão).
      - Parâmetros persistidos; cenas antigas abrem com a MÚSICA em zero.
      - Layout de 12 controles.
      - Suíte 50/50; o app recompilado abriu isolado por 10 s sem falha.
- [x] Articulação (o autor: "há timbres com notas longas, que são chatas,
      bem irritante").
      - Medido: com a música ligada, cada organismo soava 97–99% do tempo,
        um drone de cinco vozes. A energia sobe em até 1 s, cai em até
        2,25 s, e os corpos de evento tinham um piso de 28%.
      - Agora cada nota tem duração (o comprimento escrito × o legato da
        água) e um fim (a liberação da água); o ataque é de 6 ms; entre as
        notas, a voz se cala.
      - Medido depois: cada organismo soa 22–37% do tempo, com notas de
        0,4–0,9 s em mediana.
      - Sem estalos nem clipes no render (pico 0,19); o nível médio cai
        cerca de 4 dB pelos silêncios.
      - Render: `validation-output/ecological-music-2026-10-20-articulado/`.
- [ ] Escuta do autor: render e app com MÚSICA > 0.
- [x] Frequências líquidas no timbre: um passa-baixa de um polo por
      organismo, com o corte no ponto de −3 dB da absorção ∝ f² ao longo do
      caminho dentro da água até o ouvinte (`waterPathToListener`).
      - Influência 0: o caminho é pulado (bit a bit idêntico).
      - Medido em 300 s (centroide): ~2,8 → ~1,2 kHz na fervura, ~2,7 → 2,0 kHz
        no gelo, ~2,9 → 1,7 kHz na água temperada; perda de nível < 1 dB.
      - Render: `validation-output/ecological-music-2026-10-20-liquido/`.
      - Escala ajustável em `liquidCutoffHz` (×0,01), se a escuta pedir mais
        brilho.
- [x] Rede de bolhas cooperativa (jubartes) e "revolução" de canto (feitas no
      `EcologicalMusicDirector`: a espiral da rede de bolhas e a revolução que
      se espalha entre quem ouve; conferido no código em 2026-10-22).
- [ ] Aprovação musical.

## Revisão e commit do trabalho pendente — 2026-10-20

O Claude Code entrou no Aquorbium com a autorização do Codex, por pedido do
autor. O autor pediu: "revise e teste as mudanças antes de commitar,
limitando o commit ao Aquorbium".

O trabalho estava parado desde 2026-10-11: 215 arquivos não commitados
(37 modificados e 178 novos).

- **Cópia de preservação:**
  `RASGO_ARQUIVO/.local-preservation-aquorbium-20261020/` (o `tar.gz` e a
  lista).
- **Inventário:** nenhum áudio, saída de validação ou build entra no
  commit. `validation-output/`, `build-*` e `.local-build/` continuam
  ignorados.
- **Build limpo e novo** (`.local-build/revisao-20261020`, Release, JUCE em
  `third_party/JUCE`): sem erros e sem avisos.
- **Suíte:** a primeira rodada deu 47/48. O
  `aquorbium_control_response_tests` falhou em `form_gate_zero`: com FORM
  INFLUENCE em zero, os controles CONTEXT/INERTIA/MUTATION ainda mudavam o
  som.
  - **A causa:** o `CompositionalVariationDirector` (2026-10-02/03) recebia
    `formMemory.currentState()` sem o portão. O plano dele mexe na fase
    harmônica e nos acentos.
  - **A correção** (`AutonomousEnergy.cpp`): o diretor recebe o estado
    formal só com FORM INFLUENCE acima de zero. Em zero, recebe o estado de
    repouso (`formRestState = 3`, o mesmo do `reset`). O próprio comentário
    do código já pedia isso: "FORM MIX at zero cannot perturb the
    established autonomous sequence".
  - **Depois da correção:** 48/48.
- **AddressSanitizer + UndefinedBehaviorSanitizer** (só o núcleo e os
  testes): 43/43 pelo `ctest`, e os dois testes longos rodados direto
  (lens 73 s, control response 424 s), sem nenhum erro.
  - O `aquorbium_realtime_tests` fica de fora nesse modo: ele substitui o
    `operator new/delete` global para contar alocações, o que conflita com
    o ASan. Ele passa no build normal.
- **O app novo abre:** com a pasta pessoal isolada e o som numa saída nula
  (conferido com o `pw-link`), a janela aparece e o processo segue vivo.
  Não é aprovação de escuta.
- **Revisão do caminho de tempo real** (`getNextAudioBlock`,
  `consumeEvents`, a telemetria): só atômicos, filas sem trava com limite
  por bloco e estruturas fixas. Não há arquivo, alocação nem trava no
  callback.
- **Achado, não corrigido:** o `./run` abre
  `build-reactivation-20260915`, cujo binário é de 2026-09-30, anterior ao
  trabalho de 01 a 03/10. Quem roda `./run` ainda não ouve o master
  criativo, o loudness nem o diretor de variação. Basta recompilar essa
  pasta (o caminho antigo é um link para o novo) ou apontar o `./run` para
  um build novo.

## Mais variação nas sonoridades e na composição — 2026-10-20

O autor apontou a necessidade de "variar mais as sonoridades e o aspecto
composicional" e aprovou as melhorias propostas: "pode fazer as melhorias
propostas".

- [x] **Forma:** os segmentos ficaram mais longos (a base passou a 6, 4, 6
      e 3 fronteiras). O caminho até a suspensão passou de ~20 s para 49 s
      (`COMPOSITIONAL_VARIATION_DESIGN.md`).
- [x] **Caráter por segmento:** cada segmento tem um protagonista no banco
      (A, as melodias; A′, o pulso; B, as texturas e o movimento; a
      suspensão, o espaço). Medido: A′ +33 % de ataques, B −33 % e mais
      brilho.
- [x] **Striker audível:** a influência vai até 40× (era 8×). A influência
      passou de −29,8 dB para −15,2 dB sob a mistura; matéria, população e
      dispersão ficaram entre −18 e −11 dB.
- [x] **Espaço audível:** +6 dB nas reflexões, fora da realimentação. O
      retorno passou de −16,4 para −10,2 dB, o tamanho de −13,5 para −7,5 e
      a abertura de −20,7 para −14,7.
- [ ] **Espaço: tom e memória** continuam discretos (−23,7 e −26,8 dB). Dá
      para ampliar a faixa do tom ou a escala da memória depois da escuta.
- [x] **Testes:** um teste dedicado ao diretor (determinismo, ordem,
      durações, protagonistas, limites); pisos de audibilidade no
      `ControlResponseTests`, para o Striker e o Espaço não voltarem a
      sumir. Suíte 49/49; os testes do diretor, do motor autônomo e do core
      passam sob ASan/UBSan.
- [x] **O `./run` recompilado** com tudo isso.
- [ ] **Escuta do autor:**
  - `validation-output/compositional-variation-2026-10-20{,-v2}/` (antes e
    depois);
  - o app pelo `./run`, mexendo em STRIKER MIX e SPACE MIX.

## Diretor de variação composicional — refinamento musical em 2026-10-03

Diretriz do instrumento: o Aquorbium é antes de tudo musical, generativo e
criativo. A camada modular deve servir à composição e à performance, não
transformar o som em demonstração robótica de parâmetros. Nesta rodada, a
célula melódica ganhou um contorno mais coeso e as fases formais passaram a
ter durações variáveis de 1–4 fronteiras coletivas, com retenções
determinísticas ligadas ao contexto. A duração fica exposta na telemetria e no
CSV de escuta. Isso é uma hipótese composicional implementada, ainda sem
aprovação auditiva; consulte `COMPOSITIONAL_VARIATION_DESIGN.md`.

### Implementação inicial em 2026-10-02

`CompositionalVariationDirector` organiza o material existente em quatro
segmentos determinísticos: estabelecer, variar, responder e suspender. A
Geometria entrega seus acentos ao ganho das vozes, com pesos de segmento
limitados e dosados pela intensidade de `STRUCTURE MIX`; eventos sem acento
permanecem neutros. A contribuição melódica da memória também varia por
segmento, reforçando o motivo em A′/B e recuando na suspensão; coesão e calma
ajustam levemente essa retenção. A leitura direta/retrógrada em B e a rotação
do padrão euclidiano agora fazem morph contínuo, dosado por `STRUCTURE MIX`.
Avança nas
fronteiras coletivas existentes, sem alegar sincronização métrica. O plano
altera densidade, contribuição do motivo e ordem de leitura da memória,
acentuação, fase harmônica, balanço das quinze famílias e feedback espacial.
`STRUCTURE MIX` dosifica a política; com
zero, a rota anterior permanece. `LAB MIX` e as macros de família continuam
controlando se e quanto cada família é ouvida. Não foram criados controles ou
campos persistentes.

Os alvos estáticos `aquorbium_core` e
`aquorbium_compositional_variation_render` compilaram em 2026-10-03 usando um
job. O aplicativo standalone não foi compilado porque a configuração local
não encontrou JUCE (`AQUORBIUM_JUCE_PATH` não definido). Não foram executados
testes, renders nem escuta musical. Contrato e protocolo manual em
`COMPOSITIONAL_VARIATION_DESIGN.md`.

- [x] Implementar A/A′/B/suspensão com permanência variável e célula melódica
      autoral, ligando o diretor à Geometria, progressão,
      famílias do banco e reflexões usando os controles existentes
- [x] Compilar `aquorbium_core` (alvo estático, um job)
- [x] Adicionar e executar regressões de determinismo, bypass, limites e
      invariância de bloco para o diretor e suas rotas (2026-10-22:
      `VariationRouteTests` no motor completo prova o bypass exato, a ação
      real quando ligado e a saída igual bit a bit com blocos de 64, 256 e
      irregulares 7/1000; determinismo, ordem e limites do próprio diretor
      ficam em `CompositionalVariationDirectorTests`)
- [x] Renderizar ao menos quatro segmentos com seed fixa, em bypass e ativo,
      incluindo rotas autônomas e banco de módulos (feito em 2026-10-20 e
      marcado em 2026-10-22: `validation-output/compositional-variation-2026-10-20{,-v2}/`,
      seed 0x815ab1e, 49 s cada, `four_segments_reached=1`, banco com quatro
      famílias; a escuta segue aberta no item abaixo)
- [x] Preparar e compilar `aquorbium_compositional_variation_render`, com
      limite de 180 s, WAV float estéreo e CSV por segmento
- [ ] Escutar e registrar ritmo, melodia, harmonia, timbre, eventos, forma e
      função composicional; ajustar os perfis somente a partir dessa evidência

## Coorte de refinamento do núcleo — integrada em 2026-10-02

Três componentes novos e uma integração de gesto fecharam lacunas já
registradas: `OversampledSineFolder` atua nas vozes FM e BiomaBass;
`BoundedMasterFeedback` devolve redução de ganho real ao Cérebro a 200 Hz;
`AirWaterMedium` diferencia ar e água em `FLUID + PHASE`; e o
`MotionGestureRecorder` grava a trajetória XY real da Lupa e a reproduz em
`GESTURE WAVES`. Não houve novo knob, campo persistente ou alteração de layout.

Os testes dirigidos aprovaram bypass exato, redução de energia na banda alta,
limites e balística do feedback, diferença ar/água, captura/reprodução da Lupa,
motor completo e zero alocação no callback. O teste espectral é um portão
comparativo específico, não uma alegação de wavefolder universalmente livre de
alias. Meio por organismo, escuta e dispositivo real continuam abertos. A
suíte integral passou 45/45 em 93,05 s; o benchmark combinado teve pior caso de
62,08% em 96 kHz/bloco 128/sem vozes externas (1,61× offline). Contrato:
`CORE_REFINEMENT_COHORT_DESIGN.md`.

## Coorte de interação aberta — core e rota experimental em 2026-10-02

Quatro hipóteses ainda sem código receberam implementações C++20 próprias:
`DoubleScrollAttractor`, `SemanticAccessRouter`, `OpenRuleProgram` e
`ContactCrackleExciter`. Elas entram, respectivamente, em `LORENZ + DRIFT`,
`BRANCH + PERMUTE`, `EVOLVE + STAGES` e `GESTURE WAVES`, sem criar knobs,
parâmetros persistentes ou uma nova página. O atrator toma como referência
conceitual a função musical declarada pelo Joranalogue Orbit 3, mas usa modelo
digital, parâmetros e integração autorais; não copia circuito nem software.

O teste dedicado aprovou determinismo, saídas complementares, regiões do
double-scroll, conversão entre áudio/controle/evento, parser transacional,
valores não finitos, limites e bypass exato do exciter. O banco integrado
aprovou bypass, quinze famílias, determinismo, ecologia e telemetria específica
da coorte. O teste de tempo real também passou com zero alocação no callback.
As compilações foram direcionadas e executadas com um job; não foram rodados
suíte integral, benchmark, app real ou escuta porque o anfitrião permanecia com
1,7/2 GiB de swap ocupados e outra tarefa de desenvolvimento ativa.

O roteador fixo do laboratório não conclui o `AccessPort` editável por
organismo. O exciter disparado por eventos não conclui o gesto de dois pontos
nem usa jitter físico real. O programa de regras usa uma gramática pequena e
fixa, compilada fora do callback, mas ainda não possui editor GUI/APVTS. A
comparação musical Lorenz/double-scroll também aguarda escuta A/B. Contrato:
`OPEN_INTERACTION_COHORT_DESIGN.md`.

## Medição de loudness pós-safety — implementada em 2026-10-01

Objetivo: avançar a saída técnica sem alterar o caráter do instrumento. Foi
criado `LoudnessMeter`, adaptado do trabalho autoral já existente na família
RASGO, e ligado ao `OutputStage` depois da proteção final. A implementação
deriva a ponderação K da taxa ativa, forma janelas de 400 ms e 3 s e calcula o
integrado com portas absoluta e relativa. Histograma e estados têm tamanho
fixo; `push()` não aloca, não bloqueia e nunca aplica ganho ao áudio.

As leituras momentary/short-term/integrated, prontidão e número de blocos foram
acrescentados à telemetria. A ponte atômica leva M/S/I ao tooltip de ÁUDIO e a
auditoria de runtime grava esses campos no `REPORT.txt`. Não foi criado knob,
campo de cena nem alteração de layout.

Validação executada: checagem sintática estrita dos quatro arquivos de core e
teste afetados; executável leve dedicado com referência EBU, relações de
amplitude/canais, 44,1/48/96 kHz, silêncio, portas, valores não finitos e reset;
executável leve do `OutputStage` com medição pós-safety. Ambos passaram. O
preflight do anfitrião encontrou 9 GiB disponíveis, disco em `rw` com 1014 GiB
livres, mas 1,7/2 GiB de swap usados e outra sessão do Claude ativa. Por isso,
conforme a governança de recursos, não foram executados build CMake, suíte
integral, benchmark nem app real. Não houve commit, push ou modificação do
Rasgo Modular.

Retomada: com swap abaixo de 50% e sem tarefa pesada concorrente, configurar o
novo alvo, compilar com um job, executar primeiro `aquorbium_loudness_tests`,
`aquorbium_output_stage_tests` e `aquorbium_realtime_tests`; depois rodar a
suíte integral e repetir o benchmark. Somente então atualizar o número da suíte
e o binário apontado por `./run`. True peak normativo, perfis de destino,
gravação e escuta musical continuam separados e abertos.

## Master criativo inicial — concluído tecnicamente em 2026-09-30

`MasterChain` entrou entre a calibração/cauda e o `OutputStage`. A coorte reúne
crossover Linkwitz–Riley de quarta ordem em três bandas, compressão estéreo
vinculada por banda, tilt de três bandas, largura mid/side preservando o grave
e cola óptica lenta. O `crackStress` estreita médio/agudo; o limitador técnico
continua por último e não recebe modulação ecológica.

O painel ganhou um único macro grande `CREATIVE MASTER`, com bypass exato em
zero e rampa de 45 ms. Ele foi anexado ao fim do formato persistente, que passa
de 62 para 63 valores totais; cenas anteriores conservam todos os índices e
recebem zero no campo novo. A auditoria gerou 72 capturas em quatro resoluções,
com knob gráfico mínimo de 52 px, área gestual mínima de 80×92 px e sem
sobreposição nas vistas Master inspecionadas.

O teste dedicado cobre bypass, dinâmica, cola, colapso de largura sob trinca,
não finitos e zero alocação. A suíte integral passou 45/45 em 104,45 s. O
quadro de áudio mede 264 bytes e a ponte fixa permanece em 1024 bytes. Com banco e
Master ativos, o benchmark mediu 21,57% em 48 kHz/bloco 128/quatro vozes; o
pior caso foi 42,95% em 96 kHz/bloco 32/quatro vozes, margem offline de 2,33×.
Escuta musical, EQ antes/depois exposto, feedback de redução ao Cérebro e
perfis de destino de LUFS continuam abertos. O medidor BS.1770 pós-safety foi
integrado em 01/10. Contrato: `CREATIVE_MASTER_COHORT_DESIGN.md`.

O binário Release final mede 14.220.656 bytes e tem SHA-256
`d0f688d05105d1fe6c4c7f7c5a6e3942214da2b48215829e6565a5e554fe6e32`.

## Coorte de integração especializada — concluída tecnicamente em 2026-09-29

Seis ligações deixaram de ser apenas protótipos genéricos e passaram a atuar
no encadeamento real. O subgrave do `BiomaBassVoice` recebe intervalos
negativos do `IntervalQuantizer`; a captura granular deriva parâmetros de
`strength`, `agility` e `intelligence`; o `Species::Pulsar` ganhou pulsaret,
cluster e máscara por pulso; a amostra final pós-Space/cauda/saída retorna ao
sequenciador por `CrossFeedbackStateMachine`; o contexto de `Desire` usa soft
take-over; e o `MultiTapSpace` recebe deriva lenta limitada por
`SlewRateRandomWalk`. A generalização do soft take-over para um futuro
`AccessPort` permanece aberta porque essa rota ainda não existe.

Não houve novo knob, campo persistente ou mudança de layout. Build Release e
44/44 testes passaram em 94,24 s, incluindo zero alocação, segurança numérica,
determinismo e deriva do Space. Com o banco ativo, o caso
48 kHz/bloco 128/quatro vozes mediu 19,21% da janela; o pior caso observado foi
38,92% em 96 kHz/bloco 64/quatro vozes, ainda 2,57× tempo real offline. A
escuta musical e a estabilidade prolongada no dispositivo continuam abertas.
Contrato: `SPECIALIZED_INTEGRATION_COHORT_DESIGN.md`.

## Coorte de geração e interação — concluída tecnicamente em 2026-09-29

Dez mecanismos antes ausentes foram codificados, testados e ligados às
famílias existentes: `FixedLSystemGenerator`, `CellularAutomatonSequencer`,
`SpectralTransferVocoder`, `EnvelopeContagionBus`, `NoInputFeedbackBus`,
`ReactiveThresholdStrategy`, `LockInFocusExtractor`, `BehaviourSceneBank`,
`ReconfigurableResourcePool` e `LightweightBinauralSpatializer`. A superfície
continua com quinze famílias, dezesseis macros e 62 valores persistentes.

A suíte passou 44/44 em 91,95 s, incluindo banco ativo e zero alocações. Em
48 kHz/bloco 128/quatro vozes, o banco ativo mediu 23,24% da janela; o pior
ponto observado foi 41,69% em 96 kHz/bloco 32/quatro vozes, ainda 2,40× tempo
real offline. O bypass continua retornando antes das três coortes.

Foram encerrados somente os protótipos realmente entregues. HRTF medida,
integração por `Organism`/`schoolId`, cenas persistentes do `Ecosystem`, Master
sem entrada e promoção dos geradores à Geometria continuam abertas. Contrato:
`GENERATIVE_INTERACTION_COHORT_DESIGN.md`.

## Coorte avançada de voz e ecologia — concluída tecnicamente em 2026-09-28

Dez mecanismos adicionais foram codificados, testados e ligados diretamente às
famílias já visíveis do banco: `BiomaBassVoice`, `SchoolWaveVoice`,
`HarmonicRippleVoice`, `RecentMixWavetable`, `FrequencyShifter`,
`CrossFeedbackStateMachine`, `SpotlightMixer`, `HungerProcessor`,
`BoundedSelfFeedback` e `WaterInertia`. A detecção de onset do granular também
passou a disparar a captura automática. Nenhum knob, campo persistente ou
formato de cena foi acrescentado.

A suíte completa passou 43/43 em 91,09 s e o teste de tempo real continuou com
zero alocações. Com todas as quinze famílias ativas, o benchmark Release mediu
17,70% da janela em 48 kHz/bloco 128/quatro vozes; o pior ponto observado foi
35,78% em 96 kHz/bloco 64/quatro vozes, ainda 2,80× tempo real offline. Esses
números não garantem prazo do dispositivo nem ausência de XRUN.

Esta conclusão é do protótipo e de sua rota experimental. Permanecem abertas a
ligação do sub ao quantizador real, o cross-feedback da saída final ao
sequenciador, a derivação granular por `Organism`, os vínculos especializados
com `SchoolInfo`/`Ecosystem` e a escuta por família. Contrato:
`ADVANCED_VOICE_ECOLOGY_COHORT_DESIGN.md`.

## Coorte emergente integrada — concluída tecnicamente em 2026-09-28

Dez novos componentes de core foram codificados, testados e incorporados às
famílias visíveis do banco: `ReeseVoice`, `DualPeakFilter`,
`ResonantFilterSequencer`, `FrequencyIsolator`, `InteractionProcessor`,
`PowerStarve`, `QuadratureStereoRotator`, `PolygonalOscillator`,
`BoundedRoutingMatrix` e `SharedParameterPool`. Eles reutilizam as quinze
macros existentes; não acrescentam knobs, páginas, parâmetros persistentes ou
campos à ponte de áudio.

O teste dedicado cobre determinismo, limites, topologias, bypass e invariantes
dos dez componentes. O banco integrado e o teste de alocação também passaram.
O benchmark com todas as famílias em 0,6 mediu 13,77% da janela em
48 kHz/bloco 128/quatro vozes e máximo de 30,66% em 96 kHz, ainda 3,26× tempo
real. Em `LAB MIX=0`, o banco retorna antes dos motores. Esses números são
offline e não garantem prazo do dispositivo nem ausência de XRUN.

Os itens de pesquisa abaixo foram marcados como protótipos concluídos, mas
conservam explicitamente suas integrações especializadas ainda abertas: Lupa,
`AquariumTransform`, predador/presa real, `AccessPort`, `GeometricSequencer`,
forma visual do organismo e BiomaBass. Contrato:
`EMERGENT_MODULE_COHORT_DESIGN.md`.

## Promoção dos módulos codificados — concluída tecnicamente em 2026-09-28

A auditoria de visibilidade encontrou cerca de quarenta componentes de core
codificados e testados fora do callback e do painel. O
`IntegratedModuleBank` encerrou essa pendência técnica: os protótipos da fila
agora participam de uma rota ao vivo limitada, possuem estado persistente e são
operáveis no cartão `08 MÓDULOS` por `LAB MIX` e quinze macros de família.

Estado atual:

1. rota ao vivo e contribuição estéreo limitada: concluídas;
2. controles grandes, dependência visual, persistência e cenas: concluídos;
3. migração das cenas anteriores de 25/31/35/39/42/46 para 62 valores:
   concluída, com o banco em bypass;
4. bypass sem custo por amostra, determinismo e ausência de alocação:
   aprovados;
5. build Release e suíte integral: 44/44 aprovados em 91,95 s;
6. auditoria visual por capturas: não executada, pois `xvfb-run` não está
   instalado; teste geométrico aprovado;
7. escuta por família e promoção para semânticas ecológicas definitivas:
   pendentes.

O quadro de áudio mede 260 bytes e a ponte fixa 1024 bytes. Após as três
coortes, o candidato tem 14.217.776 bytes e SHA-256
`588c8dbfc0f44a18fd6fb5b3b561dd106b69c60c70bfa1b8b21ea3f1fbeddee3`.
Mapa, rota e limites: `INTEGRATED_MODULE_BANK_DESIGN.md`.

## Integração operável — memória endereçável e harmonia — 2026-09-27

`AddressableStepMemory` e `HarmonicProgression` deixaram o banco isolado. O
`AutonomousEnergy` os processa nas fronteiras de evento da Geometria: um padrão
determinístico de oito passos fornece deslocamento de altura e gate, enquanto
uma fase que interpola dia e estação seleciona a seção e os graus harmônicos.
O endereço também considera fase orbital e identidade do organismo.

O cartão `02 GEOMETRIA` expõe quatro macros grandes na superfície rolável:
`STRUCTURE MIX`, `STEP MEMORY`, `HARMONY` e `PHASE PATH`. Em `STRUCTURE MIX=0`,
os três controles dependentes ficam inativos e o áudio permanece idêntico bit
a bit ao caminho anterior. Os componentes avançam de modo determinístico no
bypass, sem alocação e sem perturbar os streams aleatórios anteriores.

Estado da família:

| Etapa | Estado |
|---|---|
| protótipos e contrato | concluídos |
| integração na fronteira de eventos | concluída |
| controles, persistência e cenas | concluídos; este marco criou 46 valores; formato atual 63, com migração 25/31/35/39/42/46/62 |
| validação automatizada | concluída para resposta, bypass, determinismo, layout e tempo real |
| escuta musical e estabilidade prolongada | pendentes |
| edição detalhada, `AccessPort` e bancos | pendentes |

A suíte completa passou 40/40 em 80,53 s e o teste de resposta passou em
59,53 s. O teste de alocação mediu quadro de 196 bytes e ponte fixa de 832
bytes. A auditoria final em
`validation-output/2026-09-27-structure-memory-layout-v3/` gerou 40 PNGs:
quatro resoluções e dez vistas por resolução. As quatro linhas da Geometria em
960×600 foram inspecionadas sem sobreposição, com knobs de pelo menos 52 px e
áreas interativas de 80×92 px preservados por rolagem.

O binário canônico apontado por `./run` tem 14.040.848 bytes e SHA-256
`8d4c32c83aa24731f8b2f7f4dfeb76819d4a16124eeaeeb17e07062abb534766`.
Ele ainda não recebeu escuta humana desta família. Retomada: terminar a sessão
de estabilidade do build anterior, reiniciar `./run`, subir `STRUCTURE MIX` e
avaliar `STEP MEMORY`, `HARMONY` e `PHASE PATH` um por vez.

## Integração operável — saída técnica inicial — 2026-09-27

`OutputStage` foi inserido no ponto final de `BiomaEngine`, depois do Espaço e
da Cauda. É uma camada técnica separada do Master criativo posterior: saneia
`NaN`/`Inf`, bloqueia DC em 10 Hz, aplica trim e mute suavizados e limita os dois
canais com detector vinculado e lookahead de 5 ms. A homeostase continua
observando o balanço interno anterior ao ganho de calibração e à proteção; o
Ecossistema não modula teto, trim ou release.

O cartão `07 MASTER` deixou de ser fictício e identifica `SAFETY ACTIVE`. Ele
expõe três macros técnicas grandes e persistentes: `OUTPUT TRIM` (-18 a +6 dB),
`CEILING` (-6 a -0,1 dB; inicial -1 dB) e `RELEASE` (50 a 500 ms; inicial
150 ms). Naquele marco, o Master criativo multibanda permanecia planejado; em
30/09 ele entrou antes desta camada sem alterar sua função técnica.

Estado desta entrega:

| Etapa | Estado |
|---|---|
| saneamento final e bloqueio DC | concluídos |
| limiter estéreo vinculado e lookahead | integrado; detector intersample 4× estimado |
| controles e cenas | a entrega criou o formato de 42 valores; formato atual com 46 e migração 25/31/35/39/42 |
| telemetria | sample peak, pico intersample estimado, RMS, redução, inválidos, latch, latência e LUFS M/S/I |
| validação automatizada | build completo e suíte 40/40 aprovados |
| loudness BS.1770 | medidor e vetores de referência EBU aprovados em teste leve em 01/10; suíte integral pendente |
| conformidade true peak | pendente; estimativa atual não reivindica dBTP normativo |
| escuta e dispositivo real | em validação; primeira sessão interativa estável |

A auditoria offline gerou 28 capturas em quatro resoluções e confirmou knobs
de pelo menos 52 px, áreas de 80×92 px, textos separados e ausência de
sobreposição no Master. As vistas 1920×1006 e 960×600 foram inspecionadas. O
teste de tempo real aprovou o estágio integrado sem alocação no callback; a
suíte completa passou 40/40 em 71,10 s. Contrato e limites ficam em
`OUTPUT_STAGE_DESIGN.md`. Isso reduz risco de clipping/contaminação numérica,
mas não prova que os estalos ou a travada históricos do dispositivo foram
resolvidos.

O binário Release usado por `./run` foi recompilado após a auditoria final:
14.026.568 bytes, SHA-256
`7bb3c59782beaf597afaf193e9a2d2d96c06cde63909f15f0d8e73d71c6e4e79`.
Os seis testes críticos também passaram nesse build canônico. Nenhuma instância
foi deixada aberta.

Escuta manual de 27/09: usando áudio interno, o usuário manipulou praticamente
todos os controles e relatou duas vezes que a reprodução continuava sem
estalos/clipes ou travamento. Ao fim do relato, a interface mostrava `XRUN 0`,
`LATE 0`, `CLIP 0` e `GUI 0`. É a primeira regressão humana ampla com resultado
positivo depois dos incidentes anteriores. Durante a continuação da mesma
sessão, os contadores chegaram a `XRUN 2` e `LATE 2`; `CLIP` e `GUI` não foram
reconfirmados nesse novo relato. Portanto, o resultado é substancialmente mais
estável, mas não uma execução tecnicamente limpa. Duração, taxa e buffer também
não foram reconfirmados; estabilidade prolongada e aprovação musical controle
a controle continuam abertas.

## Integração operável — Ambiente espectral — 2026-09-25

`SchoolSpectralBank` e `EcologicalNoiseBed` deixaram de ser apenas protótipos
isolados. O `BiomaEngine` agora os processa depois da soma dos organismos e
antes do Espaço. O cartão `05 AMBIENTE` oferece seis macros grandes e
persistentes: `ENV MIX`, `FILTER`, `CENTER`, `SPAN`, `RESONANCE` e `AMBIENCE`.
Com `ENV MIX=0`, a saída anterior é preservada bit a bit e os cinco controles
dependentes ficam inativos.

Estado da família:

| Etapa | Estado |
|---|---|
| pesquisa e contrato | concluída |
| protótipo de core | concluído |
| integração na rota de áudio | concluída |
| controles e cenas | concluídos, com migração das cenas V1 antigas |
| validação automatizada | concluída para resposta, bypass, layout e tempo real |
| escuta musical e estabilidade no dispositivo real | pendentes |

Arquivos principais: `src/core/BiomaEngine.{h,cpp}`,
`src/app/AudioControlFrame.h`, `src/app/PerformanceParameterState.h`,
`src/app/PerformanceSceneControls.h`, `src/app/Main.cpp`,
`tests/ControlResponseTests.cpp`, `tests/EcologyParameterStateTests.cpp`,
`tests/PerformanceScenePlaybackTests.cpp` e `tests/RealtimeSafetyTests.cpp`.

Validação dirigida: 6/6 testes passaram. Todas as macros produziram diferença
mensurável quando ativas; bypass produziu diferença zero; o cenário de quatro
vozes atingiu pico 0,535761. A auditoria offline gerou 16 capturas em quatro
dimensões e confirmou mínimos de 52 px/80×92 px, separação de texto e rolagem.
No marco do Ambiente, o build Release completo e a suíte 39/39 passaram em
45,86 s. Aquele binário, gerado em 25/09 às 20:11 CEST, tinha SHA-256
`cd400f36aff8959276fe421c707c0d794aa0ec3ca0e3b910a39b3165aa20bdeb`.
Ainda não houve escuta humana nem aprovação dos estalos/travamentos históricos.

Naquele marco, `BehaviourMemory` permanecia isolado e não entrava na entrega.
Desde 28/09, suas quatro classes participam da família experimental
`INTEGRITY + MEMORY`; a semântica ecológica especializada continua pendente.

## Integração operável — Cauda sintética — 2026-09-25/26

`AquariumTail` deixou de ser protótipo isolado. Uma instância compartilhada
recebe a soma já acrescida das primeiras reflexões e produz a cauda tardia. O
cartão `06 CAUDA` oferece `TAIL MIX`, `DECAY`, `DAMPING` e `NIGHTMARE`, com
knobs grandes, persistência e bypass bit a bit. `size` permanece fixo porque a
troca de comprimento ainda pode gerar descontinuidade.

Estado da família:

| Etapa | Estado |
|---|---|
| pesquisa e contrato | concluída |
| protótipo de core | concluído |
| integração na rota de áudio | concluída |
| controles e cenas | concluídos; cenas de 25 e 31 valores migram para 35 |
| validação automatizada | concluída para resposta, bypass, render, layout e tempo real |
| escuta musical e estabilidade no dispositivo real | pendentes |

Validação dirigida: as quatro macros produziram diferença mensurável; o gate
em zero produziu diferença exata zero; quatro vozes atingiram pico 0,462652.
O renderizador gerou quatro WAVs de 12 s, repetiu o bypass bit a bit e mediu
picos abaixo de 0,237. A auditoria visual gerou 20 capturas em quatro dimensões
e confirmou knobs ≥52 px e áreas ≥80×92 px; 1280×800 e 960×600 foram
inspecionados sem sobreposição. Evidências ficam em
`validation-output/2026-09-25-synthetic-tail/` e
`validation-output/2026-09-25-tail-layout/`. Escuta não foi executada.

Fechamento técnico em 26/09: build Release completo e suíte 39/39 passaram em
57,10 s. O executável apontado por `./run` tem 13.999.152 bytes e SHA-256
`d8d9f6bc953cc21eb4d2b365dfa0056db1f536bc215c0617e6626292288fdd93`.
O teste de alocação ativo passou com quadro de 152 bytes e ponte de 704 bytes.
`git diff --check` passou. Não houve teste com dispositivo real nesta rodada.

Ponto de retomada musical: executar `./run`, rolar até `06 CAUDA`, subir `TAIL MIX` e
escutar primeiro DECAY, depois DAMPING e NIGHTMARE. Registrar estalos/travadas
separadamente da avaliação musical. Depois decidir calibração e vínculo
ambiental. A integração técnica seguinte, `BiomaMarkov`, foi concluída no
marco abaixo sem apagar esta pendência de escuta.

## Integração operável — Memória formal Markov — 2026-09-26/27

`BiomaMarkov` deixou de ser apenas core isolado e agora pertence ao domínio
forma. Uma cadeia compartilhada de oito estados vive em `AutonomousEnergy`,
avança somente no cruzamento coletivo positivo do campo Relabi e desloca a
seção entregue ao `GeometricSequencer`. Não gera notas, amplitude ou timbre
diretamente e usa um stream aleatório separado.

O cartão `02 GEOMETRIA` oferece `FORM MIX`, `CONTEXT`, `INERTIA` e `MUTATION`
em knobs grandes e superfície rolável. Com `FORM MIX=0`, os três controles
dependentes ficam inativos e a saída é idêntica bit a bit; a cadeia continua
evoluindo para revelar o estado presente quando reativada. Os quatro valores
entram no snapshot, preferências e cenas. Cenas de 25, 31 e 35 valores migram
para o formato atual de 39 valores.

Estado da família:

| Etapa | Estado |
|---|---|
| pesquisa e contrato | concluída |
| protótipo de core | concluído |
| semântica e fronteira de eventos | forma; um avanço por evento coletivo |
| controles e cenas | concluídos, incluindo três migrações anteriores |
| validação automatizada | concluída para resposta, bypass, determinismo, render, layout e tempo real |
| escuta musical e estabilidade no dispositivo real | pendentes |

Validação dirigida: os quatro controles produziram diferença mensurável em
20 s; o gate de influência produziu diferença exata zero. O renderizador gerou
quatro WAVs de 20 s, sete transições por cenário, repetição do bypass bit a bit,
pico máximo 0,234 e razão de render abaixo de 0,049× tempo real. O teste de
alocação passou com quadro de 168 bytes e ponte de 704 bytes. A auditoria de
layout gerou 24 capturas em quatro dimensões e confirmou knobs ≥52 px, áreas
≥80×92 px e rolagem; 960×600 e 1280×800 foram inspecionados sem sobreposição.

Evidências: `validation-output/2026-09-26-form-memory/` e
`validation-output/2026-09-26-form-memory-layout/`. Build Release completo e
suíte 39/39 passaram em 68,78 s. O executável apontado por `./run` tem
14.008.336 bytes e SHA-256
`80b42a298b5540879f2ec0e19cd9afde44ba1afd6d39db95044cedbb1a333c19`.
Não houve teste com dispositivo real nem escuta nesta rodada.

Próximo passo: escutar primeiro `FORM MIX` em valor médio, depois variar um por
vez `CONTEXT`, `INERTIA` e `MUTATION`, observando forma de longo prazo e não
mudança instantânea. Registrar ritmo, melodia, harmonia, timbre, eventos e
função composicional; em paralelo, manter a investigação de estalos/travadas.

## Mistura e percurso de filosofias — 2026-09-25

Objetivo: estabelecer uma matemática comum para quatro fontes antes de alterar
o Ecossistema. `PhilosophyVectorMixer` produz pesos XY; `PhilosophySequencer`
percorre até 16 posições; `PhilosophyMorph` cruza fontes discretas sem salto;
`PhilosophyMatrixMixer` oferece duas mixagens estéreo independentes numa matriz
4×4 modulável.

Arquivos: `src/core/PhilosophyMixEngine.{h,cpp}`,
`tests/PhilosophyMixTests.cpp`, `tests/RealtimeSafetyTests.cpp`, `CMakeLists.txt`,
`PHILOSOPHY_MIX_DESIGN.md`, README, arquitetura, dossiê do Ecossistema e esta
lista.

Validação: cantos, centro, soma de pesos, relógio, duração, morph, matriz,
modulação, normalização, limites e heap passaram. Build Release e suíte 38/38
passaram em 42,50 s. Nenhuma rota, callback, parâmetro ou tela foi alterada.

Pendências: nomear e posicionar as quatro filosofias reais, escolher o relógio,
comparar leis de crossfade, ligar cenas/observadores e testar musicalmente.

## Assinatura espectral e ambiente — 2026-09-25

Objetivo original: prototipar identidade tímbrica de cardume e leito ambiental
sem alterar o app. `SchoolSpectralBank` reúne oito passa-bandas, centro/dispersão
logarítmicos e perfil determinístico por cardume. `EcologicalNoiseBed` deriva
textura de estação/hora, cor de turbidez/temperatura e redução temporal de
`crackStress`.

Arquivos: `src/core/SchoolSpectralBank.{h,cpp}`,
`src/core/EcologicalNoiseBed.{h,cpp}`,
`tests/SpectralEnvironmentTests.cpp`, `tests/RealtimeSafetyTests.cpp`,
`CMakeLists.txt`, `SPECTRAL_ENVIRONMENT_DESIGN.md`, README, arquitetura,
dossiê do Ecossistema e esta lista.

Validação: mapeamento, identidade, limites, determinismo, ciclos, cor,
degradação temporal e heap passaram. Build Release e suíte 37/37 passaram em
45,34 s. Nenhuma rota, callback, parâmetro ou tela do app foi alterada.

Continuidade: a integração posterior está registrada na seção acima. Permanecem
render, espectro, orçamento de ganho, escuta e diferenciação por `SchoolInfo`.

## Gestos e formas de movimento — 2026-09-24

Objetivo: materializar movimento por identidade e memória de gesto sem alterar
o aplicativo ativo. `MotionWaveBank` oferece oito formas e associação
determinística por espécie, estratégia e seed. `MotionGestureRecorder` grava
até 2.048 pontos XY, reproduz entre 0,125× e 8× e permite repetição exata ou
mutação limitada por volta.

Arquivos: `src/core/MotionWaveBank.{h,cpp}`,
`src/core/MotionGestureRecorder.{h,cpp}`, `src/core/FluidMotion.cpp`,
`tests/MotionGestureTests.cpp`, `tests/RealtimeSafetyTests.cpp`, `CMakeLists.txt`,
`GESTURE_MOTION_DESIGN.md`, README, arquitetura, dossiê do Ecossistema,
documento de movimento de cardume e esta lista.

Validação: formas, limites, identidade, loop exato, interpolação, seed,
mutação e heap passaram. Build Release e suíte 36/36 passaram em 38,31 s.
Nenhuma rota do app, callback, parâmetro ou tela foi alterada.

Pendências: render e escuta de trajetórias; captura real da Lupa ou controle
XY; mapa musical de espécie/estratégia; ligação gradual ao `Ecosystem` somente
depois dos portões de estabilidade interativa.

## Movimento compartilhado de cardume — 2026-09-24

Objetivo: testar coesão sem instanciar um gerador por organismo.
`HarmonicPhaseBank` deriva até oito fases racionais de um relógio mestre.
`FluidMotion` publica quatro leituras por forma, amplitude, tempo ou frequência
e admite autopatch limitado de uma saída na velocidade do próximo tick.

Arquivos: `src/core/HarmonicPhaseBank.{h,cpp}`,
`src/core/FluidMotion.{h,cpp}`, `tests/SchoolMotionTests.cpp`,
`tests/RealtimeSafetyTests.cpp`, `CMakeLists.txt`,
`SCHOOL_MOTION_DESIGN.md`, README, arquitetura, dossiê do Ecossistema e esta
lista.

Validação: relações racionais, determinismo, quatro modos, limites do feedback
e invariância bit a bit em feedback zero passaram. Build Release e suíte 35/35
passaram em 40,57 s, incluindo heap. Nenhuma rota do app foi alterada.

Pendências: render de trajetórias, escuta de coesão/mecanicidade, quantidade de
membros e política de espécies; só então avaliar substituição do movimento e
de `sharedPhase` atuais.

## Controle estrutural lento — 2026-09-24

Objetivo: materializar forma condicional, deriva de longo prazo e transição de
integridade sem conectar novas rotas. `ConditionalStageMachine` mantém oito
estágios com salto por perigo, noite ou trinca. `TripleSlowDrift` combina três
random walks em segundos/minutos/dezenas de minutos. `IntegrityCrossfade`
publica ganhos saudável/trincado suavizados e de potência constante.

Arquivos: `src/core/ConditionalStageMachine.{h,cpp}`,
`src/core/TripleSlowDrift.{h,cpp}`, `src/core/IntegrityCrossfade.{h,cpp}`,
`src/core/SlewRateRandomWalk.cpp`, `tests/StructuralControlTests.cpp`,
`tests/PendingModulesTests.cpp`, `tests/RealtimeSafetyTests.cpp`,
`CMakeLists.txt`, `STRUCTURAL_CONTROL_DESIGN.md`, README, arquitetura,
dossiê do Cérebro e esta lista.

Validação: o random walk passou a aceitar 0,0001 evento/s e relógios de
controle. Testes direcionados, regressão e heap passaram; build Release e
suíte 34/34 passaram em 41,01 s. Nenhuma rota do app foi alterada.

Pendências: semântica dos oito estágios; render acelerado da deriva; comparação
musical da lei de crossfade; integração gradual a `Desire`, `Environment` e
`AquariumIntegrity` somente após esses portões.

## Memória e distribuições probabilísticas — 2026-09-24

Objetivo: separar memória binária, forma estatística do acaso e estado de
repetição. `ShiftRegisterMemory` mantém 1–32 bits e mutação por avanço.
`ProbabilityDistribution` atravessa uniforme→gaussiana consumindo sempre a
mesma quantidade de acaso. `EvolutionLoopMachine` mantém 64 valores nos modos
random, looping, evolving e locked.

Arquivos: `src/core/ShiftRegisterMemory.{h,cpp}`,
`src/core/ProbabilityDistribution.{h,cpp}`,
`src/core/EvolutionLoopMachine.{h,cpp}`,
`tests/ProbabilisticMemoryTests.cpp`, `tests/RealtimeSafetyTests.cpp`,
`CMakeLists.txt`, `PROBABILISTIC_MEMORY_DESIGN.md`, README, arquitetura,
dossiê do Cérebro e esta lista.

Validação: build Release completo e suíte 33/33 passaram em 42,20 s. O teste
específico cobre seed, limites, mutação, distribuição de 20.000 amostras e os
quatro modos; o teste de heap inclui todos os avanços. Nenhuma rota do app foi
alterada.

Pendências: comparar musicalmente com `RandomToLoop`, Markov e random walk;
atribuir papéis distintos; renderizar trajetórias; só depois testar os mapas
de calma, inteligência e personalidade.

## Geradores multirrate isolados — 2026-09-24

Objetivo: evitar versões separadas do mesmo princípio em controle e áudio.
`FractalCore` cria até oito camadas autorreplicadas com razão, rugosidade e
frequência limitadas. `VectorGenerator` usa uma única curva como envelope,
LFO ou oscilador conforme relógio, frequência e modo one-shot/loop.

Arquivos: `src/core/FractalCore.{h,cpp}`,
`src/core/VectorGenerator.{h,cpp}`, `tests/MultirateGeneratorTests.cpp`,
`tests/RealtimeSafetyTests.cpp`, `CMakeLists.txt`,
`MULTIRATE_GENERATORS_DESIGN.md`, README, arquitetura e esta lista.

Validação: um primeiro teste expôs as fronteiras fase 0/1 e o encerramento de
one-shot infinitesimalmente antes de 1. A comparação passou a tratar fase como
circular e o gerador recebeu tolerância de wrap de `1e-12`. Depois dessas
correções, build Release e suíte 32/32 passaram em 41,26 s, incluindo heap.

Pendências: render, espectro, benchmark e escuta em registros distintos;
escolher uma ligação inicial ao Cérebro, Síntese ou cardume. Nenhuma rota do
aplicativo foi alterada.

## Percussão, morph harmônico e máscara de pulsos — 2026-09-24

Objetivo: materializar três hipóteses do Módulo 3 sem ampliar a rota ao vivo.
`SamplePercussionVoice` oferece quatro slots mono fixos, seleção, pitch,
decay e velocity. `HarmonicMorphOscillator` interpola até 16 coeficientes
antes da síntese aditiva. `StochasticPulseMask` decide somente no onset e
mantém a aceitação ou rejeição durante todo o pulsaret.

Arquivos: `src/core/SamplePercussionVoice.{h,cpp}`,
`src/core/HarmonicMorphOscillator.{h,cpp}`,
`src/core/StochasticPulseMask.{h,cpp}`, `tests/PercussionMorphTests.cpp`,
`tests/RealtimeSafetyTests.cpp`, `CMakeLists.txt`,
`PERCUSSION_AND_HARMONIC_MORPH_DESIGN.md`, README, arquitetura e esta lista.

Validação: os testes específicos e de heap passaram. O build Release completo
e a suíte 31/31 passaram em 54,52 s. Nenhum callback, espécie, parâmetro,
layout ou som do aplicativo foi alterado.

Pendências: renders, espectro, benchmark, escuta e função composicional;
proveniência/carregamento das amostras; famílias harmônicas e suavização;
fonte ecológica da probabilidade. A integração permanece bloqueada até esses
portões e o diagnóstico independente de estabilidade do app serem concluídos.

## Rotação, layout e curvas de deriva — 2026-09-24

Objetivo: fechar três lacunas estruturais sem ampliar a rota ao vivo.
`PatternRotationMutator` rotaciona até 32 bits a cada N ciclos preservando
pulsos. `StepMemoryLayout` traduz quatro trilhas entre 4 × 8, 2 × 16 e 1 × 32.
`DriftCurveShaper` separa suavidade degrau↔contínuo de seis famílias de curva.

Arquivos: `src/core/PatternRotationMutator.{h,cpp}`,
`src/core/StepMemoryLayout.{h,cpp}`, `src/core/DriftCurveShaper.{h,cpp}`,
`tests/GeometryEvolutionTests.cpp`, `tests/RealtimeSafetyTests.cpp`,
`CMakeLists.txt`, `GEOMETRY_EVOLUTION_DESIGN.md`, README, arquitetura e esta
lista.

Validação: build Release completo e suíte 30/30 passaram em 43,00 s, incluindo
`aquorbium_geometry_evolution_tests` e `aquorbium_realtime_tests`. Nenhum
callback, parâmetro ou layout do aplicativo foi alterado.

Pendências: origem dos ciclos, ligação à memória/sequenciador, mapeamento
performático, render, escuta e integração.

## Decisão ramificada, permutação e Relabi — 2026-09-24

Objetivo: completar mais duas formas de organização da Geometria e formalizar
um gerador temporal já ativo. `BranchingStepRouter` implementa árvore de duas
camadas com até 8 × 4 folhas. `NotePermutationMatrix` mantém 16 notas em pool e
oito ordens válidas. O teste próprio do `BiomaRelabi` agora cobre determinismo,
reset, variação, limites e taxa inválida.

Arquivos: `src/core/BranchingStepRouter.{h,cpp}`,
`src/core/NotePermutationMatrix.{h,cpp}`, `src/core/BiomaRelabi.h`,
`tests/DecisionPermutationTests.cpp`, `tests/RealtimeSafetyTests.cpp`,
`CMakeLists.txt`, `DECISION_AND_PERMUTATION_DESIGN.md`, README, arquitetura e
esta lista.

Validação: build Release completo e suíte 29/29 passaram em 38,56 s, incluindo
`aquorbium_decision_permutation_tests` e `aquorbium_realtime_tests`. Nenhum dos
componentes novos foi ligado à rota do aplicativo.

Pendências: semântica ecológica das folhas, integração ao quantizador,
seleção de linhas, escuta e possível uso do Relabi por `SchoolInfo`.

## Banco de padrões e Random to Loop isolados — 2026-09-24

Objetivo: acrescentar memória estrutural à Geometria sem tocar na rota ao
vivo. `StepPatternBank` guarda oito snapshots de até 32 passos e encadeia até
16 referências válidas. `RandomToLoop` guarda 1–64 valores e interpola entre
entrada renovada e ciclo fixo, atualizando gradualmente a memória nos estados
intermediários.

Arquivos: `src/core/StepPatternBank.{h,cpp}`,
`src/core/RandomToLoop.{h,cpp}`, `tests/PatternEvolutionTests.cpp`,
`tests/RealtimeSafetyTests.cpp`, `CMakeLists.txt`,
`PATTERN_EVOLUTION_DESIGN.md`, README, arquitetura e esta lista.

Validação: após um teste detectar que a interpolação introduzia
arredondamento nos extremos, `RandomToLoop` ganhou caminhos exatos para 0 e 1.
O build Release completo e a suíte 28/28 passaram depois da correção em
40,65 s, incluindo o teste de tempo real. Nenhuma rota do app foi alterada.

Pendências: persistência externa, nomes/versionamento, política de recuperação
por `AquariumIntegrity`, origem do sinal aleatório, render, escuta e integração.

## Navegação sazonal e por limiar isoladas — 2026-09-24

Objetivo: ampliar a Geometria sem conectá-la à rota ao vivo.
`AccumulatingTransposer` conserva um deslocamento em graus, acumula um passo
por evento, sobe na primeira metade da fase sazonal e desce na segunda, sempre
dentro de limites fixos. `ThresholdStepSelector` divide um sinal normalizado
em até 32 regiões configuráveis e informa índice, transição e direção do
cruzamento.

Arquivos: `src/core/AccumulatingTransposer.{h,cpp}`,
`src/core/ThresholdStepSelector.{h,cpp}`,
`tests/SequencerNavigationTests.cpp`, `tests/RealtimeSafetyTests.cpp`,
`CMakeLists.txt`, `SEQUENCER_MEMORY_AND_HARMONY_DESIGN.md`, README, arquitetura
e esta lista.

Validação: build Release completo com warnings como erro e suíte 27/27
passaram em 41,80 s. Isso inclui `aquorbium_sequencer_navigation_tests` e
`aquorbium_realtime_tests`, com os dois componentes sob contagem de heap. Não
houve alteração de parâmetros, callback, som ou layout do aplicativo.

Pendências: escolher qual fase sazonal alimenta o transpositor; ligar o índice
do seletor à memória; limitar a frequência das transições; renderizar e
escutar o resultado antes de qualquer integração.

## Memória endereçável e harmonia — protótipo isolado de 2026-09-24

Registro histórico do protótipo antes de sua integração em 27/09. O objetivo
era avançar a Geometria sem ampliar a rota ao vivo ainda sujeita a
escuta e diagnóstico. `AddressableStepMemory` mantém 32 passos fixos com
altura, duração de gate, viés de estratégia e desejo; permite leitura por
cursor ou CV direta sem uma leitura alterar a outra. `HarmonicProgression`
mantém 32 seções ponderadas, quantização de graus, nove qualidades e inversões
de acordes com três ou quatro vozes.

Arquivos: `src/core/AddressableStepMemory.{h,cpp}`,
`src/core/HarmonicProgression.{h,cpp}`, `tests/SequencerMemoryTests.cpp`,
`tests/RealtimeSafetyTests.cpp`, `CMakeLists.txt`,
`SEQUENCER_MEMORY_AND_HARMONY_DESIGN.md`, README, arquitetura e esta lista.

Validação daquele marco: build Release completo com warnings como erro e suíte 26/26
passaram em 39,81 s. Isso inclui `aquorbium_sequencer_memory_tests` e
`aquorbium_realtime_tests`, com os dois componentes sob contagem de heap.
Naquele momento, nenhum parâmetro, callback, som ou layout do aplicativo foi
alterado. A integração técnica posterior está registrada no início deste
documento.

Pendências atuais: edição detalhada da memória; `AccessPort`; modos paralelo,
serial e árvore; bancos de padrões; render musical dedicado e escuta no Bioma.

## Decisão, cauda, eventos e relógio isolados — 2026-09-23

Registro histórico daquele marco. A Cauda foi integrada em 25–26/09 e Markov
em 26–27/09; eventos e relógio ganharam rota experimental no banco de 28/09.

Objetivo: avançar uma memória de decisão e uma cauda espacial sem tocar na
rota ao vivo. `BiomaMarkov` contém oito estados, matriz normalizada, seed,
inércia, mutação e atração por contexto. `AquariumTail` contém quatro linhas
fixas de atraso, difusão Hadamard, amortecimento, decay até −60 dB e matriz
alternativa para deformação da cauda. Ambos permanecem independentes do JUCE,
da GUI, das espécies e do callback do aplicativo.

Arquivos: `src/core/BiomaMarkov.{h,cpp}`,
`src/core/AquariumTail.{h,cpp}`, `tests/MarkovTailTests.cpp`,
`tests/RealtimeSafetyTests.cpp`, `CMakeLists.txt`,
`BRAIN_DECISION_DESIGN.md`, `SYNTHETIC_TAIL_DESIGN.md`, README, arquitetura e
esta lista. O primeiro preflight bloqueou o build por 1,6 GiB/2 GiB de swap;
após limpeza manual, a swap ficou em 0 B e a validação pôde continuar.

Validação: `aquorbium_markov_tail_tests` e o teste de tempo real passaram;
build Release completo e suíte 25/25 passaram em 39,39 s. Isso aprova o core,
não a integração nem a escuta naquele marco. A Cauda ainda estava aberta como
função do Espaço; render e integração foram fechados em 25–26/09, enquanto
mapeamento ambiental e avaliação musical continuam pendentes. Para Markov, a
semântica de forma, integração e validação técnica foram fechadas em 26–27/09;
a escuta permanece pendente.

No mesmo incremento, `ControlEventExtractor` implementou cruzamentos de zero,
envoltória, gate e histerese; `FractionalLogicClock` implementou razões de
1/32 a 32/1 e combinações AND/OR/XOR. Arquivos adicionais:
`src/core/ControlEventExtractor.{h,cpp}`,
`src/core/FractionalLogicClock.{h,cpp}`, `tests/EventClockTests.cpp` e
`CONTROL_EVENTS_AND_CLOCK_DESIGN.md`. Testes cobrem 7/4, 5/4, divisão em
blocos, entradas inválidas e ausência de alocação. Naquele marco, nenhum dos
quatro módulos estava ligado ao app. Desde 28/09, eventos e relógio participam
de `CLOCK + EVENTS`.

Próximo passo histórico daquele marco: renderizar/escutar a cauda; escolher uma
única semântica inicial para Markov; definir fonte e destino dos eventos; só
então conectar o relógio ao `GeometricSequencer`, com reset e BPM do host
tratados à parte. A semântica Markov e sua fonte de evento já foram resolvidas
no marco de 26–27/09; a integração especializada do relógio ao sequenciador
principal permanece aberta, embora sua rota experimental esteja operável.

## Dois componentes isolados adicionais — 2026-09-22

Objetivo: avançar lacunas do Cérebro e da matéria granular sem ampliar o
callback do aplicativo enquanto estabilidade e escuta interativa continuam
abertas. `LorenzCV` integra um atrator de três estados por RK4 no relógio de
200 Hz, com parâmetros limitados, seed, saída suavizada e recuperação
numérica. `BiomaGranularCapture` guarda 65.536 amostras mono, detecta ataques
e toca até 12 grãos estéreo com velocidade, direção, pan, duração e ganho
congelados por grão. Entrada não finita vira silêncio; pool cheio conta
recusas. O dossiê específico é `GRANULAR_CAPTURE_DESIGN.md`.

Arquivos: `src/core/LorenzCV.{h,cpp}`,
`src/core/BiomaGranularCapture.{h,cpp}`, `tests/LorenzCaptureTests.cpp`,
`tests/RealtimeSafetyTests.cpp`, `tools/LorenzCaptureBenchmark.cpp`,
`CMakeLists.txt`, README, arquitetura, `GRANULAR_CAPTURE_DESIGN.md`,
`GRANULAR_STRIKER_DESIGN.md` e esta lista. Validação: build Release completo,
23/23 testes CTest (40,66 s), incluindo o teste de heap no processamento.
Benchmark isolado de um Lorenz + um granular, mediana de três trechos de
0,25 s: ocioso 0,083%/0,182% de um núcleo em 48/96 kHz; carga de 12 grãos
0,626%/1,063%. `sizeof(BiomaGranularCapture)` = 266.736 bytes; sob carga,
recusas contadas foram 312/720. Esse ensaio não inclui o resto do app nem
é um limite de CPU para qualquer máquina. Nenhum parâmetro de GUI, espécie
ou rota de áudio do app foi alterado. A autocaptura ainda depende de um sinal
fornecido manualmente ao objeto; não está ligada ao mix do aquário.

Pendências: benchmark CPU com o aplicativo e vários grãos, aliasing de pitch,
render e escuta A/B, escolha do ponto de captação, calibração de ganho,
evitar feedback acidental e teste manual prolongado de estabilidade.
Próximo passo: validar esses portões antes de qualquer integração sonora;
testar a utilidade musical do Lorenz contra o drift já ativo e o random walk.

## Protótipos isolados de módulos pendentes — 2026-09-21

Objetivo: avançar síntese FM/wavetable e deriva do Cérebro sem ampliar a rota
ao vivo enquanto travada prolongada, estalos e escuta dos controles permanecem
sem aprovação. `SynthFMVoice` usa tabela senoidal fixa de 2048 pontos com
interpolação linear para portador e modulador; a frequência instantânea do
portador aceita valor negativo e reverte a fase. `fold=0` conserva a onda
original; valores positivos interpolam para sine-shaping. `SlewRateRandomWalk`
gera alvos aleatórios em intervalos lentos, reflete nos limites ±1 e passa
por filtro de um polo abaixo de 2 Hz. Ambos têm seed/estado delimitados,
`tick()` sem alocação, trava ou I/O, e validação de valores não finitos.

Arquivos: `src/core/SynthFMVoice.{h,cpp}`,
`src/core/SlewRateRandomWalk.{h,cpp}`, `tests/PendingModulesTests.cpp`,
`tests/RealtimeSafetyTests.cpp`, `CMakeLists.txt`, arquitetura, README e esta
lista. Validação: build Release
completo aprovado e 22/22 testes CTest aprovados (40,63 s), incluindo teste
específico de reversão da fase FM, limites de saída, particionamento em blocos,
seed, continuidade e entradas inválidas. Depois, o teste de tempo real também
passou com os dois novos `tick()` sob contagem de heap (zero alocações), e o
teste específico foi repetido. Nenhuma espécie, parâmetro de GUI ou callback do
aplicativo foi conectado a esses protótipos. O ganho do fold e o espectro
em taxas/alturas extremas ainda não foram calibrados; não há oversampling,
análise de aliasing, escuta A/B ou ensaio de CPU no app. `setParameters()` do
random walk calcula coeficiente exponencial: chamá-lo fora do callback ou
precalcular controle antes de qualquer futura integração.

Próximo passo: render A/B offline e analisar aliasing/custo dos dois motores,
depois decidir se FM entra em um organismo e se a deriva substitui ou
complementa `BiomaBrain::nextDrift`. A integração ao app continua bloqueada
pelos portões de estabilidade e escuta manual descritos abaixo.

## Auditoria causal — 2026-09-21

Objetivo: explicar estalos, parada prolongada de áudio/janela e pouca resposta
audível dos controles sem alterar o DSP. Foram examinados código, testes,
evidências de 20/09, captura do usuário e recursos do host. Não houve nova
sessão de áudio ou escuta humana; os defeitos não estão declarados resolvidos.

- Prazo insuficiente na sessão fotografada: 48 kHz/16 amostras representam
  0,333 ms por callback; a tela registra XRUN 209 e LATE 198. A preferência
  real também guarda buffer 16. JUCE ALSA oferece 16, embora seu padrão seja
  512. É evidência forte para os estalos dessa sessão. CLIP 0 exclui somente
  blocos acima de 0 dBFS medidos pelo app naquele momento; não exclui
  descontinuidades abaixo do limiar nem problemas após a saída do app.
- A parada de minutos não tem causa localizada. `AUDIO WAITING` apenas indica
  que o contador de callbacks não avançou por cerca de 0,5 s. Processo e
  conexão PipeWire continuavam vivos, havia cerca de 16 GiB de RAM disponível
  e swap zerada. Não se obteve a pilha na falha. O JUCE/ALSA contém espera e
  escrita de dispositivo, mas não há prova de bloqueio nessas chamadas aqui.
  GUI 0 na imagem mede pausas >250 ms só até aquela atualização; não invalida
  a travada posterior relatada pelo usuário.
- A proteção criada após o incidente troca buffers salvos abaixo de 512 para
  512 somente quando o nome da saída contém `PipeWire`; ela foi observada em
  uma abertura de aproximadamente 8 s. A correção de uma seleção insegura
  durante a sessão acontece no timer da GUI, depois da abertura do dispositivo,
  sem teste de interação real. Um intervalo inicial em 16 e o bloqueio da GUI
  na reconfiguração são riscos a investigar, não causas provadas da falha
  anterior.
- Síntese e Espaço estavam ativados na imagem (`STRIKER MIX` 0,46,
  `SPACE MIX` 0,65, `RETURN` 0,37). Logo, os valores padrão zero não bastam
  para explicar a falta de efeito percebido. O Striker altera parâmetros de
  futuras excitações; no teste integrado, as diferenças entre extremos
  ficaram cerca de 45–51 dB abaixo do RMS da mistura de referência.
  `SPACE MIX` ficou cerca de 61 dB abaixo. O teste automatizado passa com
  diferença absoluta de RMS >1e-7, sem limiar perceptivo ou julgamento
  musical: comprova conexão numérica, não utilidade dos knobs. `RETURN` tem
  efeito maior, mas também não foi aprovado por escuta humana.
- A geometria do painel melhorou, mas o status usa fonte de 10 px e texto
  comprimido na faixa superior. Testes de contenção não validam legibilidade
  em DPI, gestos ou escuta real; os relatos visuais anteriores continuam
  pendentes de aprovação humana.
- A captura diagnóstica de 60 s depende de o callback atingir a duração ou
  de encerramento normal pedir finalização. Se os callbacks cessarem antes
  disso e o timeout externo encerrar o processo, WAV/relatório podem ficar
  incompletos. O teste unitário cobre término normal de 1 s, não essa falha.

Ordem de trabalho: (1) validar 48 kHz/512 na saída interna em sessão manual
prolongada, anotando buffer real e deltas dos contadores; (2) tornar a
captura recuperável se callbacks cessarem e registrar o ponto de bloqueio sem
operações no callback; (3) reproduzir troca de buffer com observação da GUI;
(4) fazer A/B integrado de Striker/Espaço com critério perceptivo antes de
recalibrar; (5) reavaliar legibilidade com DPI e gestos reais. Não ampliar
motores enquanto esses portões estiverem abertos. Arquivo afetado: esta
lista de tarefas. Validação desta auditoria: leitura cruzada de fontes e
evidências, RAM/swap conferidas; sem build ou teste novo. Próximo passo:
reproduzir com diagnóstico que preserve dados mesmo quando o áudio parar.

## Correções implementadas — 2026-09-21

Objetivo: impedir a seleção de 16 amostras no PipeWire, conservar evidência
se o callback parar e aumentar a presença mensurável de Síntese/Espaço sem
alterar o balanço autônomo em zero. Arquivos: `src/app/Main.cpp`,
`src/app/DiagnosticAudioCapture.h`, `src/app/AudioControlFrame.h`,
`src/core/BiomaEngine.{h,cpp}`, testes e documentação local.

- O painel de ÁUDIO oferece 512/1024/2048 na saída PipeWire e 128/256
  adicionais em hardware direto. Preferência PipeWire antiga abaixo de 512
  ainda é normalizada antes da abertura. Removida a reconfiguração periódica
  pelo timer da GUI. O status é mostrado em duas linhas e o tooltip inclui
  callbacks iniciados/concluídos para separar parada de entrega de parada
  dentro do render.
- A captura WAV fecha por tempo de parede após duração solicitada + 2 s,
  inclusive quando não recebe novos callbacks. Antes de começar, cria um
  `REPORT.txt` inicial com saída, taxa e buffer; se a GUI conseguir terminar,
  substitui-o por métricas e indica `wall_deadline_reached`. O teste unitário
  verifica um WAV parcial válido após o callback parar.
- `STRIKER MIX` também escala somente o Striker entre 1 e 8 vezes, com rampa
  de 15 ms. `SPACE MIX` também aumenta o retorno molhado até 2 vezes, sempre
  com o limite final de 0,75. Os valores zero mantêm o comportamento anterior.
  A/B numérico integrado de quatro segundos: os quatro controles Striker
  ficaram entre −27 e −33 dB (antes cerca de −45 a −51 dB); `SPACE MIX`
  ficou em −21 dB (antes cerca de −61 dB), relativamente ao RMS da referência.
  O teste agora exige presença mínima de −35 dB para Striker, −25 dB para
  `SPACE MIX`, além de pico abaixo de 0,8 no cenário de cada controle e
  abaixo de 0,95 num cenário de quatro vozes (pico medido `0,535761`).
  Isso não é limiar auditivo.
- Validação: build Release atualizado; 21/21 CTest após as mudanças. Auditoria
  de layout isolada em `validation-output/2026-09-21-layout-fix-01/` gerou
  12 capturas (1920×1006, 1280×800, 1024×650, 960×600), com mínimos dos
  knobs e separação de áreas aprovados pelo verificador; inspeção visual
  pontual de 1280×800 e 960×600 mostrou status e cartões legíveis. Essa
  auditoria não abriu dispositivo de áudio. Build e capturas são locais;
  o destino de backup (um disco interno de 2 TB) estava `ro` e não foi usado.
- Ensaio real automatizado com o binário final na saída interna padrão ALSA
  via PipeWire: `validation-output/2026-09-21-runtime-pipewire-01/REPORT.txt`.
  Em 20,019 s de relógio e 19,840 s de áudio, 48 kHz/512 amostras,
  1.860 callbacks, XRUN 0, LATE 0, CLIP 0, pausas GUI >250 ms 0,
  amostras inválidas 0, pico 0,151536 e RMS 0,028902. O protocolo foi
  programático, sem escuta ou gesto físico. Binário Release usado:
  `build-reactivation-20260915/src/app/AquorbiumApp_artefacts/Release/Aquorbium`,
  SHA-256 `eb657ae9f46b4cb8fe6aebf65d66e1ee8204e8efacb9479baa5f38057034e3ad`;
  recompilado em 21/09 às 01:18 (Europe/Paris). Nenhuma instância ficou aberta.

Uma execução isolada do teste de resposta excedeu o timeout de 65 s sem
resultado; não se atribuiu causa ao DSP. A suíte já havia concluído o mesmo
teste, e um reteste com saída por linha terminou com código zero e pico de
quatro vozes `0,535761`. Registrar essa variabilidade de duração; não a
confundir com reprodução da travada do app.

Pendente: nenhuma dessas verificações reproduziu a travada de minutos nem
prova ausência de estalos em escuta. Próximo ensaio deve usar `./run` com
saída interna real, 48 kHz/512, observando contadores sem trocar vários fatores
ao mesmo tempo; depois, `./run diagnostico` para comparar o WAV pré-dispositivo
com o ouvido. Se a janela parar, o WAV parcial deve terminar sozinho e o
relatório inicial permanecer, mas a pilha do processo continua não capturada.
Também faltam escuta A/B dos controles, DPI real e teste de troca de saída.

## Fila de retomada

Diagnóstico visual de 20/09 às 23:00: a captura enviada pelo usuário mostra
`AUDIO ONLINE`, 48,0 kHz, buffer de 16 amostras, XRUN 209, LATE 198,
CLIP 0 e GUI 0. A 48 kHz, 16 amostras dão apenas 0,333 ms por bloco; os
estalos relatados são compatíveis com a falta de prazo/underrun, não com
clipping digital medido nessa tela. O usuário havia mudado para 48 kHz:
funcionou por algum tempo, parou em `AUDIO WAITING` e retomou minutos depois.
O status com buffer 16 explica por que os ensaios automáticos em 512 amostras
não reproduziram a falha. A causa de ter sido selecionado 16 ainda precisa
ser separada da resposta do backend. Orientação imediata: mudar em `ÁUDIO`
para 512 amostras (1024 se ainda houver estalos), mantendo 48 kHz, e observar
novos incrementos de XRUN/LATE; os contadores já acumulados não zeram só por
mudar o buffer. Após o usuário fechar a instância travada, a proteção para
restaurar/retificar buffers inferiores a 512 no dispositivo PipeWire foi
compilada no `./run` atual. Build Release de uma tarefa e suíte 21/21
passaram. Reabertura breve com preferências isoladas que pediam 16 amostras
mostrou 48 kHz/512, XRUN/LATE/CLIP/GUI 0. Evidência em
`validation-output/2026-09-20-buffer-safety-restore-01/` (XML de entrada,
captura da janela e relatório). O timeout externo encerrou a abertura curta
com código 124; isso não é aprovação perceptiva. Ainda faltam ensaio manual
longo, observação da troca de buffer durante a sessão e verificação dos
controles Síntese/Espaço, cuja audibilidade continua insatisfatória.

Esclarecimento do usuário em 20/09: os estalos ocorreram usando a saída de
áudio interno, e a travada afetou a aplicação inteira, inclusive a janela.
Não era uma escuta pela AudioBox nem apenas uma interrupção do som. Na sessão
posteriormente fotografada, o app usava a saída padrão ALSA via PipeWire a
48 kHz/16 amostras; os parâmetros da primeira ocorrência permanecem
desconhecidos. A próxima reprodução deve começar pelo áudio interno e registrar
os dados exatos;
os ensaios automáticos anteriores sem falha não substituem esse cenário.

Andamento de 20/09: o bloqueio de recursos foi removido. Após verificar
16 GiB disponíveis e identificar apenas `/swapfile`, a swap foi esvaziada e
reativada no host; a leitura final manteve 2 GiB ativos e 0 B em uso, sem
desativar a proteção. O destino de backup (o disco interno de 2 TB) apareceu
`ro`, mas não foi usado nem modificado nesta tarefa. A captura diagnóstica
opt-in foi conectada ao app: WAV float32 estéreo, fila SPSC fixa de 131.072
quadros, thread de escrita separada, limite de 120 s, saída exata e
preferências isoladas. A suíte inteira passou em 21/21 testes. Um smoke de
3 s na saída interna padrão ALSA via PipeWire produziu 144.000 quadros a
48 kHz/512, sem perda de fila, erro de escrita, XRUN, LATE, CLIP ou pausa
GUI >250 ms. Evidência em
`validation-output/2026-09-20-capture-internal-smoke-01/`. O app permaneceu
aberto após gravar; o timeout externo de 12 s o encerrou (código 124),
portanto esse código não significa falha da captura. Não houve escuta humana,
interação física nem reprodução da travada/estalos. Próximo passo: sessão
interativa de até 120 s com a mesma saída interna, anotando instante e gesto
dos incidentes; comparar o WAV pré-dispositivo com o que foi ouvido. Se o
estalo não estiver no WAV, investigar a cadeia PipeWire/ALSA/dispositivo.
Para escuta manual simples, `./run` abre o build Release sem diagnóstico;
o atalho foi acrescentado a pedido do usuário e não modifica o áudio.

Escuta manual de 20/09 (incidente reproduzido pelo usuário): estalos e
"clipes" audíveis persistiram; Síntese/Striker e Espaço não produziram
diferença perceptível; após selecionar 48 kHz o áudio funcionou por algum
tempo, depois travou e o cabeçalho mostrou `AUDIO WAITING`. Consulta no host
durante o incidente encontrou o processo Aquorbium ainda vivo e uma entrada
PipeWire ALSA `[Aquorbium]` estéreo float32/48 kHz, sem mute/cork; PipeWire e
WirePlumber continuavam ativos. Havia ~16 GiB de RAM disponível e 0 B de
swap em uso. Logs recentes do PipeWire/WirePlumber não mostraram erro
relacionado; a pilha do processo não pôde ser capturada porque o attach do
`gdb` foi recusado pelo `ptrace_scope`. A tela `AUDIO WAITING` indica que o
contador de callbacks do app deixou de avançar por pelo menos ~0,5 s, mas
não identifica sozinha se ALSA, PipeWire ou outro caminho bloqueou a entrega.
O usuário relatou que a aplicação só voltou a responder após vários minutos,
sem reset nosso; isso favorece interrupção temporária, não desconexão
definitiva. Não foi feito reset do dispositivo, encerramento do app nem
modificação do DSP.
Próximo passo: preservar a situação exata (buffer e contadores no cabeçalho),
fechar normalmente se possível, e reproduzir em sessão com WAV/relatório
manual; comparar depois uma segunda rota sem mudar múltiplos fatores ao
mesmo tempo. O teste de resposta numérico ainda mostra Striker em −44,67 a
−51,03 dB sob a mistura e `SPACE MIX` em −61,31 dB; não marcar esses controles
como perceptivamente aprovados.
Foi acrescentado `./run diagnostico` como atalho explícito para a próxima
escuta manual de 60 s no áudio interno, com sessão única e limite externo
de 75 s. `./run` sozinho continua abrindo o modo normal sem captura.

O teste humano de 18/09 ainda relatou estalos, travada da aplicação, controles
sem efeito percebido, knobs pequenos e sobreposição de textos/objetos. A
captura de 00:16:27 confirma os problemas visuais. Prioridade é corrigir e
retestar áudio/GUI; não ampliar módulos ou transporte de cenas enquanto este
marco estiver rejeitado. A metodologia permanente de layout está em
`WINDOW_LAYOUT_DESIGN.md`. Medição técnica não equivale a aprovação perceptiva.

1. Confirmar a execução do app standalone recém-compilado no Linux e o
   dispositivo de áudio disponível. O core passou em 14/14 testes na build
   de reativação; parâmetros elevaram a suíte a 15 testes e a ponte de quadros
   acrescentou o 16º teste; transporte/recuperação ponta a ponta acrescentou
   o 17º teste; regressão de Lupa/mistura suavizada acrescentou o 18º teste;
   nomes/desfazer/migração do banco acrescentaram o 19º teste; a auditoria
   de resposta dos 24 controles acrescenta o 20º teste.
   O candidato com diagnóstico também foi executado no host, em dois ensaios
   reais de 20 s (AudioBox direta e saída padrão via PipeWire). Ambos terminaram
   sem os incidentes medidos; isso não equivale a aprovação perceptiva ou a
   reprodução/resolução da travada relatada. Registro da execução abaixo.
2. Reescutar primeiro o app corrigido de 18/09: escolher saída/buffer em ÁUDIO,
   registrar dispositivo, CPU/XRUN/LATE/CLIP/GUI, tempo até eventual travada da GUI
   e controles envolvidos. Tentar 48 kHz/512 amostras como diagnóstico, não
   garantia. Testar STRIKER MIX/SPACE MIX e GEOMETRY PERFORMANCE acima de zero;
   seus controles dependentes ficam inativos em zero e vários respondem somente
   nos próximos eventos. Depois, usar como comparações históricas os pares
   `2026-09-17-lens-{equal,solo}-ab`, registrando pressão/movimento/soltura e
   eventuais estalos. Eles usam alvos móveis, posse de 2 a 12 s e novo caminho
   de mistura; não comparar épocas como se fossem um mesmo DSP. Depois,
   ouvir e registrar os pares históricos com seed e parâmetros iguais: Espaço
   ligado/desligado, Ecossistema `Active`/`Neutral` e as quatro perspectivas
   individuais contra pesos iguais. Comparar presença dos cinco organismos,
   primeiro plano, sustentação, profundidade e mascaramento. Em seguida,
   calibrar retorno inicial `0,36`, os seis controles do Espaço e a Lupa.
3. Validar na janela real o gesto da Lupa, os seis controles ecológicos e as
   quatro cenas locais e os diálogos de transporte já integrados; escolher e
   testar uma consequência sonora para
   `AquariumIntegrity::shattered` somente após as decisões de escuta.
4. Selecionar uma hipótese do banco de pesquisa para cada próximo experimento
   musical. Candidatos já estruturados: Brain/deriva de longo prazo,
   BiomaBass, FM/wavefolder, granular de autocaptura e cauda do Espaço.
   Registrar problema musical, protótipo isolado, medições, escuta e decisão
   antes de marcar a implementação como concluída.
5. Ampliar interface, persistência, Master, saída técnica e entrega quando os
   motores correspondentes estiverem prontos. Validar tempo real, janela/DPI
   e dispositivo em cada marco; preparar Windows/macOS e publicação apenas
   com builds e testes próprios dessas plataformas.
6. Suspenso pela prioridade de bugs: nomes de até 32 caracteres e desfazer a
   última alteração do banco estão integrados, assim como transporte de uma
   cena. Próxima ampliação possível: transporte do banco inteiro com nomes,
   limite de 64 KiB, confirmação e backup antes de substituir. Manter
   validação transacional, arquivos/ValueTree fora do áudio e não confundir
   cenas de parâmetros com cenas comportamentais.

### Execução de 2026-09-18 — diagnóstico limitado no host

- Objetivo: medir o app durante controles/Lupa ativos, sem alterar os motores,
  balanço autoral, layout ou Rasgo Modular.
- Código: `src/app/Main.cpp` acrescenta `--runtime-audit` e
  `--runtime-output`. Diretório absoluto novo, preferências próprias, sem
  sobrescrita de sessão anterior; escolha exata da saída e recusa de fallback.
  Relatório final fora do callback, após fechamento do áudio. Sem WAV gerado.
- Protocolo de 20 s: 0–4 autonomia inicial; 4–9 macros e rolagem;
  9–14 geometria/páginas; 14–18 posse/movimento da Lupa; 18–20 liberação.
  Até 96 leituras em array fixo; não recebe MIDI externo. Usar timeout de 35 s
  para limitar inclusive travada que impeça o timer de terminar.
- Recusa de dispositivo inexistente testada: retorno 1 e enumeração real em
  `validation-output/2026-09-18-runtime-device-check-01/REPORT.txt`, sem áudio.
- AudioBox USB direta, backend ALSA: 48 kHz/512, 20,000 s de áudio,
  1.875 callbacks; XRUN/LATE/CLIP/GUI >250 ms/overflow/não finitos todos zero.
  Pico 0,12140583, RMS 0,02750742, delta máximo 0,06812768. CPU JUCE nas
  leituras periódicas: média 5,77%, máximo amostrado 11,93% (não pior deadline).
  Evidência: `validation-output/2026-09-18-runtime-audiobox-01/REPORT.txt`.
- Saída padrão ALSA via PipeWire: 48 kHz/512, 20,000 s de áudio,
  1.875 callbacks; mesmos contadores zerados. Pico 0,12216772,
  RMS 0,02781954, delta máximo 0,06855673. CPU JUCE amostrada: média 6,20%,
  máximo 15,74%. Evidência:
  `validation-output/2026-09-18-runtime-pipewire-01/REPORT.txt`.
- Limitações: ensaios curtos/programáticos, não escuta nem mouse/MIDI físico.
  Pequenas diferenças do relógio GUI impedem comparação bit a bit entre rotas.
  Delta não identifica cliques; XRUN zero não prova ausência de estalos.
  A travada não se reproduziu e sua causa continua desconhecida. Retorno zero
  do modo indica coleta completa com amostras finitas, não aprovação artística.
- Validação: build Release do app com uma tarefa/timeout 240 s; 20/20 CTest
  repetidos em 34,23 s. Relatórios ignorados pelo Git; auditoria geométrica
  anterior permanece evidência do layout inalterado, sem nova aprovação DPI.
- Candidato atual: `build-reactivation-20260915/src/app/AquorbiumApp_artefacts/Release/Aquorbium`;
  SHA-256 `243f65e05b569841a8bda74588c8797d4cb9081ad4d9b4258d570ee879a15e93`.
  Substitui o candidato sem este modo registrado nas execuções anteriores.
- Recursos: preflight host com ~12 GiB disponíveis, swap 13,8 MiB;
  conferência posterior com ~14 GiB disponíveis, swap baixa. Nenhuma escrita
  no backup, nenhuma alteração de volume/roteamento global.
- Próximo foco: ensaio prolongado/interativo no candidato atual para localizar
  estalo/travada e avaliar mascaramento dos macros; não declarar bugs resolvidos
  nem retomar novos motores/transporte do banco. Sem pull, push ou commit.

### Execução de 2026-09-19 — perfil de cenas, autonomia e vozes

- `RuntimeAuditProtocol.h` fixa e testa os dois perfis. O modo normal continua
  em 20 s; `--runtime-stress` usa 60 s e exige `--runtime-audit`. Tempos
  negativos, não finitos ou enormes terminam com segurança antes de conversão.
- Extensão do protocolo: 20–35 s alternam duas cenas locais isoladas;
  35–45 s alternam autonomia; 45–58 s geram quatro vozes/até oito eventos por
  quarto de segundo; 58–60 s liberam as notas. Limite: 420 eventos gerados e
  256 linhas de diagnóstico, sem MIDI físico ou I/O no callback. O seletor de
  áudio fica inativo até o fim da execução.
- AudioBox USB direta: 60,021 s, 5.627 callbacks, 15 recalls, 10 trocas de
  autonomia e 420 eventos; XRUN/LATE/CLIP/GUI >250 ms/overflow/não finitos
  todos zero. Pico 0,26209140; RMS 0,03554721; delta 0,10689682. CPU JUCE
  amostrada: média 6,22%, máximo 29,19%. Evidência local:
  `validation-output/2026-09-18-runtime-stress-audiobox-01/REPORT.txt`
  (iniciada na sessão de 18/09 e encerrada depois da mudança de data).
- ALSA via PipeWire: 60,000 s, 5.625 callbacks e mesmas ações/contadores
  zerados. Pico 0,27142188; RMS 0,03501456; delta 0,11249623. CPU JUCE
  amostrada: média 6,24%, máximo 25,14%. Evidência local:
  `validation-output/2026-09-19-runtime-stress-pipewire-01/REPORT.txt`.
- Os testes dos 24 controles agora informam diferença relativa ao RMS de
  referência. Na mistura, MATTER/POP/SCATTER ficaram em −51,03/−49,14/−44,67
  dB. Com somente a espécie Striker na saída técnica, a diferença em relação
  à sua própria referência foi +11,59/+3,94/+11,44 dB, mas a referência
  isolada era muito baixa. Conclusão limitada: as rotas funcionam; fonte
  discreta/mascaramento são hipóteses fortes. Não houve mudança de ganho.
- Build Release com uma tarefa aprovado; 20/20 CTest em 40,12 s, incluindo
  fases/extremos do protocolo e Striker isolado. Candidato SHA-256
  `a01207d96dcf593833286a4d80501fd65a5f2c7804258a7072c4530a7e74f30d`.
  Relatórios continuam ignorados pelo Git. Memória disponível ~14 GiB; swap
  627 MiB, abaixo de 50% e estável durante a rodada.
- Resultado: nenhum travamento reproduzido nos dois perfis. Isso reduz a
  probabilidade de falha imediata sob essas ações, mas não resolve o relato
  humano. Próximo passo continua sendo escuta interativa/prolongada, anotando
  instante, controle e dispositivo. Sem pull, push ou commit; Rasgo Modular
  não foi alterado.

### Ponto de retomada de 2026-09-18 — revisão documental

- Estado: candidato corrigido, ainda sem aprovação de estabilidade ou escuta.
  Estalos e travada da aplicação continuam pendentes; sua causa não foi
  comprovada. Não marcar esses bugs como resolvidos por redução de CPU.
- Conferência desta revisão: checksum do executável coincide com o registro
  abaixo; o relatório de `2026-09-18-panel-audit-03/` registra aprovação
  geométrica nas quatro dimensões. Os 20/20 testes são da execução anterior,
  não foram repetidos nesta revisão exclusivamente documental.
- Documentação corrigida: README passa a citar 20 testes, build com uma tarefa
  e timeout, além do comando do candidato existente. Painel mestre passa a
  indicar desenvolvimento local, não apenas ideia/especificação. Contagem
  de tarefas permanece 92 concluídas e 179 abertas.
- Próxima tarefa única: ensaio limitado do standalone com dispositivo real,
  confirmando versão/instância, saída, taxa e buffer. Registrar CPU, XRUN,
  LATE, CLIP e GUI antes/depois das alterações de controles e da Lupa; anotar
  tempo e gesto associados a estalo ou travada. Esses contadores não
  distinguem sozinhos transiente musical, dropout e bloqueio da interface.
- Depois: avaliar audibilidade dos controles do Striker/Espaço, separando
  influência manual zero, resposta no próximo evento e mascaramento entre
  organismos. Não aumentar ganhos automaticamente para aparentar resposta.
- Permanecem abertas a interação real/rolagem, DPI e calibração musical.
  Novos motores e transporte do banco inteiro continuam suspensos.
- Nenhum código foi alterado nesta revisão; nenhum teste com áudio real foi
  iniciado. Sem pull, push ou commit e sem alterações no Rasgo Modular.

### Execução de 2026-09-18 — correção após teste humano

- Objetivo: reparar layout, explicitar dependências e reduzir trabalho de
  áudio, sem mudar Rasgo Modular ou criar novos motores.
- `BiomaEngine.cpp`: coeficientes dos espacializadores passam de atualização
  a cada amostra para 200 Hz no relógio de amostras. A suavização acústica
  continua por amostra. O cache era invalidado continuamente por `audioMix()`.
- DENSITY/DRIFT: dois estados sonoros recebem suavização de 15 ms, antes
  ausente na rota direta de amplitude/forma/duty; alvos e desejo ecológico
  permanecem separados. Teste do primeiro passo em 44,1/48/96 kHz.
- `Main.cpp`: ScopedNoDenormals, telemetria limitada aproximadamente a 30 Hz,
  diagnóstico atômico de orçamento/clipping, seletor não modal de saída/buffer
  e contador de intervalos GUI >250 ms, com preferências do dispositivo
  separadas das cenas. Não foi acrescentado
  limiter nem atribuída uma causa comprovada à travada da GUI.
- Dependências visíveis: STRIKER MIX (antes DEPTH), SPACE MIX (antes GESTURE)
  e GEOMETRY PERFORMANCE ativam somente seus controles dependentes. Zero
  conserva autonomia; nenhum preset é alterado para forçar efeito sonoro.
- `MacroPanelLayout.h`: knobs ≥52 px, hit areas ≥80×92 px, rótulos de 11 px
  e valores em áreas separadas. A coluna de cartões rola em vez de comprimir
  controles. Cartões embutidos não pintam descrição atrás dos widgets.
- `OrganismLabelLayout.h`: busca finita evita legendas sobrepostas entre si
  e com halos/Lupa, sem alterar posições ecológicas. Fonte 11 px e conectores.
- `WindowLayoutPolicyTests.cpp`: mínimos e separação, além de organismos
  coincidentes/nas bordas. `ControlResponseTests.cpp`: mede diferença de áudio
  pelo mapeamento real de 24 controles, com influência ativa e neutralidade dos
  três gates em zero. Rotação é cíclica; geometria lenta precisa de vários
  eventos. Diferença numérica não certifica influência musical satisfatória.
- `--layout-audit` gera capturas/relatório sem áudio e com preferências
  isoladas. A primeira auditoria de quatro dimensões passou; a inspeção das
  imagens encontrou ainda colisões de legendas e levou ao helper acima.
  Preservar `validation-output/2026-09-18-panel-audit-01/` como diagnóstico,
  não como aprovação da revisão final.
- Segurança: uma tarefa por vez, builds com --parallel 1/timeout 240 s,
  memória disponível entre 12 e 14 GiB e swap baixa. Backup sem escrita.
  Diagnóstico no host encontrou PipeWire 1.0.5, saída padrão de áudio interno
  a 48 kHz e AudioBox presente. Isso não prova qual saída o app usava no teste.
- Documentos afetados: README, contratos de layout/tempo real, desenhos do
  Striker/Espaço e esta fila. Nova escuta e reprodução da travada permanecem abertas.
- Validação em 18/09: build Release completa aprovada; 20/20 CTest em 35,59 s,
  incluindo zero alocações no caminho de áudio, independência dos blocos,
  resposta dos 24 controles e três grupos neutros em influência zero. No
  cenário misturado, alguns macros do Striker/Espaço produzem diferença muito
  discreta; medir diferença não significa resolver mascaramento perceptivo.
- Benchmark comparativo, mesmo comando/configurações (45 casos; cinco
  medições de 0,5 s por caso, após aquecimento de 0,25 s): em 48 kHz/512
  amostras/quatro vozes externas + autonomia, custo relativo à duração do
  áudio caiu de 19,83% para 7,55%; em 96 kHz/128/quatro vozes, de 38,82%
  para 14,63%. Maior custo novo entre os 45 casos: 14,88%. Não inclui GUI,
  backend/dispositivo, pior deadline por callback ou interferência externa;
  não é prova de eliminação de xruns, cliques ou travadas.
- Auditoria gráfica offline da revisão em
  `validation-output/2026-09-18-panel-audit-02/`: 12 capturas e relatório
  aprovados em 1920×1006, 1280×800, 1024×650 e 960×600; inspeção de imagens
  nos três modos confirmou maiores knobs e áreas separadas. A inspeção ainda
  encontrou legenda sobre a Lupa; revisão final adicionou Lupa aos obstáculos.
  Build do app e teste de layout repetidos com sucesso. Auditoria final em
  `validation-output/2026-09-18-panel-audit-03/` passou nas quatro dimensões;
  inspeção de capturas confirmou legendas separadas da Lupa. Capturas são
  offline, sem áudio; não certificam DPI, mouse nem estabilidade ao vivo.
  Não sobrescrever evidências anteriores.
- Conferência final depois da correção da Lupa: 20/20 CTest novamente
  aprovados em 34,60 s; `git diff --check -- AQUORBIUM`, executado na raiz
  do repositório RASGO, passou. Evidências de auditoria estão ignoradas pelo Git.
- Binário final em `build-reactivation-20260915/src/app/AquorbiumApp_artefacts/Release/Aquorbium`;
  SHA-256 `6e2e03a4d0a5ad6c4de10cc9f7d0d1377f81cab955a7c78a4138af809df5de03`.
  Conferência em 18/09 às 01:32 CEST: cerca de 14 GiB disponíveis, swap 12 MiB;
  nenhuma escrita no backup e nenhum arquivo fora do Aquorbium alterado nesta rodada.
- Próximo passo: reabrir o binário atualizado, ensaiar os controles e obter
  captura do cabeçalho/tooltip ÁUDIO caso haja estalo ou GUI travada. Confirmar
  dispositivo e tamanho de bloco reais. Não retomar expansão de cenas/motores
  antes de classificar esse retorno. Não foi feito pull, push ou commit.

### Execução de 2026-09-15

- Build Release do core e do app standalone Linux concluída em
  `build-reactivation-20260915`; 15/15 testes passaram após a camada de
  parâmetros ecológicos.
- Lupa ligada ao gesto de pressionar/arrastar na esfera, com `Alt` para a face
  posterior. Posição, alvo e posse vêm da telemetria; mouse e áudio se comunicam
  apenas por atomics lock-free.
- Página `ECO CONTROLS` adicionada ao centro: Cohesion, Danger e quatro pesos
  de escuta. Density/Drift continuam nos controles inferiores. Os seis novos
  sliders persistem nas preferências do app e chegam ao core no callback.
- `EcologyParameterState` versionado em `ValueTree` reúne Density, Calm,
  Cohesion, Danger e quatro pesos de escuta na thread da GUI, importa as chaves
  anteriores e salva snapshot XML sem mudar o core. Teste próprio de
  round-trip, limites e rejeição de estado inválido; build do app e 15/15
  testes passaram. APVTS e snapshot dos demais controles continuam pendentes.
- Preflight antes do build: aproximadamente 12 GiB de memória disponível,
  swap praticamente livre e 1 TiB livre no disco de trabalho; build com
  `--parallel 2` e timeout. O volume de backup apareceu `ro` nesta namespace,
  mas não recebeu escrita. Não foi possível verificar processos do host fora
  da namespace do sandbox.
- Renderes novos de 30 s/48 kHz em
  `validation-output/2026-09-15-reactivation-space-{on,off}/` reproduziram
  exatamente os WAVs antigos correspondentes (SHA-256 igual em cada par).
- Quatro renders cegos de perspectiva criados em
  `validation-output/2026-09-15-perspective-{sparse,energy-memory,geometric-cycle,collective-drift}-ab/`:
  cada observador isolado contra quatro pesos iguais, ambos na rota Active,
  20 s/48 kHz e RMS igualado. As quatro referências são bit a bit idênticas;
  os focos diferem. Chaves ainda não reveladas. O comando recusa sobrescrever
  uma sessão existente e `--reveal` só cria a chave.
- A/B atual do Ecossistema criado em
  `validation-output/2026-09-15-ecosystem-ab-regression/`. O A/B de agosto
  permanece histórico: RMS neutro/ativo antes da igualação
  `0,0110563`/`0,00343291` contra `0,0519969`/`0,0187339` no render atual.
  Não misturar épocas na avaliação cega; a diferença não isola uma causa.
- Permanecem pendentes: abrir o app no sistema anfitrião, testar o gesto e o
  layout visualmente, ouvir os pares A/B, calibrar Lupa/Espaço e ampliar o
  protocolo de presets ao vivo/APVTS futuro. Não houve acesso a tela nem
  dispositivo de áudio reais no sandbox.

### Execução de 2026-09-16 — estado completo do standalone

- `PerformanceParameterState` V1 agrega o estado ecológico e os 16 controles
  contínuos restantes, mais ativação autônoma: 25 valores persistentes.
  Implementação em `src/app/PerformanceParameterState.h`, integração em
  `src/app/Main.cpp` e declaração do header em `src/app/CMakeLists.txt`.
- Restauração valida todo o snapshot antes de aplicá-lo. Se ele não existe ou
  é inválido, recupera o `ValueTree` ecológico antigo e as chaves individuais.
  Ambos os formatos antigos continuam salvos. Gate e Lupa ficam transitórios.
- Sliders e alvos atômicos recebem o mesmo valor quantizado. XML/ValueTree
  permanecem na GUI; não houve mudança nos motores DSP nem nos arquivos dos
  pares de escuta.
- Testes ampliados em `tests/EcologyParameterStateTests.cpp`: round-trip XML
  dos 25 valores, cópia independente, rejeição de campos ausentes, números
  inválidos, texto com sufixo e ativação fora de 0/1. APVTS e troca de presets
  coerente durante áudio ativo não foram implementados.
- Build limitado a uma tarefa concorrente e timeout de 120 s em
  `build-reactivation-20260915`; aproximadamente 16 GiB disponíveis e swap
  quase livre no preflight. Nenhuma escrita no backup, que aparece `ro` na
  namespace. Processos externos à namespace não foram verificáveis.
- Validação final em 2026-09-16, 20:36 CEST: build de `AquorbiumApp` e
  `aquorbium_ecology_parameter_tests` concluído sem novos warnings;
  `timeout 90 ctest --test-dir build-reactivation-20260915 --output-on-failure`
  aprovou 15/15 em aproximadamente 4,8 s. `git diff --check -- AQUORBIUM`
  passou; memória disponível permaneceu em aproximadamente 16 GiB.
- Próximo passo de código: protocolo de captura/publicação de presets ao vivo,
  sem modificar a ecologia nem acoplar o core ao JUCE. Escuta, ensaio visual e
  recuperação de preferências numa janela real continuam pendentes.

### Execução de 2026-09-16 — quadros coerentes e quatro cenas locais

- Objetivo: permitir guardar/recuperar a performance sem o callback receber
  parâmetros de duas cenas misturados, sem I/O no áudio nem reset dos motores.
- `src/core/FixedLatestSnapshot.h`: ponte SPSC de três buffers fixos, com
  transferência de propriedade por índice atômico acquire/release. Uma GUI
  produtora e um consumidor áudio; sem loops de retry, locks ou alocações.
  Quadros intermediários podem ser substituídos pelo estado mais recente.
- `src/app/AudioControlFrame.h` concentra o mapeamento dos 25 valores
  persistentes e do gesto transitório da Lupa no core. O app publica um quadro
  após cada alteração; recuperar uma cena atualiza todos os widgets sem
  notificações intermediárias e publica apenas uma vez.
- Setters do motor são aplicados somente quando há quadro novo, além da
  preparação do dispositivo. O teste de controles estáveis compara aplicação
  única contra reaplicação por bloco e exige igualdade bit a bit no cenário
  testado. Preparação passa também a aplicar os controles do Espaço.
- `src/app/PerformancePresetBank.h`: quatro cenas persistidas nas preferências
  locais, cópias independentes, restauração transacional e recusa de banco
  inválido. `ECO CONTROLS` ganhou seletor, GRAVAR, RECUPERAR, INICIAL e status.
  Banco inválido nas preferências é preservado até uma gravação explícita.
- Migração centralizada em `PerformanceParameterState::loadPreferences`:
  snapshot completo válido → snapshot ecológico válido → chaves individuais.
  XML limitado a 4 Ki caracteres ecológicos, 16 Ki do estado e 64 Ki do banco;
  texto numérico malformado não vira silenciosamente zero.
- `src/app/EcologyPanelLayout.h`: geometria pura das seis linhas e rodapé de
  cenas. `WindowLayoutPolicyTests.cpp` verifica contenção e separação em três
  painéis representativos; isso não certifica tipografia ou gestos na GUI real.
- Testes novos/ampliados: `FixedLatestSnapshotTests.cpp` (100 mil publicações
  concorrentes), `RealtimeSafetyTests.cpp` (ponte, aplicação e render com zero
  alocações) e `EcologyParameterStateTests.cpp` (quatro cenas, XML, cópias,
  rejeição transacional e prioridade/fallback da migração sem arquivo ou GUI).
- Não houve mudança nos algoritmos DSP, módulos irmãos nem nos WAVs de escuta.
  APVTS, automação sample-accurate, arquivos de presets, nomes livres e morph
  entre cenas não foram implementados. Transições sonoras ainda requerem escuta.
- Validação: build Release completo aprovado e 16/16 CTest em aproximadamente
  5,2 s; teste concorrente repetido 100 vezes sem falha. O teste de tempo real
  aprovou zero alocações e igualdade bit a bit com controles estáveis, incluindo
  novas notas e liberações. Naquela build Linux: quadro 112 bytes, ponte 576 bytes.
  Executável independente da ponte aprovado em UBSan. ASan/UBSan compilou, mas
  execução encerrou no LeakSanitizer por incompatibilidade com `ptrace`;
  ASan desta rodada e TSan continuam sem aprovação.
- Recursos: uma tarefa de compilação por vez, aproximadamente 14–16 GiB
  disponíveis durante o build e swap quase livre. Fixar UTF-8 no alvo do app
  exigiu recompilar JUCE; o primeiro lote atingiu timeout de 360 s e a retomada
  incremental, limitada a 300 s, concluiu. Nenhuma escrita no backup `ro` da
  namespace; processos externos ao sandbox não foram verificáveis.
- Binário confirmado em
  `build-reactivation-20260915/src/app/AquorbiumApp_artefacts/Release/Aquorbium`,
  compilado em 2026-09-16, 20:55:12 CEST, versão de projeto `0.0.1`.
  Strings de cenas/schema presentes; SHA-256
  `545730fef97127ee573dc39298ba65f4640c5bd2c1394088d856b15eb5649c90`.
  O aplicativo não foi aberto nesta rodada; build não equivale a ensaio real.
- Próximo passo de código: importação/exportação limitada de cenas, com erro
  explícito e gravação recuperável; ensaio real de cenas/Lupa/layout e TSan em
  ambiente compatível permanecem pendentes. Não houve pull, push ou commit.

### Execução de 2026-09-16 — transporte seguro de cenas e regressão ponta a ponta

- Objetivo: ampliar o código sem depender da escuta pendente nem alterar os
  motores musicais. Importação/exportação de uma cena foi integrada ao
  standalone; apenas arquivos locais de parâmetros, nunca áudio ou memória DSP.
- `src/app/PerformanceSceneFile.h`: `.aquscene` V1 de até 16 KiB, leitura
  limitada, UTF-8 validado, schema completo e valores normalizados. Rejeita
  arquivo vazio, tamanho excedido, bytes nulos, links simbólicos, DTD,
  entidades, parâmetros desconhecidos e aninhamento excessivo. Erro não
  altera o estado de destino. Exportação escreve/flush/relê o temporário,
  exige autorização para substituir e preserva o arquivo anterior em `.bak`
  com nome distinto dos backups já existentes.
- `src/app/ParameterXml.h` e classes de estado/banco: mesma defesa pré-parser
  nas preferências; propriedades inesperadas recusadas. Cópia e atribuição de
  `EcologyParameterState`/`PerformanceParameterState` agora são profundas;
  a revisão encontrou e eliminou o compartilhamento mutável implícito de
  `ValueTree` ao copiar os objetos.
- `src/app/PerformanceSceneControls.h`: adaptador de 25 valores para quadro
  escalar, usado na recuperação real. Captura os valores quantizados exibidos
  e preserva a Lupa transitória; não reinicia seed, notas ou memórias DSP.
- `src/app/Main.cpp`: IMPORTAR/EXPORTAR CENA, seletor assíncrono com
  `SafePointer`, slot/snapshot congelados, uma transferência por vez,
  confirmação de substituição e cancelamento sem mudança de som/banco.
  Importar apenas grava o slot; RECUPERAR aplica. Exportar usa o slot salvo,
  não controles alterados depois. Status/tooltip informam erro e backup.
- `EcologyPanelLayout.h` e teste: segunda linha do rodapé, sem sobreposição,
  três painéis representativos e limite mínimo de 440×324. CMake registra os
  novos arquivos e limita os testes de estado/cenas a 15 s por execução.
- Validação: build Release completo aprovado; 17/17 CTest, 6,17 s. Testes
  de parâmetros/arquivos e playback repetidos dez vezes cada sem falha.
  `PerformanceSceneFileTests.cpp` cobre roundtrip, backups sucessivos, recusa
  de sobrescrita não autorizada, destino inválido, UTF-8/bytes nulos, tamanho
  exato de 16 KiB, links simbólicos no Linux e preservação após erro.
  `PerformanceScenePlaybackTests.cpp` cobre XML → slot → quadro → ponte → DSP
  em 44,1/48/96 kHz × blocos 64/257/512: 144 trocas por execução, notas ativas,
  coalescência e comparação estéreo bit a bit, com amostras finitas/energia
  não nula. Naquele marco, o teste de tempo real confirmou zero alocações:
  quadro 112 bytes, ponte 576 bytes. `git diff --check` aprovado.
- Recursos: builds sequenciais com um job, timeout de 120–240 s por lote;
  aproximadamente 15 GiB disponíveis e swap 768 KiB de 2 GiB. Não houve
  crescimento sem controle. Backup montado `ro` na namespace não recebeu
  escrita; processos externos ao sandbox continuam não verificáveis.
  Temporários próprios dos testes foram removidos; build e evidências de áudio
  anteriores foram preservados.
- Binário: `build-reactivation-20260915/src/app/AquorbiumApp_artefacts/Release/Aquorbium`,
  versão `0.0.1`, compilado em 2026-09-16, 21:35:08 CEST; SHA-256
  `5c08dab86c175f344c1bb73e03d34877af6635f58084dfb52415e91901f44909`.
- Documentação sincronizada: README/tutorial, contrato de tempo real,
  ECOSYSTEM_DESIGN, WINDOW_LAYOUT_DESIGN e esta lista. Permanecem 271 tarefas
  históricas, 92 concluídas e 179 abertas; o marco de presets foi ampliado,
  não duplicado como tarefas artificiais.
- Limitações: nenhum ensaio de GUI, diálogo nativo ou dispositivo; `/dev/snd`
  ausente nesta namespace. Não se afirma ausência de cliques nem aprovação
  musical. Escrita é síncrona na GUI; filesystem de rede, alterações externas
  concorrentes e durabilidade após falha física não estão certificados.
  ASan/TSan novos não foram executados/aprovados nesta etapa.
- Retomada: ensaiar cenas/diálogos/Lupa no anfitrião e registrar a escuta
  musical já planejada. Próximo código independente: nomes limitados e opção
  de desfazer substituição de slot; banco inteiro em arquivo, APVTS, morph e
  novos motores continuam pendentes. Nada do Rasgo Modular foi alterado por
  esta execução; não houve pull, push ou commit.

### Execução de 2026-09-17 — continuidade sonora e auditoria da Lupa

- Objetivo: tratar os degraus de mistura/gesto e verificar o caminho sonoro
  da Lupa após o relato de cliques e pouca diferença ao arrastar. Não se
  atribuiu o relato a clipping nem se confirmou causa única sem dispositivo.
- `Ecosystem::audioMix()` separa a mistura sonora dos alvos físicos `mix()`:
  cinco estados fixos de quatro floats, um polo de 15 ms por amostra; reset
  parte do mesmo estado, sem fade inicial. `BiomaEngine` usa a mistura
  suavizada nas vozes externas/autônomas e parâmetros de excitação. O relógio
  ecológico continua 200 Hz, sem alocações ou números aleatórios adicionais.
- Movimento da Lupa passa a ser geodésico, limitado à chegada, sem travar
  em alvos antipodais nem oscilar em torno do destino. Mantidos velocidade
  0,55 rad/s, raio 0,38, alcance do vidro 0,75 e calibração de saída 5,0.
  Borda angular do foco contínua por smoothstep; fronteira de profundidade
  continua discreta no alvo, mas chega ao áudio pela suavização.
- GUI: rodapé agora distingue Lupa ativa/livre e `FOCO N`, contagem de
  organismos presentes geometricamente alcançados. Não é um medidor de
  volume nem considera o peso da perspectiva. Soltar libera o foco mesmo
  quando o anel permanece visível. Posse continua no observador 0.
- Auditoria: o primeiro render com alvos fixos +X/-X, posse de 2 a 12 s,
  produziu WAVs idênticos. Havia organismos perto do vidro, mas fora dos
  trajetos angulares; diagnóstico preservado e explicado em
  `validation-output/2026-09-17-lens-equal-diagnostic/`. Não se ampliou o
  alcance para fabricar diferença sonora.
- `aquorbium_ab_render --lens` acrescenta protocolo tracking-v1: direção do
  organismo 0 contra direção espelhada, atualizadas a 200 Hz; mesmo estado
  inicial/notas, banco autônomo ligado, feedback ecológico desligado,
  homeostase autônoma mantida, RMS global igualado. Acompanhamento automático
  existe apenas nesse experimento offline, não no gesto do app.
- Pares novos, sem revelar chaves: `2026-09-17-lens-equal-ab` e
  `2026-09-17-lens-solo-ab` dentro de `validation-output/`. Diferença RMS
  após igualação: 0,00565309 / 0,02012094; maiores picos de cada par:
  0,224398 / 0,463158. Um trajeto teve foco em 1199 quadros de 5 ms e o outro
  em zero. Maior diferença entre amostras: 0,086023 / 0,333688, sem usá-la
  como certificação de ausência de estalos. Protocolos e CSVs registram
  contexto, alvos, posse e contagem; áudios são locais e ignorados pelo Git.
- Validação: build Release Linux/core/app/tools, um job e timeout por lote;
  CTest final aprovou 18/18 em 26,60 s. Novo `LensAudioTests.cpp` verifica
  44,1/48/96 kHz, equação e limite por amostra da suavização, chegada
  antipodal estável, posse/foco, diferença DSP e identidade bit a bit entre
  blocos de 64/257 amostras, com pesos iguais e observador isolado.
  A diferença dos alvos fixos foi pequena, mas acima de ruído numérico;
  esse teste não impõe um limiar de percepção humana.
- Em 48 kHz, o maior degrau do alvo nesse caso foi 0,414077 contra
  0,000573277 na mistura sonora. Essa comparação mede continuidade do
  controle, não uma redução certificada dos estalos da gravação real.
  Teste de tempo real passou com zero alocações no trecho monitorado;
  naquele marco: quadro 112 bytes e ponte 576 bytes. CLI recusou sobrescrita, revelação com
  modo incompatível e uso da Lupa com observador isolado diferente de 0.
- Benchmark offline atual: 45 combinações, 44,1/48/96 kHz, blocos de
  32/64/128/256/512 e 0/1/4 vozes externas com banco autônomo/feedback
  ligados. Maior custo mediano relativo ao tempo de áudio em cada taxa:
  18,18% / 18,97% / 37,67%; menor margem foi 2,65 vezes tempo real. Não é
  percentagem total da CPU, garantia de prazo de callback ou medida de xruns.
  Não há comparação pareada com a versão anterior para afirmar ganho de CPU.
- Recursos: procedimento de segurança consultado, aproximadamente 13–14 GiB
  disponíveis, swap estável em 10 MiB de 2 GiB e cerca de 1 TiB livre no
  volume de trabalho. Builds com um job, renders/benchmark sequenciais,
  timeout de 45/60 s; nenhum crescimento observado sem controle. Backup
  visível como ro na namespace não recebeu escrita; processos pesados do
  host fora da namespace não foram verificáveis. Não foram removidas
  evidências, fontes ou builds anteriores; testes limpam seus temporários.
- Arquivos desta rodada: Ecosystem.h/.cpp, BiomaEngine.h/.cpp, Main.cpp,
  CMakeLists.txt, tests/LensAudioTests.cpp, tools/AquorbiumABRender.cpp e
  documentação local README, ECOSYSTEM_DESIGN, REALTIME_CONTRACT,
  WINDOW_LAYOUT_DESIGN e esta lista. Alterações anteriores foram preservadas.
- Binário: `build-reactivation-20260915/src/app/AquorbiumApp_artefacts/Release/Aquorbium`,
  versão 0.0.1, compilado em 2026-09-17, 22:47:29 CEST; SHA-256
  `0d58d70b363e9598d1c2277a64c0bb9da52bd98727a8c823e77d00f3c5e2d33b`.
- Limitações e retomada: `/dev/snd` continua ausente no sandbox, sem escuta
  ou inspeção visual desta versão. Reensaiar pressão/arrasto/soltura, conferir
  `FOCO N` e pesos, registrar cliques com instante/configuração de áudio;
  distinguir fonte/transição de xrun antes de escolher outra correção.
  Não se afirma que todos os estalos foram eliminados. A suavização muda a
  linha de base sonora e seu feedback: renders históricos foram preservados,
  não regravados nem apresentados como equivalentes à nova versão.
  Próximo código independente permanece nomes limitados/desfazer slot; novos
  motores continuam sujeitos ao protocolo de caso musical. Sem mudanças no
  Rasgo Modular, sem pull, push ou commit. Mantidas 271 tarefas históricas,
  92 concluídas e 179 abertas; calibração musical da Lupa continua pendente.

### Execução de 2026-09-18 — nomes locais e desfazer do banco

- Objetivo: ampliar as quatro cenas sem depender de decisões de escuta nem
  alterar DSP. Nomes de até 32 caracteres em uma linha, Unicode preservado,
  remoção com nome vazio e validação antes de modificar banco/histórico.
- `PerformancePresetBank` V2 conserva parâmetros e nome por slot; migra V1
  quando não há V2. Preferências têm chave V2 própria e continuam salvando
  V1 sem nomes para builds anteriores. Sem fusão automática entre versões;
  V2 tem prioridade. XML inválido continua preservado até mutação explícita.
- Desfazer mantém somente o último slot/nome anterior, inclusive vazio.
  Abrange gravação, importação e renomeação; não há refazer nem histórico
  persistido. Operações idênticas/recusadas preservam a possibilidade anterior
  de desfazer; restauração válida limpa histórico, rejeitada não o altera.
- GUI: campo de nome, RENOMEAR/Enter e DESFAZER no cabeçalho ECO CONTROLS,
  sem reduzir sliders ou aumentar o mínimo 440×324. GRAVAR confirma nome e
  parâmetros juntos. Desfazer seleciona o slot afetado, mas não muda o áudio;
  somente RECUPERAR aplica seus parâmetros. Diálogos bloqueiam os novos
  comandos e conservam o nome não confirmado em caso de cancelamento.
- Naquele marco, `.aquscene` permanecia V1 com 25 parâmetros, sem nome/histórico. Importar
  preserva o nome confirmado do slot; exportar não altera banco/histórico.
  Nenhuma edição desta rodada foi feita em Ecosystem, BiomaEngine ou demais
  fontes DSP; correções da Lupa e renders de 17/09 foram preservados.
- Segurança XML: serializador JUCE escreve acentos como referências decimais.
  Banco permite apenas escapes predefinidos e referências limitadas a
  caracteres Unicode imprimíveis; bloqueia controles, surrogates, caracteres
  inválidos, referências excessivas, entidades customizadas e DTD. Estados
  numéricos e `.aquscene` continuam recusando qualquer entidade.
- Validação: build Release Linux/core/app/tools com um job e timeout de
  240 s; CTest final 19/19 em 26,11 s. Novo teste do banco cobre Unicode,
  escapes, limites, desfazer para vazio/ocupado, nomes, no-ops, rejeição
  transacional, migração/preferências e isolamento dos parâmetros. Regressão
  ponta a ponta ampliada: XML → slot nomeado → gravação/desfazer → quadro →
  DSP, nove configurações, 144 transições e identidade bit a bit.
  Geometria verifica nova linha nos três painéis e no mínimo. Teste existente
  de tempo real passou com zero alocações; naquele marco, quadro/ponte eram
  112/576 bytes.
- Arquivos: PerformancePresetBank.h, ParameterXml.h, EcologyPanelLayout.h,
  Main.cpp, CMakeLists.txt, EcologyParameterStateTests.cpp,
  PerformancePresetBankTests.cpp, PerformanceScenePlaybackTests.cpp,
  WindowLayoutPolicyTests.cpp; README/tutorial, REALTIME_CONTRACT,
  WINDOW_LAYOUT_DESIGN e esta lista atualizados. Mantidas 271 tarefas
  históricas, 92 concluídas e 179 abertas; marco existente ampliado.
- Binário verificado: `build-reactivation-20260915/src/app/AquorbiumApp_artefacts/Release/Aquorbium`,
  versão 0.0.1, compilado em 2026-09-18, 00:03:46 CEST; SHA-256
  `e6a1e485bb59acca81efe42adf7116a7db6d848b55d07be745b63fd2aad4928d`.
  Chave V2 e textos RENOMEAR/DESFAZER confirmados no executável.
- Recursos: aproximadamente 14 GiB disponíveis e swap estável em 10 MiB de
  2 GiB; disco de trabalho com 1014 GiB livres. Sem tarefas pesadas paralelas
  nesta sessão; processos do host fora do sandbox não verificáveis. Backup
  ro na namespace não recebeu escrita. Não foram criados áudios, nem apagadas
  fontes/evidências/builds; temporários próprios dos testes são descartados.
- Retomada: ensaiar nome/Enter/desfazer, troca de slot, fechamento/reabertura e
  cancelamento de diálogos na GUI real; escuta dos estalos e calibração da
  Lupa continuam pendentes. Não houve aprovação visual/dispositivo nem teste
  de persistência das preferências pela GUI real. Próximo código independente
  possível: transporte do banco inteiro com nomes, limite, confirmação e
  backup. Sem mudanças no Rasgo Modular, sem pull/push/commit.

## Como ler esta lista

As fases abaixo preservam o histórico e agrupam trabalho por subsistema; não
são uma ordem rígida de execução. `[x]` indica uma entrega já registrada.
`[ ]` indica pendência, hipótese, experimento ou validação; o texto de cada
item esclarece qual deles. O banco final é uma biblioteca de propostas com
proveniência, não 64 implementações prometidas para a primeira versão. Uma
proposta só vira próximo marco depois de explicitar sua função musical, sua
relação com o Bioma, o custo técnico e a prova por escuta.

Documentos de referência: [arquitetura](aquorbium-arquitetura.md),
[padrão de desenvolvimento](MODULE_DEVELOPMENT_STANDARD.md),
[contrato de tempo real](REALTIME_CONTRACT.md) e
[medição de CPU](CPU_BASELINE.md).

## Continuidade de acervo — proporcional ao desenvolvimento

- [ ] Em uma release estável, entrega, migração ou publicação, atualizar o
      registro `ARQ-AQU-001` com fontes, versão, direitos, evidências e o
      manifesto/checksum/teste de restauração que forem proporcionais ao
      material; não arquivar um pacote novo a cada experimento ativo.

## Fase 0 — Fundação

- [x] Decidir framework definitivo: **JUCE (AGPLv3)** — decisão fechada, ver seção 2/3/11 do doc de arquitetura
- [x] Criar `LICENSE` (AGPLv3)
- [x] Inicializar no repositório git RASGO, em `AQUORBIUM/`
- [x] Criar projeto JUCE/CMake app-first com core DSP, módulos iniciais e
      shell standalone; manter `PluginProcessor` apenas como
      adaptador posterior
- [x] README inicial linkando o doc de arquitetura
- [x] Registrar no README que a primeira versão é app DSP multiplataforma;
      plugin, console embarcado e hardware são destinos posteriores
- [x] Manter o core DSP independente de GUI, dispositivo e formato de plugin
- [x] Compilar core, testes e ferramentas próprias em C++20 sem extensões, com
      warnings estritos tratados como erro; não aplicar `-Werror` ao JUCE externo
- [x] Adotar o padrão de excelência: pesquisa documentada, protótipo isolado,
      medição técnica e escuta de performance antes de integrar cada módulo
- [x] Formalizar o contrato de arquitetura dos módulos: modos autônomo,
      performance e híbrido, separação modelo/decisão/DSP/integração,
      real-time safety, telemetria, escuta e orçamento
- [x] Prever laboratório experimental por módulo, com risco artístico
      controlado por limites técnicos e passagem explícita ao modo estável
- [x] Tratar relações entre módulos como matriz de som, modulação e observação,
      com feedback limitado/suavizado e rotas de waveshape investigáveis
- [ ] Tratar `biome.odt` como pesquisa bruta; não copiar o hard clip chamado de
      `applyBrickwallLimiter` como implementação do limiter final
- [ ] Ao concluir cada marco, registrar no histórico global o que permanece
      específico do Aquorbium e o que pode servir aos instrumentos irmãos
- [ ] Manter ficha técnica, tutorial, imagens e demonstrações sincronizados com
      o estado real, seguindo `../../RASGO_DOCUMENTATION/ACERVO_E_PUBLICACAO.md`

## Fase 1 — Esqueleto de áudio mínimo

- [x] Shell standalone compilando, abrindo o dispositivo e executando
      `AudioEngine` sem crashar
- [x] Manter um teste de integração do core que não dependa de GUI, dispositivo
      nem host de plugin
- [x] Implementar vozes polifônicas no core com note on/off, velocity e roubo
      determinístico pela envolvente menos presente
- [ ] Avaliar `juce::Synthesiser` + `SynthesiserVoice` como adaptador opcional;
      o banco polifônico do core já funciona e não depende dessa troca
- [ ] Caso o adaptador JUCE seja adotado, sobrescrever `findVoiceToSteal` sem
      perder o roubo determinístico já implementado no core
- [x] Um oscilador simples (seno) tocável via gate, só pra validar o pipeline
      ponta a ponta; MIDI no shell standalone é opcional e `juce::Synthesiser`
      permanece como adaptador posterior
- [x] Receber note on/off e velocity no shell standalone por fila fixa, sem
      tocar o core diretamente a partir da thread de entrada MIDI
- [x] Tornar a entrada de eventos realmente MPSC para GUI e múltiplas threads
      MIDI, limitar consumo a 64 eventos por bloco e expor overflow na GUI
- [x] Implementar a primeira fonte autônoma de energia sonora antes de ampliar
      MIDI: cinco organismos com energia, altura e duração próprias, relógio de
      controle determinístico e banco independente das vozes externas
- [x] Provar nascimento sem `noteOn`, silêncio exato quando desligado,
      invariância de bloco/seed, rampas sem degrau e coexistência com MIDI
- [x] Gerar render offline sem MIDI com telemetria e relatório técnico
- [x] Recalibrar a saída em +13,98 dB após escuta indicar volume baixo, sem
      alterar feedback ecológico; validar cenário denso em pico 0,630
- [x] Separar stems determinísticos após escuta apontar predominância da onda
      angular; identificar Morph como fonte dominante e reduzir a distância
      Morph/Pulsar de 9,2 para 2,4 dB sem apagar nenhuma das duas camadas
- [x] Corrigir o duplo apagamento de Striker/Modal/String: o impacto já captura
      energia ecológica e sua cauda não deve desaparecer quando o organismo
      adormece; manter o ramp global capaz de produzir silêncio exato
- [x] Calibrar os três corpos eventuais por stems, elevando picos em unity de
      0,000426/0,000114/0,000215 para 0,00653/0,00612/0,01308 sem clipping
- [x] Substituir a fonte provisória do organismo `Resonator` por banco modal de
      quatro modos amortecidos e excitação determinística por eventos ecológicos
- [x] Implementar morph harmônico↔membrana, decaimentos por modo, ponto de
      impacto, brilho, dureza, teto de energia e recuperação numérica
- [x] Implementar `PerformanceIntent` de material/decaimento/brilho/dureza/ponto
      de impacto; influência zero preserva o autônomo bit a bit e novos alvos
      entram somente no impacto seguinte
- [x] Validar silêncio/cauda, seed, 200 impactos sobrepostos, polos, limites,
      real-time safety, render comparativo e CPU do ressonador
- [x] Continuar a investigação das famílias ainda não integradas: caos,
      feedback e FM/thru-zero; waveguide já foi integrado após dossiê e escuta
      (2026-10-22: o FM thru-zero e o FM de 4 operadores existem agora no
      dispositivo de síntese. 2026-10-23: CAOS, um oscilador de Duffing
      forçado na nota, adaptado do CHAOS do Rasgo Modular; e REALIMENTAÇÃO,
      um laço sem entrada afinado na nota. São fontes 13 e 14 do
      dispositivo. Medidos em `SynthVoiceTests`: o CAOS é periódico, caótico
      e periódico de novo ao longo da FORÇA, sempre a 0,30 de RMS; a
      REALIMENTAÇÃO canta sozinha em todo GANHO e DESAFINAR e fica afinada a
      0,3 cent. As PROPOSTAS não mudaram. Falta a escuta do autor.
      Detalhe em `SYNTH_DEVICE_DESIGN.md`)
- [x] Definir dossiê de investigação por fonte: questão musical, modelo,
      variações, diálogo modular, performance, evidência e destino
- [x] Prototipar oscilador morph próprio (seno → serra → pulso) com PolyBLEP e
      ligar densidade/deriva à matéria da onda; falta análise espectral/escuta
- [x] Prototipar organismo pulsar: densidade controla razão pulso/silêncio e o
      pulso arredondado evita clique; faltam agrupamento/formantes/eventos

## Fase 2 — Motor de síntese (`SynthFMVoice.h`)

- [x] Oscilador wavetable senoidal fixo com interpolação; protótipo isolado,
      ainda sem wavetable de amostras ou seleção de formas
- [x] Aplicar PolyBLEP aos caminhos de serra de `BiomaBassVoice` e
      `SchoolWaveVoice`; o oscilador polygonal usa geometria contínua e não
      introduz a mesma descontinuidade; análise espectral continua pendente
- [x] Thru-zero FM entre dois osciladores; incremento de fase assinado,
      reversão negativa coberta por teste, ainda sem integração sonora
- [x] Wavefolder por sine-shaping (`sin(gain*in)`) integrado às vozes FM e
      BiomaBass, com caminho desligado exato
- [x] Aplicar avaliação 4× interpolada e decimação curta ao sine-folder para
      reduzir aliasing residual; meia-banda/ADAA/polyBLAMP permanecem opções se
      a escuta ou medições mais amplas exigirem
- [x] Comparar espectro direto/4× num caso agressivo e verificar redução de ao
      menos 15% da energia medida na banda alta; render de referência e escuta
      musical permanecem portões separados

## Fase 2b — BiomaBass, motor de graves original (`SynthFMVoice.h`)

*Combinação original de princípios de módulos de mercado (Endorphin.es Godspeed, Cwejman VCO-2RM, Doepfer A-111-4/A-102/A-120, Oakley Diode Superladder, Studio Electronics 5089, Joranalogue Filter 8, Moog Mother-32/Mavis, Reese bass, FM bass) — ver seção "Módulo 3.4" do doc de arquitetura pra referência completa de cada uma.*

- [x] Integrar em `BiomaBassVoice` o wavefolder antes do filtro (princípio
      Mavis) usando a mesma avaliação 4×; calibração musical continua pendente
- [x] Prototipar reinjeção limitada da fundamental após o filtro do
      `BiomaBassVoice`, preservando corpo em ressonância alta; uma topologia
      auto-oscilante definitiva ainda depende de escuta
- [x] Prototipar em `ReeseVoice` múltiplos osciladores PolyBLEP dessintonizados com detune modulado por Slew-Rate Random Walk; integração especializada ao BiomaBass permanece pendente
- [x] Aplicar FM thru-zero com incremento de fase assinado no `BiomaBassVoice`
- [x] Derivar o sub-oscilador do Interval Shift Quantizer (Módulo 2) em vez de
      fixo em -12 semitons; o banco agora converte grau negativo pelo
      `IntervalQuantizer` e congela o intervalo no evento
- [x] Implementar seguidor próprio de energia, sem JUCE, na saída do
      `BiomaBassVoice`, reduzindo automaticamente detune e wavefold acima do
      limiar
- [x] Implementar auto-compensação de ganho no wavefolder (princípio WORNG
      Glowing Orb), protegendo headroom antes do filtro
- [x] Prototipar em `DualPeakFilter` dois picos ressonantes com cross-feedback limitado; substituição ou complemento do filtro do BiomaBass permanece pendente
- [x] Prototipar roteamento contínuo paralelo↔série no `DualPeakFilter`, com primeira modulação contextual no banco; ligação definitiva a `Desire::danger` no BiomaBass permanece pendente
- [x] Prototipar contagem variável de dois a quatro polos no filtro do
      `BiomaBassVoice`; o banco escolhe por evento e a exposição especializada
      ao Cérebro permanece pendente
- [x] Prototipar slopes de 18/24 dB por seleção de três/quatro polos; o
      mapeamento discreto definitivo ao Cérebro permanece pendente
- [ ] Teste A/B comparando contra um Reese bass convencional e um FM bass convencional, validando que o resultado é distinto

## Fase 3 — Sequenciador geométrico (`GeometricSequencer.h`)

- [x] Distribuição euclidiana circular de 1–32 passos, escrita do zero como
      acumulador limitado, com contagem/espaçamento/rotação testados
- [x] Cinco lanes com cursores e estados aleatórios independentes sobre uma
      geometria compartilhada; tráfego de uma lane não altera o acaso da outra
- [x] Mistura contínua entre intenção autônoma e performance: densidade
      rítmica, rotação orbital, viés intervalar e mutação
- [x] `PatternRotationMutator` aplica `std::rotate` a cada N ciclos, com
      rotação assinada, preservação da contagem de pulsos e limites fixos
- [x] Primeira mutação determinística e limitada por rotação de órbita, sem
      alterar contagem de pulsos; mutação estrutural por compasso continua aberta
- [x] Interval Shift Quantizer com graus negativos, até 16 intervalos, período
      fracionário e política explícita para valores inválidos
- [ ] Sync de tempo via `AudioPlayHead` (BPM da DAW)
- [x] Clock fracionário isolado com dois acumuladores racionais e AND/OR/XOR;
      7/4 e 5/4 testados, integração ao sequenciador/host ainda pendente
- [x] Memória fixa de 32 passos endereçável sequencialmente e por CV direta
      em `AddressableStepMemory`; a configuração inicial e os quatro macros
      agora persistem, enquanto edição detalhada e `AccessPort` seguem abertos
- [x] Transposição acumulativa isolada em `AccumulatingTransposer`, com faixa
      fixa e direção invertida pela metade de `seasonPhase`; ligação ao
      `Environment` e ao quantizador permanece pendente
- [x] Banco fixo `StepPatternBank` com oito snapshots, restauração e cadeia de
      até 16 referências; persistência externa e recuperação por
      `AquariumIntegrity` permanecem pendentes
- [x] Matriz `NotePermutationMatrix` com pool de 16 notas, oito ordens,
      validação de permutação, oitava e velocidade; integração pendente
- [x] Prototipar `HarmonicProgression`: quantizador, nove qualidades de acorde,
      inversões e até 32 seções ponderadas; integração técnica ao percurso
      dia/estação concluída em 27/09
- [ ] Escutar e aprovar a política musical que interpola dia e estação nas
      fronteiras da progressão, registrando ritmo, melodia, harmonia, eventos e
      função composicional
- [ ] Avaliar Westlicht PER|FORMER como codebase de referência adicional (é a única desta lista inteira 100% open source, hardware e firmware) — geradores de sequência algorítmicos embutidos como inspiração pro Módulo 2
- [x] Cada passo da memória carrega quatro valores simultâneos: altura,
      duração de gate, viés de estratégia e quantidade de desejo
- [x] Primeiro `Step` trivial já publica quatro CVs simultâneos normalizados
      (pitch, gate, acento e posição orbital); memória editável/persistente com
      duração, estratégia e desejo continua fora deste marco
- [x] `StepMemoryLayout` mapeia o mesmo pool de 32 endereços como 4 × 8,
      2 × 16 ou 1 × 32, sem copiar a memória subjacente
- [x] Terceiro modo de endereçamento isolado em `ThresholdStepSelector`: até
      32 limiares transformam sinal contínuo em índice, transição e direção;
      fonte ecológica e ligação à memória permanecem pendentes
- [x] Quarto modo isolado em `BranchingStepRouter`: primeira camada escolhe
      até oito categorias e a segunda até quatro variantes, com limiares
      independentes; semântica ecológica e integração permanecem pendentes
- [x] `RandomToLoop` isolado com memória de 1–64 valores e eixo contínuo entre
      entrada renovada e ciclo fixo; fonte ecológica e escuta permanecem
      pendentes

## Fase 4 — O Cérebro (`BrainEngine.h`)

- [x] Slew-rate random walk (ruído + filtro passa-baixa <2 Hz) ligado ao
      `MultiTapSpace` com profundidade sutil, seed e slew limitados; conexões
      adicionais com Cérebro e grave continuam opcionais
- [x] `DriftCurveShaper` separa suavidade degrau↔contínuo de família linear,
      exponencial, inversa, Bézier, arco ou pico; ligação às fontes permanece
      pendente
- [x] Terceira via de geração de tempo em `BiomaRelabi`: soma determinística de
      quatro LFOs lentos não relacionados, já usada por `AutonomousEnergy` e
      agora coberta por teste próprio
- [ ] Avaliar `BiomaRelabi` como fonte compartilhada de decisão para
      `SchoolInfo`, provando por escuta coordenação sem uníssono mecânico
- [x] Eventos discretos por cruzamento de zero generalizados em
      `ControlEventExtractor`, com histerese; ligação ecológica pendente
- [x] Atrator de Lorenz como gerador de CV (σ, ρ, β, dT parametrizáveis)
      em `LorenzCV`, protótipo isolado ainda sem rota ecológica
- [x] Cross-feedback state machine na saída final analisando o próprio áudio e
      realimentando regras do sequenciador; a implementação portátil usa
      balística própria, histerese e um atraso causal de uma amostra
- [x] Conectar o Cérebro ao Sequenciador (Fase 3) e ao Espaço (Fase 6)
      (2026-10-23: o registrador do `BiomaBrain`, lido como cinco
      inclinações correlacionadas, `correlation()`, chega aos dois lados.
      **Sequenciador:** com GEOMETRIA acima de zero, ele inclina a direção
      das mutações de cada organismo, e organismos correlacionados no
      registrador mutam juntos. Com GEOMETRIA em 1 e inclinações opostas, as
      2001 mutações de 4000 passos vão em sentidos opostos.
      **Espaço:** com o gesto do ESPAÇO acima de zero, o humor médio do
      Cérebro abre ou fecha a largura dos reflexos em até 10 %; medido no
      motor, de 0,94 a 1,10.
      Com GEOMETRIA ou ESPAÇO em zero, cada caminho fica idêntico bit a bit,
      e os renders dourados não mudaram. O passo do registrador (~0,23 Hz)
      chega ao Espaço pelos alvos dos taps, que já deslizam. Detalhe em
      `BRAIN_DECISION_DESIGN.md`. Falta a escuta do autor)
- [x] Extração de envoltória + gate/trigger com ataque, release, limiar e
      histerese em `ControlEventExtractor`; destino ecológico pendente
- [x] Cadeia de Markov em `BiomaMarkov`, com oito estados, contexto, inércia e
      mutação, integrada como memória formal da Geometria; escuta pendente
- [x] Prototipar `FixedLSystemGenerator` bounded como gerador formal
      alternativo; promoção ao `GeometricSequencer` permanece pendente
- [x] Prototipar `CellularAutomatonSequencer` de 32 células e regra elementar;
      promoção ao `GeometricSequencer` permanece pendente

## Fase 5 — Glitch e percussão (`BiomaGranularStriker.h`, `PhysicalStringVoice.h`)

- [x] Multi-particle noise striker próprio: pool fixo de 24 slots, nuvens de
      2–12 partículas, grãos de 2–40 ms e envelope exponencial
- [x] Primeiro organismo modal excitado por ruído curto; é um corpo ressonante
      contínuo, não substitui o futuro striker multi-partícula
- [x] Filtro passa-banda ressonante por partícula, com Q entre 20 e 100,
      coeficientes calculados somente no nascimento do grão
- [x] Superfície performática do Striker com quatro gestos (`DEPTH`, `MATTER`,
      `POP`, `SCATTER`), snapshot lock-free, persistência e layout responsivo
- [x] Provar que mudança performática afeta apenas novos grãos e não retuna
      caudas em curso; influência zero preserva o autônomo bit a bit
- [x] Prototipar `BiomaPhysicalString` a partir dos conceitos de
      Karplus-Strong/waveguide: atraso fracionário, filtro de perdas compensado,
      excitação por posição e barreira suave, sem copiar código externo
- [x] Validar afinação em 48/96 kHz, seed, cauda, estabilidade, intenção
      híbrida congelada por impacto, zero alocações, CPU e três renders
      (`filament`, `shell`, `blade`)
- [x] Gerar comparação cega String/Modal/Striker com dez eventos, seed comum,
      RMS exato e sem limiter; manter a chave fora da pasta até a revelação
- [x] Impedir a duplicação ingênua dos 131.280 bytes do String em cada
      despachante: se aprovado por escuta, usar banco fixo de cinco corpos,
      80% menor que embutir o motor em 25 despachantes
- [x] Registrar a primeira escuta cega: A reconhecido como material familiar,
      B percebido como ruído e C como golpe seco/envelope sobre matéria sonora;
      revelar A=Modal, B=Physical String e C=Granular Striker
- [x] Refinar o excitador do String após a escuta: dois polos, janela Hann sem
      degrau e contato direto reduzido; confirmar na segunda escuta a
      sofisticação da versão refinada
- [x] Integrar Physical String como quinto organismo, com um banco dedicado de
      cinco corpos (autonomia + quatro vozes), sem replicar seu delay nos 25
      despachantes compactos; publicar energia/impactos e mostrar `STRING`
- [x] Prototipar motor granular de amostra próprio, sem copiar GRNLR;
      `BiomaGranularCapture` está isolado e não substitui o Striker ativo
- [x] Confirmar corpo sonoro polimórfico por espécie sem herança virtual:
      `OrganismVoiceEngine` faz despacho tipado e alocação fixa para
      Morph/Pulsar/Modal/Granular, com fallback técnico comparativo
- [x] Interface mínima de voz de percussão por amostra (pitch + decay + seleção, princípio Erica Synths Sample Drum) como referência de fallback pra golpes curtos
- [x] Morph contínuo entre duas famílias de forma de onda por interpolação harmônica (princípio Acid Rain Chainsaw), não crossfade linear ingênuo — caminho alternativo de timbre

## Fase 5b — Granular de amostra e auto-captura (`BiomaGranularCapture.h`)

*Ver seção 9 de `aquorbium-arquitetura.md` — princípio Instruō arbhar.*

- [x] Parâmetros por grão (pitch/pan/reverse próprios), não nuvem uniforme —
      cada evento deriva duração, velocidade, panorama, ganho e reversão de
      `strength`/`agility`/`intelligence` do `GranularStriker` real
- [x] Ativar captura automática de trechos do mix recente que chega ao banco via
      detecção de onset em `BiomaGranularCapture`; topologia definitiva e
      aprovação musical seguem abertas
- [x] Prototipar em `RecentMixWavetable` resíntese ocasional a partir do buffer
      recente do próprio Aquorbium, com captura incremental e duas tabelas fixas
- [x] "Soft take-over" em toda automação vinda de `Desire`/BrainEngine para
      que um futuro `AccessPort` nunca cause descontinuidade de parâmetro
  - implementação de referência de um polo já aplicada ao contexto contínuo
    que alimenta o banco integrado; a futura edição direta pelo `AccessPort`
    deverá reutilizar o mesmo contrato
  - 2026-10-23: `AutomationClickTests` passou a cobrir os caminhos do
    Desire e do Cérebro, mudados de golpe como um `AccessPort` mudaria.
    Ganharam casos: DENSIDADE, DERIVA, COESÃO, PERIGO, a mistura de escuta,
    a Lupa segurada, o modo autônomo desligado, GEOMETRIA, FORMA,
    ESTRUTURA, MÚSICA nos dois sentidos, SUSTENTAÇÃO e ORDEM DO EQ.
  - Um defeito achado e corrigido: a influência da MÚSICA entrava sem
    rampa na mistura do filtro "líquido" e no piso dos corpos de evento.
    MÚSICA 0,6 → 0 estalava (×0,73, −69 dBFS); agora desliza em 40 ms por
    organismo (×0,011, −110 dBFS). Com MÚSICA em 0 o caminho continua
    idêntico bit a bit.
  - O contrato para o `AccessPort`: todo setter do motor desliza, e cada
    controle novo ganha um caso nesse teste.
- [x] Expandir o `Species::Pulsar` já integrado: pulsos curtos sintetizados
      (pulsaret) seguidos de silêncio, razão pulso/silêncio derivada de
      `Desire::density`/`AquariumIntegrity::crackStress` — calmo e denso agrupa
      pulsos quase tonais, tenso/trincado espaça até virar cliques isolados
      (princípio Hieroglyphic Plume, síntese de pulsar de Curtis Roads, seção
      12); pulsaret, cluster por traços e máscara por integridade estão ativos
- [x] Mascaramento estocástico em taxa de áudio (supressão probabilística de pulsos individuais) como equivalente do `Prob` do DriftLog só que no domínio do som, não do controle — componente isolado decide somente no onset; integração ao `BiomaPulsar` e escuta seguem abertas

## Fase 6 — Espaço (`MultiTapSpace.h`)

- [x] Fundação `AquariumSpatializer`: pan de potência constante, profundidade
      espectral por fonte, largura ligada à integridade e suavização em taxa de
      áudio, sem JUCE/alocação no core
- [x] Rota técnica `--space-off` que mantém o espacializador avançando para
      comparação sob os mesmos relógios ecológicos
- [x] Multi-tap delay line portátil com oito ponteiros Lagrange cúbicos,
      memória estéreo fixa e feedback limitado; a implementação conceitual
      própria substitui a dependência direta de `juce::dsp::DelayLine` no core
- [x] Ganho, pan e filtro independentes por tap, com suavização, macros
      autônoma/híbrida/performance, testes e render isolado
- [x] Integrar uma única instância wet-only pós-soma, com retorno suavizado,
      rota técnica on/off e telemetria sem multiplicar memória por organismo
- [x] Adaptador ecológico a 200 Hz para luz, temperatura, desejo e integridade,
      mantendo o feedback de homeostase pré-espaço até seu vínculo ser explícito
- [x] Reenquadrar reverb como gerador sintético de cauda, não simulador de sala:
      `AquariumTail` FDN própria, instância pós-MultiTap, quatro macros, cenas,
      bypass, render, layout e validação técnica concluídos em 25–26/09
- [ ] Escutar e decidir caráter ecológico da Cauda por `Environment`
      (dia/noite, estação) e `AquariumIntegrity::crackStress`; a integração
      atual é manual e não cria esse vínculo antes da avaliação musical
- [x] Modulação sutil dos tempos de delay pelo Slew-Rate Random Walk, com
      profundidade ecológica limitada e matriz de feedback inalterada
- [x] Prototipar em `ResonantFilterSequencer` banco de oito ressonadores com
      espaçamento de sétima maior; aplicação especializada no Multi-Particle
      Noise Striker permanece pendente
- [x] Prototipar `ResonantFilterSequencer` com oito bandas e segunda ressonância simultânea; a rota experimental usa fase contextual, enquanto `AquariumTransform::rotationY` e encontro predador/presa reais permanecem pendentes

### Checkpoint de retomada — 2026-08-12

O marco encerrado hoje é a integração técnica do Espaço: `MultiTapSpace`
compartilhado, adaptador ecológico a 200 Hz, seis controles persistidos na GUI,
testes de segurança/zero alocação, renders A/B on/off e linha de base integrada
de CPU. A próxima sessão deve começar pela escuta e calibração do retorno
`0,36` e dos controles, mantendo a distinção entre caráter musical e estabilidade
técnica. Só depois entram, em ordem experimental, cauda sintética, random walk
dos tempos e banco de ressonadores. O Master, APVTS, ThreadSanitizer em ambiente
compatível e testes de janela/DPI continuam fora deste marco.

## Fase 7 — Master (`MasterChain.h`)

### Fase 7a — Master criativo do Aquorbium

- [x] Crossover multibanda Linkwitz-Riley (grave/médio/agudo)
- [x] Compressor independente por banda (attack/release/ratio próprios)
- [x] Bypass bit-transparent ou diferença numericamente documentada para cada
      estágio criativo desligado
- [x] Medidor de loudness ITU-R BS.1770 (K-weighting → gate absoluto -70 LUFS → gate relativo -10 LU → integrar), pós-safety e sem normalização automática
- [x] Validar medição BS.1770 contra vetores de referência EBU Tech 3341, não apenas um WAV produzido pelo próprio instrumento; suíte integral ainda precisa repetir o teste quando a pressão de swap baixar
- [x] Validar perfis de LUFS de streaming/álbum como destinos distintos; não
      usar um alvo único nem normalizar automaticamente a performance ao vivo
      (2026-10-23: `core/DeliveryProfiles.h`; cada take do REC traz no texto
      ao lado o que Spotify Normal −14, Spotify Quiet −19 e EBU R128 −23
      fariam ao arquivo: ganho, LUFS e true peak resultantes. É só relatório,
      nada é aplicado ao som. Fontes em `OUTPUT_STAGE_DESIGN.md`; Apple Music
      ficou de fora até haver fonte primária. Teste em `TakeAnalysisTests`)
- [x] Expor a envolvente real de redução de ganho multibanda/cola como entrada
      extra do Cérebro a 200 Hz (princípio Cosmotronic Messor)
- [x] EQ roteável antes/depois do compressor final como escolha exposta, não ordem fixa em projeto (princípio 4ms LEQA, seção 12)
      (2026-10-23: ORDEM DO EQ, quinto controle do cartão MASTER, com
      ANTES e DEPOIS; parâmetro de cena `masterEqOrder`; cenas antigas
      abrem em ANTES, a ordem de até então. Medido em `MasterChainTests`:
      a ordem muda 1,9 dB um seno de 50 Hz com inclinação máxima. A troca
      tocando não dá degrau: no instante da troca `x·eq·c = x·c·eq`, e a
      diferença chega com o envelope do compressor (ataque de 32 ms).
      Nenhuma transição suave foi necessária; com e sem ela o resultado
      foi idêntico)
- [x] Largura estéreo final sob CV, amarrada a `AquariumIntegrity::crackStress` — o mix estreita espacialmente conforme o aquário trinca, complementando o vazamento de posição que `worldPosition` já produz (mesmo princípio LEQA)
- [x] Estágio de compressão ótica suave ("cola") antes do limiter com lookahead já planejado — caráter diferente, não substituto (mesmo princípio LEQA)
- [x] Limitar a 0,32 e suavizar com ataque/release o feedback atual do Master
      para o Cérebro; futuros taps deverão atravessar o mesmo contrato

### Fase 7b — Saída técnica comum (`OutputStage.h`)

- [x] Integrar a política
      `../../RASGO_DOCUMENTATION/architecture/SAIDA_AUDIO_COMUM.md` depois do
      `MasterChain` criativo
      (2026-10-22: conferida item a item na tabela de conformidade de
      `OUTPUT_STAGE_DESIGN.md`. A §4 foi corrigida no mesmo dia: o banco de
      módulos e o habitat leem pré-safety, e o volume deixou de mudar a
      música. A varredura de bypass (`BypassSweepTests`) e os renders
      dourados (`GoldenRenderTests`) foram feitos no mesmo dia. Aberto: o
      soak test)
- [x] Definir orçamento de ganho por organismo e por submix, compensação de
      densidade e headroom do somador sem apagar diferenças de presença
      (2026-10-22: medido em `GainBudgetTests`, antes do MASTER criativo, e
      fixado como regressão. A soma tem ~15 dB de folga em true peak, mesmo
      na densidade máxima. Os dois de primeiro plano (MORPH, PULSAR) ficam a
      −28/−30 LUFS. Os três de sustentação (GRAIN, MODAL, STRING) ficam a
      −50/−55 LUFS: 22 a 26 LU abaixo com a MÚSICA, e 30 a 38 sem ela, quase
      inaudíveis na soma. Falta o autor decidir o alvo de presença da
      sustentação; ver o grupo A)
      (2026-10-23: em vez de eu escolher o equilíbrio, um controle para o
      músico: SUSTENTAÇÃO no cartão TIMBRE, de −6 a +24 dB. É um
      multiplicador sobre os ganhos autorais das espécies, nunca no lugar
      deles; em 0 dB o som é idêntico (renders dourados inalterados). Fica
      nas cenas e tem LEARN, com rampa. Medido: +20 dB de SUSTENTAÇÃO dá
      +20,08 dB no stem dos organismos e não toca o primeiro plano)
- [x] Saneamento de `NaN`/`Inf` e proteção DC/infrassônica antes da saída
- [x] Limiter estéreo linkado com lookahead e true peak; não usar hard clipping
      como substituto de limiter (2026-10-22: detecção em true peak 4× e ganho
      pelo mínimo móvel da janela; `OUTPUT_STAGE_DESIGN.md`)
- [x] Oversampling nos estágios não lineares e na detecção/controle true peak
      (2026-10-22: a detecção e o controle de true peak já usam 4×. O DRIVE
      do dispositivo de síntese ganhou antialiasing por antiderivada, ADAA de
      1ª ordem (Parker et al., DAFx 2016): o pior espelho de um seno de 7 kHz
      com drive máximo foi de −18,6 para −30,2 dB; na região musical fica
      abaixo de −77 dB a 1,1 kHz. O oversampling de 2× ficou de fora: a
      latência dele só existiria com drive acima de zero e daria um clique ao
      sair do zero. Faltam medir as não linearidades das famílias do banco
      de módulos (`AdvancedVoiceEcologyModules`, `GenerativeInteractionModules`,
      `OpenInteractionModules`) e o limite da corda (`BiomaPhysicalString`).
      2026-10-23: medidos em `AliasingMeasureTests`, num seno de grade de
      FFT prima, com controles da métrica: seno puro −154 dB, dente de serra
      ingênua −13 dB. Achados e corrigidos: o WAVE CUT do SchoolWave, que
      aliasava como uma dente de serra ingênua (−10 dB), ganhou polyBLEP nos
      dois saltos (−29 dB, o nível do próprio polyBLEP); os harmônicos do
      HarmonicRipple agora somem antes de Nyquist (3 kHz: de −29 para
      −76 dB). Ficam como estão, medidos: BiomaBass de −45 a −78 dB (o
      dobrador já é sobreamostrado); a barreira da corda, só em sobrecarga,
      −41 dB a 2 kHz com o dobro do limite. Os módulos caóticos ou de ruído
      (atrator, crepitação, barramento de realimentação sem entrada) não
      têm grade harmônica para este método; o seu conteúdo já é de banda
      larga. Detalhe em `OUTPUT_STAGE_DESIGN.md`)
- [x] Ceiling configurável por perfil; começar a validação de exportação em
      `-1 dBTP`, sem tratar esse valor como universal
      (2026-10-23: o teto da saída já é do músico, TETO de −6 a −0,1 dBTP;
      o relatório do REC confere o arquivo gravado contra o teto de cada
      perfil e contra a regra do Spotify, −2 dBTP quando o master passa de
      −14 LUFS)
- [x] Output trim em dB, mute/fades sem clique, peak hold e clip latch
      (conferido em 2026-10-22: trim de −18 a +6 dB; mudo com rampa, sem
      clique em `AutomationClickTests`; peak hold = o true peak da sessão,
      na dica do ÁUDIO; clip latch = `clipLatched` na telemetria e o contador
      CLIP no status)
- [x] Medidores sample peak, true peak, RMS, gain reduction e correlação;
      LUFS momentary/short-term/integrated já estão no diagnóstico/runtime,
      enquanto true peak normativo e correlação mantêm a tarefa aberta
      (2026-10-22: o true peak do medidor de saída passou a ser 4× e a
      correlação estéreo foi feita, com média de ~400 ms. True peak,
      redução de ganho e correlação aparecem na dica do botão ÁUDIO. O true
      peak passa os sinais 15–23 da EBU Tech 3341 V4, dentro de +0,2/−0,4
      dBTP, com o pior caso em −0,25 dB; `EbuTruePeakTests`)
- [ ] Declarar latência ao host e oferecer perfil de baixa latência no app
      standalone sem ocultar a redução de proteção/qualidade
- [x] Garantir que `AquariumIntegrity::shattered`, no-input mixing e qualquer
      auto-oscilação permaneçam antes do limiter de segurança não modulável
      (conferido em 2026-10-22: o `OutputStage` é o último estágio de
      `BiomaEngine::render`, depois do `MasterChain`; a realimentação de
      energia, a sala e o habitat leem a saída já protegida; os parâmetros
      do limitador só mudam pela interface. O pior caso pela interface fica
      abaixo do teto, em `EngineStressTests`)
- [ ] Publicar taps limitados/suavizados: pré-master criativo, pré-safety,
      gain reduction e pós-safety

### Fase 7c — Gravação e entrega

- [x] Gravar estéreo final e stems float separados para BiomaBass,
      micropercussão/glitches e espaço/delays/reverbs (2026-10-22, pela
      mistura atual. O BiomaBass não existe mais no render. Com
      `recording.stems` = `true`, o REC grava ao lado do take cinco WAVs:
      organismos, LAB (o banco de módulos, que tem a família de pulsos
      percussivos), água, espaço e cauda. A soma deles é a mistura antes do
      MASTER criativo; `RecordTapTests` mede erro de 4·10⁻⁸. Com o filtro
      espectral do AMBIENTE ligado, o stem "água" carrega o efeito do filtro
      sobre os outros. O caminho completo no app não foi exercitado (pede um
      clique em REC))
- [x] Nomear e documentar taps de gravação: `pre-master-criativo`, `pre-safety`
      e `post-safety` (2026-10-22: `recording.tap` nos settings escolhe onde
      o REC ouve; o padrão é `post-safety`, o que se ouve. O tap vai no texto
      ao lado do take. `RecordTapTests`: o tap não muda a saída, bit a bit;
      `pre-safety` fica antes do trim e do limitador; `pre-master-criativo` é
      igual ao `pre-safety` com o MASTER em bypass. Ver `OUTPUT_STAGE_DESIGN.md`)
- [x] Remover o DC das vozes de MIDI: uma nota externa deixava um offset
      constante de +0,0055 (−45 dBFS) na mistura, antes do estágio de saída,
      que o removia. Achado em 2026-10-22 pelos taps. A origem era o trem do
      Pulsar (média +0,167), sem o bloqueador de DC de 10 Hz que o caminho
      autônomo tem. Agora cada organismo de cada voz de MIDI tem o mesmo
      bloqueador; a média ficou em −8·10⁻⁷ (`RecordTapTests`)
- [x] Exportar float32, PCM24 e PCM16; aplicar TPDF dither somente na redução
      para ponto fixo (2026-10-21: float 32 e PCM 24; 2026-10-22: PCM 16, com
      `recording.format` = `pcm16` nos settings e TPDF no LSB de 16 bits)
- [x] Validar true peak e loudness novamente depois da codificação do arquivo
      (2026-10-22: um segundo medidor recebe os inteiros escritos e o texto ao
      lado do take mostra o master e o arquivo codificado; o teste relê o WAV
      do disco e confere a igualdade. Seno de 997 Hz a −20 dBFS: em 16 bits,
      −20,06 LUFS e −19,309 dBTP, contra −19,310 do master)
- [x] Testar downmix mono e manter a fundação grave estável quando HRTF/width
      estiverem ativos (2026-10-22, `AutomationClickTests`: abaixo de 150 Hz,
      correlação de 0,90 a 0,93 e soma mono de −0,2 a −0,3 dB, nas caixas e
      nos FONES (binaural), largos e estreitos; o binaural decorrela os
      agudos, não a base)

## Fase 8 — Padrão Aquário (`Ecosystem.h`)

*O protótipo legado da raiz foi auditado e retirado da arquitetura. A
implementação canônica está em `src/core/Ecosystem.h/.cpp`, integrada ao
`BiomaEngine`. Contrato: `ECOSYSTEM_DESIGN.md`; diagnóstico:
`ECOSYSTEM_AUDIT.md`.*

### Marco concluído — fundação canônica

- [x] Substituir o protótipo por um núcleo C++20 portátil, sem JUCE e sem
      alocação no caminho de áudio
- [x] Usar `BiomaBrain` como fonte determinística única de acaso ecológico
- [x] Atualizar a física em relógio absoluto de amostras a 200 Hz, independente
      do tamanho dos blocos do host
- [x] Limitar organismos ao interior da esfera e manter observadores em órbita
      externa estável
- [x] Implementar percepção parcial para predador, presa e territorialidade
- [x] Implementar Desejo suavizado, presença, ambiente, Lupa exclusiva e
      integridade com causa, recuperação e ruptura irreversível
- [x] Implementar quatro modos de escuta autorais, panorâmica de potência
      constante e orçamento de ganho anterior ao master
- [x] Integrar os ganhos e o detalhe ecológicos às quatro vozes atuais do
      `BiomaEngine`
- [x] Verificar determinismo, seed, limites espaciais, headroom, Lupa,
      integridade e ausência de alocações em testes automatizados
- [x] Criar os modos de renderização `Active`/`Neutral`, telemetria fixa e o
      renderizador offline `aquorbium_ab_render`: sessão cega com `A.wav`,
      `B.wav`, ficha de resposta, chave sob `--reveal` e CSV de análise sob
      seed/eventos iguais, com feedback de energia desligado para o A/B
      controlado
- [x] Tratar `Active` e `Neutral` como identidades expressivas equivalentes e
      implementar `Ecosystem Morph` contínuo: rampa exata de 80 ms, sem reset,
      independente do tamanho do bloco, sem alocação e publicado atomicamente
      pela GUI protótipo

### Integrações e validações ainda necessárias

- [x] Projetar `AquariumPose` como transformação rígida separada: aquário local
      para organismos/água/Lupa e world space para câmera/observadores externos
- [x] Implementar transformação inversa dos raios de percepção e provar que
      mover/girar o aquário não viola contenção, oclusão ou posição da Lupa
- [x] Prototipar `WaterInertia` assimétrica, limitada e dependente da taxa de
      amostragem; avaliação por escuta e ligação definitiva a
      pan/profundidade/Doppler permanecem pendentes
- [x] Conectar as espécies atuais aos motores DSP reais por
      `OrganismVoiceEngine`, sem números mágicos de índice e sem colocar DSP em
      `OrganismState`: Morph/Granular/Modal/Pulsar usam o mesmo contrato nos bancos
      autônomo e externo
- [x] Acrescentar `BiomaGranularStriker` ao despacho depois do dossiê e portões,
      usando eventos ecológicos e externos no mesmo motor
- [x] Acrescentar PhysicalString/Waveguide ao despacho/ecologia após aprovação
      por escuta como quinto ser; MultiTapSpace permanece processador espacial,
      não espécie de fonte
- [x] Consolidar `Desire` (calm/density/cohesion/danger) numa camada
      `ValueTree` compatível com APVTS futuro, sem acoplar o core ao plugin.
      Os oito valores ecológicos estão em `EcologyParameterState` V1; migração
      das preferências antigas, persistência XML e teste de round-trip feitos.
      Snapshot completo e publicação coerente de cenas concluídos em
      2026-09-16; APVTS de plugin e automação sample-accurate continuam pendentes.
- [x] Expor `philosophyBlend` como quatro sliders na página `ECO CONTROLS` da
      GUI, normalizados e suavizados pelo Ecossistema
- [ ] Projetar e validar por escuta a consequência sonora de
      `AquariumIntegrity::shattered` (a máquina de estado já existe; o efeito
      audível deliberadamente ainda não)
- [x] Prototipar `AirWaterMedium`, com transição por altura, turbidez,
      temperatura, amortecimento e perda, e integrá-lo a `FLUID + PHASE`
- [ ] Promover ar/água para regiões acústicas por organismo no Ecossistema,
      depois de render e escuta do protótipo comum do banco
- [x] Medir CPU do Ecossistema junto aos motores definitivos em taxas de
      amostragem, blocos e polifonias representativos; linha de base integrada
      registrada em `CPU_BASELINE.md` (o marco inicial excluía o Master; a
      matriz de 30/09 passou a incluí-lo explicitamente)
- [ ] Realizar sessões de escuta humana dos pares A/B e registrar a calibração
      de primeiro plano, sustentação, Lupa e legibilidade das relações
      ecológicas; avaliar o feedback de energia em uma segunda rodada, fora da
      comparação controlada
- [ ] Calibrar por escuta o retorno inicial `0,36` e os seis controles do Espaço;
      registrar decisões de caráter separadamente dos critérios de estabilidade

**Evidência de escuta inicial (sessão A/B controlada, 2026-08-12):** o ouvinte
percebeu maior profundidade no take B, revelado como rota `Neutral`, mas não
identificou um take melhor: A (`Active`) e B são dois sons diferentes e ambos
são musicalmente válidos. O Ecossistema deve ser desenvolvido como modo
expressivo de performance, não como uma alegação automática de superioridade.
A calibração futura deve preservar essa diferença deliberada, controlar
mascaramento e permitir escolha clara entre os estados.

### Lupa (`MagnifyingLens`)

*A máquina canônica da Lupa está integrada ao cálculo de ganho/detalhe, com
posicionamento esférico e posse exclusiva. O gesto humano e sua memória XY no
banco estão ligados; faltam escuta, calibração e decidir automação futura.*

- [x] Controlar o alvo de `MagnifyingLens::glideTowards` por arrasto na esfera;
      `Alt` mira a meia esfera posterior, sem teleporte do estado ecológico
- [x] Ligar `tryHold`/`release` ao pressionar/soltar do mouse para o observador
      0, com posse exclusiva mantida pelo core na thread de áudio
- [ ] Decidir se `heldByObserver` pode ser automatizado pelo Cérebro (Módulo 1), ou se é sempre gesto humano direto via `AccessPort`
- [ ] Calibrar `focusRadius`/`glassReach`/`magnification`/`blurFactor` contra casos reais de organismos perto do vidro — hoje são só valores de partida
- [ ] Avaliar se o `blurFactor` deveria também atenuar filtro/brilho tonal (não só peso/presença no mix) pra reforçar a sensação de "desfocado" além do volume relativo

## Fase 9 — Parâmetros, presets e GUI

- [ ] `AudioProcessorValueTreeState` (APVTS) conectando todos os parâmetros dos módulos acima
- [x] Primeiro sistema de presets locais: quatro cenas, ampliadas de 25 para
      62 valores, com migração das cenas V1 de 25, 31, 35, 39, 42 e 46 valores, com
      GRAVAR/RECUPERAR/INICIAL, persistência nas preferências, validação
      transacional e publicação coerente ao áudio; transporte `.aquscene`
      integrado com validação limitada e backup; nomes locais de até 32
      caracteres e desfazer de um nível. Morph e
      importação/exportação do banco inteiro permanecem fora deste marco
- [x] Implementar no AQUORBIUM a primeira política de janela candidata ao padrão
      comum RASGO, conforme
      [`INTERFACES_E_LAYOUTS.md`](../../RASGO_DOCUMENTATION/design/INTERFACES_E_LAYOUTS.md),
      sem levar código ainda a RASGO Synth, Antitotem ou Navalha
- [x] Separar canvas artístico de tamanho obrigatório: primeira abertura em
      `85–90%` da área útil de um único monitor, sem ultrapassá-la
- [x] Criar regimes completo, compacto e emergência por recálculo semântico dos
      objetos; não usar escala afim global que torne texto e gestos pequenos
- [x] Persistir e revalidar monitor/bounds da janela, recuperando-a quando um
      monitor for removido, reordenado ou tiver resolução/DPI alterados
- [ ] Validar `960×600`, `1024×650`, `1366×768`, `1920×1080` e `2560×1440+`
      em `100/125/150/200%` de DPI, com um e dois monitores
      (100% já inspecionado em `720×480`, `960×600`, `1024×650`, `1280×800`,
      `1366×736` e área útil `1920×1006`; ver `WINDOW_LAYOUT_DESIGN.md`)
- [x] GUI mínima funcional: painel responsivo dos cinco cartões originais,
      depois ampliado com os cartões Ambiente e Cauda, Aquário
      central e controles Density/Drift/Ecosystem Morph
- [x] Publicar telemetria visual real por ponte de escalares atômicos; a GUI
      nunca lê diretamente o estado mutável da thread de áudio
- [x] Mostrar honestamente no painel quais módulos estão ativos, em fundação ou
      planejados, sem simular DSP ainda inexistente
- [x] Promover Geometria a módulo ativo na GUI com órbita real na telemetria e
      controles de performance conectados ao core
- [x] Promover Espaço a superfície performática real: Return, Gesture, Size,
      Spread, Tone e Memory persistidos e publicados por atomics ao
      `MultiTapSpace` compartilhado
- [ ] Transformar os cartões técnicos em superfícies de controle à
      medida que cada motor DSP for integrado
- [ ] Camada "Shift" — segunda função pro mesmo controle físico da GUI (princípio Teia Synthesizers DriftLog, seção 12), candidato pra dobrar a densidade de controle de `Desire`/`philosophyBlend`/Lupa sem multiplicar knobs — relacionado à questão em aberto de controle real da Lupa (Fase 11)
- [ ] Automação testada na DAW

## Fase 10 — Validação e acabamento

- [x] Teste automatizado de real-time safety sem alocação no render, morph e
      fila de eventos; capacidade/ordem/wrap-around/concorrência da fila cobertos
- [x] Criar benchmark offline reproduzível do core em 44,1/48/96 kHz, blocos
      32–512, autonomia ativa e zero/uma/quatro vozes externas; registrar a
      linha de base sem threshold falso
- [x] Sanear `NaN`/Inf nas entradas públicas do core e testar retenção do último
      valor válido, sample rate, velocity e render prolongado após dados inválidos
- [x] Repetir a matriz completa do benchmark Release com banco e Master ativos
      depois da coorte de refinamento; pior caso medido 62,08% em 96 kHz/bloco
      128/sem vozes externas (1,61× offline)
- [ ] Medir deadline/XRUN no dispositivo real com taxas, buffers e polifonias
      representativos depois de integrar os motores definitivos
- [ ] Repetir o teste concorrente sob ThreadSanitizer em ambiente compatível; o
      runtime atual falha antes da execução com `unexpected memory mapping`
- [x] Testes unitários exaustivos para distribuição euclidiana e quantizador
- [x] Criar testes unitários do wavefolder sine-shaping, bypass e comparação
      espectral dirigida; LUFS recebeu teste dedicado de referência em 01/10
- [x] Teste de pior caso: todos os organismos/submixes em fase e energia máxima,
      sem ultrapassar o ceiling true peak do perfil selecionado (2026-10-22,
      `EngineStressTests`: pela interface, o pior caso fica em ~−8,5 dBTP; o
      limitador, testado direto com sinais quentes, foi corrigido para true
      peak 4× e para recuperar depois de picos, `OUTPUT_STAGE_DESIGN.md`)
- [x] Testes com impulso, DC, subgrave, silêncio, `NaN`/`Inf`, seno próximo de
      Nyquist, picos intersample, blocos irregulares e troca de sample rate
      (2026-10-22, `EngineStressTests`, pela entrada da sala, que só gera
      descritores; achou e corrigiu a sala que não era ouvida com o aquário
      em silêncio)
- [x] Testar automação rápida de ganho/pan/width/bypass/mute sem clique e medir
      compatibilidade mono/correlação (2026-10-22, `AutomationClickTests`:
      15 controles trocados de uma vez. O de SÍNTESE clicava e foi suavizado;
      os outros já eram suaves. Correlação de 0,91 a 0,93 nas caixas e de 0,54
      a 0,56 nos fones; soma mono de −0,2 dB e −1,1 dB. Não há pan: a posição
      vem dos organismos. Ver `OUTPUT_STAGE_DESIGN.md`)
- [ ] Soak test do app e do plugin sem xrun, alocação, lock, frame perdido ou
      divergência entre medidor, gravação e dispositivo
      (2026-10-22: soak do engine, `EngineSoakTests`, 30 min com tudo ligado.
      Finito, abaixo do teto, nível estável, sem crescer memória (384 kB).
      CPU da thread: média de 48 % e pior bloco de 122 % em 22/10; em 23/10,
      com os denormais zerados, média de 31 % e pior bloco de 52 %, e o teste
      passa. O soak no dispositivo real segue aberto)
- [x] Desempenho do pior caso: com tudo ligado (música, síntese, banco,
      FONES) e 4 vozes de MIDI, a média é de 3,2 ms por bloco de 256 (60 %) e
      o pior bloco de 5,3 a 6,5 ms (100–122 %). Custo por recurso: base 0,38 ms;
      síntese +0,62; banco +0,64; FONES +0,2 (+0,2 com MIDI); 4 vozes de MIDI
      +1,3. Perfilar e otimizar, começando pelas vozes de MIDI (5 motores por
      voz) até o `EngineSoakTests` passar (medido em 2026-10-22)
      (2026-10-23: era quase tudo número denormal. O app já os zerava
      (`ScopedNoDenormals`), mas os testes não; agora o próprio engine liga
      flush-to-zero no `render` (`core/FlushDenormals.h`). Tudo ligado e 4
      vozes de MIDI: 1,74 ms (33 %). Soak de 30 min: média de 31 %, pior
      bloco de 52 %; `EngineSoakTests` passa. Ver `OUTPUT_STAGE_DESIGN.md`)
- [x] Cabeçalho de licença AGPLv3 em todos os arquivos-fonte (2026-10-22: SPDX
      `AGPL-3.0-or-later` e `Copyright (C) 2026 Lúcio de Araújo` nos 268
      arquivos C/C++ de `src/`, `tests/` e `tools/`; o teste
      `aquorbium_license_headers` exige o cabeçalho também nos arquivos novos)
- [x] Documentação de uso atualizada no README (2026-10-23: "Como tocar — guia
      rápido" no topo: abrir, a tela, começar, a síntese, cenas, REC e as chaves
      dos settings)

## Fase 11 — Conceito (paralelo, não bloqueia o código)

- [ ] Decidir o que é "primeiro plano sonoro" vs "estrutura que só sustenta" (ver seção 1 do doc de arquitetura)
- [ ] Trazer os conceitos complementares ainda não desenvolvidos (ambiente ar/água, densidades)
- [ ] Atualizar `aquorbium-arquitetura.md` conforme decisões forem tomadas

## Banco de hipóteses e implementações pesquisadas

As 64 tarefas abertas reunidas aqui vieram de rodadas de pesquisa e continuam
disponíveis para seleção experimental. Elas não são módulos JUCE já integrados
nem requisitos simultâneos da primeira publicação. As fichas, fontes e
diferenças autorais estão em [aquorbium-arquitetura.md](aquorbium-arquitetura.md),
seções 9–12. Duplicidades de princípio devem ser comparadas antes de programar
duas soluções para a mesma função.

### ModularGrid Top 100 — candidatos por função

*Ver seção 9 de `aquorbium-arquitetura.md` pra ficha técnica e referência completa de cada módulo real.*

- [x] Prototipar `ShiftRegisterMemory` (princípio Turing Machine Mk II) com 1–32 bits, seed e mutação; ligação ao BrainEngine e mapa de `Desire::calm` permanecem pendentes
- [x] Prototipar distribuição de probabilidade uniforme→gaussiana (princípio SAPÈL) com consumo aleatório invariável; mapa de `intelligence` permanece pendente
- [x] Prototipar máquina aleatório→looping→evolving→travado (princípio RANDOM8) com memória fixa; aplicação à personalidade permanece pendente
- [x] Prototipar máquina de oito estágios com salto condicional (princípio Rossum Control Forge) por perigo, noite ou trinca; semântica e ligação ao Ecossistema permanecem pendentes
- [x] Prototipar `FractalCore` único (princípio Blukač Fractalist) com a mesma API e trajetória de fase em taxa de controle e áudio; ligação ao BrainEngine/SynthFMVoice permanece pendente
- [x] Prototipar `VectorGenerator` único (princípio Mannequins Just Friends) como envelope/LFO/oscilador conforme modo e registro; compartilhamento por cardume permanece pendente
- [x] Prototipar 4 modos de saída (formas/amplitudes/tempos/frequências, princípio Mutable Tides) em `FluidMotion`, derivados de uma fase mestra; ligação a cardumes permanece pendente
- [x] Prototipar `BoundedRoutingMatrix` 5×5 normalizada e modulável em tempo real; o banco fornece a primeira rota contextual, enquanto edição por `Desire`/`AccessPort` permanece pendente
- [x] Prototipar 3 instâncias de Slew-Rate Random Walk em segundos/minutos/dezenas de minutos (princípio Nonlinearcircuits Triple Sloths), com seeds próprias e combinação ponderada; ligação ecológica permanece pendente
- [x] Prototipar autopatch de uma saída de `FluidMotion` na velocidade (princípio DivKid ochd), limitado a 0,1×–4× e invariante quando zerado; integração pendente
- [x] Prototipar crossfade suave de potência constante (princípio Bastl Déjà Vu) entre estados saudável↔trincado; ligação ao `AquariumIntegrity` permanece pendente
- [x] Prototipar `DoubleScrollAttractor` autoral, determinístico, tridimensional
      e com pares complementares (referência conceitual: Joranalogue Orbit 3),
      integrado experimentalmente a `LORENZ + DRIFT`; comparação musical A/B
      com `LorenzCV` permanece pendente

### Ecossistema e pesquisas de catálogo

- [x] Prototipar travamento por frações inteiras (princípio Mannequins Just Friends) em `HarmonicPhaseBank`, com uma fase mestra e até oito membros; substituição de `SchoolInfo::sharedPhase` permanece pendente
- [x] Prototipar FREQ/SPAN como centro logarítmico e dispersão em oitavas no `SchoolSpectralBank` (princípio Three Sisters); primeira integração global concluída, ligação aos campos reais de `SchoolInfo` permanece pendente
- [x] Prototipar `SpectralTransferVocoder` autoral de seis bandas, transferindo
      envelopes do mix para um corpo interno; promoção especializada ao
      BiomaBass permanece pendente e não constitui emulação do FUMANA

### Rodada EMW (seção 12 de `aquorbium-arquitetura.md`)

- [x] Prototipar `SchoolWaveVoice`, gerador único de seis membros com offsets
      derivados de direção/fase compartilhadas; ligação ao `SchoolInfo` real
      permanece pendente
- [x] Prototipar `WAVE CUT` como terceiro eixo de caráter do `SchoolWaveVoice`;
      promoção para `Species::Oscillator` depende de escuta
- [x] Prototipar gesto humano XY gravado e reproduzido em loop em `MotionGestureRecorder` (princípio EMW Pot Action Recorder), com 2.048 pontos fixos e velocidade variável; captura da Lupa/XY e ligação ao `FluidMotion` permanecem pendentes
- [x] Prototipar banco de oito passa-bandas como assinatura espectral determinística por cardume em `SchoolSpectralBank` (princípio EMW Fixed Filter Bank); primeira integração global concluída, diferenciação por cardume permanece pendente
- [x] Prototipar atmosfera de ruído cujo morph é derivado de estação e hora em `EcologicalNoiseBed` (princípio EMW Noise Cinema); ligação a `Environment` concluída na primeira integração
- [x] Prototipar redução temporal por sample-and-hold do leito ambiental governada por `crackStress`, de 1 a 64 amostras (princípio Noise Cinema); ligação a `AquariumIntegrity` concluída na primeira integração
- [x] Prototipar `philosophyBlend` sequenciado em `PhilosophySequencer`, com até 16 passos XY e duração por pulso (princípio EMW Mix Sequencer); relógio do Cérebro e integração permanecem pendentes
- [x] Prototipar banco de oito formas de onda em `MotionWaveBank` e reutilizá-lo no `FluidMotion`, com associação determinística por identidade (princípio EMW VC Wavetable LFO); mapa real de `Species`/`SurvivalStrategy` permanece pendente
- [ ] Entrada de microfone real como único ponto onde "nada de fora entra" deixa de valer de propósito (princípio EMW Mic. Interface) — a energia do envelope da voz/ambiente **estressa `AquariumIntegrity::crackStress`**, não entra como canal de mixagem direto
- [x] Prototipar ring modulation momentânea e suavizada no `InteractionProcessor`; ligação ao contato real de `PredatorStrategy` permanece pendente
- [x] Prototipar controle de riqueza harmônica do artefato de contato no `InteractionProcessor`; derivação direta de wavefold/forma dos organismos reais permanece pendente
- [x] Prototipar `SemanticAccessRouter` de oito portas e 24 rotas fixas entre
      áudio, controle/modulação e evento, com conversão por envelope e somas
      limitadas; primeira rota contextual integrada a `BRANCH + PERMUTE`
- [ ] Promover o roteamento semântico ao `AccessPort` editável: qualquer saída
      de `Organism` pode ser voz ou fonte de modulação conforme o destino,
      preservando soft take-over, limites e estado por patch (princípio
      Soundfreak Triple VCO, herança da matriz VCS3)
- [x] Prototipar `EnvelopeContagionBus` de seis membros, com propagação limitada
      aos vizinhos; ligação às envelopes e ao `schoolId` reais permanece
      pendente
- [x] Prototipar cor do leito ambiental por corte derivado de turbidez e temperatura em `EcologicalNoiseBed` (princípio Soundfreak Envelope Shaper/Noise Generator); primeira ligação às estruturas ecológicas concluída

### Rodada DriftLog (Teia Synthesizers, seção 12)

- [x] Prototipar `DriftHistoryLoop` com captura contínua e repetição de janela
      ajustável sem congelar a deriva; o banco já o usa em `PATTERN + LOOP`,
      enquanto aplicação por organismo e `isMatingSeason()` permanecem pendentes
- [x] Prototipar `MovementSignatureLock`, congelando suavização, família de
      curva e probabilidade enquanto o restante varia; o banco já o usa em
      `INTEGRITY + MEMORY`, mas o lock individual por organismo segue pendente
- [x] Prototipar `PriorityChoke` com limiar e desempate determinístico por
      prioridade; o banco já o usa para legibilidade entre fontes, enquanto a
      hierarquia real de organismos/filosofias segue pendente

### Rodada SchneidersLaden — páginas 6-15 (seção 12)

- [x] Prototipar `ContactCrackleExciter` estéreo de estado fixo, sensível a
      posição, pressão, velocidade e entropia, com saída limitada e bypass
      exato; eventos ecológicos o tornam audível em `GESTURE WAVES`
- [ ] Ligar o exciter a um gesto GUI de dois pontos no vidro, distinto da Lupa,
      usando jitter físico real do dispositivo para variar o contato sem
      prometer reprodução idêntica (princípio ADDAC708/Michel Waisvisz)
- [x] Prototipar `FrequencyIsolator` com foco integral numa faixa e bypass neutro; empilhamento com a Lupa e seleção de organismo permanecem pendentes
- [x] Prototipar variante XOR por polaridade no `InteractionProcessor`; seleção definitiva por `SurvivalStrategy` permanece pendente
- [x] Prototipar `PowerStarve` e ligá-lo experimentalmente a `AquariumIntegrity::crackStress`; aplicação por organismo específico permanece pendente

### Rodada SchneidersLaden — primeira página do catálogo (seção 12)

- [x] Prototipar matriz 4×4 modulável em `PhilosophyMatrixMixer`, formando duas mixagens estéreo independentes das mesmas quatro fontes (princípio SetonixSynth Shaka-8/Shaka-4); substituição de `philosophyBlend`/pan e varredura autônoma permanecem pendentes
- [x] Prototipar duck suavizado de perseguição no `InteractionProcessor`; fontes reais de predador e presa ainda precisam substituir os sinais experimentais do banco
- [ ] Formalizar a brecha externa de `AquariumIntegrity` com três pontos de controle separados (quanto sai, quanto volta, quanto se mistura), todos amarrados a `crackStress`, em vez de um simples on/off de áudio externo (princípio earthlink modular FLUME)
- [ ] (exploração) `Desire`/BrainEngine varrendo o parâmetro de expressão de um pedal físico real via CV, puxando hardware que Lúcio já tenha pra dentro do patch (mesmo princípio FLUME)
- [x] Prototipar `SpotlightMixer`: foco automático, fundo preservado e
      redistribuído em estéreo e duck limitado; integração no nível real do
      `Ecosystem`, complementar à Lupa, permanece pendente
- [x] Prototipar `HungerProcessor`: densidade e escassez reduzem alimentação e
      introduzem falhas determinísticas limitadas; aplicação por `Organism`
      permanece pendente
- [x] Prototipar `NoInputFeedbackBus` com três regiões filtradas, saturação e
      realimentação interna limitada; promoção ao Master, compressor próprio e
      ligação a `AquariumIntegrity::shattered` permanecem pendentes

### Rodada SchneidersLaden — páginas 16-25 (seção 12)

- [x] Prototipar `ReactiveThresholdStrategy` com oito estados, limiares e
      histerese; substituição da transição real de `SurvivalStrategy` permanece
      pendente
- [x] Prototipar controle de cluster formântico no `HarmonicRippleVoice`, como
      voz única de múltiplas ondulações; integração especializada ao Pulsar e
      ao `SchoolInfo` permanece pendente

### Rodada SchneidersLaden (seção 12 — sensores físicos, páginas 16-25)

- [ ] Avaliar KOMA Elektronik Sensei (luz/movimento/toque) e Bela Gliss 1U (toque + gravação de movimento) como entradas concretas de `Environment`, junto da linha ADDAC 308-311 já registrada — sem princípio novo, mas confirma que sensor físico real é uma família de hardware inteira, não um caso isolado

### Rodada SchneidersLaden — páginas 26-45 (seção 12)

- [x] Prototipar `OpenRuleProgram`, gramática limitada a 16 regras compiladas
      transacionalmente fora do callback e avaliadas sem alocação; primeira
      consequência integrada a `EVOLVE + STAGES`
- [ ] Expor o programa de regras como texto editável ao vivo via GUI/APVTS,
      com publicação atômica, diagnóstico de erro e recuperação segura sem
      parar o áudio (princípio Emute Labs uSeq/modulisp)
- [x] Prototipar `LockInFocusExtractor` por referência de frequência e fase;
      seleção de um `Organism` real e promoção como terceiro modo do Observer
      permanecem pendentes
- [x] Prototipar `FrequencyShifter` com FIR de Hilbert fixa, deslocamento linear
      em Hz e bypass seco exato; associação definitiva a BiomaBass/`Species`
      permanece pendente
- [x] Prototipar `QuadratureStereoRotator` com preservação de potência e usá-lo em `FLUID + PHASE`; ligação direta a `AquariumTransform` permanece pendente
- [x] Árvore ramificada e `RandomToLoop` possuem protótipos isolados cobertos
      por testes; esta entrada duplicada fica encerrada em favor da Fase 3
- [x] Prototipar `PresenceStateLatch`, mantendo presença/postura até um evento
      ecológico configurado; o banco já o usa em `INTEGRITY + MEMORY`, enquanto
      timbre por ramo e ligação individual ao organismo permanecem pendentes

### Rodada SchneidersLaden — páginas 46-65 (seção 12)

- [x] Prototipar mistura XY bilinear entre quatro cantos em `PhilosophyVectorMixer` (princípio RYK Modular Vector Wave); identidade e posição das filosofias reais permanecem pendentes
- [x] Gravar a 200 Hz o **trajeto 2D** real que
      `MagnifyingLens::glideTowards` percorre durante a posse e reproduzi-lo em
      loop em `GESTURE WAVES` após soltar, usando memória fixa de 2.048 pontos
      (mesmo princípio RYK Vector Wave, gravação de CV do joystick)
- [x] Prototipar `SharedParameterPool` com 16 valores, oito consumidores e leitura direta/rotativa/sustentada; múltiplos `GeometricSequencer` e `schoolId` como consumidores reais permanecem pendentes
- [x] Prototipar em `HarmonicRippleVoice` ondulações harmonicamente travadas à
      fundamental; promoção como segunda variante de `Species::Pulsar` depende
      de escuta
- [x] Prototipar `BoundedSelfFeedback` com realimentação filtrada, saturada e
      limitada; inserção dentro de um `Organism` específico permanece pendente
- [ ] Estágio de espacialização binaural real (tipo HRTF) por `Organism` ou por mix de `Observer`, com ouvinte virtual no centro do aquário ou na posição de um `Observer` — posição em ângulo/altura/distância CV-controlável a partir de `FluidMotion`/`worldPosition`, grave ancorado num canal central sem processamento HRTF enquanto harmônicos altos se movem ao redor da cabeça (princípio Neuzeit Instruments Quasar, mixer binaural 3D)
  - `LightweightBinauralSpatializer` já prototipa ITD, sombra de cabeça e grave
    central no banco; HRTF medida e espacialização por organismo continuam
    explicitamente fora desse protótipo.
  (2026-10-21: feito em parte. O seletor CAIXAS/FONES usa o modelo
  estrutural de cabeça de Brown & Duda por organismo, com o ouvinte no
  centro; faltam a HRTF medida e o ouvinte na posição de um `Observer`.)

### Rodada SchneidersLaden — páginas 66-85 (seção 12)

- [x] Prototipar `BehaviourSceneBank` com oito cenas de quatro dimensões e
      transição suavizada; nomes, persistência, recuperação manual/CV e ligação
      ao `Ecosystem`/`SchoolInfo` permanecem pendentes
- [x] Prototipar crossfade de duração definida entre quatro filosofias discretas em `PhilosophyMorph`, cruzando suas saídas em vez do enum (princípio Harlequin's Context); banco de cenas e ligação ao Ecossistema permanecem pendentes
- [x] Prototipar variante "muta levemente a cada repetição" no `MotionGestureRecorder`, com seed, repetição exata em zero e perturbação limitada a 5% por volta (princípio u-he CVilization); integração ao DriftLog, `FluidMotion` e GUI permanece pendente
- [x] Prototipar `ReconfigurableResourcePool` com os modos mixer, switch,
      recorder e router; consumidores reais do `GeometricSequencer` continuam
      pendentes
- [x] Prototipar `PolygonalOscillator`, cujo áudio deriva literalmente da geometria; mapeamento da forma visual futura de cada `Organism` permanece pendente
- [x] Publicar saídas X/Y literais no `PolygonalOscillator`; encaminhamento a osciloscópio/laser/vídeo externo permanece pendente
