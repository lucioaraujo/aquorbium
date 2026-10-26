# Aquorbium — laboratório de fontes de energia sonora

**Estado:** primeira integração autônoma estável; novas famílias permanecem em pesquisa
**Princípio:** MIDI pode iniciar ou dialogar, mas não deve ser a condição para o
Bioma produzir som. A fonte principal é a energia interna dos organismos e suas
relações.

## Famílias a investigar

| Família | Energia/matemática | Papel possível no Bioma | Pergunta de validação |
|---|---|---|---|
| Oscilador geométrico | fase, PolyBLEP/PolyBLAMP, morph | corpo tonal e matéria para waveshape | permanece expressivo sem aliasing audível? |
| FM/thru-zero | relação portador-modulador | tensão, deslocamento e instabilidade harmônica | a energia do modulador produz gesto distinto de simples detune? |
| Caos determinístico | Hénon, Lorenz, double-scroll | fonte contínua de deriva ou áudio não periódico | cria vida sem perder controle por seed? |
| Pulsar/FOF | pulsos, silêncio, formantes | organismos que respiram, agrupam ou viram clique | densidade altera ritmo e timbre como uma única energia? |
| Ruído + ressonador | excitação estocástica e modos | matéria, água, micropercussão e encontros | o ressonador transforma ruído em organismo reconhecível? |
| Feedback limitado | atraso, saturação, envelope | memória/autoescuta do organismo | a realimentação cresce musicalmente sem dominar ou instabilizar? |
| Waveguide/Karplus-Strong | energia circulando em delay filtrado | corda, casco, corpo percussivo | espécie integrada: preservar memória sofisticada sem encobrir os outros corpos |
| Granular autocapturado | memória do próprio mix | ecossistema que se alimenta do que já produziu | o material reaparece transformado, não como loop indistinto? |
| Morph harmônico aditivo | coeficientes interpolados antes da soma | passagem contínua entre famílias tímbricas | produz transformação própria sem duplicar o morph PolyBLEP? |
| Percussão por amostra curta | quatro memórias fixas, pitch e decay | fallback de ataque definido | acrescenta função inequívoca sem virar bateria escondida? |
| Núcleo fractal multirrate | fase autorreplicada em até oito camadas | ritmo lento ou textura de áudio pelo mesmo processo | a mudança de registro preserva identidade sem produzir apenas aspereza? |
| Vetor de função multirrate | curva de subida/descida em loop ou one-shot | envelope, LFO ou oscilador compartilhado | os três papéis permanecem legíveis sem duplicar motores? |

## Ordem inicial de protótipos

1. **Oscilador morph + modulação lenta do Bioma:** primeiro protótipo concluído
   no core: seno → serra → pulso com PolyBLEP nas formas descontínuas;
   densidade e deriva escolhem a matéria da onda. Falta comparação espectral e
   escuta de performance antes de considerar o desenho estabilizado.
2. **Pulsar:** primeiro protótipo concluído no core: densidade governa a razão
   pulso/silêncio, com pulso senoidal que evita click de gate retangular. Falta
   investigar agrupamento, relações de formante e eventos ecológicos.
3. **Ruído excitando ressonador:** primeiro banco modal integrado como terceiro
   organismo. Quatro senoides amortecidas recebem impactos determinísticos;
   `Material` interpola uma série harmônica com proporções de membrana circular.
   Falta escuta comparativa para calibrar presença, decaimento e material.
4. **FM/thru-zero e wavefold:** investigar como perigo/energia deslocam a
   forma de onda sem reduzir a experiência a um knob de distorção.
5. **Physical String/Waveguide:** espécie integrada depois da escuta da segunda
   iteração. Usa atraso fracionário, filtro de perdas, excitação por posição,
   dois polos e janela Hann. Sua função de memória circulante foi considerada
   diferente do corpo modal e do Striker.
6. **Percussão curta, morph harmônico e máscara de pulsos:** protótipos
   isolados concluídos em 24/09. O primeiro guarda quatro amostras curtas, o
   segundo interpola até 16 coeficientes e o terceiro congela uma decisão
   probabilística por pulsaret. Faltam render, espectro, benchmark e escuta;
   nenhum altera o `./run`.
7. **Geradores multirrate:** `FractalCore` e `VectorGenerator` compartilham
   implementação entre controle e áudio. Passaram nos testes de fase, ciclo,
   one-shot, determinismo, limites e heap; ainda não têm conexão musical.

O primeiro campo autônomo já combina morph, granular, modal, pulsar e string com
energia, altura e duração próprias por organismo. Ele passa por testes de determinismo,
finitude, invariância de bloco, zero alocações e render sem MIDI. Isso estabiliza
a infraestrutura de nascimento; não encerra o dossiê tímbrico dessas fontes.

Nenhuma família entra apenas por ser complexa. Cada protótipo deve ter seed
reproduzível, limites de nível e CPU, teste de finitude, comparação auditiva e
uma ligação explícita a densidade, deriva, presença ou relação entre organismos.

## Dossiê de investigação por fonte

Cada possibilidade ganha um dossiê antes de virar módulo. Para osciladores,
isso significa estudar formas (seno, geométricas, wavetable, pulsar, formante,
caótica, estocástica), modelos de fase/energia e funções em registros de áudio
e controle — não apenas adicionar mais uma onda. Para as outras famílias vale a
mesma profundidade.

O dossiê responde, no mínimo:

1. **Questão musical:** que comportamento ou gesto ainda ausente ele cria?
2. **Modelo:** equação, estado, estabilidade, faixa de parâmetros e limites.
3. **Variações:** versões previsíveis, extremas e experimentais; o que muda
   quando a fonte é acelerada, desacelerada, excitada ou realimentada.
4. **Diálogo:** entradas e saídas de áudio, CV, evento e análise; que módulo
   pode modular, deformar, excitar ou observar essa fonte.
5. **Performance:** quais controles são tocáveis e quais devem continuar como
   energia autônoma do Bioma.
6. **Evidência:** teste numérico, CPU, espectro quando aplicável, render por
   seed e escuta comparativa registrada.
7. **Destino:** núcleo estável, modo experimental ou estudo arquivado.

Assim investigaremos a fundo uma possibilidade de cada vez e guardaremos a
aprendizagem mesmo quando o resultado não entrar na versão final.

## MIDI

MIDI é uma porta opcional para notas, gesto ou sensores convertidos em
controle. Ele não define a arquitetura: o banco autônomo já nasce dos estados
internos do Bioma sem teclado, enquanto um banco externo separado permite que a
performance dialogue com ele sem interromper ou roubar seus organismos.
