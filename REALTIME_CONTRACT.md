# Aquorbium — contrato entre threads

Atualização: 2026-09-30.

Este documento registra a fronteira de tempo real do app JUCE. Ele descreve o
que o código implementa hoje; não antecipa APVTS, plugin ou automação de host.

## Propriedade do estado

### Correção e diagnóstico de 2026-09-18

O teste humano reportou estalos e travada da aplicação; estabilidade não está
aprovada. A continuidade samplewise introduzida em 17/09 fazia os valores de
`audioMix()` mudarem a cada amostra. Isso invalidava o cache de `setGainPair`
e repetia conversões polares, trigonometria e coeficientes de filtro por
amostra para cada espacializador ativo. Agora esses alvos são atualizados no
relógio interno de 200 Hz, independente dos blocos; tick, ganho (20 ms),
tom (45 ms), mistura ecológica (15 ms) e fontes continuam por amostra.
Isso muda ligeiramente a rota acústica: os pares anteriores são históricos,
não referência bit a bit desta versão. Não altera o número de organismos.

DENSITY/DRIFT também tinham uma rota direta sem rampa: mudavam amplitude,
forma do oscilador e duty do pulsar imediatamente. Alvos ecológicos continuam
inalterados; dois floats sonoros recebem um polo de 15 ms calculado em prepare,
sem exponenciais no laço. A telemetria distingue alvo/valor sonoro corrente;
o teste verifica o primeiro passo em 44,1/48/96 kHz. Parâmetros capturados por
novos impactos continuam sendo decisões de evento, não caudas retunadas.

O callback standalone usa `ScopedNoDenormals`, constrói/publica telemetria
no máximo uma vez por bloco e aproximadamente a 30 Hz quando os blocos são
pequenos, em vez de fazê-lo em todo bloco. Contadores atômicos lock-free
registram callbacks acima do orçamento e blocos com magnitude ≥1. Não há
I/O, log textual, seletor ou limiter no callback. Leitura de dispositivo,
CPU e xruns acontece na GUI a 2 Hz; seleção e persistência do dispositivo
ficam fora dos presets. Esses contadores são diagnósticos, não certificação.
Um contador exclusivo da GUI marca intervalos entre timers acima de 250 ms;
inclui possíveis suspensões do sistema e não identifica sozinho a causa.

Após a escuta de 20/09, a captura da janela mostrou 48 kHz/16 amostras,
XRUN 209, LATE 198 e CLIP 0. A duração nominal do bloco era 0,333 ms; isso
é compatível com falha de prazo e não prova clipping digital. Para a saída
ALSA/PipeWire, a GUI corrige buffer salvo abaixo de 512 antes de abrir o
dispositivo. Em 21/09 o seletor genérico foi substituído por um painel que
não oferece menos de 512 nessa saída; a correção posterior no timer foi
removida para não reabrir o dispositivo durante a execução normal. Hardware
direto conserva opções a partir de 128. A abertura isolada com XML de 16
amostras confirmou 48 kHz/512 em teste curto; a estabilidade prolongada
ainda precisa de validação humana.

Integração de 20/09: `--capture-audio` é opt-in, com saída explícita
`--capture-output`, preferências isoladas, diretório novo e máximo de 120 s.
O callback apenas copia um bloco para uma fila SPSC fixa ou conta quadros
perdidos; não escreve em disco, não espera e não aloca. A thread consumidora
produz WAV float32 estéreo e atualiza seu cabeçalho ao finalizar. A GUI
amostra CPU/XRUN/LATE/CLIP/pausas e cinco controles a cada ~0,5 s; gera
`REPORT.txt` fora do callback. Mudança de taxa de amostragem pede parada
sem espera no callback e é sinalizada no relatório. O modo normal só paga
uma leitura de ponteiro atômico por bloco, com ponteiro nulo. Teste unitário
e suíte completa passaram (21/21); smoke real de 3 s no áudio interno
ALSA/PipeWire produziu 144.000 quadros a 48 kHz, sem perdas/erros contados.
Isso não confirma ausência de estalos perceptivos nem de travada prolongada.

Revisão de 21/09: o escritor do WAV tem prazo próprio por tempo de parede
(duração pedida + 2 s). Se o callback cessar, a thread de disco ainda fecha
o cabeçalho sem depender do timer da GUI; `REPORT.txt` já contém metadados
iniciais antes da captura. O relatório final marca se o prazo foi atingido.
Contadores separados no callback indicam entradas e conclusões; uma diferença
entre eles sugere parada dentro do callback, enquanto ambos imóveis sugerem
falta de entrega pelo dispositivo. É diagnóstico, não localização definitiva
da causa da travada. Teste unitário cobre o caso de callback interrompido.

`STRIKER MIX` também escala somente o sinal granular de 1 a 8 vezes, com
rampa acústica de 15 ms; zero preserva o balanço autônomo. `SPACE MIX`
também escala a meta de retorno até 2 vezes, limitada pelo teto de 0,75 e
pela rampa anterior do retorno. Não há I/O, heap ou lock novos no callback.
O binário Release de 21/09 completou um ensaio automatizado de 20 s no
dispositivo padrão ALSA/PipeWire a 48 kHz/512 com XRUN, LATE, CLIP, pausas
GUI >250 ms e amostras inválidas em zero; pico `0,151536`. O teste numérico
com quatro vozes e controles máximos teve pico `0,535761`. Ambos são
protocolos limitados; não reproduziram a travada nem certificam escuta.

### Diagnóstico limitado com dispositivo real — 2026-09-18

`--runtime-audit` é opt-in: preferências isoladas, diretório novo, protocolo
de 20 segundos (60 com `--runtime-stress`) e até 256 leituras GUI em array
fixo na revisão de 19/09. Nome explícito de saída
deve corresponder exatamente a um dispositivo; uma abertura com fallback não
vale como ensaio da saída solicitada. Não habilita entradas MIDI nem captura.
Aplicar timeout externo de 35 s, pois o próprio timer não pode encerrar um
event loop bloqueado (80 s externos para o perfil de 60 s). Timeout,
cancelamento ou relatório ausente não são sucesso.

O callback acumula contagem de amostras, valores não finitos, soma de quadrados,
pico e delta consecutivo em escalares fixos. A flag desse modo não muda enquanto
o callback estiver conectado. A GUI somente lê os acumuladores depois de
`shutdownAudio()` retirar/juntar o callback; gera então o relatório em disco.
Leituras periódicas de CPU/xrun e controles programáticos permanecem na GUI.
Esse trabalho de medição acrescenta custo ao ensaio, não ao laço normal por
amostra. Não é detector de cliques, gravação de áudio ou alteração dos DSPs.

AudioBox direta/ALSA e saída padrão ALSA via PipeWire completaram 20 s cada,
ambas a 48 kHz/512, com zero XRUN/LATE/CLIP, pausas GUI >250 ms, overflow ou
amostras não finitas. As mudanças seguem o relógio GUI, portanto os dois
ensaios não constituem replay bit a bit. Nenhuma escuta foi aprovada e a
travada não foi reproduzida; reteste prolongado/interativo continua necessário.
Evidências e medições completas ficam na fila local, não neste resumo.

Extensão de 19/09: cenas de diagnóstico são construídas antes de abrir áudio
e recuperadas pela mesma fronteira transacional da GUI. Depois, alterna-se a
autonomia; a fase de quatro vozes gera no máximo oito eventos a cada 250 ms,
usando a fila existente, com liberação final. Isso testa eventos internos
programáticos, não transporte MIDI físico. Não há novo motor ou calibração.
`RuntimeAuditProtocol.h` limita duração/fases e rejeita tempos inválidos antes
da conversão para inteiro; transições e extremos são cobertos pelo teste de
resposta dos controles. O seletor de áudio fica inativo durante a medição
para manter a rota escolhida.

Os dois perfis de 60 s concluídos em 19/09, AudioBox direta e ALSA/PipeWire,
registraram 5.627/5.625 callbacks e 60,021/60,000 s de áudio. Em ambos:
XRUN, LATE, CLIP, GUI >250 ms, overflow e amostras não finitas ficaram em
zero. Picos 0,2621/0,2714 e deltas consecutivos máximos 0,1069/0,1125 não
constituem detecção de clique; a leitura precisa ser correlacionada a áudio
ou escuta. Os resultados não reproduziram a travada relatada.

O seletor de áudio é uma janela não modal, sem event loop aninhado. Sua vida
é limitada pelo componente; é destruído antes do gerenciador de dispositivo.
O modo `--layout-audit` não inicializa dispositivos nem entradas MIDI.

Validação de 18/09: 20/20 CTest em 35,59 s. Mesmo benchmark do core, 45
configurações: 48 kHz/512 amostras/quatro vozes + autonomia passou de 19,83%
para 7,55% de custo relativo ao áudio; 96 kHz/128/quatro vozes, de 38,82%
para 14,63%. Máximo novo entre casos: 14,88%. Os percentuais são medianas
offline, não uso total da máquina nem pior deadline do standalone. Não
incluem a GUI, o backend, o dispositivo ou o escalonamento do sistema.

- A thread de áudio é a única proprietária mutável de `BiomaEngine`.
- GUI e callbacks MIDI nunca chamam setters, `noteOn`, `noteOff`, `render` ou
  `telemetry` diretamente no core.
- A GUI publica um `AudioControlFrame` completo por `FixedLatestSnapshot`:
  61 controles contínuos persistentes, ativação autônoma e posição/posse
  transitória da Lupa.
  O callback consome no máximo um quadro novo antes de renderizar o bloco.
  Não há mistura de valores de duas cenas. O gate usa a fila de eventos abaixo.
- Arrastar a Lupa publica posição esférica e posse no mesmo quadro.
  Só o callback chama `BiomaEngine::setLensGesture()`, que atualiza
  o observador 0 no core. O alvo é limitado pelo core e a Lupa desliza no
  relógio ecológico de 200 Hz; a GUI não muda a física diretamente.
- Desde 2026-09-17, os alvos ecológicos `mix()` são separados de `audioMix()`:
  cinco estados fixos de quatro floats, interpolados a cada amostra por um
  polo de 15 ms. O coeficiente depende da taxa e só é calculado em `prepare`;
  o laço sonoro não aloca nem calcula exponenciais. Ganhos e detalhe dos
  motores recebem os valores suavizados; telemetria conserva os alvos.
  Pressionar/soltar não reinicia a suavização. Reset copia o estado inicial,
  sem fade de partida. Continuidade de controle não garante ausência de
  clique audível, de mudanças de excitação ou de xruns no dispositivo.
- A página `ECO CONTROLS` mantém sliders e `EcologyParameterState`/`ValueTree`
  versionado na thread de GUI. A restauração importa chaves antigas de
  `PropertiesFile`, rejeita snapshots incompletos ou não finitos e só depois
  publica o quadro completo. A thread de áudio aplica `AudioControlFrame::apply`
  somente quando há quadro novo; não lê `PropertiesFile`, `ValueTree` nem
  os caches escalares da GUI. `prepareToPlay` reaplica o quadro corrente ao
  motor recém-preparado, incluindo os controles do Espaço.
- A GUI recebe apenas cópias escalares publicadas depois do render. Ela monta
  um `VisualFrame` próprio a 30 Hz, incluindo posição/alvo/posse da Lupa, e
  não conserva referência ao core.

`PerformanceParameterState` captura os 62 valores persistentes na GUI ao
salvar. A restauração valida o snapshot completo antes de publicar os alvos;
um snapshot incompleto ou não finito é recusado e o app usa os formatos
anteriores. O valor aplicado ao áudio é o valor efetivamente exibido pelo
slider, após sua quantização. Nenhum XML ou `ValueTree` chega ao callback.
Recuperar uma cena atualiza os componentes sem notificações intermediárias e
publica apenas um quadro ao final. Sementes, relógios, notas e memórias DSP não
são reiniciados; permanecem as suavizações internas existentes. Isso não é
uma promessa de morph timbral entre cenas ou de clique inaudível: a calibração
musical de transições ainda requer escuta.

O `IntegratedModuleBank` acrescenta dezesseis floats ao quadro: `LAB MIX` e
quinze famílias. Todos iniciam em zero. Enquanto mestre corrente e alvo forem
zero, o banco retorna antes de processar seus motores; assim cenas antigas não
pagam o novo custo por amostra. Ao ser ativado, os controles usam polo de 20 ms,
memórias fixas e streams de seed próprios. O teste de tempo real automatiza as
quinze famílias durante a contagem de heap e permaneceu com zero alocações.

A coorte emergente de 28/09 acrescenta dez processadores internos a essas
mesmas famílias, sem aumentar o quadro, a ponte ou o número de macros. Filtros,
osciladores, matriz 5×5 e pool usam capacidades fixas; mudanças de coeficiente
acontecem em fronteiras de evento, exceto parâmetros escalares sem alocação.
O benchmark offline ganhou `--module-bank-on`: em 48 kHz/bloco 128/quatro
vozes mediu 13,77% contra 8,75% com bypass; o pior ponto ativo foi 30,66% em
96 kHz/bloco 128/quatro vozes. Isso mede throughput mediano, não deadline,
XRUN ou comportamento do driver.

A coorte avançada do mesmo dia acrescenta dez mecanismos às famílias
existentes e ativa a autocaptura granular, novamente sem ampliar quadro,
ponte, estado ou controles. Todos usam memória fixa; a cópia do wavetable é
incremental e nunca transfere suas 512 amostras de uma vez no callback. A suíte
43/43 e o teste de alocação passaram. No benchmark Release, 48 kHz/bloco
128/quatro vozes mediu 17,70% com o banco ativo; o pior ponto foi 35,78% em
96 kHz/bloco 64/quatro vozes, 2,80× tempo real offline. O bypass mestre retorna
antes das duas coortes. Esses resultados não substituem sessão no dispositivo.

A coorte de geração e interação de 29/09 conserva o mesmo quadro e bypass.
L-system, autômato, limiares, cenas e pools mudam em fronteiras de evento; o
vocoder usa seis pares fixos de biquads; lock-in, contágio, barramento sem
entrada e binaural usam estado fixo. A suíte 44/44 e o teste de alocação
passaram. O pior benchmark ativo foi 41,69% em 96 kHz/bloco 32/quatro vozes,
2,40× tempo real offline. Esse aumento precisa ser reavaliado em escuta e no
dispositivo antes de promover a coorte para fora do laboratório.

A coorte especializada de 29/09 também não amplia o quadro. O contexto do
banco usa nove polos escalares de 45 ms; a amostra final retorna somente como
float limitado no sample seguinte. O Pulsar decide sua máscara apenas no onset
e mantém a decisão durante o pulsaret. O MultiTap possui um único random walk
de capacidade fixa, compartilhado pelos oito offsets; não altera a matriz de
feedback. A suíte 44/44 e a contagem de heap passaram. O benchmark ativo mediu
19,21% em 48 kHz/bloco 128/quatro vozes e máximo observado de 38,92% em
96 kHz/bloco 64/quatro vozes, 2,57× tempo real offline.

O Master criativo de 30/09 acrescenta quatro bytes ao quadro de controle e não
aloca no callback. O crossover, as três dinâmicas, largura e cola usam apenas
estado fixo; parâmetros ecológicos são atualizados no relógio já existente de
200 Hz. `influence=0` retorna antes desses corpos. A suíte integral passou
45/45; com banco e Master ativos, o benchmark mediu 21,57% em
48 kHz/bloco 128/quatro vozes e máximo de 42,95% em
96 kHz/bloco 32/quatro vozes.

## Validação da continuidade sonora

Regressão sonora de 2026-09-17: `LensAudioTests.cpp` verifica a interpolação e
limite por amostra em 44,1/48/96 kHz, chegada antipodal, contagem de foco e
pressão/soltura. O caminho DSP apresenta diferença acima de ruído numérico e
é idêntico bit a bit com blocos de 64/257 amostras para o mesmo calendário de
eventos. CTest final passou em 18/18; o teste de alocações existente também
passou. Benchmark offline de 45 configurações teve menor margem mediana de
2,65 vezes tempo real (96 kHz, quatro vozes externas e banco autônomo), não
uma garantia de latência, prazo de callback ou ausência de xruns.

## Transporte de cenas

`PerformanceSceneFile` opera somente na GUI e limita o arquivo a 16 KiB,
inclusive a leitura após consultar seu tamanho. Importação valida UTF-8, schema
V1 completo, valores normalizados e ausência de DTD/entidades antes de mudar
o slot; não publica controles ao áudio. O usuário precisa recuperar a cena.
`PerformanceSceneControls` converte o estado já validado em quadro escalar,
preservando a Lupa transitória. Na recuperação, a conversão usa valores
efetivamente exibidos/quantizados pelos sliders.

Exportar congela a cena salva, escreve em temporário no diretório escolhido,
verifica escrita/flush, relê o temporário para validar os parâmetros e só
substitui um destino existente após autorização e
backup `.bak` não sobrescrito deliberadamente. Falhas mantêm o backup criado.
O app permite uma transferência por vez, captura o slot e usa `SafePointer`
nos callbacks de seletor/confirmação. O seletor é destruído numa mensagem
posterior, não dentro do seu próprio callback.

`parseParameterXml` protege também as preferências locais: 4/16/64 KiB para
ecologia/estado/banco, número de tags limitado, DTD/entidades customizadas/texto
livre recusados antes de restaurar. Somente o banco local permite os cinco
escapes XML predefinidos (`amp`, `lt`, `gt`, `quot`, `apos`) e referências
decimais a caracteres Unicode imprimíveis, emitidas pelo serializador JUCE.
Referências longas, controles, surrogates, caracteres inválidos, entidades
desconhecidas e DTD continuam recusados. Arquivos `.aquscene` e estados
somente de parâmetros conservam a rejeição de qualquer entidade.
Os dois tipos de estado têm cópia profunda;
copiar uma cena não compartilha propriedades mutáveis com a origem.

Desde a sessão de 2026-09-18, `PerformancePresetBank` V2 reúne quatro cenas
e nomes locais limitados a 32 caracteres sem controles/quebras de linha.
V1 permanece legível e salvo em sua chave antiga sem nomes; V2 tem chave
própria e prioridade. Banco inválido não é substituído no fechamento sem uma
mutação explícita. Restauração valida os quatro slots antes de alterar estado
e limpa o histórico somente quando é bem-sucedida.

O histórico reserva apenas um slot anterior (inclusive vazio), seu nome e
índice; não acumula comandos nem se persiste. Gravação/importação/renomeação
capturam cópias na GUI. Desfazer restaura o banco, não chama setters do core,
não publica quadro e não desfaz notas, memórias DSP ou áudio já produzido.
Somente `RECUPERAR` aplica a cena restaurada. Operações idênticas ou recusadas
conservam o histórico. Diálogos congelam seleção e desabilitam edição de nome,
renomeação e desfazer até terminarem; exportação não muda histórico/banco.
Nomes ficam fora do `.aquscene` e do `AudioControlFrame`.

Não há acesso ao disco, XML, diálogos ou cópia de `ValueTree` na thread de
áudio. A operação de disco é síncrona na GUI, com as tentativas limitadas de
substituição do JUCE; filesystem remoto, escrita concorrente por outro programa
e recuperação após falha física não foram certificados. Não é automação de host.

## Ponte de quadros completos

`FixedLatestSnapshot<Value>` reserva três buffers de tamanho fixo: um do
produtor GUI, um do consumidor áudio e um intermediário. A transferência de
propriedade usa um índice atômico lock-free com marca de atualização e
`exchange` acquire/release. O produtor nunca escreve no buffer que o consumidor
está copiando. Cada publicação/consumo possui trabalho fixo, sem retry loops,
locks, alocação ou espera.

Há exatamente um produtor e um consumidor. `prepareToPlay` e o callback devem
ser serializados pelo ciclo de vida JUCE; não podem consumir simultaneamente.
Quadros intermediários podem ser substituídos antes de serem lidos: controles
contínuos usam o estado mais recente, não uma fila de automação sample-accurate.
Eventos de nota continuam na fila MPSC e não usam esta ponte. Os caches de GUI
ainda são escalares atômicos, mas só a GUI os usa para montar o quadro.

O teste concorrente publica 100 mil quadros com payload verificado e exige
ordem crescente, quadro inteiro e entrega final. O teste de alocação cobre
publicação, consumo, aplicação no motor e render. Uma comparação adicional
mostra igualdade bit a bit entre controles estáveis aplicados uma vez e
reaplicados a cada bloco no cenário testado. Isso não substitui TSan nem
deadline/xruns com dispositivo real.

## Eventos discretos

`FixedMpscQueue<NoteEvent, 256>` recebe simultaneamente eventos da GUI e de uma
ou mais threads MIDI. A fila:

- usa capacidade e memória fixas;
- não usa mutex, espera, arquivo ou alocação;
- preserva a ordem global das reservas e a ordem de cada produtor;
- possui muitos produtores e exatamente um consumidor, a thread de áudio;
- consome no máximo 64 eventos por callback, evitando que uma tempestade MIDI
  monopolize o bloco;
- descarta o evento novo quando cheia e incrementa um contador atômico. A GUI
  mostra `EVENTS OK` ou `DROPPED N`, portanto a perda nunca é silenciosa.

## Estado do dispositivo

O texto `AUDIO ONLINE` deriva de um contador incrementado pelo callback real.
Se a GUI não observar avanço por 15 ticks do timer, aproximadamente 500 ms, o
estado muda para `AUDIO WAITING`. O último quadro ecológico permanece visível,
mas não é apresentado como atividade sonora atual.

## Restrições do callback

No callback são proibidos:

- alocação e liberação dinâmica;
- locks, espera ativa, arquivo, rede ou mensagens de GUI;
- leitura direta de `ValueTree` ou componentes JUCE;
- acesso concorrente não atômico ao estado da interface;
- trabalho de eventos sem limite por bloco.

## Política numérica

- sample rate não finito usa 48 kHz; valores finitos são limitados ao intervalo
  suportado de 8–768 kHz antes de qualquer conversão para contador inteiro;
- parâmetros persistentes `NaN`/infinitos são rejeitados e preservam o último
  valor válido, inclusive bit a bit quando isso evita deriva de arredondamento;
- velocity MIDI não finita descarta a nota antes de ocupar uma voz;
- energia de saída não finita falha para zero; alvos inválidos de Lupa e stress
  são ignorados;
- limites finitos continuam sendo aplicados normalmente, por exemplo controles
  normalizados em `[0, 1]` e nota MIDI em `[0, 127]` antes do cálculo de pitch.

Essa defesa na entrada é complementada pelo saneamento final de `NaN`/`Inf`
em `OutputStage`; ambas permanecem camadas independentes.

## Calibração de saída

`BiomaEngine::outputCalibrationGain` aplica `×5` (+13,98 dB) somente depois da
mistura. `lastOutputEnergy` e a telemetria `outputEnergy` permanecem antes desse
ganho porque representam a grandeza usada pela homeostase, não um medidor em
dBFS. Assim uma correção de escuta não muda o percurso generativo.

Uma única memória `MultiTapSpace` é reservada quando `BiomaEngine` é construído,
antes de `prepare()` e do dispositivo iniciar. O callback apenas atualiza oito
alvos a 200 Hz e executa o estado fixo por amostra. O retorno wet é suavizado em
80 ms. `lastOutputEnergy` permanece pré-espaço neste marco: ligar reflexões não
cria uma realimentação ecológica implícita.

Uma única memória `AquariumTail` também é reservada na construção. Ela recebe
o sinal pós-MultiTap e avança em todos os samples, inclusive em bypass. Os
quatro alvos de performance têm rampa de 15 ms e seus coeficientes são
recalculados no mesmo relógio de 200 Hz; `size` permanece fixo para não trocar
comprimentos de delay durante uma cauda. `TAIL MIX=0` não soma o retorno e
preserva a rota anterior bit a bit.

Desde 2026-09-27, uma instância fixa de `OutputStage` encerra a rota depois da
Cauda e do ganho de calibração. Ela saneia não finitos, bloqueia DC em 10 Hz,
suaviza trim/mute em 15 ms e usa limiter estéreo vinculado com 5 ms de
lookahead. O teto inicial é -1 dB, o release inicial é 150 ms e a latência é
240 amostras em 48 kHz. Nenhuma dessas variáveis recebe modulação ecológica.
O detector intersample Catmull-Rom em quatro fases é uma estimativa de proteção,
não conformidade true peak BS.1770; por isso fixtures externas, oversampling
polifásico e loudness continuam abertos. O contrato completo está em
`OUTPUT_STAGE_DESIGN.md`.

O ganho por espécie é limitado a `[0, 2]`, rejeita valores não finitos e atua
antes da mistura espacial. Zerar uma espécie silencia sua contribuição, mas seu
DSP continua avançando; isso permite stems comparáveis sem reiniciar fases ou
caudas. Mudanças devem obedecer à mesma propriedade de thread dos outros setters.

O piso de audibilidade dos corpos eventuais é sempre multiplicado por
`AutonomousEnergy::enableAmount()`. Assim ele preserva caudas quando apenas o
organismo adormece, mas não contorna o mute/ramp global da autonomia.

## Evidência automatizada atual

- `aquorbium_realtime_tests`: render, morph e fila operam com zero alocações
  durante a janela monitorada, inclusive com o banco autônomo e vozes externas
  ativos simultaneamente. Desde 28/09, a janela também automatiza as quinze
  famílias do `IntegratedModuleBank` com `LAB MIX` aberto.
- `aquorbium_emergent_module_cohort_tests`: dez componentes emergentes,
  incluindo invariantes numéricas, determinismo, topologias, bypass, matriz,
  pool e saídas X/Y.
- `aquorbium_autonomous_energy_tests`: nascimento sem nota, silêncio desligado,
  determinismo entre blocos, seed, segurança numérica e rampas exatas.
- `aquorbium_modal_resonator_tests`: geometria modal, polos, cauda, seed,
  impactos sobrepostos, parâmetros inválidos e autonomia/performance híbridas.
- `aquorbium_granular_striker_tests`: pool fixo, duração, densidade, seed,
  saturação explícita, extremos numéricos, mapeamento dos macros e prova de que
  intenção nova não modifica partículas já nascidas.
- `aquorbium_physical_string_tests`: linha fixa, afinação fracionária em 48/96
  kHz, estabilidade do laço, seed, cauda, impactos extremos e prova de que uma
  intenção nova não retuna energia já circulando.
- `aquorbium_organism_voice_tests`: despacho espécie→DSP, determinismo,
  corpos Morph/Pulsar/Granular/Modal e fallbacks técnicos.
- `aquorbium_space_tests`: transformação rígida e inversa, desacoplamento
  acústico da pose, potência constante, profundidade, suavização e rejeição de
  parâmetros inválidos.
- `aquorbium_multitap_space_tests`: oito leituras fracionárias, pan/filtro por
  tap, limite recursivo, macros híbridas, reset determinístico, extremos e zero
  alocações durante processamento e automação.
- `aquorbium_mpsc_queue_tests`: capacidade, overflow explícito, wrap-around e
  quatro produtores concorrentes com 80.000 eventos.
- `aquorbium_snapshot_tests`: quadro inteiro, coalescência, ordem crescente e
  entrega final com 100 mil publicações concorrentes. A rodada de 2026-09-16
  aprovou 100 repetições e o executável separado com UBSan. Na build Linux,
  `AudioControlFrame` ocupa agora 264 bytes e sua ponte fixa ocupa 1024 bytes após
  Ambiente, Cauda, memória formal Markov, saída técnica, estrutura de
  memória/harmonia, o banco integrado de quinze famílias e o macro do Master
  criativo.
- `aquorbium_ecology_parameter_tests`: snapshots completos, quatro cenas,
  cópias independentes, restauração transacional, limites de XML e prioridade
  da migração em `PropertySet` em memória. Inclui transporte em diretório
  temporário próprio: roundtrip, backups distintos, recusa de sobrescrita não
  autorizada, tamanho exato de 16 KiB, UTF-8/bytes nulos, links simbólicos no
  Linux/macOS, destino inválido e estado preservado após erro. Não abre GUI.
- `aquorbium_scene_playback_tests`: XML → banco → adaptador → ponte → DSP,
  com 62 valores, migração dos formatos de 25/31/35/39/42/46, Lupa transitória, notas e
  trocas repetidas de cena. Matriz de
  44,1/48/96 kHz e blocos de 64/257/512 amostras; compara os dois canais bit a
  bit com cenas originais e verifica amostras finitas/energia não nula. Isso
  comprova o percurso técnico, não inaudibilidade das transições.
- `aquorbium_output_stage_tests`: latência/lookahead, silêncio inicial, limiter
  estéreo vinculado, teto, telemetria, DC, não finitos, faixas e mute suave.
  `aquorbium_realtime_tests` confirma também a conversão dos três controles e
  zero alocação com o estágio integrado.
- `aquorbium_numeric_safety_tests`: `NaN`, infinitos, sample rates inválidos,
  retenção do último estado válido e render prolongado sem contaminação.
- O teste de alocação exercita o Physical String isolado e também sua integração
  real: um corpo autônomo mais quatro corpos das vozes externas, inclusive
  impactos e snapshots performáticos durante a janela monitorada.
- O mesmo teste automatiza retorno, tamanho, abertura, tom e memória do
  `MultiTapSpace`; a alocação única da memória ocorre antes da janela monitorada.
- O mesmo teste automatiza a Cauda integrada com mix, decay, damping e
  nightmare ativos; sua memória também é alocada antes da janela monitorada.
- O mesmo teste automatiza a memória formal com influência, contexto, inércia
  e mutação. `BiomaMarkov` conserva estado e RNG fixos, avança apenas no
  relógio de controle e não aloca no callback.
- A janela monitorada agora também alterna posse/alvo da Lupa e varia
  Cohesion, Danger e os quatro pesos de escuta durante o render; a contagem de
  alocações permaneceu zero na suíte de 2026-09-15.
- O teste de integração do `BiomaEngine` cobre posse, movimento e liberação da
  Lupa, alteração dos ganhos da mistura por pesos de perspectiva e os alvos
  Cohesion/Danger. A compilação do app confirma o adaptador JUCE; ainda falta
  ensaio visual e sonoro com dispositivo real.
- A suíte anterior recebeu validação com AddressSanitizer/UBSan. A nova ponte
  passou em UBSan nesta rodada. O teste separado com ASan/UBSan compilou, mas
  encerrou no LeakSanitizer com erro de incompatibilidade com `ptrace` neste
  ambiente; esse teste não é registrado como aprovado.
- ThreadSanitizer foi tentado, mas o runtime disponível encerra antes do teste
  com `unexpected memory mapping`. A validação TSan permanece pendente em um
  ambiente compatível; isso não deve ser registrado como teste aprovado.

## Próximas ampliações

- Integrar o snapshot persistente completo ao futuro APVTS; a ponte coerente
  de cenas locais já existe, mas não atende automação sample-accurate de host.
- Definir política de timestamps/sample offsets para eventos MIDI; hoje eles
  entram no início do próximo bloco disponível.
- Medir deadline e xruns com dispositivo real. A matriz offline de 30/09 já
  inclui `MultiTapSpace`, banco integrado, Master criativo e saída técnica:
  21,57% em 48 kHz/bloco 128/quatro vozes e máximo de 42,95% em
  96 kHz/bloco 32/quatro vozes. Isso não substitui prazo do dispositivo.
