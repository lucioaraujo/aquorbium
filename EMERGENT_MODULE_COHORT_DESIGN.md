# Aquorbium — coorte de módulos emergentes

Atualização: 2026-09-28.

## Objetivo

Este marco transforma dez componentes ainda não codificados da pesquisa em
componentes C++ puros, limitados e testáveis. A coorte entra diretamente no
`IntegratedModuleBank`: não volta a criar uma coleção invisível e não aumenta
o número de knobs. Cada mecanismo reforça uma das quinze famílias já operáveis
no cartão `08 MÓDULOS`.

## Componentes

| Componente | Função | Família visível |
|---|---|---|
| `ReeseVoice` | três serras PolyBLEP, detune simétrico e deriva semeada | `FM + HARMONICS` |
| `DualPeakFilter` | dois picos ressonantes, cross-feedback limitado e paralelo↔série | `FM + HARMONICS` |
| `ResonantFilterSequencer` | oito bandas fixas selecionadas por fase; contexto de encontro abre uma segunda banda | `CLOCK + EVENTS` |
| `FrequencyIsolator` | foco passa-banda preparado para futura composição com a Lupa | `GRANULAR CAPTURE` |
| `InteractionProcessor` | ring, XOR e duck de duas fontes com transição suave | `BRANCH + PERMUTE` |
| `PowerStarve` | redução de alimentação, resolução e atualização conforme trinca | `INTEGRITY + MEMORY` |
| `QuadratureStereoRotator` | rotação estéreo de potência constante por seno/cosseno | `FLUID + PHASE` |
| `PolygonalOscillator` | síntese derivada de polígono e saídas X/Y literais | `FRACTAL + VECTOR` |
| `BoundedRoutingMatrix` | matriz 5×5 normalizada, modulada por contexto | `BRANCH + PERMUTE` |
| `SharedParameterPool` | pool de 16 valores para oito consumidores, com leitura direta, rotativa e sustentada | `ROTATE + LAYOUT` |

`PolygonalOscillator` fecha o protótipo de duas tarefas da mesma hipótese: o
motor de síntese e a saída X/Y para visualização futura.
`InteractionProcessor` reúne ring, XOR, riqueza harmônica e ducking porque são
estados da mesma relação, não quatro módulos de saída independentes. O banco
usa sinais contextuais provisórios; contatos reais de predador/presa,
`SurvivalStrategy`, Lupa, `AquariumTransform`, BiomaBass, `AccessPort` e forma
visual de `Organism` continuam integrações especializadas pendentes.

## Limites

- nenhuma classe conhece JUCE, GUI, arquivo ou dispositivo;
- toda memória tem capacidade fixa e todo processamento é `noexcept`;
- frequências são limitadas pela taxa preparada;
- filtros recuperam estado inválido e todos os sinais publicados são finitos;
- feedback, cross-modulação, ducking e starve têm ganho máximo explícito;
- a matriz normaliza cada saída pela soma dos ganhos absolutos;
- seeds são separadas das fontes anteriores;
- a rota inteira permanece removível por `LAB MIX=0`.

## Critério de pronto

Cada classe deve ter teste de determinismo ou invariantes, extremos não
finitos, saída limitada e consequência dentro da família integrada. O teste de
tempo real deve executar a coorte dentro do banco sem alocação. Escuta humana,
calibração e eventual promoção para Cérebro, Ecossistema, espécies ou Master
permanecem etapas posteriores.

## Validação concluída

- teste dedicado: dez componentes aprovados em determinismo, limites, bypass,
  topologias, roteamento e retenção;
- banco integrado: quinze famílias com consequência audível e bypass mestre;
- teste de tempo real: coorte ativa sem alocação no callback;
- suíte completa: 42/42 em 89,32 s;
- build Release: 14.166.224 bytes, SHA-256
  `05bc65f0a3640c540fcfb1e88933b4009e8d3b75875348dc4943d81df19be82c`;
- benchmark offline com todas as famílias em 0,6: 13,77% em
  48 kHz/bloco 128/quatro vozes; pior ponto observado, 30,66% em
  96 kHz/bloco 128/quatro vozes, equivalente a 3,26× tempo real.

O benchmark não comprova prazo de callback, XRUN zero ou estabilidade do
dispositivo. A aprovação musical também permanece aberta.
