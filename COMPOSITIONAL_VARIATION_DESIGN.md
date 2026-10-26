# Aquorbium — diretor de variação composicional

Atualização: 2026-10-20 (contrastes, durações e testes; ver o fim).

## Decisão musical

O Aquorbium é, antes de tudo, um instrumento musical generativo e criativo. A
geração de formas, melodias, harmonias, ritmos e timbres é o objetivo; módulos,
telemetria e controles são meios para compor e tocar, não uma demonstração de
parâmetros. O critério final é a escuta musical humana, não a quantidade de
variações internas.

O instrumento já dispõe de memória formal, passos, progressão harmônica,
organismos sonoros e quinze famílias experimentais. O novo `CompositionalVariationDirector`
coordena esse material em segmentos recorrentes `A → A′ → B → suspensão`.
Cada segmento reutiliza a matéria anterior com mudança de densidade, leitura
da memória, peso harmônico, acentuação e balanço entre famílias.

O avanço acontece somente na fronteira coletiva já produzida pelo campo
Relabi. Essa fronteira organiza uma forma lenta; não representa compasso, BPM
ou sincronização com DAW. A política é determinística e sem novo gerador
aleatório. O estado formal do `BiomaMarkov` e calma, coesão, perigo e densidade
do Ecossistema modulam o perfil sem romper a sequência reconhecível.

Os segmentos não avançam mais num ciclo rígido de um evento por fase. Cada
entrada estabelece uma duração de 1 a 4 fronteiras coletivas: A e B partem de
duas, A′ e suspensão de uma, com retenções determinísticas influenciadas pelo
estado formal e, sob tensão alta e calma baixa, pelo contexto ecológico. Isso
quebra a periodicidade mecânica sem introduzir sorteio ou relógio paralelo.
`eventsInSegment` e `segmentLength` ficam disponíveis na telemetria para
relacionar a duração formal à escuta.

A memória melódica inicial também foi reescrita como uma célula autoral curta,
com contorno ascendente e de resposta (`0, 2, 5, 3, 7, 5, 2, 3` graus) e
acentos/gates desiguais. Ela usa a escala e a harmonia já existentes; não
pretende ser uma melodia final aprovada. Sua função é oferecer identidade
reconhecível para a autonomia transformar, em vez de depender de saltos amplos
e de uma alternância de gates quase uniforme.

## Consequências do ciclo

| Segmento | Papel | Consequência principal |
|---|---|---|
| A — estabelecer | apresentar material e campo harmônico | densidade contida, motivo moderado, padrão geométrico base e textura reduzida |
| A′ — variar | conservar identidade e alterar a leitura | mais pulso, motivo destacado, rotação rítmica positiva e leve deslocamento harmônico |
| B — responder | criar contraste reconhecível | motivo mais presente, leitura retrógrada e rotação rítmica inversa |
| Suspensão | abrir espaço antes do retorno | menos eventos, motivo recuado, rotação rítmica parcial, mais textura e espaço |

A acentuação calculada pela Geometria agora também afeta, de forma limitada,
o ganho energético das vozes. A, A′, B e suspensão têm pesos próprios; a
influência de `STRUCTURE MIX` reduz esse efeito até zero. Eventos sem acento
mantêm ganho neutro. Assim, `geometricAccent` deixa de ser apenas telemetria e
passa a contribuir para a articulação audível.

A memória de passos também recebe um peso por segmento antes de contribuir
para a altura. A′ reforça o motivo e B permite uma transformação mais marcada;
a suspensão reduz essa influência para deixar respirar a matéria harmônica e
ecológica restante. A operação reutiliza os passos existentes e é neutralizada
quando `STRUCTURE MIX=0`. Coesão e calma do Ecossistema também ajustam
levemente esse peso: condições mais coesas e calmas preservam um pouco mais a
memória, sem interromper o ciclo formal.

Em B, a leitura retrógrada agora é interpolada continuamente com a leitura
direta conforme `STRUCTURE MIX` sobe. Isso remove o antigo limiar abrupto em
0,5; no máximo, o motivo chega à forma retrógrada integral.

A máscara euclidiana também pode interpolar entre o padrão original e sua
rotação de um passo. `GeometricSequencer::Step::gateProbability` representa
essa probabilidade contínua; a decisão final usa o fluxo aleatório determinista
já pertencente à autonomia. Em `STRUCTURE MIX=0`, a probabilidade continua
exatamente binária e a sequência anterior não muda.

Não há novo gerador de notas nem segundo relógio. A memória de passos continua
fornecendo seu material; a progressão existente continua fornecendo os graus
harmônicos. A resposta retrógrada só altera a ordem de leitura dos mesmos
endereços.

## Integração

`AutonomousEnergy` avança o diretor no evento coletivo. `STRUCTURE MIX`
controla a intensidade da política: em zero, os valores de áudio anteriores
permanecem preservados; ao subir, o plano afeta densidade, chance de despertar,
intensidade do motivo lembrado, leitura dos passos, acentuação e fase harmônica.

Quando `LAB MIX` também está ativo, as quinze famílias recebem pesos agrupados:

- pulso: `PERC + PULSE`, `CLOCK + EVENTS`, `ROTATE + LAYOUT`, `PATTERN + LOOP` e `SHIFT + PROB`;
- tonalidade: `FM + HARMONICS` e `TRANSPOSE + SELECT`;
- textura: `GRANULAR CAPTURE`, `FRACTAL + VECTOR`, `BRANCH + PERMUTE`, `EVOLVE + STAGES` e `INTEGRITY + MEMORY`;
- movimento: `LORENZ + DRIFT`, `FLUID + PHASE` e `GESTURE WAVES`.

Os pesos multiplicam os controles que o usuário já ajustou, passam por
suavização do contexto e são neutralizados quando `STRUCTURE MIX=0`. O campo
de espaço usa a mesma intenção para alterar pouco o feedback das reflexões,
respeitando o teto existente de 0,50. Nenhum knob ou campo persistente novo foi
criado.

## Como experimentar

1. Ativar a autonomia e usar uma seed fixa para comparar passagens.
2. No cartão `02 GEOMETRIA`, aumentar `STRUCTURE MIX` de zero para um valor
   moderado.
3. No cartão `08 MÓDULOS`, abrir `LAB MIX` e selecionar poucas famílias de
   início: primeiro síntese/harmonia e pulso; depois textura, movimento e
   gesto.
4. Ouvir pelo menos quatro fronteiras coletivas, anotando se A′ preserva A,
   se B responde e se a suspensão abre espaço. Repetir sem o banco para
   separar a composição autônoma das famílias experimentais.

Foi preparado o renderizador offline `aquorbium_compositional_variation_render`.
Depois de compilado, execute na raiz do projeto com um diretório novo:

```sh
./.local-build/aquorbium_compositional_variation_render validation-output/compositional-variation-2026-10-03
```

Ele grava `structure_bypass.wav` e `structure_active.wav` (WAV float estéreo),
mais CSVs de telemetria e `REPORT.txt`. Usa seed fixa, autonomia e quatro
famílias do banco; limita cada render a 180 s e encerra ao alcançar o segmento
de suspensão após A/A′/B. Os CSVs incluem agora o índice do evento e a duração
planejada do segmento. A ferramenta foi compilada, mas ainda não foi
executada. Os WAVs não foram gerados; execução, inspeção e escuta permanecem
pendentes. Escolha um nome de diretório ainda inexistente para preservar
qualquer render anterior.

## Estado e portões

Os alvos estáticos `aquorbium_core` e
`aquorbium_compositional_variation_render` compilaram em 2026-10-03 com um job.
A configuração local não encontrou JUCE (`AQUORBIUM_JUCE_PATH` não foi
definido), portanto o aplicativo standalone não foi compilado nesta etapa.
Nenhum teste ou render foi executado. Não declarar a variação aprovada nem livre de
cliques: continuam pendentes regressões dirigidas, renders determinísticos e
escuta humana, anotando ritmo, melodia, harmonia, timbre, eventos, forma e
função composicional. O ciclo lento não equivale a frase métrica;
sincronização a andamento e DAW continua fora do escopo atual.

## Revisão de 2026-10-20: um protagonista por segmento e uma forma mais longa

O autor apontou a necessidade de "variar mais as sonoridades e o aspecto
composicional". O primeiro render de escuta, executado nesta data (antes
pendente), mostrou duas coisas:

- o ciclo inteiro durava uns **20 s**: o A tinha 11,7 s, o A′ 2,2 s e o B
  5,6 s, porque a base era de 2, 1, 2 e 1 fronteiras;
- os segmentos se distinguiam pouco: cerca de 3 dB e algumas centenas de
  hertz de centroide. Ligado ou desligado, o diretor desenhava quase a
  mesma curva.

**O que mudou:**

- **Durações:** a base passou a 6, 4, 6 e 3 fronteiras, com as mesmas
  retenções contextuais, e o máximo subiu de 4 para 8
  (`maximumSegmentLength`). No render, o A dura 23,1 s, o A′ 10,5 s e o B
  15,2 s; a suspensão chega aos 49 s.
- **Protagonistas:** os pesos das famílias usam o intervalo inteiro que o
  banco aceita (0,25 a 1,25):

| Segmento | Protagonista | Em segundo plano |
|---|---|---|
| A, estabelecer | os corpos melódicos (tonal 1,18) | pulso 0,72, textura 0,42 |
| A′, variar | o **pulso** (1,25), com densidade 1,36 e acentos 1,32 | textura 0,36 |
| B, responder | **textura (1,25) e movimento (1,22)**, leitura retrógrada, fase harmônica −0,045 | pulso 0,52 |
| suspensão | **o espaço** (1,25) | pulso 0,28, densidade 0,44, memória 0,30 |

- **Medição no render (fluxo espectral):** com o diretor, o A′ tem 33 % mais
  ataques do que o mesmo trecho sem ele (8,4 contra 6,3 por segundo). O B
  tem 33 % menos (4,0 contra 6,0) e é o trecho mais claro (2,25 kHz). Não
  há clipping, e o pico fica em −11 dBFS.
  - O A continua sendo o trecho com mais ataques, porque é quando os
    organismos despertam (2 → 5). Isso vem do ecossistema, não do diretor.
- **Testes novos** (`aquorbium_compositional_variation_director_tests`):
  - o determinismo;
  - a ordem A → A′ → B → suspensão;
  - durações mínimas (3 fronteiras por segmento, 16 por ciclo);
  - o protagonista de cada segmento e a amplitude dos contrastes;
  - limites para qualquer contexto, inclusive os extremos.

**Os áudios:**

- antes: `validation-output/compositional-variation-2026-10-20/`;
- depois: `validation-output/compositional-variation-2026-10-20-v2/`.

Cada pasta tem `structure_bypass.wav` e `structure_active.wav`. A aprovação
musical continua sendo do autor, pela escuta.


## A gênese da sessão — 2026-10-24

O autor, ao abrir o app: "essa sequência de notas aleatórias é chata";
"sempre se repete, cada vez que eu abro". A medição confirmou. Com
sementes diferentes, cada organismo fazia o mesmo caminho de alturas (o
organismo 0 sempre em 34 → 29/31 → 38 → 41…), porque eram constantes:
- a tônica;
- o modo dórico;
- os registros;
- a órbita de 8 passos;
- as rotações;
- o contorno;
- o passo de vida;
- quem abre.

Também em 60 s soavam as 12 classes de altura: o dia de 75 s tem 8 seções,
cada uma transpunha o modo inteiro, e a temperatura somava ±2 semitons.

`src/core/SessionGenesis.h`: a semente da sessão escolhe um **mundo**.
- uma tônica e um modo (10 por enquanto);
- um caminho de acordes **dentro da tonalidade**, em graus (I IV V vi, um
  vamp modal, um bordão…), com 1, 2 ou 4 seções por acorde;
- os registros em oitavas, com a quinta expressa como grau;
- a órbita, de 5 a 16 passos, e as rotações;
- um de 6 contornos;
- um andamento de vida (0,62× a 1,62×);
- quem abre;
- a cor de cada corpo (forma e largura de pulso);
- a hora e a estação em que a sessão acorda.

É o conceito da semente como hipótese do Rasgo Modular
(`dossies/ESTUDO_seed_composicao_generativa.md` §1.1), reescrito, sem
código compartilhado.

Não toca nos controles nem nos patches do músico. `classicWorld()` é o
instrumento de antes, bit a bit: os renders dourados não mudaram.
`session.genesis = 0` volta a ele.

**Testes (`SessionGenesisTests`).**
- 200 sementes dão 199 mundos distintos.
- Duas sementes cantam melodias diferentes: 4 de 136 notas iguais, antes
  10 de 182 e o mesmo contorno.
- Nenhuma nota fora da tonalidade: 0 de 136 em lá jônio, 0 de 211 em dó#
  mixolídio.
- O mundo clássico é idêntico ao motor sem gênese.

**Próximo:** trocar a lista de 10 modos pelo banco e pela máquina de
escalas do Trioio (`TRIOIO/docs/ESCALAS.md`, recurso transversal
`KNOW-ESCALAS`), numa cópia fixada com proveniência.

## Encadeamentos harmônicos e politonalismo — 2026-10-24

O autor: "pode haver encadeamentos harmônicos também, multitonalismos, e
muitos outros recursos composicionais e que geram emoção a quem escuta".

**A origem.** As linguagens vêm da máquina de harmonia do Trioio
(`TRIOIO/docs/MAQUINA_DE_HARMONIA.md`), reescritas para um instrumento que
toca ao vivo (`src/core/HarmonyPlan.h`). O código do Trioio não foi
copiado: ele está preso ao compositor offline de lá.

**As linguagens.** A semente da sessão escolhe uma.

| linguagem | o que ela traz |
|---|---|
| funcional | duas frases T…S…D, a primeira com meia cadência, a segunda autêntica, às vezes de engano ou com cor plagal; dominantes secundárias; no menor, a sensível e às vezes a terça picarda (Riemann, Piston) |
| modal | vamps i–♭VII, i–IV, i–♭II e I–♭VII–IV, muitas vezes sobre pedal de tônica |
| brasileira | I6/9, diminuto de passagem ou VI7(♭9), IIm7, V7(9), IV7M, a plagal da bossa (ivm6) |
| impressionista | *planing* de nonas, tons inteiros, fim em ♭VII(add9)–I6/9, nunca V–I |
| cromática | P, L e R de Cohn: mediantes cromáticas, duas notas comuns a cada passo |

Em cerca de um terço das sessões há **episódios politonais** (Milhaud): a
camada aguda vai a outra tonalidade (trítono, quinta, terça menor ou sexta
maior).

**A voz de cada organismo.**
- O baixo (o registro mais grave) toca o baixo do acorde; num pedal, a
  tônica.
- As vozes do meio tocam a nota do acorde que menos se move.
- A melodia (quem abre a sessão) canta o modo. Nos tempos fortes, ela vai
  a uma nota do acorde. A nota cromática do acorde substitui o grau
  vizinho (a variante por acorde de Russell, como no Trioio).
- A harmonia tem relógio próprio, de 2,5 a 8 s por acorde, conforme o
  andamento da sessão.

**A harmonia evolui.** A cada volta do ciclo vem uma das três coisas:
- uma progressão nova;
- uma modulação para uma tonalidade vizinha;
- outra linguagem.

A cada três ciclos, ela volta para casa (A B A'). Medido em 10 min:
lá → fá# → lá; ré → sol → ré; brasileira → impressionista → funcional.

**Testes (`SessionGenesisTests`).**
- Em 5 min, de 87 % a 98 % das notas caem no acorde que soa.
- 2 de 1179 caem fora do modo e do acorde (glissandos na troca).
- 64 de 64 planos funcionais fecham na tônica.
- Nenhum V–I no impressionismo.
- 462 de 462 passos de Cohn guardam duas notas comuns.
- O mundo clássico não muda (goldens).

Um defeito pego pela sonda e corrigido: acordes como `std::initializer_list`
escolhidos por um operador ternário liam memória morta. A linguagem
brasileira começava com "A A A A A". Agora os acordes são `std::array`.

**Próximo, em ordem:**
1. O arco de emoção da sessão: a tensão de cada acorde (já no plano)
   puxando densidade, registro e dinâmica; clímax e repouso.
2. Retardos que resolvem, respiração e silêncio, pergunta e resposta, o
   motivo que volta transformado.
3. O banco e a máquina de escalas do Trioio no lugar dos 10 modos.
4. A MÚSICA ecológica (MÚSICA > 0) seguindo a mesma harmonia: feito no
   mesmo dia. Ela dita as próprias alturas (`directedNote`), e só 50 % a
   69 % das notas dela caíam no acorde. Agora ela mantém ritmo, contorno e
   acento, e a harmonia da sessão dá a nota: 86 % a 97 % no acorde. Teste
   com MÚSICA 0,7: 566 de 643 no acorde, 11 fora do modo e do acorde.

## A emoção da forma — 2026-10-24

Só nos mundos gerados; o mundo clássico não muda.

**O arco.** A semente escolhe uma forma de tensão e um período de 4 a
8 min, que se repete:
- subir até o clímax e cair;
- ondas;
- crescimento lento;
- calmaria entre duas agitações.

A tensão do momento é 0,6 × o arco + 0,4 × a tensão do acorde (a dominante
puxa, a tônica repousa), suavizada em cerca de 2 s.

**O que a tensão move.**

| recurso | efeito |
|---|---|
| eventos | o passo de vida vai de 0,5× a 1,5× |
| dinâmica | de 0,45 a 1,0 do nível clássico, nunca acima dele, para não pressionar o limitador |
| registro | no clímax (acima de 0,75), a melodia sobe uma oitava |
| respiração | depois de cada cadência, a melodia se cala por meio acorde |
| retardos | no começo de um acorde novo, uma voz do meio pode segurar a nota anterior, a um tom ou menos de uma nota do acorde, e resolve no evento seguinte (o 4–3) |

**Medido** (`FormEmotionTests`, 8 min, semente 1899755276):
- 2,99 contra 3,76 eventos por segundo no quartil calmo contra o tenso;
- 4,6 dB entre o calmo e o tenso;
- a melodia se cala em 3403 de 3403 blocos de respiração;
- 161 de 183 notas seguradas resolvem por grau conjunto dentro do acorde.

O contraste é **sutil de propósito**: muitos eventos vêm de outros motores
do ecossistema (os eventos coletivos), não do passo de vida. A escuta do
autor decide se pede mais. A subida de oitava no clímax não tem teste
próprio.

**Próximo:**
- pergunta e resposta entre organismos;
- o motivo que volta transformado;
- o banco e a máquina de escalas do Trioio;
- a MÚSICA ecológica seguindo a harmonia.

## O motivo, a pergunta e a resposta — 2026-10-24

Só nos mundos gerados.

- **O motivo.** O motivo são as últimas notas da melodia (até 6),
  guardadas como semitons acima da tonalidade.
- **A resposta.** Ao fechar uma cadência, a frase vira o motivo, e o
  respondente responde com ele transformado. O respondente é a voz mais
  aguda que não é o baixo nem a melodia. A transformação sorteada é uma
  destas: imitação a partir da fundamental do acorde novo, inversão,
  retrógrado, ou retrógrado da inversão (Schoenberg, *Fundamentals of
  Musical Composition*, 1967).
- **O retorno.** A cada duas viradas do ciclo harmônico, a própria melodia
  reapresenta o motivo, igual ou invertido.
- **O ritmo e as alturas.** O ritmo vem dos eventos de cada voz; o motivo
  dá as alturas, ajustadas ao modo.
- **Duas filas, uma para a resposta e outra para o retorno.** A primeira
  versão tinha uma só, e na virada do ciclo a resposta sobrescrevia o
  retorno: zero notas de retorno.

**Medido** (`FormEmotionTests`, 8 min):
- 102 notas de motivo em respostas e 48 em retornos;
- a tensão agora move mais: o andamento vai de 0,35× a 1,65×, e a
  participação nos gestos coletivos também acompanha a tensão. Eram 2,5
  contra 3,7 eventos por segundo entre o quartil calmo e o tenso.

**Dois defeitos achados pelo teste de gênese e corrigidos:**
- Com MÚSICA ligada, a nota de passagem da melodia vinha da escala da
  música ecológica e saía do modo; agora ela é ajustada ao modo.
- Um baixo abaixo de 24 era cortado em 24 e virava outra nota; agora ele
  sobe uma oitava (`foldIntoRange`).

Resultado: de 26 para 3 em 743 notas fora do modo e do acorde.
