> **Este é o diário de desenvolvimento do Aquorbium** (até 2026-10-26 era o
> `README.md`; movido para `docs/` a pedido do autor, para a publicação). O
> README público, curto e nas quatro línguas, está na raiz do repositório.

# Aquorbium

Localização atual (2026-10-11): `RASGO/PROJETOS_AUTORAIS/AQUORBIUM/`.
O caminho antigo na raiz é um alias de compatibilidade, não outra fonte.
Criação e desenvolvimento são a finalidade; autoria, licença e identidade
permanecem próprias. Ler [as instruções RASGO](../../AGENTS.md) e a continuidade
local antes de trabalhar. Organização e validações no
[registro da migração](../../RASGO_DOCUMENTATION/architecture/MIGRACAO_PROJETOS_AUTORAIS.md).
Aplicar o [procedimento de recursos](../../../PROCEDIMENTO_SEGURANCA_RECURSOS.md)
antes de tarefas controladas; publicação continua uma decisão separada.


Atualização documental: 2026-10-22.

**Identificador arquivístico:** `ARQ-AQU-001`
**Estado:** desenvolvimento local; app standalone e core ativos, sem release congelada.
**Licença:** GNU AGPL 3.0 ou posterior ([LICENSE-AGPL-3.0.txt](../LICENSE-AGPL-3.0.txt)).
Cada arquivo-fonte começa com `SPDX-License-Identifier: AGPL-3.0-or-later` e
`Copyright (C) 2026 Lúcio de Araújo`. O teste `aquorbium_license_headers`
exige esse cabeçalho, inclusive em arquivos novos.

## Identidade visual — aprovada em 2026-10-25

A versão final escolhida é o lockup transparente e sem degradê, com sonar
circular e lettering vetorial original: [SVG](../assets/identity/aquorbium-logo-sonar-texto-compacta.svg)
e [PNG](../assets/identity/aquorbium-logo-sonar-texto-compacta.png). O símbolo,
wordmark, favicon e ícones em vários tamanhos estão em
[`assets/identity/`](../assets/identity/IDENTIDADE.md). Os ativos estão prontos;
a integração do símbolo e do lettering no cabeçalho JUCE continua pendente.
Não restaurar variantes descartadas pelo autor.

## Como tocar — guia rápido

**Abrir:** nesta pasta, `./run`. O comando abre o binário mais recente de
`build-reactivation-20260915/`. Para recompilar:
`cmake --build build-reactivation-20260915 --target AquorbiumApp`.

**O que é:** um bioma audível. Cinco organismos (MORPH, GRAIN, MODAL,
PULSAR, STRING) vivem numa esfera de água e cantam. Com **BIOMA AUTÔNOMO**
ligado, ele toca sozinho. Você toca o **ambiente**: o desejo, a água, a
memória e os corpos dos organismos. Um teclado MIDI também pode tocar notas.

**A tela:**
- **Centro:** a esfera, com as relações (linhas coloridas) e o painel do
  estado da água, das populações e da tensão.
  - Pressionar e arrastar sobre a esfera move a **LUPA**, que aproxima a
    escuta de uma região.
  - **CONTROLES ECO**, dentro do aquário, abre coesão, perigo, os modos de
    escuta e as **cenas**.
- **Faixa de baixo:** DENSIDADE (quanto acontece), DERIVA (o quanto está
  inquieto) e MORPH (do som neutro ao ecológico); e o **VARIA**, ao lado de
  BIOMA AUTÔNOMO, com o slider de quanto a mão anda (0 parada, 1 o padrão,
  2 o dobro).
- **Cartões:** CÉREBRO, GEOMETRIA (com MÚSICA e ÁGUA), SÍNTESE (o Striker),
  ESPAÇO, AMBIENTE, CAUDA, MASTER (nove botões, com o MASTER CRIATIVO por
  dentro), MÓDULOS (LAB), TIMBRE, MEDIDORES, LUPA e, nas telas menores,
  CÉREBRO · SALA.
- **A porta de cada cartão:** o primeiro botão de um cartão (STRIKER, CAUDA,
  AMBIENTE, MASTER CRIATIVO, LAB MIX, FORMA, ESTRUTURA, MÚSICA, ESPAÇO) é a
  quantidade daquele módulo na mistura. Em 0, o módulo está desligado e os
  outros botões do cartão ficam cinzas e travados. **Suba a porta primeiro**
  e depois mexa nos outros.
- **COMANDOS ECO** (painel central): coesão, perigo, os modos de escuta, o
  **CÉREBRO** (RELÓGIO, COORDENAÇÃO), a **SALA** (SENSIBILIDADE, REAÇÃO) e as
  cenas.
- **Cabeçalho:**
  - o fader do **VOLUME DE SAÍDA** (o mesmo botão do MASTER; nas telas
    largas);
  - REC, SILENCIAR, PAUSA;
  - CAIXAS/FONES (com FONES, cada organismo vem da sua direção);
  - ÁUDIO (dispositivo; a dica mostra LUFS, true peak e correlação);
  - LEARN, idioma, TUTORIAL, SOBRE.

**Para começar:**
1. Abra o **TUTORIAL**. Os capítulos 1 a 8 explicam o instrumento; as
   **lições 9 a 13** ensinam o dispositivo de síntese, um controle por vez.
2. Ligue o **LEARN** e aponte para qualquer controle: a explicação aparece no
   painel.
3. Suba **MÚSICA** (cartão GEOMETRIA): os acontecimentos passam a nascer da
   teia de relações. Mova **ÁGUA** devagar, do gelo à fervura.
4. Mexa em **uma coisa por vez** e dê alguns segundos: o ecossistema é lento.

**Cada abertura é uma peça nova** (a gênese da sessão, 2026-10-24). A
semente da sessão escolhe:
- uma tonalidade e um modo;
- uma linguagem harmônica (funcional, modal, brasileira, impressionista ou
  cromática), com modulações, trocas de linguagem e, às vezes, trechos
  politonais;
- os registros, as órbitas, o contorno e o andamento;
- quem abre;
- um arco de tensão de 4 a 8 min.

Os organismos têm papéis:
- o mais grave faz o baixo;
- os do meio fazem as notas do acorde;
- quem abre canta a melodia, respira depois das cadências, e outra voz lhe
  responde com o motivo transformado;
- o motivo volta mais tarde.

O status mostra o mundo, por exemplo "A ionian · brasileira · órbita 10";
`session.genesis = 0` volta ao instrumento clássico.

**A abertura também propõe o som** (a semente de abertura, 2026-10-26). A
mesma semente sorteia um **bioma**: temperatura, luz, turbidez, correnteza,
população, predação e habitat. Dele saem:
- quais módulos acordam (de um a quatro);
- os controles dos cartões, em torno dos valores clássicos;
- o corpo de cada organismo, puxado pela afinidade da espécie;
- a ecologia e as vozes de síntese (a PROPOSTA com a mesma semente).

O estágio de saída fica com você: MASTER, MASTER CRIATIVO, SUSTENTAÇÃO,
LUPA e SALA. O estado que você deixou fica guardado em
`opening.previousStateV1`; `opening.seeded = 0` restaura os controles
salvos, como antes.

**VARIA** (ligado por padrão): os controles se movem sozinhos ao longo da
música, devagar e em relação, em torno de onde a semente ou a sua mão os
deixou. No clímax da forma a água esquenta: tom mais claro, mais população,
espaço menor. No repouso, assenta. Mexa num controle e ele passa a respirar
em torno do seu valor. As portas fechadas, a escolha dos corpos e o estágio
de saída nunca se movem. É o conceito do Motion Engine do Rasgo Modular,
reescrito para o bioma (`src/core/OpeningSeed.h`, `src/core/ControlMotion.h`). Detalhes:
[COMPOSITIONAL_VARIATION_DESIGN.md](../COMPOSITIONAL_VARIATION_DESIGN.md).

**O dispositivo de síntese** (cartão TIMBRE → **SÍNTESE** e **EDITAR**):
- Uma voz semimodular por organismo, tocada pelas notas do ecossistema.
- **PROPOR TODOS** dá um ponto de partida para os cinco organismos.
- Na **MODULAÇÃO**, a vida do bioma move os parâmetros.
- SÍNTESE em 0 é o som de antes.
- Fontes: oscilador, ruído, FM, FM de 4 operadores, wavetable, aditiva,
  pulsar, corda, matéria, os corpos e, desde 2026-10-23, **CAOS** (Duffing)
  e **REALIMENTAÇÃO** (um laço sem entrada).
- Detalhes: [SYNTH_DEVICE_DESIGN.md](../SYNTH_DEVICE_DESIGN.md).

**SUSTENTAÇÃO** (cartão TIMBRE): o nível de GRAIN, MODAL e STRING diante de
MORPH e PULSAR, de −6 a +24 dB. Em 0 dB eles ficam bem atrás (~22-26 dB);
suba para trazê-los à frente.

**ORDEM DO EQ** (cartão MASTER): ANTES ou DEPOIS. Diz se o EQ de
inclinação do MASTER CRIATIVO fica antes da dinâmica multibanda, que então
reage ao som já inclinado, ou depois dela, colorindo o que a dinâmica
deixou. Só age com o MASTER CRIATIVO acima de zero e com o som alto: no
nível usual a dinâmica do MASTER quase não comprime, e então a ordem não
muda nada.

**Cenas:** CONTROLES ECO → GRAVAR / RECUPERAR, quatro slots, que também se
exportam e importam como `.aquscene`. Uma cena guarda todos os controles e
a síntese.

**Gravar:** o botão **REC** grava o que se ouve em
`Música/Aquorbium/AAAA-MM-DD/`, com um texto ao lado (medição, semente,
controles). Cada abertura sorteia uma semente nova; ela aparece no status
de áudio.

**Áudio sem falhas (XRUN) no Linux:** a thread de áudio pede prioridade de
tempo real sozinha, e o status mostra `RT 70` quando consegue ou `RT não`
quando falta permissão. Para dar a permissão:
1. `sudo usermod -aG pipewire $USER`;
2. **reinicie o computador**. Sair e entrar de novo não basta, porque o
   systemd do usuário mantém os grupos antigos.

Sem tempo real, outros programas pesados (compilações, testes,
transcrição) atrasam o áudio: em 2026-10-24, uma compilação ao lado levou a
sessão a 1030 XRUN.

**Ajustes só no arquivo de configurações:** `~/RASGO/Aquorbium/Aquorbium.settings`.
Edite **com o app fechado**, porque ele regrava o arquivo ao sair.

| Chave | Valores | Efeito |
|---|---|---|
| `recording.format` | `pcm24`, `pcm16` (padrão: float 32) | formato do REC |
| `recording.tap` | `post-safety` (padrão), `pre-safety`, `pre-master-criativo` | onde o REC ouve |
| `recording.stems` | `1` | grava também cinco stems (organismos, LAB, água, espaço, cauda) |
| `recording.ambisonics` | `1` | grava também `_ambix.wav`, Ambisonics de 1ª ordem (AmbiX), com cada organismo vindo da sua direção, para qualquer decodificador (caixas ou binaural) |
| `recording.directory` | uma pasta | onde o REC grava |
| `recording.artist` | um nome | o artista nas tags do WAV |
| `session.fixedSeed` | um número | repete uma sessão (a semente aparece no status) |
| `session.genesis` | `0` | volta ao instrumento clássico; por padrão cada abertura é um mundo novo: tonalidade e caminho de acordes, modo, registros, órbitas, contorno, andamento, quem abre, cor dos corpos, hora e estação (o status mostra, por exemplo, "A ionian · órbita 10") |

**Validar sem ouvir:**
- `ctest` em `.local-build/revisao-20261020` (a rotina; `-LE slow` pula os
  testes longos);
- `--layout-audit=<pasta>` desenha todas as telas sem cliques e sem áudio.

## Estado atual (2026-10-22)

As pendências estão consolidadas no topo de
[aquorbium-tarefas.md](../aquorbium-tarefas.md#pendências-abertas--consolidado-de-2026-10-22).
O que mais depende do autor:
- usar o dispositivo de síntese;
- testar a SALA com o microfone real;
- as escutas de aprovação musical;
- a logo;
- a revisão das traduções.

- **Dispositivo de síntese (completo).** O autor pediu ferramentas de
  síntese em vez de timbres prontos
  ([SYNTH_DEVICE_DESIGN.md](../SYNTH_DEVICE_DESIGN.md)).
  - Cada organismo tem uma voz semimodular, tocada pelas notas do
    ecossistema: duas fontes, mistura (RING, FM), filtro, dois envelopes e
    dois LFOs.
  - Doze fontes por slot:
    - oscilador, ruído e o corpo nativo;
    - FM e FM 4 operadores (8 algoritmos), wavetable, aditiva, pulsar,
      corda e matéria, adaptadas do Rasgo Modular com proveniência;
    - os onze corpos do instrumento.
  - O filtro tem passa-baixas, passa-banda, passa-altas, rejeita-faixa e
    FORMANTE (as vogais de Fant, com VOGAL e TRATO).
  - Uma matriz de 8 vias liga os sinais do ecossistema aos parâmetros:
    fome, tensão, distância, canal SOFAR, temperatura, presas, habitat,
    sala.
  - O editor abre pelo EDITAR no cartão TIMBRE. Rola abaixo de 1280×800 e
    tem dicas que acompanham cada escolha.
  - PROPOSTA sugere um patch de partida pela espécie, pela estratégia e
    pela semente. O instrumento escuta e nivela o que propõe.
  - As cenas guardam a mistura e os cinco patches (banco V3; V2 e V1 sem a
    síntese, para builds antigos).
  - SÍNTESE 0 mantém o som de antes, bit a bit.
  - Explicado no tutorial (capítulo 8), no LEARN e no Sobre.
- **Saída e segurança (2026-10-22)**
  ([OUTPUT_STAGE_DESIGN.md](../OUTPUT_STAGE_DESIGN.md)):
  - O limitador detecta em true peak 4×. Antes, deixava passar até
    +0,9 dBTP sobre um teto de −0,1 dB.
  - O ganho do limitador não fica mais preso depois de um pico.
  - Um teste de estresse do engine cobre o pior caso pela interface,
    impulso, DC, NaN/Inf, blocos irregulares e troca de taxa de
    amostragem. Pela interface, o pior caso fica em cerca de −8,5 dBTP.
  - A SALA passou a ser ouvida com o aquário em silêncio.
  - Mudar qualquer controle de uma vez não estala; a mistura SÍNTESE, que
    estalava, foi suavizada. A soma mono perde 0,2 dB nas caixas e 1,1 dB
    nos fones.

Uma sessão longa com o autor (Claude Code, autorizado pelo Codex e pelo
autor). O detalhe e as pendências estão em
[aquorbium-tarefas.md](../aquorbium-tarefas.md); abaixo, o mapa.

- **REC e semente da sessão (2026-10-21).**
  - O botão REC, no cabeçalho, grava o que se ouve em WAV float de 32 bits
    em `Música/Aquorbium/AAAA-MM-DD/`. Na máquina do autor, essa pasta é um
    link para `AUDIOS_RASGO/Aquorbium`, no disco de 4 TB.
  - Ao lado de cada take fica um texto com a medição (LUFS integrado, true
    peak 4×, DC), a semente e os controles. Os detalhes estão em
    [OUTPUT_STAGE_DESIGN.md](../OUTPUT_STAGE_DESIGN.md#gravação-rec--2026-10-21).
  - Cada abertura sorteia uma semente nova, e as sequências deixam de ser
    sempre as mesmas. A semente aparece no status de áudio;
    `session.fixedSeed` nos settings reproduz uma sessão.

- **A música vem do ecossistema** ([ECOLOGICAL_MUSIC_DESIGN.md](../ECOLOGICAL_MUSIC_DESIGN.md)).
  O `EcologicalMusicDirector` trabalha com:
  - a teia de relações: caça, fuga, rivalidade, cardume, corte, território;
  - a harmonia por compatibilidade, com tensão e cadência;
  - a densidade como capacidade do nicho;
  - os estados líquidos com calor latente;
  - a propagação do som na água: atraso, absorção, Doppler, eco e buzz;
  - as assinaturas e codas, a revolução de canto, a rede de bolhas;
  - as notas articuladas pela água.
- **Timbre** ([TIMBRE_BODIES_DESIGN.md](../TIMBRE_BODIES_DESIGN.md)):
  - um segundo corpo por organismo: voz, matéria, sopro, corda, gota,
    dobra, membrana, vidro ou coro;
  - os corpos são nivelados contra o nativo, e o TIMBRE ECOLÓGICO os
    transforma;
  - as frequências líquidas: a absorção pela distância.
- **Interface:**
  - todos os controles visíveis, com o ecossistema no centro e o visual de
    painel de submarino;
  - SILENCIAR, PAUSA, TUTORIAL, LEARN e SOBRE;
  - quatro idiomas (EN/PT/FR/ES) e a frase de crédito no rodapé;
  - a logo provisória em [assets/identity](../assets/identity/IDENTIDADE.md);
    a definitiva fica com o autor.
- **Conceitos verificados e roteiro:**
  [CONCEITOS_VERIFICACAO_E_APROFUNDAMENTO.md](../CONCEITOS_VERIFICACAO_E_APROFUNDAMENTO.md)
  traz cada conceito conferido na fonte, os projetos livres com potencial e
  doze módulos propostos, todos aprovados pelo autor como prioridade. O
  primeiro, `ChorusPhase` (o nicho temporal como coro de osciladores),
  e o segundo, `SongForm` (o canto em temas que voltam), estão feitos; o
  terceiro, as correções de fidelidade (endereçamento, Lombard em dB, o medo
  que sincroniza), e o quarto, `SpectralConsonance` (a harmonia que nasce
  dos corpos), e o quinto, a física da água (Ainslie & McColm, Mackenzie,
  MAR / ÁGUA DOCE), o sexto, `ThermalCloud` (Xenakis), e o sétimo,
  `PopulationCycles` (Lotka–Volterra: ciclos de fartura e escassez de
  minutos), e o oitavo, `LifeCycle` (vozes que comem, envelhecem, morrem e
  renascem com os cantos dos pais recombinados), e o nono, `SoundChannel` (o
  canal SOFAR: a altura decide quem ouve quem), e o décimo, a marca sonora
  e o hi-fi/lo-fi (Schafer), o 11º, `HabitatHealth` (ADI e ACI do próprio
  som), e o 12º, `RoomCoupling` (a sala pelo microfone, opcional e só com
  descritores), também estão feitos: **o roteiro de 12 módulos está
  completo**. Depois dele, o `CodaExchange` (o alfabeto das codas de
  cachalote, os clãs e o rubato que converge no diálogo) e a MARCA (a marca
  sonora escolhida pelo músico) também estão feitos, e dois corpos físicos
  novos, o ARCO (corda friccionada) e a PALHETA (tubo de clarinete), e o
  `BubbleField` (a água que borbulha na turbulência e na fervura), e o
  seletor CAIXAS / FONES (nos fones, cada organismo vem da sua direção na
  esfera, pelo modelo de cabeça de Brown & Duda). Falta
  o autor escutar
  e testar a SALA com o microfone real.
- **Timbre, corpos:** são onze (voz, matéria, sopro, corda, gota, dobra,
  membrana, vidro, coro, arco e palheta). Todos são também fontes do
  dispositivo de síntese.
- **Validação:**
  - A suíte local (`ctest`) tem 69 testes e passou inteira a cada etapa.
    O de estresse do engine leva ~2 min; `ctest -LE slow` o pula na rotina.
  - `--layout-audit=<pasta>` desenha todas as telas sem cliques e sem áudio,
    inclusive o editor de síntese, o tutorial e o Sobre, nos quatro
    tamanhos e idiomas.
  - O app é aberto com `./run`.
  - Os renders de escuta ficam em `validation-output/` e
    `AUDIOS_RASGO/Aquorbium/`, fora do git.
  - Testes do app com som vão para o sink mudo `rasgo_mudo`
    (`PIPEWIRE_NODE=rasgo_mudo PULSE_SINK=rasgo_mudo`), conferido com
    `pw-link`.

## Banco integrado após a auditoria de visibilidade

A auditoria de 27/09 encontrou cerca de quarenta componentes de core codificados
e testados sem rota ao vivo. Em 28/09, todos os protótipos daquela fila foram
ligados ao `BiomaEngine` por um banco experimental com bypass mestre e quinze
famílias audíveis. O cartão `08 MÓDULOS` oferece dezesseis macros grandes e
roláveis: `LAB MIX` mais uma macro por família. Com o mestre em zero, a rota
anterior é preservada e o banco não consome processamento por amostra.

Essa promoção é técnica, não uma aprovação composicional. Cada família passou
por resposta, determinismo, cenas, layout e tempo real; ainda precisa de escuta
separada de ritmo, melodia, harmonia, timbre, eventos e função composicional.
O contrato completo, o mapa das classes e os limites estão em
[INTEGRATED_MODULE_BANK_DESIGN.md](../INTEGRATED_MODULE_BANK_DESIGN.md).

Na continuação de 28/09, dez componentes novos foram codificados e entraram
diretamente nessas famílias, sem criar outra camada invisível ou novos knobs:
Reese, filtro de dois picos, sequenciador ressonante, isolador espectral,
interação ring/XOR/duck, Power Starve, rotação em quadratura, oscilador
polygonal X/Y, matriz 5×5 e pool compartilhado. A suíte passou 42/42 e o
benchmark manteve margem mínima offline de 3,26× tempo real com todas as
famílias ativas. Os vínculos especializados com Lupa, predador/presa,
BiomaBass, `AquariumTransform`, `AccessPort` e forma visual continuam abertos.
Detalhes em
[EMERGENT_MODULE_COHORT_DESIGN.md](../EMERGENT_MODULE_COHORT_DESIGN.md).

Uma segunda coorte acrescentou BiomaBass experimental completo, cardume de
seis vozes, ondulações harmônicas, resíntese do mix recente, frequency shifter,
estados por cross-feedback, foco SPOT, fome, auto-feedback e inércia da água.
Eles reforçam as mesmas famílias e mantêm os 62 valores persistentes. A suíte
atual passou 43/43; a margem mínima medida foi 2,80× tempo real offline no caso
extremo de 96 kHz/bloco 64/quatro vozes. Escuta e vínculos especializados
continuam abertos. Detalhes em
[ADVANCED_VOICE_ECOLOGY_COHORT_DESIGN.md](../ADVANCED_VOICE_ECOLOGY_COHORT_DESIGN.md).

Em 29/09, a terceira coorte acrescentou geração formal por L-system, autômato
celular, transferência espectral, contágio de envelopes, barramento sem
entrada, estratégia por limiares, foco lock-in, cenas comportamentais, pool
reconfigurável e binaural leve. A interface e os 62 valores não mudaram. A
suíte passou 44/44 e a menor margem offline foi 2,40× tempo real no caso extremo
de 96 kHz/bloco 32/quatro vozes. O binaural não é HRTF medida e o barramento não
é ainda o Master. Detalhes em
[GENERATIVE_INTERACTION_COHORT_DESIGN.md](../GENERATIVE_INTERACTION_COHORT_DESIGN.md).

Ainda em 29/09, a coorte de integração especializada fechou seis ligações que
continuavam genéricas: subgrave pelo quantizador intervalar, grãos pelos traços
do organismo, Pulsar ecológico completo, áudio final realimentando o
sequenciador, soft take-over do contexto e deriva lenta do MultiTap. Não houve
novo controle nem alteração dos 62 valores. A suíte passou 44/44; o benchmark
ativo mediu 19,21% em 48 kHz/bloco 128/quatro vozes e permaneceu no mínimo
2,57× tempo real offline. Detalhes em
[SPECIALIZED_INTEGRATION_COHORT_DESIGN.md](../SPECIALIZED_INTEGRATION_COHORT_DESIGN.md).

Em 30/09, o Master criativo deixou de ser apenas tarefa. `MasterChain` reúne
crossover Linkwitz–Riley em três bandas, dinâmica vinculada por banda, tilt,
largura mid/side acima do grave e cola óptica antes do `OutputStage`. Um único
macro grande `CREATIVE MASTER` abre a cadeia com rampa e retorna bit a bit em
zero. O `crackStress` estreita médio/agudo; redução de ganho e picos aparecem
na telemetria. O estado atual possui 63 valores persistentes, com migração dos
62 anteriores. A suíte integral passou 45/45; benchmark combinado e 72
capturas de layout também passaram. A escuta musical continua aberta. Detalhes em
[CREATIVE_MASTER_COHORT_DESIGN.md](../CREATIVE_MASTER_COHORT_DESIGN.md).

Em 01/10, o `OutputStage` recebeu medição de loudness pós-proteção conforme a
estrutura ITU-R BS.1770/EBU R128: ponderação K derivada da taxa ativa, janelas
momentânea de 400 ms e curta de 3 s, e integrado com portas absoluta de
-70 LUFS e relativa de -10 LU. O medidor apenas informa; não normaliza nem
modifica a performance. Um histograma fixo mantém memória e custo limitados no
callback. M/S/I LUFS seguem pela ponte atômica para o tooltip de ÁUDIO e para o
relatório da auditoria de runtime. Testes leves de referência EBU e integração
pós-safety passaram; o build e a suíte integrais aguardam redução da swap do
anfitrião. O true peak continua sendo estimado e não foi reclassificado como
conforme. Detalhes em [OUTPUT_STAGE_DESIGN.md](../OUTPUT_STAGE_DESIGN.md).

Em 02/10, quatro hipóteses ainda sem core entraram no banco sem aumentar o
painel: `DoubleScrollAttractor` em `LORENZ + DRIFT`, `SemanticAccessRouter` em
`BRANCH + PERMUTE`, `OpenRuleProgram` em `EVOLVE + STAGES` e
`ContactCrackleExciter` em `GESTURE WAVES`. O teste dedicado, o banco e a
segurança de tempo real passaram, incluindo telemetria que prova a participação
dos quatro mecanismos e zero alocação no callback. Esta entrega não cria ainda
o `AccessPort` editável, o gesto GUI de dois pontos ou o editor textual de
regras; são superfícies separadas e continuam abertas. Suíte integral,
benchmark, app real e escuta aguardam redução da swap do anfitrião. Detalhes em
[OPEN_INTERACTION_COHORT_DESIGN.md](../OPEN_INTERACTION_COHORT_DESIGN.md).

Ainda em 02/10, a coorte de refinamento fechou quatro lacunas sem ampliar a
superfície: sine-folder 4× nas vozes FM/BiomaBass, redução de ganho real do
Master realimentando o Cérebro com teto e balística, transição ar/água na
família `FLUID + PHASE` e gravação/reprodução da trajetória XY real da Lupa em
`GESTURE WAVES`. Os testes dirigidos, o motor completo e a segurança de tempo
real passaram. A suíte integral passou 45/45; o benchmark combinado mediu
29,69% em 48 kHz/bloco 128/quatro vozes. O maior custo foi 62,08% em
96 kHz/bloco 128 sem vozes externas. A comparação espectral é limitada ao caso
testado; escuta, medição no dispositivo real e meio por organismo continuam
abertos. Detalhes em
[CORE_REFINEMENT_COHORT_DESIGN.md](../CORE_REFINEMENT_COHORT_DESIGN.md).

O primeiro diretor de variação composicional também foi ligado ao core: ele
coordena segmentos determinísticos A, A′, B e suspensão, com permanência
variável e uma célula melódica autoral, nas fronteiras
coletivas, afetando Geometria, harmonia, famílias do banco e reflexões através
de `STRUCTURE MIX` e `LAB MIX` já existentes. Os acentos geométricos também
articulam o ganho das vozes, dosados por `STRUCTURE MIX`. O core estático compilou;
regressões, build do app, renders e escuta continuam pendentes. Não sincroniza
a DAW nem define compassos. Ver
[COMPOSITIONAL_VARIATION_DESIGN.md](../COMPOSITIONAL_VARIATION_DESIGN.md).

Para preparar a escuta comparativa dos quatro segmentos, o alvo offline
`aquorbium_compositional_variation_render` gera bypass/ativo e telemetria em
diretório local novo, com limite de 180 s. Está compilado, mas o render não foi
executado; instruções em
[COMPOSITIONAL_VARIATION_DESIGN.md](../COMPOSITIONAL_VARIATION_DESIGN.md).

As seções cronológicas abaixo registram o estado de cada protótipo quando foi
criado. Menções históricas a “isolado” ou “fora do app” foram superadas pelo
banco de 28/09, mas as integrações ecológicas especializadas nelas propostas
continuam abertas.

### Como ouvir o banco

Execute `./run`, role a coluna direita até `08 MÓDULOS` e abra primeiro
`LAB MIX` em valor moderado. Depois mova uma única família por vez. O cartão
mostra `LAB ATIVO N/15`, com a contagem real de famílias acima de zero. O banco
possui rolagem interna para suas oito linhas; os knobs mantêm o mesmo tamanho
dos demais cartões. Para voltar exatamente à rota anterior, coloque `LAB MIX`
em zero — não é necessário zerar as quinze famílias.

Historicamente, em 21/09 foram acrescentados ao core dois protótipos então isolados:
`SynthFMVoice` (wavetable senoidal, FM thru-zero, sine-shaping) e
`SlewRateRandomWalk` (deriva lenta limitada). Há teste específico para ambos,
Desde as coortes de 28–29/09 ambos participam do banco, e o random walk também
modula sutilmente o `MultiTapSpace`. O registro técnico e as pendências estão
em [aquorbium-tarefas.md](../aquorbium-tarefas.md).

Em 22/09, mais dois componentes então isolados foram codados: `LorenzCV` gera
controle caótico suavizado, e `BiomaGranularCapture` pode receber áudio,
detectar ataques e reproduzir grãos estéreo de um histórico circular fixo.
Ambos agora recebem rota pelo banco; a captura granular também deriva cada
evento dos traços reais do organismo. O contrato e os portões históricos do granular estão em
[GRANULAR_CAPTURE_DESIGN.md](../GRANULAR_CAPTURE_DESIGN.md).

Em 23/09 foram preparados `BiomaMarkov`, para decisões estruturais com
memória, e `AquariumTail`, uma cauda sintética estéreo. A Cauda deixou de ser
isolada em 25–26/09: agora complementa as reflexões existentes, possui quatro
macros no cartão `06 CAUDA`, cenas, bypass transparente e renders técnicos.
Em 26–27/09, `BiomaMarkov` também deixou o banco isolado: tornou-se memória de
forma da Geometria, avançada apenas por eventos coletivos e controlada por
quatro macros grandes no cartão `02 GEOMETRIA`. Contratos em
[BRAIN_DECISION_DESIGN.md](../BRAIN_DECISION_DESIGN.md) e
[SYNTHETIC_TAIL_DESIGN.md](../SYNTHETIC_TAIL_DESIGN.md).

Na mesma sessão foram implementados `ControlEventExtractor`, para transformar
cruzamentos de zero e envoltória em eventos, e `FractionalLogicClock`, com
dois relógios racionais combinados por AND/OR/XOR. O build completo e 25/25
testes passaram. Eles também permanecem fora do `./run`; integração exige
decidir fontes, destinos, sincronização e consequência musical. Ver
[CONTROL_EVENTS_AND_CLOCK_DESIGN.md](../CONTROL_EVENTS_AND_CLOCK_DESIGN.md).

Em 24/09, a Geometria recebeu dois protótipos então isolados.
`AddressableStepMemory` oferece 32 passos, quatro controles por passo e leitura
sequencial ou por endereço direto. `HarmonicProgression` oferece até 32 seções
ponderadas, graus de escala, nove qualidades de acorde e inversões. Os testes
específico e de heap passaram, assim como a suíte completa 26/26. Em 27/09,
ambos foram ligados à fronteira de eventos do `AutonomousEnergy` e tornaram-se
operáveis no cartão Geometria por `STRUCTURE MIX`, `STEP MEMORY`, `HARMONY` e
`PHASE PATH`. `STRUCTURE MIX=0` preserva o caminho anterior. A integração
técnica passou na suíte 40/40 e ainda aguarda escuta musical. Contrato em
[SEQUENCER_MEMORY_AND_HARMONY_DESIGN.md](../SEQUENCER_MEMORY_AND_HARMONY_DESIGN.md).
O build canônico resultante tem 14.040.848 bytes e SHA-256
`8d4c32c83aa24731f8b2f7f4dfeb76819d4a16124eeaeeb17e07062abb534766`.

Ainda em 24/09, `AccumulatingTransposer` acrescentou deslocamento acumulativo
em graus com direção derivada da fase sazonal, e `ThresholdStepSelector`
acrescentou seleção discreta de até 32 endereços por limiares configuráveis.
Os testes específicos e de heap passaram, assim como a suíte completa 27/27.
Ambos continuam isolados: não alteram o áudio do `./run`.

O mesmo incremento acrescentou `StepPatternBank`, com snapshots e cadeia de
padrões, e `RandomToLoop`, que atravessa continuamente acaso renovado e ciclo
fixo. Os contratos e portões de integração estão em
[PATTERN_EVOLUTION_DESIGN.md](../PATTERN_EVOLUTION_DESIGN.md). Eles também
permanecem fora da rota ao vivo. O build completo e a suíte 28/28 passaram
após a correção dos extremos numéricos do crossfade.

Também em 24/09, `BranchingStepRouter` materializou uma árvore de decisão em
duas camadas e `NotePermutationMatrix` acrescentou oito ordens sobre um pool
de 16 notas. `BiomaRelabi`, já ativo dentro de `AutonomousEnergy`, recebeu um
teste próprio de determinismo e reset. Ver
[DECISION_AND_PERMUTATION_DESIGN.md](../DECISION_AND_PERMUTATION_DESIGN.md). O
build completo e a suíte 29/29 passaram; esses caminhos continuam isolados da
rota ao vivo.

Na continuação de 24/09, `PatternRotationMutator`, `StepMemoryLayout` e
`DriftCurveShaper` fecharam rotação periódica, topologias 4 × 8/2 × 16/1 × 32
e curvas de deriva com suavidade independente. Contrato em
[GEOMETRY_EVOLUTION_DESIGN.md](../GEOMETRY_EVOLUTION_DESIGN.md). Permanecem
isolados e não mudam o som do aplicativo. O build completo e a suíte 30/30
passaram.

O lote seguinte acrescentou `SamplePercussionVoice`,
`HarmonicMorphOscillator` e `StochasticPulseMask`: respectivamente um fallback
curto com quatro amostras, um morph por coeficientes antes da síntese e uma
decisão probabilística mantida por pulsaret. O contrato está em
[PERCUSSION_AND_HARMONIC_MORPH_DESIGN.md](../PERCUSSION_AND_HARMONIC_MORPH_DESIGN.md).
Eles continuam fora do app; o build completo e a suíte 31/31 passaram.

Ainda em 24/09, `FractalCore` e `VectorGenerator` criaram duas bases
multirrate. A primeira conserva um processo autorreplicado em relógios lentos
ou de áudio; a segunda usa a mesma curva como envelope, LFO ou oscilador. O
contrato está em
[MULTIRATE_GENERATORS_DESIGN.md](../MULTIRATE_GENERATORS_DESIGN.md). Nenhum dos
dois está ligado ao app; o build completo e a suíte 32/32 passaram.

O lote probabilístico de 24/09 acrescentou `ShiftRegisterMemory`,
`ProbabilityDistribution` e `EvolutionLoopMachine`. Eles separam memória
binária mutável, forma estatística do acaso e estados de repetição/evolução.
Ver [PROBABILISTIC_MEMORY_DESIGN.md](../PROBABILISTIC_MEMORY_DESIGN.md). Os três
permanecem isolados; o build completo e a suíte 33/33 passaram.

Na sequência, `ConditionalStageMachine`, `TripleSlowDrift` e
`IntegrityCrossfade` acrescentaram forma condicional, deriva de longo prazo e
transição saudável↔trincado. O random walk passou a alcançar escalas de dezenas
de minutos. Ver [STRUCTURAL_CONTROL_DESIGN.md](../STRUCTURAL_CONTROL_DESIGN.md).
Os componentes continuam fora do app; build e suíte 34/34 passaram.

O marco seguinte criou `HarmonicPhaseBank` e `FluidMotion`: até oito membros
derivam de uma fase mestra, quatro saídas reinterpretam forma/amplitude/tempo/
frequência e um autopatch limitado deforma a velocidade. Ver
[SCHOOL_MOTION_DESIGN.md](../SCHOOL_MOTION_DESIGN.md). Eles ainda não substituem
o movimento atual; build e suíte 35/35 passaram.

O lote seguinte acrescentou `MotionWaveBank`, com oito formas e identidade
determinística, e `MotionGestureRecorder`, com até 2.048 pontos XY, velocidade
variável e mutação opcional por volta. `FluidMotion` passou a reutilizar o
banco, mas o gravador e os mapas de espécie/estratégia continuam isolados. Ver
[GESTURE_MOTION_DESIGN.md](../GESTURE_MOTION_DESIGN.md). O build completo e a
suíte 36/36 passaram.

Em 25/09, `SchoolSpectralBank` reuniu oito filtros, centro/dispersão e um perfil
determinístico por cardume. `EcologicalNoiseBed` acrescentou texturas derivadas
de estação/hora, cor de água e degradação temporal por trinca. A família agora
está ligada ao mix antes do Espaço e possui o cartão `05 AMBIENTE`, com seis
macros grandes, persistência e bypass transparente em `ENV MIX=0`. Ver
[SPECTRAL_ENVIRONMENT_DESIGN.md](../SPECTRAL_ENVIRONMENT_DESIGN.md). A integração
passou nos testes automatizados de resposta, cenas, layout e tempo real; escuta
humana e estabilidade no dispositivo real continuam pendentes.

Em 25–26/09, `AquariumTail` foi conectado após o espaço compartilhado. `TAIL
MIX`, `DECAY`, `DAMPING` e `NIGHTMARE` alteram a cauda tardia; em zero, a rota
é bit a bit transparente. O parâmetro de tamanho permanece fixo para evitar
descontinuidades. Testes de resposta, cenas, tempo real e quatro renders de
12 segundos passaram tecnicamente; a escuta humana continua pendente. Ver
[SYNTHETIC_TAIL_DESIGN.md](../SYNTHETIC_TAIL_DESIGN.md).

Em 26–27/09, `BiomaMarkov` foi integrado ao `AutonomousEnergy` como memória
formal de oito regiões. `FORM MIX`, `CONTEXT`, `INERTIA` e `MUTATION` atuam
somente nos próximos eventos coletivos e deslocam seção/contorno da Geometria;
não geram áudio, ganho ou notas diretamente. Em `FORM MIX=0`, os demais
controles são inativos e a saída é idêntica bit a bit, embora a memória siga
avançando. Resposta, cenas, tempo real, render e layout passaram tecnicamente;
escuta humana continua pendente. Ver
[BRAIN_DECISION_DESIGN.md](../BRAIN_DECISION_DESIGN.md).

Em 27/09, `OutputStage` foi integrado depois da Cauda como proteção técnica
separada do Master criativo que entrou em 30/09. Ele saneia não finitos, bloqueia DC,
suaviza trim/mute e limita o par estéreo com 5 ms de lookahead. O cartão `07
MASTER` expõe `OUTPUT TRIM`, `CEILING` e `RELEASE`; cenas passaram de 39 para
42 valores com migração compatível. Build completo, suíte 40/40 e auditoria de
28 capturas passaram. O pico intersample 4× ainda é estimado e não equivale a
conformidade BS.1770. Na primeira escuta manual ampla com áudio interno, o
usuário manipulou praticamente todos os controles sem perceber estalos/clipes
ou travamento; `XRUN`, `LATE`, `CLIP` e `GUI` permaneceram em zero em duas
observações consecutivas. Com a mesma sessão ainda em curso, os contadores
chegaram depois a `XRUN 2` e `LATE 2`; `CLIP` e `GUI` não foram reconfirmados
nesse ponto. A sessão continua sendo evidência positiva de melhora, mas não
uma execução sem falhas; a validação prolongada e a auditoria musical controle
a controle continuam abertas. Ver
[OUTPUT_STAGE_DESIGN.md](../OUTPUT_STAGE_DESIGN.md).

Em 01/10, a telemetria do mesmo estágio passou a incluir LUFS momentary,
short-term e integrated pós-safety. O teste de referência EBU confirmou
-23 LUFS para o par estéreo a -26 dBFS RMS por canal e a relação foi repetida
em 44,1, 48 e 96 kHz. Essa entrega não altera áudio, não escolhe alvo de
publicação e não resolve a pendência separada de true peak normativo.

O lote seguinte criou `PhilosophyVectorMixer`, `PhilosophySequencer`,
`PhilosophyMorph` e `PhilosophyMatrixMixer`: quatro fontes podem ocupar um
plano XY, seguir um padrão, cruzar estados discretos e alimentar dois
barramentos estéreo independentes. A semântica e a integração permanecem
abertas. Ver [PHILOSOPHY_MIX_DESIGN.md](../PHILOSOPHY_MIX_DESIGN.md). O build
completo e a suíte 38/38 passaram.

Instrumento generativo RASGO baseado no conceito de **Bioma**. A primeira
versão é um app standalone JUCE/C++ sob AGPL-3.0-or-later.

## Continuidade e acervo

Este README, a arquitetura, os contratos de tempo real, as medições, os testes
e a lista de tarefas formam o dossiê vivo do instrumento. O registro
`ARQ-AQU-001` permanece em desenvolvimento no catálogo central; uma versão
estável, entrega ou migração deverá acrescentar manifesto, checksum e teste de
restauração proporcionais ao material preservado. Não é necessário congelar ou
duplicar o acervo a cada experimento corrente.

### Correção em curso após teste de 2026-09-18

Lucio relatou estalos persistentes, travada da aplicação, controles sem efeito
percebido, knobs pequenos e sobreposições. Esta versão não está aprovada para
uso estável. A regra permanente de dimensões, separação e auditoria está em
[`WINDOW_LAYOUT_DESIGN.md`](../WINDOW_LAYOUT_DESIGN.md); o ponto de retomada está
em [`aquorbium-tarefas.md`](../aquorbium-tarefas.md).

Os cartões à direita agora rolam quando falta espaço, sem reduzir os knobs.
`STRIKER MIX` substitui o rótulo DEPTH: acima de zero, libera MATTER/POP/SCATTER,
que afetam o Striker granular nos próximos impactos, não todos os organismos.
Agora também eleva só o Striker na mistura, de 1 a 8 vezes, com rampa de 15 ms;
em zero preserva o balanço autônomo. `SPACE MIX` substitui GESTURE: libera
SIZE/SPREAD/TONE/MEMORY e eleva o retorno molhado até 2 vezes, limitado ao
teto de 0,75 já existente no motor. RETURN permanece
independente. `GEOMETRY PERFORMANCE` libera ritmo/rotação/intervalos/mutação;
alterações aparecem nos próximos eventos. Zero conserva a autonomia e mostra
os controles dependentes inativos; uma intenção totalmente neutra também
pode tornar o aumento da influência inaudível.

O botão `ÁUDIO` permite escolher saída, taxa e buffer; a configuração é
separada das cenas e persistida ao fechar. Para o próximo diagnóstico, tentar
48 kHz e 512 amostras, sem tratar esse valor como cura garantida. No cabeçalho,
o número após a barra é o buffer em amostras. Na rota ALSA/PipeWire interna,
o app substitui buffers salvos de 1–511 por 512 antes de abrir áudio. O seletor
próprio não oferece menos de 512 na saída PipeWire; hardware direto pode usar
128/256. Não há mais reconfiguração automática pelo timer após a abertura,
que podia bloquear a GUI; um estado externo inseguro fica visível no status.
Em 20/09, a sessão manual havia ficado em
48 kHz/16, com XRUN 209 e LATE 198; CLIP permanecia 0. O build novo passou
em 21/21 testes e uma abertura isolada com preferência de 16 mostrou 512 e
contadores zerados por alguns segundos. As mudanças de 21/09 ainda exigem
escuta manual prolongada; não garantem que a parada longa
tenha sido eliminada.

O binário final de 21/09 completou um novo ensaio automatizado de 20 s na
saída interna ALSA/PipeWire em 48 kHz/512: XRUN/LATE/CLIP/pausas GUI zerados,
sem amostras inválidas e pico 0,152. Ele confirma funcionamento técnico curto
da rota nova, não ausência de estalos em uso manual ou solução da travada
prolongada. Relatório na lista de tarefas.

Para abrir o instrumento normalmente e testá-lo à mão, entre nesta pasta e
execute `./run`. Esse atalho usa o build Release local, aceita argumentos
opcionais e não ativa captura nem auditoria por padrão.

Para investigar estalos com gestos manuais no áudio interno, feche primeiro
a instância anterior e use `./run diagnostico`. Ele cria uma sessão nova
sob `validation-output/escuta-*`, registra até 60 s de WAV/contadores e
encerra pelo timeout externo em 75 s se a janela não for fechada. O escritor
do WAV agora finaliza por tempo de parede após a duração solicitada mais 2 s,
mesmo sem callbacks; um `REPORT.txt` inicial guarda saída/taxa/buffer se a
GUI não conseguir substituí-lo pelo relatório final. Anote o
segundo aproximado do incidente; a gravação é pré-dispositivo, portanto
um problema do driver pode ser ouvido sem aparecer no WAV.

CPU é a carga estimada pelo JUCE, XRUN é o contador disponível no backend,
LATE conta callbacks do app acima da duração do bloco, CLIP conta blocos com
amostras de magnitude ≥1 e GUI conta intervalos da interface acima de 250 ms
(também pode contar suspensão do sistema). O tooltip de ÁUDIO contém os detalhes.
Desde 27/09 há um estágio técnico final com saneamento, bloqueio DC e limiter
lookahead; ele não repara xruns do dispositivo nem a travada da GUI. Ausência
de contadores continua não provando ausência de estalos ou de travada.

Diagnóstico automático disponível: `--runtime-audit=/diretorio/absoluto/novo`
executa 20 segundos de autonomia, macros, geometria/páginas e Lupa, e encerra
o app com relatório local. `--runtime-output="nome exato da saída"` escolhe
um dispositivo enumerado; saída inexistente, ambígua ou fallback não é aceita.
O diretório deve ser novo: o modo recusa reutilizar uma sessão existente,
isola preferências e não altera volume nem roteamento globais. Usar timeout
externo de 35 s; falta de relatório ou saída 124 significa ensaio incompleto.
O relatório contém métricas, não gravação WAV. Código de saída zero significa
coleta concluída com amostras finitas, não aprovação de estabilidade ou escuta.
Com `--runtime-stress`, a duração passa a 60 s: acrescenta recuperação das
cenas do banco isolado, alternância da autonomia e quatro vozes externas
programáticas. Nesse caso, usar timeout externo de 80 s. O relatório inclui
as contagens dessas ações; nenhuma cena normal do usuário é substituída.
Não misturar `--runtime-stress` com uso normal sem `--runtime-audit`.
Captura interativa opt-in, sem alterar o modo normal:

```bash
./build-reactivation-20260915/src/app/AquorbiumApp_artefacts/Release/Aquorbium \
  --capture-audio=/caminho/absoluto/para/sessao-nova \
  '--capture-output=Default ALSA Output (currently PipeWire Media Server)' \
  --capture-seconds=120
```

O nome de saída precisa corresponder exatamente ao dispositivo enumerado;
o exemplo foi validado no áudio interno em 20/09, mas pode mudar. A sessão
deve ter diretório novo. Preferências ficam isoladas ali; `CAPTURE.wav`
(estéreo float32, amostras antes do dispositivo) e `REPORT.txt` registram
contadores e controles a cada ~0,5 s. A gravação para no limite, mas a janela
permanece aberta; feche-a normalmente depois. Para limitar inclusive um
eventual bloqueio da janela, usar `timeout` externo superior ao tempo de
captura. Uma falha do driver pode ser audível sem aparecer nesse WAV.
Não é gravação da AudioBox nem prova de solução dos estalos. O smoke de 3 s
e o resultado de 21/21 testes estão na fila de retomada.

Em 18/09, dois ensaios reais de 20 s em 48 kHz/512 amostras terminaram pela
AudioBox USB direta e pela saída padrão ALSA via PipeWire: ambos com zero
XRUN/LATE/CLIP e zero pausas GUI >250 ms. A travada não se reproduziu nesse
protocolo curto. Não houve avaliação perceptiva, ensaio prolongado ou gesto
físico de mouse/MIDI; estalos e audibilidade dos macros continuam em aberto.
Resultados e candidato atual estão em [`aquorbium-tarefas.md`](../aquorbium-tarefas.md).

Na passagem para 19/09, o perfil de 60 s também terminou nas duas rotas, com
15 recuperações de cena, 10 trocas da autonomia e 420 eventos em cada: todos
os contadores acima permaneceram zerados. A AudioBox teve CPU JUCE amostrada
de 6,22% em média/29,19% no máximo; PipeWire, 6,24%/25,14%. São amostras da
estimativa, não pior deadline. Na versão anterior, o Striker respondeu
numericamente aos três macros, mas suas diferenças na mistura ficaram cerca
de 45–51 dB abaixo do RMS de referência. A revisão de 21/09 elevou essa
presença de forma limitada e medida, descrita em `aquorbium-tarefas.md`;
ainda não há aprovação por escuta. A travada e os estalos continuam abertos.

## Primeiro marco

O núcleo atual já possui um banco bioenergético autônomo de cinco organismos:
abrir o app basta para o Bioma produzir uma forma sonora evolutiva, sem nota
MIDI, arquivo ou host. Esse banco é determinístico por seed, mantém relógio
próprio a 100 Hz e não ocupa nem rouba as quatro vozes externas. Cada organismo usa um
motor coerente com sua espécie: um oscilador morph (seno → serra → pulso com
PolyBLEP), um Striker granular multi-partícula, um corpo modal excitado por
energia, um pulsar e uma corda física refinada. A densidade passa
a afetar também a matéria sonora. Há deriva estéreo, velocity, note on/off e
roubo de voz pela envolvente menos presente. Um `BiomaBrain` compartilha uma
fonte aleatória correlacionada e movimento lento entre os organismos, para que
o conjunto pareça um bioma — não cinco LFOs isolados. Não há alocação de
memória no caminho de renderização.

MIDI é uma porta opcional do app standalone: note on/off e velocity entram por
uma fila MPSC fixa, segura para GUI e múltiplas threads MIDI, e acionam as
mesmas vozes do Bioma. O processamento é limitado a 64 eventos por bloco e a
GUI torna qualquer overflow visível. O teclado não define a arquitetura; a
direção principal já implementada faz o som nascer de energia, densidade,
deriva e relações internas. O desenho e os limites desta primeira camada estão
em [`AUTONOMOUS_ENERGY_DESIGN.md`](../AUTONOMOUS_ENERGY_DESIGN.md); o contrato entre threads está em
[`REALTIME_CONTRACT.md`](../REALTIME_CONTRACT.md).

A medição reproduzível do core atual e seus limites de interpretação estão em
[`CPU_BASELINE.md`](../CPU_BASELINE.md). O executável `aquorbium_core_benchmark`
imprime a matriz completa em CSV sem transformar o resultado numa meta
arbitrária de produto.

A saída possui calibração acústica fixa de `×5` (+13,98 dB), aplicada depois da
medição de energia ecológica. Isso corrige o excesso de headroom do primeiro
render sem alterar decisões, homeostase ou relações internas. Não há compressor
ou limiter disfarçando a dinâmica; um cenário denso com autonomia e quatro vozes
externas chegou a pico `0,630`, mantendo cerca de 4,0 dB de margem.

Depois que a escuta revelou predominância da camada angular, stems determinísticos
mostraram que ela vinha do Morph, não principalmente do Pulsar. O balanço autoral
atual usa Morph `0,55`, Pulsar `1,20`, Granular `1,25`, Modal `1,50` e String
`1,50`. A distância Morph/Pulsar caiu de 9,2 para 2,4 dB: a aresta permanece,
mas o corpo redondo deixa de funcionar como fundo remoto.

Os mesmos stems revelaram um segundo problema: Striker, Modal e String eram
excitados corretamente, mas suas caudas eram apagadas uma segunda vez pela
energia instantânea do organismo. A integração agora calibra o nível nominal de
cada corpo e dá às memórias eventuais um piso de audibilidade ligado ao ramp
global da autonomia. Dormir não corta uma cauda física; desligar a autonomia
continua produzindo silêncio exato.

As entradas públicas do core também possuem política explícita contra
`NaN`/infinito: parâmetros inválidos preservam o último estado válido, velocity
inválida não cria voz e sample rates inválidos recebem fallback limitado. A
saída técnica futura continuará responsável por uma segunda defesa independente.

Os testes do core confirmam amostras finitas, nascimento de áudio sem `noteOn`,
reprodução idêntica com a mesma seed em blocos de tamanhos diferentes, mudança
efetiva com outra seed, rampas exatas de ativação e coexistência com as vozes
polifônicas externas.

O primeiro marco do novo Ecossistema também está integrado ao core. Ele
substitui, no código ativo, o protótipo conceitual antigo por uma física de
controle determinística e sem alocação: cinco organismos com perspectiva
parcial, papéis de primeiro plano/sustentação, cardume, estratégias, ambiente,
desejo, Lupa, presença, integridade causal e quatro modos autorais de escuta.
O contrato e seus limites atuais estão em
[`ECOSYSTEM_DESIGN.md`](../ECOSYSTEM_DESIGN.md); a auditoria do protótipo anterior
está em [`ECOSYSTEM_AUDIT.md`](../ECOSYSTEM_AUDIT.md).

O primeiro marco de Geometria também está ativo. Cinco lanes transformam
eventos ecológicos em gates, acentos e alturas por distribuição euclidiana e
campo intervalar; `Geometry Performance` mistura continuamente densidade,
rotação, viés e mutação humanos com o percurso autônomo. O desenho está em
[`GEOMETRIC_SEQUENCER_DESIGN.md`](../GEOMETRIC_SEQUENCER_DESIGN.md).

O terceiro organismo já deixou de ser provisório. `BiomaModalResonator` soma
quatro modos amortecidos e interpola relações harmônicas com modos inarmônicos
de membrana. Eventos ecológicos o excitam sem reiniciar suas caudas; material,
decaimento, brilho, dureza e ponto de impacto aceitam intenção performática
híbrida. Pesquisa, equações e limites estão em
[`MODAL_RESONATOR_DESIGN.md`](../MODAL_RESONATOR_DESIGN.md).

O segundo organismo agora é `BiomaGranularStriker`: cada evento cria de duas a
doze partículas sintéticas de 2–40 ms, com dispersão temporal e espectral,
envelope exponencial e passa-banda de Q 20–100. Não usa arquivo ou amostra e
mantém pool fixo no callback. Conceito, fontes, limites e evidência estão em
[`GRANULAR_STRIKER_DESIGN.md`](../GRANULAR_STRIKER_DESIGN.md).

No cartão Síntese, `STRIKER MIX`, `MATTER`, `POP` e `SCATTER` oferecem uma superfície
performática compacta para o Striker. Eles deslocam somente partículas futuras,
preservando caudas em curso, e seus valores são persistidos entre sessões. O
organismo aparece como `GRAIN` no aquário e mostra a população ativa durante
uma nuvem.

O quinto organismo é `BiomaPhysicalString`. Ele recircula
energia em uma linha de atraso com afinação fracionária, filtro de perdas,
posição de pinçamento e barreira suave de energia. A escuta aprovou a versão
refinada como uma identidade diferente — não como substituta do Modal ou do
Striker — e ela entrou como espécie própria, autônoma e acionável pelas vozes
externas. Ainda não possui macros dedicadas na interface. Modelo,
limites e evidência estão em
[`PHYSICAL_STRING_DESIGN.md`](../PHYSICAL_STRING_DESIGN.md).

As espécies audíveis agora chegam a esses motores por
`OrganismVoiceEngine`: o despacho é feito pela espécie, sem números mágicos de
índice e sem método virtual dentro da física ecológica. O contrato está em
[`ORGANISM_VOICE_ENGINE_DESIGN.md`](../ORGANISM_VOICE_ENGINE_DESIGN.md).

O primeiro corpo do módulo Espaço também está ativo. Cada fonte passa por
panorâmica de potência constante, profundidade espectral e suavização própria;
`AquariumPose` separa rigorosamente o espaço local da esfera do mundo externo.
Mover a pose física é inaudível por padrão e só participa da percepção quando o
vínculo acústico é habilitado deliberadamente. O contrato está em
[`AQUARIUM_SPACE_DESIGN.md`](../AQUARIUM_SPACE_DESIGN.md).

O segundo corpo de Espaço está integrado como uma única memória compartilhada:
`MultiTapSpace` usa oito leituras fracionárias cúbicas de uma memória estéreo,
com ganho, pan e filtro independentes, feedback limitado por construção e
macros híbridas de tamanho, abertura, tom e memória. Luz, temperatura, desejo e
integridade deslocam esse campo a 200 Hz sem copiar os delays por organismo. O
retorno wet pós-soma começa em `0,36`; a homeostase ainda observa a soma direta,
evitando feedback ecológico oculto. O dossiê está em
[`MULTITAP_SPACE_DESIGN.md`](../MULTITAP_SPACE_DESIGN.md).

Os sons `Neutral` e `Active` são dois estados expressivos, não versões pior e
melhor. No app, `Ecosystem Morph` faz a passagem contínua entre eles em 80 ms,
sem reiniciar seed, vozes ou relógio ecológico. Density, Drift e Morph são
publicados atomicamente para a thread de áudio.

Em `ECO CONTROLS`, o painel central mostra `COHESION` e `DANGER`, além dos
pesos das quatro perspectivas de escuta (`SPARSE FOCUS`, `ENERGY MEMORY`,
`GEOMETRIC CYCLE`, `COLLECTIVE DRIFT`). Os pesos são normalizados no core;
quatro zeros recuperam a distribuição igual. `DENSITY` e `DRIFT` permanecem
na faixa inferior e completam os quatro eixos de `Desire`: `DRIFT` é o inverso
de `calm`. O botão `AQUARIUM VIEW` retorna ao aquário. Os seis valores do
painel são persistidos nas preferências locais do app.
Esses oito parâmetros ecológicos formam um `ValueTree` versionado na thread
da GUI. Desde 2026-09-16, `PerformanceParameterState` guarda também Ecosystem
Morph, cinco controles de Geometria performática, quatro da memória formal,
quatro do Striker, seis do Espaço, seis do Ambiente, quatro da Cauda, três da
saída técnica, quatro controles de estrutura, dezesseis do banco experimental,
um do Master criativo e ativação autônoma: 63 valores
persistentes no total. Ao abrir,
o app prefere
o snapshot completo válido; se ausente ou inválido, recupera o estado ecológico
anterior e as chaves individuais. Ao salvar, conserva os dois formatos antigos
para compatibilidade. O core continua recebendo apenas os alvos atômicos.
Gate externo e gesto da Lupa são transitórios e não são reativados ao abrir.
Isso prepara presets/automação futura, mas ainda não é APVTS nem plugin.

### Cenas locais

Em `ECO CONTROLS`, escolha `Cena 1` a `Cena 4`. `GRAVAR` guarda os 63 valores
atuais e o nome digitado na cena escolhida, substituindo a cena anterior desse
slot. `RECUPERAR`
aplica a cena; fica desabilitado quando ela está vazia. `INICIAL` restaura os
valores iniciais sem apagar nenhuma cena. A cena é salva nas preferências
locais do standalone, não no repositório nem em arquivos de áudio.

O campo `Nome da cena` aceita até 32 caracteres, em uma única linha. Acentos
e símbolos como `&`, aspas e `<` são preservados. Para uma cena já salva,
`RENOMEAR` ou Enter confirma somente o nome, sem alterar os parâmetros ou o
som. Nome vazio remove o nome personalizado. Em um slot vazio, digite um
nome opcional e use `GRAVAR`. Trocar de slot descarta a edição de nome ainda
não confirmada; abrir/cancelar um diálogo de arquivo conserva essa edição.

`DESFAZER` recupera a última alteração efetiva do banco: gravação, importação
ou renomeação. Recupera parâmetros e nome anteriores; se era um slot vazio,
volta a ficar vazio. Seleciona o slot afetado, mas não aplica seus parâmetros
ao áudio: use `RECUPERAR` se desejar ouvi-los. Há um único nível, sem refazer;
o histórico desaparece ao fechar o app. Operações sem mudança e tentativas
inválidas não consomem esse histórico. Nome/desfazer ficam bloqueados durante
diálogo de transporte ou confirmação de substituição.

Recuperar atualiza todos os controles em um único quadro entregue no início
do próximo callback que observar a publicação. Não reinicia seed, notas,
relógio ecológico ou memórias dos motores. Gate e Lupa não são gravados; um
gesto transitório já em curso permanece. Não há morph entre cenas: cada motor
mantém suas suavizações existentes. A escuta das transições ainda está pendente.

Banco XML inválido é recusado; seu conteúdo nas preferências não é substituído
ao fechar. Uma gravação explícita passa a salvar o banco local novo. O status
informa falha de escrita, se ocorrer; nomes e desfazer seguem disponíveis em
memória. O banco V2 persiste nomes; o V1 anterior é lido quando não há V2 e
continua salvo como representação dos parâmetros para builds antigos. Não
há fusão automática de edições feitas alternadamente em builds antigos e
novos: o novo app prefere V2. Morph e automação de host continuam pendentes.

### Transportar uma cena

O nome personalizado é metadado local do slot: não entra no `.aquscene`.
Importar preserva o nome já confirmado no slot; num slot vazio, fica sem
nome. `DESFAZER` pode recuperar a cena anterior à importação. Exportar não
modifica o banco nem o histórico e conserva a estrutura XML V1, agora com os
63 valores atuais.

`EXPORTAR CENA` exporta a cena salva no slot escolhido, não os controles que
você modificou depois. Use `GRAVAR` primeiro se quiser incluir essas mudanças.
Escolha um destino local; a extensão `.aquscene` é acrescentada automaticamente.
Para substituir um arquivo existente, confirme no diálogo. O arquivo anterior
fica ao lado como `.bak` (ou um nome numerado, se já houver backup). O tooltip
do status informa os caminhos. Backups não são apagados automaticamente.

`IMPORTAR CENA` lê um `.aquscene` para o slot escolhido. Se ele já estiver
ocupado, pede confirmação antes de substituir a cena local. Importar não muda
o som: clique `RECUPERAR` para aplicar. Exportar e importar congelam o slot
no momento de abrir o seletor; apenas uma transferência pode estar em curso.
Cancelar mantém os arquivos, o banco e o som anteriores.

O formato V1 atual contém os 63 valores normalizados. Cenas V1 anteriores com
25 valores continuam válidas e recebem os valores iniciais do Ambiente, da
Cauda e da memória formal; cenas com 31 valores recebem Cauda e memória
formal; cenas com 35 valores recebem os quatro valores da memória formal;
cenas com 39 valores recebem os três valores iniciais da saída técnica; cenas
com 42 valores recebem os quatro valores iniciais da nova estrutura; cenas com
46 valores recebem o banco experimental inteiro em bypass; cenas com 62
valores recebem o Master criativo em bypass.
Arquivos vazios,
maiores que 16 KiB, UTF-8 inválido, links simbólicos, parâmetros desconhecidos,
valores fora de `[0, 1]`, DTD, entidades e XML excessivamente aninhado são
recusados sem mudar o slot. A escrita usa um temporário na mesma pasta, verifica
o flush, relê/valida o temporário e preserva o destino anterior antes da
substituição. Se falhar, o status mostra o erro; eventual backup permanece
disponível. O transporte é
de uma cena por arquivo, não do banco inteiro.

A leitura/escrita é limitada e ocorre na GUI, nunca no callback de áudio.
Use arquivos locais e evite pastas de rede: tamanho limitado não garante
latência de filesystem. Não há trava contra outro programa modificando o
mesmo destino simultaneamente, nem garantia de durabilidade após falha física
do disco. Os diálogos ainda precisam de ensaio real no sistema anfitrião.

## Painel visual

O app já possui o primeiro layout funcional do instrumento. Sete cartões
aparecem ao redor do Aquário central: Cérebro, Geometria, Síntese, Espaço,
Ambiente, Cauda e Master. O cartão Master reúne o macro criativo e a saída
técnica, preservados como camadas distintas no motor.

O cartão Espaço é uma superfície performática real: `RETURN` regula a presença
wet, `SPACE MIX` mistura autonomia e intenção humana, e `SIZE`, `SPREAD`, `TONE`
e `MEMORY` controlam o mesmo `MultiTapSpace` compartilhado. Os seis controles
persistem entre sessões e chegam ao callback por atomics lock-free; em
`SPACE MIX=0`, os quatro últimos ficam inativos e não apagam a resposta ecológica.

O cartão Ambiente controla a primeira família retirada do banco de protótipos
e ligada integralmente ao instrumento. `ENV MIX` ativa a rota; `FILTER`,
`CENTER`, `SPAN` e `RESONANCE` moldam o banco espectral, enquanto `AMBIENCE`
regula o leito ecológico. Em zero, a rota é transparente e os cinco controles
dependentes ficam inativos. O cartão fica abaixo de Espaço na coluna rolável,
sem reduzir os knobs para caber.

O cartão Cauda fica entre Ambiente e Master. `TAIL MIX` regula a presença do
FDN tardio; `DECAY`, `DAMPING` e `NIGHTMARE` controlam duração, absorção e
circulação. Em `TAIL MIX=0`, os controles dependentes ficam inativos e a saída
anterior é preservada bit a bit. O cartão usa quatro knobs grandes e rolagem,
sem expor `size` enquanto sua mudança puder criar descontinuidade.

O cartão Geometria contém agora uma segunda superfície rolável com quatro
knobs grandes. `FORM MIX` revela a memória de forma; `CONTEXT`, `INERTIA` e
`MUTATION` moldam a próxima transição Markov e ficam inativos em zero. A
rolagem preserva diâmetro e área gestual nas janelas compactas. Os cinco
sliders lineares de intenção geométrica continuam na faixa inferior e não
foram substituídos.

O cartão Master expõe quatro macros grandes. `CREATIVE MASTER` controla a
cadeia multibanda e mostra `CREATIVE + SAFETY` quando ativo; `OUTPUT TRIM`,
`CEILING` e `RELEASE` pertencem ao estágio técnico posterior. O Master
criativo reage ao Ecossistema, enquanto a saída técnica saneia amostras,
remove DC e limita com lookahead sem modulação ecológica. O detector
intersample 4× ainda é uma estimativa, não conformidade BS.1770; detalhes em
[`CREATIVE_MASTER_COHORT_DESIGN.md`](../CREATIVE_MASTER_COHORT_DESIGN.md) e
[`OUTPUT_STAGE_DESIGN.md`](../OUTPUT_STAGE_DESIGN.md).

O Aquário central recebe uma cópia segura da telemetria real a 30 quadros por
segundo. Posição, energia, observação e detalhe dos cinco organismos são
publicados pela thread de áudio somente por escalares atômicos lock-free; a GUI
não acessa o estado mutável da engine nem bloqueia o callback. O indicador de
áudio também usa o heartbeat do callback real, sem declarar o dispositivo
online apenas porque a janela abriu.

No sentido GUI → áudio, controles e gesto da Lupa usam agora um quadro completo
com três buffers fixos. O callback não recebe uma combinação parcial de cenas
e só reaplica controles quando há um quadro novo. MIDI continua numa fila
limitada separada.

Para usar a Lupa, pressione e arraste dentro da esfera. Enquanto o botão do
mouse está pressionado, o observador 1 ocupa a Lupa; soltar libera a posse.
Arraste com `Alt` para mirar a metade posterior da esfera. O anel maior mostra
a posição real, que desliza em direção ao pequeno marcador de destino; anéis
mais apagados indicam a face posterior. A Lupa modifica detalhe e presença da
perspectiva do primeiro observador (índice 0 no core), não gera som por conta
própria. O rodapé distingue `LUPA ATIVA` de `LUPA LIVRE`: depois de soltar,
o anel conserva a posição, mas não há foco sonoro. `FOCO N` conta organismos
presentes que o foco geométrico alcança; zero significa que nenhum está no
alcance angular/profundo da Lupa, não que o motor parou. Essa contagem não mede
volume: o peso da perspectiva também precisa ser maior que zero. Nos quatro pesos iguais,
a Lupa só modifica uma das quatro perspectivas. Para uma escuta diagnóstica,
use Sparse Focus = 100% e os demais pesos = 0% em `ECO CONTROLS`, depois volte
aos pesos habituais. Organismos profundos ou fora do foco não são ampliados.

Desde 2026-09-17, a mistura sonora ecológica tem suavização amostra a amostra
com constante de 15 ms, incluindo pressionar/soltar a Lupa. A borda angular
do foco é contínua e o movimento chega ao alvo sem ultrapassagem, inclusive
em alvos opostos. A velocidade continua 0,55 rad/s: uma meia volta leva cerca
de 5,7 s. Isso corrige degraus de controle; não certifica ausência de estalos
no dispositivo real.

Dois pares locais novos para a próxima escuta estão em
`validation-output/2026-09-17-lens-{equal,solo}-ab/`: alvos móveis na direção do
organismo 0 e na direção espelhada, seed e notas iguais, banco autônomo ligado,
feedback ecológico desligado e RMS global
igualado. Posse de 2 a 12 s; comparar foco/timbre/panorama e os instantes
de pressionar/soltar. Não comparar esses WAVs como se fossem a mesma versão
DSP dos pares históricos de 15/09. A homeostase do banco autônomo permanece
ativa; não é uma comparação isolada de filtro. O diagnóstico anterior de dois
alvos fixos permanece separado em `validation-output/2026-09-17-lens-equal-diagnostic/`:
nenhum deles alcançou organismos no intervalo de posse e os WAVs são idênticos.

Para gerar outra sessão sem sobrescrever as anteriores:

```bash
timeout 45 ./build-reactivation-20260915/aquorbium_ab_render validation-output/lens-nova-sessao --lens
timeout 45 ./build-reactivation-20260915/aquorbium_ab_render validation-output/lens-solo-nova-sessao --lens --observer=0
```

Só depois de preencher `LISTENING_PROTOCOL.md`, repetir o comando da sessão
com `--reveal` para obter a chave, sem regravar os arquivos de escuta.

A janela separa canvas artístico de superfície disponível: abre dentro da área
útil de um monitor, restaura bounds validados e possui modos completo, compacto
e emergência sem aplicar zoom global. O desenho e as evidências estão em
[`WINDOW_LAYOUT_DESIGN.md`](../WINDOW_LAYOUT_DESIGN.md).

## Prova A/B do Ecossistema

O alvo `aquorbium_ab_render` gera uma sessão cega: `A.wav` e `B.wav`, ambos com
20 segundos, mesma seed e mesmos eventos. O take ativo é igualado por RMS ao
neutro para uma escuta justa. Na prova controlada, o feedback de energia fica
desligado nos dois lados, pois ele é uma variável musical a ser avaliada em
rodada própria.

```sh
cmake -S . -B build-ab -DAQUORBIUM_BUILD_APP=OFF
cmake --build build-ab
./build-ab/aquorbium_ab_render validation-output/aquorbium-ecosystem-ab
```

Preencha `LISTENING_PROTOCOL.md` ouvindo somente os WAVs da raiz. Os CSVs,
medidas e identidade técnica dos takes ficam em `analysis/` e só devem ser
abertos após a resposta. Para revelar a chave depois da escuta:

```sh
./build-ab/aquorbium_ab_render validation-output/aquorbium-ecosystem-ab --reveal
```

O contrato da comparação está em [`ECOSYSTEM_DESIGN.md`](../ECOSYSTEM_DESIGN.md).

Para comparar uma perspectiva isolada às quatro perspectivas com pesos iguais,
use `--observer=0`, `1`, `2` ou `3` em um diretório novo. As duas tomadas usam
a rota `Active`, a mesma seed e os mesmos eventos; o renderizador iguala o RMS
da perspectiva isolada ao da mistura de quatro observadores. As quatro sessões
prontas para escuta, ainda sem chave revelada, estão em:

```text
validation-output/2026-09-15-perspective-sparse-ab/
validation-output/2026-09-15-perspective-energy-memory-ab/
validation-output/2026-09-15-perspective-geometric-cycle-ab/
validation-output/2026-09-15-perspective-collective-drift-ab/
```

Cada diretório contém `A.wav`, `B.wav` e `LISTENING_PROTOCOL.md`. As quatro
referências de pesos iguais são bit a bit idênticas; os quatro takes de foco
são distintos. Isso verifica que só o peso focal muda entre sessões, não a
referência nem os eventos.

Para uma sessão nova ou para revelar sua chave após preencher a ficha:

```sh
./build-ab/aquorbium_ab_render validation-output/minha-perspectiva-ab --observer=0
./build-ab/aquorbium_ab_render validation-output/minha-perspectiva-ab --observer=0 --reveal
```

O segundo comando grava apenas `REVEAL_AFTER_LISTENING.txt`; não reescreve
WAVs nem a ficha. Sem `--reveal`, uma sessão já existente é recusada. Não
abra `analysis/` nem rode `--reveal` antes de registrar a escuta.

O A/B antigo em `validation-output/aquorbium-ecosystem-ab/` é evidência
histórica de agosto, não referência de nível para os renders atuais. No
manifesto antigo, o RMS neutro/ativo antes de igualar era
`0,0110563`/`0,00343291`; no par refeito em 2026-09-15, em
`validation-output/2026-09-15-ecosystem-ab-regression/`, é
`0,0519969`/`0,0187339`. Essa diferença é compatível com a calibração
posterior da saída, mas não é prova causal isolada. Para uma escuta atual,
use o par de 2026-09-15 e nunca compare takes de épocas diferentes como se
fossem um mesmo A/B controlado.

## Render autônomo sem MIDI

O alvo `aquorbium_autonomous_render` produz 30 segundos de áudio, telemetria e
um relatório técnico sem enviar qualquer evento externo:

```sh
cmake --build build-release --target aquorbium_autonomous_render
./build-release/aquorbium_autonomous_render \
  validation-output/aquorbium-autonomous
```

O WAV serve para escuta e calibração, não como alegação de que composição,
afinação e paleta sonora já estão finalizadas.

Para diagnosticar balanço sem mudar seed ou percurso, o mesmo render aceita
`--feedback-off --solo=oscillator|granular|resonator|pulsar|string`. A espécie
isolada soa, mas todos os cinco motores continuam avançando em silêncio.

Para comparar o Striker multi-partícula com o fallback Morph sob a mesma seed,
81 eventos ecológicos e feedback desligado:

```sh
./build-release/aquorbium_autonomous_render \
  validation-output/aquorbium-granular-on --feedback-off
./build-release/aquorbium_autonomous_render \
  validation-output/aquorbium-granular-off --feedback-off --granular-off
```

Os três caracteres isolados da superfície performática são gerados por:

```sh
cmake --build build-release --target aquorbium_granular_striker_render
./build-release/aquorbium_granular_striker_render \
  validation-output/aquorbium-striker-characters
```

O estudo Physical String produz `filament`, `shell` e `blade`, todos com a
mesma seed e a mesma sequência de oito impactos:

```sh
cmake --build build-release --target aquorbium_physical_string_render
./build-release/aquorbium_physical_string_render \
  validation-output/aquorbium-string-characters-refined
```

Para decidir seu papel sem viés de nome ou volume, a comparação cega com Modal
e Striker gera três takes em RMS idêntico, sem limiter:

```sh
cmake --build build-release --target aquorbium_physical_bodies_blind_render
./build-release/aquorbium_physical_bodies_blind_render \
  validation-output/aquorbium-physical-bodies-refined
```

O laboratório isolado de reflexões compartilhadas gera uma resposta wet e uma
sonda musical dry+wet:

```sh
cmake --build build-release --target aquorbium_multitap_space_render
./build-release/aquorbium_multitap_space_render \
  validation-output/aquorbium-multitap-space
```

Para comparar a integração sob a mesma seed e os mesmos 101 eventos:

```sh
./build-release/aquorbium_autonomous_render \
  validation-output/aquorbium-multitap-integrated-on --feedback-off
./build-release/aquorbium_autonomous_render \
  validation-output/aquorbium-multitap-integrated-off \
  --feedback-off --multitap-off
```

A chave só aparece com `--reveal`, depois da escuta. Método, medições anônimas
e decisão de memória estão em
[`PHYSICAL_BODIES_COMPARISON.md`](../PHYSICAL_BODIES_COMPARISON.md).

Para comparar o novo organismo modal com a antiga fonte morph usando a mesma
seed, eventos e relógio:

```sh
./build-release/aquorbium_autonomous_render \
  validation-output/aquorbium-resonator-on
./build-release/aquorbium_autonomous_render \
  validation-output/aquorbium-resonator-off --resonator-off
```

Para isolar a nova fundação espacial, desligue o feedback nos dois renders. O
espacializador continua avançando na referência, mas a rota antiga é ouvida:

```sh
./build-release/aquorbium_autonomous_render \
  validation-output/aquorbium-space-on --feedback-off
./build-release/aquorbium_autonomous_render \
  validation-output/aquorbium-space-off --feedback-off --space-off
```

## Compilar e abrir o standalone no Linux

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
  -DAQUORBIUM_JUCE_PATH=../../third_party/JUCE
timeout 240 cmake --build build --parallel 1
timeout 90 ctest --test-dir build --output-on-failure
./build/src/app/AquorbiumApp_artefacts/Release/Aquorbium
```

O caminho do JUCE acima é o checkout local usado nesta retomada. Ajuste
`AQUORBIUM_JUCE_PATH` se estiver em outro lugar. Para ensaio isolado de
preferências, acrescente `--settings-dir=/caminho/temporario` ao comando do
app. A compilação Linux e os 41 testes disponíveis passaram em 2026-09-28; a
interação visual e o áudio com dispositivo real ainda precisam de ensaio
humano no sistema anfitrião.

Para abrir o candidato já compilado nesta retomada, sem recompilar:

```sh
cd <pasta do Aquorbium>
./build-reactivation-20260915/src/app/AquorbiumApp_artefacts/Release/Aquorbium
```

Conferir antes se outra instância está aberta, para não testar um binário
antigo ou disputar o dispositivo. O checksum do candidato e as evidências
estão na [fila de retomada](../aquorbium-tarefas.md). Em 18/09, a revisão
documental reconfirmou o checksum e o relatório gráfico da primeira correção;
a rodada seguinte acrescentou o diagnóstico ao vivo, recompilou o app e repetiu
20/20 CTest. A evidência gráfica é da revisão de layout anterior; o novo modo
não altera a geometria do painel.

Arquitetura: [aquorbium-arquitetura.md](../aquorbium-arquitetura.md).
Tarefas: [aquorbium-tarefas.md](../aquorbium-tarefas.md).
Pesquisa de fontes: [ENERGY_SOURCES_LAB.md](../ENERGY_SOURCES_LAB.md).
Padrão dos módulos: [MODULE_DEVELOPMENT_STANDARD.md](../MODULE_DEVELOPMENT_STANDARD.md).
