# Aquorbium — dossiê do Granular Striker

## Problema musical

O Aquorbium já possui continuidade tonal (`BiomaMorphOscillator`), articulação
por pulsarets (`BiomaPulsar`) e um corpo modal de cauda longa
(`BiomaModalResonator`). Falta uma espécie que transforme um único acontecimento
ecológico em uma pequena população de impactos: som descontínuo, microscópico e
material, capaz de ocupar a fronteira entre clique, ruído e altura.

`BiomaGranularStriker` será essa fonte. Ele não simula um objeto conhecido e não
é um reprodutor de amostras. Cada excitação cria uma constelação determinística
de partículas sintéticas, cada uma com duração, atraso e frequência próprios.
O resultado deve lembrar matéria se reorganizando no aquário, não uma caixa de
ritmos escondida.

## Fontes e apropriação conceitual

- Curtis Roads descreve o grão por duração, densidade, faixa de frequência,
  envelope e dispersão, e mostra que a redução da duração alarga o espectro e
  conduz à percepção de clique. Fonte primária do autor:
  <https://www.curtisroads.net/articles-1>.
- O projeto PhISEM de Perry Cook usa eventos estocásticos e ressonâncias para
  sintetizar famílias de partículas e percussões de maneira interativa. Fonte
  primária do Princeton Sound Lab:
  <https://soundlab.cs.princeton.edu/research/phymod/>.
- As notas do Princeton Sound Lab distinguem modelo de partícula, excitação por
  ruído e ressonância como blocos paramétricos de síntese física informada:
  <https://soundlab.cs.princeton.edu/learning/tutorials/phys.html>.

Essas referências fundamentam o vocabulário, não a implementação. Nenhum código,
coeficiente proprietário, preset ou topologia completa será copiado. O motor é
desenhado para o contrato ecológico e de tempo real do RASGO.

## Modelo DSP do primeiro marco

Uma excitação de intensidade `A` agenda até doze partículas em um pool fixo de
vinte e quatro. Para a partícula `i`:

1. um atraso `d_i` a espalha dentro de uma janela curta;
2. a duração `T_i` varia no intervalo musical de 2–40 ms;
3. ruído branco determinístico é multiplicado por um envelope de ataque
   instantâneo e queda exponencial até aproximadamente −60 dB;
4. um passa-banda ressonante estável dá centro espectral próprio à partícula;
5. todas as partículas são somadas com normalização pela raiz do número criado.

A frequência de cada partícula parte de `centerHz` e recebe uma dispersão
exponencial simétrica. Isso preserva razões de altura ao transpor. O Q varia de
20 a 100, calculado somente ao criar a partícula; não há trigonometria por
amostra. Um limitador suave finito protege a saída sem converter o ganho interno
em clipping duro.

O pool comporta duas nuvens máximas simultâneas. Quando está cheio, uma nova
partícula substitui a de menor energia prevista. A ocorrência é contada em
telemetria: sobrecarga nunca fica invisível.

## Parâmetros generativos

| Parâmetro | Faixa | Consequência musical |
|---|---:|---|
| `centerHz` | 20 Hz–limite seguro | centro transponível da matéria |
| `grainDurationMs` | 2–40 ms | clique seco → fragmento mais definido |
| `particleDensity` | 0–1 | 2–12 partículas por golpe |
| `spectralSpread` | 0–1 | corpo concentrado → estilhaço largo |
| `resonance` | 0–1 | Q 20–100, poeira → metal focalizado |
| `irregularity` | 0–1 | impacto compacto → constelação dispersa |

Cada nuvem congela um snapshot desses parâmetros. Alterar intenção não remodela
partículas que já nasceram e, portanto, não cria saltos de coeficientes no áudio.

## Autonomia, performance e modo híbrido

- **Autônomo:** um `lifeEvent` geométrico do organismo cria a nuvem. Altura vem
  do campo harmônico; energia, traços do organismo, ambiente e integridade
  definem força, duração, densidade, dispersão, ressonância e irregularidade.
- **Performance:** uma nota externa excita exatamente o mesmo motor. Pitch e
  velocity deslocam centro e energia; não existe caminho DSP MIDI separado.
- **Híbrido:** `PerformanceIntent::influence` interpola os macros generativos no
  próximo golpe. Influência zero deve preservar a sequência autônoma bit a bit.

### Superfície performática

A interface não expõe os cinco parâmetros internos como engenharia de filtro.
`GranularPerformance` traduz quatro gestos normalizados:

| Gesto | Função |
|---|---|
| `STRIKER MIX` (antes `DEPTH`) | mistura entre personalidade ecológica e intenção humana |
| `MATTER` | poeira breve/baixa ressonância → metal longo/focalizado |
| `POP` | quantidade de partículas no próximo impacto |
| `SCATTER` | dispersão espectral e irregularidade temporal |

`MATTER` usa smoothstep antes de mapear duração e ressonância, preservando os
extremos com maior resolução ao redor da transição poeira/metal. Os gestos são
publicados por atomics lock-free e aplicados no início do callback. Partículas
que já nasceram congelam seus coeficientes: mover controles não retuna uma
cauda existente; a intenção passa a valer no próximo impacto.

Os quatro macros vivem dentro do cartão Síntese, não numa área global sem
contexto. A inspeção histórica em 1280×800, 1024×650 e 960×600 não certifica
novos controles: em 2026-09-18 houve relato de knobs pequenos e sobreposição.
A revisão segue os mínimos e a auditoria de `WINDOW_LAYOUT_DESIGN.md`, com
rolagem local. `STRIKER MIX=0` mostra os outros três macros inativos. Os valores
são persistidos junto às demais preferências e restaurados com validação em
`[0,1]`; a execução automatizada usa `--settings-dir` e não contamina as
preferências normais.

## Integração ecológica

O segundo organismo, antes um duplicado `Oscillator`, passou a
`Species::GranularStriker`. A formação sonora fica composta por quatro corpos
distintos: Morph, Granular, Modal e Pulsar. O índice continua sendo posição
inicial; o despacho permanece exclusivamente pela espécie.

`OrganismVoiceEngine` possui o motor e o mantém avançando mesmo no fallback
técnico. Um interruptor de validação pode comparar Granular com Morph sem
reiniciar relógios ecológicos ou fases.

## Limites e contrato de tempo real

- memória fixa: 24 partículas por corpo;
- nenhum arquivo, amostra, JUCE, lock, alocação ou chamada virtual;
- trabalho por amostra limitado ao pool; partículas inativas custam apenas um
  teste de estado;
- estado aleatório local e seed explícita;
- valores não finitos preservam o último parâmetro válido;
- filtros são reinicializados quando uma partícula é reutilizada;
- saída sempre finita e limitada a `(-1, 1)`.

Com quatro vozes externas mais autonomia, existem no máximo cinco corpos
Granular, ou 120 slots. Esse é o pior caso desta etapa e será medido no benchmark.

## Alternativas adiadas ou descartadas

- **Granulação de amostra/autocaptura:** separada do Striker. Um protótipo
  próprio, `BiomaGranularCapture`, foi codado em 22/09, com buffer circular,
  onset e grãos estéreo, mas continua isolado; integrá-lo agora impediria
  avaliar o sintetizador de partículas e a estabilidade do app. Ver
  `GRANULAR_CAPTURE_DESIGN.md`.
- **Percussão curta por amostra:** `SamplePercussionVoice` foi codada em
  24/09 como fallback isolado de quatro slots, com pitch e decay. Ela ainda
  não substitui nem complementa o Striker no app; primeiro precisa provar por
  escuta uma função que não duplique os golpes sintéticos já existentes. Ver
  `PERCUSSION_AND_HARMONIC_MORPH_DESIGN.md`.
- **Novo banco modal compartilhado:** descartado nesta fonte. O organismo modal
  já cumpre a função de corpo ressonante longo; o Striker deve conservar cauda
  curta e multiplicidade interna.
- **Uma alocação por grão:** descartada por incompatibilidade com tempo real.
- **Aleatoriedade global:** descartada; quebraria reprodutibilidade e
  independência entre organismos/vozes.

## Portões de aceitação

1. Produz energia somente após excitação e volta ao silêncio.
2. Mesma seed e eventos geram os mesmos bits; seeds diferentes divergem.
3. Todos os extremos e entradas não finitas permanecem finitos e limitados.
4. Densidade altera a contagem real de partículas; duração e dispersão alteram
   medidas observáveis, não apenas telemetria.
5. Excitações sobrepostas e saturação do pool não corrompem o estado.
6. Influência performática zero preserva o autônomo bit a bit.
7. A suíte de alocação em render continua passando com a espécie ativa.
8. Render isolado A/B e benchmark antes/depois são registrados sem declarar que
   diferença sonora equivale automaticamente a melhoria.

## Perguntas de escuta

- O golpe é reconhecido como população de partículas, não como um único tom?
- Duração curta e longa são duas matérias úteis, sem virar apenas mais volume?
- A dispersão produz largura espectral legível sem mascarar os demais seres?
- O quarto corpo acrescenta contraste ao aquário sem destruir profundidade?
- Em autonomia, os eventos parecem consequência de vida ou padrão de bateria?

## Evidência do marco

- `aquorbium_granular_striker_tests` cobre silêncio/cauda, seed, duração,
  densidade, dispersão, parâmetros inválidos, 200 impactos sobrepostos,
  saturação do pool, influência performática e taxas de 8 a 768 kHz.
- `aquorbium_organism_voice_tests` prova o despacho Granular e seu fallback.
- `aquorbium_autonomous_energy_tests` prova excitação real por eventos de vida,
  determinismo entre blocos e o mapa Morph/Granular/Modal/Pulsar.
- `aquorbium_realtime_tests` mantém zero alocações com autonomia e quatro vozes
  externas simultâneas, inclusive alterando os quatro gestos a cada bloco.
- O render autônomo de 30 s, sem nota externa e com feedback desligado, registrou
  seis nuvens, RMS `0,008274`, pico `0,042264`, delta máximo `0,037509` e nenhum
  NaN, infinito ou denormal. O fallback comparativo recebeu a mesma seed e os
  mesmos 81 eventos ecológicos.
- Cinco corpos saturados, com 24 partículas ativas e nova nuvem a cada 128
  amostras, mediram 1,03% de uma janela a 48 kHz e 1,81% a 96 kHz. O estado de
  cada corpo ocupa 1.272 bytes nesta ABI.

Arquivos de escuta reproduzíveis:

- `validation-output/aquorbium-granular-on/AQUORBIUM_AUTONOMOUS.wav`;
- `validation-output/aquorbium-granular-off/AQUORBIUM_AUTONOMOUS.wav`.

Os dois sons são tratados como identidades diferentes. As medidas demonstram
segurança e diferença audível; não afirmam superioridade estética.

Três caracteres isolados, com a mesma seed, nove impactos e alturas idênticas,
estão em `validation-output/aquorbium-striker-characters/`:

- `dust`: Matter baixo, pouca população e Scatter alto;
- `swarm`: população alta e matéria intermediária;
- `metal`: Matter alto e dispersão concentrada.

Eles são referências reproduzíveis do espaço performático, não presets oficiais
nem uma hierarquia estética.

## Revisão de presença performática — 2026-09-21

No app, `STRIKER MIX` continua interpolando a matéria dos próximos impactos,
mas agora também eleva somente a saída acústica dessa espécie de 1 a 8 vezes.
Uma rampa de 15 ms evita salto de ganho durante o gesto; influência zero
preserva o balanço autônomo. Não foram alteradas partículas, excitações ou
ganhos das outras espécies. No teste integrado de seed 42 e notas idênticas,
as diferenças entre extremos dos quatro controles passaram de cerca de
−45/−51 dB para −27/−33 dB relativamente ao RMS da mistura de referência.
O cenário automatizado de quatro vozes e Espaço máximo teve pico `0,535761`,
abaixo do portão de 0,95; isso não cobre todas as performances possíveis nem substitui
escuta humana. Critério numérico mínimo de −35 dB foi acrescentado para
esses quatro controles no teste integrado. Próximo portão: ouvir se Matter,
Pop e Scatter representam caracteres distintos sem transformar o bioma em
uma sequência de impactos dominantes.
