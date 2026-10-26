# Aquorbium — estágio técnico de saída

Atualizado em 2026-10-01.

## Função e posição

`OutputStage` é a proteção técnica final do standalone. A ordem atual é:

```text
organismos → Ambiente → primeiras reflexões → Cauda → calibração ×5
            → MasterChain criativo → OutputStage → dispositivo/captura
```

O Master criativo inicial entrou antes de `OutputStage`. O estágio de saída
não recebe modulação ecológica, não decide forma e não substitui a cadeia
criativa. A energia de homeostase permanece
medida antes da calibração e da proteção para que o limiter não apague a
dinâmica observada pelo Ecossistema.

## Processamento

Por amostra e sem alocação, lock ou I/O:

1. valores não finitos viram zero e incrementam contador saturado;
2. passa-altas de um polo em 10 Hz remove DC e infrassom extremo;
3. trim e mute recebem rampa de 15 ms;
4. um detector estéreo vinculado estima picos entre amostras por interpolação
   Catmull-Rom em quatro fases;
5. o áudio percorre uma linha fixa de lookahead de 5 ms;
6. a redução entra antes do pico e recupera segundo `RELEASE`;
7. o sinal pós-proteção alimenta medição de loudness somente diagnóstica.

Faixas públicas:

| Controle | Faixa | Inicial |
|---|---:|---:|
| `OUTPUT TRIM` | -18 a +6 dB | 0 dB |
| `CEILING` | -6 a -0,1 dB | -1 dB |
| `RELEASE` | 50 a 500 ms | 150 ms |

O mute existe no core e é suavizado, mas ainda não possui controle no painel.
A latência é explícita em amostras (`240` em 48 kHz) e aparece na telemetria;
o standalone ainda não oferece perfis alternativos nem compensação de host.

## Telemetria

O snapshot fornece sample peak de entrada e saída, pico intersample estimado,
RMS, redução de ganho, contagem de amostras inválidas, latência, latch de clip
e LUFS momentary, short-term e integrated. Os acumuladores podem ser limpos
fora do processamento.

`LoudnessMeter` aplica ponderação K derivada da taxa de amostragem corrente.
Energias de 100 ms compõem a janela momentânea de 400 ms, a janela curta de
3 s e os blocos integrados com 75% de sobreposição. O integrado usa porta
absoluta em -70 LUFS e relativa em -10 LU. Um histograma fixo de 0,1 LU evita
alocação, crescimento de memória e custo proporcional à duração da sessão.
A quantização só participa da decisão da porta; a média final usa as energias
acumuladas. O medidor não aplica ganho nem normalização.

As leituras atravessam a ponte de telemetria por escalares atômicos. M/S/I
aparecem no tooltip do botão ÁUDIO; a auditoria de runtime registra os três
valores e a quantidade de blocos observados.

`estimatedTruePeak` é deliberadamente nomeado como estimativa. A interpolação
4× atual é útil para proteção e regressão, mas não é um detector true peak
polifásico validado por fixtures ITU-R BS.1770. Portanto:

- não declarar conformidade dBTP;
- não fechar as tarefas de true peak ou oversampling;
- validar true peak futuramente contra fixtures externas e novamente após
  codificação de arquivo;
- manter margem de teto até essa conformidade existir.

## Evidência de loudness — 2026-10-01

- referência EBU Tech 3341: estéreo idêntico a -26 dBFS RMS por canal mediu
  -23 LUFS, com tolerância de 0,12 LU;
- âncora mono em 1 kHz, soma estéreo de +3,01 LU e variação de amplitude de
  +6 LU aprovadas;
- 44,1, 48 e 96 kHz mantiveram a leitura dentro de 0,16 LU;
- silêncio, cauda silenciosa, portas, janelas, valores não finitos e reset
  aprovados;
- integração pós-safety no `OutputStage` aprovada em executável leve separado;
- checagem sintática com `-Wall -Wextra -Wpedantic -Werror` aprovada para os
  arquivos afetados;
- build CMake, suíte integral, teste global de zero alocação e benchmark ainda
  não executados neste marco: o preflight encontrou 1,7/2 GiB de swap usados e
  outra sessão de desenvolvimento ativa, acima do limite da governança.

## Gravação (REC) — 2026-10-21

O autor perguntou "há um dispositivo de rec? onde são salvos os arquivos?"
e pediu "excelência de áudio", conferindo o que já fizemos no Navalha, no
Antitotem e nos outros instrumentos.

**O que se grava:**
- o sinal que se ouve: depois do `OutputStage` (guarda de finitos, DC,
  limitador com lookahead e teto em true peak), do MASTER e do SILENCIAR;
- em WAV IEEE float de 32 bits, estéreo, na taxa do dispositivo;
- o float é a cópia-mestra: não quantiza e não precisa de dither. É a
  mesma política do Navalha (`DITHER_TPDF.md`): o dither TPDF só vale para
  formatos inteiros e é uma questão de codificação, não do barramento.

**Onde se grava:**
- em `Música/Aquorbium/AAAA-MM-DD/aquorbium_HH-MM-SS_seed<N>.wav`;
- na máquina do autor, `~/Music/Aquorbium` é um link para
  `AUDIOS_RASGO/Aquorbium`, no disco de áudio do autor, como
  nos outros instrumentos;
- `recording.directory` nos settings muda a pasta;
- a gravação recusa começar com menos de 2 GiB livres;
- nunca sobrescreve: o arquivo é criado de forma exclusiva e o nome é
  único.

**Como se grava:**
- o callback de áudio só copia para o anel limitado do
  `DiagnosticAudioCapture`; o arquivo é escrito pela sua própria thread;
- o cabeçalho é regravado a cada 4 s, então uma queda deixa um arquivo
  tocável;
- o limite é de uma hora, abaixo do teto RIFF de 4 GiB;
- uma mudança de taxa encerra a gravação, porque um WAV tem uma taxa só.

**A medição de cada take (`core/TakeAnalysis`), na thread de escrita:**
- loudness integrado e o máximo de curto prazo (BS.1770 / EBU R128, o
  `LoudnessMeter` do próprio Aquorbium);
- true peak 4× (o `TruePeakDetector` copiado do Navalha 2, commit b5343c4,
  com a proveniência no cabeçalho);
- pico de amostra, RMS e DC por canal;
- amostras em fundo de escala;
- amostras não finitas, gravadas como silêncio digital e contadas.

**O texto ao lado (`.txt`), como a receita do Navalha:**
- versão, início, duração, taxa, quadros perdidos;
- a medição, com o veredito de true peak ≤ −1 dBTP;
- a **semente da sessão**: `session.fixedSeed` nos settings reproduz a
  sessão;
- o estado dos controles no início.

**Metadados RIFF LIST/INFO depois dos dados, como no Antitotem:** INAM,
ICRD, ISFT e ICMT, este com a semente. IART só entra se
`recording.artist` estiver preenchido: quem toca não é presumido.

**Os testes (`TakeAnalysisTests`):**
- um seno estéreo de 997 Hz a −20 dBFS lê −20 LUFS (como o caso 1 da EBU
  Tech 3341, −23 dBFS dá −23 LUFS);
- um seno em fs/4, defasado 45°, tem o pico de amostra 3 dB abaixo, e o
  true peak o encontra;
- NaN e infinito viram silêncio e são contados;
- fundo de escala e DC são medidos;
- um take real lido de volta é RIFF válido, com o tamanho exato do LIST,
  o título no arquivo e a análise de todos os quadros.

**A verificação no app real** (uma cópia isolada dos settings e um clique
sintético no REC):
- o botão contou o tempo, e o aviso mostrou o arquivo, −19,7 LUFS e
  −11,2 dBTP;
- o `ffmpeg ebur128`, independente, leu o mesmo −19,7 LUFS;
- o `ffprobe` leu título, data, software e comentário.

**Acrescentado depois, em 2026-10-21:**
- **LRA (EBU Tech 3342):**
  - os valores de curto prazo a cada 100 ms, com porta absoluta em
    −70 LUFS e porta relativa 20 LU abaixo da média de energia;
  - o resultado é P95 − P10, num histograma de 0,1 LU, com memória fixa
    para qualquer duração;
  - os casos 1 a 3 da norma (seno estéreo de 1 kHz em dois níveis) dão
    exatamente 10, 5 e 20 LU.
- **PCM de 24 bits com dither TPDF** (`recording.format = pcm24` nos
  settings; o padrão continua sendo o float de 32 bits):
  - a política é a do Navalha: o dither pertence à codificação inteira,
    nunca ao barramento float, e a medição fica sobre a mestra float;
  - os testes conferem o cabeçalho PCM 24 bits, 6 bytes por quadro e
    tamanhos exatos;
  - no silêncio, o dither fica dentro de ±1 LSB e mexe em 25,2% das
    amostras (o esperado para TPDF com arredondamento é ~25%);
  - o nível médio é mantido exato (0,5 → 4194303,5), e NaN é gravado como
    silêncio e contado.

**O que fica para depois:** comparar com uma implementação de referência
certificada (o Navalha também ainda não fez).

## Estado, cenas e layout

Os três valores normalizados da saída técnica foram anexados ao fim do snapshot
V1 neste marco histórico. Depois das integrações posteriores, o estado atual
possui 63 valores totais. O `creativeMaster` foi anexado por último; cenas com
os 62 valores anteriores recebem zero sem reordenar campos antigos.

O cartão Master usa o mesmo `MacroPanelLayout` dos outros módulos. Quatro knobs
ocupam duas linhas para preservar legibilidade também nas janelas compactas;
diâmetro mínimo de 52 px e área de 80×92 px são invariantes auditadas.

## Evidência de 2026-09-27

- teste isolado: latência, silêncio inicial, limiter vinculado, teto, medidores,
  proteção DC/não finitos, limites e mute suave;
- teste integrado: conversão completa dos três controles, latência e
  processamento sem alocação;
- build Release temporário completo aprovado;
- suíte integral no build canônico: 40/40 em 71,10 s;
- auditoria offline: 28 PNGs, quatro resoluções e sete vistas por resolução;
- inspeção visual do Master em 1920×1006 e 960×600 sem sobreposição.
- binário apontado por `./run`: 14.026.568 bytes, SHA-256
  `7bb3c59782beaf597afaf193e9a2d2d96c06cde63909f15f0d8e73d71c6e4e79`;
  seis testes críticos repetidos no build canônico.

## Escuta manual de 2026-09-27

O primeiro portão em dispositivo real produziu evidência positiva. No áudio
interno, o usuário manipulou praticamente todos os controles e manteve o
instrumento em reprodução. Em duas observações consecutivas, não percebeu
estalos/clipes nem travamento; naquele ponto, a interface indicava `XRUN 0`,
`LATE 0`, `CLIP 0` e `GUI 0`. Com a sessão ainda em curso, os contadores
chegaram depois a `XRUN 2` e `LATE 2`. `CLIP` e `GUI` não foram reconfirmados
nesse terceiro relato.

O relato confirma uma sessão interativa substancialmente mais estável com o
binário canônico acima, mas não uma execução sem falhas nem o encerramento da
regressão histórica. A duração não foi cronometrada e taxa/buffer não foram
reconfirmados. O próximo portão é concluir a escuta prolongada, anotando
duração, taxa, buffer, contadores finais e se cada XRUN coincidiu com artefato
audível ou gesto específico; depois, uma auditoria musical separada deve
verificar a contribuição de cada família, sem confundir estabilidade técnica
com eficácia sonora dos controles.

## Limitador em true peak e recuperação depois de picos — 2026-10-22

O teste de estresse do engine (`tests/EngineStressTests.cpp`) mediu o
estágio com um detector **independente** de true peak, com 4× oversampling
(`TruePeakDetector`, o do Navalha), e encontrou dois defeitos.

**1. Picos entre amostras passavam do teto.** O detector do limitador era a
estimativa Catmull-Rom de 4 amostras.
- Um seno em fs/4 defasado 45° (as amostras ficam 3 dB abaixo do pico),
  rajadas e quadradas passavam do teto:
  - com teto de −1 dB, o true peak chegava a 0,0 dBTP;
  - com teto de −0,1 dB, chegava a **+0,9 dBTP**.
- A telemetria mostrava o teto, porque usava a mesma estimativa.
- Conserto: o limitador detecta com o `TruePeakDetector` 4×, que responde
  ~8 amostras depois, bem dentro do lookahead de 5 ms. A estimativa antiga
  fica como piso. O medidor de saída também lê o 4×.

**2. O ganho não voltava durante uma sobrecarga.** O ganho só descia e o
"hold" era renovado a cada amostra acima do teto. Um único pico de +18 dB
deixava um seno contínuo de +6 dB **12 dB abaixo do teto**, enquanto a
sobrecarga durasse. Pelo mesmo motivo, relaxar o teto durante uma
sobrecarga não fazia efeito.
- Conserto: o ganho segue o **mínimo móvel** do ganho exigido na janela
  (lookahead + latência do detector + 4 amostras).
  - O mínimo é calculado por uma fila monotônica de capacidade fixa, sem
    alocação.
  - O ganho desce na hora e volta pelo release quando o pico sai da janela.

**Depois:**
- seis sinais quentes, com tetos de −1 e −0,1 dB: true peak exatamente no
  teto (−1,00 e −0,10 dBTP);
- depois do pico, o seno volta ao teto;
- custo do estágio: 78 µs por bloco de 256 amostras (1,5 % do orçamento).

**Regressões em `OutputStageTests`:**
- `truePeakUnderTheCeiling`: falha no limitador antigo (−0,10 dBTP sobre um
  teto de −1 dB) e passa no novo;
- `limiterRecoversAfterASpike`.

Contexto: pela interface, o Aquorbium hoje não chega ao teto. No pior caso
alcançável (todos os controles no máximo, trim +6 dB, SÍNTESE 1 com
patches no nível e drive máximos, quatro vozes), o pico fica em cerca de
−8,5 dBTP. Os defeitos valiam para qualquer material que chegasse ao
limitador.

## Automação rápida sem clique e compatibilidade mono — 2026-10-22

**`tests/AutomationClickTests.cpp`** (`--automation`, ~20 s).

**Método:**
- Dois engines idênticos (mesma semente, mesmos controles) tocam juntos, e
  só um recebe a mudança brusca entre dois blocos. A diferença D entre eles
  é exatamente o efeito da mudança.
- Mede-se a banda aguda de D (segunda diferença) nos primeiros 2 ms em que
  D aparece. Ela aparece 5 ms depois, por causa do lookahead do limitador.
  Compara-se com 30–32 ms depois.
- Uma mudança suavizada (~15 ms) começa como fração do que será: razão de
  0,01 a 0,08. Um degrau aparece inteiro desde a primeira amostra: 1,26 nos
  controles positivos (um degrau real de ×1,5 e um corte seco para zero,
  aplicados à saída gravada).
- O limite é 0,5 com a banda aguda acima de −80 dBFS, e fica com margem de
  ~7× para cada lado.

**Achado: a mistura SÍNTESE mudava de uma vez.** Medido: 0,87 a −67 dBFS
nos dois sentidos. É um clique ao girar SÍNTESE com o aquário tocando.
- Agora ela segue o alvo amostra a amostra (15 ms) e trava exatamente no
  alvo; SÍNTESE 0 continua bit a bit o som anterior.
- Depois do conserto: 0,019. O teste falha na versão antiga.

**Sem clique:**
- OUTPUT TRIM (+6 e −18 dB) e MUTE;
- CAIXAS → FONES;
- SPACE MIX e SPREAD (largura);
- MASTER CRIATIVO, CAUDA, AMBIENTE;
- MORPH, TIMBRE MIX e LAB MIX;
- SÍNTESE, depois do conserto.

O STRIKER MIX age nos próximos impactos (aqui, 714 ms depois). O ataque de
um impacto novo é som, não clique: ele só é relatado.

Não há controle de pan: a posição vem dos organismos. A largura é o SPREAD
do Espaço, que só age com o GESTO do Espaço acima de zero, como na interface.

**Mono** (20 s com a música ecológica):

| Saída | Correlação estéreo | Soma (L+R)/2 |
|---|---|---|
| Caixas | 0,91–0,93 | −0,2 dB |
| Fones (binaural) | 0,54–0,56 | −1,1 dB |

Nenhuma configuração tem fase invertida, e a soma mono perde pouco.

## REC: PCM 16 e a medição do arquivo codificado — 2026-10-22

- **Formatos:** `recording.format` nos settings aceita `pcm16`, além de
  `pcm24`; o padrão continua o float 32.
  - No PCM, o dither TPDF é de ±1 LSB da profundidade escolhida.
  - O dither só entra na redução para inteiros, nunca no barramento float.
- **Duas medições no texto ao lado do take:**
  - o master em float, como antes;
  - **o arquivo como foi escrito**: os mesmos inteiros, medidos de novo
    (`encoded_integrated_lufs`, `encoded_true_peak_dbtp`...). No float, as
    duas são iguais.
- **Teste:** relê o WAV do disco e confere que a medição codificada é a do
  arquivo, em 16 e em 24 bits.
  - Exemplo, seno de 997 Hz a −20 dBFS: master −20,06 LUFS / −19,310 dBTP;
    16 bits −20,06 LUFS / −19,309 dBTP.

## Medidor de correlação — 2026-10-22

- **O que mede:** o `OutputStage` mede a correlação estéreo da saída, com
  média exponencial de ~400 ms (como o RMS).
  - +1 = mono; 0 = sem relação; −1 = canais opostos.
  - Dá 0 com silêncio ou com um lado mudo.
- **Teste em `OutputStageTests`:** mono +1, oposto −1, ruído independente
  −0,002, um lado mudo 0.
- **Onde aparece:** a dica do botão ÁUDIO mostra, além de LUFS M/S/I, o
  TRUE PEAK da sessão (4×), a redução de ganho do limitador e a correlação.

**A base grave no mono (2026-10-22).** Abaixo de 150 Hz (passa-baixas de
4ª ordem), a correlação fica entre 0,90 e 0,93 e a soma mono perde de 0,2 a
0,3 dB, nas caixas e nos fones, largos e estreitos. O modelo de cabeça
decorrela os agudos (correlação total de 0,54 nos fones), mas não a base.

## True peak contra a EBU Tech 3341 — 2026-10-22

`tests/EbuTruePeakTests.cpp` sintetiza os sinais 15–23 da Tabela 1 da EBU
Tech 3341 V4 (nov. 2023) a partir da descrição publicada, sem nenhum arquivo
da EBU, e os mede com a `TakeAnalysis` (o medidor do REC, `TruePeakDetector`
4×). Tolerância: +0,2/−0,4 dBTP.

| Sinal | Esperado | Medido |
|---|---|---|
| #15 fs/4, 0,50 FFS, 0° | −6,0 | −6,02 |
| #16 fs/4, 45° | −6,0 | −6,19 |
| #17 fs/6, 60° | −6,0 | −6,09 |
| #18 fs/8, 67,5° | −6,0 | −6,06 |
| #19 fs/4, 1,41 FFS, 45° | +3,0 | +2,82 |
| #20–23 um período de fs/4 a 4·fs, offsets 0–3 | 0,0 | −0,16, −0,16, −0,25, −0,16 |

Nos sinais 20–23, o filtro anti-aliasing da síntese é uma escolha local
(Blackman, 255 coeficientes). A fonte entrou no Atlas.

## Loudness contra a EBU Tech 3341 — 2026-10-22

`tests/EbuLoudnessTests.cpp` sintetiza 11 dos 14 casos de loudness da Tabela
1 (V4, nov. 2023) e os mede com o `LoudnessMeter`. Tolerância: ±0,1 LU.
- **Casos medidos:** 1–5 (M, S, I e os portões), 9 e 12 (S e M constantes),
  10–11 (S máximo, arquivo e ao vivo), 13–14 (M máximo, arquivo e ao vivo).
- **Fora:** o #6 (5.0 canais; o medidor é estéreo) e os #7–8 (programas
  autênticos da EBU, que as regras RASGO não deixam entrar no repositório).

**Achado:** o medidor guardava a energia em fatias de 100 ms. Um tom de 400
ms deslocado em múltiplos de 20 ms nunca cabia inteiro na janela, e os casos
#13–14 erravam 0,45 LU. Isso vale também para quem lê o M ao vivo.
- Agora as fatias são de 10 ms: M e S andam a cada 10 ms.
- O integrado continua com os blocos normativos de 400 ms a cada 100 ms, e
  os resultados dele não mudaram.

**Depois:** o pior erro nos 11 casos é de 0,04 LU, e os #13–14 ficam em
0,007 LU.

## Pontos de gravação do REC — 2026-10-22

`recording.tap` nos settings escolhe onde o REC ouve:
- `post-safety` (padrão): o que se ouve, depois do MASTER criativo, do
  limitador de segurança e do SILENCIAR;
- `pre-safety`: a saída do MASTER criativo, antes do trim e do limitador;
- `pre-master-criativo`: a mistura antes do MASTER criativo.

Nos dois últimos, o SILENCIAR e a pausa do app não se aplicam. O tap vai no
texto ao lado do take e no comentário do WAV.

**Implementação:** o engine escreve o tap pedido em buffers que valem só
para o próximo `render()`, como a entrada da sala. Os buffers são reservados
no `prepareToPlay`, sem alocação no callback.

**`RecordTapTests`:**
- pedir um tap não muda a saída, bit a bit;
- `pre-safety` fica 6,0 dB acima da saída com o trim em −6 dB;
- `pre-master-criativo` é igual ao `pre-safety` com o MASTER em bypass, e
  diferente quando ele age.

**Achado:** a mistura antes do estágio de saída traz um DC de +0,0055
(−45 dBFS) quando há uma nota de MIDI; sem MIDI, ele é zero. O bloqueador de
DC do estágio de saída o remove, então ele não se ouve nem chega ao
limitador, mas iria para uma gravação `pre-safety` ou `pre-master-criativo`.
Ficou registrado como pendência.

**Corrigido no mesmo dia:** o DC vinha do trem de pulsarets do Pulsar
(média +0,167 na voz isolada). O caminho autônomo tem um bloqueador de DC de
10 Hz por organismo; o caminho das vozes de MIDI não tinha. Agora tem o
mesmo. Com uma nota de MIDI, a média antes do MASTER criativo ficou em
−8·10⁻⁷ (`RecordTapTests`).

## Orçamento de ganho por espécie — 2026-10-22

**`tests/GainBudgetTests.cpp`:** cada espécie sozinha e a soma, medidas no
tap `pre-master-criativo` (antes do MASTER criativo), com a MÚSICA em 0,6 e
a densidade máxima.

| | LUFS | dBTP |
|---|---|---|
| Soma | −26,2 | −14,8 |
| MORPH (oscilador) | −28,7 | −16,6 |
| PULSAR | −27,9 | −18,8 |
| GRAIN (striker) | −55,0 | −27,9 |
| MODAL (ressonador) | −54,4 | −27,6 |
| STRING | −54,7 | −28,3 |

**Sem a MÚSICA, na densidade 0,45:**
- MORPH −27,3 e PULSAR −32,5 LUFS;
- GRAIN −65,3, MODAL −58,8 e STRING −57,1 LUFS, com picos de −37 a
  −43 dBTP.

**Leitura:**
- O somador tem folga: ~15 dB em true peak antes do MASTER, mesmo na
  densidade máxima.
- A densidade de 0,45 para 1,0 sobe a soma em ~2,4 LU.
- A sustentação fica 22 a 26 LU abaixo do primeiro plano com a MÚSICA, e
  30 a 38 sem ela. São eventos esparsos, com fator de crista de ~27 dB; o
  LUFS os subestima um pouco, mas a distância é grande.

**Decisão do autor:** o alvo de presença da sustentação. O teste fixa:
- a folga (≥ 10 dB);
- a coesão de cada grupo (≤ 3 LU);
- a distância (entre 0 e 30 LU).

Assim, uma mudança de equilíbrio aparece no teste em vez de passar
despercebida.

## Stems do REC — 2026-10-22

Com `recording.stems` = `true` nos settings, o REC grava ao lado do take
cinco WAVs (`…_stem-organisms.wav`, `-lab`, `-water`, `-space`, `-tail`).

**Os stems:**
- **organismos:** as vozes autônomas, as de MIDI e a síntese;
- **LAB:** o banco de módulos, que inclui a família de pulsos percussivos;
- **água:** o que o filtro espectral do AMBIENTE faz com os dois primeiros,
  mais o ambiente e as bolhas;
- **espaço:** as reflexões;
- **cauda.**

**A soma é exata:** os cinco somam a mistura antes do MASTER criativo.
`RecordTapTests` mede o pior erro em 4·10⁻⁸. Com o filtro do AMBIENTE
ligado, os stems de organismos e LAB são as versões antes do filtro, e a
"água" carrega a diferença, que pode ser negativa. Por isso um stem pode
medir mais que a mistura. O texto ao lado do take lista os stems.

**Limite:** o caminho completo no app (REC → seis arquivos) não foi
exercitado, porque exige um clique em REC numa sessão do app.

## Conformidade com `SAIDA_AUDIO_COMUM.md` — 2026-10-22

Item a item, a política comum de saída do RASGO
(`RASGO_DOCUMENTATION/architecture/SAIDA_AUDIO_COMUM.md`, §3–5) contra o
Aquorbium.

| Requisito | Estado | Evidência |
|---|---|---|
| float interno, orçamento de ganho | feito; o alvo de presença da sustentação é decisão do autor | `GainBudgetTests` |
| sem alocação, lock, disco ou espera no callback | feito | `RealtimeSafetyTests`; taps e stems com buffers reservados no `prepareToPlay` |
| `NaN`/`Inf` saneados | feito | `OutputStage`, `EngineStressTests` (NaN, ±Inf pela sala) |
| proteção contra DC e infrassom | feito | o bloqueador de 10 Hz do `OutputStage` e, desde 22/10, nas vozes de MIDI |
| limitador estéreo linkado, lookahead, true peak | feito | `OutputStageTests` (true peak 4×, mínimo móvel) |
| latência declarada | no app, 5 ms do lookahead, exposta; ao host, só com plugin | — |
| trim em dB, mute com fade, peak hold, clip latch | feito | `AutomationClickTests`; dica do ÁUDIO |
| sample peak, true peak, RMS, GR ao vivo | feito | dica do ÁUDIO |
| LUFS M/S/I | feito; casos 1–5 e 9–14 da EBU Tech 3341 | `EbuLoudnessTests` |
| correlação e downmix mono | feito, com a base grave estável | `OutputStageTests`, `AutomationClickTests` |
| dither só na redução para PCM | feito (PCM 24 e 16, TPDF) | `TakeAnalysisTests` |
| taps nomeados | feito: `pre-master-criativo`, `pre-safety`, `post-safety`, e stems | `RecordTapTests` |
| reconfigurar em troca de taxa, buffer, dispositivo | feito na taxa (22,05 a 192 kHz) e em blocos irregulares | `EngineStressTests` |
| **§4** feedback generativo não só pós-limitador | feito em 22/10: a homeostase lê pré-master; o banco de módulos e o `HabitatHealth` leem pré-safety (antes do trim). Antes, o volume mudava a própria música. A SALA continua pós-safety: é o que sai nas caixas | `RecordTapTests` (o trim não muda a mistura, bit a bit) |
| **§5** soma em fase no máximo | feito (pior caso −8,5 dBTP pela interface) | `EngineStressTests` |
| **§5** impulso, DC, silêncio, subgrave, perto de Nyquist | feito | `EngineStressTests` |
| **§5** picos intersample, teto como true peak | feito (sinais 15–23 da EBU 3341) | `EbuTruePeakTests` |
| **§5** automação rápida sem clique | feito (a SÍNTESE estalava e foi suavizada) | `AutomationClickTests` |
| **§5** bypass e neutralidade de cada estágio opcional | feito: 11 estágios neutros em zero, bit a bit, com controle positivo | `BypassSweepTests` (`--bypass-sweep`) |
| **§5** teste prolongado sem xrun | feito no engine: 30 min finitos, abaixo do teto, estáveis, sem crescer memória; CPU média de 31 % e pior bloco de 52 % (com os denormais zerados). O soak no dispositivo segue aberto | `EngineSoakTests` (slow) |
| **§5** exportações float, PCM24, PCM16 | feito | `TakeAnalysisTests` |
| **§5** fixtures BS.1770 e testes dourados | feito: fixtures EBU 3341; cinco renders dourados (LUFS, true peak, RMS em quatro janelas) com tolerância de 0,01 dB, que valem no Linux/GCC/x86-64 | `EbuLoudnessTests`, `EbuTruePeakTests`, `GoldenRenderTests` |

**O que fica aberto:**
- o soak test;
- a latência ao host, quando houver plugin.

**Testes dourados (22/10):**
- Uma mudança intencional no som atualiza os valores: rode
  `aquorbium_golden_render_tests --print`, cole os valores e diga o
  porquê no commit.
- O motor é caótico: uma diferença mínima de ponto flutuante entre
  compiladores pode virar outra música. Por isso os valores só obrigam
  onde foram gravados.
- Observação: com o banco de módulos em 0,7, o som fica em −17 LUFS e
  −7 dBTP, bem mais quente que as outras configurações (−29 a −33 LUFS).

**Achado e conserto (22/10):** a realimentação do banco de módulos
(`finalOutputFeedbackSample`) e o `HabitatHealth` liam a saída depois do
trim e do limitador. Mudar o OUTPUT TRIM, que é só o volume, mudava a
própria música: de 0 para −18 dB, a mistura divergia em 66 ms. Agora os dois
leem a saída do MASTER criativo. O `RecordTapTests` fixa que trims de −18,
0 e +6 dB dão a mesma mistura, bit a bit.

## Desempenho do pior caso: os denormais — 2026-10-23

**O sintoma:** o soak de 30 min deu média de 48 % do orçamento e um bloco a
122 %. Uma medição por recurso deu 3,2 ms por bloco de 256 com tudo ligado
e 4 vozes de MIDI.

**A investigação:**
- O `perf` está bloqueado nesta máquina (`perf_event_paranoid = 4`), e o
  perfil linha a linha do `gprof` falhou.
- Um amostrador dentro do próprio teste (`ITIMER_PROF` mais `addr2line`)
  apontou aritmética trivial como o mais quente: os modos da MATÉRIA
  (`SynthSources.cpp`) e o bloqueador de DC das vozes de MIDI.
- Isso é a assinatura dos **números denormais**. Filtros e ressonadores
  que decaem no silêncio entram na faixa denormal, onde a CPU fica até
  ~100× mais lenta.

**A causa da diferença:** o callback do app já liga `juce::ScopedNoDenormals`.
Os testes e as medições não ligavam. A carga real do app era a com FTZ:
1,76 ms (33 %), e não 3,1 ms (59 %).

**O conserto:** o engine passa a ligar flush-to-zero sozinho durante o
`render` (`core/FlushDenormals.h`), restaurando o estado ao sair. No
x86-64 usa o MXCSR (FTZ e DAZ); no AArch64, o FPCR (FZ).
- Qualquer hospedeiro tem a mesma carga: os testes, o app e um futuro
  plugin.
- Com tudo ligado e 4 vozes de MIDI: 1,74 ms por bloco (33 %), com ou sem
  FTZ por fora.
- Os renders dourados não mudaram nem na quarta casa decimal.

**Conserto menor no mesmo trabalho:** o `OrganismVoiceEngine` configurava
os dois geradores (oscilador e Pulsar) em todas as espécies, a cada amostra.
Agora configura só o que a espécie toca; a saída é idêntica. O ganho medido
é de ~2 %: o `gprof` sem incorporação exagerava o custo.

**Soak depois do conserto:** 30 minutos com tudo ligado, média de 1,67 ms
por bloco (31 %) e pior bloco de 2,78 ms (52 %), contra 48 % e 122 % antes.
A memória cresceu 256 kB. O `EngineSoakTests` passa.

## SUSTENTAÇÃO: a presença da sustentação nas mãos do músico — 2026-10-23

Em vez de eu escolher o equilíbrio, que é uma decisão estética do autor, um
controle no cartão TIMBRE: **SUSTENTAÇÃO**, de −6 a +24 dB, para GRAIN,
MODAL e STRING juntos.
- **Por cima do equilíbrio autoral:** é um multiplicador sobre os ganhos
  das espécies (MORPH 0,55, PULSAR 1,20, MODAL 1,50, GRAIN 1,25,
  STRING 1,50), nunca no lugar deles. A primeira versão substituía esses
  ganhos e foi refeita antes do commit; o teste do engine a acusou.
- **Em 0 dB o som é idêntico:** os renders dourados não mudaram.
- **Rampa:** desliza por amostra (15 ms), sem clique.
- **Cenas:** o novo parâmetro `supportPresence` entra nas cenas; as antigas
  abrem em 0 dB.
- **Interface:** tem LEARN e dica nos quatro idiomas. No layout largo, as
  linhas do cartão TIMBRE foram compactadas para caber.

**Medido (`GainBudgetTests`):** +20 dB de SUSTENTAÇÃO dá +20,08 dB no stem
dos organismos e deixa o MORPH intocado. Na mistura a GRAIN sobe ~17 dB:
- a realimentação do ESPAÇO satura suavemente por desenho, e as reflexões
  sobem só ~17 dB;
- uma espécie esparsa é boa parte do próprio eco.


## Perfis de entrega no REC — 2026-10-23

Cada take do REC traz, no texto ao lado, uma seção `[delivery profiles]`.
Ela é calculada sobre o arquivo como foi gravado (`encoded_*`), já com a
quantização PCM. A seção diz o que cada destino faria com o arquivo e
**não aplica nada ao som**. A performance ao vivo continua sem
normalização e sem um alvo único (Fase 7a). O código está em
`src/core/DeliveryProfiles.h`.

| perfil | alvo | teto | como o destino aplica o ganho |
|---|---|---|---|
| `spotify_normal` | −14 LUFS | −1 dBTP | abaixa por inteiro; ao subir, para quando o true peak chega a −1 dBTP |
| `spotify_quiet` | −19 LUFS | −1 dBTP | idem |
| `ebu_r128` | −23 LUFS | −1 dBTP | ganho inteiro; o relatório avisa se o pico passa do teto |

Também entra a recomendação de master do Spotify: o true peak deve ficar
abaixo de −1 dBTP, ou abaixo de −2 dBTP quando o master passa de −14 LUFS.
Ela vale para o arquivo antes de qualquer normalização. O teto do próprio
Aquorbium continua nas mãos do músico (TETO, de −6 a −0,1 dBTP). O
relatório só mostra se o take serve a cada destino; o −1 dBTP não é tratado
como universal.

Ficaram de fora:
- o modo Loud do Spotify (−11 LUFS), porque ele usa um limiter próprio da
  plataforma, que não simulamos;
- Apple Music e outros serviços, até que haja fonte primária. A página da
  Apple consultada não publica o alvo; ele só aparece em fontes
  secundárias.

Teste: `TakeAnalysisTests`, que inclui o exemplo do próprio Spotify (uma
faixa a −20 LUFS com pico de −5 dBTP sobe só até −16 LUFS), o corte de
ganho positivo no teto e o ganho inteiro do R128 com o pico acusado.

A formatação no app compilou (build de 2026-10-23), mas ainda não foi
exercida num REC real: não há flag que acione o REC sem as mãos do músico.

Fontes:
- Spotify, *Loudness normalization on Spotify*,
  https://support.spotify.com/us/artists/article/loudness-normalization/
  (lida em 2026-10-23);
- EBU R 128, *Loudness normalisation and permitted maximum level of audio
  signals*, e EBU Tech 3341 (já no Atlas).

## ORDEM DO EQ no MASTER CRIATIVO — 2026-10-23

O `MasterChain` já sabia pôr a inclinação (±3 dB em graves e agudos)
antes ou depois da dinâmica multibanda (`eqAfterDynamics`), mas o motor
fixava ANTES. Agora a escolha é do músico. Ela fica no quinto controle do
cartão MASTER, ORDEM DO EQ, com duas posições, ANTES e DEPOIS (princípio
4ms LEQA). É parâmetro de cena (`masterEqOrder`) e tem LEARN. Cenas
anteriores abrem em ANTES.

Medição (`MasterChainTests`), com um seno de 50 Hz a 0,9 e inclinação +1:

- ANTES: −16,6 dB;
- DEPOIS: −18,5 dB.

A ordem decide se a dinâmica reage ao som inclinado ou se a inclinação vem
depois dela.

A troca durante a performance não precisa de rampa. No instante da troca,
`x·eq·c` e `x·c·eq` são o mesmo número, e a diferença só chega à medida
que o envelope do compressor de banda reage (ataque de 32 ms). Verificado:
com uma transição de 15 ms e com a troca instantânea, o maior passo
amostra a amostra foi idêntico ao do regime (0,00143). O teste fica como
guarda.

## Aliasing dos estágios não lineares — 2026-10-23

O teste é `tests/AliasingMeasureTests.cpp` e roda em cerca de 1 s.

**Método.** Um sinal periódico cuja grade fundamental cai num número primo
q de bins da FFT (N = 65 536, janela de Hann). Tudo o que o estágio produz
de fato cai em múltiplos de q. Um componente que dobra de volta de cima de
Nyquist cai em N − k·q, que não é múltiplo de q. A energia fora da grade,
dividida pelo total, é o aliasing (mais o ruído próprio do estágio). Um
segundo número dá a parte abaixo de 5 kHz, onde o aliasing soa como tom
inarmônico, e não como chiado perto de Nyquist.

Controles da métrica, a 2 kHz:

| sinal | fora da grade | abaixo de 5 kHz |
|---|---|---|
| seno puro | −154 dB | −160 dB |
| dente de serra ingênua | −13 dB | −21 dB |

**Pior caso de cada estágio, fora da grade:**

| estágio | pior caso | observação |
|---|---|---|
| `BiomaBassVoice` (polyBLEP, FM, dobrador sobreamostrado, ladder e saída tanh) | −45 dB a 880 Hz | −76 dB a 220 Hz; sem mudança |
| `SchoolWaveVoice` com WAVE CUT | era −10 dB a 1760 Hz, agora −29 dB | corrigido |
| `SchoolWaveVoice`, corte no centro | −30 dB | o polyBLEP de 2 pontos da dente de serra; sem mudança |
| `HarmonicRippleVoice` | era −29 dB a 3 kHz, agora −76 dB | corrigido |
| `BiomaPhysicalString::energyBarrier` | −41 dB a 2 kHz com o dobro do limite | só em sobrecarga; −74 dB a 440 Hz; sem mudança |

**As duas correções:**

- **WAVE CUT.** O corte multiplica a parte do ciclo depois da fronteira, e
  isso cria dois saltos: na fronteira e na volta do ciclo. Antes nenhum
  deles tinha correção, e com o corte nas pontas o resultado aliasava como
  uma dente de serra ingênua. Agora cada salto ganha um resíduo polyBLEP da
  sua própria altura. Com o corte no centro, o resultado é a mesma
  dente de serra polyBLEP de antes.
- **HarmonicRipple.** Cada harmônico some entre 0,30 e 0,40 da taxa de
  amostragem. A soma mantém os pesos inteiros, então um harmônico que some
  baixa o nível, mas não redistribui a ondulação.

Os renders dourados do banco de módulos ficaram dentro de 0,01 dB: a
energia removida estava 29 dB ou mais abaixo do sinal.

**Não medidos por este método:** os módulos caóticos ou de ruído (o
atrator de dupla rolagem, a crepitação de contato, o barramento de
realimentação sem entrada). Eles não têm grade harmônica, e o seu conteúdo
já é de banda larga.

**Possível próximo passo, se a escuta pedir:** BLAMP nas quinas do
triângulo e do seno cortado. Hoje elas ficam abaixo de −42 dB em 5 kHz a
1760 Hz.

Referência: V. Välimäki e A. Huovilainen, "Antialiasing oscillators in
subtractive synthesis", *IEEE Signal Processing Magazine* 24(2), 2007
(polyBLEP).

### Achado de 2026-10-23: a dinâmica do MASTER CRIATIVO quase não age no nível usual

No nível normal do instrumento (cerca de −28 LUFS), a dinâmica multibanda do
MASTER CRIATIVO fica abaixo dos seus limiares (−20, −24 e −27 dB por
banda). A redução de ganho medida foi de 0,000 dB. Mesmo com SUSTENTAÇÃO
em +24 dB, DENSIDADE em 1 e TIMBRE em 1, ela não passou de 0,002 dB, e só
na banda média.

Com o ganho da dinâmica em 1, `x·eq·1` e `x·1·eq` são o mesmo número. Por
isso a ORDEM DO EQ só muda o som quando o sinal está alto, por exemplo com
o banco de módulos aberto (−17 LUFS). Nesse nível o teste de cliques a
ouve, e ela também não estala.

Decidir se os limiares devem acompanhar o nível real do Aquorbium é uma
questão de som e fica com o autor. No nível usual o MASTER CRIATIVO age pela
inclinação e pela largura; a cola também ficou em 0,000 dB.

## Ambisonics no REC — 2026-10-23

Com `recording.ambisonics` ligado, cada take do REC ganha ao lado um
`_ambix.wav`.

**Formato.** Ambisonics de primeira ordem no padrão AmbiX:
- canais na ordem ACN (W, Y, Z, X), normalização SN3D;
- float de 32 bits;
- `WAVE_FORMAT_EXTENSIBLE` com máscara de canais 0, porque os canais não
  são caixas, e chunk `fact`.

Qualquer decodificador Ambisonics toca o arquivo, em caixas ou em binaural.

**O que entra.**
- Cada organismo vem da direção dele ao ouvinte, a mesma que o binaural de
  FONES usa (`Ecosystem::directionFromListener`). A direção desliza em
  cerca de 20 ms.
- O resto (vozes de MIDI, banco, água, espaço, cauda) entra como o par
  estéreo a ±30°.
- É a mistura antes do MASTER CRIATIVO, como os stems.
- W é (L + R)/√2 dessa mistura nas caixas.

**Custo.** O barramento é entregue antes de cada render e limpo por ele,
como os stems. Uma sessão sem a opção fica idêntica bit a bit e não paga
nada. A gravação usa um gravador próprio de 4 canais (`AmbisonicCapture`),
no mesmo desenho do estéreo: anel sem bloqueio, thread de escrita e
criação exclusiva do arquivo.

**Validação.**
- `AmbisonicTests`, codificador: 119 direções recuperadas com erro máximo
  de 5·10⁻⁷, e o par estéreo cai a ±30°.
- `AmbisonicTests`, motor: a saída fica idêntica com o barramento ligado;
  W bate com (L + R)/√2 da soma dos stems a 4·10⁻⁸; a energia direcional
  sobre a omnidirecional fica em 0,91.
- `AmbisonicTests`, arquivo: cabeçalho e amostras lidos de volta.
- Verificação independente: o ffprobe lê `pcm_f32le`, 4 canais e layout
  desconhecido; o `astats` do ffmpeg dá os níveis esperados por canal.

**Limitação.** A gravação no app compila e segue o caminho dos stems, mas
não foi exercida num REC real, porque não há flag que acione o REC.

**Pendente:** a saída multicanal ao vivo, que depende de um sistema de
caixas.

Referências:
- C. Nachbar, F. Zotter, E. Deleflie e A. Sontacchi, "AmbiX — a suggested
  Ambisonics format", Ambisonics Symposium, 2011;
- M. A. Gerzon, "Periphony: with-height sound reproduction", JAES 21(1),
  1973.

## Taps de realimentação e o cartão MEDIDORES — 2026-10-24

**Os taps.** A arquitetura pede que a energia que realimenta o Cérebro seja
publicada em taps separados. Observar só a saída já limitada apagaria a
dinâmica do ecossistema. O motor publica quatro envoltórias
(`BiomaEngine::feedbackTaps()` e a telemetria), cada uma com ataque de
10 ms e release de 150 ms, limitadas a 0..1:
- pré-MASTER CRIATIVO;
- pré-segurança;
- redução do limitador (1 = 12 dB);
- pós-segurança.

O limitador expõe o próprio ganho (`OutputStage::limiterGainNow`), sem
montar a telemetria a cada amostra. Os consumidores de hoje continuam
lendo de onde liam.

**Teste (`RecordTapTests`).**
- Os taps ficam finitos e entre 0 e 1.
- O pré-master e o pré-segurança ficam idênticos entre −18 e +6 dB de
  volume de saída; o pós-segurança acompanha o volume.
- A redução só aparece quando o limitador age: pico de 0,87 com o banco
  aberto e teto de −6 dB; zero no sinal normal.

**O cartão 10 MEDIDORES** (pedido do autor: "pode ser os medidores"). Na
tela larga ele fica abaixo dos MÓDULOS; nas telas menores, no fim da
coluna da direita. São nove barras com o valor:
- LUFS M, S e I;
- true peak (vermelho acima de −1 dBTP);
- limitador;
- correlação (centrada; vermelho abaixo de 0);
- os três taps, em dB.

Ele lê só a cópia de telemetria da interface e redesenha a no máximo
15 Hz. O layout-audit passou com 105 capturas.

## Os cartões LUPA, CÉREBRO · SALA e o MASTER CRIATIVO por dentro — 2026-10-24

O autor: "insira os demais". São 12 controles novos, parâmetros de cena
(com migração das cenas antigas), salvos nas configurações e traduzidos nos
quatro idiomas. **O centro de cada um é o instrumento clássico.** O valor é
o de antes vezes 2^0 = 1 exatamente; a COORDENAÇÃO começa em 0.

| onde | controles | o que fazem |
|---|---|---|
| cartão 11 LUPA (tela larga: abaixo do AMBIENTE) | RAIO, ALCANCE, AMPLIAÇÃO, DESFOQUE | a forma da lupa no vidro (`Ecosystem::setLensShape`): de ½× a 2× o raio e a ampliação, o alcance na água, o recuo do que fica fora de foco |
| cartão 07 MASTER (nove botões) | MULTIBANDA, COLA, INCLINAÇÃO, LARGURA | desvios sobre os valores que a ecologia define; o centro é só a ecologia |
| painel COMANDOS ECO (tela larga) ou cartão 12 CÉREBRO · SALA (telas menores) | RELÓGIO, COORDENAÇÃO | o giro do registrador do Cérebro (0,06 a 0,9 Hz) e quanto os organismos compartilham uma inclinação (0 cada um a sua, 1 um cardume); status com os 8 bits e o humor |
| o mesmo lugar | SENSIBILIDADE, REAÇÃO | a SALA: o microfone ouvido ±12 dB, e a força da resposta da música; status com nível, força e acoplamento |

Os status do Cérebro e da Sala chegam pela telemetria; a interface nunca lê
o estado da thread de áudio.

**Testes.**
- `CardControlsTests`: no centro, nada muda. Fora dele, cada grupo muda o
  som: LUPA −9,2 dB, MASTER −13,5 dB, RELÓGIO +2,5 dB e COORDENAÇÃO
  +2,6 dB de diferença em relação ao som.
- Migração das cenas em `EcologyParameterStateTests`.
- O layout-audit ganhou a vista "new-cards" e passou em todos os tamanhos;
  as imagens de 1920 e 1280 foram conferidas.

**Ainda falta:**
- o quadro compacto da SÍNTESE;
- o "slider do master": se o autor quis um fader de volume geral à vista,
  ele ainda não existe; o VOLUME DE SAÍDA continua no cartão MASTER.
