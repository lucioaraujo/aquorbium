# Aquorbium — coorte de geração e interação

Atualização: 2026-09-29.

## Objetivo

Esta terceira coorte transforma dez hipóteses ainda sem código em protótipos
portáteis e audíveis dentro do `IntegratedModuleBank`. Ela prioriza geração de
forma, relações entre vozes e escuta situada. Não cria controles, páginas ou
estado persistente: as quinze famílias existentes continuam sendo a superfície
experimental de comparação.

## Componentes e destino experimental

| Componente | Função | Família visível |
|---|---|---|
| `FixedLSystemGenerator` | reescrita formal bounded convertida em eventos de avanço e giro | `CLOCK + EVENTS` |
| `CellularAutomatonSequencer` | autômato elementar de 32 células com regra e densidade observáveis | `PATTERN + LOOP` |
| `SpectralTransferVocoder` | seis bandas fixas transferem a envoltória espectral do mix para um corpo interno | `FM + HARMONICS` |
| `EnvelopeContagionBus` | envelopes de seis fontes contaminam vizinhos sem uníssono forçado | `FM + HARMONICS` |
| `NoInputFeedbackBus` | três regiões filtradas, saturação e realimentação interna limitada | `INTEGRITY + MEMORY` |
| `ReactiveThresholdStrategy` | estímulo contínuo seleciona estado por limiares e histerese | `EVOLVE + STAGES` |
| `LockInFocusExtractor` | foco síncrono por referência de frequência, distinto do foco espacial e espectral fixo | `GRANULAR CAPTURE` |
| `BehaviourSceneBank` | oito cenas de quatro dimensões com recall e transição limitada | `EVOLVE + STAGES` |
| `ReconfigurableResourcePool` | mesmo pool fixo opera como mixer, switch, recorder ou router | `ROTATE + LAYOUT` |
| `LightweightBinauralSpatializer` | protótipo de ITD, sombra de cabeça e grave central estável | `FLUID + PHASE` |

## Limites conceituais

- L-system e autômato são geradores alternativos do laboratório; ainda não
  substituem o `GeometricSequencer`.
- O vocoder é um banco autoral de bandas fixas, não uma emulação de FUMANA.
- Contágio, estratégia e cenas recebem fontes experimentais; a ligação por
  `schoolId`, `SurvivalStrategy` e `Ecosystem` permanece separada.
- O barramento sem entrada é uma matéria estrutural limitada, não o Master
  criativo definitivo.
- O foco lock-in não conhece um organismo real enquanto permanece no banco.
- A espacialização binaural é um primeiro modelo leve de ITD/sombra, não uma
  HRTF medida nem uma promessa de localização tridimensional validada.

## Contrato técnico

- C++20 puro, sem JUCE, I/O, lock ou alocação no processamento;
- arrays e filas com capacidade fixa;
- expansão formal recusada quando excederia o armazenamento;
- filtros, feedback, distância e ganhos limitados;
- parâmetros não finitos usam fallback seguro;
- mudanças estruturais ocorrem em configuração ou fronteira de evento;
- `LAB MIX=0` retorna antes de toda a coorte;
- teste dedicado antes da integração, seguido pelos testes do banco, de tempo
  real e pela suíte completa;
- benchmark obrigatório antes de declarar o marco técnico concluído.

## Validação técnica

- teste dedicado com 11 invariantes aprovado;
- banco integrado e teste de tempo real aprovados, com zero alocações;
- suíte completa: 44/44 testes em 91,95 s;
- benchmark Release em 48 kHz/bloco 128/quatro vozes: 23,24% da janela,
  margem de 4,30× tempo real;
- pior ponto ativo: 41,69% em 96 kHz/bloco 32/quatro vozes, margem de 2,40×;
- bypass em 48 kHz/bloco 128/quatro vozes: 10,38%, confirmando que as três
  coortes não são processadas quando `LAB MIX=0`;
- binário Release: 14.217.776 bytes, SHA-256
  `588c8dbfc0f44a18fd6fb5b3b561dd106b69c60c70bfa1b8b21ea3f1fbeddee3`.

O custo aumentou principalmente pelo vocoder de seis bandas e pelo binaural.
A margem continua suficiente para laboratório offline, mas requer escuta e
medição no dispositivo antes de qualquer promoção. HRTF medida, Master, cenas
persistentes e vínculos ecológicos especializados permanecem abertos.
