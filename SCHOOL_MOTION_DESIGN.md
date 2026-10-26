# Aquorbium — movimento compartilhado de cardume

Atualização: 2026-09-24.

## Objetivo

Este marco investiga coesão real de movimento sem criar um gerador por peixe.
Uma única fase mestra alimenta membros relacionados, quatro leituras de
movimento e um autopatch limitado. Nada foi conectado ao `Ecosystem` ativo.

## HarmonicPhaseBank

`HarmonicPhaseBank` mantém uma fase mestra e até oito relações racionais. As
quatro relações iniciais são 1/1, 5/4, 3/2 e 2/1. Cada membro recebe sua fase
como múltiplo racional do mesmo relógio; não há acumuladores independentes que
possam derivar uns dos outros.

O relógio longo é reduzido a cada 720.720 ciclos. Esse número é divisível por
todos os denominadores de 1 a 16, portanto a redução não quebra nenhuma das
relações permitidas. O componente é uma possível implementação futura de
`SchoolInfo::sharedPhase`, não uma alteração da estrutura atual.

## FluidMotion

`FluidMotion` usa o banco racional e publica quatro saídas interpretadas por um
dos seguintes modos:

- `shapes`: quatro formas diferentes na mesma fase;
- `amplitudes`: a mesma forma em quatro níveis;
- `times`: a mesma forma em quatro posições temporais;
- `frequencies`: a mesma forma nas quatro relações racionais.

Assim, um gerador pode fornecer movimento relacionado a um cardume inteiro.
Os quatro canais são leituras do mesmo estado, não quatro organismos ocultos.

## Continuidade: formas e gesto

`FluidMotion` passou a delegar sua avaliação de formas ao `MotionWaveBank`.
Isso preserva as leituras já testadas e cria um ponto único para oito formas e
para uma futura associação por espécie, estratégia e seed.

O `MotionGestureRecorder` foi criado separadamente para registrar até 2.048
pontos XY, repetir o gesto em velocidades variáveis e, opcionalmente, aplicar
mutação limitada por volta. Ele ainda não alimenta este gerador nem substitui
movimento no Ecossistema. O contrato completo está em
`GESTURE_MOTION_DESIGN.md`; o marco ampliou a suíte para 36/36.

## Autopatch

Uma saída selecionável pode realimentar a velocidade do relógio. O ganho é
limitado a `[-0,95; 0,95]` e o multiplicador final permanece entre 0,1× e 4×.
Feedback zero conserva a trajetória sem feedback bit a bit. A realimentação
usa o quadro anterior, evitando dependência circular dentro do mesmo tick.

## Contrato e validação

- no máximo oito fases e quatro saídas;
- relações com numerador/denominador de 1 a 16;
- estado fixo, sem lock, I/O ou alocação;
- saídas finitas em `[-1,1]`;
- parâmetros e feedback limitados;
- uma única fase mestra reproduzível.

`aquorbium_school_motion_tests` cobre relações, limites, determinismo, quatro
modos distintos, autopatch e invariância em feedback zero. O teste de heap
executa o banco e `FluidMotion`. Em 2026-09-24, o build Release completo e os
35 testes CTest passaram em 40,57 s.

## Próximo portão

Renderizar trajetórias XY dos quatro modos, comparar coesão e mecanicidade e
definir quantos membros um cardume pode receber. Só depois substituir a média
de fase ou as instâncias de movimento atuais. A integração deverá preservar
seed, perspectiva situada e independência de tamanho de bloco.
