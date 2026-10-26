# Política responsiva de janela — AQUORBIUM

**Data:** 2026-09-28
**Estado:** primeira implementação candidata ao padrão comum RASGO

## Objetivo

Separar o canvas artístico do tamanho obrigatório da janela. A interface deve
preservar hierarquia, leitura e precisão gestual em vez de escalar o painel
inteiro. Redimensionar, mover ou restaurar a janela não participa do estado
musical nem altera o motor de áudio.

## Metodologia obrigatória de layout — 2026-09-18

Esta regra vale para todo novo controle e para revisões do painel existente.
Não aprovar um layout somente porque todos os objetos cabem na janela.

- Knobs: diâmetro gráfico mínimo de 52 px lógicos, área interativa mínima de
  80×92 px, rótulo separado de 20 px com fonte de pelo menos 11 px e campo de
  valor separado de 20 px. Considerar o padding real do LookAndFeel, não só
  os bounds externos do slider. Nunca diminuir esses mínimos para caber.
- Texto descritivo, cabeçalho, status, rótulo, knob e valor possuem áreas
  próprias. Cartões com controles embutidos não desenham descrição atrás
  dos widgets. Reservar 68 px antes da área de macros.
- Falta de espaço: reorganizar colunas e usar rolagem local da coluna de
  cartões, mantendo cada banco inteiro em seu tamanho mínimo. Não comprimir
  knobs, não cortar um controle permanentemente e não escalar o painel todo.
  Arrastar knobs não arrasta o viewport; a roda rola a coluna sem mudar valores.
- Controles condicionais: mostrar a dependência. Com influência manual zero,
  seus controles dependentes ficam inativos; o controle de influência permanece
  acessível. Não alterar presets nem ativar influência silenciosamente.
- Na esfera, posições dos organismos são dados musicais, não decisões de
  layout. Preservá-las; deslocar as legendas, com fonte de 11 px, conectores
  e busca limitada que evita colisão com outras legendas, halos e Lupa. Em espaço
  excepcionalmente insuficiente, não sobrepor nem reduzir a fonte: a ausência
  de uma legenda deve ser tratada como limitação de layout, não aprovação.
- Critério de pronto: testes de contenção, separação e mínimos; auditoria dos
  bounds reais JUCE; inspeção das capturas em modo completo, compacto e
  emergência, incluindo bancos inferiores alcançados por rolagem. Repetir
  quando um novo controle mudar a densidade do painel. Inspeção de DPI e
  interação real continuam necessárias; geometria pura não certifica tudo.

Implementação local: `MacroPanelLayout.h`, `OrganismLabelLayout.h`, viewports
do painel e `WindowLayoutPolicyTests.cpp`. `--layout-audit=/diretorio` gera
capturas de quatro dimensões sem abrir áudio, com preferências isoladas;
recusa sobrescrever capturas/relatório existentes. Saídas de auditoria ficam
em `validation-output/`, fora do Git. Esta metodologia não autoriza mudar
outros instrumentos, especialmente Rasgo Modular.

## Implementação

- `src/app/WindowLayoutPolicy.h` contém somente geometria pura, sem JUCE:
  classificação de superfície, abertura inicial, reencaixe e recuperabilidade;
- a primeira abertura usa `88%` da área útil de um monitor, limitada ao canvas
  preferencial `1280×800`; pode chegar a `90%` para preservar o modo compacto;
- a janela restaura bounds e fullscreen em propriedades do usuário, separadas
  de presets/projetos, e revalida tudo contra os displays conectados;
- o monitor mais próximo dos bounds salvos é usado; o primário é apenas o
  fallback da primeira abertura;
- a GUI recalcula áreas semânticas nos modos completo, compacto e emergência;
- abaixo de `960×600`, um viewport mantém o canvas de emergência rolável em
  vez de produzir microtexto ou controles inacessíveis;
- `--settings-dir=/caminho` isola preferências durante ensaios automatizados e
  manuais, sem contaminar a configuração normal.

## Breakpoints atuais

| Modo | Entrada | Comportamento |
|---|---|---|
| completo | `≥1200×720` | módulos laterais, aquário central e bancos de performance roláveis |
| compacto | `≥1024×650` | três colunas de performance, cartões condensados e aquário preservado |
| emergência | abaixo do compacto | mesmos controles essenciais, margens menores e fallback rolável |

Os breakpoints usam coordenadas lógicas fornecidas pelo JUCE; portanto, não são
resoluções físicas fixas e não devem ser multiplicados manualmente pelo DPI.

## Evidência desta rodada

A matriz de resoluções não é uma rotina de toda alteração. Ela deve ser
repetida apenas quando a mudança afeta estrutura, densidade visual, navegação,
breakpoints, janela ou coordenadas dos componentes. Mudanças exclusivas de DSP,
testes, documentação ou lógica sem consequência geométrica usam somente os
testes proporcionais à área alterada. Quando o layout muda, validam-se primeiro
os três modos representativos (completo, compacto e emergência); a matriz ampla
fica reservada a marcos de interface e preparação de release.

- teste puro `aquorbium_window_layout_tests` aprovado para classificação,
  abertura, bounds fora da tela, monitor à esquerda e superfície removida;
- app JUCE compilado com JUCE `8.0.13`;
- inspeção histórica, anterior aos novos controles, aprovada em `1280×800`, `1040×700`, `1024×650`, `960×600` e
  fallback rolável `720×480`;
- inspeção de notebook aprovada em `1366×736`, a área útil disponível no
  monitor de `1368×768` desta sessão;
- inspeção solicitada em monitor de 1920 aprovada em `1920×1006`, sua área
  operacional nesta sessão: sem cortes, zoom global ou scroll;
- fechar/reabrir restaurou `1920×1006` no mesmo monitor usando preferências
  isoladas em `/tmp`;
- dois monitores permaneceram ativos durante os testes; nenhum código de
  RASGO Synth, Antitotem ou Navalha foi modificado.

## Validação ainda aberta

Revisão de 18/09: regras de mínimos/separação aprovadas em testes puros e
auditoria dos bounds reais JUCE. Evidência final em
`validation-output/2026-09-18-panel-audit-03/`: 12 capturas de 1920×1006,
1280×800, 1024×650 e 960×600, três vistas por dimensão (Síntese, Espaço,
Ecologia). Inspeção das capturas nos três modos confirmou maiores knobs,
separação dos textos de controle e legendas sem colisão com Lupa/entre si.
O ensaio é offline, sem dispositivo de áudio; não é aprovação de estabilidade,
gestos de mouse/teclado, DPI ou resposta musical. Rolagem pode mostrar um
cartão parcialmente no limite visível, mas permite alcançar todos os controles
no tamanho mínimo; não se trata de sobreposição nem corte permanente.

O teste humano de 2026-09-18 rejeitou a interface então em uso: knobs pequenos
e texto sobreposto nos cartões de Síntese/Espaço, além de travada da aplicação
e estalos. A captura `Screenshot from 2026-09-18 00-16-27.png`, fornecida por
Lucio, é o ponto de partida desta correção. As aprovações históricas acima
não se aplicam automaticamente ao painel atual. A nova coluna direita usa
rolagem e tamanho mínimo dos bancos; não houve redução dos gestos musicais.

Na sessão de 2026-09-18, o cabeçalho de `ECO CONTROLS` recebeu campo de nome,
`RENOMEAR` e `DESFAZER` numa linha de 24 px em y=50, antes dos sliders em
y=80. Ela substitui a legenda antiga, sem reduzir as seis linhas nem alterar
rodapés/breakpoints. `EcologyPanelLayout` testa contenção e separação nos
três painéis e no mínimo existente de 440×324; entrada tem pelo menos 208 px
e os dois botões 92 px. Nomes longos podem rolar no editor e aparecer completos
no tooltip do seletor. Inspeção visual, teclado/Enter e DPI continuam pendentes.

Em 2026-09-17, o rodapé de 42 px do aquário ganhou uma linha de 18 px com
`LUPA ATIVA/LIVRE` e contagem geométrica `FOCO N`. Os indicadores anteriores
ficam nos 24 px restantes, sem mudar a esfera ou os breakpoints. Esta alteração
compila, mas ainda não tem inspeção visual nos três modos nem aprovação de DPI.

O incremento de 2026-09-16 acrescenta seletor de quatro cenas, gravação,
recuperação, valores iniciais, importação/exportação e status no rodapé de
`ECO CONTROLS`. Transporte ocupa uma segunda linha, separada dos quatro
comandos locais; seleção/gravação/novo transporte ficam desabilitados durante
seletor e confirmação. Textos de erro completos e caminhos de backup ficam
também no tooltip do status. Diálogos assíncronos usam referência segura ao
componente e cancelamento sem alterar som/banco.
`EcologyPanelLayout.h` calcula as posições em geometria pura; o teste existente
verifica contenção, separação de linhas/botões e espaço dos sliders em três
painéis representativos e no limite mínimo de 440×324, incluindo ambas as
linhas do rodapé. Dimensões menores são recusadas pelo helper. A geometria
testável não substitui inspeção visual,
leitura de textos, acessibilidade ou ensaio gestual na janela JUCE.

- a página central `ECO CONTROLS` e o gesto de arrasto da Lupa compilaram em
  2026-09-15, mas ainda requerem inspeção visual e interação nos três modos de
  superfície; não há captura de tela nem aprovação de layout deste incremento;
- DPI real em `125%`, `150%` e `200%`;
- display `2560×1440` ou maior;
- desconexão/reconexão física de monitor durante a execução;
- plataformas Windows e macOS;
- ensaio humano dos gestos quando os cartões se tornarem módulos completos.

## Verificação de 2026-09-21

O status de áudio no cabeçalho foi dividido em duas linhas, com 12 px para
estado/taxa/buffer e 11 px para contadores, sem reduzir a área dos knobs.
`--layout-audit` produziu 12 capturas isoladas em
`validation-output/2026-09-21-layout-fix-01/` (1920×1006, 1280×800,
1024×650 e 960×600; Síntese, Espaço e Ecologia). O verificador confirmou
diâmetro gráfico mínimo de 52 px, área interativa mínima de 80×92 px,
rótulos/valores separados e controles dependentes inativos em zero. Inspeção
visual pontual das capturas 1280×800/Síntese e 960×600/Espaço não encontrou
nova colisão de texto. O diálogo de ÁUDIO novo não foi incluído nessas
capturas; sua interação e a legibilidade em DPI real ainda exigem teste na
janela. O modo de auditoria não abriu o dispositivo nem validou áudio.

Esta utilidade permanece local ao AQUORBIUM. Sua promoção para código comum do
RASGO só pode ocorrer depois de um segundo consumidor autorizado e validado.

## Verificação do cartão Ambiente — 2026-09-25

O cartão `05 AMBIENTE` reutiliza `MacroPanelLayout` com seis controles. A
auditoria offline gerou 16 capturas em 1920×1006, 1280×800, 1024×650 e 960×600,
incluindo uma vista dedicada ao Ambiente em cada dimensão. O verificador
confirmou knobs de pelo menos 52 px, áreas interativas mínimas de 80×92 px,
rótulos e valores separados e controles dependentes inativos em `ENV MIX=0`.
Inspeção das vistas de 1280×800 e 960×600 não encontrou sobreposição; no modo
menor, a rolagem preserva o tamanho dos controles inferiores. Isso ainda não
substitui o teste humano de rolagem, DPI e gesto na janela real.

## Verificação do cartão Cauda — 2026-09-25/26

O cartão `06 CAUDA` reutiliza `MacroPanelLayout` com quatro controles e fica
entre Ambiente e Master na coluna rolável. A auditoria offline em
`validation-output/2026-09-25-tail-layout/` gerou 20 capturas: Síntese, Espaço,
Ambiente, Cauda e Ecologia em 1920×1006, 1280×800, 1024×650 e 960×600. O
verificador confirmou diâmetro mínimo de 52 px, área interativa mínima de
80×92 px, rótulos e valores separados e DECAY/DAMPING/NIGHTMARE inativos em
`TAIL MIX=0`. As vistas da Cauda em 1280×800 e 960×600 foram inspecionadas sem
sobreposição; a menor conserva o tamanho por rolagem. DPI real e interação
manual permanecem pendentes.

## Verificação da memória formal na Geometria — 2026-09-26/27

O cartão `02 GEOMETRIA` passou a reutilizar `MacroPanelLayout` para `FORM MIX`,
`CONTEXT`, `INERTIA` e `MUTATION`. A superfície possui rolagem própria: ela não
reduz os knobs nem interfere nos cinco sliders lineares da faixa inferior.
`FORM MIX=0` mantém os três controles dependentes visualmente inativos.

A auditoria em `validation-output/2026-09-26-form-memory-layout/` gerou 24
capturas: Síntese, Espaço, Ambiente, Cauda, Geometria e Ecologia em 1920×1006,
1280×800, 1024×650 e 960×600. O verificador confirmou diâmetro mínimo de 52 px,
área interativa mínima de 80×92 px e separação entre rótulo, knob e valor. As
vistas da Geometria em 1280×800 e 960×600 foram inspecionadas: a segunda linha
é alcançada por rolagem e não há sobreposição. DPI real, gesto manual e escuta
continuam pendentes.

## Verificação da saída técnica no Master — 2026-09-27

`MacroPanelLayout` passou a aceitar também três controles sem alterar os
contratos existentes de quatro e seis. O cartão `07 MASTER` contém `OUTPUT
TRIM`, `CEILING` e `RELEASE`: três colunas quando há largura suficiente e duas
linhas nas janelas compactas. O Master criativo não é simulado nessa superfície.

A auditoria em `validation-output/2026-09-27-output-stage-layout/` gerou 28
capturas: Síntese, Espaço, Ambiente, Cauda, Geometria, Master e Ecologia nas
quatro dimensões. O verificador confirmou knobs ≥52 px, áreas ≥80×92 px e
separação de rótulo/knob/valor. As vistas Master em 1920×1006 e 960×600 foram
inspecionadas sem sobreposição; em 960×600 o terceiro controle ocupa a segunda
linha e permanece integralmente rolável. DPI real e gesto manual continuam
pendentes.

## Verificação da estrutura na Geometria — 2026-09-27

`MacroPanelLayout` passou a acomodar oito controles no cartão Geometria sem
reduzir knobs ou áreas interativas. `STRUCTURE MIX`, `STEP MEMORY`, `HARMONY` e
`PHASE PATH` ocupam duas linhas adicionais na superfície rolável; os três
últimos ficam visualmente inativos quando a influência está em zero.

A auditoria canônica em
`validation-output/2026-09-27-structure-memory-layout-v3/` gerou 40 capturas:
Síntese, Espaço, Ambiente, Cauda, quatro vistas da Geometria, Master e Ecologia
em 1920×1006, 1280×800, 1024×650 e 960×600. O verificador confirmou knobs de
pelo menos 52 px, áreas interativas de 80×92 px e separação entre rótulo, knob
e valor. As quatro vistas da Geometria em 960×600 foram inspecionadas sem
sobreposição; a rolagem é necessária e intencional. DPI real e gesto manual
continuam pendentes. As auditorias v1 e v2 permanecem como trilha histórica,
mas v3 é a evidência final deste marco.

## Verificação do banco integrado — 2026-09-28

O cartão `08 MÓDULOS` contém `LAB MIX` e quinze macros de família. Os dezesseis
controles formam oito linhas numa superfície interna rolável; o cartão fica no
fim da própria coluna direita rolável. `MacroPanelLayout` preserva o diâmetro
gráfico mínimo de 52 px e a área interativa de 80×92 px também para 16 itens.
Com `LAB MIX=0`, as quinze famílias aparecem inativas sem desaparecer.

O teste puro de layout passou nas superfícies representativas e a suíte integral
passou 41/41. A auditoria de bounds foi ampliada para 18 vistas por resolução:
as dez vistas anteriores e oito linhas do banco. As capturas não foram geradas
nesta rodada porque `xvfb-run` não está instalado; portanto, inspeção visual,
rolagem real e DPI permanecem pendentes e não são declarados aprovados.

## Verificação do Master criativo — 2026-09-30

`CREATIVE MASTER` foi acrescentado antes dos três controles técnicos. Os quatro
macros ocupam duas linhas estáveis em todas as larguras auditadas; nenhum knob
foi reduzido para encaixar o novo corpo.

O modo `--layout-audit` gerou 72 capturas temporárias: dezoito vistas em cada
uma das quatro resoluções. O relatório confirmou diâmetro gráfico mínimo de
52 px, área interativa mínima de 80×92 px e rótulos/valores separados. As
vistas Master em 1920×1006 e 960×600 foram inspecionadas sem sobreposição. O
valor do novo macro foi normalizado para duas casas decimais após a inspeção;
DPI físico e gesto manual continuam pendentes.
