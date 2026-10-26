# Aquorbium — controle estrutural lento

Atualização: 2026-09-24.

## Objetivo

Este marco materializa três transições de longa duração sem conectá-las ao
aplicativo: percurso condicional entre seções, deriva em três escalas e
passagem suave entre integridade saudável e trincada.

## ConditionalStageMachine

`ConditionalStageMachine` mantém oito estágios fixos. Cada estágio contém um
valor, duração de 1 a 1.000.000 de ticks, destino normal, destino de salto e
uma condição opcional:

- perigo acima de um limiar;
- período noturno;
- stress de trinca acima de um limiar.

A condição é avaliada apenas na fronteira do estágio. O resultado informa
entrada, avanço, salto e próximo índice. Isso permite construir forma musical
sem consultar `Environment` ou `AquariumIntegrity` dentro do componente. O
mapeamento desses estados continua sendo uma decisão ecológica posterior.

## TripleSlowDrift

`TripleSlowDrift` combina três instâncias independentes de
`SlewRateRandomWalk`. Os períodos padrão são 5 segundos, 1 minuto e 20
minutos; pesos padrão 0,5/0,3/0,2. Seeds derivadas impedem que as três linhas
percorram a mesma trajetória.

Para suportar dezenas de minutos, o random walk original passou a aceitar
0,0001 evento/s e suavização de 0,0001 Hz, além de relógios de controle a
partir de 1 Hz. Os limites anteriores impediam eventos mais lentos que 100
segundos. Os testes anteriores foram mantidos e atualizados para o novo limite.

## IntegrityCrossfade

`IntegrityCrossfade` transforma `crackStress` em um alvo suave entre dois
limiares. Uma rampa exponencial evita saltos e os ganhos saudável/trincado usam
lei de potência constante:

`healthyGain² + crackedGain² = 1`.

O componente apenas produz ganhos. Ele não mistura áudio, não altera
`AquariumIntegrity` e não define quais estados sonoros representam saúde ou
fratura.

## Contrato e validação

- memória fixa e processamento limitado;
- nenhum lock, I/O ou alocação em `tick()`;
- contexto e parâmetros inválidos são limitados ou recebem fallback;
- deriva e estágios são reproduzíveis;
- crossfade é monotônico, finito e de potência constante.

`aquorbium_structural_control_tests` cobre saltos por perigo, noite e trinca,
avanço normal, escalas de deriva, períodos longos, determinismo, suavidade e
potência. A regressão de `SlewRateRandomWalk` e o teste de heap também passam.
Em 2026-09-24, o build Release e os 34 testes CTest passaram em 41,01 s.

## Próximo portão

Definir um único uso inicial para a máquina de estágios, renderizar horas
aceleradas da deriva e comparar crossfade linear, potência constante e mudança
discreta. Somente depois os componentes poderão receber dados reais de
`Desire`, `Environment` e `AquariumIntegrity`.
