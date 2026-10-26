# Aquorbium — comparação cega dos corpos físicos

**Estado:** segunda escuta aprovada; String integrado como quinto organismo  
**Corpos:** `BiomaPhysicalString`, `BiomaModalResonator` e
`BiomaGranularStriker`

## Por que comparar

Os três motores respondem a impactos, mas armazenam energia de maneiras
diferentes:

- circulação numa guia de onda;
- soma de modos amortecidos;
- população de partículas curtas filtradas.

A comparação não escolhe o “melhor” som. Ela investiga se o Physical String
acrescenta uma função musical ou apenas ocupa o território de um corpo já
integrado.

## Protocolo incorporado ao renderizador

`aquorbium_physical_bodies_blind_render` cria `A.wav`, `B.wav` e `C.wav` sem
escrever a chave na pasta. Cada take possui:

- 18 segundos, 48 kHz, estéreo float de 32 bits;
- a mesma seed;
- os mesmos dez instantes, pitch anchors e intensidades;
- parâmetros materiais intermediários apropriados à física de cada motor;
- fade final idêntico de 80 ms;
- RMS exato de 0,016 (-35,92 dBFS).

O alvo foi escolhido pelo maior fator de crista. Assim nenhum take toca o teto
de 0,70 e não há limiter, compressor ou saturação posterior. A dinâmica nativa
é preservada e loudness não se torna atalho para preferência.

Cada corpo traduz o pitch anchor segundo sua função. Modal e String usam a
fundamental diretamente. O Striker recebe o mesmo contorno intervalar em um
registro fixamente transposto, pois sua partícula curta não reivindica altura
como uma corda sustentada. Essa diferença é parte explícita do papel, não uma
tentativa de apresentar os três como modelos acústicos equivalentes.

## Medições cegas

| Take | RMS | Pico | DC | Delta máximo | Razão HF | Fator de crista |
|---|---:|---:|---:|---:|---:|---:|
| A | 0,016 | 0,1160 | 5,40e-7 | 0,0259 | 0,000590 | 7,25 |
| B | 0,016 | 0,6585 | -3,92e-9 | 0,6134 | 0,276752 | 41,16 |
| C | 0,016 | 0,4659 | -4,51e-6 | 0,1106 | 0,016699 | 29,12 |

Correlações de forma de onda em atraso zero:

- A/B: `0,07468`;
- A/C: `-0,00195`;
- B/C: `-0,00447`.

As métricas demonstram contraste forte de envelope e distribuição espectral.
Elas não revelam qual papel será mais fértil no instrumento e não autorizam a
integração sozinhas.

## Decisão de memória antes da integração

`BiomaEngine` possui quatro vozes externas e um banco autônomo. Cada banco tem
um `OrganismVoiceEngine` por organismo. No momento desta decisão havia 20
despachantes; o quinto organismo elevaria o total a 25.

`BiomaPhysicalString` ocupa 131.280 bytes nesta ABI. Colocá-lo como membro de
cada despachante acrescentaria:

- 2.625.600 bytes com os quatro organismos atuais;
- 3.282.000 bytes caso fosse criado um quinto organismo.

Somente cinco corpos físicos podem soar simultaneamente para uma espécie — um
no banco autônomo e um em cada voz externa. Um banco dedicado de cinco
instâncias usa 656.400 bytes, 80% menos que duplicá-lo por 25 despachantes.

Portanto, a integração aprovada segue uma coleção fixa específica por banco,
com propriedade e roteamento explícitos. O grande buffer não entra como membro
indiferenciado de `OrganismVoiceEngine`. Essa
decisão preserva zero alocações no render e impede que a conveniência do
despacho desperdice memória.

## Procedimento usado

Os três WAVs foram o procedimento inteiro, sem formulário obrigatório. Depois
da impressão, a chave foi mostrada por:

```sh
./build-release/aquorbium_physical_bodies_blind_render --reveal
```

Até a revelação, documentação e interface chamaram os arquivos somente de A,
B e C.

## Primeira escuta e revelação

Relato do usuário, antes da chave:

- A: “mais o que já estávamos escutando”;
- B: “há uma espécie de noise ou algo do gênero”;
- C: “som mais seco, curto”, percebido como um envelope atuando sobre uma
  matéria característica.

Chave revelada depois do relato:

- A = Modal Resonator;
- B = Physical String;
- C = Granular Striker.

A descrição de A confirma a familiaridade do corpo já integrado. A leitura de
C corresponde aos envelopes exponenciais curtos das partículas, embora o
motor não seja literalmente um ADSR sobre uma waveshape. B identificou um
problema real: o contato estocástico aparecia antes da memória da corda. Logo,
o Physical String não foi integrado.

## Segunda iteração do String

Sem alterar A ou C, o excitador de B recebeu:

- dois estágios de filtragem governados por `hardness`;
- janela Hann durante um percurso, começando e terminando sem degrau;
- caminho de contato direto reduzido de 32% para 4%;
- saída dominada pela energia que já circulou na guia.

Sob o mesmo RMS de 0,016:

| Medida de B | Original | Refinado |
|---|---:|---:|
| Pico | 0,6585 | 0,3138 |
| Delta máximo | 0,6134 | 0,1486 |
| Razão HF | 0,2768 | 0,1105 |
| Fator de crista | 41,16 | 19,61 |

A e C permaneceram idênticos byte a byte. Na segunda escuta, o usuário percebeu
“uma sofisticação implementada” no refinado. Isso fechou o portão musical: o
String não substitui A nem C, pois os três são corpos diferentes, e entrou como
quinto organismo.

## Integração escolhida

O plano de memória foi aplicado literalmente. `BiomaEngine` possui cinco
instâncias de `BiomaPhysicalString`: uma no banco autônomo e uma em cada uma das
quatro vozes externas, totalizando 656.400 bytes nesta ABI. O motor grande não
foi embutido nos 25 despachantes de organismo.

`Species::PhysicalString` ocupa a quinta posição da formação, recebe eventos do
mesmo campo ecológico e da mesma rede geométrica, tem parâmetros derivados de
calma, detalhe, luz, inteligência, força e acento, e publica energia e impactos
na telemetria. A interface o mostra como `STRING`; macros performáticas próprias
ficam para uma etapa posterior, depois de observar seu diálogo autônomo.
