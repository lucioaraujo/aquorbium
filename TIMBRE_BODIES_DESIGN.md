# Aquorbium — corpos dos organismos e timbre ecológico

Criado em 2026-10-20. O autor:
- "só há sons sintéticos, não há variação de timbres, precisamos de módulos
  que trabalhem essa variação sonora";
- "não consigo mudar os timbres".

A escolha do autor, feita entre três caminhos: **corpos por organismo** e
**timbre que vem da ecologia**. O material gravado pelo autor ficou para
depois.

## O diagnóstico

Cada espécie tinha um único corpo de DSP, fixo:
- o Oscillator, o morph;
- o Pulsar, o pulsar;
- o Resonator, o modal;
- o String, a corda;
- o Striker, os grãos.

Nenhum controle trocava o timbre.

## O que existe agora

`src/core/TimbreBody.h/.cpp`: um **segundo corpo** por organismo, misturado
ao nativo (MISTURA) e com um controle de caráter (MATÉRIA):

| Corpo | Como soa | MATÉRIA | Fonte (teoria, sem código copiado) |
|---|---|---|---|
| voz | fonte glotal por três formantes, vibrato lento; com os glissandos da música ecológica, canta como baleia | as vogais [u]-[ɔ]-[ɑ]-[ɛ]-[i] | Peterson & Barney (1952); SVF trapezoidal (Zavalishin; Simper 2013) |
| matéria | o ressonador modal do próprio projeto | modos harmônicos (madeira, tubo) → razões de membrana, mais longo e brilhante | `BiomaModalResonator` |
| sopro | o ar pelas ressonâncias de um tubo, com um tom suave na fundamental | quanto ar | tubo ressonante (Cook 1992, como conceito) |
| corda | guia de onda | dedilhada → friccionada | Karplus & Strong (1983); Jaffe & Smith (1983) |
| gota | gotas d'água: senoides cuja altura sobe enquanto decaem | gotas → chuva | Minnaert (1933); van den Doel (2005) |
| dobra | senoide dobrada sobre si, limitada pela altura | profundidade da dobra | Buchla 259, como no SHAPE do RASGO_MODULAR (conceito) |

**TIMBRE ECOLÓGICO** (`BiomaEngine::updateTimbreTargets`, a 200 Hz) desloca
a MATÉRIA e a presença de cada corpo. É uma tendência:
- **a água:** o gelo endurece e clareia; a água fria amacia; a fervura
  aguça; a turbulência faz a matéria oscilar;
- **a teia:** o caçador aguça e vem à frente; a presa amacia (vira sopro);
  o par em corte vai ao meio da faixa (a vogal aberta); rivais e guardas
  endurecem;
- **o vigor** clareia; **o vidro danificado** escurece.

**Garantias:**
- todos os corpos em "nativo", com MISTURA e ECOLOGIA em zero, dão o som
  anterior, bit a bit (testado);
- a troca de corpo tem rampa de 15 ms e não estala;
- a memória é fixa, sem alocação, trava nem I/O.

**No app,** o cartão **09 TIMBRE** traz, por organismo, o seletor de corpo,
a MATÉRIA e a MISTURA, e embaixo o TIMBRE ECOLÓGICO. No layout amplo ele
fica abaixo da GEOMETRIA; nas telas menores, na coluna da direita. São 16
parâmetros persistidos, e as cenas antigas abrem com os corpos nativos.

### Mais três corpos e o nivelamento (2026-10-20, segunda rodada)

| Corpo | Como soa | MATÉRIA | Fonte |
|---|---|---|---|
| membrana | seis modos da membrana circular (razões dos zeros das funções de Bessel J0–J3), baqueta macia, a afinação cede logo após o golpe | tambor d'água solto e abafado → pele esticada que ressoa | Rayleigh, *The Theory of Sound* |
| vidro | parciais inarmônicos, cada um um par que bate, inchando devagar | brilho e velocidade do batimento | Rossing, *The Science of Sound* (harmônica de vidro) |
| coro | três vozes levemente desafinadas, cada uma com seu vibrato, pelas mesmas formantes | as vogais | o efeito coral |

- **Nivelamento:** cada corpo foi medido no motor contra o nativo numa
  estação com a música ecológica. Antes, os desvios iam de −5,2 a +2,4 dB,
  e a corda ficava a −12,2 dB. Depois, todos ficam a ±0,1 dB, e a corda a
  −5,3 dB, de propósito, para que o pico do dedilhado fique contido.
- **A escolha do corpo** agora é o parâmetro `timbreKind*`, codificado como
  índice / 15 (espaço para 16 corpos). As cenas salvas com o `timbreBody*`
  (índice / 6) são migradas sem perder a escolha (testado com os valores
  salvos pelo autor).

## Validação (2026-10-20)

- `tests/TimbreBodyTests.cpp` cobre:
  - os seis corpos × três MATÉRIAS × três alturas, com valores finitos, nível
    comparável (0,03–0,3 RMS) e pico < 3;
  - a MATÉRIA audível e o determinismo;
  - a troca sem estalo;
  - no motor: o bypass bit a bit, os corpos audíveis e abaixo do fundo de
    escala, e a MATÉRIA que se move com a ecologia (0,46 → 0,85 em 60 s).
- A suíte passou: 51/51.
- O render está em `validation-output/timbres-2026-10-20/`
  (`corpos_e_timbre_ecologico.wav`): pico 0,14, sem estalos, centroide
  1,2–3,3 kHz por janela, contra 1,3–2,3 kHz sem os corpos.
- A aprovação musical depende da escuta do autor.

## Próximos

1. A escuta e o ajuste de nível de cada corpo.
2. Mais corpos, conforme a escuta, como membrana, vidro tocado e coral.
3. A opção adiada: grãos de gravações do próprio autor (hidrofone, voz,
   instrumentos acústicos).


## Arco e palheta — 2026-10-21

O autor pediu para avançar nos módulos que ainda não existiam. O FAUST
`physmodels.lib` serviu só como referência de método. Os dois corpos são
reescritos da física, sem código de terceiros.

**ARCO, a corda friccionada** (McIntyre, Schumacher & Woodhouse 1983;
Smith, *Physical Audio Signal Processing*):
- **O modelo:** a corda são duas linhas de ida e volta, uma de cada lado do
  arco. O arco se acopla a ela por uma curva suave de atrito: gruda
  enquanto a corda anda com ele e escorrega quando a diferença de
  velocidade cresce. O movimento de Helmholtz nasce da física.
- **MATÉRIA:** move o arco de sul tasto (β = 0,18, doce) a sul ponticello
  (β = 0,08, brilhante). A pressão sobe perto do cavalete, como pede o
  diagrama de Schelleng (1973): abaixo de uma força mínima, a corda
  escorrega duas vezes por período e salta para a oitava. Isso foi
  medido, e é o "double slip".
- **O nível:** o que a pressão acrescenta (+6,5 dB) é compensado, para
  que a MATÉRIA mude a cor e não o volume.

**PALHETA, um tubo fechado na palheta e aberto na campânula, só com
harmônicos ímpares, como um clarinete** (Smith, PASP, "Single-reed
instruments"):
- **O modelo:** meia-período de linha, a campânula invertendo por uma média
  de dois pontos, e a palheta como uma reflexão que depende da pressão
  através dela.
- **A pressão da boca:** fica logo acima do limiar de oscilação (~0,5,
  medido) enquanto o organismo tem energia. Acima de certo ponto a palheta
  se fecha e cala, como uma palheta soprada forte demais.
- **MATÉRIA (a embocadura):** o tom que irradia da campânula, de ~500 Hz a
  ~6 kHz, fora do laço.

**Correção de fidelidade feita junto:** o comentário do `breath` prometia
um guia de onda com jato que o código nunca teve. Agora ele descreve o que
o código faz: ruído pelas ressonâncias de um tubo. O guia de onda é a
palheta.

**O que a medição ensinou:**
- A interpolação linear do atraso fracionário é um passa-baixas cuja força
  depende da fração. O terceiro harmônico da palheta ia de −13 dB a 110 Hz
  a −57 dB a 220 Hz. Os dois corpos passaram a usar a interpolação por
  passa-tudo de primeira ordem (Jaffe & Smith 1983).
- Uma varredura sem janela tinha apontado harmônicos que eram vazamento
  espectral. As medições daqui usam janela de Hann.
- O atraso do filtro do cavalete foi compensado exatamente: 0,82 amostra,
  menos 0,15 medidos na junção.

**Medido e testado:**
- **Arco:** afinado a ±0,7 cent e com espectro de dente-de-serra (H2 ≈
  −6 dB) em todas as posições e notas, de 82 a 660 Hz.
- **Palheta:** afinada a ±0,2 cent, com os harmônicos ímpares de uma onda
  quadrada (H3 −10, H5 −14 dB) e os pares abaixo de −44 dB.
- **Sem energia:** o arco para e a palheta não é soprada.
- **No motor completo:** os dois foram nivelados ao corpo nativo (0,0 dB).
- **Os testes gerais dos corpos** (sem clique na troca, saída finita, a
  MATÉRIA muda o som, determinismo) cobrem os dois.

**Escutas:**
- estão em `AUDIOS_RASGO/Aquorbium/escutas/2026-10-21-arco-palheta/`:
  nativo, arco-tasto, arco-ponticello, palheta-escura e palheta-brilhante,
  com a mesma semente, 40 s, float de 32 bits;
- loudness integrado de −32,5 LUFS (nativo) e de −33,1 a −35,4 LUFS para os
  outros (medido pelo ffmpeg ebur128).


## Arco e palheta, v2 — 2026-10-21

O autor escutou as primeiras escutas: "ainda sintético demais"; "não lembra
um clarinete"; e das bolhas: "dá para escutar bem". A física estava certa
(afinação, Helmholtz, ímpares), mas faltavam os sinais que o ouvido usa
para reconhecer um instrumento real.

**Medido antes:**
- a palheta tinha os harmônicos pares a −80 dB e o ruído a −95/−106 dB
  entre os parciais, uma onda quadrada perfeita até o infinito;
- o arco tinha ruído a −62/−76 dB e o espectro cru da força no cavalete.

**O que entrou:**
- **ARCO:**
  - o corpo do instrumento: seis ressonâncias fixas, que não seguem a nota
    (o modo de ar A0 ~280 Hz, CBR ~410, B1− ~460, B1+ ~540, a região de
    1 kHz e a "bridge hill" ~2,5 kHz; Jansson, *Acoustics for Violin and
    Guitar Makers*, KTH), do tamanho de um violoncelo no grave ao de um
    violino no agudo;
  - o vibrato atrasado, ~5,5 Hz e ~5 cents, que cresce ~0,4 s depois de
    cada ataque;
  - a irregularidade da velocidade do arco;
  - o ruído do breu no escorregamento e o chiado da crina, calibrados para
    ~−41 dB entre os parciais.
- **PALHETA:**
  - a rede de orifícios: um corte fixo, de 1 kHz (embocadura fechada) a
    2,2 kHz (aberta), em dois polos, acima do qual os parciais caem
    (Benade, *Fundamentals of Musical Acoustics*);
  - a campânula, que irradia mal os graves (passa-altas ~300 Hz);
  - o ataque de língua (~30 ms a cada nota);
  - a pressão da boca que varia ±3%;
  - o ruído de sopro na banda do jato (~2,5 kHz), que sobe e desce com a
    abertura da palheta, calibrado para ~−45 dB entre os parciais.

**Tentado e descartado:** uma curva de palheta assimétrica, para os pares
fracos de um clarinete real, quebrou a auto-oscilação (com 0,04 o
espectro se desfez; com 0,08 a palheta parou). Ficou registrado no
código.

**Testes:** a física (Helmholtz, ímpares) é verificada no sinal de
excitação, antes do corpo e da radiação, que moldam o espectro de
propósito. A afinação é medida na média de 1,5 s, porque o vibrato e o
arco a fazem oscilar em volta da nota, como num instrumentista: ±0,8 cent
na média longa.

**Nível:** os dois foram renivelados ao corpo nativo no motor (0,0 dB).

**Escutas v2,** em `AUDIOS_RASGO/Aquorbium/escutas/2026-10-21-arco-palheta/`:
- no ecossistema, os mesmos quatro ajustes com o sufixo `_v2`, para o A/B
  com as primeiras;
- `aquorbium_arco-solo_v2.wav` e `aquorbium_palheta-solo_v2.wav`: só o
  corpo, fora do ecossistema, numa frase em ré menor tocada duas vezes
  (MATÉRIA 0,2 e depois 0,8), para julgar o timbre sem a água nem o
  espaço.


## Arco e palheta v2 revertidos — 2026-10-21

O autor escutou as v2: "ficou pior", "tudo rachado", e "a palheta solo v2
não está boa". As v2 foram revertidas para a v1 (commit `5c4d4a5`).

**O que a medição mostrou:**
- nenhuma saturação nas escutas;
- nenhum clique digital: os eventos impulsivos não se alinham com os
  blocos de 256 amostras nem com o controle de 480; são conteúdo agudo
  comum, presente também no nativo e na v1.

**A explicação mais provável para o "rachado":** os ruídos acrescentados
na v2 entravam pulsados a cada período da nota (o breu só no
escorregamento, o sopro só quando a palheta abre). A ~−41/−45 dB entre os
parciais, isso dava uma textura áspera e quebrada.

**Lição para os próximos corpos:**
- realismo não se ganha somando ruído;
- escutar o corpo solo antes de colocá-lo no ecossistema;
- mudar uma coisa de cada vez (o corpo de ressonâncias, o vibrato, o
  ruído) e entregar o A/B de cada uma, em vez de um pacote.

As escutas v2 ficam em `AUDIOS_RASGO/Aquorbium/escutas/2026-10-21-arco-palheta/`
como registro do que não funcionou.
