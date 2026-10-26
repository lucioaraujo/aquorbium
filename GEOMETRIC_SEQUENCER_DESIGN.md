# Aquorbium — Geometric Sequencer, marco 1

## Pergunta musical

Como transformar eventos irregulares do Bioma em relações reconhecíveis de
ritmo e altura sem impor uma grade MIDI, congelar um loop ou converter acaso em
notas aleatórias independentes?

O primeiro marco cria uma geometria de decisão entre `AutonomousEnergy` e os
motores de voz. O relógio continua ecológico e não periódico; a geometria
decide quais encontros ganham ataque e onde pousam dentro de um campo intervalar.

## Pesquisa apropriada

Godfried Toussaint demonstrou que a estrutura do algoritmo euclidiano distribui
ataques tão uniformemente quanto possível e relacionou essas distribuições a
ritmos tradicionais. O Aquorbium adota somente esse princípio matemático. A
implementação é escrita do zero como acumulador circular limitado a 32 passos,
sem copiar código do artigo ou de outros instrumentos.

Fonte primária: G. T. Toussaint, *The Euclidean Algorithm Generates Traditional
Musical Rhythms*, McGill University,
<https://cgm.cs.mcgill.ca/~godfried/teaching/dm-reading-assignments/Euclidean-Rhythms.pdf>.

As apropriações de memória endereçável, múltiplos CVs por passo e pool
compartilhado registradas na arquitetura continuam como expansões posteriores.
Elas não são anunciadas neste marco.

## Arquitetura

O módulo é C++20 portátil e não sintetiza áudio.

- `EuclideanPattern`: gera e gira até 32 posições com custo e memória fixos.
- `IntervalQuantizer`: converte graus inteiros, inclusive negativos, em
  semitons; aceita até 16 intervalos e período fracionário para não bloquear
  futura microtonalidade.
- `GeometricSequencer`: mantém cinco lanes independentes sobre uma forma
  compartilhada, uma seed própria e um cursor por lane.
- `GenerativeInput`: densidade, coesão, perigo, raiz, âncora, registro e seção
  vindos do Bioma.
- `PerformanceIntent`: densidade rítmica, rotação, viés intervalar e mutação,
  com `influence` contínua entre autonomia e performance.
- `Step`: gate, acento, grau, pitch e quatro CVs normalizadas; é dado trivial e
  pode atravessar snapshots futuros.

Decisão geométrica acontece apenas quando `AutonomousEnergy` recebe um evento
de vida ou encontro coletivo. Não há `AudioPlayHead`, BPM, JUCE ou callback no
módulo. Um adaptador de sync futuro poderá gerar eventos sem alterar esta API.

## Comportamento generativo

- Densidade escolhe o número de ataques na órbita.
- Coesão aproxima as lanes de uma âncora intervalar compartilhada.
- Perigo amplia a órbita de graus e a chance limitada de mutação.
- Seção gira o ponto de entrada sem destruir a identidade do padrão.
- Cada lane tem fase própria, mas lê a mesma geometria: relação de cardume sem
  cinco sequências idênticas.
- Seed afeta mutações, nunca a contagem básica de ataques nem os limites.

O padrão é circular: as distâncias entre ataques diferem por no máximo um
passo. Casos extremos, zero ataques e todos os ataques, possuem definição
explícita.

## Performance e modo híbrido

`influence = 0` torna todos os controles de performance inaudíveis e preserva
o percurso autônomo bit a bit. `influence = 1` entrega a intenção inteira; o
intervalo intermediário soma viés ao estado ecológico em vez de trocar de
motor.

- **Rhythm Density:** rarefaz ou preenche ataques.
- **Orbit Rotation:** muda o ponto de contato sem reordenar a geometria.
- **Interval Bias:** desloca o centro melódico em graus, não em Hz arbitrário.
- **Mutation:** aumenta desvios determinísticos limitados; nunca muda seed,
  tamanho da órbita ou memória no callback.

Esses são contratos do core. A superfície JUCE só será criada quando a
integração sonora provar que cada controle merece espaço de performance.

## Limites e provas

- estado fixo, sem alocação, exceção, lock ou I/O no avanço;
- 1–32 passos, 0–32 ataques e rotação normalizada;
- pitches e CVs sempre finitos e limitados;
- mesma seed e eventos produzem resultado idêntico;
- intenção com influência zero não altera o resultado;
- contagem e espaçamento euclidianos testados exaustivamente;
- integração deve preservar a invariância ao tamanho do bloco já provada pela
  bioenergia;
- escuta decidirá se gate e contorno acrescentam forma ou apenas rarefação.

## Deliberadamente fora deste marco

Sync DAW, swing, memória de passos editável, quatro valores persistentes por
passo, endereçamento por CV/limiar, encadeamento 8/16/32, matriz de permutação,
Scala e progressões harmônicas. A API deixa espaço para eles, mas a fundação
será estabilizada antes de ampliar a superfície.
