# Aquorbium — linha de base de CPU do core

Data: 2026-08-12. Esta medição é uma linha de base para regressão, não uma meta
de produto e não substitui teste de deadline/xrun com dispositivo real.

## Método

- build `Release`, GCC 13.3.0;
- Intel Core i5-6500, 4 núcleos, 3,20 GHz;
- taxas de 44,1, 48 e 96 kHz;
- blocos de 32, 64, 128, 256 e 512 amostras;
- autonomia ativa, zero, uma e quatro vozes externas, Ecossistema Active e
  feedback de energia habilitado;
- aquecimento de 250 ms;
- mediana de cinco renders offline de 500 ms por caso;
- checksum finito obrigatório para impedir uma medição de caminho vazio.

`cpu_percent` representa o tempo gasto para produzir 500 ms de áudio como
fração desses 500 ms. É uma medida de um único caminho de render offline, não o
percentual global mostrado pelo sistema operacional.

## Casos representativos

| Taxa | Bloco | Vozes externas | CPU da janela | Vezes mais rápido que tempo real |
|---:|---:|---:|---:|---:|
| 44,1 kHz | 32 | 0 | 1,46% | 68,27× |
| 48 kHz | 32 | 0 | 1,64% | 61,09× |
| 48 kHz | 128 | 0 | 1,65% | 60,58× |
| 48 kHz | 128 | 4 | 5,29% | 18,89× |
| 96 kHz | 128 | 0 | 3,16% | 31,68× |
| 96 kHz | 128 | 4 | 10,69% | 9,35× |

Na matriz completa de 45 casos com ressonador modal e espacializador, só a
autonomia ficou entre 1,46% e 1,66% em 44,1/48 kHz e entre 3,10% e 3,26% em
96 kHz. Autonomia mais uma voz externa ficou entre 2,30% e 2,66% nas taxas
menores e entre 5,14% e 5,40% em 96 kHz. Com quatro vozes externas, ficou entre
4,86% e 5,55% nas taxas menores e entre 10,50% e 11,62% em 96 kHz. A primeira
implementação recalculava trigonometria a cada amostra e chegou a 8,80% no caso
48 kHz/bloco 128/quatro vozes; cachear a conversão no relógio de controle
reduziu esse caso para 5,29%. A dispersão inclui escalonamento do sistema; a
mediana reduz, mas não elimina esse ruído.

## Verificação após corpo DSP por espécie

A introdução de `OrganismVoiceEngine` retirou o despacho por índice do
`BiomaEngine`. Em três execuções consecutivas, cada ponto ainda medido como
mediana de cinco janelas, o caso `48 kHz / 128 / 0 vozes externas` teve mediana
entre rodadas de `1,67%`; com quatro vozes, `5,62%`; em `96 kHz / 128 / quatro
vozes`, `11,30%`. As linhas anteriores eram respectivamente `1,65%`, `5,29%` e
`10,69%`.

Isso registra pequena diferença possível no caminho polifônico, ainda misturada
à dispersão do sistema. Não foi imposto threshold artificial nem alegado custo
zero. Detalhes arquiteturais e faixas das três rodadas estão em
[`ORGANISM_VOICE_ENGINE_DESIGN.md`](ORGANISM_VOICE_ENGINE_DESIGN.md).

## Verificação após Granular Striker

O segundo Morph duplicado foi substituído por `BiomaGranularStriker`. O corpo
possui 24 slots fixos e, no instrumento atual, existe uma instância dessa
espécie no banco autônomo e em cada uma das quatro vozes externas.

Em três pares sequenciais do benchmark Release, alternando fallback Morph e
Striker sem concorrência entre medições, os casos representativos ficaram:

| Caso | Striker ativo | Fallback Morph | Interpretação |
|---|---:|---:|---|
| 48 kHz / 128 / autonomia | 1,64% mediano | 1,69% mediano | dentro da dispersão |
| 48 kHz / 128 / +4 vozes | 5,56% mediano | 6,02% mediano | dentro da dispersão |
| 96 kHz / 128 / +4 vozes | 11,17% mediano | 11,71% mediano | dentro da dispersão |

O seletor A/B mantém os dois corpos avançando para preservar continuidade;
portanto esses pares medem a escolha de saída, não removem o custo do Striker.
O benchmark dedicado mede cinco corpos diretamente. Inativos consumiram 0,40%
da janela a 48 kHz e 0,72% a 96 kHz. No teste deliberadamente extremo — cinco
corpos com todos os 24 slots ocupados e nova nuvem a cada 128 amostras — o custo
foi 1,03% a 48 kHz e 1,81% a 96 kHz, ainda 96,96× e 55,35× tempo real.
Cada corpo ocupa 1.272 bytes nesta ABI. A condição saturada é um teste de teto,
não um padrão musical pretendido.

## Physical String — medição isolada e integração

`aquorbium_physical_string_benchmark` mede cinco corpos fora do instrumento,
portanto estes números medem apenas o corpo. Em Release, na mesma máquina:

| Taxa | Estado | CPU da janela | Mais rápido que tempo real |
|---:|---|---:|---:|
| 48 kHz | inativo | 0,07% | 1.396,73× |
| 48 kHz | impacto a cada 128 amostras/corpo | 0,65% | 153,46× |
| 96 kHz | inativo | 0,13% | 761,04× |
| 96 kHz | impacto a cada 128 amostras/corpo | 1,28% | 77,95× |

Os valores são as medianas entre três rodadas sequenciais; cada rodada já usa
a mediana de cinco janelas. Cada corpo ocupa 131.280 bytes nesta ABI: o custo
computacional é pequeno, mas a memória das duas linhas fixas é uma decisão
relevante antes de multiplicar o motor por bancos autônomos e vozes externas.
A condição saturada é muito mais densa que o uso musical previsto. A integração
usa exatamente cinco corpos: autonomia + quatro vozes externas, 656.400 bytes,
em vez de 25 cópias dentro de cada despachante.

A alta deliberada do pior caso em relação à primeira versão vem do excitador
janelado e de dois polos: eles retiram o degrau/ruído frontal apontado na escuta
cega. Mesmo nesse cenário irreal de 1.875 impactos por segundo a 48 kHz, cinco
corpos permanecem mais de 153× em tempo real. O custo musical normal foi medido
novamente no instrumento integrado.

## Linha integrada com o quinto organismo

Após a integração e a calibração dos cinco corpos, três execuções Release
sequenciais produziram estas medianas entre rodadas:

| Taxa | Bloco | Vozes externas | CPU da janela | Mais rápido que tempo real |
|---:|---:|---:|---:|---:|
| 48 kHz | 128 | 0 | 2,26% | 44,32× |
| 48 kHz | 128 | 4 | 8,11% | 12,33× |
| 96 kHz | 128 | 0 | 4,58% | 21,86× |
| 96 kHz | 128 | 4 | 15,13% | 6,61× |

Na matriz standalone, o maior ponto observado foi 16,75% em 96 kHz/bloco 512
com quatro vozes externas, ainda 5,97× tempo real no render offline. A suíte integrada
também confirmou zero alocações durante render e passou ASan/UBSan. É uma nova
linha de regressão, não uma garantia de deadline de dispositivo.

## Reproduzir

```sh
cmake -S . -B build-release \
  -DAQUORBIUM_BUILD_APP=OFF \
  -DAQUORBIUM_BUILD_TESTS=ON \
  -DAQUORBIUM_BUILD_TOOLS=ON \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build-release --target aquorbium_core_benchmark
./build-release/aquorbium_core_benchmark
```

A ferramenta imprime CSV com todos os 45 casos. Comparações futuras devem usar
a mesma máquina, compilador, tipo de build e condições de energia. Master,
oversampling e novos motores exigirão outra linha de base e testes de áudio em
tempo real; estes números não constituem orçamento para eles.

### Banco integrado com coorte emergente — 2026-09-28

O benchmark agora aceita `--module-bank-on`, que abre `LAB MIX=0,65` e mantém
as quinze famílias em `0,6`. A comparação sequencial no mesmo build Release
mediu:

| Taxa | Bloco | Vozes | Banco em bypass | Banco ativo | Margem ativa |
|---:|---:|---:|---:|---:|---:|
| 48 kHz | 128 | 0 | 4,49% | 9,56% | 10,46× |
| 48 kHz | 128 | 4 | 8,75% | 13,77% | 7,26× |
| 96 kHz | 128 | 0 | 8,98% | 18,60% | 5,38× |
| 96 kHz | 128 | 4 | 16,86% | 30,66% | 3,26× |

O pior ponto da matriz ativa foi o último da tabela. O banco em bypass retorna
antes de processar os motores emergentes. São medianas offline e não substituem
o teste de alocação, uma sessão prolongada no dispositivo nem os contadores
XRUN/LATE do standalone.

### Banco integrado com coorte avançada de voz e ecologia — 2026-09-28

Após acrescentar os dez mecanismos avançados e ativar a autocaptura granular,
a mesma matriz Release foi repetida. O banco em bypass continua retornando
antes de ambas as coortes.

| Taxa | Bloco | Vozes | Banco em bypass | Banco ativo | Margem ativa |
|---:|---:|---:|---:|---:|---:|
| 48 kHz | 128 | 0 | 4,70% | 13,29% | 7,53× |
| 48 kHz | 128 | 4 | 8,80% | 17,70% | 5,65× |
| 96 kHz | 128 | 0 | 8,72% | 26,01% | 3,85× |
| 96 kHz | 128 | 4 | 21,48% | 34,17% | 2,93× |

O pior caso ativo da matriz foi 35,78% em 96 kHz/bloco 64/quatro vozes,
equivalente a 2,80× tempo real offline. O valor de bypass a 96 kHz apresentou
variação em relação à rodada anterior mesmo sem executar a coorte; por isso os
números devem ser tratados como uma amostra de capacidade, não como regressão
isolada nem garantia de deadline.

### Banco com coorte de geração e interação — 2026-09-29

A terceira coorte acrescenta, entre outros mecanismos, um vocoder de seis
bandas e espacialização binaural leve. A comparação sequencial no mesmo build
Release mediu:

| Taxa | Bloco | Vozes | Banco em bypass | Banco ativo | Margem ativa |
|---:|---:|---:|---:|---:|---:|
| 48 kHz | 128 | 0 | 4,62% | 13,92% | 7,19× |
| 48 kHz | 128 | 4 | 10,38% | 23,24% | 4,30× |
| 96 kHz | 128 | 0 | 9,16% | 34,68% | 2,88× |
| 96 kHz | 128 | 4 | 16,39% | 35,64% | 2,81× |

O pior ponto ativo foi 41,69% em 96 kHz/bloco 32/quatro vozes, margem de
2,40×. O custo é aceitável para o laboratório, mas claramente maior que na
coorte anterior. `LAB MIX=0` continua retornando antes das três coortes; as
variações do baseline sem banco são dispersão entre rodadas. Medição offline
não garante deadline, XRUN ou estabilidade do driver.

### Coorte de integração especializada — 2026-09-29

Com Pulsar ecológico, soft take-over, feedback da saída final, parâmetros
granulares por traço, subgrave quantizado e deriva do MultiTap ativos, a mesma
matriz Release mediu 19,21% em 48 kHz/bloco 128/quatro vozes, equivalente a
5,20× tempo real offline. O pior ponto observado foi 38,92% em
96 kHz/bloco 64/quatro vozes, margem de 2,57×. A variação favorável em relação
à rodada anterior está dentro da dispersão esperada do sistema e não deve ser
anunciada como otimização causada pela coorte.

### Master criativo com banco integrado — 2026-09-30

O benchmark aceita agora `--creative-master-on`; nesta rodada ele foi usado
junto de `--module-bank-on`, com `CREATIVE MASTER=0,70`. A coluna
`creative_master` do CSV torna o cenário explícito.

| Taxa | Bloco | Vozes | CPU | Margem |
|---:|---:|---:|---:|---:|
| 48 kHz | 128 | 0 | 18,74% | 5,34× |
| 48 kHz | 128 | 4 | 21,57% | 4,64× |
| 96 kHz | 128 | 0 | 32,45% | 3,08× |
| 96 kHz | 128 | 4 | 41,45% | 2,41× |

O pior ponto da matriz foi 42,95% em 96 kHz/bloco 32/quatro vozes, margem de
2,33×. A cadeia adicionou custo limitado à última linha de base ativa, mas a
comparação entre rodadas inclui dispersão do sistema. Esses valores offline
não garantem deadline, estabilidade do driver ou ausência de XRUN.

### Refinamento de core — 2026-10-02

Após integrar avaliação 4× do sine-folder, captura real da Lupa, meio ar/água
e feedback do Master limitado, a matriz Release completa foi executada com
`--module-bank-on --creative-master-on`. O benchmark usa quinze famílias em
0,6, `LAB MIX=0,65` e influência do Master 0,70.

| Taxa | Bloco | Vozes | CPU | Margem |
|---:|---:|---:|---:|---:|
| 48 kHz | 128 | 0 | 22,28% | 4,49× |
| 48 kHz | 128 | 4 | 29,69% | 3,37× |
| 96 kHz | 128 | 0 | 62,08% | 1,61× |
| 96 kHz | 128 | 4 | 56,41% | 1,77× |
| 96 kHz | 32 | 4 | 57,11% | 1,75× |

O pior caso da matriz foi 62,08% em 96 kHz/bloco 128/sem vozes externas;
isso é superior em custo ao pior caso da rodada anterior, embora as rodadas
offline sofram dispersão de CPU e temperatura. A margem mínima medida foi
1,61×. O banco em bypass continua fora da rota de processamento das famílias.
Esta medição não confirma prazo de callback, ausência de XRUN nem estabilidade
do dispositivo; isso exige teste no app JUCE em hardware real. A configuração
de build utilizada não incluiu JUCE.

## MultiTapSpace isolado e integrado

`aquorbium_multitap_space_benchmark` mede uma instância com oito taps
sempre ativos, interpolação Lagrange cúbica e feedback. Em três rodadas Release,
cada ponto já medido como mediana de cinco janelas, a mediana entre rodadas foi
`1,78%` em silêncio e `1,63%` excitado a 48 kHz; a 96 kHz, `2,99%` e `3,05%`.
O corpo ocupa `2.097.776 bytes` nesta ABI.

A integração mantém uma única instância pós-soma. Em três rodadas da matriz
completa, cada ponto já medido como mediana de cinco janelas, a mediana entre
rodadas foi:

| Taxa | Bloco | Vozes externas | CPU da janela | Mais rápido que tempo real |
|---:|---:|---:|---:|---:|
| 48 kHz | 128 | 0 | 3,96% | 25,26× |
| 48 kHz | 128 | 4 | 9,11% | 10,97× |
| 96 kHz | 128 | 0 | 8,05% | 12,42× |
| 96 kHz | 128 | 4 | 18,51% | 5,40× |

O maior ponto observado foi `20,64%` em 96 kHz/bloco 512/quatro vozes, ainda
`4,84×` tempo real offline. O retorno desligado tecnicamente continua calculando
o corpo para preservar estado, portanto não é um benchmark honesto de custo
antes/depois; a diferença deve ser comparada à linha integrada anterior e ao
benchmark isolado.
