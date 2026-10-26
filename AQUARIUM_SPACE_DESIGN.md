# Aquorbium — espaço situado e movimento do aquário

## Pergunta musical

Como tornar audível que cada organismo ocupa um lugar e é percebido por pontos
de vista parciais, sem reduzir o espaço a um efeito decorativo e sem deixar uma
animação da interface alterar o áudio por acidente?

Este primeiro marco não é o futuro `MultiTapSpace` e não simula uma sala. Ele é
a fundação anterior a delays e caudas: transformação geométrica correta,
panorâmica, profundidade espectral, largura e transições contínuas por fonte.

## Dois referenciais e três movimentos

O sistema agora distingue:

- **Aquarium local:** organismos, água, limite esférico e Lupa. A física de
  contenção nunca abandona esse referencial;
- **world space:** posição/orientação física do recipiente e observadores
  externos;
- **câmera/UI:** pertence apenas à apresentação e nunca entra no core de áudio.

`AquariumPose` é uma transformação rígida com translação e quaternion unitário.
Não existe escala: aumentar o desenho do aquário não aumenta o tanque físico.
`localToWorld()` e `worldToLocal()` são inversas, assim como as variantes para
direções. Uma rotação endógena lenta continua sendo parte do Ecossistema; já a
pose física externa só influencia a escuta quando
`AquariumMotionAudioEnabled` é ativado explicitamente. O padrão é desligado.
Assim, mover a janela, a câmera ou uma representação visual não vira modulação
oculta. Quando o vínculo físico for habilitado em performance, observação,
oclusão e espaço passam a responder à pose deliberadamente.

A Lupa é comparada aos organismos no espaço local. Isso a mantém presa ao
vidro durante qualquer translação ou rotação do recipiente.

## Pesquisa apropriada

A especificação Web Audio do W3C define panorâmica mono→estéreo de potência
constante com ganhos seno/cosseno e descreve modelos separados para distância.
O JUCE também oferece regras de pan de `-3 dB`, confirmando que panorâmica e
ganho são decisões distintas. O Aquorbium usa essas referências conceituais,
mas implementa um corpo próprio, portátil e independente de JUCE no core.

Fontes institucionais:

- W3C, *Web Audio API 1.1*, algoritmos de panning e distance effects:
  <https://www.w3.org/TR/webaudio-1.1/#panning-algorithm>
- JUCE, `dsp::Panner` e suas regras:
  <https://docs.juce.com/master/classjuce_1_1dsp_1_1Panner.html>

Nenhum código dessas fontes foi copiado.

## Corpo DSP por organismo

Cada organismo de cada voz possui um `AquariumSpatializer` de memória fixa. O
alvo ecológico fornece os ganhos de perspectiva dos observadores, observação,
detalhe e integridade. O módulo então:

1. recupera magnitude e ângulo da dupla de ganhos L/R;
2. aplica largura ao ângulo, sem alterar a magnitude deliberadamente;
3. reconstrói ganhos seno/cosseno de potência constante;
4. deriva profundidade de observação e detalhe;
5. move um passa-baixa de um polo entre aproximadamente `900 Hz` e `18 kHz`;
6. suaviza ganho/pan em `20 ms` e cor de profundidade em `45 ms` por amostra.

O fechamento espacial associado a trincas ocorre antes do futuro Master: a
largura mínima atual é `28%`, portanto integridade baixa não colapsa informação
estéreo completamente. O Master ainda poderá ter largura global própria; são
escalas diferentes, fonte e saída.

Os alvos ecológicos chegam a 200 Hz, mas não são aplicados como degraus. O
espacializador mantém estado contínuo na taxa de áudio, inclusive quando a rota
técnica de referência está sendo ouvida. Isso permite comparação sem mudar
relógios ou sequência ecológica.

## O que ficou deliberadamente fora

- Doppler: depende de velocidade física, escala e velocidade de propagação
  acordadas; será avaliado por escuta antes de entrar;
- HRTF/binaural: exige conjunto de respostas, convolução e contrato de ouvinte;
- reverb difuso: pertence ao próximo corpo do módulo Espaço; as reflexões
  multi-tap já estão documentadas separadamente;
- inércia da água: requer um modelo limitado e um A/B próprio;
- automação pública da pose: passará pela mesma fronteira lock-free dos demais
  controles, não por acesso concorrente direto ao `Ecosystem`.

## Invariantes e prova

- ida e volta local↔mundo dentro da tolerância numérica;
- quaternion normalizado; dados inválidos preservam a última pose válida;
- pose física não altera posições locais nem contenção;
- pose é bit a bit inaudível com o vínculo acústico desligado;
- vínculo ligado muda a perspectiva externa de maneira verificável;
- ganhos de pan obedecem `L² + R² = gain²` no alvo;
- largura zero centraliza sem zerar a fonte;
- profundidade reduz energia de alta frequência;
- saltos completos de pan são suavizados sem descontinuidade grande;
- silêncio permanece silêncio e `NaN`/infinito não contaminam a saída;
- nenhum lock, alocação, arquivo ou JUCE em `tick()`.

## Estado do marco

`AquariumPose`, `AquariumSpatializer` e sua integração nas quatro fontes
autônomas e nas dezesseis combinações organismo/voz externa estão ativos. O
modo técnico `--space-off` conserva a rota anterior para render controlado; não
é um controle artístico da interface.

Evidência em 2026-08-12:

- nove testes Release e nove testes ASan/UBSan aprovados;
- teste de tempo real continua registrando zero alocações durante render;
- render autônomo estéreo de 30 s, sem MIDI e com feedback desligado: RMS
  `0,00830951`, pico `0,0433364`, delta máximo `0,0374994`;
- referência espacial anterior sob os mesmos 81 eventos: RMS `0,00838246`, pico
  `0,0445234`, delta máximo `0,0545386`;
- diferença entre rotas: RMS `-35,56 dBFS`; é diferença A/B, não stem do módulo;
- componente lateral L−R da rota nova: RMS `-50,57 dBFS`, portanto o arquivo é
  realmente estéreo, embora mais coeso que a referência — decisão de caráter a
  ser confirmada por escuta, não superioridade assumida;
- benchmark 48 kHz/bloco 128: `1,65%` autônomo e `5,29%` com quatro vozes
  externas, depois de retirar trigonometria redundante da taxa de áudio.

Artefatos: `validation-output/aquorbium-space-on/` e
`validation-output/aquorbium-space-off/`.

## Segundo marco isolado

O corpo compartilhado de reflexões já existe como `MultiTapSpace`: oito
cabeças Lagrange cúbicas, ganho/pan/filtro próprios, feedback com limite
analítico e macros híbridas de tamanho, abertura, tom e memória. Uma instância
wet-only está inserida depois da soma estéreo e responde ao Ecossistema a 200
Hz; o retorno é suavizado e pode ser retirado tecnicamente sem parar o corpo.
Modelo e evidência estão em
[`MULTITAP_SPACE_DESIGN.md`](MULTITAP_SPACE_DESIGN.md).
Os seis controles de Espaço são persistidos nas configurações do instrumento e
entram no áudio por atômicos, mantendo a thread de áudio sem alocação.


## Caixas ou fones: o binaural — 2026-10-21

O autor escolheu "fones e caixas (seletor)". O botão CAIXAS / FONES, no
cabeçalho, fica guardado em `audio.headphones`.

**CAIXAS** mantém a mistura estéreo, intacta e idêntica bit a bit. Ela
não é geométrica: soma quatro observadores fixos, como quatro microfones
com pans −0,78, +0,78, −0,26 e +0,26, pesados pela visibilidade. Medido,
o estéreo é estreito: a correlação entre esquerda e direita é de 0,93.

**FONES** é uma perspectiva nova: você é o observador 0, com a cabeça
reta, e cada organismo vem da direção real em que está na esfera
(`Ecosystem::directionFromListener`: azimute, elevação, distância).
- **O nível** de cada um continua vindo da mistura ecológica (a soma mono
  do par estéreo), então a visibilidade e o equilíbrio do ecossistema se
  mantêm.
- **A cabeça** é o modelo estrutural de Brown & Duda (1998), reescrito do
  artigo, sem nenhum banco de HRTF de terceiros:
  - a sombra da cabeça, um polo e um zero por orelha;
  - o atraso entre as orelhas, de Woodworth, com cabeça de 8,75 cm;
  - a orelha externa, cinco ecos que dependem da elevação e de frente e
    trás.
- **A troca entre os modos** faz uma transição de ~50 ms.

**Por que não Ambisonics de primeira ordem:** para cinco fontes pontuais,
a síntese binaural direta por fonte é mais nítida e mais barata. A
primeira ordem borra a localização. Ambisonics vale quando há muitas
fontes ou várias caixas, e fica como caminho para um arranjo multicanal.

**Medido e testado:**
- o atraso entre as orelhas de 0,667 ms (Woodworth: 0,656);
- a diferença de nível de 0,5 dB a 200 Hz e de 16 dB a 6 kHz (a sombra
  cresce com a frequência);
- frente idêntica nas duas orelhas;
- a elevação muda o espectro em até 18 dB (2–14 kHz).

**No motor:**
- o nível de FONES iguala o de CAIXAS (−36,03 contra −36,07 dB);
- a correlação entre esquerda e direita cai de 0,93 para 0,55: a cena se
  abre.

**Limitação:** as vozes tocadas por MIDI (o caminho das vozes de evento)
continuam estéreo; só o som autônomo dos organismos passa pela cabeça. O
espaço, a cauda e as bolhas continuam estéreo e funcionam nos fones.

**Escuta A/B** (60 s, mesma semente, ouvir de fones), em
`AUDIOS_RASGO/Aquorbium/escutas/2026-10-21-fones/`: caixas-estereo e
fones-binaural, −31,4 e −31,0 LUFS.
