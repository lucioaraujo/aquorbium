# Aquorbium — organismo ressonador modal

## Pergunta musical

Como dar ao terceiro organismo um corpo que recebe, conserva e dissipa energia,
em vez de apenas trocar sua forma de onda, sem copiar um instrumento acústico
ou reduzir “ressonador” a um filtro genérico de Q alto?

O ressonador deve responder aos encontros do Bioma como matéria: um evento
curto excita vários modos, cada um persiste por um tempo próprio, e o som atual
carrega memória dos impactos anteriores.

## Pesquisa apropriada

Sistemas lineares vibrantes podem ser descritos por modos independentes. Em
síntese modal, o som resulta da soma de senoides amortecidas, cada uma com
frequência, decaimento e amplitude próprios. A implementação do Aquorbium usa
esse princípio, mas sua identidade não tenta reproduzir um objeto real.

Uma membrana circular ideal possui frequências modais proporcionais aos zeros
de funções de Bessel, portanto não forma uma série harmônica inteira. As
primeiras proporções escolhidas para o extremo “membrana”, normalizadas pela
fundamental, são aproximadamente `1, 1.593, 2.136, 2.295`. O extremo oposto usa
`1, 2, 3, 4`. `Material` percorre continuamente essas duas geometrias.

Fontes primárias/institucionais:

- University of Illinois, *Vibrations of Ideal Circular Membranes*:
  <https://courses.physics.illinois.edu/phys406/sp2017/Lecture_Notes/P406POM_Lecture_Notes/P406POM_Lect4_Part2.pdf>
- Penn State Graduate Program in Acoustics, modos de membrana circular:
  <https://www.acs.psu.edu/drussell/demos/membranecircle/circle.html>
- W3C Audio EQ Cookbook, referência institucional para filtros digitais de
  segunda ordem: <https://www.w3.org/TR/audio-eq-cookbook/>
- McGill, síntese modal como soma de senoides amortecidas:
  <https://www.cs.mcgill.ca/~kry/pubs/dafx/ModalAnalysis_Final.pdf>

Nenhum código dessas fontes será copiado. Equações e proporções físicas são
matéria de pesquisa; estado, API, excitação, orçamento e integração são desenho
próprio do instrumento.

## Modelo DSP

`BiomaModalResonator` possui quatro modos complexos. Cada amostra gira o estado
do modo por sua frequência e reduz seu raio por um fator exponencial menor que
um. Um ruído determinístico curto injeta energia no plano complexo. O raio é
calculado a partir do tempo para a amplitude cair a `1/e`, tornando o parâmetro
de decaimento independente da taxa de amostragem.

O módulo mantém:

- quatro estados complexos e coeficientes de rotação;
- gerador pseudoaleatório próprio, separado do Cérebro;
- envolvente de excitação com duração definida por `Hardness`;
- filtro simples da excitação para variar brilho sem realimentação instável;
- teto de energia modal que escala estados antes de qualquer overflow;
- corte explícito de caudas subnormais e recuperação para silêncio se surgir
  valor não finito.

Não há oversampling porque o primeiro marco é linear. Qualquer não linearidade
futura exigirá estudo de aliasing separado.

## Controles musicais

| Controle | Extremos | Consequência |
|---|---|---|
| Fundamental | altura segura | centro modal no momento do impacto |
| Material | harmônico → membrana | relação entre os quatro modos |
| Decay | curto → longo | memória do corpo |
| Brightness | amortecido → aberto | energia relativa e persistência dos modos altos |
| Hardness | sopro → impacto | duração e espectro do ruído excitador |
| Strike Position | borda → centro → borda | modos reforçados ou cancelados |

`excite(amount)` adiciona energia com limite; não reinicia fase nem apaga a
cauda. Repetir eventos constrói uma história em vez de repetir uma amostra.

## Modo autônomo e performance

No caminho autônomo:

- pitch geométrico fixa a fundamental no instante do evento;
- calma amplia o decaimento;
- perigo endurece o impacto;
- detalhe/observação abre os modos altos;
- posição X do organismo define o ponto de impacto;
- energia, acento geométrico e integridade limitam a força;
- somente um evento acordado/gateado excita o corpo.

No caminho externo, uma nota pode excitar o mesmo corpo modal sem substituir a
bioenergia. `PerformanceIntent` interpola continuamente material, decaimento,
brilho, dureza e ponto de impacto contra a decisão ecológica; influência zero
preserva o autônomo bit a bit. O novo alvo é comprometido somente no próximo
impacto, evitando mover polos no meio de uma cauda. A GUI só ganhará uma
superfície dedicada depois que escuta e medição confirmarem que cada macro
merece espaço.

## Invariantes e testes

- C++20 portátil, memória fixa, sem JUCE, alocação, lock ou I/O em `tick()`;
- coeficientes finitos e polos sempre estritamente dentro do círculo unitário;
- frequências abaixo de 45% da taxa de amostragem;
- silêncio exato antes da primeira excitação e retorno a silêncio após a cauda;
- mesma seed, parâmetros e eventos produzem as mesmas amostras;
- seed diferente altera o transiente, sem mudar altura/limites;
- excitações sobrepostas permanecem finitas e dentro do teto;
- parâmetros inválidos preservam o último estado válido;
- energia e máximo delta medidos em ataques suaves e duros;
- integração preserva invariância de bloco e zero alocações do core.

## Fora deste marco

Waveguide/Karplus–Strong, ressonância não linear, acoplamento físico entre
organismos, análise modal de amostras, banco com dezenas de modos, morph de
topologia durante uma cauda e ressonância do próprio aquário. São pesquisas
posteriores, não funções implícitas deste organismo.

## Evidência do marco

- oito testes Release e oito testes ASan/UBSan aprovados no conjunto completo;
- teste modal cobre proporções, raio dos polos, silêncio, cauda, seed, dados
  inválidos, intenção performática e 200 impactos sobrepostos;
- render autônomo de 30 s sem MIDI: sete impactos, RMS `0,00836083`, pico
  `0,0444968`, maior delta adjacente `0,0545138` e DC abaixo de `1,8e-6`;
- ao final, a cauda modal ainda tinha energia `0,00457161`, comprovando memória
  além do ataque;
- render de referência morph usa exatamente seed, eventos e relógios iguais,
  pois ambos os motores avançam mesmo quando não são ouvidos; os RMS globais
  ficaram próximos (`0,00836083` modal e `0,00838855` referência), adequados a
  escuta comparativa sem alegação automática de superioridade;
- benchmark em 48 kHz/bloco 128: `1,29%` só autonomia e `3,74%` com quatro vozes
  externas na máquina de referência.

Artefatos:

- `validation-output/aquorbium-resonator-on/` — organismo modal;
- `validation-output/aquorbium-resonator-off/` — fonte morph provisória,
  produzida com `--resonator-off` para comparação técnica.
