# Aquorbium — percussão por amostra, morph harmônico e máscara de pulsos

Atualização: 2026-09-24.

## Objetivo

Este marco materializa três hipóteses do Módulo 3 como componentes isolados.
Eles ampliam o vocabulário disponível para experimentação, mas não entram no
callback, nas espécies, no painel nem no som do `./run` nesta etapa.

## Voz percussiva por amostra

`SamplePercussionVoice` é um fallback mínimo para golpes curtos e definidos.
Mantém quatro slots mono fixos de até 4.096 amostras cada. O carregamento copia
e saneia a amostra fora do callback; disparo e reprodução não alocam memória.

- seleção entre quatro amostras;
- pitch de −36 a +36 semitons por incremento fracionário;
- interpolação linear de leitura;
- decay exponencial entre 5 ms e 8 s, chegando a aproximadamente −60 dB;
- velocity e saída limitadas a `[0,1]` e `[-1,1]`.

O limite curto é deliberado: esta voz não é sampler geral, streaming de disco
ou substituta do `BiomaGranularCapture`. Antes de qualquer integração ainda é
necessário definir proveniência das amostras, ponto de carregamento, orçamento
de vozes e função musical que não duplique o Striker.

## Morph por coeficientes harmônicos

`HarmonicMorphOscillator` conserva duas famílias de até 16 coeficientes. O
controle de morph interpola cada coeficiente antes da síntese aditiva; não
mistura duas saídas de osciladores já prontas. As parciais compartilham a mesma
fase fundamental, são normalizadas pela soma absoluta e são interrompidas
acima de 45% da taxa de amostragem.

Essa proteção reduz a geração direta de parciais acima da faixa segura, mas
não substitui uma medição espectral nem garante ausência perceptiva de aliasing
em todas as modulações. Famílias definitivas, suavização de gesto e ligação
ecológica continuam abertas.

## Máscara estocástica de pulsos

`StochasticPulseMask` decide aceitar ou rejeitar somente na borda de subida do
pulso. A decisão fica congelada até a borda de descida, evitando alternância
probabilística dentro de um mesmo pulsaret. Seed explícita garante repetição e
os contadores de aceitos/rejeitados tornam a ação observável.

O componente recebe gates booleanos; ele ainda não está ligado ao
`BiomaPulsar`. A integração deverá definir qual estado ecológico governa a
probabilidade e verificar por escuta se a rarefação produz frase, silêncio ou
apenas perda de presença.

## Contrato e validação

- estados e buffers têm tamanho fixo;
- `trigger`, `tick` e `process` são limitados e não alocam;
- entradas inválidas recebem fallback finito ou são rejeitadas;
- saídas permanecem finitas e limitadas;
- seed e reset permitem repetição dos ensaios.

Em 2026-09-24, `aquorbium_percussion_morph_tests` validou seleção, pitch,
decay, limites, interpolação dos coeficientes, determinismo e decisão apenas
no onset. `aquorbium_realtime_tests` incluiu os três componentes sob contagem
de heap. O build Release completo e os 31 testes CTest passaram em 54,52 s.

## Próximo portão

Gerar renders isolados A/B, medir espectro e custo, fazer escuta comparativa e
atribuir uma função composicional inequívoca a cada componente. Só depois um
deles poderá ser integrado gradualmente ao app, sem misturar esse trabalho com
o diagnóstico ainda aberto de estalos e travamentos.
