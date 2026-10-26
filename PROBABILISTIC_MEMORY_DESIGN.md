# Aquorbium — memória e distribuições probabilísticas

Atualização: 2026-09-24.

## Objetivo

Este marco separa três formas de acaso que antes apareciam juntas na lista de
pesquisa: memória binária mutável, forma estatística do acaso e estado de
repetição de uma personalidade. Os componentes permanecem isolados e não
alteram o Cérebro ativo, o Ecossistema, o painel ou o som do `./run`.

## ShiftRegisterMemory

`ShiftRegisterMemory` conserva de 1 a 32 bits. A cada avanço, publica o bit que
sai, decide uma mutação e insere o bit recebido ou sua inversão. Seed, padrão
inicial e probabilidade são explícitos.

A futura integração poderá derivar a probabilidade de `Desire::calm`, mas esse
mapeamento não pertence ao componente: calma pode significar menor mutação,
maior persistência ou outra relação musical ainda a ser escutada.

## ProbabilityDistribution

`ProbabilityDistribution` atravessa continuamente duas distribuições
limitadas a `[-1,1]`:

- uniforme, com a mesma densidade em todo o intervalo;
- aproximadamente gaussiana, obtida pela soma de seis variáveis uniformes e
  concentrada em torno de zero.

Cada chamada consome sete números pseudoaleatórios em qualquer posição do
morph. Assim, mudar o controle altera a forma da distribuição sem deslocar a
sequência aleatória futura. O traço `intelligence` é um destino possível, não
uma conexão já implementada.

## EvolutionLoopMachine

`EvolutionLoopMachine` possui 64 posições fixas e quatro modos:

- `random`: renova e grava cada posição;
- `looping`: percorre a memória sem alterá-la;
- `evolving`: substitui gradualmente posições segundo uma probabilidade;
- `locked`: sustenta uma única posição.

O modo evolving mistura o valor antigo e um novo valor em partes iguais, em
vez de introduzir um salto completo. A classe expõe modo e índice, mas não
decide sozinha quando a personalidade de um organismo muda de estado.

## Contrato e validação

- memória fixa: 32 bits e 64 valores;
- seed e reset reproduzíveis;
- nenhum lock, I/O ou alocação no avanço;
- probabilidades e comprimentos limitados;
- saídas finitas e limitadas.

`aquorbium_probabilistic_memory_tests` cobre deslocamento, mutação, padrões de
32 bits, determinismo, concentração estatística em 20.000 amostras, os quatro
modos evolutivos e limites. `aquorbium_realtime_tests` executa os três
componentes sob contagem de heap. Em 2026-09-24, o build Release completo e os
33 testes CTest passaram em 42,20 s.

## Próximo portão

Comparar as três fontes com `RandomToLoop`, `BiomaMarkov` e o random walk já
existentes. Cada uma deve receber um papel diferente antes da integração:
estrutura binária, distribuição de gesto ou memória de personalidade. Depois
será possível renderizar trajetórias e testar mapeamentos de calma e
inteligência sem transformar todo parâmetro em acaso simultaneamente.
