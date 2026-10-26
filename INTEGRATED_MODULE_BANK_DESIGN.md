# Aquorbium — banco integrado dos módulos experimentais

Atualização: 2026-10-02.

## Objetivo e estado

O `IntegratedModuleBank` torna operáveis, no standalone, os protótipos de core
que a auditoria de 27/09 encontrou compilados e testados, mas ainda sem rota ao
vivo. Esta é uma integração experimental explícita: ela permite ouvir e
comparar o material existente sem fingir que dezenas de mecanismos internos
são dezenas de novas vozes independentes.

O cartão `08 MÓDULOS` oferece um bypass mestre, `LAB MIX`, e quinze famílias.
Todos os controles iniciam em zero. Com `LAB MIX=0`, o banco não processa seus
motores por amostra, retorna zero e preserva o comportamento e o orçamento do
instrumento anterior. Os quinze controles dependentes ficam visualmente
inativos até o usuário abrir o mestre.

## Famílias visíveis

| Macro | Implementações reunidas | Consequência no banco |
|---|---|---|
| `FM + HARMONICS` | `SynthFMVoice`, `OversampledSineFolder`, `HarmonicMorphOscillator`, `ReeseVoice`, `DualPeakFilter`, `BiomaBassVoice`, `SchoolWaveVoice`, `SpectralTransferVocoder`, `EnvelopeContagionBus` | corpos FM/harmônico, wavefold 4×, grave, cardume, transferência espectral e contágio de envelopes |
| `GRANULAR CAPTURE` | `BiomaGranularCapture`, `FrequencyIsolator`, `RecentMixWavetable`, `LockInFocusExtractor` | memória granular, foco espectral/síncrono e resíntese incremental do mix recente |
| `PERC + PULSE` | `SamplePercussionVoice`, `StochasticPulseMask`, `HarmonicRippleVoice` | ataques, supressão probabilística e ondulações harmônicas em cluster |
| `FRACTAL + VECTOR` | `FractalCore`, `VectorGenerator`, `PolygonalOscillator`, `FrequencyShifter` | geração autorreplicada, vetorial, polygonal X/Y e deslocamento linear de parciais |
| `LORENZ + DRIFT` | `LorenzCV`, `DoubleScrollAttractor`, `TripleSlowDrift`, `SlewRateRandomWalk`, `BoundedSelfFeedback` | dois modelos de caos, deriva em múltiplas escalas e auto-feedback limitado |
| `FLUID + PHASE` | `FluidMotion`, `HarmonicPhaseBank`, `MotionWaveBank`, `QuadratureStereoRotator`, `WaterInertia`, `AirWaterMedium`, `LightweightBinauralSpatializer` | fase, deformação fluida, rotação, inércia, transição ar/água e binaural leve |
| `GESTURE WAVES` | `MotionGestureRecorder`, `MotionWaveBank`, `ContactCrackleExciter` | trajetória XY sintética e trajetória real da Lupa reproduzidas, com excitação limitada por contato/evento |
| `CLOCK + EVENTS` | `FractionalLogicClock`, `ControlEventExtractor`, `ResonantFilterSequencer`, `FixedLSystemGenerator` | relógio, eventos, percurso por bandas e forma reescrita |
| `TRANSPOSE + SELECT` | `AccumulatingTransposer`, `ThresholdStepSelector` | deslocamento melódico e seleção por limiar |
| `BRANCH + PERMUTE` | `BranchingStepRouter`, `NotePermutationMatrix`, `InteractionProcessor`, `BoundedRoutingMatrix`, `SemanticAccessRouter`, `SpotlightMixer`, `ReactiveThresholdStrategy` | decisão, limiares, permutação e rotas semânticas/contextuais entre domínios |
| `ROTATE + LAYOUT` | `PatternRotationMutator`, `StepMemoryLayout`, `AddressableStepMemory`, `SharedParameterPool`, `ReconfigurableResourcePool` | rotação, topologia e recursos compartilhados/reconfiguráveis |
| `PATTERN + LOOP` | `StepPatternBank`, `RandomToLoop`, `CellularAutomatonSequencer` | bancos, encadeamento, acaso em ciclo e autômato celular |
| `SHIFT + PROB` | `ShiftRegisterMemory`, `ProbabilityDistribution` | memória binária mutável e forma estatística |
| `EVOLVE + STAGES` | `EvolutionLoopMachine`, `ConditionalStageMachine`, `DriftCurveShaper`, `CrossFeedbackStateMachine`, `BehaviourSceneBank`, `OpenRuleProgram` | evolução, saltos, contorno, estados por energia, cenas e regras limitadas |
| `INTEGRITY + MEMORY` | `IntegrityCrossfade`, classes de `PhilosophyMixEngine`, classes de `BehaviourMemory`, `PowerStarve`, `HungerProcessor`, `NoInputFeedbackBus` | transição íntegro/trincado, degradação, fome, estrutura sem entrada e memória |

As implementações já integradas anteriormente — Ecossistema, Cérebro,
organismos, Geometria, `BiomaRelabi`, `BiomaMarkov`, progressão harmônica,
Ambiente, Espaço, Cauda e saída técnica — continuam em suas rotas próprias e
não são duplicadas por este banco.

## Rota e contrato

O banco recebe a soma estéreo dos organismos depois da autonomia e antes do
Ambiente/Espaço/Cauda. Ele também recebe densidade, perigo, fases de dia e
estação, tensão da integridade, frequência-base do primeiro corpo autônomo,
os três traços do `GranularStriker` e a amostra final anterior do instrumento.
As quinze famílias formam uma contribuição estéreo limitada a `±0,65`, somada
à rota estabelecida antes das etapas posteriores.

Os valores são saneados e limitados a `[0,1]`, suavizados em 20 ms e enviados
pela mesma ponte de snapshot completo dos demais controles. Não há alocação,
lock, I/O ou tamanho de fila variável no callback. Material fixo e amostra
percussiva são preparados antes do áudio; sementes são independentes e o reset
rebobina também o gesto gravado.

A coorte de interação aberta acrescenta double-scroll por amostra, roteamento
semântico fixo, regras avaliadas somente na fronteira de evento e excitação de
contato com decaimento por amostra. A telemetria observa os quatro mecanismos,
mas não cria componentes visuais. Parsing e configuração de rotas permanecem
fora do callback. Detalhes e limites em `OPEN_INTERACTION_COHORT_DESIGN.md`.

O `CompositionalVariationDirector` agrupa as quinze famílias em quatro
funções de mix: pulso, tonalidade, textura e movimento. Ele modula os ganhos
familiares conforme o segmento formal quando `STRUCTURE MIX` está ativo; as
macros familiares permanecem como autorização e nível de cada camada. O plano
de quatro segmentos também modula, no core de autonomia, a acentuação das
vozes a partir do acento já produzido pela Geometria. O efeito é limitado,
dosado por `STRUCTURE MIX`, e eventos sem acento não recebem alteração. O
plano também dosifica a contribuição da memória de passos para a melodia por
segmento. É determinístico e ainda não tem aprovação auditiva.
Contrato em `COMPOSITIONAL_VARIATION_DESIGN.md`.

A coorte de refinamento acrescenta contexto real de altura, turbidez,
temperatura, posição e posse da Lupa. `AirWaterMedium` processa somente a
família `FLUID + PHASE`. A Lupa é amostrada a 200 Hz enquanto segurada e entra
em loop depois da soltura, mas somente quando `LAB MIX` está ativo. O Master
realimenta o Cérebro fora deste banco, também a 200 Hz. Detalhes e limites em
`CORE_REFINEMENT_COHORT_DESIGN.md`.

A coorte especializada acrescenta um segundo polo de 45 ms no contexto
contínuo vindo de `Desire`/Brain. A amostra de feedback não passa por esse polo:
ela entra com um atraso causal de uma amostra no `CrossFeedbackStateMachine`,
que já possui balística e histerese, e sua modulação limitada desloca seleção
por limiar e ramificação. O intervalo do subgrave vem do mesmo
`IntervalQuantizer` da decisão melódica. Os parâmetros do grão são congelados
no evento e derivados de `strength`, `agility` e `intelligence`.

## Interface, estado e compatibilidade

- `MacroPanelLayout` reserva 80×92 px por área interativa e pelo menos 52 px de
  diâmetro gráfico; as dezesseis macros ocupam oito linhas roláveis;
- rótulo, knob e valor permanecem em áreas separadas;
- a coorte não acrescenta campos; o formato total atual do aplicativo conserva
  63 valores persistentes depois da entrada independente do Master criativo;
- cenas V1 anteriores com 25, 31, 35, 39, 42 ou 46 valores continuam aceitas;
  os dezesseis valores do banco entram em zero;
- presets, importação/exportação e preferências escalares incluem o banco.

## Validação de 2026-09-29

- build Release do app concluído com warnings tratados como erro;
- suíte completa: 44/44 testes aprovados em 94,24 s;
- cada uma das quinze famílias produziu diferença determinística no teste
  dedicado; bypass mestre retornou zero;
- teste de tempo real exercitou simultaneamente as quinze famílias e passou
  com zero alocações; `AudioControlFrame` mede 260 bytes e a ponte fixa 1024;
- teste de layout aprovou 16 macros nos modos representativos, mantendo os
  mínimos de tamanho;
- a coorte emergente acrescentou dez componentes sem novos controles nem
  mudança no quadro persistente; teste dedicado e benchmark estão em
  `EMERGENT_MODULE_COHORT_DESIGN.md`;
- a coorte avançada acrescentou outros dez componentes e ativou a autocaptura
  granular, ainda sem novos controles ou mudança de estado; teste e benchmark
  estão em `ADVANCED_VOICE_ECOLOGY_COHORT_DESIGN.md`;
- a coorte de geração e interação acrescentou dez componentes às mesmas
  famílias, novamente sem mudar interface ou estado; contrato e limites em
  `GENERATIVE_INTERACTION_COHORT_DESIGN.md`;
- a coorte especializada integrou seis vínculos ecológicos/sonoros sem mudar
  interface ou persistência; contrato em
  `SPECIALIZED_INTEGRATION_COHORT_DESIGN.md`;
- `git diff --check` passou nos arquivos do marco;
- binário: 14.209.824 bytes, SHA-256
  `cdb25a33ea9dab4db3b5383dd711ed17815442835563f822fd64d7e8379180cc`.

A geração de capturas por `--layout-audit` não foi executada nesta rodada
porque `xvfb-run` não está instalado. Portanto, a geometria foi validada por
teste, mas inspeção visual das oito linhas, gesto de rolagem, DPI e escuta
humana continuam pendentes.

## Validação direcionada de 2026-10-02

- quatro componentes da coorte aberta compilados e aprovados no teste dedicado;
- banco aprovado com bypass, quinze famílias, determinismo, integração
  ecológica e observabilidade específica dos quatro mecanismos;
- teste de tempo real aprovado com zero alocação no callback;
- não houve novo controle, campo persistente ou alteração geométrica;
- suíte integral, benchmark, app real e escuta não foram executados porque o
  preflight encontrou 1,7/2 GiB de swap em uso e outra tarefa concorrente.

Depois da limpeza de swap, a coorte de refinamento passou em testes dirigidos
de core, banco, motor e tempo real, e a suíte integral passou 45/45 em 93,05 s.
A telemetria observou a mistura submersa e o loop real da Lupa; o cenário
completo observou feedback do Master dentro do teto de 0,32. O benchmark com
banco e Master ativos mediu 29,69% em 48 kHz/bloco 128/quatro vozes. O pior
ponto foi 62,08% em 96 kHz/bloco 128/zero vozes externas (1,61× offline).
Medições offline não atestam deadline nem XRUN no dispositivo.

## Próxima decisão

Abrir `LAB MIX` com moderação e ouvir uma família por vez. Registrar ritmo,
melodia, harmonia, timbre, eventos, função composicional, custo percebido e
eventuais cliques/XRUNs. Somente essa auditoria decidirá quais famílias deixam
o laboratório e ganham semântica ecológica própria; integração técnica não é
aprovação musical.
