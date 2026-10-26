# Aquorbium — memória endereçável e progressão harmônica

Atualizado em 2026-09-27. Estado: família integrada ao `AutonomousEnergy`,
operável no cartão Geometria e validada tecnicamente. A aprovação musical por
escuta continua pendente.

## Decisão de integração de 2026-09-27

A promoção foi feita no `AutonomousEnergy`, na fronteira de eventos da
Geometria. `AddressableStepMemory` acrescenta memória de altura e gate aos
eventos já existentes; `HarmonicProgression` interpreta uma fase ecológica
explícita e desloca o campo harmônico. O controle mestre recupera o caminho
anterior de forma bit a bit, permitindo comparação e retirada segura.

A interface não abre dezenas de passos editáveis nesta etapa. Quatro macros
grandes foram acrescentadas à superfície rolável da Geometria: `STRUCTURE MIX`,
`STEP MEMORY`, `HARMONY` e `PHASE PATH`. Os padrões iniciais são determinísticos
e a edição detalhada da memória continua uma etapa posterior. Com `STRUCTURE
MIX=0`, os três controles dependentes ficam inativos e cenas antigas migram com
influência zero.

## Implementação ao vivo

O motor carrega no reset um padrão autoral determinístico de oito passos. Em
cada fronteira de evento já existente, o endereço combina fase ecológica, fase
orbital e deslocamento do organismo. O passo selecionado pode deslocar altura e
probabilidade de despertar; a seção harmônica oferece graus de acorde ao mesmo
evento, sem criar um segundo relógio ou uma voz paralela.

`PHASE PATH` interpola explicitamente entre `dayPhase` e `seasonPhase`. A fase
resultante seleciona a seção ponderada da progressão. `STEP MEMORY` e `HARMONY`
dosam as duas consequências musicais, enquanto `STRUCTURE MIX` é o gate mestre.
Os componentes continuam avançando deterministicamente em bypass para que a
reativação revele o estado presente, mas não alteram a saída nem perturbam os
streams aleatórios anteriores quando o gate está em zero.

Os quatro valores atravessam `AudioControlFrame`, snapshots, preferências e as
quatro cenas locais. O snapshot V1 passa a conter 46 valores. Formatos de 25,
31, 35, 39 e 42 valores continuam aceitos; cenas de 42 valores recebem os
defaults neutros desta família sem reordenar os campos existentes.

## Memória de passos

`AddressableStepMemory` mantém até 32 passos em armazenamento fixo. Cada passo
carrega quatro valores com função explícita: deslocamento de altura em
semitons, duração de gate, viés de estratégia e quantidade de desejo. Existem
duas leituras independentes:

- cursor sequencial, com indicação de volta ao início;
- endereço direto normalizado, em que `0` seleciona o primeiro passo e `1` o
  último.

A leitura direta não move o cursor sequencial. Isso permite que um sinal do
Bioma consulte uma posição sem destruir a continuidade rítmica. Valores não
finitos recebem padrões definidos; altura, duração, estratégia e desejo são
limitados antes de entrar na memória.

Persistência, edição pela interface, ligação ao `AccessPort`, trilhas
paralelas/seriais e endereçamento por limiar ou árvore permanecem etapas
separadas.

## Progressão harmônica

`HarmonicProgression` mantém até 32 seções de peso variável. Uma seção define
grau fundamental, qualidade de acorde, inversão e duração relativa. O motor
reutiliza o `IntervalQuantizer` para localizar graus positivos e negativos e
produz acordes de três ou quatro vozes com nove qualidades.

A entrada é uma fase normalizada e cíclica. Os pesos dividem o ciclo em
seções, e a saída informa quando a seleção mudou. O módulo não mistura por
conta própria `dayPhase` e `seasonPhase`: essa política deve ser decidida no
adaptador ecológico, para que uma escolha musical não fique escondida no
núcleo matemático.

## Contrato e validação

Os dois componentes são C++20 portável, têm memória limitada, operações
`noexcept` no caminho de eventos e não fazem alocação, bloqueio ou I/O. O teste
`SequencerMemoryTests` cobre cursor, volta, CV direta, quatro controles,
sanitização, pesos, fronteiras, graus negativos, qualidades, inversões e escala
de período configurável. `RealtimeSafetyTests` executa ambos com contagem de
heap ativa.

Em 24/09/2026, o protótipo isolado passou no build Release e na suíte 26/26.
Depois da integração de 27/09, a suíte completa passou 40/40 em 80,53 s. O
teste de resposta dos controles passou em 59,53 s; o teste de tempo real mediu
`AudioControlFrame` com 196 bytes e ponte fixa com 832 bytes, sem alocação no
caminho exercitado. O bypass em zero é bit a bit transparente e o modo ativo
produz diferença limitada e mensurável.

A auditoria final de layout gerou 40 PNGs, quatro resoluções e dez vistas por
resolução em
`validation-output/2026-09-27-structure-memory-layout-v3/`. As quatro linhas da
Geometria em 960×600 foram inspecionadas sem sobreposição; knobs e áreas de
interação mantêm os mínimos de 52 px e 80×92 px por rolagem. O executável
canônico apontado por `./run` tem 14.040.848 bytes e SHA-256
`8d4c32c83aa24731f8b2f7f4dfeb76819d4a16124eeaeeb17e07062abb534766`.

Esses resultados aprovam integração, persistência, layout e segurança técnica;
não aprovam musicalmente o módulo nem provam estabilidade prolongada do
dispositivo de áudio. A próxima etapa é escutar separadamente variação de
ritmo, melodia, harmonia, eventos e função composicional. Depois vêm edição
detalhada dos passos, ligação ao `AccessPort`, bancos de padrões e modos de
navegação adicionais.

## Extensão: navegação sazonal e por limiar

`AccumulatingTransposer` guarda um deslocamento em graus de escala. Cada evento
soma um passo configurável; fases sazonais na primeira metade do ciclo sobem e
fases na segunda metade descem. A faixa de −128 a +128 graus impede crescimento
sem limite e informa quando um extremo foi alcançado. A classe recebe apenas a
fase normalizada: conhecer `Environment` e aplicar o resultado ao quantizador
são responsabilidades do adaptador futuro.

`ThresholdStepSelector` mantém até 32 limites superiores estritamente
crescentes. Um sinal contínuo seleciona uma região e a saída informa endereço,
mudança, direção e número total de transições. A primeira seleção é explícita;
permanecer dentro da mesma região não gera nova transição. Limiares inválidos
são rejeitados sem alterar a configuração anterior.

`SequencerNavigationTests` cobre direção, limites, wrap de fase, limiares
uniformes e personalizados, cruzamentos ascendentes/descendentes e entradas
não finitas. Os dois componentes também são exercitados sob contagem de heap
em `RealtimeSafetyTests`. Ainda faltam integração, render e escuta.

Após essa extensão, o build Release completo e a suíte 27/27 passaram em
41,80 s em 24/09/2026. A swap permaneceu sem uso durante o build registrado.
