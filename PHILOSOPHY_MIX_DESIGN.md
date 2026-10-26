# Aquorbium — mistura e percurso de filosofias

Atualização: 2026-09-25.

## Objetivo

Este marco cria uma base matemática comum para quatro fontes de comportamento
ou escuta. Ele testa mistura vetorial, sequência, morph e matriz antes de
atribuir nomes e consequências ecológicas às quatro filosofias. Nada foi
ligado ao `Ecosystem`, às cenas, à GUI ou ao áudio ativo.

## Mistura vetorial

`PhilosophyVectorMixer` posiciona quatro fontes nos cantos de um plano:
inferior esquerdo, inferior direito, superior esquerdo e superior direito. Um
ponto XY produz pesos bilineares cuja soma permanece igual a um. Nos cantos,
somente uma fonte está ativa; no centro, cada fonte recebe 25%.

Esse contrato permite substituir futuramente um eixo abstrato de
`philosophyBlend` por uma posição bidimensional sem definir ainda qual
filosofia ocupa cada canto.

## Sequência

`PhilosophySequencer` mantém até 16 passos XY. Cada passo dura de um a 65.535
pulsos e só avança na borda ascendente do relógio. O padrão inicial percorre os
quatro cantos. A origem concreta do relógio no Cérebro permanece pendente.

## Morph discreto

`PhilosophyMorph` faz uma transição linear com duração definida entre quatro
fontes discretas. Uma mudança durante a transição parte do estado misturado
atual, evitando salto de peso. O protótipo realiza o crossfade das saídas, mas
não contém banco de cenas e não muda enums ou estados do Ecossistema.

## Matriz de saídas

`PhilosophyMatrixMixer` implementa uma matriz 4×4. As quatro linhas formam dois
barramentos estéreo independentes. Cada célula possui ganho base e profundidade
de modulação; o processamento limita a modulação e normaliza cada linha pelo
orçamento absoluto de ganhos.

A matriz começa como identidade. Ela não substitui ainda `panLeft`, `panRight`
ou os pesos dos observadores. Varredura autônoma e mapeamento CV ecológico
permanecem pendentes.

## Contrato de tempo real

- quatro fontes, quatro saídas e no máximo 16 passos em memória fixa;
- ausência de alocação, lock, I/O e chamadas ao sistema;
- pesos XY não negativos e normalizados;
- transição com duração independente do tamanho do bloco;
- ganhos e modulações limitados, com normalização por linha;
- entradas inválidas são retidas, limitadas ou substituídas por valor neutro;
- nenhuma rota, callback, tela ou parâmetro do app alterado.

## Validação

`aquorbium_philosophy_mix_tests` cobre cantos, centro, soma dos pesos, bordas de
relógio, duração dos passos, continuidade e término do morph, independência dos
dois barramentos, modulação, normalização e saneamento. O teste de tempo real
executa sequenciador, morph e matriz sob rastreamento de alocações.

Em 2026-09-25, o build Release completo e os 38 testes CTest passaram em
42,50 s.

## Próximos portões

Definir as quatro filosofias reais e sua ordem espacial; comparar blend linear
e potência constante por escuta; escolher o relógio; testar padrões lentos;
decidir o vínculo com cenas e observadores. Só depois integrar gradualmente a
matriz e o blend, com teste interativo de resposta, estalos e travamentos.
