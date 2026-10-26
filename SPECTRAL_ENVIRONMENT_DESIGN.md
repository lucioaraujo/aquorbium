# Aquorbium — assinatura espectral e ambiente

Atualização: 2026-09-25.

## Objetivo

Este marco materializa duas famílias relacionadas e a primeira integração
operável do lote de protótipos de setembro: uma assinatura espectral de oito
bandas e um leito de ruído governado por ciclos e condições da água. Ambos
entram no `BiomaEngine` depois da soma dos organismos e antes do Espaço.

## SchoolSpectralBank

`SchoolSpectralBank` contém oito filtros passa-banda ressonantes. `centre`
define o centro logarítmico do território espectral e `span` sua dispersão em
oitavas. `resonance` controla a seletividade comum das bandas.

Uma identidade formada por `schoolId` e seed cria um perfil fixo de oito
ganhos entre 0,35 e 1. O perfil é determinístico e não consome o acaso do
Cérebro. A soma é normalizada pelo orçamento total de ganhos, evitando que um
perfil mais cheio seja automaticamente mais alto.

O componente reúne duas hipóteses antes separadas: FREQ/SPAN como equivalentes
espectrais de centro/dispersão e um banco de oito bandas como assinatura
tímbrica. A primeira integração usa um perfil compartilhado no mix estéreo;
a ligação diferenciada a `SchoolInfo::schoolCohesion`, posição ou espécie
permanece pendente.

## EcologicalNoiseBed

`EcologicalNoiseBed` gera dois canais determinísticos a partir de seed. A soma
cíclica de `seasonPhase` e `dayPhase` percorre quatro texturas: fluxo profundo,
ruído filtrado, ruído claro e ruído aéreo com eventos esparsos. Não existe um
controle manual independente de morph.

Turbidez e temperatura definem juntas o corte do filtro de cor: água turva ou
fria escurece o leito; água clara ou quente preserva mais conteúdo agudo.
`crackStress` reduz a resolução temporal por sample-and-hold, de uma amostra
no estado íntegro a 64 amostras no estado máximo. Na integração atual, estação,
hora, temperatura e trinca vêm do Ecossistema; a turbidez é a média dos cinco
organismos.

## Superfície de performance

O cartão `05 AMBIENTE` oferece seis macros grandes:

- `ENV MIX`: influência total e chave de bypass transparente;
- `FILTER`: quantidade de filtragem espectral;
- `CENTER`: centro logarítmico das oito bandas;
- `SPAN`: dispersão em oitavas;
- `RESONANCE`: seletividade comum das bandas;
- `AMBIENCE`: presença do leito ecológico.

Com `ENV MIX=0`, os outros cinco controles ficam visualmente inativos e não
alteram a saída. Todos os valores são suavizados em 15 ms, persistidos nas
cenas e publicados ao áudio num quadro coerente. O estado V1 antigo, sem esses
seis campos, continua aceito e recebe os valores iniciais seguros.

## Contrato de tempo real

- oito filtros e dois estados de ruído em memória fixa;
- ausência de alocação, lock, I/O e chamadas ao sistema no processamento;
- seed reproduzível e identidade estável;
- parâmetros inválidos retidos ou limitados;
- frequências limitadas abaixo de 42% da taxa de amostragem;
- saída ambiental limitada pelo ganho máximo de 0,5;
- coeficientes atualizados no relógio de controle de 200 Hz;
- DSP interno continua avançando no bypass, evitando reinicialização ao ativar;
- `ENV MIX=0` preserva a saída anterior bit a bit no cenário automatizado.

## Validação

`aquorbium_spectral_environment_tests` cobre ordenação e dispersão das bandas,
identidade determinística, resposta finita, ruído reproduzível, limite de
amplitude, mapas de ciclos/cor/trinca e saneamento. O teste de tempo real
executa os dois componentes sob rastreamento de alocações.

Em 2026-09-25, a integração compilou no core e no aplicativo. Os testes
direcionados de resposta, estado/cenas, banco de presets, core espectral e
tempo real passaram (6/6). Cada uma das seis macros produziu diferença
mensurável quando ativa; `ENV MIX=0` produziu diferença zero; o cenário de
quatro vozes atingiu pico 0,535761. A auditoria offline gerou 16 capturas em
quatro dimensões e confirmou knobs de pelo menos 52 px, áreas de 80×92 px,
rótulos/valores separados e rolagem sem compressão. Depois da recompilação
final daquele marco, a suíte completa passou 39/39 em 45,86 s. O binário do
marco tinha SHA-256
`cd400f36aff8959276fe421c707c0d794aa0ec3ca0e3b910a39b3165aa20bdeb`; ele foi
substituído depois pela integração da Cauda.

O renderizador reproduzível `aquorbium_spectral_environment_render
DIRETORIO_NOVO` fechou a medição offline. Ele gera quatro WAVs alinhados de
12 segundos (`bypass`, `filter`, `ambience`, `combined`) e recusa um diretório
já existente. A evidência preservada em
`validation-output/2026-09-25-spectral-environment/` registrou repetição do
bypass bit a bit, picos de 0,0208 a 0,2546 e diferenças RMS contra o bypass de
0,04198, 0,01367 e 0,02363. O relatório inclui RMS, DC, maior salto entre
amostras e oito proxies de bandas. Isso é evidência técnica, não escuta.

## Próximos portões

A família está integrada, operável e validada tecnicamente, mas ainda não está
aprovada musicalmente. É necessário ouvir a faixa útil das seis macros,
verificar cliques/estalos e estabilidade no dispositivo real e decidir se o perfil deve variar por
`SchoolInfo` ou espécie. Os estalos e travamentos históricos do standalone
continuam sendo um diagnóstico independente.
