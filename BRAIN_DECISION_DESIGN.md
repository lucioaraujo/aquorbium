# Aquorbium — memória formal Markov

Atualizado em 2026-09-27. Estado: memória formal integrada à Geometria do
standalone; validação técnica concluída, escuta musical pendente.

## Função

`BiomaMarkov` é uma memória estrutural de oito estados. Ele não produz áudio:
os estados representam regiões de forma e deslocam somente a seção/contorno
entregue ao `GeometricSequencer`. Presença, estratégia, timbre e ganho não são
alterados diretamente. Essa fronteira evita transformar uma técnica
probabilística genérica em mais uma espécie sonora ou em modulação universal.

Cada estado possui uma linha de oito pesos normalizados. A matriz padrão
favorece permanência, vizinhos e pequenos saltos. Três parâmetros alteram a
decisão seguinte:

- `inertia`: peso adicional do estado presente;
- `mutation`: probabilidade difusa de alcançar estados menos prováveis;
- `contextInfluence`: quanto um alvo contextual normalizado atrai a cadeia.

O contexto tem `target` e `intensity`. A integração calcula o alvo a partir de
luz, temperatura, pulso de acasalamento e perigo. Seed e matriz são locais, a
contagem de transições é limitada e `advance()` não aloca, trava ou faz I/O.
Uma transição só ocorre no cruzamento coletivo positivo do campo Relabi; não
há decisão Markov na taxa de áudio nem avanço por organismo.

## Integração formal

O domínio inicial escolhido é forma. Os oito estados usam os deslocamentos de
seção `{-5, -3, -2, -1, 0, 1, 3, 5}`. `FORM MIX` interpola esses deslocamentos
até zero antes de entrarem na Geometria; com zero, a cadeia continua avançando
em seu stream aleatório próprio, mas a saída sonora permanece idêntica bit a
bit. Mudanças de altura decorrentes da nova seção conservam o glide existente.

O cartão `02 GEOMETRIA` expõe quatro macros grandes:

- `FORM MIX`: presença da memória formal na seção geométrica;
- `CONTEXT`: atração do próximo estado pelo contexto ecológico;
- `INERTIA`: tendência de permanecer no estado atual;
- `MUTATION`: redistribuição para saltos menos prováveis.

Os três últimos ficam visualmente inativos quando `FORM MIX=0`. Os quatro
valores atravessam `AudioControlFrame`, cenas e preferências; cenas antigas de
25, 31 e 35 valores recebem os defaults formais e continuam válidas. A
telemetria publica estado, número de transições e alvo contextual.

## Validação de integração — 2026-09-26/27

- `AutonomousEnergyTests`: determinismo entre blocos, cadeia avançando em
  bypass, transparência bit a bit em zero e mudança estrutural limitada ativa;
- `ControlResponseTests`: os quatro controles produzem diferença mensurável em
  20 s; os três dependentes juntos produzem diferença exata zero no bypass;
- `RealtimeSafetyTests`: automação ativa sem alocação; quadro de controle com
  168 bytes e ponte fixa com 704 bytes;
- `FormMemoryRender`: quatro WAVs de 20 s, sete transições por cenário,
  repetição do bypass bit a bit, picos abaixo de 0,234 e render abaixo de
  0,049 vez o tempo real;
- auditoria de layout: 24 capturas em quatro dimensões, knobs de pelo menos
  52 px e áreas de 80×92 px; vistas 960×600 e 1280×800 inspecionadas;
- build Release e suíte completa: 39/39 testes em 68,78 s.

Evidências técnicas: `validation-output/2026-09-26-form-memory/` e
`validation-output/2026-09-26-form-memory-layout/`. Elas não substituem escuta
humana nem estabilidade no dispositivo real.

Não houve cópia de implementação externa. A implementação materializa a
hipótese Markov já registrada na arquitetura e permanece local ao Aquorbium.
O marco isolado original, com suíte de 25 testes, permanece documentado como
proveniência da integração atual.

## Componentes probabilísticos complementares — 2026-09-24

Três componentes isolados foram acrescentados sem alterar `BiomaMarkov`:

- `ShiftRegisterMemory` conserva padrões binários com mutação explícita;
- `ProbabilityDistribution` muda a forma estatística entre uniforme e
  aproximadamente gaussiana;
- `EvolutionLoopMachine` alterna renovação, repetição, evolução e retenção.

Eles não são três substitutos para Markov. A cadeia agora decide regiões de
forma com dependência contextual; o shift register conserva estrutura binária; a
distribuição altera a densidade do acaso; a máquina evolutiva administra a
memória de uma sequência. A comparação musical e a atribuição de domínio
continuam pendentes para os três componentes isolados. Contrato em
`PROBABILISTIC_MEMORY_DESIGN.md`; build
Release e suíte 33/33 passaram em 24/09.

## Forma e deriva lenta — 2026-09-24

`ConditionalStageMachine` acrescenta oito estágios com destino normal e salto
condicional por perigo, noite ou stress de trinca. `TripleSlowDrift` combina
três random walks com períodos padrão de 5 segundos, 1 minuto e 20 minutos.
`IntegrityCrossfade` produz ganhos de potência constante para uma futura
passagem saudável↔trincado.

Esses componentes não são promovidos pela integração do Markov nem substituem
o Cérebro atual. Eles isolam mecanismos que poderão receber contexto depois que
seus papéis musicais forem definidos. Contrato em
`STRUCTURAL_CONTROL_DESIGN.md`; build Release e suíte 34/34 passaram.

## O Cérebro chega ao Sequenciador e ao Espaço — 2026-10-23

O `BiomaBrain` tem um registrador de deslocamento de 8 bits, avançado por
um relógio de cerca de 0,23 Hz. Ele é lido por cinco somas ponderadas, uma
por organismo, com pesos diferentes. O resultado são cinco **inclinações
correlacionadas** entre −1 e 1: organismos que pesam os mesmos bits tendem
a concordar, como num cardume. Até agora essas inclinações só moviam a
deriva de cada organismo no áudio. Agora `correlation()` as entrega a mais
dois lugares.

**Sequenciador (Geometria).** Quando um passo muta, a direção (um grau
acima ou abaixo) era uma moeda justa por organismo. Agora o limiar da moeda
é `0,5 − 0,5 · inclinação · GEOMETRIA`.

- Com GEOMETRIA em 0, a moeda continua justa e o resultado é idêntico bit a
  bit, porque o sorteio é o mesmo.
- Com GEOMETRIA em 1 e inclinação ±1, a direção fica decidida pelo Cérebro.
- O acaso continua decidindo *se* muta; o Cérebro só decide *para onde*.
  Organismos correlacionados tendem a subir ou descer juntos.

**Espaço.** O humor do cardume (a média das cinco inclinações) multiplica a
largura dos reflexos por `1 + 0,10 · humor · gesto do ESPAÇO`.

- O passo do registrador chega aos taps pelos alvos, que já deslizam até o
  valor novo.
- O teste de cliques passa com o gesto do Espaço em 0,5.

**Testes:**
- `GeometricSequencerTests`: idêntico em GEOMETRIA 0; com inclinações
  opostas, 2001 de 4000 passos mutam em sentidos opostos.
- `BiomaEngineTests`: no motor, durante 60 s, a razão da largura com o
  gesto do Espaço em 1 contra 0 vai de 0,94 a 1,10.
- Renders dourados sem mudança.

A ligação do motor ao Sequenciador (`setBrainLean` a cada atualização de
controle) foi conferida por leitura do código; o teste da direção é de
unidade.

Falta a escuta: se a coordenação se ouve como cardume ou como mecanismo.
