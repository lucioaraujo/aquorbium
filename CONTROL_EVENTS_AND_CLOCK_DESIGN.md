# Aquorbium — eventos de controle e relógio fracionário

Atualizado em 2026-09-23. Estado: protótipos isolados compilados e testados;
sem integração ao Ecossistema, ao sequenciador ou ao aplicativo.

## `ControlEventExtractor`

Converte qualquer sinal contínuo limitado em dois vocabulários discretos:

- cruzamentos positivo e negativo de zero, com histerese;
- subida e queda de uma envoltória, com tempos independentes de ataque e
  relaxamento, limiar e histerese de gate.

Assim, Lorenz, random walk, Relabi, energia de áudio ou movimento podem gerar
eventos sem cada consumidor reinventar detecção. O módulo não decide o que o
evento significa. Uma futura ligação deverá escolher explicitamente se ele
avança presença, estratégia, passo, seção ou excitação sonora.

## `FractionalLogicClock`

Mantém dois acumuladores de fase com razões racionais independentes, de 1/32 a
32/1 sobre BPM. Cada ramo produz um gate; AND, OR ou XOR produz o gate composto
e um trigger apenas na borda de subida. Razões como 7/4 e 5/4 conservam sua
contagem em qualquer divisão de blocos porque o estado é contínuo por amostra.

Este relógio é interno e não substitui `AudioPlayHead`: sincronização de host,
reposicionamento de transporte e mudança de BPM continuam tarefas próprias.

## Contrato e validação

Ambos usam estado fixo, rejeitam valores não finitos e não alocam, travam ou
fazem I/O no processamento. `EventClockTests` cobre cruzamentos, histerese,
razões 7/4 e 5/4, invariância de particionamento e os três modos lógicos.
`RealtimeSafetyTests` os executa sob contagem de heap. A suíte completa passou
em 25/25 testes em 23/09/2026.

Antes da integração: escolher fonte e destino de cada evento; limitar taxa de
eventos; estabelecer reset no transporte; testar mudanças de BPM/razão sem
duplicar triggers; escutar se a lógica composta cria variação reconhecível sem
transformar a autonomia em grade mecânica.
