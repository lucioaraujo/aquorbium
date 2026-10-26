# Rasgo: Aquorbium — Arquitetura Conceitual e Técnica

Atualização de caminhos: 2026-10-11; decisões técnicas mantidas.

*Documento vivo. Nada aqui é descartado, só organizado — o que for redundante ou divergente do conceito será marcado, não apagado.*

**Hierarquia de fontes:** este arquivo e `aquorbium-tarefas.md` são os
documentos consolidados do projeto. `biome.odt` é memória bruta de pesquisa e
conversa: preserva ideias úteis, mas seus exemplos de código não são
especificação. Em particular, o `applyBrickwallLimiter` do ODT é hard clipping,
não limiter com lookahead/true peak, e não deve ser copiado.

**Hierarquia de implementação do Ecossistema:** `src/core/Ecosystem.h/.cpp` é
a implementação canônica e `ECOSYSTEM_DESIGN.md` é seu contrato. O protótipo
legado que ocupava `Ecosystem.h` na raiz foi auditado e removido; não é fonte de
arquitetura. Seu diagnóstico histórico permanece em `ECOSYSTEM_AUDIT.md`.

**Nascimento sonoro autônomo:** `src/core/AutonomousEnergy.h/.cpp` é a primeira
ponte estável entre a física do Ecossistema e o áudio. Seu contrato está em
`AUTONOMOUS_ENERGY_DESIGN.md`. O módulo produz energia, altura e duração para
quatro organismos sem MIDI e usa um banco sonoro separado das vozes externas.

**Registro transversal:** método, parentesco e artefatos públicos são mantidos em
[`METODOLOGIA_DE_DESENVOLVIMENTO.md`](../../RASGO_DOCUMENTATION/METODOLOGIA_DE_DESENVOLVIMENTO.md),
[`HISTORICO_GLOBAL.md`](../../RASGO_DOCUMENTATION/HISTORICO_GLOBAL.md)
e
[`ACERVO_E_PUBLICACAO.md`](../../RASGO_DOCUMENTATION/ACERVO_E_PUBLICACAO.md).
Este arquivo continua sendo a fonte da identidade e arquitetura específicas do
Aquorbium.

**Integração especializada vigente:** a coorte de 29/09 liga mecanismos que
antes eram somente protótipos genéricos ao estado real do instrumento. O
quantizador decide o subgrave; os traços do organismo decidem os grãos; desejo,
integridade e traços dão forma ao Pulsar; a saída final volta causalmente ao
sequenciador; o contexto do Cérebro recebe soft take-over; e a deriva lenta
atua nos tempos do Espaço. O contrato e os limites estão em
`SPECIALIZED_INTEGRATION_COHORT_DESIGN.md`.

---

## 0. Onde isso se encaixa

**Rasgo** é o guarda-chuva que abarca os instrumentos originais *synth DIY* de Lúcio — não é o nome de um instrumento, é a marca que reúne todos eles. Este documento descreve um instrumento específico dentro desse guarda-chuva, guiado pelo conceito **Bioma**.

**Nome definido: Aquorbium** (em inglês, como o resto da nomenclatura do projeto). Chegou depois de várias rodadas de verificação de candidatos: "Aquarium" sozinho, ficou limpo mas genérico demais; "Spherical Aquarium", limpo mas descritivo/burocrático; "Spherium", descartado por colidir de fato com "Spherum FX Re-Synthesizer" (Eplex7 DSP); "Squarium", limpo mas com contradição semântica com a forma esférica; "Aquarical", limpo e forte, mas perde a referência explícita à esfera; "Orb"/"Orbarium", "Orb" descartado por saturação de mercado ("Orb Plugins", "Orb Synth X", "Orbinator"), "Orbarium" limpo mas com risco de diluição; "Orquarium", limpo contra produtos, mas com colisão fonética real com "orquidário" (jardim de orquídeas, PT/ES) — risco de leitura errada. **Aquorbium** (aqu- + orb + -ium) foi verificado limpo contra sintetizadores, plugins e módulos Eurorack existentes, e sem colisão semântica com nenhuma palavra real encontrada — mantém tanto a raiz aquática quanto a referência à esfera/orbe, sem repetir "Aquarium" ou "sphere" de forma literal. O nome também não é decorativo: o Ecossistema canônico é construído em torno da metáfora do aquário esférico — organismos, observadores e o próprio tanque — então o nome descreve com precisão como o motor por dentro funciona.

**Nome curto adotado: Aquorb.** Aquorbium também pode ser chamado de Aquorb — não é um apelido informal solto, é a forma curta oficial do mesmo nome (aqu- + orb, o mesmo miolo de Aquorbium sem o sufixo -ium), útil pra fala corrida, logotipo ou onde "Aquorbium" for longo demais. Verificado limpo (ver seção 8): sem conflito com nenhum synth, plugin, módulo Eurorack ou produto de áudio; as únicas ocorrências encontradas são um monstro de fã-wiki não-comercial ("My Singing Monsters Fanon Wiki") e um Pokémon de fã-arte — nenhum dos dois é um produto real nem tem qualquer relação com música/áudio, risco desprezível.

**Primeira forma de produto:** app DSP multiplataforma. A arquitetura pode
permitir plugin posteriormente, mas a primeira versão deve funcionar como
instrumento autônomo. Console embarcado ou hardware dedicado pertencem a uma
etapa futura e não devem contaminar o core DSP com dependências de plataforma.

**Fronteira visual entre instrumentos:** o Aquorbium preserva o Aquário como
interface e metáfora operacional. Um futuro maestro/orquestrador RASGO pode
usar uma superfície modular própria; Eurorack é apenas uma analogia possível,
não uma decisão visual. Esse sistema deve nascer separado, não como skin deste.
Motores só poderão ser compartilhados depois de maduros e promovidos pelo
portão G4; GUI, fluxo de performance e identidade visual continuam específicos
de cada instrumento.

"Rasgo Synth" (usado nas primeiras versões deste documento) foi um apelido de trabalho que confundia guarda-chuva com instrumento — corrigido aqui pra não se perder de vista conforme outros instrumentos da série Rasgo forem entrando.

## 1. Conceito

O Aquorbium é generativo, original, escrito do zero em C++/JUCE, licenciado em **AGPLv3**, no espírito do DIY synth: pesquisa, se inspira e se apropria livremente de técnicas e tecnologias de código aberto — sem clonar nenhum instrumento específico.

O conceito guarda-chuva é **Bioma**: o instrumento como ecossistema vivo, onde módulos se influenciam mutuamente (feedback cruzado) em vez de seguir um fluxo linear MIDI → som → saída. Isso deixou de ser só uma metáfora de nome — depois de traduzida no Ecossistema canônico (ver `ECOSYSTEM_DESIGN.md`), ela revelou um princípio que organiza o sistema inteiro: **nenhuma entidade tem visão total**. Nem o organismo (campo de percepção limitado + visão subaquática atenuada), nem o observador de fora (oclusão + o viés do próprio modo de escuta), nem o próprio conteúdo do aquário é fixo no tempo (presença, desejo, integridade estrutural). Perspectiva parcial e situada, aplicada recursivamente do macro (qual dos 4 modos de escuta domina agora) ao micro (o que este organismo específico consegue enxergar neste instante) — essa é a coluna vertebral conceitual do instrumento, não só uma imagem bonita.

O aquário também pode mover-se no mundo. Isso exige dois referenciais
explícitos, não posições improvisadas: seres internos, água e Lupa permanecem
em coordenadas locais da esfera; observadores e câmera permanecem no mundo. Um
`AquariumPose` já faz a transformação rígida e sua inversa. A pose física é
inaudível por padrão e só altera a percepção externa quando o vínculo acústico
é ativado explicitamente; câmera e animação de interface nunca entram no core.
Isso permite movimento do recipiente sem romper contenção ou deslocar a Lupa.
Inércia limitada do líquido e Doppler continuam como pesquisa posterior. Ver
`AQUARIUM_SPACE_DESIGN.md`.

Quatro consequências dessa coluna vertebral, que mudam a natureza do instrumento e não só sua complexidade:

- **Desejo sem controle direto** (`Desire`/`AccessPort`): o sistema aceita intenção vinda de fora sem virar determinístico — reconfigura como as próprias funções se comportam, sem substituir a geração por automação fixa. Nem totalmente autônomo, nem totalmente manual.
- **Tempo narrativo, não só textura** (`Environment`: estações, dia/noite, acasalamento; predador/presa): o instrumento tem manhã e noite, época de caça, época de acasalamento — forma de longo prazo nascendo da lógica ecológica, sem precisar de um sequenciador dizendo "agora entra a seção B".
- **Consequência** (`Ecosystem::Integrity`): sustentar tensão tem custo — a máquina de estado admite uma ruptura irreversível, rara e ligada a causas identificáveis. A consequência sonora dessa ruptura ainda será projetada e validada; não existe efeito anunciado prematuramente.
- **Foco tem custo** (`Ecosystem::Lens` — a Lupa): uma lente de aumento acoplada ao próprio vidro do aquário, não flutuando livre dentro d'água nem fixa fora como os 4 observadores — desliza livremente sobre a superfície esférica, em qualquer direção, sem trajetória obrigatória. Serve pra UM observador de cada vez aproximar o olhar e ouvir com detalhe macro os seres que se aproximam do vidro, bem no ponto onde ela está encostada. Mas é uma troca, não um bônus: enquanto um observador está com o olho na lupa, tudo que não é o ponto focado deixa de ser visão panorâmica normal e vira desfoque — sem nitidez, só zonas de cor em movimento. Reforça a coluna vertebral do instrumento (nenhuma entidade tem visão total) com sua própria versão pro ponto de vista humano: ver mais de perto em algum lugar sempre significa ver pior em outro. Ver o contrato em `ECOSYSTEM_DESIGN.md`.

**Questão em aberto, parcialmente estruturada:** a quantidade de mecanismos (cardume, personalidade, estratégia de sobrevivência, desejo, presença, integridade, visão subaquática, estações) é grande, e tudo converge pra 2 canais de saída. A fundação nova já distingue `Foreground` de `Support` e aplica orçamento de ganho, mas isso ainda é uma hipótese de mixagem, não prova de legibilidade sonora. Nem toda dimensão precisa ser audível como algo separado; algumas devem ficar como estrutura interna. Provas A/B e escuta humana devem decidir o que permanece no primeiro plano antes que novos motores aumentem a densidade.

### Padrão de excelência do instrumento

O Aquorbium é um instrumento de performance de excelência, não uma coleção de
efeitos. Cada módulo novo deve justificar uma contribuição musical própria ao
Bioma e passar por um ciclo de pesquisa, engenharia e escuta antes de entrar no
motor principal.

1. **Pesquisa e hipótese:** registrar o problema musical, o princípio
   matemático/DSP, referências verificadas e a diferença concreta em relação ao
   que já existe. Uma referência inspira a pergunta; nunca define uma cópia.
2. **Protótipo isolado:** construir a menor implementação que exponha o
   comportamento. Comparar com uma base simples quando houver alegação de
   melhoria — por exemplo, espectro com e sem anti-aliasing, estabilidade de um
   feedback, ou resposta de um filtro.
3. **Critérios técnicos:** toda rota de áudio deve ser finita e limitada,
   determinística quando a seed assim exigir, invariante ao tamanho do bloco e
   sem alocação, bloqueio, espera ou acesso a dispositivo dentro do callback.
   Medir CPU em taxas e blocos representativos antes de ampliar polifonia ou
   complexidade.
4. **Critérios de instrumento:** testar sob gesto real de performance: resposta
   previsível do controle, transições sem clique, faixa útil ampla, leitura
   sonora das relações ecológicas e espaço para surpresa sem perda de domínio.
5. **Integração e memória:** só então o módulo ganha API estável, testes,
   documentação da origem e dos limites, e uma ligação explícita aos demais
   módulos do Bioma. Se puder servir a outro instrumento RASGO, sua promoção ao
   comum segue o portão G4 da metodologia.

O objetivo não é maximizar quantidade de recursos. É fazer com que matemática,
design de onda, mecânica de interação e limites reais do computador trabalhem
juntos para produzir um instrumento profundo, responsivo e sustentável em
palco.

### Laboratório por módulo

Todo módulo pode ter uma versão **experimental**. Ela é o lugar deliberado da
intuição: o equivalente digital de trocar um resistor por um diodo, realimentar
um circuito onde não havia realimentação ou aceitar a gambiarra que produz um
novo gesto sonoro. O resultado incerto pode ser musicalmente mais valioso do
que uma solução prevista; experimentar também é compor improvisando.

Essa liberdade não é uma segunda implementação descartável nem uma desculpa
para instabilidade silenciosa. O laboratório de um módulo deve declarar:

- qual parâmetro, topologia, lei matemática ou rota de sinal está sendo
  deslocada;
- quais conexões do Bioma pode receber e devolver, inclusive rotas de feedback;
- limites de segurança não negociáveis: ganho, `NaN`/`Inf`, DC, CPU, memória e
  transições sem clique;
- como congelar seed/estado ou gravar um gesto para que uma descoberta possa ser
  ouvida, repetida e comparada;
- o critério de destino: integrar ao módulo estável, manter como modo
  experimental explícito ou arquivar como estudo.

O caminho estável preserva confiança em palco. O caminho experimental preserva
o risco, a surpresa e a descoberta. Ambos compartilham o mesmo vocabulário de
entradas, saídas e adaptação ao Bioma, para que uma invenção possa amadurecer
sem perder a conversa com o resto do instrumento.

### Matriz de relações sonoras

Os módulos do Aquorbium não formam uma cadeia fixa. Cada um deve poder atuar
como fonte, transformador, observador ou modulador de outro: áudio pode virar
controle por análise de envelope/espectro; controle pode deformar fase,
frequência, tempo, filtro, espaço ou densidade; uma forma de onda pode modular
outra por FM, phase modulation, ring modulation, waveshaping, wavefold ou
leitura de wavetable. A conexão só é interessante quando muda a relação entre
organismos, e não apenas quando acrescenta mais um efeito.

A matriz deve distinguir três tipos de rota:

- **som:** sinal audível entre voz, transformador, espaço e master;
- **modulação:** sinal contínuo ou evento que desloca parâmetros de outro
  módulo, incluindo modulação de forma de onda;
- **observação:** análise limitada e suavizada de energia, espectro, eventos ou
  estado que retorna ao Cérebro e ao Ecossistema sem carregar áudio bruto.

Rotas podem ser permanentes, escolhidas pelo Bioma ou expostas como gesto de
performance. Feedback é permitido quando possuir ganho, faixa, atraso/slew e
critério de saturação explicitamente definidos; a segurança técnica não pode
ser modulada nem desligada pelo modo experimental. Assim, a interconexão vira
uma prática de composição e não uma fonte opaca de acidentes.

### Organismos, instâncias e módulos compostos

Um organismo pode ser uma instância simples ou um módulo composto — por
exemplo, oscilador + modulação + waveshape + filtro + espaço local. Instâncias
da mesma espécie preservam estado próprio, mas podem compartilhar seed, fase,
clock, energia ou memória para formar um cardume. A interface deverá permitir
ver se a relação é uníssono, desvio, série, paralelo, cascata ou feedback
limitado.

O módulo composto oferece uma leitura macro para performance, mas não esconde
seu diálogo: entradas, saídas e pontos internos relevantes podem ser expostos
para patch. Assim o Aquorb mantém organismos reconhecíveis sem perder a chance
de quebrar, misturar ou recompor suas relações durante a experimentação.

## 2. Filosofia e licenciamento

- **Licença do projeto:** AGPLv3 (decisão fechada — ver nota abaixo sobre JUCE).
- **Princípio de apropriação:** referências de artistas/hardware servem como ponto de partida conceitual e técnico, não como especificação a ser copiada. Toda referência é tratada como "verificada" ou "não verificada" (ver seção 6) antes de virar decisão de design.
- **Só tecnologia livre:** bibliotecas, frameworks e códigos de referência usados no projeto devem ter licença livre (GPL, AGPL, LGPL, ISC, MIT, BSD).

### Nota sobre a licença do JUCE

O JUCE é dual-licenciado: os módulos "core" (`juce_audio_basics`, `juce_audio_devices`, `juce_core`, `juce_events`) são licença **ISC** (permissiva). Os módulos maiores (GUI, DSP, processadores de plugin) são licenciados sob **AGPLv3** ou licença comercial. Isso é importante porque **AGPLv3 não é GPLv3** — o AGPL tem uma cláusula adicional de uso em rede que a GPL comum não tem. Na prática, pra um plugin/instrumento desktop isso quase não muda nada, mas o nome correto da licença do JUCE gratuito é AGPLv3.

**Decisão definitiva: JUCE.** O framework fica confirmado — o Aquorbium é construído em JUCE/C++ (não migra pra DPF; a alternativa continua listada na tabela abaixo só como referência de arquitetura, não como opção em aberto). Consequência direta: a licença do projeto casa com **AGPLv3** (a licença gratuita real do JUCE), não GPLv3 puro — decisão que estava pendente na seção 11 e agora está fechada.

## 3. Stack tecnológico livre (candidatos)

| Framework | Licença | Observação |
|---|---|---|
| **JUCE** | ISC (core) / AGPLv3 ou comercial (resto) | Framework padrão da indústria, mais documentação e exemplos, é o que já está sendo usado no projeto |
| **DPF (DISTRHO Plugin Framework)** | ISC (100%) | Mais "cru" que o JUCE, mas sem ambiguidade de licença; suporta LADSPA, LV2, VST2/3, CLAP |

Codebases GPL de referência pra estudar arquitetura (não pra copiar, pra aprender):

- **Surge XT** — GPLv3, synth híbrido (subtrativo, FM, wavetable, granular) — vale estudar a estrutura de módulos e o motor de wavetable.
- **Vital** — GPLv3, synth wavetable com "spectral warping" — referência forte pro módulo de síntese tonal.
- **VCV Rack** — GPLv3+, plataforma modular virtual — referência de arquitetura para o pensamento "ecossistema de módulos" que é o cerne do conceito Bioma.
- **Westlicht PER|FORMER** — hardware e firmware 100% open source (ver seção 9) — único item de todo o levantamento ModularGrid que pode ser lido como código de estudo sem qualquer ambiguidade de licença.
- **Mutable Instruments** (Blades, Tides — ver seção 9) — DSP historicamente publicado em MIT no GitHub de Émilie Gillet — coerente com o princípio de só tecnologia livre.

## 4. Arquitetura geral (fluxo de dados)

Não é um pipeline linear. É um ciclo de feedback entre 5 módulos:

```
[ MÓDULO 1: CÉREBRO ] ──(dados/CV)──► [ MÓDULO 2: GEOMETRIA/SEQUENCIADOR ]
        ▲                                          │
        │ (feedback de modulação,                  │ (notas/gatilhos)
        │  análise do próprio áudio de saída)       ▼
[ MÓDULO 5: MASTER ] ◄──(áudio)── [ MÓDULO 4: ESPAÇO ] ◄──(áudio)── [ MÓDULO 3: SÍNTESE/GLITCH ]
```

Essa visão de 5 módulos é a arquitetura de processamento de áudio. Em paralelo, existe o **padrão Aquário** (`Ecosystem.h`) — um modelo de como os módulos se relacionam entre si e são mixados, com peixes/organismos, observadores, cardumes, predador/presa, ambiente e desejo. Os dois níveis convivem: o fluxo de 5 módulos é o "hardware" de DSP; o Aquário é a "física" que rege como os módulos se comportam e se ouvem.

## 5. Módulos técnicos

Cada módulo lista: conceito, técnica(s) candidata(s) (todas mantidas, nenhuma descartada), e status.

### Módulo 1 — O Cérebro (geração de incerteza e estado)

Gera o "combustível" matemático do sistema: valores flutuantes e decisões estruturais.

**Estado atual:** `BiomaBrain`, o Ecossistema e `AutonomousEnergy` já formam
uma primeira cadeia ativa: decisão a 100 Hz, ciclos Relabi e homeostase a partir
da energia de saída. Desde 28/09, os protótipos antes isolados deste módulo
também podem ser ouvidos no banco experimental `08 MÓDULOS`; isso não os
transforma ainda em substitutos especializados do Cérebro ou do Ecossistema.
`SlewRateRandomWalk` e `LorenzCV`, por exemplo, alimentam a família `LORENZ +
DRIFT`, mas sua variação ainda não foi aprovada no contexto musical.
`BiomaMarkov` materializa uma cadeia contextual de oito estados. Desde
26–27/09, uma instância compartilhada atua como memória formal da Geometria:
avança nos eventos coletivos Relabi e desloca seção/contorno sem gerar áudio,
ganho ou presença diretamente. Possui quatro macros, cenas e bypass bit a bit.
`ControlEventExtractor` transforma cruzamentos/energia em eventos e participa
da família experimental `CLOCK + EVENTS`.
`FractalCore` e `VectorGenerator` acrescentam em 24/09 dois protótipos com a
mesma implementação em taxa de controle e áudio. Eles possuem consequência
audível em `FRACTAL + VECTOR`, mas não integração definitiva ao Cérebro ou aos
cardumes.
`ShiftRegisterMemory`, `ProbabilityDistribution` e `EvolutionLoopMachine`
separam memória binária, forma do acaso e memória de personalidade. Estão
audíveis nas famílias `SHIFT + PROB` e `EVOLVE + STAGES`; calma, inteligência e
personalidade ainda não receberam esses mapeamentos definitivos.
`ConditionalStageMachine` acrescenta oito estágios e saltos por perigo, noite
ou trinca; `TripleSlowDrift` combina três derivas de segundos a dezenas de
minutos; `IntegrityCrossfade` oferece transição de potência constante. Os três
passaram na validação técnica e agora têm uma rota experimental, ainda sem
ligação ecológica definitiva.
`HarmonicPhaseBank` e `FluidMotion` acrescentam uma fase mestra racional,
quatro interpretações de movimento e autopatch limitado. A família `FLUID +
PHASE` os torna audíveis, mas eles não substituem o movimento individual nem
`sharedPhase` atuais.
`MotionWaveBank` centraliza oito formas de movimento e uma escolha
determinística por identidade. `MotionGestureRecorder` mantém até 2.048 pontos
XY, playback interpolado e mutação opcional por volta. O primeiro já é
reutilizado pelo `FluidMotion`; o segundo é reproduzido por `GESTURE WAVES`.
Desde 02/10, uma segunda instância grava a 200 Hz o movimento XY real da Lupa
durante a posse e o reproduz depois da soltura enquanto o laboratório está
ativo. Isso não substitui o movimento ativo do `Ecosystem` nem automatiza a
posse.
Em 28/09, uma nova coorte entrou diretamente no banco audível: `ReeseVoice`,
`DualPeakFilter`, `ResonantFilterSequencer`, `FrequencyIsolator`,
`InteractionProcessor`, `PowerStarve`, `QuadratureStereoRotator`,
`PolygonalOscillator`, `BoundedRoutingMatrix` e `SharedParameterPool`. A
arquitetura não ganhou dez módulos visuais: os mecanismos reforçam oito das
quinze famílias existentes e herdam `LAB MIX`, estado e limites do banco. São
implementações técnicas reais, mas seus vínculos especializados com BiomaBass,
Lupa, `AquariumTransform`, `AccessPort`, forma do organismo e encontros
predador/presa continuam pendentes. Ver `EMERGENT_MODULE_COHORT_DESIGN.md`.
Ainda em 28/09, a coorte avançada acrescentou `BiomaBassVoice`,
`SchoolWaveVoice`, `HarmonicRippleVoice`, `RecentMixWavetable`,
`FrequencyShifter`, `CrossFeedbackStateMachine`, `SpotlightMixer`,
`HungerProcessor`, `BoundedSelfFeedback` e `WaterInertia`. Eles também entram
nas famílias existentes, sem ampliar a superfície ou o estado, e tornam
audíveis hipóteses antes apenas descritas. A arquitetura definitiva ainda não
declara essas rotas experimentais como novos organismos: sub/quantizador,
sequenciador realimentado pela saída, parâmetros por organismo, `SchoolInfo` e
`Ecosystem` aguardam integração especializada e escuta. Ver
`ADVANCED_VOICE_ECOLOGY_COHORT_DESIGN.md`.
Em 29/09, a coorte de geração e interação acrescentou
`FixedLSystemGenerator`, `CellularAutomatonSequencer`,
`SpectralTransferVocoder`, `EnvelopeContagionBus`, `NoInputFeedbackBus`,
`ReactiveThresholdStrategy`, `LockInFocusExtractor`, `BehaviourSceneBank`,
`ReconfigurableResourcePool` e `LightweightBinauralSpatializer`. Eles tornam
audíveis dez hipóteses sem ampliar a superfície. A integração é deliberadamente
experimental: L-system/autômato ainda não são modos da Geometria; lock-in não
seleciona um organismo real; binaural leve não é HRTF medida; no-input não é o
Master; cenas não pertencem ainda ao `Ecosystem`. Ver
`GENERATIVE_INTERACTION_COHORT_DESIGN.md`.
Em 02/10, a coorte de interação aberta acrescentou `DoubleScrollAttractor`,
`SemanticAccessRouter`, `OpenRuleProgram` e `ContactCrackleExciter` às famílias
existentes. Os quatro possuem core, teste, rota de áudio e telemetria, sem novo
controle visual. Permanecem experimentais: o double-scroll ainda precisa de
escuta A/B contra Lorenz; o roteador não é ainda o `AccessPort` editável por
organismo; o exciter não recebe gesto GUI de dois pontos; e o programa de
regras não possui editor textual/APVTS. Ver
`OPEN_INTERACTION_COHORT_DESIGN.md`.
Em seguida, `CompositionalVariationDirector` passou a ordenar os eventos
coletivos em segmentos A/A′/B/suspensão e coordenar leitura da memória,
densidade, harmonia, balanço das famílias e reflexões. `STRUCTURE MIX` dosa a
política; os ganhos familiares continuam subordinados a `LAB MIX` e às macros
existentes. Acentos geométricos modulam de forma limitada a energia das vozes,
dosados por `STRUCTURE MIX`, e deixam de ser apenas telemetria. A influência
da memória de passos sobre a melodia também tem pesos próprios por segmento.
Coesão e calma do Ecossistema modulam sutilmente essa retenção melódica; a
mistura entre leitura direta e retrógrada em B é gradual, sem limiar de 0,5.
O ritmo também interpola probabilisticamente o padrão euclidiano com rotações
de um passo específicas por segmento, preservando gates binários em bypass.
O alvo estático do core compilou; app standalone, regressões, renders e escuta
continuam pendentes.
Como o evento Relabi não é relógio métrico, a forma não promete compassos nem
sincronismo com DAW. Ver `COMPOSITIONAL_VARIATION_DESIGN.md`.
Na mesma data, `AirWaterMedium` entrou em `FLUID + PHASE` usando altura,
turbidez e temperatura reais do contexto. É uma primeira diferença acústica
entre ar e água no laboratório, não ainda um meio separado por organismo no
Ecossistema. `CORE_REFINEMENT_COHORT_DESIGN.md` registra também o wavefolder
4× e o feedback limitado do Master.
`SchoolSpectralBank` acrescenta oito filtros ressonantes, centro/dispersão e
perfil fixo por cardume. `EcologicalNoiseBed` relaciona estação/hora à textura,
turbidez/temperatura à cor e trinca à resolução temporal. A primeira integração
processa a soma estéreo antes do Espaço, recebe ciclos, água e integridade do
Ecossistema e expõe seis macros no cartão Ambiente. O perfil ainda é comum ao
mix; diferenciação por `SchoolInfo`/espécie e aprovação por escuta permanecem
abertas.
`PhilosophyVectorMixer`, `PhilosophySequencer`, `PhilosophyMorph` e
`PhilosophyMatrixMixer` estabelecem blend XY, percurso, transição e duas saídas
estéreo independentes para quatro fontes. A família `INTEGRITY + MEMORY` os
torna audíveis; as filosofias concretas ainda não foram atribuídas e nenhum
peso do `Ecosystem` foi substituído.
L-systems e autômatos continuam candidatos, não funções anunciadas.

- **Slew-Rate Random Walk**: ruído aleatório suavizado por filtro passa-baixa de 1º polo (<2 Hz) — emula geradores de incerteza tipo Buchla Source of Uncertainty. `[core]`
- **Atrator de Lorenz**: sistema de equações diferenciais (σ, ρ, β, dT) gerando CV caótica mas contínua/orgânica. `[core integrado]`
- **Atrator double-scroll**: sistema tridimensional determinístico por trechos,
  com regiões biestáveis e pares complementares. `[core integrado no banco;
  comparação musical A/B pendente]`
- **Cadeias de Markov**: memória formal contextual de oito regiões, avançada
  somente por eventos coletivos e aplicada à seção geométrica. `[core integrado]`
- **L-systems**: gramáticas de reescrita recursiva de símbolos — pitch/duração/timbre como símbolos com regras de mutação. `[core integrado experimental]`
- **Autômatos celulares**: grade binária com regras de transição (tipo Jogo da Vida), mais indicado pra síntese sonora bruta que pra decisões estruturais. `[core integrado experimental]`
- **Cross-Feedback State Machine**: detector de envelope (`juce::dsp::BallisticsFilter`) na saída final analisa o próprio áudio gerado e realimenta as regras do sequenciador. `[core — fecha o loop do conceito Bioma]`

### Módulo 2 — Geometria / Sequenciador

Transforma os dados do Módulo 1 em ritmo e notas.

**Estado atual:** a camada autônoma possui uma rede intervalar e seções lentas
mínimas para dar coerência à primeira prova sem MIDI. O
`GeometricSequencer` já contém distribuição circular, cinco lanes e
quantizador reutilizável. `AddressableStepMemory` e `HarmonicProgression`
estão integrados à fronteira de eventos do `AutonomousEnergy`, com padrão
determinístico, fase dia/estação e quatro macros na superfície rolável da
Geometria. `AccumulatingTransposer` conserva memória de
transposição com direção sazonal e `ThresholdStepSelector` oferece um terceiro
modo de navegação por cruzamento de regiões. Edição detalhada, `AccessPort` e
aprovação musical continuam pendentes. `StepPatternBank` acrescenta
snapshots/encadeamento em memória, enquanto `RandomToLoop` permite que uma
fonte de controle passe gradualmente de renovação contínua a repetição fixa.
O cartão Geometria expõe ainda `FORM MIX`, `CONTEXT`, `INERTIA` e `MUTATION`
para a memória Markov; esse eixo de forma complementa, sem substituir, os
cinco controles lineares de intenção performática geométrica.
`BranchingStepRouter` organiza decisões em categoria e variante;
`NotePermutationMatrix` mantém ordens alternativas sobre um pool comum de
notas, sem duplicar o material harmônico. `PatternRotationMutator` acrescenta
mutação estrutural agendada; `StepMemoryLayout` reorganiza os mesmos 32
endereços entre paralelo e serial; `DriftCurveShaper` separa a continuidade da
deriva de sua família matemática.

`FractionalLogicClock` acrescenta dois relógios de razão
racional e combinação AND/OR/XOR. `AddressableStepMemory` acrescenta 32 passos
com quatro valores e leituras sequencial/direta; `HarmonicProgression`
acrescenta 32 seções ponderadas e acordes de três ou quatro vozes. Esses dois
últimos já participam da Geometria principal; os demais componentes possuem
agora uma rota audível no banco experimental, mas continuam fora do
sequenciador principal, do `Environment` e do transporte de host. `AudioPlayHead`,
edição/persistência dos passos e resets externos
continuam pendentes.

- **Ritmo Euclidiano (algoritmo de Bjorklund)**: distribuição uniforme de N pulsos em K passos; com mutação por rotação de bits (`std::rotate`) a cada X compassos. `[core]`
- **Interval Shift Quantizer**: em vez de notas absolutas aleatórias, soma offsets relativos a uma escala fixa — garante coerência tonal mesmo em caos rápido. `[core]`
- **Matrizes de permutação**: sequência fixa de N notas cuja ordem/oitava/velocidade muda matematicamente a cada repetição (inspirado em lógica de sequenciadores combinatórios). `[candidato]`
- **Clock fracionário**: divisões de compasso assimétricas (7/4, 11/8, 5/4) calculadas em tempo real via `AudioPlayHead`, não só binárias (1/4, 1/8). `[candidato]`
- **Microtonalidade / afinação não-temperada**: suporte a tabelas Scala (.scl) para just intonation. `[candidato — fica pra depois]`

### Módulo 3 — Síntese e Micro-Glitch

Onde os dados viram áudio.

**Estado atual:** o oscilador morph com PolyBLEP, o pulsar,
`BiomaModalResonator` e `BiomaGranularStriker` estão integrados. O ressonador soma
quatro modos amortecidos; o Striker agenda nuvens de partículas sintéticas em
pool fixo. Ambos recebem impactos ecológicos e aceitam intenção performática
híbrida. `BiomaPhysicalString` já ocupa o quinto organismo; sua pesquisa
isolada permanece disponível para comparação. `SynthFMVoice` tem tabela
senoidal interpolada, thru-zero FM e sine-shaping e agora soa em `FM +
HARMONICS`; não é ainda uma espécie do Ecossistema. `BiomaGranularCapture`
possui histórico fixo, grãos de amostra e detecção de onset e agora recebe o
mix na família `GRANULAR CAPTURE`. Wavetable de amostra continua pesquisa.
Desde 02/10, o sine-folder de `SynthFMVoice` e `BiomaBassVoice` usa avaliação
4× interpolada e decimação curta; um teste dirigido confirmou menor energia na
banda alta, sem alegar antialias universal.
`SamplePercussionVoice` acrescenta um fallback curto de quatro slots com
pitch e decay; `HarmonicMorphOscillator` interpola coeficientes antes da
síntese aditiva; `StochasticPulseMask` mantém uma decisão probabilística por
pulsaret. Eles soam nas famílias `PERC + PULSE` e `FM + HARMONICS`, mas ainda
não têm espécie ou função ecológica definitiva. `FractalCore` e
`VectorGenerator` também operam em áudio no banco; sua função composicional
permanece em avaliação.

- **Wavetable + Thru-Zero FM**: dois osciladores (`juce::dsp::Oscillator`), modulador empurra a fase do portador até frequência negativa (thru-zero) em vez de só dobrar — preserva profundidade de modulação mesmo em frequências baixas. `[core]`
- **Wavefolder**: sine-shaping (`sin(gain*in)`, método Serge) em vez da dobra piecewise ingênua, com avaliação 4× interpolada e decimação curta. Meia-banda, ADAA ou polyBLAMP continuam candidatas se medições mais amplas exigirem. `[core integrado]`
- **PolyBLEP/PolyBLAMP**: anti-aliasing matemático na descontinuidade da onda (dente-de-serra, quadrada, triângulo) sem oversampling pesado — necessário se osciladores clássicos forem gerados "na unha" e não por wavetable. `[core, se houver osciladores geométricos]`
- **Karplus-Strong / Digital Waveguides**: modelagem física de corda dedilhada (delay line curto + filtro passa-baixa realimentado) — abre a porta pra timbres de percussão/corda "física" de verdade. `[candidato — expande o motor de síntese além do digital puro]`
- **Ressonador modal:** quatro osciladores complexos amortecidos com excitação
  de ruído curta, proporções harmônicas↔membrana, teto de energia e controle
  híbrido aplicado em fronteira de impacto. `[core — primeiro organismo físico]`
- **Multi-Particle Noise Striker**: rajadas de ruído branco filtrado (grãos de 2–40 ms) em filtro passa-banda ressonante (Q 20–100) — estalos metálicos/glitch. `[core]`
- **Síntese granular**: motor de grãos de amostra (referência de implementação: projeto GRNLR, open-source em JUCE). `[candidato — módulo de textura/glitch mais denso]`
- **Voice stealing / polifonia**: `juce::Synthesiser` + `SynthesiserVoice`, com `findVoiceToSteal` sobrescrito pra um comportamento próprio. `[infraestrutura obrigatória]`

### Módulo 3.4 — BiomaBass (motor de graves original)

Proposta de síntese original a partir de comportamentos de módulos de mercado — não clona nenhum, combina princípios de vários e fecha um loop de feedback com o Módulo 1 que nenhum hardware citado tem.

**Referências de mercado estudadas (mantidas, nenhuma descartada):**

- **Endorphin.es Godspeed** — núcleo "Strong Zero VCO" com saída de sub-oscilador dedicada.
- **Cwejman VCO-2RM** — modo de sync que funciona bem pra grave (oscillator sync clássico).
- **Doepfer A-111-4** — quad sub-oscillator dedicado, referência de sub uma oitava abaixo.
- **Oakley Diode Superladder** — filtro descrito como "o mais líquido e suculento", também grita/guincha em ressonância alta.
- **Doepfer A-102 (Diode)** — outro filtro a diodo citado entre os mais "bassy".
- **Doepfer A-120** — ladder filter econômico, "um toque de Moog" sem o preço do Moog.
- **Studio Electronics 5089** — filtro estilo Moog com controle de ressonância via CV.
- **Joranalogue Filter 8** — 4 polos low-pass com 8 saídas; em ressonância alta, auto-oscila **sem perder o corpo do grave** (a maioria dos filtros perde grave nesse ponto).
- **Moog Mother-32** — referência do "calor Moog" clássico: cheio, redondo no grave, doce no topo.
- **Moog Mavis** — dois osciladores + wavefolder a diodo, destrava harmônicos que a voz ladder normal não entrega.
- **Técnica Reese bass** — múltiplos osciladores dessintonizados entre si, grave "xaroposo"/denso (clássico de drum'n'bass).
- **Técnica FM bass** — sidebands do modulador geram harmônicos complexos que furam a mixagem sem precisar de grave excessivamente gordo.
- **Escolha de forma de onda** — senoidal pra grave puro/limpo, dente-de-serra quando se quer agressividade.

**O que é original nessa combinação:**

- Wavefolder a diodo (princípio Mavis) posicionado **antes** do filtro, não depois — empurra harmônico pro filtro trabalhar em cima, em vez de só colorir depois.
- Filtro com topologia que preserva a fundamental em self-oscillation (princípio Joranalogue Filter 8), implementado como design próprio — não precisa ser diode ladder.
- Detune do "Reese" **não é fixo**: é modulado em tempo real pelo Slew-Rate Random Walk do Módulo 1 — o chorus do baixo respira organicamente em vez de ser uma configuração estática de patch.
- FM usa o motor **thru-zero** já desenhado no Módulo 3.1 — a maioria dos bass FM comerciais (Serum e afins) não usa thru-zero.
- Sub-oscilador não é fixo em -12 semitons: é derivado do Interval Shift Quantizer do Módulo 2, escolhendo harmonicamente sua relação com a nota principal.
- **Loop de feedback exclusivo do conceito Bioma:** um `juce::dsp::BallisticsFilter` escuta a energia do grave gerado pelo próprio BiomaBass e, ao passar de um limiar, reduz automaticamente o detune/wavefold — o baixo "recua" sozinho antes de saturar a mixagem. Nenhum módulo de mercado citado tem esse comportamento.

### Módulo 4 — Espaço / Textura

- **Multi-Tap Delay Line ativo**: memória estéreo portátil própria, oito
  ponteiros Lagrange cúbicos, cada um com tempo, ganho, pan e filtro; uma única
  instância compartilhada pós-soma responde ao Ecossistema. `[core]`
- **Modulação do delay pelo Módulo 1**: os tempos de delay são sutilmente modulados pelo Slew-Rate Random Walk — fecha outro loop do conceito Bioma. `[core]`
- **Cauda sintética**: `AquariumTail` propõe uma rede de quatro atrasos com
  difusão Hadamard, amortecimento e deformação `nightmare`. É implementação
  própria, wet-only e não simula sala. Uma instância compartilhada está ativa
  depois de `MultiTapSpace`, com quatro macros, cenas, bypass transparente,
  render e validação técnica. Escuta e integração ambiental continuam
  pendentes; `size` não é exposto até ser descontínuo-seguro. `[core]`

### Módulo 5 — Master / Saída

Único módulo que não estava no rascunho original — fecha a lacuna de "potência sonora e volume de mercado".

- **Crossover multibanda (Linkwitz-Riley)**: separa grave/médio/agudo pra processar dinâmica por banda sem lavar o transiente inteiro junto. `[core]`
- **Compressão por banda**: attack/release/ratio independentes por banda. `[core]`
- **Limiter com lookahead**: reduz ganho antes do pico chegar (não só "atrasa" o sinal) — referência de implementação: projeto SimpleCompressor (Daniel Rudrich), documentado e open-source. `[core]`
- **Medidor de loudness (ITU-R BS.1770 / LUFS)**: pipeline K-weighting → elevar ao quadrado → gate absoluto (-70 LUFS) → gate relativo (-10 LU) → integrar. Update rate mínimo 10 Hz (short-term) / 1 Hz (integrated). Dá o número real pra saber se bateu o alvo de streaming/álbum. `[core — é o critério objetivo de "pronto"]`

#### Decisão: master criativo e saída técnica são camadas distintas

O Módulo 5 acima é o **master criativo do Aquorbium**: crossover, compressão,
largura, cola e feedback podem reagir ao Bioma e participar do caráter do
instrumento. Depois dele deve existir uma **saída técnica comum RASGO**, definida
em [`SAIDA_AUDIO_COMUM.md`](../../RASGO_DOCUMENTATION/architecture/SAIDA_AUDIO_COMUM.md),
responsável por teto true peak, proteção DC, output trim/mute, medição e entrega
ao dispositivo.

Primeira materialização local em 2026-09-27: `OutputStage` já encerra a rota
de `BiomaEngine` com saneamento, bloqueio DC, trim/mute suavizados, limiter
estéreo vinculado e lookahead de 5 ms. O cartão Master expõe trim, ceiling e
release. O detector intersample 4× é ainda uma estimativa; LUFS, true peak
conforme BS.1770 e oversampling validado permanecem abertos.
Detalhes e evidência: `OUTPUT_STAGE_DESIGN.md`.

Segunda materialização em 2026-09-30: `MasterChain` entrou antes do
`OutputStage`, com crossover Linkwitz–Riley de quarta ordem em três bandas,
compressão vinculada por banda, tilt, largura mid/side preservando o grave e
cola óptica lenta. `CREATIVE MASTER=0` é bypass exato; o `crackStress`
estreita apenas médio/agudo. EQ antes/depois existe como possibilidade interna,
mas a escolha ainda não está exposta. Desde 02/10, a redução real das bandas e
da cola realimenta o `BiomaBrain` a 200 Hz por um adaptador com ataque, release
e teto de 0,32; influência zero conserva o alvo em zero. Evidência:
`CREATIVE_MASTER_COHORT_DESIGN.md` e `CORE_REFINEMENT_COHORT_DESIGN.md`.

Consequências:

- multibanda, compressão ótica e largura sob CV devem ter bypass neutro e não
  são obrigatórias nos outros instrumentos;
- o limiter de segurança fica por último e não pode ser modulado pelo Cérebro;
- auto-oscilação/no-input mixing causada por `AquariumIntegrity::shattered`
  acontece antes dessa proteção final;
- a energia que realimenta o Cérebro deve ser publicada em taps separados
  pré-master, pré-safety e como redução de ganho; observar apenas a saída já
  limitada apagaria a dinâmica do ecossistema;
- LUFS é medição e critério de renderização, não normalização automática da
  performance ao vivo;
- o app deve oferecer perfis de latência/qualidade documentados; lookahead e
  oversampling nunca podem ter latência implícita;
- a saída estéreo final e stems de BiomaBass, glitches e espaço devem poder ser
  gravados em ponto flutuante para mixagem posterior.

#### Orçamento de ganho do Ecossistema

O Ecossistema canônico entrega perspectiva L/R por organismo, redução explícita
de sustentação e compensação de densidade limitada a uma soma de 0,92 por canal
antes do master. `AquariumSpatializer` transforma essa perspectiva em pan de
potência constante, profundidade espectral e largura suavizadas por amostra. A
compensação preserva diferenças de presença; não normaliza todo estado para o
mesmo volume.

O caminho de renderização usa memória fixa e o teste dedicado registra zero
alocações durante 500 blocos. Isso resolve a falha imediata do protótipo legado,
mas não substitui a futura medição de CPU do instrumento completo nem o teste de
pior caso com os motores DSP definitivos.

## 6. Estrutura de classes sugerida

```
src/
├── core/
│   ├── AudioEngine.h            <- orquestração DSP sem dependência da GUI
│   ├── BrainEngine.h            <- Módulo 1 e cross-feedback
│   ├── GeometricSequencer.h     <- Módulo 2
│   ├── OrganismVoiceEngine.h    <- espécie ecológica → corpo DSP real
│   ├── SynthFMVoice.h           <- Módulo 3.1
│   ├── BiomaPhysicalString.h    <- Módulo 3.2, protótipo isolado
│   ├── BiomaGranularStriker.h   <- Módulo 3.3, partículas sintéticas
│   ├── AquariumPose.h           <- transformação rígida local/mundo
│   ├── AquariumSpatializer.h    <- fundação estéreo por organismo
│   ├── MultiTapSpace.h          <- reflexões compartilhadas ativas do Módulo 4
│   ├── MasterChain.h            <- Módulo 5, master criativo
│   ├── OutputStage.h            <- saída técnica comum
│   ├── Ecosystem.h              <- contrato C++ do padrão Aquário
│   └── Ecosystem.cpp            <- física, percepção e mix ecológica
├── app/Main.cpp                 <- primeiro produto: app DSP standalone
├── app/MainComponent.h          <- GUI e dispositivo de áudio
└── plugin/PluginProcessor.h     <- adaptador posterior, sem reger o core
```

## 7. Referências por artista — status de verificação

Tratamento honesto do material original (Meta AI): confirmado contra entrevistas/fontes reais onde possível, mantido como inspiração conceitual onde não.

| Artista | Confirmado | Não confirmado / provável erro |
|---|---|---|
| **Richard Devine** | Intellijel Shapeshifter, Mutable microBraids (x3), Make Noise Wogglebug, Intellijel Rainmaker (criou 41 presets oficiais), Orthogonal Devices ER-301 | Soma Scion, Eventide Misha, ALM Skipmin/Pamela's, WMD Fracture/Chimera; "Intellijel Quad LFO" provavelmente é Modcan Quad LFO |
| **Autechre** | Nord Modular G1/G2, Elektron Machinedrum SPS-1UW, Monomachine SFX-60+, MaxMSP (fase atual) | Alesis Quadraverb não confirmado nesta pesquisa |
| **Caterina Barbieri** | Make Noise, oscilador Verbos, Orthogonal Devices **ER-301** (Sound Computer) | Texto original citou "ER-101 (Indexed Quad Sequencer)" — modelo errado, é ER-301 |
| **Alessandro Cortini** | Buchla Music Easel, filosofia "designers não módulos" (Buchla/Make Noise/Verbos/Harvestman/Livewire) | Tascam Portastudio 414 e EMS Synthi AKS não confirmados nesta pesquisa |
| **Keith Fullerton Whitman** | Setup real documentado: PowerBook + Max/MSP + instrumentos analógicos (violão, oud) | ER-301 como módulo central **não confirmado** — provável confusão com Devine/Barbieri |
| **Venetian Snares** | Synth modular **caseiro** (não rack comercial), Qu-Bit Nebulæ, Waldorf Microwave XT | Descrição do texto original era genérica demais, sem módulos específicos verificáveis |

## 8. Referências técnicas (fontes)

### Nome "Aquorbium" — verificação de conflito

- [KVR Audio — Aqua by Osiris (não é "Aquarium")](https://www.kvraudio.com/product/aqua_by_osiris)
- [KVR Audio — Water by xoxos (não é "Aquarium")](https://www.kvraudio.com/product/water-by-xoxos)
- [Vital preset "Aquarium" por Jazen Sounds — não é um instrumento próprio](https://www.betterbeatsblog.com/post/the-ultimate-list-of-free-vital-presets)
- [SyndtSphere (Klevgrand) — synth de UI esférica, nome diferente, sem conflito](https://klevgrand.com/products/syndtsphere/)
- Busca direta por "Aquorbium" (geral e cruzada com synth/plugin/música): nenhum produto, marca ou termo estabelecido encontrado — resultados foram só ruído fonético (Aequorin, Aquoral, Aquarmium/parque temático)
- "Spherium" (candidato descartado) colide com [Spherum FX Re-Synthesizer (Eplex7 DSP)](https://www.kvraudio.com/product/spherum-fx-resynthesizer-by-eplex7-dsp)
- "Orquarium" (candidato descartado) colide foneticamente com "orquidário"/"Orquidarium", termo real em PT/ES
- "Aquorb" (versão curta) — sem resultados em KVR Audio, ModularGrid ou Muffwiggler; busca geral só retorna um monstro de [My Singing Monsters Fanon Wiki](https://my-singing-monsters-fanon.fandom.com/wiki/Aquorb) (fã-wiki não-comercial) e um Pokémon de fã-arte — nenhum produto real, nenhuma colisão de mercado

### Pesquisa EMW — Electronic Music Works (seção 12)

- [EMW — catálogo Eurorack completo](https://www.electronicmusicworks.com/eurorack.html)
- [WAVE-6 Polyphonic Osc.](https://www.electronicmusicworks.com/eurorack/wave-6-poliphonic-osc.html)
- [POT ACTION RECORDER](https://www.electronicmusicworks.com/eurorack/pot-action-recorder.html)
- [RESONANT FILTER SEQUENCER](https://www.electronicmusicworks.com/eurorack/resonant-filter-sequencer.html)
- [FIXED FILTER BANK](https://www.electronicmusicworks.com/eurorack/fixed-filter-bank.html)
- [NOISE CINEMA](https://www.electronicmusicworks.com/eurorack/noise-cinema.html)
- [MIX SEQUENCER](https://www.electronicmusicworks.com/eurorack/mix-sequencer.html)
- [ENVELOPE FOLLOWER](https://www.electronicmusicworks.com/eurorack/envelope-follower.html)
- [VC WAVETABLE LFO](https://www.electronicmusicworks.com/eurorack/vc-wavetable-lfo-v2.html)
- [MIC. INTERFACE](https://www.electronicmusicworks.com/eurorack/mic-interface.html)
- [RING MODULATOR](https://www.electronicmusicworks.com/eurorack/ring-modulator.html)
- [GLIDE PROCESSOR](https://www.electronicmusicworks.com/eurorack/glide-processor.html)
- [X-Y WAVE REWORK](https://www.electronicmusicworks.com/eurorack/x-y-wave-rework.html) / [BIT TRANSFORMER](https://www.electronicmusicworks.com/eurorack/bit-transformer.html) / [WAVE STEREOLIZER](https://www.electronicmusicworks.com/eurorack/wave-stereolizer.html) — ficha técnica incompleta na própria página do fabricante
- [Soundfreak — Sequential Voltage Source](https://www.soundfreak.io/sequencial-voltage-source) (fabricante distinto da EMW, incluído nesta mesma rodada)
- [Soundfreak — Triple VCO](https://www.soundfreak.io/triplevco)
- [Soundfreak — Dual Filter Oscillator](https://www.soundfreak.io/dual-filter-oscillator)
- [Soundfreak — Envelope Shaper / Noise Generator](https://www.soundfreak.io/envelope-shaper-noise-generator)
- [Soundfreak — Dual Ring Modulator](https://www.soundfreak.io/dual-ring-modulator)
- [Teia Synthesizers — DriftLog Manual (PDF)](https://teiasynthesizers.com/wp-content/uploads/2026/06/DriftLog-Manual.pdf)
- [SchneidersLaden — catálogo Eurorack 3U](https://schneidersladen.de/en/eurorack-modular-3u) (só página 1 percorrida até agora)
- [SetonixSynth — Shaka-8](https://schneidersladen.de/en/setonixsynth-shaka-8)
- [earthlink modular — FLUME](https://schneidersladen.de/en/earthlink-modular-flume)
- [4ms Company — Leveling EQ Amplifier (LEQA)](https://schneidersladen.de/en/4ms-company-leveling-eq-amplifier-leqa)
- [Michigan Synth Works — MSW-820 Euro](https://schneidersladen.de/en/michigan-synth-works-msw-820-euro)
- [Mannequins — Silhouette](https://schneidersladen.de/en/mannequins-silhouette)
- [Neutral Labs — Scrooge](https://schneidersladen.de/en/neutral-labs-scrooge)
- [Schlappi Engineering — Eddy](https://schneidersladen.de/en/schlappi-engineering-eddy-black)
- [Make Noise — GTE](https://schneidersladen.de/en/make-noise-gte) / [Toppobrillo — Matrixplexer](https://schneidersladen.de/en/toppobrillo-matrixplexer-black) / [WMD — Channel Surfer](https://schneidersladen.de/en/wmd-wmd-channel-surfer-chnl-srfr) (reforços breves, sem ficha completa)
- [SchneidersLaden — catálogo Eurorack 3U, páginas 6-15](https://schneidersladen.de/en/eurorack-modular-3u?p=6) (páginas 1 a 15 percorridas até agora, de 111 totais)
- [ADDAC — 708 Michel Waisvisz's Cracklebox](https://schneidersladen.de/en/addac-systems-708-michel-waiswisz-s-cracklebox)
- [ADDAC — 405 VC Relabi Generator](https://schneidersladen.de/en/addac-systems-405-vc-relabi-generator) ([manual](https://www.addacsystem.com/contents/productdownload/ADDAC405_RelabiGenerator_A_2.pdf))
- [Hieroglyphic — Opalina](https://schneidersladen.de/en/hieroglyphic-opalina-silver)
- [Rides In The Storm — BOC MK2](https://schneidersladen.de/en/rides-in-the-storm-rides-in-the-storm-boc-mkii)
- [apf.audio — Tiliqua](https://schneidersladen.de/en/apf.audio-tiliqua) / [ADDAC — 300 Power Starvation](https://schneidersladen.de/en/addac-systems-300-rev.02-power-starvation) / [Blood Cells Audio — Mangle](https://schneidersladen.de/en/blood-cells-audio-mangle) / [Magerit — Laniakea](https://schneidersladen.de/en/magerit-laniakea) (reforços breves)
- [SchneidersLaden — catálogo Eurorack 3U, páginas 16-25](https://schneidersladen.de/en/eurorack-modular-3u?p=16) (páginas 1 a 25 percorridas até agora, de 111 totais)
- [New Systems Instruments — Discrete Map](https://schneidersladen.de/en/new-systems-instruments-discrete-map)
- [Hieroglyphic — Plume Pulsar Generator](https://schneidersladen.de/en/hieroglyphic-plume-pulsar-generator-silber)
- KOMA Elektronik — Sensei / linha 1U (UFO, Detective, FM Radius, KMNDR) / Bela — Gliss 1U (reforços breves, mesmo tema ADDAC 308-311)
- [SchneidersLaden — catálogo Eurorack 3U, páginas 26-45](https://schneidersladen.de/en/eurorack-modular-3u?p=26) (páginas 1 a 45 percorridas até agora, de 111 totais)
- [Emute Labs — uSeq](https://schneidersladen.de/en/emute-labs-useq) / [modulisp (linguagem)](https://www.emutelabinstruments.co.uk/modulisp-about/) / [GitHub](https://github.com/Emute-Lab-Instruments/uSEQ)
- [Joranalogue x Hainbach — Collide 4](https://schneidersladen.de/en/joranalogue-audio-design-x-hainbach-collide-4)
- [Robaux Musique — DCSN3](https://schneidersladen.de/en/robaux-musique-dcsn3) ([manual](https://robaux.io/assets/files/robauxDCSN3Manual.pdf))
- ADDAC 507 Random Bezier Waves / ADDAC 508 Swell Physics / Noise Engineering Gamut Repetitor / Expert Sleepers Otterley / Flame Ton, Takt, Airlectron — observados por nome no catálogo, sem página individual aberta (ver seção 12)
- [SchneidersLaden — catálogo Eurorack 3U, páginas 46-65](https://schneidersladen.de/en/eurorack-modular-3u?p=46) (páginas 1 a 65 percorridas até agora, de 111 totais)
- [RYK Modular — Vector Wave](https://schneidersladen.de/en/ryk-modular-vector-wave)
- [Tiptop Audio — Z-8000 Matrix Sequencer](https://schneidersladen.de/en/tiptop-audio-z-8000-matrix-sequencer)
- [Xaoc Devices — Sofia](https://schneidersladen.de/en/xaoc-devices-sofia)
- [Neuzeit Instruments — Quasar](https://schneidersladen.de/en/neuzeit-instruments-quasar) ([manual](https://www.neuzeit-instruments.com/mediafiles/Manuals/Quasar_Manual_1_1.pdf))
- Five12 Vector Sequencer — observado por nome, descrição de fabricante truncada demais pra virar ficha técnica verificável (ver seção 12)
- [SchneidersLaden — catálogo Eurorack 3U, páginas 66-85](https://schneidersladen.de/en/eurorack-modular-3u?p=66) (páginas 1 a 85 percorridas até agora, de 111 totais)
- [Shakmat Modular — Harlequin's Context](https://schneidersladen.de/en/shakmat-modular-harlequin-s-context) ([manual](https://shakmat.com/doc/hc/HC-User_Manual.pdf))
- [u-he — CVilization](https://schneidersladen.de/en/u-he-cvilization) ([manual](https://uhe-dl.b-cdn.net/manuals/eurorack/cvilization/CVilization-user-guide.pdf) / [cheat sheet](https://uhe-dl.b-cdn.net/manuals/eurorack/cvilization/CVilization-cheat-sheet.pdf))
- [E-RM — Polygogo](https://schneidersladen.de/en/e-rm-polygogo) ([página do fabricante](https://www.e-rm.de/polygogo/))
- [Doepfer — A-174-4v 3D Joystick (Vintage Edition)](https://schneidersladen.de/en/doepfer-a-174-4v-3d-joystick-vintage-edition) — reforço breve, ver seção 12
- [SchneidersLaden — catálogo Eurorack 3U, páginas 86-111](https://schneidersladen.de/en/eurorack-modular-3u?p=86) (páginas 1 a 111 percorridas — catálogo completo, ver seção 12)
- [WORNG Electronics — Vector Space](https://schneidersladen.de/en/worng-electronics-vector-space) — reforço breve, ver seção 12

### BiomaBass (Módulo 3.4)

- [Best Eurorack modules — MusicRadar](https://www.musicradar.com/news/the-best-eurorack-modules-in-the-world)
- [Good oscillator choices for bass sounds — ModWiggler](https://www.modwiggler.com/forum/viewtopic.php?p=56965)
- [Eurorack Filter Comparison — Moinsound](https://moinsound.wordpress.com/2019/03/04/eurorack-filter-comparison/)
- [15 Best Eurorack Modules — sarahleeandjohnny.com](https://www.sarahleeandjohnny.com/best-eurorack-modules/)
- [Synth Bass: 7 Bass Types and How to Build Them — LANDR](https://blog.landr.com/synth-bass/)
- [How to use FM and oscillator sync to bring synth bass to life — MusicRadar](https://www.musicradar.com/how-to/fm-osc-sync-bass)

### Técnicas de síntese e engenharia

- [Tutorial: Wavetable synthesis — JUCE](https://juce.com/tutorials/tutorial_wavetable_synth/)
- [Thru-Zero FM Explainer](http://navsmodularlab.blogspot.com/2017/11/thru-zero-fm-explainer.html)
- [Wavefolding — KVR Audio DSP forum](https://www.kvraudio.com/forum/viewtopic.php?t=501471)
- [Antialiasing Piecewise Polynomial Waveshapers (DAFx23)](https://www.dafx.de/paper-archive/2023/DAFx23_paper_61.pdf)
- [PolyBLEP Oscillator (GitHub, martinfinke)](https://github.com/martinfinke/PolyBLEP)
- [Karplus–Strong string synthesis — Wikipedia](https://en.wikipedia.org/wiki/Karplus%E2%80%93Strong_string_synthesis)
- [Bjorklund Algorithm em C++ (GitHub Gist)](https://gist.github.com/unohee/d4f32b3222b42de84a5f)
- [Euclidean rhythm — Wikipedia](https://en.wikipedia.org/wiki/Euclidean_rhythm)
- [Lorenz Attractor — Cherry Audio](https://store.cherryaudio.com/modules/lorenz-attractor)
- [GRNLR — tutorial de síntese granular em JUCE (GitHub Wiki)](https://github.com/passivist/GRNLR/wiki/Getting-Started)
- [juce::Synthesiser class reference](https://docs.juce.com/master/classSynthesiser.html)
- [SimpleCompressor — limiter com lookahead (Daniel Rudrich)](https://github.com/DanielRudrich/SimpleCompressor)
- [Multiband_Crossover — Linkwitz-Riley em JUCE](https://github.com/marcossrivas/Multiband_Crossover)
- [ITU-R BS.1770 Revisited (Thomas Lund)](https://toneprints.com/media/1018237/lundt013011.pdf)
- [JUCE LICENSE.md](https://github.com/juce-framework/JUCE/blob/master/LICENSE.md)
- [DPF — DISTRHO Plugin Framework](https://github.com/DISTRHO/DPF)
- [Surge XT — FAQ / licenciamento](https://surge-synthesizer.github.io/faq/)
- [Vital — código-fonte GPLv3 (GitHub)](https://github.com/mtytel/vital)
- [VCV Rack — Plugin Licensing](https://vcvrack.com/manual/PluginLicensing)

### Pesquisa ModularGrid Top 100 (seção 9)

- [ModularGrid — Top 100 Eurorack Modules](https://modulargrid.net/e/modules/evaluationlists)
- [Music Thing Modular — Turing Machine Mk II (Black)](https://modulargrid.net/e/music-thing-modular-turing-machine-mk-ii-black)
- [Frap Tools — SAPÈL](https://modulargrid.net/e/frap-tools-sapEl)
- [mylar melodies — RANDOM8](https://modulargrid.net/e/mylar-melodies-random8)
- [Shakmat — Time Wizard](https://modulargrid.net/e/shakmat-time-wizard)
- [Blukač — Fractalist](https://modulargrid.net/e/blukac-fractalist)
- [Rossum Electro-Music — Control Forge](https://modulargrid.net/e/rossum-electro-music-control-forge)
- [Joranalogue Audio Design — Step 8](https://modulargrid.net/e/joranalogue-audio-design-step-8)
- [Intellijel — Metropolix](https://modulargrid.net/e/intellijel-metropolix)
- [vpme.de — Euclidean Circles v2](https://modulargrid.net/e/vpme-de-euclidean-circles-v2-)
- [ACL — Sinfonion](https://modulargrid.net/e/acl-sinfonion)
- [Mannequins — JUST FRIENDS](https://modulargrid.net/e/mannequins-just-friends)
- [Befaco — Oneiroi](https://modulargrid.net/e/befaco-oneiroi)
- [Instruō — arbhar](https://modulargrid.net/e/instruo-arbhar)
- [Noise Engineering — Desmodus Versio](https://modulargrid.net/e/noise-engineering-desmodus-versio)
- [Xaoc Devices — Praga](https://modulargrid.net/e/xaoc-devices-praga)
- [WORNG Electronics — Glowing Orb](https://modulargrid.net/e/worng-electronics-glowing-orb)
- [Make Noise — Maths](https://modulargrid.net/e/make-noise-maths--)
- [Mannequins — THREE SISTERS](https://modulargrid.net/e/mannequins-three-sisters)
- [Joranalogue Audio Design — Orbit 3](https://modulargrid.net/e/joranalogue-audio-design-orbit-3)
- [WMD — Sequential Switch Matrix (SSM)](https://modulargrid.net/e/wmd-sequential-switch-matrix-ssm)
- [Nonlinearcircuits — Triple Sloths V2](https://modulargrid.net/e/nonlinearcircuits-triple-sloths-v2)
- [vpme.de — QD Quad Drum Voice](https://modulargrid.net/e/vpme-de-qd-quad-drum-voice)
- [Acid Rain Technology — Chainsaw](https://modulargrid.net/e/acid-rain-technology-chainsaw)
- [Bastl Instruments — Déjà Vu](https://modulargrid.net/e/bastl-instruments-deja-vu)
- [DivKid — ochd](https://modulargrid.net/e/divkid-ochd)
- [Make Noise — 0-Coast](https://modulargrid.net/e/make-noise-0-coast-)
- [Xaoc Devices — Belgrad](https://modulargrid.net/e/xaoc-devices-belgrad)
- [Xaoc Devices — Kamieniec II](https://modulargrid.net/e/xaoc-devices-kamieniec-ii)
- [Frap Tools — FUMANA](https://modulargrid.net/e/frap-tools-fumana)
- [Mutable Instruments — Tides](https://modulargrid.net/e/mutable-instruments-tides)
- [Mutable Instruments — Blades](https://modulargrid.net/e/mutable-instruments-blades)
- [Rossum Electro-Music — Evolution](https://modulargrid.net/e/rossum-electro-music-evolution)
- [Cosmotronic — Messor](https://modulargrid.net/e/cosmotronic-messor)
- [Erica Synths — Sample Drum](https://modulargrid.net/e/erica-synths-sample-drum-)
- [Random*Source — Serge Resonant Equalizer (ResEQ)](https://modulargrid.net/e/random-source-serge-resonant-equalizer-reseq)
- [Westlicht — PER|FORMER](https://modulargrid.net/e/westlicht-per-former)

### Setups de artistas

- [Richard Devine — rig ao vivo (Reverb.com)](https://reverb.com/news/right-on-the-seat-of-losing-it-all-richard-devine-on-the-thrill-of-eurorack)
- [Autechre — Equipboard](https://equipboard.com/pros/autechre)
- [Caterina Barbieri — entrevista Sound on Sound](https://www.soundonsound.com/news/interview-minimalist-electronic-artist-caterina-barbieri)
- [Alessandro Cortini — live rig (Synthtopia)](https://www.synthtopia.com/content/2017/10/31/alessandro-cortini-explains-his-live-rig/)
- [Keith Fullerton Whitman — Wikipedia](https://en.wikipedia.org/wiki/Keith_Fullerton_Whitman)
- [Venetian Snares — modular caseiro (Vice)](https://www.vice.com/en/article/how-venetian-snares-homemade-modular-synth-inspired-his-new-album/)

## 9. Pesquisa ModularGrid Top 100 — triagem e apropriação original

Levantamento do [Top 100 Eurorack Modules do ModularGrid](https://modulargrid.net/e/modules/evaluationlists) (mais votados + mais usados em racks). Critério de triagem: descartadas utilidades puras (mults, mixers passivos, fones, blank panels) e mantido só o que tem princípio de circuito/algoritmo relevante aos módulos já definidos na arquitetura. Cada item abaixo tem ficha técnica verificada na página do fabricante/ModularGrid e uma proposta de **apropriação original** — não clonar o módulo, usar o princípio.

| Módulo real | Conceito / ficha técnica | Apropriação original proposta pro Aquorbium |
|---|---|---|
| **Music Thing Modular — Turing Machine Mk II** ($110, 10HP) | Sequenciador de loop aleatório: registrador de deslocamento com realimentação, trimmer "Lock" controla a chance de mutação de cada bit por ciclo | `ShiftRegisterMemory` dentro do BrainEngine, com a probabilidade de mutação amarrada a `Desire::calm` — quanto mais "calmo" o desejo, mais o padrão trava em loop; quanto mais agitado, mais muta. Ninguém liga isso a um sistema de "desejo" externo. |
| **Frap Tools — SAPÈL** ($490, 18HP) | "Tamed Random Source": 2 canais paralelos de CV aleatória (flutuante, quantizada em semitons, quantizada em oitavas, S&H não quantizado), com **distribuição de probabilidade configurável** (uniforme → gaussiana) por potenciômetro | Ligar a forma da distribuição de probabilidade ao traço `intelligence` de cada `Organism`: seres "inteligentes" sorteiam de uma distribuição mais gaussiana (decisões "ponderadas"), seres "malucos" de uma distribuição mais achatada/uniforme (comportamento errático) — personalidade afeta a própria estatística do acaso, não só a amplitude. |
| **mylar melodies — RANDOM8** ($325, 8HP) | 8 canais de CV aleatória com máquina de estado por canal: aleatório → **looping** → **evolving** (valores mudam devagar) → **travado**, ciclada por botão | Aplicar essa mesma máquina de 4 estados não à posição, mas à *personalidade* do organismo (fica evolvendo lentamente e eventualmente "trava" um traço por um tempo) — encaixa direto na ideia já existente de estações/acasalamento: travar = compromisso de uma característica durante a época de acasalamento. |
| **Shakmat — Time Wizard** ($182, 8HP) | Divisor de clock sêxtuplo em 2 seções, com multiplicação estranha (x4/÷7 etc.) e combinação lógica (AND/OR) entre duas saídas de divisor | Usar o mesmo truque de AND/OR entre dois divisores pra gerar acentos polirrítmicos "de graça" em cima do núcleo euclidiano de Bjorklund, sem CPU extra — implementa o item "clock fracionário" já previsto na Fase 3 com uma técnica concreta e barata. |
| **Blukač — Fractalist** ($350, 12HP) | Oscilador "fractal": um CORE se auto-replica a cada ITERação: em frequência alta vira textura digital caótica, em frequência baixa vira **gerador de ritmo primitivo** — o mesmo algoritmo, dois papéis, só mudando o registro de frequência | Implementar um `FractalCore` único usado tanto no BrainEngine (taxa de controle) quanto no SynthFMVoice (taxa de áudio) — literaliza a ideia de "ecossistema" de que a mesma lógica de organismo opera em escalas de tempo diferentes (ritmo macro vs. timbre micro). |
| **Rossum Electro-Music — Control Forge** ($599, 22HP) | Gerador de função/CV de 8 estágios, 191 formas de transição, **salto condicional por estágio** (pula pra outro estágio se uma CV cruzar um limiar, ou um gate/borda ocorrer) | Blueprint concreto pro "Cross-Feedback State Machine"/Markov já listado como "exploração futura" no Módulo 1: em vez de tabela Markov abstrata, estágios com salto condicional literal amarrado aos próprios campos do Aquário (`Desire::danger`, `Environment::isNight()`, `AquariumIntegrity::crackStress`) — promove esse item de "exploração futura" pra core, já que mapeia direto no código existente. |
| **Joranalogue — Step 8** ($405, 16HP) | Registrador sequencial/de amostragem: 8 estágios endereçáveis tanto pelo contador interno (sequencial) quanto **diretamente por uma CV externa** (acesso associativo, fora da ordem) | Precedente de hardware exato pra como `AccessPort`/`Desire` deveriam poder "alcançar" um ponto específico do estado interno do sequenciador sem quebrar a regra de sistema fechado — implementar a memória de passos do GeometricSequencer nesse molde de endereçamento duplo. |
| **Intellijel — Metropolix** ($579, 34HP) | Sequenciador multitrilha: contagem de ratchet e probabilidade por passo, e **transposição acumulativa** (o offset de tom se acumula a cada repetição do ciclo) | Aplicar transposição acumulativa ao Interval Shift Quantizer, com a direção do acúmulo invertendo conforme `Environment::seasonPhase` (verão soma, inverno subtrai) — o centro tonal deriva ao longo do tempo por causa da estação, não por configuração manual. |
| **vpme.de — Euclidean Circles v2** ($299, 10HP) | 6 canais euclidianos de 16 passos, encadeáveis até 48 passos, com **salvamento/restauração rápida** de padrão ao vivo | Encadear padrões pra dar forma de longo prazo (casa com a ideia de "seções" do Sinfonion abaixo); usar salvar/restaurar como o mecanismo concreto de recuperação do `AquariumIntegrity` — ao curar, volta pro último padrão travado; ao trincar, força um padrão novo aleatório. |
| **ACL — Sinfonion** ($1075, 42HP) | "Processador harmônico complexo": 3 quantizadores + gerador de acorde de 4 vozes + arpejador + **sequenciador de progressão de até 32 seções**, pra músicas completas de 4 partes | Proposta de novo submódulo "Progressão Harmônica" entre o Cérebro e o Sequenciador: motor leve de acordes/seções cujas fronteiras de seção são ditadas pelo ciclo dia/noite/estação do `Environment`, não por contagem fixa de compassos — forma de música nascendo do relógio ecológico, não de arranjo pré-definido. |
| **Mannequins — JUST FRIENDS** ($420, 14HP) | 6 canais idênticos reaproveitados como envelope/LFO/oscilador conforme o registro de TIME; **INTONE** trava múltiplos canais em relações harmônicas de fração inteira entre si | Reforça o padrão do Fractalist: um `VectorGenerator` único usado em taxa de controle e de áudio. E usar o travamento por fração inteira como mecanismo literal de `SchoolInfo::sharedPhase` — cardumes passam a ter coesão de fase harmônica de verdade, não só uma média (lerp) como hoje. |
| **Befaco — Oneiroi** ($479, 30HP) | Voz ambiente digital: oscilador wavetable de 2048 amostras que lê do **próprio buffer do looper** (resíntese do próprio áudio gravado); "soft take-over" faz parâmetros aleatorizados deslizarem de volta pra posição física do knob em vez de saltar | Duas apropriações: (a) deixar o BiomaBass ou o granular striker ocasionalmente resintetizar uma wavetable a partir do buffer de saída recente do próprio Aquorbium — literaliza em áudio o Cross-Feedback State Machine que hoje só existe em modulação; (b) usar suavização "soft take-over" em toda automação vinda de `Desire`/BrainEngine, garantindo que `AccessPort` nunca cause descontinuidade — implementação concreta do princípio "desejo sem controle direto" da seção 1. |
| **Instruō — arbhar** ($655, 18HP) | Processador granular: até 88 grãos polifônicos com **parâmetro definido por grão** (não é uma nuvem uniforme), detecção de onset pra captura automática | Cada `Organism` de `Species::GranularStriker` ganha seu próprio conjunto de parâmetros por grão derivado dos traços de personalidade (força/agilidade/inteligência), e captura automaticamente trechos do mix recente do aquário via detecção de onset como matéria-prima — de novo, autoescuta virando material sonoro de verdade, não só modulação. |
| **Noise Engineering — Desmodus Versio** ($393, 10HP) | Reverb descrito explicitamente como **não-simulador de sala** — gerador sintético de cauda, contínuo entre delay → reverb → atmosfera "pesadelo"; plataforma de DSP com firmware aberto | Reenquadrar o módulo de Reverb da mesma forma: não simula sala física, gera uma "cauda de ambiente do aquário" cujo caráter (tamanho/escuridão/direção) responde a `Environment` (dia/noite, estação) e a `AquariumIntegrity::crackStress` (quanto mais perto da fratura, mais "pesadelo" a cauda fica). |
| **Xaoc Devices — Praga** ($543, 20HP) | Mixer VC de 4 canais estéreo: **compressão dinâmica da própria CV de controle** pra não estourar quando empurrada demais, panorâmica de loudness igual (equal-power) | Aplicar direto em `Ecosystem::process()`: comprimir suavemente os pesos calculados de `philosophyBlend` antes de usar, e trocar o pan linear atual de `panLeft`/`panRight` por panorâmica de potência constante — corrige um detalhe de engenharia real que o header ainda não tem. |
| **WORNG Electronics — Glowing Orb** ($339, 10HP) | Filtro SVF estéreo (núcleo estilo SSM/Prophet-5) com **drive de auto-compensação de ganho** — a saída recua automaticamente conforme a distorção aumenta, protegendo headroom | Aplicar o mesmo princípio de auto-compensação no estágio de wavefolder a diodo do BiomaBass (antes do filtro) — cranking do wavefold não estoura o headroom sozinho, complementando o loop de feedback com BallisticsFilter já planejado. |

### Segunda rodada — mais módulos e múltiplos caminhos de apropriação por módulo

*Ampliação pedida explicitamente: 15 módulos numa lista de 100 é pouco. Esta rodada soma mais 20 (total 36 aprofundados), e — mais importante — trata cada módulo em busca de mais de um caminho de uso: função alternativa do mesmo circuito, rota de modulação incomum, ou tipo de voz distinto. Organizado por tema, não por módulo, pra facilitar comparação.*

#### Um gerador, várias funções (mesma célula, registro de frequência diferente)

Esse padrão já apareceu duas vezes na primeira rodada (Just Friends, Fractalist) — aqui ele se confirma como recorrente, não coincidência:

- **Mutable Instruments Tides** ($259, 14HP — núcleo digital MIT-licenciado, ver nota de licenciamento abaixo) — um único gerador de função triangular assimétrica opera em 3 modos (AD, AR, looping/VC-LFO/VC-DO) e tem 4 *modos de saída* diferentes a partir do mesmo core: formas diferentes, amplitudes diferentes, tempos diferentes, e **frequências diferentes** (saídas 2/3/4 deslocadas em frequência da saída 1, criando "acordes ou LFOs polirrítmicos" a partir de um único gerador). — **Apropriação:** usar exatamente essa técnica de "4 modos de saída" no `FluidMotion` do Ecosystem — hoje cada organismo tem sua própria instância; em vez disso, um só gerador por cardume poderia alimentar todos os membros com fases/frequências relacionadas, reduzindo custo de CPU e aumentando coesão real de cardume ao mesmo tempo.
- **Rossum Control Forge** (já citado) — o mesmo motor de 8 estágios vira sequenciador puro usando "formas DC" em vez de curvas de transição. — **Apropriação (caminho alternativo):** o mesmo `ShiftRegisterMemory`/state machine proposto pro BrainEngine pode, com o registro de tempo acelerado pra taxa de áudio, virar diretamente um oscilador de estados discretos (uma onda "step" gerada pelo mesmo autômato que gera ritmo) — thread entre Módulo 1 e Módulo 3 sem duplicar código.

#### Caminhos de modulação incomuns

- **WMD Sequential Switch Matrix (SSM)** ($329, 16HP) — matriz de roteamento 4×4 onde cada configuração de roteamento é ela mesma um "quadro" numa sequência de até 16 por banco (4 bancos), avançável por gate, endereçável por CV, e com um modo de **randomizar automaticamente a cada seleção**. — **Apropriação:** tratar as conexões entre os 5 módulos técnicos (Cérebro→Sequenciador→Síntese→Espaço→Master) não como um roteamento fixo no `PluginProcessor`, mas como uma "matriz" que o `Desire`/`AccessPort` pode reconfigurar em tempo real — literalmente a topologia do feedback loop do Bioma vira parametrizável, não só os valores que fluem por ela.
- **Nonlinearcircuits Triple Sloths V2** ($210, 8HP) — 3 circuitos de caos independentes em taxas muito diferentes (15–30s, 60–90s, 30–40min), com um **retificador de diferença** que combina os três (`Apathy + Inertia − Torpor`) em duas saídas (+/−). — **Apropriação:** em vez de um único Slew-Rate Random Walk no Cérebro, ter 3 instâncias em escalas de tempo bem diferentes (segundos/minutos/dezenas de minutos) e combiná-las algebricamente — dá ao Aquorbium uma "deriva de longuíssimo prazo" real (na escala de uma sessão inteira), coisa que nenhum dos geradores atuais cobre.
- **DivKid ochd** ($185, 4HP) — 8 LFOs analógicos não sincronizados, com o truque documentado de **repatchar uma saída de volta na própria entrada de CV** pra criar formas exponenciais/quadradas/escalonadas por feedback. — **Apropriação:** aplicar autopatch/feedback ao próprio `FluidMotion` — a saída de fase de um organismo realimentando seu próprio parâmetro de velocidade, gerando formas de movimento não-senoidais sem adicionar osciladores novos.
- **Bastl Instruments Déjà Vu** ($163, 4HP) — utilitário simples (atenuador/atenuverter/crossfader) que vira "controle macro de performance" no modo crossfade. — **Apropriação:** usar o modo crossfade como o mecanismo de transição suave entre dois estados do `AquariumIntegrity` (saudável ↔ trincado) em vez de uma troca abrupta — suaviza a "consequência" descrita na seção 1 sem exigir uma implementação de envelope dedicada.
- **Joranalogue Orbit 3** ($290, 10HP) — atrator caótico "double-scroll" real (não é ruído aleatório, é caos determinístico com regiões biestáveis), atua tanto em CV quanto em áudio, saídas em pares complementares pensadas pra visualização XY em osciloscópio. — **Apropriação:** `DoubleScrollAttractor` implementa uma interpretação digital autoral desse papel e já participa de `LORENZ + DRIFT`; continua faltando o teste A/B musical contra `LorenzCV` como fonte de caos orgânico não repetitivo.

#### Filtro como algo além de filtro

- **Xaoc Devices Belgrad** ($378, 14HP) — filtro multimodo de dois picos ressonantes simultâneos, 10 respostas de frequência, capacidade de formante, **realimentação e cross-modulação não-lineares entre os dois núcleos**, self-oscillation calibrada (funciona como oscilador de verdade). — **Apropriação:** usar como referência de topologia pro filtro do BiomaBass em vez de um diode ladder simples — dois picos ressonantes cross-modulando entre si dá o "corpo duplo" que nenhuma das referências de mercado já listadas (Joranalogue Filter 8, Oakley Diode Superladder) tem sozinha.
- **Mutable Instruments Blades** ($359, 18HP — MIT) — overdrive/wavefolder controlado por CV *antes* de cada um dos dois filtros (mesmo princípio já adotado do Mavis, mas aqui com wavefold *continuamente* variável até soft-clip), e roteamento contínuo entre série/paralelo/**crossfade** entre os dois filtros. — **Apropriação:** ao BiomaBass, adicionar a opção de roteamento variável (não fixo) entre um estágio "grave" e um estágio "agressivo" do filtro, controlável pelo próprio `Desire::danger` — quanto mais "perigoso" o desejo, mais a mixagem interna desliza de paralelo pra série.
- **Rossum Electro-Music Evolution** ($499, 16HP) — ladder filter clássico (linhagem Moog/E-mu) com **contagem de polos controlada por tensão** (3 a 6 polos, mudando o ângulo de fase da ressonância em tempo real — som "bolhoso" ao modular). — **Apropriação:** expor o número efetivo de polos do filtro do BiomaBass como um parâmetro modulável pelo Cérebro, não fixo em projeto — nenhuma referência de mercado já listada faz isso, é uma dimensão nova de timbre.
- **Frap Tools FUMANA** ($991, 42HP) — dois bancos analógicos de 16 filtros passa-banda cada; um banco analisa (envelope followers), o outro resintetiza (VCAs) = vocoder/transferência espectral completo, com controles de "tilt" (inclinar o espectro) e "scan" (pico/vale móvel tipo EQ paramétrico). — **Apropriação:** motor de vocoder como caminho alternativo de síntese pro Módulo 3 — o Cérebro (ou o próprio mix do Aquário) analisando espectralmente e resintetizando sobre a saída do BiomaBass, uma forma literal do "cross-feedback" já batizado no conceito, mas no domínio espectral em vez de CV.
- **Random\*Source Serge ResEQ** ($399, 18HP, licenciado do design Serge original) — 10 bandas ressonantes espaçadas por **sétima maior** (não oitava, não terço — um espaçamento deliberadamente "desafinado" que produz caráter de formante orgânico), saídas COMB (soma de bandas alternadas) vs. saída mixada total, feedback com inversão de fase pra cancelamento. — **Apropriação:** usar esse espaçamento não-octavado como referência pro banco de ressonadores do Módulo 4 (Espaço) — foge do "filtro passa-banda ressonante Q 20–100" genérico já no plano, dando uma cor de formante mais orgânica no Multi-Particle Noise Striker.
- **Mannequins THREE SISTERS** ($420, 10HP) — 3 filtros ligados: FREQ move todos juntos, SPAN os empurra pra longe uns dos outros (isolamento); em Q alto viram "coro" cantando 3 tons. — **Apropriação:** modelo direto pro `SchoolInfo` de um cardume de 3+ organismos do tipo síntese — FREQ/SPAN mapeiam literalmente em "frequência central do cardume" vs. "dispersão individual", os mesmos dois parâmetros que já regem `schoolCohesion` hoje, mas aplicados a um banco de filtros real em vez de só à posição espacial.

#### Vozes diferentes

- **vpme.de QD — Quad Drum Voice** ($425, 22HP) — 4 vozes de percussão digital independentes, cada uma podendo ser sample player, VCO wavetable, **ou** modelo físico de percussão — tudo no mesmo módulo, escolhido por voz. — **Apropriação:** cada organismo pode escolher seu corpo sonoro, mas a implementação canônica evita `renderSample()` virtual dentro de `OrganismState`. `OrganismVoiceEngine` faz despacho tipado, fixo e testável entre espécie e DSP; motores grandes futuros exigem orçamento/pool próprio.
- **Erica Synths Sample Drum** ($399, 14HP) — voz de percussão baseada em amostra via cartão SD, controle de pitch/decay por voz. — **Apropriação:** referência simples pra decidir a interface mínima de uma eventual voz de percussão baseada em amostra dentro do Aquorbium (pitch + decay + seleção), caso o granular/físico não seja suficiente pra golpes curtos e definidos.
- **Acid Rain Technology Chainsaw** ($269, 4HP, descontinuado) — 3 vozes polifônicas, 7 formas de onda cada, todas morfando continuamente de super-serra pra super-quadrada. — **Apropriação:** técnica de morph contínuo entre duas famílias de forma de onda (não um crossfade linear ingênuo, mas uma interpolação de harmônicos) como caminho alternativo de geração de timbre pro Módulo 3, complementar ao wavetable+FM já planejado.
- **Make Noise 0-Coast** ($510, 45HP) — sintetizador west-coast completo (não é só um módulo, é uma arquitetura de voz inteira: função geradora, low-pass gate, osciladores complementares) — referência de arquitetura de ponta a ponta, não de circuito isolado. — **Apropriação:** vale estudar o fluxo interno completo do 0-Coast como um terceiro "arquétipo de voz" de referência ao lado do BiomaBass (que já mistura west-coast/east-coast) — não pra clonar, mas pra garantir que o Aquorbium também ofereça uma voz "pura" west-coast entre as opções de organismo.
- **Cosmotronic Messor** ($267, 8HP) — compressor cuja **própria envolvente de redução de ganho é disponibilizada como saída de CV separada** — o efeito dinâmico vira, ele mesmo, uma fonte de modulação. — **Apropriação:** no Módulo 5 (Master), expor a envolvente de redução de ganho do compressor multibanda como uma entrada adicional pro Cérebro — fecha mais um loop de feedback genuíno do conceito Bioma (a própria dinâmica final do master realimentando decisões estruturais), além do já previsto `BallisticsFilter`.

#### Sequenciamento e memória

- **Make Noise Maths** ($290, 20HP — o módulo mais votado da história do ModularGrid, 583 votos) — gerador de função complexo multifunção: 2 canais de envelope/slew mais lógica embutida, atenuadores bipolares, tudo combinável. — **Apropriação:** referência canônica de "um bloco, muitas funções" — mantido aqui explicitamente porque é o precedente histórico direto de todo o padrão "VectorGenerator único, vários papéis" já proposto em duas outras entradas desta lista; vale citar como a origem conceitual dessa técnica no mundo Eurorack real.
- **Westlicht PER|FORMER** ($472, 34HP) — sequenciador de 8 trilhas **totalmente open source** (hardware e firmware), com sistema de snapshot e **geradores de sequência algorítmicos** embutidos (não só programação manual passo a passo). — **Apropriação:** além de reforçar o sistema de salvar/restaurar já proposto (via Euclidean Circles v2), o fato de ser open source de verdade o torna a única referência desta lista inteira que pode ser lida linha a linha como código de estudo, sem qualquer ambiguidade de licença — candidato forte pra "codebase de referência" ao lado de Surge XT/Vital/VCV Rack na seção 3.

**Nota sobre Mutable Instruments:** Blades e Tides (ambos citados acima) são de fabricante cujo código-fonte de DSP é historicamente publicado em MIT no GitHub de Émilie Gillet — coerente com o princípio "só tecnologia livre" da seção 2, e um motivo a mais pra tratá-los como referência de estudo, não só de conceito.

**Outros candidatos observados na lista, ainda não aprofundados** (nomes guardados pra não perder a referência): Xaoc Sarajewo, Xaoc Sofia, Joranalogue Filter 8 (já referenciado na seção 5), Joranalogue Contour 1, Frap Tools BRENSO, Mutable Instruments Ripples (2020)/Veils (2020), Doepfer A-140-2, Schippmann VCF-02, Cosmotronic Delta-V, Industrial Music Electronics Kermit MK III/Piston Honda MK III.

## 10. Panorama completo do Top 100 — o que aconteceria se usássemos tudo

Pergunta direta: e se o Aquorbium absorvesse os 100 módulos do ranking, não só os 36 aprofundados? Resposta honesta, feita classificando os 100 (por nome, categoria e — pros 36 já pesquisados — ficha técnica real; os outros 64 foram classificados por categoria/nome do ModularGrid e conhecimento geral, sem ficha técnica individual verificada) em baldes funcionais:

| Balde | Quantos dos 100 | O que isso significa |
|---|---|---|
| **Utilidade pura** (mults, VCA genérico, headphone amp, painel de caso, output buffer) | ~14 | Zero pra apropriar — é infraestrutura que o JUCE/APVTS já resolve de fábrica (mixer bus, gain staging). Não vira código original, vira `juce::dsp::Gain` e ponto. |
| **Cérebro** (caos, modulação, clock, lógica) | ~23 | Alta redundância: por trás de LFOs quádruplos, divisores de clock e geradores de caos diferentes tem só **3–4 princípios reais** (distribuição de probabilidade configurável, atrator caótico determinístico, divisão/multiplicação de clock com lógica, matriz de roteamento). Já extraídos nas duas rodadas. |
| **Sequenciador/Geometria** | ~19 | Mesma coisa: os 4 módulos "DROID" do ecossistema Der Mann mit der Maschine são catálogo fragmentado de uma única plataforma. Removendo duplicatas, sobram uns **5 princípios reais** (euclidiano+mutação, transposição acumulativa, registrador endereçável, snapshot, progressão harmônica por seção) — todos já capturados. |
| **Síntese / Voz** | ~26 | O balde mais genuinamente diverso — é onde "vozes diferentes" realmente aparece. Arquétipos distintos: complexo west-coast (Buchla 281t/258t), digital wavetable (Freak, ONA), percussão (QD, Sample Drum, C4RBN), granular (arbhar), caótico (Akemie's Castle, Noise Plethora), plucked/touch (Plinky). ~metade já coberta; a outra metade é candidata real pra uma terceira rodada, não redundante. |
| **Filtro/Timbre** | ~13 | Diversidade real também, mas menor — a maioria já apareceu (Belgrad, Blades, Evolution, FUMANA, ResEQ, Three Sisters cobrem quase todo o espaço de "filtro como algo além de filtro"). |
| **Espaço/Master** | ~6 | Cobertura mais fina de todas — Praga, Messor, Rainmaker, Desmodus Versio já capturam o essencial; sobra pouco território novo aqui. |

**Veredito:** implementar "os 100" não significa escrever 100 pedaços de DSP — depois de remover utilidade pura e redundância, o Top 100 se reduz a algo entre **25 e 30 princípios genuinamente distintos**. E isso bate quase 1:1 com o que as duas rodadas de pesquisa já extraíram e colocaram no checklist. O ganho real de "usar os 100" não está em mais módulos — está no balde de Síntese/Voz, que é onde ainda há território não-redundante (arquétipos de voz completos, não só técnicas de modulação).

O risco de tentar usar tudo mesmo assim não é técnico, é o mesmo já apontado na seção 1: se cada um desses ~25–30 princípios virar uma espécie audível e distinta dentro do aquário, o ouvinte não consegue mais separar nada em 2 canais de saída — a "legibilidade sonora" quebra antes da CPU. (Ver seção 12, Mannequins Silhouette, pra um mecanismo de hardware real que ataca exatamente esse problema — primeiro plano que varre automaticamente, resto redistribuído como fundo, não uma escolha binária.) A recomendação prática é manter `Species` pequeno. Hoje o enum canônico contém Oscillator, Pulsar, Resonator e GranularStriker; a formação audível usa um corpo de cada espécie. Cérebro, Sequenciador, filtro, Espaço e Master colorem ou processam essas vozes, não viram automaticamente espécies. Isso favorece poucas vozes profundas e distinguíveis em vez de muitas vozes rasas.

**Candidatos concretos pra uma eventual terceira rodada** (só o balde Síntese/Voz não-redundante): Buchla 281t/258t (Tiptop), Freak (Vult), ONA (NANO Modules), Akemie's Castle (ALM), Noise Plethora (Befaco), Plinky (Thonk), Kermit MK III / Piston Honda MK III (Industrial Music Electronics).

## 11. Questões em aberto pra próxima rodada

- ~~Decisão definitiva: JUCE (AGPLv3) ou migrar pra DPF (ISC)~~ — **fechado**: fica em JUCE, licença AGPLv3 (ver seção 2 e 3).
- Legibilidade sonora: o que vira som audível como algo separado, e o que fica só como estrutura interna (ver seção 1 e seção 10).
- `AquariumIntegrity` (fissura/quebra) ainda é stub — decidir se a quebra definitiva abre side-chain de áudio externo de verdade, ou fica só interno. Pista concreta da seção 12 (EMW Mic. Interface): se abrir, a entrada externa deveria estressar `crackStress` via envelope, não entrar como canal de mixagem direto.
- Ambiente ar vs água, densidades diferentes por região do aquário — ainda não incorporado ao `Ecosystem.h`.
- Lupa: `glideTowards` espera um alvo externo (XY pad, parâmetro de host) — ainda não definido qual controle real da GUI/APVTS vai alimentar isso, nem como `tryHold`/`release` vira gesto de UI (clique-e-segura? toggle por observador?).
- Lupa de frequência (ver seção 12, Hieroglyphic Opalina modo Isolator): foco espectral complementar ao foco espacial já implementado — ainda não desenhado nem em conceito nem em código, só anotado como direção.
- Definir qual módulo vira código primeiro.

## 12. Pesquisa EMW (Electronic Music Works) — módulos que Lúcio já usou

*Diferente das seções 9-10 (pesquisa de catálogo, sem uso prático), esta rodada parte de hardware que Lúcio já tocou de verdade — o crivo aqui não é só "é original?", é "isso já provou o próprio valor sonoro na mão". Fonte: [electronicmusicworks.com](https://www.electronicmusicworks.com/) (catálogo completo, ~80 módulos, ficha técnica extraída direto da página de cada produto — ver seção 8). Mesmo critério de triagem das seções anteriores: utilidade pura (VCA genérico, mixer, atenuador, patch cable, MIDI thru) fica de fora da tabela — está coberta por tarefas já existentes e não pede apropriação nova.*

| Módulo real (EMW) | Ficha técnica | Apropriação original |
|---|---|---|
| **WAVE-6 Polyphonic Osc.** ($198, 10HP) | Osciladora wavetable **6 vozes simultâneas** por MIDI, 128 wavetables internas, ADSR/VCA embutido, CV de wave-morph e um segundo CV "WAVE CUT" (tipo PWM) | Em vez de instanciar um `Organism` por peixe de um cardume, um único gerador polifônico serve o `SchoolInfo` inteiro — as 6 vozes derivam de `sharedDirection`/`sharedPhase` com pequenos offsets de wave-morph por membro, em vez de 6 vozes independentes recalculando tudo do zero. WAVE CUT vira um terceiro eixo de caráter sonoro pro `Species::Oscillator`, ortogonal ao wavefolder e ao thru-zero FM já planejados (Fase 2/2b) |
| **POT ACTION RECORDER** ($152, 8HP) | Grava o gesto físico de girar um potenciômetro como CV (±2.5V), loop instantâneo, velocidade de playback ajustável | Literaliza `Desire`/`AccessPort` como **gesto humano gravado**: em vez de só um valor de `Desire` estático, capturar o próprio movimento de quem mexe no XY pad/philosophyBlend e devolver esse movimento como fonte de drift — um "gesto lembrado" que se repete e pode acelerar/desacelerar, categoria de motion nova ao lado de `FluidMotion` (procedural) e da migração/território (regrada). Também serve pra gravar o próprio caminho da Lupa sobre o vidro e fazer ela repetir o próprio trajeto |
| **RESONANT FILTER SEQUENCER** ($194, 16HP) | 8 filtros ressonantes fixos, sequenciados; modo "1/8" (um de cada vez) ou "4/2" (dois simultâneos); 4 modos de sequência (forward/backward/alternado/aleatório); ganho/atenuação de 10dB por banda | Passo do filtro sequenciado usando a própria rotação do aquário (`AquariumTransform::rotationY`) como "clock" em vez de um clock de áudio dedicado — o timbre muda conforme o aquário gira, não conforme um contador separado. O modo "4/2" (duas ressonâncias simultâneas) mapeia bem pra um encontro predador/presa: as duas frequências ressoando juntas são literalmente as duas "vozes" do embate |
| **FIXED FILTER BANK** ($138, 12HP) | 8 filtros passa-banda fixos analógicos, ±10dB, 8 saídas independentes de 6dB + saída principal, baixo ruído | Cada cardume ganha uma "assinatura espectral" própria: os 8 bandpasses fixos mapeados nas mesmas duas variáveis que já regem `SchoolInfo` (posição/dispersão do cardume), fazendo cada grupo soar reconhecível por timbre, não só por movimento — complementa a apropriação de Three Sisters (FREQ/SPAN) já no checklist (Fase 8), dessa vez como banco fixo em vez de par de picos móveis |
| **NOISE CINEMA** ($185, 12HP) | Morph contínuo entre **10 "atmosferas" de ruído**, modo contínuo ou por envelope, redução de sample rate, controle de tom, delay digital — o próprio fabricante descreve como "todo som conta uma história" | Combinação quase perfeita com `Environment`: a posição do Morph entre as 10 atmosferas passa a ser **derivada de `seasonPhase`/`dayPhase`**, não controlada manualmente — cada estação/hora do dia tem sua própria textura de fundo, não só velocidade/comportamento diferente. E a redução de sample rate amarrada a `AquariumIntegrity::crackStress`: um aquário trincado degrada digitalmente o próprio leito sonoro, não só a posição dos peixes (`worldPosition` já faz isso espacialmente — este é o equivalente timbral) |
| **MIX SEQUENCER** ($299, 22HP) | Baseado num módulo do ARP 2500 histórico: chaveia sinais de CV ou áudio entre entradas em padrões/modos, liga/desliga por canal, nível independente por entrada, clock externo | Precedente concreto de hardware pra uma ideia que já estava só comentada no código: `Ecosystem::philosophyBlend` (hoje estático ou XY pad manual) vira de fato **sequenciado** — um padrão que percorre qual das 4 filosofias domina, no tempo, clocado pelo próprio Cérebro (Módulo 1) em vez de só automação de host |
| **ENVELOPE FOLLOWER** ($82, 8HP) | Extrai envelope de áudio/CV/trigger/gate, attack/decay ajustável (rápido/lento), gera trigger/gate ao cruzar limiar ajustável | Confirma a abordagem já planejada via `juce::dsp::BallisticsFilter` (Cross-Feedback State Machine, Fase 4) e acrescenta um caminho novo: o cruzamento de limiar pode disparar diretamente uma transição de `PresenceState` ou reforçar um ataque de `PredatorStrategy` — a energia de saída do próprio Aquorbium passa a gerar eventos discretos (gate/trigger), não só valores de modulação contínuos |
| **VC WAVETABLE LFO** ($144, 8HP) | 31 formas de onda internas (do triângulo/rampa/seno/quadrada até ruído, padrões rítmicos e formas assimétricas), sync in/out, CV de FM dedicado | `FluidMotion` hoje deriva tudo de seno/cosseno (`std::sin`/`std::cos`), igual pra toda espécie. Trocar por um banco de formas — cada `Species` (ou cada `SurvivalStrategy`) sorteia sua forma de drift de um conjunto pequeno de formas não-senoidais, dando personalidade de movimento realmente distinta por tipo de peixe, não só velocidade/fase diferente |
| **MIC. INTERFACE** ($84, 6HP) | Pré-amp de microfone + gerador de trigger por limiar; o próprio fabricante recomenda usar junto do Envelope Follower pra seguir "o envelope da voz humana" | Esta é a única peça de hardware real de toda a pesquisa (ModularGrid + EMW) que justifica de verdade a exceção já registrada em `AquariumIntegrity`: microfone real, seguido por envelope follower, é o único ponto onde "nada de fora entra" deixa de valer *de propósito*. Apropriação: a voz/som do ambiente não entra como áudio mixado direto — ela **estressa `AquariumIntegrity`** via a energia do envelope, sentida como tensão estrutural, não como canal extra |
| **RING MODULATOR** ($79, 6HP) | Circuito clássico à base do LM1496 (mesmo do Roland System 100M) — dois sinais (carrier/modulation) multiplicados, saída = soma e diferença das frequências, timbre "robótico"/metálico | Em vez de efeito estático sempre ligado, ring-modular os dois organismos envolvidos no exato instante em que um `PredatorStrategy` alcança sua presa (`closest` cruza um limiar de contato) — um artefato sonoro literal do encontro, momentâneo, ligado a um evento ecológico específico, não a um insert de efeito permanente |
| **GLIDE PROCESSOR** ($62, 6HP) | Dupla de circuitos de glide/portamento, slew ajustável de 0 a 1.5s, aplicável a qualquer CV/sequência | Técnica concreta e barata (filtro passa-baixa de um polo, `slew += (target - slew) * coeff`) pra resolver o "soft take-over" já listado como tarefa (Fase 5b, princípio Befaco Oneiroi) — o Glide Processor é o precedente de hardware mais simples possível pra essa mesma necessidade, vale citar como a implementação de referência mais direta |
| **Sequential Voltage Source** (Soundfreak, €1870 — fabricante distinto da EMW) | Sequenciador multi-passo inspirado no Buchla Model 123/140: 8 passos, cada um guardando **4 tensões independentes** (linhas A-D) via array de potenciômetros; modo paralelo (4 trilhas × 8 passos) ou serial encadeado (16 ou 32 passos); pulso de clock interno/externo avança o passo, LED indica o passo ativo | Cada passo do `GeometricSequencer` carrega **4 CVs simultâneos**, não um só — ex.: pitch, duração de gate, viés de `SurvivalStrategy` dominante nesse passo, e quantidade de `Desire` aplicada — em vez de 4 sequenciadores paralelos independentes. E o chaveamento paralelo↔serial encadeado é precedente direto de hardware pra tornar a "memória de passos endereçável" (já no checklist, princípio Joranalogue Step 8) elástica em tamanho, não fixa em 8 |

**Observados e descartados por ficha técnica insuficiente na própria página** (X-Y Wave Rework, Bit Transformer, Wave Stereolizer): a EMW não publicou descrição textual desses três além do nome e das dimensões físicas. Ainda assim, os nomes por si só reforçam ideias já mapeadas — "X-Y Wave Rework" é mais um precedente de hardware real pra controle bidimensional tipo joystick (reforça a ideia já registrada de controlar `philosophyBlend`/Lupa por XY pad); "Bit Transformer" (bitcrusher) soma-se à degradação digital já proposta via Noise Cinema como reação sonora ao `AquariumIntegrity::crackStress`; "Wave Stereolizer" agrupa com Stereolizer Processor/Pseudo Stereo/Stereo Enhancer, já observados na seção 9 como candidatos de espacialização por observador. (O EMW "SEQUENTIAL VOLTAGE 16", que também tinha ficha incompleta, ganhou apropriação própria acima via o Soundfreak Sequential Voltage Source — produto diferente, mesma família conceitual de sequenciador multi-tensão Buchla.)

**Módulos vistos e não aprofundados por serem redundantes com apropriações já registradas** (sem ficha nova, só confirmação de que o caminho já escolhido é sólido): MINI-3 Oscillator Bank / VCO 104 / Sub-Osc Generator / Sub Station / 4-CH Poly-MIDI (reforçam osciladores múltiplos dessintonizados e polifonia — já cobertos pelo BiomaBass e pela apropriação do WAVE-6 acima); VCF S100 / VCF OB12 / VCF MG24 / EMW-200 Filters (mais três topologias de filtro clássicas — reforçam a ideia de roteamento/topologia variável já no checklist via Belgrad/Blades/Joranalogue Filter 8, Fase 2b); Bit Transformer (ver acima); Echo/Dual Delay/Phaser 4/Phaser 12 (cobertos pelo Multi-Tap Space, Fase 6); T-Drum/Digi Drum/BPM Center/T-Sequencer (cobertos pelas apropriações de sequenciamento já na Fase 3); Stereolizer Processor/Stereo Enhancer/Pseudo Stereo (ver Wave Stereolizer acima); Lógica (Gate OR 101, Logic 101, Random Pulse 4x) — reforçam o "salto condicional" já apropriado do Rossum Control Forge (Fase 2c).

### Soundfreak — módulos irmãos do Sequential Voltage Source

*Soundfreak não é a EMW — é outro fabricante boutique, formato 4U (não Eurorack padrão), recriações/reinterpretações do EMS VCS3/Synthi com adições modernas (controle de tensão onde a VCS3 original não tinha). Preços bem mais altos (€1260–2220), mas a herança de design da VCS3 — matriz que não separa arquitetonicamente áudio de controle — traz um princípio que a pesquisa ainda não tinha capturado desse jeito.*

| Módulo real (Soundfreak) | Ficha técnica | Apropriação original |
|---|---|---|
| **Triple VCO** (€2220) | 3 osciladores com controles idênticos, faixas/formas de onda diferentes por posição; faixa estende bem abaixo do audível (serve como som OU como modulação, herança direta da matriz da VCS3 original); saídas em Tini-Jax e banana | Generaliza o padrão já usado em `FractalCore`/`VectorGenerator`/ochd ("o mesmo gerador serve taxa de controle e taxa de áudio") como **princípio de roteamento do `AccessPort`**, não mais de um gerador específico só: qualquer saída de `Organism` pode virar voz OU fonte de modulação de outro parâmetro, dependendo só de pra onde é roteada — a fronteira áudio/controle deixa de ser arquitetural e vira só uma questão de patch/roteamento |
| **Dual Filter Oscillator** (€1670) | Recriação do Filter/Oscillator da VCS3: Response leva de passa-baixa ressonante a passa-banda auto-oscilante (produz seno puro utilizável como fonte sonora); FM de corte por CV ou áudio com switch de modo de resposta; slope comutável 18/24dB/oct | Reforça a auto-oscilação já prevista (Joranalogue Filter 8, Fase 2b) e acrescenta: **slope comutável exposto ao Cérebro** (18 vs 24dB), não fixo em projeto — mais uma dimensão discreta de timbre modulável, complementando a "contagem de polos controlada por tensão" (Rossum Evolution) já no checklist |
| **Envelope Shaper / Noise Generator** (€1530) | VCA + envelope loopável (funciona como LFO) que controla o VCA **e** sai como CV simultaneamente; 4 entradas de áudio semi-normalizadas (mini-submixer embutido); gerador de ruído com "color" (tilt grave/agudo do ruído branco) agora controlável por tensão | A mesma envelope que molda a amplitude de um organismo sai, ao mesmo tempo, como CV disponível pros outros — um canal de **contágio audível** entre membros do cardume, distinto do `SchoolInfo::sharedDirection`/`sharedPhase` (que hoje só regem movimento, não amplitude/envelope). E o "color" de ruído VC-controlável liga o tom do leito de ruído ambiente (Noise Cinema, acima) a `UnderwaterVisibility::turbidity`/`Environment::temperature` — água turva ou fria escurece o timbre do fundo, não só a visão |
| **Dual Ring Modulator** (€1260) | Dois ring mods idênticos à VCS3 (soma/diferença de frequências); quanto mais complexa a forma de onda de entrada (dente-de-serra/pulso), mais metálico e discordante o resultado, por causa da relação harmônica mais rica | Refina a ring-modulação de contato predador/presa já proposta (EMW Ring Modulator, acima): a intensidade "metálica" da mordida passa a escalar com a riqueza harmônica das ondas envolvidas — mais wavefold/dente-de-serra do BiomaBass produz uma mordida mais violenta e dissonante; presas mais "puras" (senoidais) produzem um contato mais suave. Liga `strength`/quantidade de wavefold ao *caráter* do artefato sonoro, não só à sua presença |

### Teia Synthesizers — DriftLog (gerador de tensão aleatória cíclica)

*Manual completo (não só página de produto) — [DriftLog Manual, Teia Synthesizers](https://teiasynthesizers.com/wp-content/uploads/2026/06/DriftLog-Manual.pdf). Vale um tratamento mais longo que a tabela acima porque um único módulo concentra 6 princípios genuinamente distintos, vários deles diretamente sobre o núcleo já existente do conceito Bioma (`FluidMotion`, o Slew-Rate Random Walk do Cérebro).*

**Ficha técnica:** gerador de tensão aleatória de 2 canais, 12HP, Eurorack. `Slope` morfa a saída de degraus (stepped, tipo sample-and-hold) até glide contínuo entre valores; `Shape` (só atua quando Slope > 0) escolhe a família de curva do glide — Arco/Pico, Exponencial/Exponencial-inversa, ou Bezier (ease-in/ease-out) — um único controle cega entre os dois extremos da família escolhida, com o centro sempre linear. `Prob` define a chance de um novo valor ser gerado a cada passo de clock (pulsos de clock podem passar sem produzir mudança). O clock é global e compartilhado pelos 2 canais, nunca afetado pelos botões `Lock`; cada canal tem seu próprio `Lock`, que congela Slope/Shape/Prob **daquele canal**, deixando o outro livre — permite comportamentos de movimento contrastantes usando o mesmo conjunto de controles. Cada canal grava continuamente seu histórico recente de valores e pode entrar em `Loop`, reproduzindo uma janela ajustável (`Start`/`Length`) desse histórico — ajustável ao vivo durante a reprodução, sem alterar a sequência gravada nem travar Slope/Shape. As saídas lógicas (gate/trigger) são derivadas diretamente da própria polaridade da tensão aleatória (cruzar de negativo pra positivo gera o pulso), sem precisar de um estágio de limiar separado. Uma camada `Shift` dá função secundária aos mesmos controles físicos (ex.: `Prob` vira `Swing`, variação de tempo do clock). Uma função `Choke` estabelece hierarquia entre canais: o de número menor silencia os de número maior quando ambos estão em modo Choke. Salva/recupera estado entre power cycles.

**Apropriações originais** (múltiplos caminhos, mesmo espírito da segunda rodada da seção 9):

- **Dois eixos ortogonais pra caracterizar "como" o acaso se move**, em vez de um só: grau de suavização (Slope) × família de curva (Shape). Aplicado a `FluidMotion` no lugar do par fixo `std::sin`/`std::cos`, é mais rico que o banco de formas do VC Wavetable LFO (EMW, acima) — a MESMA curva de drift vai de degraus abruptos a glide contínuo, e dentro do glide ainda troca de família (arco, exponencial, bezier), dando à personalidade de movimento de cada `Species`/`SurvivalStrategy` dois graus de liberdade genuinamente independentes, não uma forma fixa escolhida de um banco.
- **Captura contínua + loop de uma janela ajustável do próprio histórico de aleatoriedade**, editável ao vivo sem travar a "textura" da curva. Diferente da memória de gesto humano (EMW Pot Action Recorder, acima) porque aqui não é um gesto gravado por alguém — é o próprio caminhar aleatório do organismo (ou do Cérebro) que vira loop. Um organismo pode "lembrar" um trecho recente da própria deriva e repeti-lo, com Start/Length ajustáveis ao vivo — candidato natural pra comportamento ritual em `Environment::isMatingSeason()`: em vez de continuar vagando livre, uma janela da deriva recente vira motivo repetido durante a época de acasalamento.
- **Eventos discretos derivados direto de um sinal contínuo já existente** (o próprio cruzamento de zero da tensão aleatória gera gate/trigger), sem estágio de envelope/limiar separado — técnica mais barata que o Envelope Follower (EMW, acima) pra gerar eventos: o `drift` que já sai de `FluidMotion::tick` pode disparar um evento sempre que cruza de negativo pra positivo num eixo, reforçando `matingCohesionBoost` ou uma transição de `PresenceState`, sem lógica nova.
- **Lock por canal com clock sempre global**: mecanismo barato pra um organismo "congelar" a própria assinatura de movimento (Slope/Shape/Prob) enquanto o resto do sistema continua variando ao redor dele. Complementa `TerritorialStrategy` (que resiste espacialmente) com resistência na própria TEXTURA do movimento, não só na posição.
- **Choke — hierarquia de prioridade entre canais** — candidato concreto de implementação pra uma preocupação que já estava só em aberto (seção 1 e 10, "legibilidade sonora"): quando duas filosofias de mixagem (ou dois organismos) disputariam prominência ao mesmo tempo, uma regra de prioridade simples e hierárquica resolve o conflito em vez de deixar as duas se sobrepuserem sem controle.
- **Camada Shift — segunda função pro mesmo controle físico** (ex.: Prob vira Swing): mecanismo de UI reaproveitável pros poucos controles de `Desire`/`philosophyBlend` na GUI real — dobra a densidade de controle sem multiplicar knobs, relevante pra questão já em aberto de como a Lupa e o resto dos parâmetros vão se materializar na GUI (Fase 9 e 11 do checklist).

### SchneidersLaden — catálogo geral (não curado, ~111 páginas)

*Diferente da EMW/Soundfreak/Teia (listas fechadas, já totalmente percorridas), a [SchneidersLaden](https://schneidersladen.de/en/eurorack-modular-3u) é uma loja com catálogo enorme, ordenado por "novo/em alta" — não por relevância. Esta rodada cobriu só a primeira página (24 itens), triando o que tinha ficha técnica real e não era puro utilitário (cabos, cases, parafusos). Três achados com apropriação genuinamente nova, mais um confirmando caminhos já registrados.*

| Módulo real | Ficha técnica | Apropriação original |
|---|---|---|
| **Shaka-8** (SetonixSynth, €265, 12HP) | Matriz estéreo 4x2 totalmente analógica — 4 entradas estéreo pra 2 saídas estéreo, cada um dos 8 pontos de cruzamento com seu próprio VCA estéreo (chips SSI2164, ganho aplicado por lado pra preservar a imagem estéreo) e CV dedicado; dá duas mixagens independentes das mesmas 4 fontes ao mesmo tempo; expansível via Shaka-4 (cada um soma uma coluna de saída, até 4x3/4x4+) — "o próprio roteamento vira mais um parâmetro patcheável" | O `philosophyBlend`/`panLeft`/`panRight` de `Ecosystem` hoje é uma matriz 4×2 **fixa** (constantes hardcoded). Shaka-8 sugere torná-la uma matriz real e explícita, sob CV — e, mais importante, produzir **duas (ou mais) mixagens estéreo simultâneas e independentes** das mesmas 4 filosofias, não só uma saída final. Uma "mix de perto" e uma "mix de longe" das mesmas 4 vozes ao vivo, por exemplo, em vez de um único par estéreo definitivo |
| **FLUME** (earthlink modular, €416, 12HP) | Interface de send/return estéreo pra pedais de efeito externos: três pontos sob CV (Send, Return, Mix — cada um um VCA estéreo discreto), conversor CV→expressão com chave de polaridade pra varrer o controle de expressão de um pedal, LFO triangular embutido normalizado pra Mix e Exp, saída 9V isolada pra alimentar o pedal direto | Precedente concreto de hardware pra formalizar a brecha externa já cogitada em `AquariumIntegrity`: em vez de um simples on/off de áudio externo, **três pontos de controle separados** (quanto sai, quanto volta, quanto se mistura) amarrados a `crackStress` — o aquário trincado controla o quanto "vaza" pra fora e o quanto volta, com seu próprio crossfade, não um buraco binário. A ideia do CV-pra-expressão também sugere o caminho inverso: `Desire`/BrainEngine varrendo o parâmetro mais importante de um pedal real que Lúcio já tenha, puxando hardware físico pra dentro do patch |
| **4ms LEQA** (€349, 14HP) | Processador de fim de cadeia combinando compressor ótico (dois ratios, 3:1 e 10:1), EQ (shelf grave/agudo + sino médio + HPF, roteável antes/depois do compressor ou em bypass) e processador de largura estéreo (estreita, alarga ou "super estéreo"; mono→estéreo via truque tipo Haas; largura sob CV) | Enriquece a Fase 7 (Master) com três ideias concretas: EQ roteável antes/depois do compressor como **escolha exposta**, não ordem fixa; largura estéreo sob CV amarrada a `AquariumIntegrity::crackStress` — o mix **estreita espacialmente conforme o aquário trinca**, complementando o "vazamento" de posição que `worldPosition` já produz, agora como colapso de largura estéreo; e um estágio de compressão ótica mais suave ("cola") antes do limiter com lookahead já planejado, personagem diferente, não substituto |
| **MSW-820** (Michigan Synth Works, €479, 46HP) | Monosynth analógico completo, 2 VCOs AS3340 + filtro AS3109 de 16 modos (linhagem Roland SH-101/MC-202); botão "THIS" liga a triangular do VCO1 direto no corte do filtro em taxa de áudio, sem precisar patchear; os mesmos 3 controles físicos do LFO (Rate/Fade/Shape) editam também o gerador de ruído e um terceiro oscilador digital opcional, trocando de alvo por switch | Confirma dois caminhos já registrados em vez de abrir um novo: reforça o thru-zero FM do BiomaBass (Fase 2/2b), mas como lição de UX — caminhos de auto-modulação comuns merecem um knob dedicado embutido, não exigir patch virtual; e confirma que a camada Shift do DriftLog (acima) escala além de 2 alvos — aqui os mesmos 3 controles editam **3 sistemas diferentes**, não só uma função primária/secundária |

### SchneidersLaden — páginas 2-5 (mais ~100 itens, mesma triagem)

*Três achados fortes o bastante pra ficha completa, mais um punhado de reforços breves. Muitos módulos dessas páginas já eram conhecidos da pesquisa (Just Friends, Three Sisters, Mangrove, RANDOM8, Fractalist, Glowing Orb, Déjà Vu, Kamieniec II) — sinal de que a triagem das seções 9-10 já tinha mirado certo, não é redundância vazia.*

| Módulo real | Ficha técnica | Apropriação original |
|---|---|---|
| **Silhouette** (Mannequins, €529, 10HP) | Combinador analógico de 6 canais organizados num campo circular. O controle SPOT varre continuamente esse campo, selecionando 1+ entradas adjacentes como "primeiro plano" (saída LIGHT direta + SHADOW através de delay BBD analógico com filtro variável); crossfade suave entre vizinhos, nunca troca discreta. **Todo o resto forma automaticamente o "fundo"**, redistribuído num par estéreo que se rearranja continuamente conforme o SPOT se move. O fundo tem 3 modos dinâmicos: VCA simples, ducking tipo sidechain **disparado pelo próprio primeiro plano**, ou expansor descendente. BLUR é um processador de ambiência digital separado. O delay não tem feedback interno de propósito — incentiva patchear a saída SHADOW de volta pra fora, até auto-oscilação | É a peça de hardware que mais diretamente ataca a "legibilidade sonora" (questão em aberto desde a seção 1, reforçada na seção 10): um mecanismo real de **primeiro-plano-varre-automaticamente-o-campo, resto vira fundo redistribuído**, não uma escolha binária. Isso sugere uma **segunda camada de foco**, complementar (não substituta) à Lupa: enquanto a Lupa é escolha humana manual e local (um ponto no vidro), um "SPOT" no nível do `Ecosystem` varreria automaticamente qual organismo(s) está em primeiro plano a cada instante, com o resto sendo redistribuído (não silenciado) em estéreo — e o modo de ducking-pelo-primeiro-plano é tecnologia pronta pra "quem está em foco empurra o resto pra trás", em vez de só region de peso independente por `MixPhilosophy` |
| **Scrooge** (Neutral Labs, €628, 42HP) | "Gerador de malfuncionamento sequenciado" — 5 vozes analógicas que **não têm fonte de alimentação própria pros circuitos de som**: roubam energia dos próprios sinais de controle do sequenciador (tensão de passo, forma, probabilidade e timing viram parte literal do som). Resultado: caráter instável, "faminto" — cliques, crepitações, estouros instáveis, "detrito analógico". Sequenciador completo por trás (parameter locks, até 64 passos, encadeamento de padrões, variações generativas) | Literaliza uma ideia que o conceito Bioma só tinha de forma abstrata: **organismos disputando um recurso compartilhado limitado**. Em vez de cada `Organism` sempre ter energia plena independente da carga do sistema, o "poder" sonoro disponível por organismo passa a depender da contenção em tempo real — quanto mais denso o aquário (`Desire::density` alto, muitos organismos ativos ao mesmo tempo), menos "alimentação" sobra pra cada voz individual, e o resultado é degradação audível (instabilidade, crepitação, envelope incompleto) **precisamente quando o ecossistema está mais cheio** — não um limitador de CPU escondido, uma consequência ecológica audível de densidade alta, no mesmo espírito de capacidade de suporte biológica |
| **Eddy** (Schlappi Engineering, €309, 8HP) | Submixer estéreo de 3 canais com compressor de feedback embutido e estágio de distorção agressivo — mas o pulo do gato: **sem nada patcheado na entrada esquerda de um canal, esse canal é normalizado pra um barramento de feedback** passando por 3 filtros passa-banda fixos (grave/médio/agudo) depois do estágio de distorção. Virar uma chave faz o canal se alimentar de si mesmo — território de "no-input mixing": ruído e percussão auto-oscilantes **saindo só do mixer**, sem osciladores | É a técnica concreta pra uma possibilidade que a filosofia "sistema fechado" do Aquorbium já sugeria mas nunca tinha resolvido: **o aquário pode soar mesmo sem organismo nenhum dentro**, puramente por realimentação estrutural. Candidato natural pro momento em que `AquariumIntegrity::shattered` se torna verdadeiro — em vez de silêncio ou de um efeito sonoro genérico de "quebra", o barramento master, roteado por 3 bandas fixas + distorção + compressor e realimentado nele mesmo, literalmente **resmunga sozinho** a partir da própria estrutura vazia do aquário quebrado |

Reforços breves (sem ficha nova, confirmam ou enriquecem apropriações já registradas): Make Noise **GTE** ("Gestural Time Extractor" — extrai ritmo/pulsos de qualquer forma de onda em movimento) generaliza o cruzamento-de-zero do DriftLog pra "qualquer gesto de CV vira ritmo", não só sinais aleatórios — reforça o item de eventos discretos a partir de sinal contínuo (Fase 4). Toppobrillo **Matrixplexer** (matriz 4x4 com "modos de animação independentes por eixo") enriquece a apropriação do Shaka-8 (Fase 8): a matriz de `philosophyBlend` poderia "escanear" sozinha, com comportamento de varredura diferente por eixo (organismos vs. saídas). WMD **Channel Surfer** (channel strip com ducking por sidechain embutido) reforça a ideia de duck predador→presa já lógica pelo ecossistema, mas no nível de canal, não só de evento pontual.

### SchneidersLaden — páginas 6-15 (mais ~240 itens, mesma triagem)

*Essas dez páginas foram dominadas pelo catálogo numerado da ADDAC Systems (dezenas de módulos das séries 200/300/400/500/700/800), majoritariamente utilitário (attenuverters, multiples, adaptadores de pedal, roteadores) — mas com quatro exceções que abrem princípio genuinamente novo, e que por isso ganharam ficha completa e fetch da página individual do produto. Fora a ADDAC, o resto das páginas foi principalmente reforço do já mapeado (osciladores, filtros, VCAs em variações previsíveis).*

| Módulo real | Ficha técnica | Apropriação original |
|---|---|---|
| **ADDAC708 — Michel Waisvisz's Cracklebox** (€179, 12HP) | Reedição em formato modular de um instrumento histórico do STEIM (Amsterdã, fim dos anos 1960): um circuito de brinquedo com pastilhas sensíveis ao toque — encostar o dedo entre duas pastilhas *completa o circuito através da própria resistência da pele*, gerando um som crepitante imprevisível, literalmente diferente a cada toque e a cada pessoa. Tem bateria 9V isolada (protege o usuário e isola o sinal da fonte do Eurorack) e um knob "Power Starve" que reduz a tensão de alimentação do circuito, mudando timbre/altura. Saída utilizável como áudio, gate ou CV | É a peça de hardware mais literal pra "som que nasce do contato, não do patch": um evento sonoro cuja identidade real depende de uma variável que o instrumento *não pode controlar* (a resistência da pele de quem toca). Sugere um modo de interação do GUI em que o Observer toca (ou arrasta entre) dois pontos do vidro simultaneamente — não pra ver (isso é a Lupa), mas pra *provocar* um evento sonoro cujo timbre exato depende de um ruído de entrada genuinamente não determinístico (jitter do próprio dispositivo de entrada, por exemplo), nunca reproduzível de forma idêntica. O "Power Starve" reforça, por uma tradição de circuito diferente da densidade populacional já registrada (Scrooge, rodada anterior), a mesma ideia geral: **reduzir energia disponível degrada o timbre de forma característica**, não só silencia |
| **ADDAC405 — VC Relabi Generator** (€315, 10HP) | Implementa o conceito "Relabi" cunhado por John Berndt (2009): "a experiência de uma pluralidade coordenada de eventos que parece cíclica mas simultaneamente impede a identificação de um padrão uniforme — parece pulsada e não-pulsada ao mesmo tempo". Tecnicamente: 4 LFOs senoidais internos (cada um com ganho, frequência e mute próprios) somados geram a "Relabi Wave", uma fonte de CV bipolar complexa (±5V) utilizável tanto como gate/trigger quanto como modulação | Terceira via pra geração rítmica do `Ecosystem`, distinta tanto do puro aleatório (DriftLog) quanto do puro periódico: somar 4 osciladores lentos e simples de frequências não relacionadas produz uma sensação de pulso **sem nunca cristalizar num padrão único identificável** — presença cíclica sem previsibilidade mecânica. Encaixa como opção de geração de tempo pro BrainEngine (Fase 4): em vez de escolher entre "regular" e "caótico", os organismos de um cardume (`SchoolInfo`) poderiam ter seus instantes de decisão movidos por uma soma de LFOs de fase relativa lentamente derivante — parecem se mover junto, mas nunca em uníssono mecânico |
| **Hieroglyphic Opalina** (€335, 14HP) | Rede de filtro estéreo analógico state-variable com três modos: filtragem convencional, modo **Resonator** (ressonância alta, quase-osciladora) e modo **Isolator** — nesse último, em vez de atenuar gradualmente o que está fora da banda escolhida, a banda selecionada é isolada por completo, deixando o resto efetivamente mudo, não só atenuado | É o par direto da Lupa, mas no domínio da frequência em vez do espaço: enquanto a Lupa isola nitidez num ponto do vidro e borra o resto (espacial), o modo Isolator isola nitidez numa faixa espectral e silencia o resto (espectral). Vale registrar como direção de expansão explícita pra seção 1/11: uma "Lupa de frequência" complementar — focar não *onde* no aquário, mas *em qual banda espectral* do que já está em foco, aplicada por exemplo a um organismo individual selecionado pela Lupa espacial, empilhando os dois focos |
| **Rides In The Storm — BOC MK2** (€195, 12HP) | VCO/LFO analógico discreto com quatro saídas de sub-osciladores em intervalos musicais (2/3, 3/4, 1/2, 1/4) e uma saída **Gravity**, que combina a triangular principal com o sub 3/4 via **lógica XOR** — produz uma aspereza tipo pseudo-ring-mod, diferente de multiplicação analógica pura | Irmão "digital-lógico" da já registrada apropriação do Ring Modulator (EMW/Soundfreak, seção 12): em vez de multiplicar analogicamente os sinais de dois organismos em contato (predador/presa), combiná-los por **XOR** (comparação de sinal/polaridade, não de amplitude) produz uma textura de contato mais áspera e "quantizada" — uma segunda variante de evento-de-contato pro `Ecosystem`, escolhida por `SurvivalStrategy` (XOR pra Predator, multiplicação analógica pra Territorial, por exemplo) em vez de um único algoritmo de contato fixo |

Reforços breves (sem ficha nova ou já cobrindo terreno conhecido): apf.audio **Tiliqua** (plataforma FPGA reconfigurável e *open-source*, "shapeshifter" multitool pra Eurorack) ressoa com o próprio espírito AGPLv3 do Aquorbium, mas como filosofia de projeto, não como técnica de apropriação específica. ADDAC **300 Power Starvation** e Blood Cells Audio **Mangle** (destruição de áudio por privação de energia) reforçam, por hardware dedicado, a mesma ideia já anotada via Scrooge: degradação sonora characterizada por falta de energia, candidata a acompanhar `AquariumIntegrity::crackStress`. ADDAC **308/309/310/311** (luz, pressão, pedal de expressão e piso convertidos em CV) reforçam a `Environment` já existente em `Ecosystem.h` — sensores físicos reais como fonte de entrada externa, não só osciladores internos. Magerit **Laniakea** ("cluster voicing" — um oscilador que gera agrupamentos de vozes) reforça o conceito já presente de `SchoolInfo` — cardume como voz única com múltiplas cabeças, não uma ideia nova.

### SchneidersLaden — páginas 16-25 (mais ~240 itens, mesma triagem)

*Dez páginas seguintes, mesmo critério. Dois achados com princípio genuinamente novo — um de endereçamento de sequência, outro de síntese —, mais reforço do tema de sensor físico já registrado via ADDAC 308-311.*

| Módulo real | Ficha técnica | Apropriação original |
|---|---|---|
| **New Systems Instruments Discrete Map** (€555, 34HP) | Seletor de passo **não-sequencial disparado por limiar**: em vez de avançar um índice fixo por pulso de clock, cada passo carrega seu próprio limiar, e um sinal de entrada arbitrário e contínuo (não precisa ser um clock) decide qual passo soa a seguir, conforme cruza o limiar daquele passo — "acaso controlável" derivado de qualquer sinal, não de um gerador de aleatoriedade dedicado | Terceiro modo de endereçamento pro `GeometricSequencer`, ao lado do sequencial fixo e do endereçamento direto por CV já no checklist (princípio Joranalogue Step 8): o próximo passo não é escolhido por incremento nem por CV direto de índice, mas por **qual limiar um sinal de entrada cruza** — literaliza posição no aquário, `Desire::density` ou proximidade de um predador como o próprio "clock" reativo que decide o próximo passo. Mesmo princípio aplicável a `SurvivalStrategy`: a próxima transição de comportamento de um `Organism` escolhida por qual limiar de um estímulo (nível de perigo, distância do predador) é cruzado, não por uma máquina de estados determinística fixa |
| **Hieroglyphic Plume Pulsar Generator** (€388, 18HP) | Implementa **síntese de pulsar** (Curtis Roads) — técnica de microssom a meio caminho entre oscilador, granulador e gerador de clock: pulsos curtos sintetizados (`pulsaret`) seguidos de silêncio, cuja proporção entre pulso e silêncio varre continuamente de tom puro a textura tipo ruído/densidade de grão. 7 formas de onda pro pulsaret, wavefolder de linhagem West Coast, mascaramento estocástico (pulsos individuais podem ser aleatoriamente suprimidos), controles de Formant e Cluster | Novo arquétipo `Species::Pulsar`, entre o `Species::Oscillator` (wavetable) e o granular por-grão já planejado (Fase 5b): pulsos curtos sintetizados cuja razão pulso/silêncio deriva de `Desire::density` ou `AquariumIntegrity::crackStress` — aquário calmo e denso produz pulsos agrupados quase tonais, aquário tenso ou trincado espaça os pulsos até virarem cliques isolados. O mascaramento estocástico é a mesma ideia de supressão probabilística do `Prob` do DriftLog (seção 12), mas em **taxa de áudio** em vez de taxa de controle — borra ainda mais a fronteira oscilador/sequenciador já generalizada via Soundfreak Triple VCO. O controle Cluster (agrupar vozes de pulsar com relação de formante ajustável) é um segundo caminho de hardware pra "voz única com múltiplas cabeças" de `SchoolInfo`, dessa vez no nível interno de um oscilador, não entre organismos separados |

Reforços breves (sem ficha nova, mesmo tema já registrado via ADDAC 308-309-310-311): KOMA Elektronik **Sensei** ("The World as a Modulator" — interface dupla de sensor de luz, movimento e toque) e a linha 1U da KOMA (UFO, Detective, FM Radius, KMNDR infravermelho), além da Bela **Gliss 1U** (controlador de toque + gravador de movimento), reforçam de novo `Environment` como ponto de entrada de sensores físicos reais — nenhum princípio novo em relação ao já registrado, mas confirmam que o caminho é uma família inteira de hardware, não um caso isolado.

### SchneidersLaden — páginas 26-45 (mais ~480 itens, mesma triagem)

*Vinte páginas seguintes (26 a 45), mesmo critério — a faixa mais longa percorrida numa única rodada até agora. Dominada por variações previsíveis de VCA/mixer/filtro/case já cobertas por tarefas existentes, mas com três achados que abrem princípio genuinamente novo o bastante pra fetch da página individual do produto, todos ligados por um mesmo fio: formas distintas de trazer processos "emprestados" de outros domínios (linguagem de programação ao vivo, instrumentação de laboratório de física, aprendizado de máquina) pro Eurorack sem virar clone de nada existente.*

| Módulo real | Ficha técnica | Apropriação original |
|---|---|---|
| **Emute Labs uSeq** (€335, 8HP) | Módulo híbrido digital/analógico de **live coding**: conecta via USB a um editor rodando no navegador (ou offline), programado numa linguagem própria simplificada baseada em LISP ("modulisp"), reprogramável ao vivo durante a performance. 2 entradas de CV, 2 entradas de pulso, 3 saídas de CV, 3 saídas de pulso, chave momentânea, chave de três posições, dois atenuadores-inversores dedicados às entradas de CV. Hardware e software **open source** | Sugere uma camada de comportamento **reescrevível em tempo real** pro `Ecosystem`, sem recompilar: não um DSL completo tipo modulisp dentro do Aquorbium, mas o princípio de expor um pequeno conjunto de regras (ex.: a fórmula que define transição de `SurvivalStrategy`, ou a curva que pondera `MixPhilosophy::philosophyBlend`) como texto editável ao vivo via GUI/APVTS, carregado e trocado sem parar o áudio — um "modo de regra aberta" ao lado do comportamento fixo já compilado, coerente com o espírito "documento vivo" do próprio projeto. Also reforça, por analogia direta, a natureza aberta (AGPLv3) do próprio Aquorbium — não como técnica de apropriação sonora, mas como parentesco de filosofia de projeto |
| **Joranalogue x Hainbach Collide 4** (€639, 20HP) | Reencarnação em Eurorack de um **amplificador lock-in** — instrumento de laboratório de física nuclear pra extrair um sinal específico enterrado em ruído, sincronizado a uma referência. Filtro de largura de banda variável **pingável e auto-oscilante**, estágios de ganho antes/depois do filtro (até 130dB no total), oscilador seno/cosseno em quadratura com through-zero, rede de **transformada de Hilbert** (frequency shifter), dois moduladores em anel com filtros passa-baixa na saída, seção de saída com computação analógica de magnitude e fase, topologia X/Y de quadratura dedicada a uso estéreo, CV sobre tudo com compensação de temperatura e rastreio V/oitava | Dois caminhos distintos, um por técnica: (1) o **Hilbert transform** como frequency shifter é tecnicamente diferente de pitch-shift (desloca todas as parciais pelo mesmo Hz, não pela mesma razão — quebra a relação harmônica de propósito) — candidato a uma nova variante de `Species` ou de processamento do BiomaBass, produzindo inarmonicidade controlável por CV, distinta do wavefolder/thru-zero FM já planejados; (2) o **princípio do lock-in em si** (extrair seletivamente um sinal específico da mistura, sincronizado a uma referência) sugere um terceiro modo de foco pro Observer, ao lado da Lupa (espacial) e do modo Isolator do Opalina (espectral, seção 12 rodada 6-15): uma "Lupa sincronizada" que tranca numa referência de frequência/fase pertencente a UM organismo específico (ex.: sua própria taxa de `FluidMotion`) e extrai só a contribuição sonora dele da mixagem inteira, mesmo com todos os outros organismos tocando ao mesmo tempo — foco por sincronismo, não por posição nem por banda fixa. A topologia X/Y de quadratura também é candidata concreta pra rotação estéreo contínua de `AquariumTransform`, usando os pares seno/cosseno como fonte direta de pan em vez de crossfade linear |
| **Robaux Musique DCSN3** (€99, 12HP) | Gerador de ritmo/gate aleatório baseado em **árvore de decisão** (o próprio fabricante evoca Machine Learning/IA): um gate/trigger de entrada é roteado pra 1 de 3 saídas primárias, e dali pra 1 de 3 sub-saídas — 12 saídas no total, em duas camadas de ramificação. Um único knob ("Random to Loop") varre continuamente de resultados totalmente caóticos a um padrão fixo de loop de 16 passos. Modo **Latch** sustenta o sinal até o próximo gate, combinável com um filtro multi-modo pra variar o timbre por ramo. Também funciona como divisor de clock (três tipos, 1/2 a 1/16 nas 4 saídas superiores, sequência contínua nas 8 inferiores) | Complementa (não substitui) o Discrete Map da rodada anterior (seção 12, páginas 16-25): em vez de um único limiar sobre um sinal contínuo, uma **árvore de decisão ramificada em camadas** — primeira camada decide categoria ampla, segunda camada refina dentro da categoria escolhida. Mapeamento direto pra `SurvivalStrategy`: a primeira ramificação decide entre famílias de comportamento (territorial/migratório/predador-presa), a segunda refina a variante específica dentro da família escolhida, em vez de uma máquina de estados plana com todas as transições no mesmo nível. O knob "Random to Loop" é um controle contínuo concreto e barato pro eixo `ChaosPrecision` do `MixPhilosophy` — de comportamento imprevisível a um ciclo fixo repetido, um único parâmetro, não uma escolha binária. E o modo Latch (sustentar até o próximo gate, com timbre variável por ramo) sugere um mecanismo de "sustentação de estado" pra `PresenceState`: um organismo mantém sua postura comportamental atual até o próximo evento ecológico relevante cruzar a árvore, com o timbre mudando por qual ramo está ativo no momento, não por um novo estado imediato a cada instante |

**Observados por nome/categoria, sem página individual aberta nesta rodada** (não custa registrar a direção, mas sem ficha técnica verificada não vira apropriação): ADDAC **507 Random Bezier Waves** (nome sugere curvas CV aleatórias com easing tipo bezier — reforçaria o eixo Shape do DriftLog, seção 12) e ADDAC **508 Swell Physics** (nome sugere modelagem física de "inchaço"/crescimento — reforçaria o princípio já registrado de degradação/comportamento por consequência física, tema Scrooge/Power Starvation); Noise Engineering **Gamut Repetitor** (nome sugere repetição/loop de padrão — reforçaria a captura-e-loop do DriftLog); Expert Sleepers **Otterley**; e a linha sensor-driven da Flame (**Ton** Melody Looper, **Takt** Rhythm Sequencer, **Airlectron** infravermelho) — reforçariam de novo `Environment` como entrada de sensor físico, mesma família já confirmada via KOMA/Bela na rodada anterior. Nenhum desses ganhou fetch de página individual nesta rodada; ficam anotados como candidatos, não como achados verificados.

Últimas duas páginas da faixa (44-45) não renderam achado novo nem reforço distinto — predominância de compressores/EQs de canal, cases, clones diretos de circuito histórico (Toppobrillo Cluster = reinterpretação do Buchla 257, já teria contado como clone se fosse produto próprio do Aquorbium) e módulos com descrição de fabricante vaga demais pra virar apropriação verificável (Ritual Electronics Amnis: "shift register com alguns ases na manga", sem detalhe técnico real).

### SchneidersLaden — páginas 46-65 (mais ~480 itens, mesma triagem)

*Vinte páginas seguintes (46 a 65). Trecho dominado por clones extensos (a série After Later Audio de clones Mutable Instruments ocupa boa parte destas páginas), reedições de circuito histórico (Tiptop série Buchla, filtros clássicos G-Storm Electro) e utilidade pura (cases, fontes, cabos, painéis cegos, VCAs e mixers genéricos) — mas com quatro achados de princípio genuinamente novo, três deles vindos do mesmo território (síntese e sequenciamento por controle multidimensional/compartilhado) e o quarto abrindo uma frente inteiramente nova pro projeto: espacialização binaural real.*

| Módulo real | Ficha técnica | Apropriação original |
|---|---|---|
| **RYK Modular Vector Wave** (€438, 17HP) | **Síntese vetorial**: 16 osciladores organizados em 4 bancos de 4, misturados continuamente por uma posição 2D controlada por joystick — mesmo princípio do Prophet VS/Korg Wavestation, mas com o joystick físico substituindo o envelope vetorial pré-programado. A posição do joystick também pode ser **gravada como CV** e reproduzida depois, repetindo o gesto de mistura original | Precedente concreto de hardware pra **mistura contínua entre 4 fontes usando uma coordenada 2D**, não 1D — sugere um modo de `MixPhilosophy` onde `philosophyBlend` deixa de ser só um eixo (ou par de eixos independentes) e passa a ser uma posição XY real amostrando as 4 filosofias como se fossem os 4 bancos do Vector Wave, com blend contínuo nos quatro "cantos" simultaneamente. Separadamente, a **gravação do gesto do joystick** é uma terceira variante de captura-de-movimento humano, ao lado do EMW Pot Action Recorder (1 eixo) e do loop do DriftLog (deriva no tempo) — aqui especificamente um **trajeto 2D**, o que mapeia diretamente pro deslizar da Lupa sobre a superfície esférica: gravar o caminho que `MagnifyingLens::glideTowards` percorreu e devolver como uma varredura autônoma repetível, distinta de um observador humano controlando ao vivo |
| **Tiptop Audio Z-8000 Matrix Sequencer** (€368, 28HP) | **Sequenciamento por pool compartilhado**: 10 canais de sequência de CV independentes (quatro trilhas horizontais de 4 passos, quatro verticais de 4 passos, uma horizontal de 16 passos, uma vertical de 16 passos) — mas todos os 10 canais leem valores de um **pool compartilhado de apenas 16 potenciômetros** físicos, não de memória própria por canal. Mexer em um único knob pode alterar várias sequências ao mesmo tempo, dependendo de quais trilhas compartilham aquela célula da matriz | Princípio estruturalmente distinto de tudo já registrado nas Fases 3/4: em vez de cada canal/organismo ter armazenamento de parâmetro independente, múltiplos `GeometricSequencer` (ou múltiplos organismos do mesmo `schoolId`) podem **ler de um pool pequeno e compartilhado de valores subjacentes**, de forma que ajustar um único parâmetro do Cérebro se propaga simultaneamente por várias sequências/organismos ao mesmo tempo — modela interdependência ecológica (recurso ambiental compartilhado, não memória privada por peixe) diretamente na engine de sequenciamento, em vez de simular a interdependência só via `SchoolInfo::sharedDirection`/`sharedPhase` depois do fato |
| **Xaoc Devices Sofia** (€489, 24HP) | **Síntese de formante FOF estendida**: núcleo de tom com core triangular gera pares de elementos de "ondulação" (ripple) a cada ciclo, derivados da fundamental e **harmonicamente travados** a ela (não livres pra dessintonizar), com forma de onda/decaimento/densidade modulável, e **auto-patcheável** (a própria saída pode realimentar os parâmetros de ondulação) pra timbres estendidos | Complementa, não substitui, o `Species::Pulsar` já registrado a partir do Hieroglyphic Plume (seção 12, páginas 16-25): onde o pulsar do Plume pode dessintonizar livremente das parciais da fundamental (textura metálica/atonal), as ondulações da Sofia ficam **travadas harmonicamente** de propósito — sugere uma segunda variante dentro de `Species::Pulsar` (ou um archetype irmão), dando dois registros texturais distintos a partir da mesma lógica de "trem de pulsos por ciclo": um atonal/percussivo (já coberto), um vocálico/formante e tonal (novo). O **auto-patch** da Sofia (sua própria saída realimentando os próprios parâmetros de ondulação) é candidato concreto a um loop de auto-feedback *dentro* de um único organismo, distinto do cross-feedback entre organismos que já é o princípio central do `Ecosystem` |
| **Neuzeit Instruments Quasar** (€479, 14HP) | Primeiro **mixer binaural 3D** do Eurorack, segundo o próprio fabricante: 2 entradas mono roteadas por matriz interna pra até 3 posições (POS A, POS B, CNTR), cada posição ajustável em ângulo/altura/distância manualmente ou por CV, processadas por algoritmo que emula a forma da cabeça e do pavilhão auditivo (HRTF) — cria imagem sonora 3D real ao redor da cabeça do ouvinte, funciona melhor em fones. Filtros digitais passa-alta/passa-baixa por posição decidem quais faixas de frequência vão pra qual posição; a posição CNTR (centro) distribui igualmente pros dois ouvidos **sem** processamento HRTF, mantendo o grave estável na base da mixagem enquanto os agudos "dançam" ao redor da cabeça. Delay estéreo (3ms–20s) e reverb embutidos. Dois encoders sem fim com anéis de LED RGB, display OLED, CV flexível roteável pra qualquer parâmetro com múltiplos destinos por CV | Apropriação mais direta desta rodada: é precedente de hardware real pra exatamente o que o nome "Aquorbium" já promete — posicionar som ao redor de uma cabeça virtual usando ângulo/altura/distância, não só pan estéreo. Sugere um estágio de espacialização real tipo HRTF pra cada `Organism` (ou pro mix final de cada `Observer`), com o ouvinte virtual situado no centro do aquário esférico ou na posição de um dos 4 `Observer`s — CV-controlável a partir de `FluidMotion`/`worldPosition`, então o movimento natural de um peixe já move sua posição binaural continuamente, sem controle manual extra. A decisão de deixo o grave estável num canal central sem HRTF (só os agudos "dançando" ao redor da cabeça) é diretriz técnica concreta pra não destruir a fundação de baixo do BiomaBass ao espacializar — grave fica ancorado, harmônicos altos se movem |

**Observado por nome, insuficiente pra virar apropriação:** Five12 Vector Sequencer — nome sugere parentesco conceitual com o Vector Wave acima (síntese/sequência vetorial), mas a descrição do fabricante na página estava truncada demais pra extrair ficha técnica real; sem fetch de página individual, fica só como candidato anotado, não achado verificado.

Páginas 62-65 não renderam achado novo além dos quatro acima — dominadas por adaptadores, mixers/VCAs utilitários, fontes/decksavers/cases, e reedições/variações já cobertas por tarefas existentes (Doepfer A-126-2v Frequency Shifter reforça, sem novidade de princípio, o Hilbert transform já registrado via Collide 4 na rodada anterior).

### SchneidersLaden — páginas 66-85 (mais ~400 itens, mesma triagem)

*Vinte páginas seguintes (66 a 85). Trecho ainda mais dominado por utilidade pura (fontes, cases, cabos, buffers, multiples, VCAs/mixers genéricos) e reedições de circuito clássico (Doepfer A-120/A-106-5, AJH Fixed Filter Bank 914) do que a rodada anterior — mas com três achados de princípio genuinamente novo (gerência de cena de modulação com morphing entre tipos de algoritmo, processador 4x4 com gravação de CV mutante, e uma técnica de síntese inteiramente nova batizada pelo próprio fabricante) e um reforço breve de um princípio já registrado.*

| Módulo real | Ficha técnica | Apropriação original |
|---|---|---|
| **Shakmat Modular Harlequin's Context** (€279, 12HP) | **Modulation Preset Manager**: 4 canais de CV configuráveis individualmente para um de 15 algoritmos de modulação (offset, LFO, random, envelope, serial), com 14 "sets" diferentes definindo como o módulo reage às entradas de CV/Gate. Os estados dos 4 canais são salvos em **16 cenas**, selecionáveis manualmente, endereçáveis por CV ou sequenciáveis por clock. Memória não-volátil de **16 bancos de 16 cenas** (256 cenas no total). Tem um **modo de morphing** que interpola entre cenas com *tipos de algoritmo diferentes* entre si (ex.: LFO morphando pra random, não só um LFO mudando de frequência). Compatível com o Select Bus (Make Noise) como transmissor ou receptor | Precedente de hardware pra guardar, recuperar e **fazer o morph entre configurações comportamentais inteiras**, não só entre valores de parâmetro — distinto do "modo de regra aberta" do uSeq (live coding do algoritmo em si) já registrado a partir da rodada 26-45: aqui a ênfase é banco de cenas pré-preparadas, endereçáveis ao vivo. Sugere um **banco de cenas de comportamento** no `Ecosystem` — capturar e nomear configurações inteiras de `MixPhilosophy`/`SurvivalStrategy`/`Desire` de um organismo (ou de um cardume via `SchoolInfo`) como uma "cena", armazenar várias, e recuperá-las manualmente, por CV/parâmetro externo ou sequenciadas no tempo. O ponto mais difícil (e mais interessante) de portar é o **morph entre tipos de algoritmo diferentes**: como `MixPhilosophy` é um enum discreto (ChaosPrecision/StructuralMutation/GeometricPermutation/OrganicDrift), não dá pra fazer lerp numérico direto entre duas cenas de filosofias diferentes — é preciso um estágio de transição que cross-fade as *saídas* das duas filosofias durante a janela de morph, do jeito que o Harlequin's Context deve fazer internamente pra ir de um LFO senoidal a uma sequência randômica sem salto abrupto |
| **u-he CVilization** (€328, 10HP) | **Processador/gravador/mutador de CV 4x4** com quatro modos principais selecionáveis no mesmo hardware físico: (1) Matrix Mixer 4x4, (2) Sequential Switch 4x4, (3) **gravador de CV quádruplo com capacidade de mutação**, (4) painner quadrofônico. Os três primeiros modos têm utilidades embutidas (Sample & Hold, Glide, Quantizer). A função de mutação no modo gravador de CV é descrita pelo próprio fabricante/loja como "an exciting innovation in Euroland" — o loop gravado não se repete de forma idêntica, mutando a cada passagem | Dois princípios originais separáveis. Primeiro, a **função de mutação no gravador de CV** é precedente concreto de hardware pra um loop capturado que **não repete exatamente** — relevante direto pro DriftLog (captura-e-loop já registrado nas fases anteriores) e pro playback de gesto gravado do `FluidMotion`/joystick (RYK Vector Wave, rodada 46-65): dá a esses dois mecanismos já planejados uma variante "muta levemente a cada repetição" ao lado da repetição exata atual — cada vez que um `Organism` reproduz um trajeto gravado, ele pode se afastar sutilmente do original, envelhecendo o gesto em vez de congelá-lo pra sempre. Segundo, o próprio **design do módulo**: o mesmo bloco físico de 4x4 canais de CV é reinterpretado inteiramente — mixer, switch, gravador, ou painner — dependendo só de um seletor de modo, sem circuito dedicado por função. É um segundo exemplo (ao lado do pool compartilhado de potenciômetros do Tiptop Z-8000) de **recurso compartilhado reconfigurável por contexto**, o que sugere que a "pool" de valores subjacentes já planejada pros `GeometricSequencer` também poderia trocar de *topologia* de uso (mixer/switch/gravador) via um modo, não só ser lida por múltiplos consumidores fixos |
| **E-RM Polygogo** (€569, 32HP) | Primeiro oscilador estéreo digital do mercado a usar uma técnica batizada pelo próprio fabricante de **"Polygonal Synthesis"**: síntese baseada em conformação bidimensional complexa da amplitude de ondas senoidais, representada visualmente atrás de um display OLED como um **polígono real** — o número de vértices (**Order**) e o deslocamento deles (**Teeth**) determinam diretamente a fundamental e a razão harmônica do som. **Roll** é modulação rotacional do polígono, que ou faz panning estéreo ou age como vibrato. Um operador de FM linear interno (OP ratio/OP amt) modula a fundamental e o conteúdo espectral agudo. Um estágio analógico de overflow na saída (**Fold**) age como wave crusher, dando o acabamento final ao som. As saídas **X/Y** do módulo podem alimentar diretamente um laser, osciloscópio ou vídeo modular — literalmente exibindo o polígono que está gerando o áudio como uma imagem em movimento. Três modos de operação (Infinite Cycling, quantized Order, OP ratio), entrada de hard sync, 1V/oitava | Apropriação estrutural, não só sonora: aqui o objeto geométrico *é* o motor de síntese, e a mesma estrutura de dados (vértices + deslocamento) serve simultaneamente de representação visual e de fonte do áudio. Isso é diretamente portável pro Aquorbium, cujos organismos já têm alguma forma visual implícita — sugere que a geometria de um `Organism` (proposta futura de corpo/silhueta) não precise ser só decorativa: os mesmos parâmetros que descrevem sua forma (número de "vértices" ligado à complexidade da espécie, deslocamento ligado a uma característica de personalidade do indivíduo) podem alimentar diretamente um oscilador polygonal-synthesis-like, fazendo o timbre do organismo derivar literalmente da sua forma, não apenas correlacionar com ela por convenção de design. A saída X/Y como sinal literal também é precedente pra uma futura visualização externa do Aquorbium (osciloscópio/laser/vídeo) alimentada diretamente pelos mesmos dados de síntese, sem estágio de renderização separado |

**Reforços breves:** Doepfer A-174-4v 3D Joystick (Vintage Edition), €198 — acrescenta um terceiro eixo (Z, rotação do manípulo) ao joystick clássico XY, gerando 10 CVs de um único gesto (X/Y/Z, X/Y/Z invertidos, e quatro valores derivados dos quadrantes Q1-4). Reforça, sem introduzir princípio novo, o já registrado a partir do RYK Vector Wave (posição de joystick controlando síntese/mixagem multidimensional) — aqui a novidade é só a dimensão extra (rotação) e a derivação de CVs de quadrante, ambas absorvíveis como refinamento do mesmo mecanismo já anotado, não como entrada nova na tabela.

Páginas 82-85 não renderam achado novo além dos três acima — dominadas por utilidade pura (fontes, cases, buffers, cabos, decksavers), reedições de circuito clássico (Doepfer A-106-5 SEM VCF, AJH FFB914) e clones/variações já cobertas por tarefas existentes.

### SchneidersLaden — páginas 86-111 (últimas ~26 páginas — catálogo completo)

*Trecho final do catálogo, o mais denso em módulos de catálogo profundo de fabricante já coberto (toda a série RS da Analogue Systems, dezenas de módulos A-100 da Doepfer, série STG soundlabs, série RS-xxx, ES-xxx da Expert Sleepers) e em utilidade pura (fontes, cases Monster/DIY, rails, parafusos, cabos flat ribbon, adaptadores). Nenhum princípio genuinamente novo apareceu nesse trecho — nada que passasse a régua de "mecanismo que o Aquorbium ainda não tem coberto por um precedente já registrado". Um único candidato foi avaliado e rebaixado a reforço breve.*

**Reforços breves:** WORNG Electronics Vector Space (€349, catálogo página 93) — descrito pelo fabricante/loja como mixagem de CV tridimensional: três sinais de CV são interligados dentro de um "modelo espacial", comparado pela loja ao Wiard/Malekko JAG mas com uma dimensão a mais (três eixos em vez de dois). Sem manual/ficha técnica aprofundada consultada além da descrição de catálogo, então não vira entrada de tabela própria — mas reforça, junto com RYK Vector Wave, Doepfer A-174-4v 3D Joystick e Neuzeit Quasar, o cluster já registrado de **controle/mixagem vetorial multidimensional** (posição espacial abstrata controlando múltiplas saídas simultaneamente). Não introduz mecanismo além do que já está anotado nesse cluster.

Com isso, as 111 páginas do catálogo Eurorack 3U da SchneidersLaden foram percorridas por completo, em rodadas sucessivas (1 → 2-5 → 6-15 → 16-25 → 26-45 → 46-65 → 66-85 → 86-111). Esta pesquisa fica encerrada como fonte primária deste levantamento; qualquer expansão futura de catálogo (novidades adicionadas depois desta data) exigiria uma nova varredura à parte, não uma continuação desta.
