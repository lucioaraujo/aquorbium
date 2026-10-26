# Aquorbium — gestos e formas de movimento

Atualização: 2026-09-24.

## Objetivo

Este lote materializa duas hipóteses de movimento sem alterar o aplicativo
ativo: um banco comum de formas de onda e um gravador bidimensional de gestos.
Os componentes podem servir futuramente ao `FluidMotion`, à Lupa ou a um
controle XY, mas ainda não estão ligados ao `Ecosystem`, à GUI ou ao áudio.

## MotionWaveBank

`MotionWaveBank` reúne oito formas de controle: seno, triângulo, pulso suave,
seno duplo, rampa ascendente, rampa descendente, quadrada e assimétrica. Todas
recebem fase normalizada e devolvem valores finitos em `[-1, 1]`.

O método `shapeForIdentity` associa espécie, estratégia e seed a uma forma de
modo determinístico. Essa associação não consome o fluxo aleatório do Cérebro,
portanto não desloca decisões musicais preexistentes. `FluidMotion` passou a
usar esse banco para as formas que já publicava, eliminando uma implementação
local duplicada sem mudar suas rotas.

## MotionGestureRecorder

`MotionGestureRecorder` mantém até 2.048 pontos XY em armazenamento fixo. O
ciclo possui três estados: inativo, gravação e reprodução. São necessários ao
menos dois pontos para iniciar o playback.

A reprodução aceita velocidades entre 0,125× e 8× e interpola linearmente
posições fracionárias. Com mutação zero, o gesto repete exatamente. A opção de
mutação aplica, ao fim de cada volta, uma perturbação determinística e limitada
de até 5% em cada eixo. Pontos gravados e mutados permanecem em `[-1, 1]`.

## Contrato de tempo real

- memória fixa, sem alocação durante gravação ou reprodução;
- ausência de lock, I/O e chamadas ao sistema;
- seed reproduzível e mutação opcional;
- valores inválidos são saneados;
- formas e trajetórias permanecem limitadas;
- nenhum parâmetro, callback ou rota do app foi acrescentado.

## Validação

`aquorbium_motion_gesture_tests` cobre as oito formas, limites, distinção entre
formas, associação determinística, loop exato, interpolação fracionária,
mutação reproduzível e saneamento. O teste de heap executa os dois componentes
com o rastreamento de alocação ativo.

Em 2026-09-24, o build Release completo e os 36 testes CTest passaram em
38,31 s. A máquina permaneceu dentro dos limites de RAM e swap definidos pelo
projeto.

## Próximos portões

Antes de integrar, é necessário renderizar trajetórias, avaliar repetição e
mutação musicalmente e definir o primeiro consumidor. A captura do gesto da
Lupa ou de um controle XY, o mapa de espécie/estratégia e a substituição de
movimento no `Ecosystem` permanecem pendentes. A integração deverá ser gradual
e acompanhada de teste interativo de estalos, travamentos e resposta audível.
