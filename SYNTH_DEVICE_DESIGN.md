# Aquorbium — o dispositivo de síntese (proposta de 2026-10-21; completo em 2026-10-22)

**Estado (2026-10-22):** completo. As fases 1–6 estão feitas, mais os patches nas cenas, o FORMANTE e o FM 4 operadores, cada uma com a sua seção abaixo. Pendente, fora do código: o uso real pelo autor e a revisão nativa FR/ES.

## Origem da decisão

O autor, depois de escutar os corpos ARCO e PALHETA:

> acho melhor que o aquorbium tenha um dispositivo de síntese sonora
> inteligente, sólido e autônomo, como fizemos com o rasgo modular ou o
> antitotem. Melhor do que você ficar criando timbres. Os timbres são
> consequência de ferramentas bem desenvolvidas de síntese sonora no
> instrumento e o músico que utilizar o instrumento cria os sons.

**Consequência:** o Aquorbium deixa de receber timbres prontos e ganha uma
ferramenta de síntese. Quem cria o som é o músico. O ecossistema continua
decidindo quando e o que soa (as notas da música ecológica) e passa
também a mover os parâmetros da síntese.

## O que já existe e será aproveitado

- **Rasgo Modular** (`PROJETOS_AUTORAIS/RASGO_MODULAR/`, repositório
  próprio, AGPLv3):
  - 66 módulos DSP sem framework em `src/dsp/`, cada um com um dossiê de
    pesquisa em `dossies/`: OSC (formas, PWM, sync, FM through-zero,
    sub, drift), OPERATOR (FM), WAVETABLE, FORMANT, ADDITIVE, PLANAR,
    PULSAR, STRING, MATTER, RESONATOR, NOISE, FILTER, LPG, SHAPE, CRUSH,
    ENVELOPE e FUNCTION, entre outros.
  - Eles dependem da abstração de portas do Modular
    (`core/SignalGraph.hpp`). Pela regra de autonomia
    (GOVERNANCA §7.0), copia-se o **núcleo de processamento** de cada
    módulo, a partir de um commit conhecido, com proveniência (origem,
    commit, checksums, licença, créditos), adaptado a classes do
    Aquorbium. Nada de depender da pasta irmã no build.
- **Antitotem:** o modelo de painel fixo, com caminho de sinal legível
  (fontes → filtro → espaço → mixer), e o "três modos obrigatórios" do
  `MODULE_DEVELOPMENT_STANDARD` (autônomo, performance, híbrido).
- **Aquorbium:** o motor ecológico, as notas da música ecológica (altura,
  acento, duração, release, glissando), os corpos nativos e os
  `TimbreBody`, que continuam disponíveis como fontes.

## O dispositivo: uma voz de síntese por organismo

Um caminho semimodular fixo, como no Antitotem, com peças escolhíveis,
como no Modular:

```text
FONTE A ─┐
         ├─ MISTURA (crossfade · ring · FM A→B) ─► FORMA (dobra · crush)
FONTE B ─┘                                           │
                                                     ▼
                         FILTRO (SVF LP/BP/HP/notch · LPG · formante)
                                                     │
                                       VCA ◄── ENVELOPE AMP
                                                     ▼
                         para o espaço do aquário (estéreo ou fones)
```

- **Fontes** (A e B, cada uma escolhida numa lista): OSC, OPERATOR,
  WAVETABLE, FORMANT, ADDITIVE, PULSAR, STRING, MATTER, NOISE, o corpo
  NATIVO da espécie e os corpos `TimbreBody`.
- **Envelopes:** dois, um de amplitude e um de modulação, no estilo
  FUNCTION: ADSR, AD ou em laço. São disparados pelas notas do
  ecossistema.
- **Moduladores próprios:** dois LFOs, com o `drift` do Modular.

## O que o faz inteligente: a matriz ecológica

Uma matriz de modulação de 8 vias, fonte → destino × quantidade.

**Fontes de modulação:**
- os envelopes e os LFOs;
- os **sinais do ecossistema**, cada um já calculado e normalizado:
  - a energia e a velocidade do organismo;
  - a intensidade da relação que ele vive (caça, fuga, corte…);
  - a tensão harmônica;
  - a distância até o ouvinte e a altura no canal SOFAR;
  - a temperatura e o estado da água;
  - a fome e a idade (`LifeCycle`);
  - a abundância de presas;
  - a saúde do habitat;
  - o ruído da sala (SALA).

**Destinos:** qualquer parâmetro da voz.

O músico decide o que liga a quê e quanto. O ecossistema toca os
parâmetros: a caça abre o filtro, a fome escurece, a fervura dobra a
onda. O som nasce da ferramenta e da vida do aquário, não de um timbre
fixo.

## Autonomia: o instrumento propõe, o músico decide

O botão **PROPOSTA** gera, para cada organismo, um patch de partida a
partir da espécie e da ecologia: fontes, filtro, envelopes e algumas
ligações da matriz. É determinístico pela semente. O músico edita a
partir daí.

Isso segue a identidade do Modular: o instrumento não "toca sozinho no
lugar do músico", ele propõe pontos de partida. Os patches ficam nas
cenas, e é possível copiar o patch de um organismo para outro.

## Contrato

- **SÍNTESE em 0 dá o som atual, idêntico bit a bit.** A mistura com o
  corpo nativo é um controle.
- Seguro no áudio: memória fixa, sem alocação, lock nem I/O; custo
  medido (5 vozes).
- Cada peça tem testes do conceito: afinação, banda limitada (aliasing),
  tempos de envelope, limites e determinismo.
- Nenhum timbre é "aprovado por escuta" do autor como critério de pronto.
  O critério é a ferramenta: precisa, estável, documentada, tocável.

## Fases de entrega

1. **Voz mínima:**
   - fontes OSC e NOISE e o NATIVO;
   - o filtro SVF multimodo;
   - os dois envelopes e os dois LFOs;
   - disparada pelas notas;
   - o controle de mistura SÍNTESE;
   - testes.
2. **A matriz ecológica**, com as fontes do ecossistema e testes de que
   cada sinal chega normalizado.
3. **As fontes do Modular** (OPERATOR, WAVETABLE, FORMANT, ADDITIVE,
   PULSAR, STRING, MATTER) e FORMA/LPG, copiadas com proveniência e
   adaptadas.
4. **A interface:** o editor por organismo (seletor de organismo, seções
   FONTES / FORMA / FILTRO / ENVELOPES / MODULAÇÃO), persistência e cenas.
5. **PROPOSTA:** os patches de partida generativos por espécie.
6. **A documentação,** o tutorial, o LEARN e o Sobre.

## Decisões do autor (2026-10-21)

- **Os corpos atuais viram fontes dentro do dispositivo:** o NATIVO de
  cada espécie e os `TimbreBody` (voz, matéria, sopro, corda, gota, dobra,
  membrana, vidro, coro, arco, palheta). Nada se perde.
- **A interface é um editor sobreposto por organismo,** como TUTORIAL e
  SOBRE, com seletor de organismo e as seções FONTES / FORMA / FILTRO /
  ENVELOPES / MODULAÇÃO. A tela principal não fica mais apertada.


## Fase 1 feita — 2026-10-21: a voz mínima

`src/core/SynthVoice.*`, uma voz por organismo, no motor
(`BiomaEngine::setSynthMix`, `setSynthPatch`).

**O que a voz tem:**
- **FONTES A e B:**
  - oscilador, com FORMA de seno a triângulo, dente-de-serra e pulso,
    largura de pulso, suboitava, afinação, ajuste fino e drift;
  - ruído, com COR de branco a rosa (Kellet) e marrom;
  - o NATIVO, o próprio corpo do organismo.
- **A mistura:** os níveis, o ring (A × B) e a FM linear de B em A.
- **O FILTRO:** SVF TPT (Simper) passa-baixas, passa-banda, passa-altas
  ou rejeita-faixa, com ressonância, keytrack, envelope de modulação (em
  oitavas) e drive.
- **Os dois envelopes ADSR** com curva, e **os dois LFOs** (seno,
  triângulo, quadrada, deriva suave), por enquanto em duas rotas fixas:
  vibrato (LFO 1 → altura) e wah (LFO 2 → corte). A matriz é a fase 2.
- **O portão:** as notas da música ecológica abrem e fecham a voz (o
  acento pesa no nível); sem música ecológica, a energia do organismo
  abre o portão.
- **A água:** a voz passa pelo mesmo filtro de absorção do corpo nativo.

**Proveniência:** os núcleos do oscilador, do SVF, do envelope e do ruído
rosa foram copiados do Rasgo Modular (commit 4ce898f, AGPLv3, mesmo
autor), adaptados, com as checksums no cabeçalho de `SynthVoice.h`.

**Testes (`SynthVoiceTests`):**
- a afinação, ≤0,06 cent no seno; o dente-de-serra dá até 1,5 cent a
  1760 Hz, um viés do medidor nas bordas;
- a banda limitada, com aliasing 37,6 dB abaixo a 2489 Hz;
- a quadrada sem pares, com a terça a 1/3; o dente-de-serra com o 2º
  harmônico a 1/2; o seno puro;
- as cores do ruído: plano, −2,5 e −5,4 dB por oitava;
- o filtro: −25 dB duas oitavas além do corte, o passa-banda unitário no
  centro e o rejeita-faixa abaixo de −30 dB;
- os tempos de envelope: ataque de 99,9 ms para 100 e release de 300,0 ms
  para 300;
- o NATIVO passa intacto;
- extremos finitos e determinismo;
- no motor, SÍNTESE 0 é idêntico bit a bit e SÍNTESE 1 soa e fica finita.

**Medido no motor:**
- o nível da síntese, com o patch padrão, iguala o nativo (±0,35 dB);
- o custo das cinco vozes é de ~305 µs por bloco de 256 amostras (o motor
  inteiro fica em ~650 de 5333 µs), com os coeficientes recalculados a
  cada 8 amostras.

**Ainda não há interface:** SÍNTESE fica em 0 até a fase 4 (o editor).


## Fase 2 feita — 2026-10-21: a matriz ecológica

São 8 vias, cada uma fonte → destino × quantidade (−1..1).

**Fontes (18):**
- nenhuma;
- da voz: envelope de amplitude, envelope de modulação, LFO 1, LFO 2 e o
  acento da nota;
- do ecossistema, normalizados 0..1 pelo motor na taxa de controle:
  - a energia e a velocidade do organismo;
  - a relação mais forte que ele vive;
  - a tensão harmônica;
  - a distância até o ouvinte, pelo caminho na água;
  - a presença no canal SOFAR;
  - a temperatura da água;
  - a fome e a idade;
  - as presas;
  - a saúde do habitat;
  - a sala.

**Destinos (18):** nenhum, altura (±12 semitons), corte (±4 oitavas),
ressonância, drive, forma A/B, largura A/B, nível A/B, ring, FM, cor
A/B, nível geral (×(1±1)) e a velocidade dos LFOs (×2^(±3)). As vias
somam e são limitadas à faixa de cada parâmetro.

**Testes:**
- a energia ligada ao corte abre o filtro (o 20º harmônico de −60 para
  −28 dB); sem a via, nada muda;
- o LFO quadrado ligado à altura dá exatamente ±100 cents (+100,2 e
  −99,9);
- todas as vias no extremo ficam limitadas.

**Medido no motor completo** (120 s), cada sinal se move:

| Sinal | Faixa |
|---|---|
| energia | 0–1, depois da escala ×1,9; sem ela, pico em 0,53 |
| velocidade | 0,01–0,97 |
| relação | 0–0,82 |
| tensão | 0–0,62 |
| distância | 0,02–0,98 |
| canal | 0–1 |
| temperatura | 0,47–1 |
| fome | 0–1 |
| idade | 0–0,72 |
| presas | 0,24–0,83 |
| habitat | 0,50–0,77 |
| sala | 0, desligada |


## Fase 4 feita — 2026-10-21: o editor

Feita antes da fase 3, para o dispositivo já ser tocável.

**No cartão TIMBRE,** uma linha nova: o controle SÍNTESE, a mistura de
todas as vozes com os corpos (0 = o som de antes), e o botão EDITAR.

**O editor sobreposto** (`src/app/SynthEditor.h`), como TUTORIAL e
SOBRE:
- o seletor dos cinco organismos;
- SÍNTESE;
- COPIAR PARA TODOS;
- as seções FONTE A e B (fonte, forma, largura, afinar, fino, sub, cor,
  nível), MISTURA (ring, FM B>A, deriva, acento, nível), FILTRO (modo,
  corte, ressonância, nota, envelope, drive), ENVELOPE AMP e ENVELOPE
  MOD (ataque, queda, sustento, soltura, curva), LFO 1 (forma, veloc.,
  vibrato) e LFO 2 (forma, veloc., wah);
- a MODULAÇÃO, com 8 vias de fonte, destino e quantidade;
- tudo nos quatro idiomas.

**Da interface ao áudio:** os cinco patches e a mistura vão juntos por
um `FixedLatestSnapshot`, sem trava, aplicados no callback e reaplicados
a cada `prepareToPlay`.

**Persistência:** `synth.mix` e `synth.patch.N` nas configurações, no
formato de texto do `SynthPatchCodec` (chave=valor, tolerante a chaves
novas e limitado às faixas; testado em `SynthPatchCodecTests`).

**Conferido no app real** (configurações isoladas, um clique conferido no
EDITAR): o editor abre com todas as seções e a matriz.

**Pendente:**
- os patches nas CENAS (hoje ficam só nas configurações);
- a fase 5 (PROPOSTA);
- a fase 6 (tutorial e LEARN).

## Fase 3 feita — 2026-10-22: as fontes do Rasgo Modular e os corpos

Cada fonte (A e B) escolhe agora entre onze: DESLIGADA, OSCILADOR, RUÍDO,
NATIVA, **FM, WAVETABLE, ADITIVA, PULSAR, CORDA, MATÉRIA** e **CORPO**.
Os três controles da seção (FORMA, LARGURA, COR) mudam de sentido conforme
a fonte, e o editor troca os rótulos:

| Fonte | FORMA | LARGURA | COR |
|---|---|---|---|
| FM | razão B/A (tabela musical) | índice | realimentação de A |
| WAVETABLE | posição (dente > quadrada > formante > seno) | warp de fase | deriva |
| ADITIVA | inclinação | ímpares/pares | estiramento (inarmônico) |
| PULSAR | formante (0,1x a 8x a nota) | forma do pulsaret | janela |
| CORDA | amortecimento | ponto da palhetada | sustento |
| MATÉRIA | estrutura (harmônica > estirada) | brilho | amortecimento |
| CORPO | — (escolhe-se o corpo: os onze do instrumento, inclusive ARCO e PALHETA) | | |

As fontes com ataque (CORDA, MATÉRIA, CORPO) são excitadas no início de
cada nota do ecossistema, com o acento.

**Proveniência:** núcleos copiados do Rasgo Modular do mesmo autor
(commit `4ce898f`, AGPLv3), com sha256 dos arquivos de origem no
cabeçalho de `src/core/SynthSources.h`: `Operator.hpp` (dois dos quatro
operadores), `Wavetable.hpp` (receita de 16 quadros, 10 mip-maps, sem
captura), `Additive.hpp` (64 parciais, sem o pente), `Pulsar.hpp`
(Curtis Roads, 4 grãos), `StringVoice.hpp` (Karplus-Strong com all-pass
fracionário) e `Matter.hpp` (24 modos). Adaptados a classes simples, com
memória fixa e sem alocação no áudio. O FORMANT ficou para depois, como
modo de filtro.

**Medido nos testes** (`tests/SynthVoiceTests.cpp`):
- FM com índice 0 é um seno puro (H2 −113 dB); com índice, aparecem as
  bandas laterais na razão escolhida;
- WAVETABLE: dente de serra com harmônicos e seno puro nas pontas;
  alias −140 dB numa nota aguda;
- ADITIVA: a inclinação escurece, ÍMPARES zera os pares, o estiramento
  desloca as parciais;
- PULSAR: o centróide sobe de 1,2 para 5,7 vezes a nota com o
  formante, e a altura fica na nota;
- CORDA: afinação em 0,43 cent e decaimento conforme o sustento;
- MATÉRIA: o 2º modo vai de 400 para 422 Hz com a estrutura;
- CORPO PALHETA: ímpares dominantes, como no corpo nativo;
- todos os extremos limitados e finitos.

**Correções durante a fase:**
- a LARGURA era limitada a 0,02–0,98 (resto do pulso do oscilador) —
  deixava o índice da FM e o warp sempre acima de zero. Agora é 0–1 na
  voz e no `SynthPatchCodec`; o oscilador limita a largura do pulso
  sozinho;
- a CORDA saía 4,4 cents baixa: o atraso do all-pass agora é compensado.

**CPU** (5 vozes, as duas fontes iguais, notas sustentadas, bloco de 256
amostras a 48 kHz, orçamento 5333 µs): a ADITIVA custava 2656 µs em
média e 3532 µs no pior bloco. Três mudanças:
- a voz não calcula nada entre notas, depois que o envelope termina;
- a aditiva soma só as parciais acima de −80 dB;
- o passo de fase não usa `floor`.

Depois: ADITIVA 1360 µs em média e 2138 µs no pior bloco (40 % do
orçamento, no caso extremo de dez aditivas). O OSCILADOR fica em ~600 µs
e o CORPO em ~650 µs.

**Validação:** a suíte inteira passa (65/65) e o app foi recompilado
(binário de 2026-10-22 02:09). A interface com as fontes novas não foi
clicada no app nesta fase.

**Pendente:** fase 5 (PROPOSTA), os patches nas CENAS, fase 6
(tutorial e LEARN) e o FORMANT como modo de filtro. (A fase 5 foi feita
logo depois; ver abaixo.)

## Fase 5 feita — 2026-10-22: PROPOSTA

Dois botões no editor: **PROPOSTA** (patch de partida para o organismo
escolhido) e **PROPOR TODOS** (os cinco de uma vez). Cada clique faz um
novo sorteio. O primeiro sorteio da sessão é a própria semente da
sessão. Com SÍNTESE em zero, a mistura sobe para 0,5, porque proposta
que não se ouve não serve.

O instrumento propõe; quem decide é o músico. É a identidade do Rasgo
Modular. No Modular a ideia está registrada (`RASGO_MODULAR.md`,
"sugestão por seed"), mas ainda sem código. Por isso o gerador foi
escrito aqui: `src/core/SynthProposal.h`.

**De onde vem cada escolha:**
- **Espécie:** famílias de fonte, gesto do envelope e registro do
  filtro.
  - MORPH: sustentado, fontes melódicas (oscilador, wavetable, FM,
    aditiva), passa-baixas que abre.
  - GRAIN: percutido, sem sustento; matéria, corpos gota, vidro e
    matéria, ruído e pulsar.
  - MODAL: ressonante, queda longa; matéria, corpos vidro, membrana e
    coro, aditiva levemente estirada.
  - PULSAR: formante que se move, LFO nervoso no filtro.
  - STRING: dedilhada ou friccionada; corda, corpos corda, arco e
    palheta, oscilador com vibrato.
- **Estratégia ecológica:** a primeira ligação da matriz.
  - predador: fome → corte;
  - territorial: tensão → ressonância;
  - migrante: distância → corte (negativo);
  - presa: população de presas → LFO 2;
  - séssil: saúde do habitat → corte.
  - Depois vêm uma ou duas ligações gerais sorteadas.
- **Semente:** cada escolha dentro dessas faixas (SplitMix64). A mesma
  semente dá o mesmo patch.

O fonte NATIVO pode aparecer como fonte B, para manter um fio com o som
da espécie. Nada aqui é preset fixo: a espécie dá a influência e a
semente dá a partida.

**O instrumento escuta o que propõe.** Cada candidato toca uma nota de
teste: 0,6 s a 220 Hz com acento 0,8, medida pelos 10 ms mais altos
(RMS).
- Um candidato quase mudo cede lugar ao próximo sorteio da mesma
  semente, em até 8 tentativas. Exemplos: passa-banda longe da fonte,
  canto silencioso de um corpo.
- O candidato escolhido recebe o NÍVEL que o leva a −14 dBFS, com pico
  ≤ 1,5.

O critério é medido, não é gosto. O custo é de alguns milissegundos por
clique, na thread da interface.

**Medido (`tests/SynthProposalTests.cpp`, 120 sementes × 5 organismos =
600 propostas):**
- determinismo: a mesma semente dá o mesmo patch;
- 600 patches distintos;
- todo patch fica dentro das faixas do codec;
- o gesto de cada espécie e a ligação de cada estratégia;
- todas audíveis: a mais baixa fica em −20 dBFS;
- todas finitas e com pico ≤ 1,5;
- 600 de 600 a menos de 6 dB do alvo;
- as onze fontes aparecem.

**Dois bugs da fase 3 achados por esses testes, com regressões em
`SynthVoiceTests` (conferido: os testes falham sem o conserto):**
- **Corpo percutido mudo na primeira nota.**
  - Causa: o corpo era excitado antes de receber seu tipo (o
    crossfade ainda estava no nativo).
  - Conserto: `TimbreBody::setKindNow`. O tipo é fixado antes da
    excitação, sem crossfade no ataque ou numa voz em silêncio.
- **FM B→A sobre fonte ressonante explodia até 1e37.**
  - Causa: um ressonador de dois polos re-afinado a cada amostra não
    conserva energia.
  - Conserto: a FM age só nas fontes osciladoras. CORDA, MATÉRIA e
    CORPO mantêm a afinação; a modulação deles fica com RING e a
    matriz.
  - Proteção adicional: uma fonte que sai do finito (ou passa de 64) é
    zerada.

**Editor:**
- **Tamanho mínimo e rolagem.**
  - Abaixo de 1280×800 os knobs viravam pontos e o controle SÍNTESE
    sumia (desde a fase 4).
  - Agora o editor tem tamanho mínimo e rola dentro de um viewport.
- **Sobreposição.** O editor volta para a frente depois de cada
  layout; os cartões da tela principal o cobriam.
- **Lista de corpos.** Fica apagada quando a fonte não é CORPO.

**Auditoria de layout** (`--layout-audit`):
- Agora inclui o editor com as propostas da semente 42 nos quatro
  tamanhos e nos quatro idiomas:
  - controles dentro do cartão;
  - botões sem sobreposição;
  - texto cabendo.
- **Falha antiga corrigida.** A auditoria falhava sem dizer o motivo
  desde o layout largo, porque exigia 224 px de cartão em SÍNTESE e
  CAUDA, que nesse layout têm uma fileira. Agora cada falha grava o
  motivo em `FAILED.txt`, e o layout largo exige uma fileira. Os
  limites de tamanho dos knobs continuam os mesmos.

**Validação:**
- suíte 66/66;
- auditoria de layout passa, com imagens conferidas em 1920, 1280,
  1024 e 960;
- app recompilado (binário de 2026-10-22 18:48).
- Não houve clique real no app nesta fase: a auditoria renderiza o
  editor sem mouse e sem áudio.

**Pendente:** os patches nas CENAS, a fase 6 (tutorial, LEARN e Sobre)
e o FORMANT como modo de filtro.

## Os patches nas CENAS — 2026-10-22

Cada cena guarda agora a mistura SÍNTESE e os cinco patches. Isso vale
para os quatro slots e para os arquivos `.aquscene`.

**Formato.** O `AQUORBIUM_PERFORMANCE_V1` ganha um segundo filho opcional,
`SYNTH { mix, patch0..patch4 }`. Os patches ficam em texto do
`SynthPatchCodec`. A validação é estrita:
- exatamente as seis propriedades;
- a mistura entre 0 e 1;
- cada patch com até 2048 caracteres de `[A-Za-z0-9.=;+-]`.

Uma cena com síntese inválida é recusada inteira, e nada é substituído.

**Compatibilidade:**
- O banco passa à versão V3 (`parameter.presetBankV3`), com a síntese.
- A V2 e a V1 continuam sendo gravadas sem a síntese, para que builds
  anteriores ao dispositivo ainda leiam as cenas.
- O `parameter.performanceStateV1` também é gravado sem a síntese, que
  tem suas próprias chaves (`synth.mix`, `synth.patch.N`).
- Um `.aquscene` exportado com síntese não abre em builds anteriores ao
  dispositivo.

**Recuperação:**
- Recuperar uma cena aplica a mistura e os patches dela.
- Uma cena anterior ao dispositivo soa como foi salva (SÍNTESE 0) e não
  apaga os patches em edição.
- PADRÃO zera a SÍNTESE e preserva os patches.
- Na abertura do app, o estado salvo não mexe na síntese, que vem das
  próprias chaves.

**Limites medidos:** cinco patches de 900 caracteres cabem num
`.aquscene` (limite de 16 KiB, 5 tags XML) e nos quatro slots do banco
(64 KiB, 32 tags).

**Validação:**
- testes do banco (V3, V2 e V1 sem síntese, limites, adulterações) e do
  arquivo (ida e volta, cena antiga sem síntese);
- suíte 66/66.
- A auditoria de execução `--runtime-stress` agora alterna a cada
  segundo entre uma cena sem síntese e uma com as propostas da semente
  42 em SÍNTESE 1.
  - Resultado a 48 kHz com buffer 512: 15 recuperações, 0 xrun, 0
    clipping, CPU de 6–7 % para 9–10 % enquanto a síntese entra e sai.
  - O áudio foi para o sink mudo `rasgo_mudo`, conferido com
    `pw-link`.

**Pendente:** fase 6 (tutorial, LEARN e Sobre do dispositivo) e o
FORMANT como modo de filtro. (A fase 6 foi feita em seguida; ver abaixo.)

## Fase 6 feita — 2026-10-22: tutorial, LEARN e Sobre

**TUTORIAL:** um capítulo novo, o 8, "O dispositivo de síntese", nos quatro
idiomas. Ele cobre:
- SÍNTESE e EDITAR;
- as duas fontes e os controles que mudam com elas;
- RING e FM, filtro, envelopes e LFOs;
- a MODULAÇÃO como o lugar em que o ecossistema entra no som;
- PROPOSTA, que escuta o que propõe;
- os patches nas cenas.

DICA: "comece com PROPOR TODOS e SÍNTESE pela metade; depois mude uma via
de cada vez".

**Ajuda dentro do editor.** O painel do LEARN fica embaixo do editor
sobreposto, então as explicações vêm em dicas (tooltips) que acompanham a
escolha:
- **Lista de fontes:** diz o que a fonte escolhida é e o que fazem os seus
  três controles.
- **Títulos das seções:** MISTURA (inclusive a regra da FM, que age só nas
  fontes osciladoras), FILTRO, ENVELOPES e LFOs.
- **Matriz:** o título explica as vias e as faixas; a fonte de cada via
  explica o sinal escolhido (fome, canal SOFAR, habitat etc.).

**LEARN na janela principal:**
- SÍNTESE e EDITAR, no cartão TIMBRE, têm entrada própria.
- O texto do cartão TIMBRE apresenta o dispositivo.
- EDITAR ganhou dica.

**SOBRE:** um parágrafo sobre o dispositivo e a proveniência dos núcleos
copiados do Rasgo Modular do autor (oscilador de banda limitada de Välimäki
e Huovilainen, filtro de Simper, ruído rosa de Kellet, FM, wavetable,
aditiva, pulsar de Roads, Karplus-Strong, matéria modal). A linha de fontes
passa a citar `SYNTH_DEVICE_DESIGN.md`.

**Achado no caminho:** os títulos dos capítulos eram convertidos para
maiúsculas com `juce::String::toUpperCase`, que segue o locale C e deixa as
letras acentuadas em minúscula ("A áGUA", "SíNTESE").
- Corrigido com `upper()` em `UiLanguage.h`, que cobre Latin-1 e œ.
- Testado em `EcologyParameterStateTests`: todos os títulos, nos quatro
  idiomas, ficam sem minúsculas.

**Layout:**
- Os botões de capítulo (agora 12) ajustam a altura para caber em 960×600.
- A auditoria de layout passou a renderizar também o capítulo 8 e o SOBRE
  nos quatro tamanhos, e confere que cada controle fica dentro do cartão.

**Validação:**
- suíte 66/66;
- auditoria de layout passa, com imagens conferidas;
- app recompilado (binário de 2026-10-22 20:04).

**Pendente:**
- o FORMANT como modo de filtro (feito em seguida; ver abaixo);
- os algoritmos de 4 operadores do Operator;
- a revisão nativa das traduções FR/ES (vale para todo o app).

## FORMANTE como modo de filtro — 2026-10-22

O FILTRO tem um quinto modo, **formante (vogais)**: cinco passa-faixas em
paralelo cujas frequências, larguras e ganhos seguem uma tabela de vogais
cantadas, A > E > I > O > U.
- **VOGAL**, um knob novo e um destino novo da matriz, percorre as vogais.
  Assim o ecossistema pode "falar".
- **CORTE vira TRATO**, o tamanho do trato vocal (1000 Hz = a voz da
  tabela). O nome muda no editor. As cinco formantes andam juntas, e NOTA,
  ENV, WAH e as vias de corte continuam valendo.
- **RESSON.** estreita as bandas.
- VOGAL fica apagado nos outros modos.

**Proveniência:** adaptado de `src/dsp/Formant.hpp` do Rasgo Modular do
autor (commit `4ce898f`, sha256 `079366eeb48e3006...`, AGPLv3, mesmo autor).
- Copiado: os cinco SVF TPT (Simper) com a saturação no laço, a
  interpolação logarítmica entre vogais, e a largura dividida por
  (1 + 8·res).
- Deixado de fora: o vocoder, a deriva e o seco/molhado (a voz tem a sua
  mistura SÍNTESE).
- A tabela de vogais é dado fonético publicado (Fant, *Acoustic Theory of
  Speech Production*, 1960; as tabelas de formantes do apêndice do manual
  do Csound): voz de baixo, F1–F5 com ganho e largura. Os valores são os
  mesmos do Modular.

**Duas diferenças medidas em relação ao Modular:**
- **Nível.** Com ganho de pico unitário por banda, como no Modular, um
  dente de serra saía cerca de 30 dB abaixo do passa-baixas, e mais baixo
  ainda com RESSON.
  - O ganho passou a ×√k, o que mantém constante a energia de um espectro
    largo quando as bandas estreitam. O ruído sai no mesmo nível do
    passa-baixas.
  - O nível do modo foi calibrado em ×2,5: um dente de serra pelas cinco
    vogais fica entre −7 e −1,5 dB do passa-baixas a 2 kHz.
- **Larguras.** As larguras acompanham o TRATO (Q constante); no Modular
  elas ficavam fixas.

**PROPOSTA:** MORPH propõe o modo formante em cerca de um quarto das vezes
(23 de 120 sementes), com vogal, trato e uma via relação → vogal. As
propostas continuam escutadas e niveladas: 600 de 600 a menos de 6 dB do
alvo.

**Medido (`SynthVoiceTests`):**
- A: F1 605 Hz e F2 1045 Hz (tabela: 600 e 1040);
- I: F1 220 Hz (tabela: 250) e uma ressonância local em 1760 Hz (×1,36
  acima da vizinhança). O F2 do I é −30 dB na tabela, mais fraco que a saia
  do F1;
- CORTE 2000 (trato ×2) leva o F1 do A a cerca de 1200 Hz;
- uma via para VOGAL leva A a I;
- os extremos (RESSON. 1, DRIVE 1, trato ×4, 2 kHz) ficam finitos e
  limitados;
- o codec guarda o modo, a vogal e a via.

**CPU:** com 10 slots aditivos no pior caso, a média sobe de 1432 para
1544 µs por bloco (+8 %). O pior bloco isolado varia entre execuções
(agendador).

**Validação:**
- suíte 66/66;
- auditoria de layout passa: VOGAL aparece apagado fora do modo formante;
- app recompilado (binário de 2026-10-22 20:41).
- A troca do rótulo CORTE→TRATO não aparece nas imagens da semente 42.

**Pendente:**
- Operator de 4 operadores (feito em seguida; ver abaixo);
- a revisão nativa FR/ES.

## FM 4 operadores — 2026-10-22

Uma fonte nova, a 12ª: **FM 4 operadores**, o OPERATOR inteiro do Rasgo
Modular. O FM de 2 operadores continua como estava; os patches salvos não
mudam, e a fonte nova entra no fim da lista.
- **Operadores:** quatro operadores senoidais, A, B, C e D.
- **Oito algoritmos** de roteamento, numa lista na mesma linha da lista de
  corpos (aparece uma ou outra). Na notação, ">" quer dizer "modula" e "+"
  soma as portadoras. Do 1 (A>B>C>D) ao 8 (A + B + C + D, aditivo).
- **Os controles da fonte:**
  - FORMA = RAZÃO B;
  - LARGURA = ÍNDICE (0 = senos puros);
  - COR = REALIM. de A (média de duas amostras, como no DX7).
- **RAZÃO C e RAZÃO D:** dois knobs na segunda fileira da seção, ativos só
  nessa fonte.
- As razões vêm da tabela musical (0,5x a 9x). Os knobs mostram a razão
  ("2,0x"), e o FORMA também a mostra quando a fonte é FM ou FM 4-OP.
- **Saída:** as portadoras somadas, divididas pelo número de portadoras,
  e o soft clip do Modular (transparente até 1, assíntota em 1,5).

**Proveniência:** adaptado de `src/dsp/Operator.hpp` do Rasgo Modular do
autor (commit `4ce898f`, sha256 `3e167ab18c2979f9...`, AGPLv3, mesmo autor),
inclusive a tabela de algoritmos e a de razões.
- Usa a tabela de seno compartilhada no lugar da LUT própria.
- Não tem deriva, porque a voz tem DERIVA.
- O algoritmo e as razões C/D são lidos a cada ciclo de controle.

**PROPOSTA:** MODAL troca o FM de 2 operadores pelo de 4 nas suas
famílias, como sino (algoritmos 5 a 8, de portadoras em paralelo, com
razões livres). As propostas usam agora as 12 fontes. Continuam escutadas e
niveladas: 600 de 600 a menos de 6 dB do alvo.

**Medido (`SynthVoiceTests`):**
- algoritmo 8: quatro senos iguais em 1x, 2x, 3x e 5x, e nada entre eles;
- algoritmo 1 com índice 0: só a portadora D soa, em 5x; com índice,
  aparecem bandas laterais em D ± C;
- algoritmo 5 com índice 0: um seno;
- os oito algoritmos nos extremos: finitos e ≤ 1,5;
- o codec guarda algoritmos e razões.
- O teste do codec agora espera "a última fonte" em vez de nomear CORPO.

**Auditoria de layout:**
- uma página de demonstração em 1280: MORPH no FORMANTE e MODAL no FM 4-OP;
- conferida: TRATO e VOGAL, a lista de algoritmos, RAZÃO C/D em "x" e
  apagados nas outras fontes.

**Validação:** suíte 66/66; app recompilado (binário de 2026-10-22 21:55+).

## A mistura SÍNTESE sem clique — 2026-10-22

O teste de automação (`OUTPUT_STAGE_DESIGN.md`) mostrou que `setSynthMix`
aplicava o valor de uma vez: girar SÍNTESE com o aquário tocando dava um
degrau na mistura. Agora a mistura segue o alvo amostra a amostra, com a
suavização dos controles diretos do engine (15 ms), e trava exatamente no
alvo. SÍNTESE 0 continua bit a bit o som anterior, e o teste disso passa.

## O DRIVE sem aliasing — 2026-10-22

**Medido:** o DRIVE (linear até 1, depois 1 + tanh) gerava harmônicos acima
de Nyquist que voltavam espelhados, sem oversampling. Pior parcial
espelhada contra a fundamental:
- 7 kHz com drive máximo: −18,6 dB;
- 5 kHz com drive 0,5: −41 dB.

**Conserto:** antialiasing por antiderivada de 1ª ordem (Parker, Zavalishin
& Le Bivic, DAFx 2016). Em vez de f(x), usa-se [F(xₙ) − F(xₙ₋₁)] / (xₙ −
xₙ₋₁), com F a integral da curva escrita em forma fechada.

**Depois:**
- 7 kHz com drive máximo: −30,2 dB;
- 5 kHz com drive 0,5: −55 dB;
- 1,7 kHz com drive máximo: −56 dB;
- 1,1 kHz com drive máximo: −77 dB.

Com drive em zero, o caminho é o mesmo de antes, bit a bit.

O oversampling de 2× ficou de fora: a latência dele só existiria com drive
acima de zero e daria um clique ao sair do zero. As medições estão fixadas
em `SynthVoiceTests`.

## Cinco lições no TUTORIAL — 2026-10-23

O autor, depois de abrir o editor: "preciso aprender a tocar, são muitos
parâmetros". Escolhida a opção 2, entre (1) um modo SIMPLES no editor, (2)
lições no tutorial e (3) as duas: o TUTORIAL ganhou cinco lições depois
do capítulo do dispositivo (capítulos 9 a 13), nos quatro idiomas.

Cada lição tem um objetivo, poucos passos do tipo "faça / ouça" e uma dica:
1. **ouvir a síntese:** SÍNTESE de 0 a 1, PROPOR TODOS várias vezes;
2. **a fonte e o brilho:** FONTE A e CORTE, num organismo só;
3. **o gesto da nota:** ATAQUE e SOLTURA; GRAIN golpeado, MODAL
   ressonante;
4. **cor e voz:** FORMA, RESSON. e o formante, com TRATO e VOGAL;
5. **o ecossistema toca o som:** uma via por vez (energia → corte,
   distância → corte negativo, presas → veloc. LFO 2), COPIAR PARA TODOS e
   cenas.

**Conferido:**
- Os nomes citados são os rótulos exatos da tela em cada idioma.
- Os 17 capítulos cabem em 960×600.
- A auditoria de layout renderiza também a lição 5.
- O teste dos títulos em maiúsculas cobre as lições.

**Pendente:** o modo SIMPLES (opção 1), se o autor ainda achar o editor
cheio depois das lições.


## CAOS e REALIMENTAÇÃO — 2026-10-23

Duas fontes novas, acrescentadas no fim da lista: índices 12 e 13, de modo
que patches salvos mantêm as suas fontes. Ficam disponíveis no EDITAR; as
PROPOSTAS não as usam, então nenhuma proposta mudou.

**CAOS (`ChaosSource`).** O poço duplo do CHAOS do Rasgo Modular
(`guia/36_chaos.md`), reescrito como fonte de áudio afinada: um oscilador
de Duffing forçado, `x'' + d·x' − x + x³ = g·cos t`, com a força na
frequência da nota. É integrado por RK4 em subpassos de no máximo 0,15.

| macro | o que faz |
|---|---|
| FORÇA (`g`, de 0,15 a 0,70) | órbita pequena num poço só, depois caos (com janelas de período 3 e 6), depois a órbita grande, periódica, pelos dois poços |
| AMORTEC. (`d`) | desloca essas fronteiras |
| COR | vai da posição à velocidade, mais brilhante |

Um seguidor de nível lento (80 ms) põe os três regimes no mesmo volume; a
órbita pequena ficava 12 dB abaixo do caos.

Medido nesta configuração: a entrada no caos é por crise, não por cascata
de duplicações de período. A documentação diz isso e não promete as
duplicações.

Referências: Y. Ueda, 1979; J. Guckenheimer e P. Holmes, *Nonlinear
Oscillations, Dynamical Systems, and Bifurcations of Vector Fields*, 1983,
§2.2.

**REALIMENTAÇÃO (`FeedbackSource`).** Um laço sem entrada, como numa mesa
sem entrada: um atraso afinado no período da nota, um passa-baixa, um
bloqueio de DC e um tanh. É excitado no ataque da nota e se mantém pelo
próprio ganho. Para a afinação, a fase do passa-baixa e a do bloqueio de DC
saem do atraso; antes dessa compensação exata ele ficava 14 cents agudo.

| macro | o que faz |
|---|---|
| GANHO | o ganho do laço na fundamental, de 1,03 (mal canta) a 2,5 (saturado) |
| DESAFINAR | soma um segundo tap fora do período; o laço salta para outros modos |
| COR | abre o passa-baixa, de 1,5 a 16 vezes a nota |

A perda do passa-baixa e a dos dois taps na fundamental é compensada (no
máximo 4×), de modo que o laço sempre canta em algum modo. A REALIMENTAÇÃO
fica fora do FM B→A, como os ressonadores: um laço re-afinado a cada
amostra não é estável.

**Testes (`SynthVoiceTests`).**
- CAOS: correlação de período 1,0, −0,10 e 1,0 nos três regimes, a
  0,30 de RMS (±1,5 dB); finito e limitado de 30 Hz a 9 kHz.
- REALIMENTAÇÃO: canta no menor GANHO (RMS acima de 0,1) e fica afinada
  (0,1 a 0,3 cent a 110, 220 e 440 Hz); canta em todo GANHO e DESAFINAR;
  limitada a 0,5.
- As duas: audíveis na voz inteira e mantidas pelo codec.

**Custo.** Medido em ns por amostra:

| fonte | 110 Hz | 880 Hz | 4 kHz | 9 kHz |
|---|---|---|---|---|
| CAOS | 85 | 78 | 188 | 339 |
| REALIMENTAÇÃO | 57 | 57 | 57 | 57 |
| FM4 | 97 | 99 | 101 | 105 |

No registro musical, o CAOS custa menos que o FM4; acima de 4 kHz os
subpassos do RK4 encarecem.

Falta a escuta do autor.
