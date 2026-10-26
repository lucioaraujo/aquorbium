# Aquorbium — protótipo de autocaptura granular

Atualizado em 2026-09-29. Estado: core integrado experimentalmente ao banco.

## Função e fronteira

`BiomaGranularCapture` testa a hipótese de que acontecimentos sonoros recentes
do próprio aquário possam voltar como matéria de grãos. Ele é diferente de
`BiomaGranularStriker`: o Striker sintetiza partículas a partir de ruído;
este motor reproduz fragmentos do sinal recebido por `process(input)`.

O código é implementação original local a partir da arquitetura do Aquorbium.
Não usa código ou amostras de terceiros, arquivos, JUCE ou dependências DSP
externas. A referência conceitual de grãos com parâmetros individuais e
captura por onset já estava registrada em `aquorbium-arquitetura.md`.

## Contrato do protótipo

- Entrada mono limitada a [-1, 1]; saída estéreo somente molhada. O banco usa
  o downmix pré-Ambiente/Espaço/Cauda e não realimenta a saída granular no
  próprio histórico.
- Histórico circular fixo de 65.536 amostras mono (256 KiB) e até 12 grãos.
  Um único corpo compartilhado é a hipótese inicial; multiplicar por voz ou
  organismo exigiria nova decisão de memória, CPU e legibilidade musical.
- Um grão congela duração, velocidade 0,5–2×, direção, panorama e ganho no
  instante de `trigger()`. Alterar os valores padrão não retuna grãos ativos.
  O pool cheio recusa novos grãos e conta a recusa; não há roubo silencioso.
- Duração pedida de 5–150 ms, limitada internamente a 8.192 amostras para
  preservar uma zona de leitura anterior à escrita em todas as taxas
  admitidas. Em 96 kHz, o máximo efetivo é ~85 ms; em 768 kHz, ~10,7 ms.
  A API ainda não expõe essa duração efetiva em telemetria.
- Leitura fracionária linear e janela Hann tabulada com extremos zero. O
  início é colocado no passado com margem de 64 amostras e espaço para até
  2× de velocidade, evitando ler a região que está sendo sobrescrita.
- Ataques são estimados pela razão entre envoltórias de 3 e 50 ms, com piso
  e período refratário. `autoCapture` é desligado por padrão. O detector
  responde ao mix real recebido pelo banco.
- Em cada evento, duração, velocidade, panorama, ganho e reversão são
  congelados a partir de `strength`, `agility` e `intelligence` do organismo
  `GranularStriker`, combinados com o contexto formal do evento.
- Memória preparada e zerada fora do processamento; `process()` e `trigger()`
  não alocam, não travam e não fazem I/O. A soma usa teto fixo por grão de
  `0,8/12`, que protege headroom mas pode ser musicalmente discreto.

## Validação e portões

`tests/LorenzCaptureTests.cpp` verifica silêncio, limites, sobreposição,
direção, velocidade, snapshot, detecção de ataque, entradas inválidas e
histórico após wrap. `RealtimeSafetyTests.cpp` inclui um grão dentro da
contagem de heap do processamento. A suíte completa passou em 23/23 casos.
`tools/LorenzCaptureBenchmark.cpp` mediu um corpo e um gerador Lorenz em
48/96 kHz: 0,626%/1,063% de um núcleo sob a carga artificial de até 12
grãos. As recusas foram contadas; o benchmark não representa o custo do app
inteiro nem certifica prazo real de callback. Dados completos e condições
estão em `aquorbium-tarefas.md`.

Após integrar: inspecionar aliasing
da leitura interpolada em transposições; renderizar A/B com Striker e silêncio
de entrada; confirmar audibilidade em mix sem mascarar os cinco organismos;
definir captação pós/pré-espaço e evitar feedback involuntário; escutar com
usuário após a travada e os estalos serem investigados. Não criar controles
na GUI nem declarar o módulo musicalmente aprovado só pelos testes numéricos.
