# Aquorbium — MultiTapSpace

Atualização 2026-09-29: os tempos dos oito taps recebem agora uma modulação
lenta opcional de um `SlewRateRandomWalk` sem alocação. O adaptador ecológico
usa `DRIFT`, densidade e `crackStress` para escolher taxa e profundidade em
faixa sutil; cada tap recebe polaridade e escala fixas diferentes. A matriz de
feedback, o número de taps e os limites de leitura não mudam. `amount=0`
continua neutro, e seed/reset reproduzem a trajetória bit a bit.

## Papel musical

`AquariumSpatializer` situa cada organismo; `MultiTapSpace` dá memória ao
espaço compartilhado. Ele não tenta imitar uma sala identificável. Cada ataque
deixa reflexos com direção, duração e matéria espectral próprias, de modo que o
Aquário começa a lembrar sem ainda depender do futuro gerador de cauda.

O corpo continua deliberadamente **wet-only**. Ele nasceu isolado, recebeu uma
escuta reproduzível e agora existe no `BiomaEngine` como uma única instância
compartilhada pós-soma e pré-calibração. O laboratório independente permanece
para separar caráter, calibração e erro de roteamento.

## Pesquisa apropriada

Filtros de atraso fracionário aproximam instantes de amostragem não inteiros.
Laakso, Välimäki, Karjalainen e Laine organizam FIR/interpolação e all-pass como
famílias diferentes; Välimäki e Laakso mostram que a interpolação linear é
Lagrange de ordem um e tem precisão limitada às baixas frequências. O
MultiTapSpace usa Lagrange de ordem três: quatro leituras contíguas por cabeça,
melhor compromisso entre banda, custo fixo e ausência de estado recursivo
particular em cada interpolador.

Variação brusca de atraso é uma mudança da posição de leitura e pode produzir
transientes. A pesquisa de Välimäki, Laakso e Mackenzie trata explicitamente
esse problema em delays fracionários variantes no tempo. Aqui não copiamos o
método all-pass do artigo: usamos uma rampa de atraso própria, limitada, porque
o objetivo musical aceita o glide/Doppler discreto de uma memória que muda de
tamanho, mas não aceita saltos.

Fontes primárias:

- T. I. Laakso et al., *Splitting the Unit Delay*, IEEE Signal Processing
  Magazine 13(1), 1996, DOI
  <https://doi.org/10.1109/79.482137>;
- V. Välimäki e T. I. Laakso, *Principles of Fractional Delay Filters*,
  ICASSP 2000, DOI <https://doi.org/10.1109/ICASSP.2000.860248>;
- V. Välimäki, T. I. Laakso e J. Mackenzie, *Elimination of Transients in
  Time-Varying Allpass Fractional Delay Filters*, ICMC 1995,
  <http://hdl.handle.net/2027/spo.bbp2372.1995.096>;
- J. Dattorro, *Effect Design, Part 1: Reverberator and Other Filters*, JAES
  45(9), 1997, pp. 660–684,
  <https://freeverb3-vst.sourceforge.io/doc/EffectDesignPart1.pdf>.

Nenhum código dessas fontes foi copiado. Dattorro informa a próxima pesquisa
de cauda difusa; o algoritmo de plate não pertence a este marco.

## Modelo DSP

Uma única memória circular estéreo de `262144` frames recebe a entrada e uma
realimentação cruzada. Oito cabeças leem tempos independentes. Para posição
fracionária `mu` entre as amostras centrais, cada leitura usa o polinômio
Lagrange cúbico:

```text
c[-1] = -mu(mu-1)(mu-2) / 6
c[ 0] =  (mu+1)(mu-1)(mu-2) / 2
c[ 1] = -(mu+1)mu(mu-2) / 2
c[ 2] =  (mu+1)mu(mu-1) / 6
```

A cabeça forma uma perspectiva mono da memória estéreo antes de se localizar
novamente no campo:

```text
source_i = delayedL_i (1-r_i) + delayedR_i r_i
r_i      = 0.5 + 0.35 pan_i
```

Isso não é um downmix global: cada reflexão escolhe outra perspectiva. O filtro
individual é um passa-baixa de um polo,
`y = x(1-a) + y[-1]a`, com `a = exp(-2 pi fc / Fs)`. Depois vêm panorama
seno/cosseno de potência constante e ganho próprio.

### Limite do laço

Os coeficientes Lagrange cúbicos têm soma máxima de módulos `1,25`. A soma de
ganhos usa `B = sum(max(abs(gL_i), abs(gR_i)))`. A saída wet aplica:

```text
Nout = 1 / max(1, 1.25 B)
```

O retorno usa `Nfb = 0.78 / max(0.78, 1.25 B)`, matriz estéreo cruzada com soma
unitária por linha e feedback público limitado a `0,88`. Assim o ganho superior
conservador do laço é `0,78 × 0,88 = 0,6864`, mesmo se todos os taps coincidirem.
Não há limiter ou saturador escondido no corpo.

## Três modos, um só motor

- **Autônomo:** `Parameters` contém o desenho produzido pelo ambiente. O preset
  `aquariumEarlyReflections()` é apenas uma fixture autoral inicial;
- **Performance:** `PerformanceIntent` oferece `size`, `spread`, `tone` e
  `memory`; `influence=1` aplica completamente essa intenção;
- **Híbrido:** `influence` interpola continuamente a escala de tempo, abertura,
  frequência dos filtros e feedback, preservando os oito taps e o mesmo estado.

`size` escala os tempos exponencialmente, `spread` contrai/abre os panoramas,
`tone` transpõe os cutoffs numa faixa de quatro oitavas e `memory` alcança no
máximo o feedback estável. Não há random interno: continuidade ecológica e
seed pertencem ao adaptador do `BiomaEngine`, não ao DSP.

## Integração ecológica

Uma atualização limitada a 200 Hz cria `Parameters` a partir da fixture e do
estado atual do Ecossistema. As rampas do próprio corpo transformam os snapshots
em trajetória contínua:

- temperatura e trinca deslocam o tamanho do campo entre aproximadamente
  `0,92×` e `1,14×`;
- coesão, perigo e trinca movem a abertura entre `0,72×` e `1,08×`;
- luz do dia e saúde escurecem ou abrem os oito filtros;
- densidade e pulso de acasalamento alteram discretamente o ganho dos reflexos;
- calma, densidade e trinca levam o feedback ecológico de uma base `0,22` até
  no máximo `0,50`, ainda abaixo do limite performático de `0,88`.

O retorno autoral começa em `0,36`, soma wet ao sinal direto e possui fade de
`80 ms`; zero é o bypass performático. A chave `setMultiTapSpaceEnabled()` é
somente uma rota técnica de A/B: o corpo continua avançando nos dois lados.

Neste marco, `lastOutputEnergy` continua medindo a soma direta pré-espaço.
Portanto reflexões audíveis não viram por acidente uma segunda realimentação
da homeostase. Conectar deliberadamente o Cérebro ao Espaço permanece outra
tarefa, com ganho e estabilidade próprios.

## Suavização e extremos

- ganho, panorama e polo: constante de tempo `35 ms`;
- feedback: `90 ms`;
- atraso: `140 ms`, com deslocamento adicional limitado a `0,20`
  amostra por amostra;
- atraso mínimo: duas amostras;
- atraso máximo: capacidade menos três amostras, isto é, aproximadamente
  `5,46 s` em 48 kHz, `2,73 s` em 96 kHz e `1,36 s` em 192 kHz;
- ganho por tap `[0, 1]`, pan `[-1, 1]`, cutoff entre `80 Hz` e o menor valor
  entre `22 kHz` e `0,45 Fs`, feedback `[0, 0,88]`;
- parâmetros não finitos preservam o último campo válido; amostra não finita
  entra como zero; contaminação interna reinicia o corpo em silêncio.

## Tempo real, CPU e memória

`tick()` executa exatamente oito taps. Não aloca, bloqueia, acessa arquivos,
usa JUCE nem depende do tamanho de bloco. O objeto ocupa `2.097.776 bytes`
nesta ABI: praticamente todo o custo é a memória estéreo fixa de 262144 frames.
Uma instância compartilhada é a intenção arquitetural; não deve ser multiplicada
por organismo ou voz. `BiomaEngine` reserva essa instância uma vez na sua
construção, antes de `prepare()`/`render()`, evitando colocar 2 MB adicionais
na pilha de hosts menores. Não há alocação depois disso.

Em três rodadas Release, cada uma mediana de cinco renders de 500 ms, a mediana
entre rodadas foi:

| Taxa | Entrada | CPU da janela | Mais rápido que tempo real |
|---:|---|---:|---:|
| 48 kHz | silêncio | 1,78% | 56,06× |
| 48 kHz | estéreo contínuo | 1,63% | 61,38× |
| 96 kHz | silêncio | 2,99% | 33,45× |
| 96 kHz | estéreo contínuo | 3,05% | 32,78× |

A diferença silêncio/excitado está dentro da dispersão de escalonamento do
sistema; a implementação não possui bypass oportunista.

Integrado ao instrumento, a mediana entre três rodadas do benchmark completo
ficou em `3,96%` para autonomia e `9,11%` com quatro vozes em 48 kHz/bloco 128;
em 96 kHz, `8,05%` e `18,51%`. O maior ponto observado na matriz foi `20,64%`,
ainda `4,84×` tempo real no render offline.

## Provas e estado

- suíte Release: 14/14;
- suíte ASan/UBSan: 14/14, com LeakSanitizer desligado porque o runtime sob
  `ptrace` encerra qualquer processo, inclusive testes não relacionados;
- teste próprio: atraso fracionário, pan, filtro, cauda no feedback máximo,
  silêncio, reset bit a bit, macros autônoma/híbrida/performance, extremos,
  valores não finitos e zero alocações;
- resposta wet de 6 s: pico `0,203890`, RMS `0,000562826`, side RMS
  `0,000587751`;
- sonda musical de 16 s: pico `0,363136`, RMS `0,0402481`, side RMS
  `0,0240454`.

Os WAVs recebem ganho de audição explícito `×2,5`, registrado nos relatórios;
o core e os testes de estabilidade continuam usando níveis sem essa compensação.

Artefatos em `validation-output/aquorbium-multitap-space/`. Este marco passa
pelos portões de arquitetura, correção, tempo real e orçamento. A aceitação de
áudio permanece uma decisão de escuta: diferença espacial não será chamada de
melhoria automaticamente.

Na integração autônoma de 30 s, com feedback de saída desligado para preservar
o mesmo percurso ecológico, as rotas produziram os mesmos 101 eventos:

| Rota | RMS | Pico | Delta máximo |
|---|---:|---:|---:|
| reflexos ativos | `0,0236807` | `0,148686` | `0,0829979` |
| reflexos fora da saída | `0,0235820` | `0,146327` | `0,0830910` |

O stem de diferença é exatamente o retorno espacial; recebeu `×12` apenas para
escuta diagnóstica e não representa o nível usado no instrumento. Os arquivos
estão em `validation-output/aquorbium-multitap-integrated-on/` e
`validation-output/aquorbium-multitap-integrated-off/`.

Na retomada de 2026-09-15, os renders Release foram repetidos com a mesma
seed, 30 s/48 kHz e `--feedback-off`. Os novos WAVs em
`validation-output/2026-09-15-reactivation-space-on/` e
`validation-output/2026-09-15-reactivation-space-off/` têm SHA-256 idêntico,
respectivamente, aos WAVs antigos on e off. Isso verifica que os novos
adaptadores de controle da GUI não mudaram o percurso offline padrão.
A escuta humana e a calibração de `Return=0,36` continuam pendentes; o roteiro
curto está em `validation-output/aquorbium-multitap-integrated-on/LISTEN.md`.

## Próximo portão

O marco técnico e a superfície mínima de performance na GUI estão concluídos:
seis controles persistidos (`Return`, `Space Mix` — antes `Gesture`, `Size`, `Spread`, `Tone` e
`Memory`) governam uma única instância compartilhada. Resta calibrar por escuta
o retorno `0,36` e o caráter desses controles. Reverb difuso, random walk dos
tempos e banco de ressonadores continuam módulos posteriores, não controles
fictícios.

Em 2026-09-18, o teste humano reportou estalos e rejeitou a densidade visual.
O cartão agora reserva áreas próprias para cabeçalho/knobs, preserva diâmetro
mínimo de 52 px e usa rolagem local quando necessário. `SPACE MIX=0` mostra
SIZE/SPREAD/TONE/MEMORY inativos, mantendo RETURN independente. A metodologia
e o critério de auditoria estão em `WINDOW_LAYOUT_DESIGN.md`; aprovação
musical e estabilidade com dispositivo real continuam abertas.

## Revisão de presença performática — 2026-09-21

`SPACE MIX` continua interpolando tamanho, abertura, tom e memória, mas
também multiplica a meta de `RETURN` entre 1 e 2. O motor mantém o limite
absoluto de 0,75 e a suavização anterior; `RETURN` continua independente
quando `SPACE MIX=0`. Assim o gesto de mix não fica numericamente ativo
porém quase silencioso quando os outros parâmetros estão neutros. No teste
integrado, a diferença entre `SPACE MIX=0` e `1` passou de cerca de −61 dB
para −21 dB relativamente ao RMS da mistura de referência. O critério mínimo
automatizado agora é −25 dB, apenas uma guarda de presença; profundidade,
caráter, cauda e estabilidade ainda precisam de escuta no dispositivo real.
