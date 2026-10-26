# Aquorbium — contrato do novo Ecossistema

Atualização: 2026-09-28.

**Estado:** primeiro marco integrado ao core
**Implementação canônica:** `src/core/Ecosystem.h` e
`src/core/Ecosystem.cpp`
**Protótipo anterior:** removido após auditoria; diagnóstico preservado em
`ECOSYSTEM_AUDIT.md`

## 1. Função

O Ecossistema é a física de controle do Aquorbium. Ele não sintetiza todas as
fontes, não é a GUI e não substitui o master. Sua responsabilidade é calcular,
de maneira determinística, como organismos percebem, influenciam e expõem uns
aos outros.

O `BiomaEngine` continua responsável pelas vozes audíveis. O Ecossistema
publica estados limitados de mixagem e detalhe para essas vozes. Essa separação
permite testar a coerência ecológica sem dispositivo de áudio e impede que a
metáfora se torne uma classe monolítica.

`OrganismVoiceEngine` é a camada de integração entre espécie e corpo DSP. Ela
escolhe Morph, Granular, Pulsar ou Modal pelo valor de `Species`, nunca pelo índice do
organismo. Physical String usa a mesma identidade tipada, mas seu grande estado
de delay pertence a um banco dedicado no `BiomaEngine`. Não usa herança virtual
e não devolve síntese ao `OrganismState`.
Contrato e evidência: [`ORGANISM_VOICE_ENGINE_DESIGN.md`](ORGANISM_VOICE_ENGINE_DESIGN.md).

`AutonomousEnergy` consome snapshots limitados dessa física e produz alvos de
energia e altura em um relógio próprio de 100 Hz. O banco audível autônomo fica
separado das quatro vozes externas: MIDI pode dialogar com o Bioma, mas não é a
condição para ele nascer nem toma o lugar de um organismo interno.

## 2. Invariantes conceituais

1. Nenhuma entidade possui visão total.
2. Uma reação só pode usar aquilo que a entidade conseguiu observar.
3. Desejo altera tendências, não escreve acontecimentos diretamente.
4. Foco local reduz a definição periférica.
5. Organismos da mesma espécie/cardume preservam diferenças individuais.
6. Primeiro plano e sustentação são papéis explícitos.
7. Integridade responde a causas identificáveis; funcionamento normal não
   quebra inevitavelmente o aquário.
8. O Bioma pode influenciar o master criativo, nunca remover a proteção final.

## 3. Invariantes técnicos

- C++20 portátil no núcleo; JUCE adapta aplicação, buffers e parâmetros.
- Uma seed e os mesmos eventos produzem o mesmo estado e o mesmo áudio.
- O relógio ecológico usa contagem absoluta de amostras a 200 Hz; fronteiras
  de callback não alteram o resultado.
- Não há alocação, lock, arquivo, rede ou dispositivo em `tickSample()`.
- Posições permanecem dentro da esfera e observadores permanecem fora dela.
- Posições deformadas por fissura são estado cacheado, não novo ruído por
  consulta.
- Pesos são finitos, não negativos e compensados para um teto de soma antes do
  master criativo.
- Mudanças de desejo e blend recebem suavização temporal.
- O `BiomaEngine` possui os modos `Active` e `Neutral`: o segundo conserva o
  relógio ecológico e a seed, mas renderiza a rota-base para comparação A/B.
- Os dois estados são identidades expressivas sem hierarquia de qualidade. O
  controle `Ecosystem Morph` percorre continuamente o intervalo `[0, 1]`, com
  chegada exata ao alvo em até 80 ms. Forma, densidade do pulsar e matriz
  estéreo mudam juntas; não há troca discreta nem reset da simulação.
- A interpolação dos dois buses é linear, pois as rotas compartilham a mesma
  fonte e são fortemente correlacionadas. Um crossfade equal-power criaria um
  ressalto artificial de aproximadamente 3 dB no centro.
- `BiomaEngine::Telemetry` copia estados fixos e continua não sendo, por si só,
  uma API de GUI entre threads. O app a consulta somente na thread de áudio e
  publica seus campos escalares por uma ponte atômica; a GUI monta um snapshot
  independente a 30 Hz. O contrato está em `REALTIME_CONTRACT.md`.

Os setters do core devem ser chamados pela mesma thread do render ou com o
áudio parado. O app JUCE publica um `AudioControlFrame` completo, com controles
e gesto da Lupa, por `FixedLatestSnapshot` lock-free. O callback consome no
máximo um quadro novo no início do bloco; controles visuais não tocam o core.
Presets locais usam essa ponte. APVTS e automação de host permanecem futuros.

## 4. Primeiro conjunto de organismos

O marco atual mantém cinco organismos e cinco arquétipos audíveis. Cada
instância carrega sua espécie, que seleciona o corpo sonoro; os índices abaixo
são posições da formação inicial, não uma API de roteamento DSP.

| Índice | Espécie conceitual | Papel | Estratégias iniciais |
|---|---|---|---|
| 0 | Oscilador | primeiro plano | predador + territorial |
| 1 | Striker granular | sustentação/acento | territorial |
| 2 | Ressonador modal | sustentação | migratório |
| 3 | Pulsar | primeiro plano/acento | presa |
| 4 | Physical String | sustentação/memória | séssil + territorial |

O índice 2 usa `BiomaModalResonator`: quatro modos amortecidos excitados por
eventos de vida, com energia e cauda próprias. A física ecológica determina
impacto, material, brilho e decaimento; `PerformanceIntent` pode deslocar esses
alvos no próximo impacto sem mover polos durante uma cauda.

O índice 1 usa `BiomaGranularStriker`: eventos de vida geram nuvens curtas de
partículas, cujos parâmetros derivam dos traços individuais e do Desejo. O
índice descreve apenas a formação inicial; a seleção continua sendo feita por
`Species::GranularStriker`.

## 5. Percepção e movimento

Organismos possuem posição, velocidade, direção do olhar, campo perceptivo e
turbidez. Predador, presa e território consultam essa percepção. O movimento
combina:

- deriva contínua com eventos e pausas;
- temperatura e densidade desejada;
- direção compartilhada do cardume;
- coesão individual e pulso de acasalamento;
- até duas estratégias combináveis;
- contenção/reflexão na superfície interna da esfera.

Os observadores se deslocam tangencialmente numa esfera externa de raio fixo e
continuam olhando para o centro. Assim, nunca entram acidentalmente no aquário.

### Referenciais do aquário móvel

`AquariumPose` agora carrega posição, orientação por quaternion e transformação
inversa, sem somar deslocamentos às coordenadas individuais:

- organismos, água e Lupa vivem no referencial local do aquário;
- câmera e observadores externos vivem no referencial do mundo;
- posições e direções externas podem entrar no aquário pela transformação inversa;
- a Lupa acompanha o vidro, enquanto observadores não são arrastados;
- pose externa é inaudível por padrão e só entra na percepção quando o vínculo
  acústico é ativado explicitamente;
- inércia do líquido pode modular movimento e áudio por uma camada limitada,
  sem deslocar organismos para fora da esfera;
- câmera e movimento meramente visual não entram no core.

Esse desenho evita deriva numérica e permite testar separadamente pose,
percepção, movimento visual e consequência sonora. Panorâmica de potência
constante e profundidade espectral já estão ativas e suavizadas por amostra;
inércia e Doppler continuam fora até teste auditivo. O contrato detalhado está
em `AQUARIUM_SPACE_DESIGN.md`.

## 6. Quatro modos de escuta

Os nomes são comportamentais e autorais. As referências artísticas que
estimularam a pesquisa permanecem na documentação, não no nome das classes.

- `SparseFocus`: alterna um subconjunto nítido contra um fundo reduzido;
- `EnergyMemory`: relaciona energia do organismo à memória suavizada da saída;
- `GeometricCycle`: circula a proeminência por uma regra geométrica;
- `CollectiveDrift`: mantém o conjunto presente com flutuação lenta.

Cada observador combina visibilidade geométrica, oclusão, modo de escuta e
Lupa. O resultado passa por panorâmica de potência constante e compensação de
densidade. O Ecossistema entrega ganhos L/R por organismo, não áudio final.

Uma sessão cega reproduzível compara um observador em peso 1 contra os quatro
pesos iguais: `aquorbium_ab_render <diretório-novo> --observer=0..3`.
Os takes usam a mesma seed, eventos e rota `Active`, com feedback desligado;
o RMS é igualado antes da escuta. `--reveal` apenas cria a chave após a ficha
ser preenchida, sem regenerar áudio. As sessões das quatro perspectivas foram
geradas em 2026-09-15 sob `validation-output/2026-09-15-perspective-*-ab/`;
as quatro referências de pesos iguais têm SHA-256 idêntico, enquanto os
quatro focos são diferentes. Nenhuma chave dessas sessões foi revelada.
Não há conclusão musical sem escuta humana.

O standalone expõe os pesos dos quatro observadores como quatro sliders na
página `ECO CONTROLS`. O core normaliza os pesos e suaviza a mudança no
relógio ecológico; quatro zeros recuperam pesos iguais. `COHESION` e `DANGER`
também estão nessa página. `DENSITY` e `DRIFT` permanecem na faixa inferior,
com `DRIFT = 1 - calm`. A camada `EcologyParameterState` baseada em `ValueTree`
versionado concentra os oito valores ecológicos na thread da GUI, importa as
chaves antigas e salva junto às preferências. O callback de áudio recebe
apenas quadros escalares coerentes; APVTS e automação de host continuam pendentes.
Desde 2026-09-16, essa camada ecológica é um filho do snapshot completo
`PerformanceParameterState` V1, agora com 62 valores persistentes do standalone,
incluindo os quatro controles de memória endereçável e harmonia da Geometria.
A validação completa antecede a restauração; os formatos antigos permanecem
como fallback. Gate e posse da Lupa não fazem parte da persistência.
Quatro cenas locais já podem guardar/recuperar esses valores. A GUI publica um
`AudioControlFrame` completo por ponte de três buffers fixos; o motor recebe
um estado coerente no limite do bloco, sem ler XML/ValueTree. As cenas não
reiniciam a ecologia nem os motores e não fazem morph comportamental.
Cada cena também pode ser transportada em `.aquscene`, com limite de 16 KiB,
validação transacional e backup antes de substituir um arquivo autorizado.
Importar guarda no slot; só recuperar publica ao áudio. Nenhum áudio, estado
interno de organismo ou gesto transitório é serializado. O teste ponta a ponta
compara bit a bit o render após XML/banco/ponte, sem substituir a escuta humana.

## 7. Lupa

A Lupa fica na superfície do vidro e só pode pertencer a um observador por vez.
Seu alvo é projetado na esfera. No foco, ela aumenta `detail` e apenas uma
fração limitada do ganho; fora do foco, reduz a definição daquele observador.

Neste primeiro marco, `detail` altera discretamente matéria do oscilador e
densidade do pulsar. Filtro/brilho espectral específico dependerá dos futuros
motores de voz e será validado por escuta antes de entrar.

O gesto de performance atual é clicar e arrastar dentro da esfera da GUI.
Enquanto pressionado, o observador 0 possui a Lupa; soltar a libera. `Alt`
permite mirar a meia esfera posterior. O ponto clicado vira alvo, não teleporte:
o anel acompanha a posição ecológica real e o marcador pequeno mostra o alvo.
Esse é o primeiro contrato de controle humano; automação da posse pelo Cérebro
e calibração de raio, detalhe periférico e ganho por escuta seguem em aberto.

Correção de 2026-09-17: movimento geodésico limitado a 0,55 rad/s, com chegada
sem ultrapassagem e tangente determinística para um alvo antipodal. O foco
angular usa `smoothstep`: passa continuamente do detalhe periférico (0,35)
à ampliação (2,5), sem o salto anterior de 1 para 0,35 na borda. A elegibilidade
em profundidade continua limitada a 0,75 unidade do vidro; sua fronteira
não foi redesenhada. O efeito permanece local ao observador que possui a Lupa.

`mix()` conserva o alvo ecológico de 200 Hz para diagnóstico e visualização;
`audioMix()` interpola esquerda, direita, observação e detalhe a cada amostra,
por um polo com constante de 15 ms. O áudio usa essa segunda mistura, incluindo
os parâmetros de excitação do ressonador e da corda. Reset inicializa ambas
com o mesmo estado: não introduz fade de partida. A suavização não muda o
relógio físico nem consome números aleatórios, mas o áudio e, quando ligado,
seu feedback passam a ter uma nova linha de base.

O rodapé explicita posse ativa/livre e quantidade de organismos presentes no
foco geométrico; a contagem não pondera o ganho da perspectiva. O anel parado
após soltar não significa foco mantido. Pares locais de escuta contrastam
dois alvos móveis (organismo 0 e direção espelhada), tanto com pesos
iguais como com observador 0 isolado; diferença matemática e estabilidade
de controles não substituem avaliação perceptiva nem diagnóstico de xruns.
Um diagnóstico anterior com alvos fixos +X/-X não atingiu organismos de 2 a
12 s e produziu WAVs idênticos. Isso é evidência de um gesto sem foco naquele
caso, não prova de uma Lupa desligada. O alcance não foi ampliado para forçar
diferença e o renderer não implementa acompanhamento automático no app.

## 8. Integridade

A integridade possui histerese entre estado saudável e trincado. Há dois tipos
de causa:

- estresse explícito, vindo de um evento ecológico documentado;
- estresse sustentado somente quando perigo e energia já estão ambos em faixa
  extrema.

Calma e ausência de perigo permitem recuperação. `shattered` é irreversível
durante a sessão. O primeiro marco testa a máquina de estado, mas ainda não
atribui som à quebra. Side-chain externo, no-input mixing e power starvation
continuam hipóteses de pesquisa e não devem ser anunciados como funções.

## 8.1 Movimento compartilhado experimental

`HarmonicPhaseBank` e `FluidMotion` foram codados isoladamente em 24/09. O
primeiro deriva até oito fases por relações racionais de um único relógio. O
segundo produz quatro leituras do mesmo estado — forma, amplitude, tempo ou
frequência — e aceita autopatch limitado sobre a velocidade.

Esse protótipo não altera `Ecosystem::motionStep`, `organismMotion` nem o
áudio. Antes de integrar, é necessário renderizar trajetórias, verificar se o
cardume ganha coesão sem uníssono mecânico e decidir como espécie, estratégia
e indivíduo recebem offsets. Contrato em `SCHOOL_MOTION_DESIGN.md`; build
Release e suíte 35/35 passaram.

## 8.2 Formas e memória de gesto experimentais

`MotionWaveBank` centraliza oito formas limitadas e permite escolher uma delas
de modo determinístico a partir de espécie, estratégia e seed. `FluidMotion`
passou a chamar esse banco em vez de manter fórmulas locais duplicadas.

`MotionGestureRecorder` registra até 2.048 pontos XY e reproduz o trajeto com
interpolação e velocidade variável. Mutação zero preserva o loop exatamente;
uma mutação opcional, limitada e sem alocação pode alterar levemente cada nova
volta. O gravador ainda não recebe gesto da Lupa ou de controles da GUI.

O lote não altera `Ecosystem::motionStep`, `organismMotion`, callback, áudio ou
parâmetros. Antes de integrar, requer render de trajetória, escuta musical e
novo teste interativo de estabilidade. Contrato em
`GESTURE_MOTION_DESIGN.md`; build Release e suíte 36/36 passaram.

## 8.3 Assinatura espectral e ambiente integrados

`SchoolSpectralBank` reúne oito passa-bandas ressonantes. Centro e dispersão
ocupam um domínio logarítmico, enquanto `schoolId` e seed criam um perfil fixo
de ganhos normalizados. A primeira rota integrada processa a soma estéreo dos
organismos antes do Espaço; diferenciação por `SchoolInfo` permanece aberta.

`EcologicalNoiseBed` gera dois canais determinísticos. Estação e hora percorrem
quatro texturas; turbidez e temperatura definem a cor; trinca reduz a resolução
temporal por sample-and-hold. A integração lê `Environment`, a turbidez média
dos organismos e `AquariumIntegrity`, e soma o leito antes do Espaço.

O cartão `05 AMBIENTE` expõe `ENV MIX`, `FILTER`, `CENTER`, `SPAN`,
`RESONANCE` e `AMBIENCE`. Influência zero é bypass transparente e desativa
visualmente os cinco controles dependentes. Integração, persistência, resposta,
layout, tempo real e render offline passaram na validação automatizada; escuta
humana continua pendente. Contrato em `SPECTRAL_ENVIRONMENT_DESIGN.md`.

## 8.4 Cauda sintética integrada

`AquariumTail` é uma rede FDN estéreo de quatro linhas, compartilhada depois
das primeiras reflexões. O cartão `06 CAUDA` expõe `TAIL MIX`, `DECAY`,
`DAMPING` e `NIGHTMARE`; influência zero é bypass bit a bit e desativa os três
controles dependentes. Tamanho e difusão permanecem fixos porque a mudança de
comprimento ainda não é segura durante uma cauda. Integração, cenas, resposta,
tempo real, render e layout passaram tecnicamente; escuta e eventual vínculo
com o Ecossistema permanecem abertos. Contrato em `SYNTHETIC_TAIL_DESIGN.md`.

## 8.5 Memória formal Markov integrada

Uma `BiomaMarkov` compartilhada ocupa o domínio forma dentro de
`AutonomousEnergy`. Ela avança uma vez por cruzamento coletivo positivo do
campo Relabi. O alvo contextual combina luz, temperatura, pulso de
acasalamento e perigo; os oito estados produzem deslocamentos limitados da
seção enviada ao `GeometricSequencer`.

`FORM MIX=0` zera exatamente essa contribuição, sem congelar a cadeia nem
consumir o RNG das decisões autônomas existentes. `CONTEXT`, `INERTIA` e
`MUTATION` não alteram áudio enquanto o mix permanece em zero. Estado,
transições e alvo contextual entram somente na telemetria; a cadeia não muda
presença, estratégia, ambiente ou integridade. Contrato em
`BRAIN_DECISION_DESIGN.md`.

## 8.6 Mistura de filosofias experimental

Quatro componentes isolados definem a matemática antes da semântica.
`PhilosophyVectorMixer` distribui pesos por posição XY; `PhilosophySequencer`
percorre até 16 posições por pulsos; `PhilosophyMorph` cruza saídas discretas
durante uma janela; `PhilosophyMatrixMixer` roteia quatro fontes para quatro
saídas, organizadas como dois barramentos estéreo, com modulação por célula.

Nenhuma filosofia concreta ocupa ainda os cantos e o relógio não vem do
Cérebro. A matriz não substitui pesos de observador, pan, cenas ou parâmetros
do app. O lote deve passar por escuta e teste interativo antes de qualquer
ligação. Contrato em `PHILOSOPHY_MIX_DESIGN.md`; build Release e suíte 38/38
passaram.

## 8.7 Saída técnica integrada

`OutputStage` vive depois de toda a rota criativa e nunca devolve seus
controles ao Ecossistema. A homeostase continua medindo a soma interna, antes
da calibração e da redução de ganho. Assim, proteção final e comportamento
ecológico não formam um loop oculto.

O estágio saneia não finitos, bloqueia DC, suaviza trim/mute e limita o par
estéreo com 5 ms de lookahead. A telemetria de redução pode ser observada, mas
não é ainda fonte de modulação. A estimativa intersample 4× não reivindica
conformidade true peak. Contrato: `OUTPUT_STAGE_DESIGN.md`.

## 9. Entrada de novos módulos

Uma referência de módulo não gera uma classe automaticamente. Para entrar, uma
ideia precisa declarar:

1. pergunta e consequência musical;
2. papel como fonte, transformador, modulação ou observação;
3. relação ecológica que não existe ainda;
4. modelo matemático e limites;
5. seed, estado e política de reset;
6. orçamento de CPU, memória e ganho;
7. teste técnico, A/B e estudo de escuta;
8. destino: estável, experimental ou arquivado.

Código externo não será copiado. Código dos instrumentos RASGO pode ser
reapropriado com autoria, origem, diferenças e testes documentados.

## 10. Próximos portões

1. O renderizador `aquorbium_ab_render` já produz uma sessão A/B cega: `A.wav`
   e `B.wav` RMS-equalizados na raiz, protocolo de escuta e telemetria técnica
   isolada em `analysis/`. A chave só é criada por `--reveal` após o registro
   da resposta. Os takes usam seed e eventos idênticos e a comparação
   controlada desliga o feedback de energia nos dois lados; uma futura sessão
   separada deverá avaliar o feedback como relação musical própria.
2. Fazer sessões humanas de escuta A/B, registrar decisões e calibrar primeiro
   plano, sustentação, Lupa e legibilidade.
   A primeira escuta controlada identificou maior profundidade na rota
   `Neutral`, mas confirmou que as rotas `Active` e `Neutral` são duas
   identidades sonoras válidas, sem hierarquia de qualidade. Tratar o
   Ecossistema como modo expressivo selecionável e calibrar a transição entre
   os estados, não como benefício universal a provar.
3. A fundação autônoma já possui benchmark com zero/uma/quatro vozes externas;
   repetir a medição quando os motores definitivos forem integrados. A ausência
   de alocação já possui teste automatizado.
4. O app JUCE já usa canais atômicos lock-free para o gesto da Lupa e os seis
   controles ecológicos, além de telemetria visual. Validar o layout e a
   interação em janela real. O snapshot persistente completo já existe;
   quatro cenas locais já usam publicação coerente. Integrar APVTS somente no
   adaptador de plugin futuro e validar por escuta as transições de cena.
5. O primeiro ressonador já está integrado. Antes de outro organismo ou efeito
   ecológico, ouvir e calibrar o conjunto de cinco organismos e o Espaço.
6. Integrar presença como gesto de performance e encontros predador/presa
   como eventos sonoros somente após uma decisão musical documentada.
8. Implementar consequência sonora da integridade por último.
