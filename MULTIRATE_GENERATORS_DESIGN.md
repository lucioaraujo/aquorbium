# Aquorbium — geradores multirrate

Atualização: 2026-09-24.

## Objetivo

Este marco cria dois núcleos que conservam a mesma identidade matemática em
taxa de controle e taxa de áudio. Eles são componentes isolados: ainda não
alimentam `BrainEngine`, `SynthFMVoice`, cardumes, espécies, painel ou saída.

## FractalCore

`FractalCore` parte de uma fase fundamental e acrescenta até sete réplicas. Em
cada iteração, a fase local é multiplicada por uma razão configurável e recebe
uma pequena realimentação do valor anterior. A amplitude de cada réplica cai
segundo `roughness`; a soma é normalizada pelo ganho acumulado.

Parâmetros:

- frequência de 0,0001 Hz até 45% do relógio de execução;
- razão de replicação entre 1,25 e 8;
- rugosidade entre 0 e 0,95;
- uma a oito iterações.

O mesmo objeto pode operar, por exemplo, a 200 Hz como campo rítmico lento ou
a 48 kHz como fonte de áudio. Isso não significa que os dois usos já tenham
função musical aprovada. A implementação evita criar duas versões divergentes
do algoritmo antes da escuta.

## VectorGenerator

`VectorGenerator` usa uma única curva de subida e descida com skew e curvatura.
O modo de ciclo contínuo permite LFO ou oscilador; o modo one-shot transforma a
mesma curva em envelope. A saída pode ser unipolar ou bipolar.

- `skew` define a posição do pico entre 1% e 99% do ciclo;
- `curve` atravessa resposta rápida, linear e lenta;
- `loop` volta ao início; `oneShot` encerra em silêncio;
- a frequência usa o mesmo contrato multirrate do `FractalCore`.

Uma tolerância numérica de `1e-12` trata o instante final do ciclo. Sem isso,
dez incrementos de 0,1 podiam permanecer infinitesimalmente abaixo de 1 e
adiar o encerramento de um one-shot por uma amostra.

## Contrato técnico

- estado fixo e nenhum buffer dinâmico;
- `tick()` limitado, determinístico e sem alocação;
- parâmetros não finitos preservam o último valor válido;
- fase inicial explícita e saída sempre finita em `[-1,1]`;
- frequências limitadas em relação ao relógio informado.

O teste multirrate compara `FractalCore` a 200 e 2.000 Hz nos mesmos instantes.
A fase 0 e a fase 1 são comparadas circularmente, pois representam o mesmo
ponto do ciclo. Também são cobertos determinismo bit a bit, efeito das
iterações, limites, ciclo, one-shot e os três registros do `VectorGenerator`.
O teste de heap executa ambos durante o render controlado.

Em 2026-09-24, o build Release completo e os 32 testes CTest passaram em
41,26 s.

## Próximo portão

Gerar renders em registros lentos e de áudio, medir espectro e CPU, escutar se
há identidade própria e só então escolher uma primeira ligação. Candidatos:
decisões do Cérebro para o `FractalCore` e envelope/LFO compartilhado de
cardume para o `VectorGenerator`. A integração não será feita enquanto a
função musical e os limites de aliasing não estiverem definidos.
