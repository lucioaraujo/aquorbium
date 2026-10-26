# Corpo sonoro por organismo — AQUORBIUM

**Data:** 2026-08-12  
**Estado:** fundação integrada e validada

## Decisão

`Ecosystem::OrganismState` continua sendo modelo ecológico e não possui DSP,
buffer, fase ou método virtual de áudio. O corpo audível vive na camada de
integração `OrganismVoiceEngine`, que recebe a espécie e controles contínuos e
despacha para um motor DSP real.

Esta separação corrige duas alternativas inadequadas:

- escolher geradores por índice (`2 = ressonador`, `3 = pulsar`) tornava a
  identidade dependente da ordem do array;
- adicionar `virtual renderSample()` ao organismo misturaria física, decisão e
  síntese, além de impor hierarquia/alocação como padrão sem necessidade.

O despacho atual é um `switch` fechado sobre `Ecosystem::Species`, incorporável
pelo compilador e sem RTTI, ponteiro proprietário, alocação ou chamada virtual.

## Corpos implementados

| Espécie | Corpo DSP | Estado |
|---|---|---|
| `Oscillator` | `BiomaMorphOscillator` | ativo |
| `Pulsar` | `BiomaPulsar` | ativo |
| `Resonator` | `BiomaModalResonator` | ativo; Morph é fallback técnico comparativo |
| `GranularStriker` | `BiomaGranularStriker` | ativo; Morph é fallback técnico comparativo |
| `PhysicalString` | banco dedicado `BiomaPhysicalString` em `BiomaEngine` | ativo; identidade publicada pelo despachante compacto |

O granular passou pelos portões de pesquisa, estabilidade, tempo real e
orçamento em [`GRANULAR_STRIKER_DESIGN.md`](GRANULAR_STRIKER_DESIGN.md). O
oscilador genérico aparece somente no seletor técnico A/B e nunca é apresentado
como se fosse o corpo granular.

## Propriedade e continuidade

Cada organismo de cada voz possui estado DSP fixo. O banco autônomo e as vozes
externas usam a mesma classe de corpo, mas mantêm instâncias próprias; MIDI não
rouba fase ou cauda do Bioma autônomo.

No ressonador e no Striker, o corpo especializado e o Morph de comparação
continuam avançando quando o seletor técnico muda. Assim, a comparação não
reinicia relógio, fase ou Ecossistema. Organismos adormecidos também avançam
seus motores implementados; a energia suavizada determina audibilidade.

Os motores atuais coexistem em memória fixa. O Striker possui 24 slots por
corpo, suficientes para duas nuvens máximas; cinco corpos ocupam 120 slots. Um
futuro motor de captura não será simplesmente replicado com buffers grandes em
todas as vozes: deverá declarar capacidade, propriedade e pior caso próprios.
O Physical String aplica esse princípio agora: 131.280 bytes por corpo, somente
cinco corpos (autonomia + quatro vozes externas), 656.400 bytes no total. Ele
não é replicado dentro dos 25 despachantes compactos.

## Telemetria

`BiomaEngine::Telemetry::sources` publica, por organismo:

- espécie solicitada;
- gerador efetivamente audível;
- se a espécie está implementada;
- telemetria dos corpos granular e modal locais.

Isso permite que GUI, render e testes distingam uma função real de uma espécie
apenas planejada.

`BiomaEngine::setSpeciesGain` fornece um balanço explícito sem suspender o DSP.
Na calibração atual, Morph/Pulsar passaram de 9,2 dB de distância nos stems em
unity para 2,4 dB na mistura autoral. Os corpos eventuais permanecem acima de
unity para ganhar presença sem transformar todos os seres em fontes contínuas.

A normalização interna da integração usa compensações separadas do ganho
performático: Granular `9,0`, Modal `16,5` e String `19,0`. Elas foram derivadas
da comparação de dez impactos e dos stems autônomos; não alteram o estado dos
motores. Com ganho de espécie em unity, os picos autônomos passaram de
`0,000426/0,000114/0,000215` para `0,00653/0,00612/0,01308`, respectivamente.
Na mistura autoral, continuam eventos abaixo das fontes contínuas, mas deixam de
ser presença apenas telemétrica.

## Evidência

- `aquorbium_organism_voice_tests`: identidade dos cinco geradores, saída
  finita, limite unitário, fallbacks técnicos e seed;
- `aquorbium_autonomous_energy_tests`: o mapa ecológico real é
  Morph/Granular/Modal/Pulsar/String, coincide com a espécie publicada e excita os
  corpos percussivos por evento de vida;
- `aquorbium_realtime_tests`: banco autônomo mais quatro vozes externas, morph
  ecológico e eventos continuam com zero alocações durante o render;
- a suíte completa permanece determinística entre tamanhos de bloco.

Três rodadas consecutivas do benchmark Release, cada ponto já sendo mediana de
cinco janelas, produziram:

| Caso | CPU observada | Mediana entre rodadas | Linha anterior |
|---|---:|---:|---:|
| 48 kHz, bloco 128, autonomia | 1,57–1,81% | 1,67% | 1,65% |
| 48 kHz, bloco 128, +4 vozes | 5,38–6,08% | 5,62% | 5,29% |
| 96 kHz, bloco 128, +4 vozes | 10,02–11,66% | 11,30% | 10,69% |

Há dispersão do sistema e um pequeno custo possível no caminho polifônico. O
resultado continua folgado em render offline, mas não é autorização para
acumular novos corpos sem nova linha de base e teste real de deadline/xrun.

## Próximo portão sonoro

Observar o diálogo autônomo do quinto ser antes de criar macros dedicadas ou
modelos mais complexos de arco, martelo e dispersão. Autocaptura granular
permanece pesquisa. Espaço e Master permanecem processadores da rota, não
espécies sonoras.
