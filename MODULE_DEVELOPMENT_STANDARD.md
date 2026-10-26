# Aquorbium — padrão de arquitetura dos módulos

Atualização: 2026-10-02.

## Finalidade

Um módulo do Aquorbium não entra no instrumento apenas por executar um
algoritmo conhecido. Ele precisa acrescentar uma identidade musical ao Bioma,
continuar fértil sem MIDI e oferecer intervenção performática legível. A
sofisticação procurada é profundidade de relação, não quantidade de opções.

## Três modos obrigatórios

Todo módulo deve funcionar no mesmo motor em três situações, sem duplicar DSP:

1. **Autônoma:** ambiente, organismos, desejo e memória interna fornecem a
   energia e as decisões.
2. **Performance:** controles humanos deslocam uma intenção musical clara com
   transição suave, faixa útil e retorno previsível.
3. **Híbrida:** gesto externo influencia probabilidades, limites ou foco, mas
   não substitui o relógio e o estado do Bioma.

MIDI, GUI e futura automação são adaptadores. O módulo do core recebe um
snapshot pequeno de intenção; ele não conhece dispositivo, componente JUCE ou
thread de interface.

## Camadas arquiteturais

- **Modelo:** matemática, invariantes, unidades, faixas e política de seed.
- **Decisão:** estado discreto em taxa de controle; não sintetiza áudio.
- **DSP:** transformação por amostra/bloco, estado fixo e transições suaves.
- **Integração:** rotas explícitas de áudio, modulação, evento e observação.
- **Performance:** poucos controles macro com consequência musical descrita.
- **Telemetria:** somente o necessário para ouvir, visualizar e diagnosticar.

Uma classe não deve acumular essas responsabilidades quando puderem ser
testadas separadamente. Comunicação em tempo real usa dados triviais, memória
pré-alocada e propriedade inequívoca do estado.

## Dossiê antes da implementação

O desenho registra:

- problema musical e papel ecológico ainda ausente;
- fontes primárias e quais conceitos foram apropriados, sem copiar código;
- equações, estados, comportamento nos extremos e estratégia antialias/estabilidade;
- modo autônomo, controles de performance e regra do encontro entre os dois;
- entradas, saídas, feedbacks e limites de ganho/CPU/memória;
- alternativas descartadas e motivo;
- critérios técnicos e perguntas de escuta.

## Portões de aceitação

1. **Arquitetura:** API pequena, dependências direcionais, responsabilidade
   clara, core C++ portátil e integração removível sem quebrar o instrumento.
2. **Correção:** testes de extremos, seed, reset, invariância de bloco, valores
   não finitos e transições de estado.
3. **Tempo real:** nenhuma alocação, lock, espera, I/O ou trabalho sem limite no
   callback; memória e pior caso conhecidos.
4. **Áudio:** saída limitada, DC e descontinuidades medidas, espectro avaliado
   quando houver não linearidade ou formas descontínuas.
5. **Generativo:** produz percurso coerente sem eventos externos e não cai em
   repetição curta acidental nem aleatoriedade sem memória.
6. **Performance:** controles possuem nome e efeito audível claros, suavização
   definida e nenhum salto destrutivo ao alternar entre autônomo e híbrido.
7. **Escuta:** render reproduzível, comparação adequada e registro honesto do
   que mudou; diferença não é automaticamente melhoria.
8. **Orçamento:** benchmark antes/depois e decisão explícita de manter,
   simplificar, deixar experimental ou arquivar.

## Vocabulário obrigatório de estado

Para evitar confundir código existente com função disponível no instrumento,
cada família deve declarar separadamente estes estados:

1. **pesquisada** — hipótese e função musical documentadas;
2. **protótipo de core** — classe isolada compilando e testada, ainda sem mudar
   o som do aplicativo;
3. **integrada ao áudio** — participa de uma rota explícita do `BiomaEngine`;
4. **operável** — possui controles visíveis, grandes, persistentes e ligados ao
   callback por snapshot coerente;
5. **validada tecnicamente** — resposta, bypass, limites, layout e segurança de
   tempo real passaram pelos testes correspondentes;
6. **aprovada musicalmente** — escuta humana registrou utilidade, faixa e função
   composicional.

Uma caixa marcada na lista de tarefas conclui somente a frase daquela tarefa.
“Prototipar” marcado não significa “módulo operável”, e teste automatizado não
significa aprovação musical. A documentação deve mostrar o estado mais baixo
ainda pendente e não usar apenas “implementado” para etapas diferentes.

## Regra de profundidade

O marco inicial de um módulo pode ser pequeno, mas deve ser completo dentro do
que promete. Funções futuras aparecem como candidatas, nunca como controles
fictícios. Só ampliamos a superfície depois que a fundação passou pelos portões
e demonstrou espaço musical para a próxima camada.

## Estado do método — marco Espaço, 2026-08-12

O ciclo foi aplicado ao módulo de Espaço: dossiê conceitual e pesquisa antes do
código; protótipo isolado; testes de borda, silêncio e alocação; renders de
impulso e pares A/B; integração como uma única instância compartilhada no
`BiomaEngine`; medição de CPU; e superfície performática mínima com seis
controles persistidos (`Return`, `Gesture`, `Size`, `Spread`, `Tone` e `Memory`).

O próximo passo não é ampliar a arquitetura, mas calibrar por escuta o caráter
do retorno e desses controles. Decisões de musicalidade continuam sendo
registradas como evidência de escuta, separadas dos critérios de estabilidade,
tempo real e consumo de CPU.

## Regra de promoção após a auditoria de visibilidade — 2026-09-27/28

Quando houver componentes de core já testados mas invisíveis, a dívida de
integração tem prioridade sobre criar novos protótipos. Uma família por vez deve
atravessar rota ao vivo, semântica de controle, persistência/cenas, bypass,
resposta, tempo real, layout e escuta. Só então a próxima família é escolhida.

A interface não compacta controles para acomodar uma família nova. Mantêm-se
knobs de pelo menos 52 px e áreas interativas de 80×92 px; quando faltar espaço,
usa-se rolagem ou páginas com navegação explícita. Sobreposição, abreviação
ilegível e knobs pequenos são falhas de integração, não compromissos aceitos.

O primeiro uso desta regra promoveu `AddressableStepMemory` e
`HarmonicProgression` à Geometria. Em seguida, a dívida restante foi reunida
no `IntegratedModuleBank`: uma rota experimental removível, um bypass mestre e
quinze famílias, sem transformar cada helper em um módulo visual independente.
Essa integração coletiva serve para escuta e triagem. Uma família só deixa o
laboratório e recebe integração ecológica especializada depois da auditoria
humana de ritmo, melodia, harmonia, timbre, eventos e função composicional.

A coorte emergente de 28/09 reforça esta regra: todo componente novo entrou no
mesmo marco com teste próprio, consequência numa família visível, bypass
herdado, teste de alocação e medição de CPU. Quando a semântica final ainda
depende de `Organism`, Lupa, predador/presa, `AccessPort` ou
`AquariumTransform`, a tarefa registra “protótipo concluído” e mantém esse
vínculo explicitamente pendente; o banco experimental não finge concluir a
integração ecológica.

A coorte avançada de voz e ecologia aplicou o mesmo método. BiomaBass, cardume,
resíntese, SPOT, fome, auto-feedback e inércia ganharam código, teste, rota e
orçamento antes de qualquer novo knob. Onde o protótipo usa contexto genérico
do banco em vez de `Organism`, `SchoolInfo`, `Ecosystem`, quantizador ou saída
final reais, a tarefa especializada continua aberta e não é absorvida pelo
marco técnico.

A coorte de geração e interação mantém a mesma distinção. Prototipar foco
lock-in não cria ainda um terceiro modo do Observer; ITD e sombra de cabeça não
são HRTF medida; uma voz estrutural sem entrada não é o Master; e cenas fixas
do laboratório não são cenas persistentes do `Ecosystem`. Esses nomes só podem
ser promovidos quando a integração correspondente for implementada e ouvida.

A coorte de interação aberta de 02/10 reforça também a regra de superfície:
quatro mecanismos novos foram ligados a famílias existentes e ganharam
telemetria de observabilidade sem criar controles menores ou sobreposição. Um
roteador fixo não conclui um `AccessPort` editável; um exciter acionado por
evento não conclui o gesto físico; um parser de core não conclui o editor de
regras. Cada adaptador de performance permanece uma tarefa independente.

A coorte de refinamento do mesmo dia estabelece uma segunda regra: concluir
um protótipo não impede voltar ao núcleo para reduzir aliasing, fechar um loop
ecológico real ou substituir contexto sintético por gesto/ambiente observado.
Refinamentos devem preservar bypass e reset, ter teste comparativo específico
e declarar o alcance da evidência. Uma redução espectral num caso dirigido não
autoriza chamar o algoritmo de livre de alias; uma rota comum ar/água não
autoriza anunciar regiões acústicas por organismo; captura da Lupa no banco
não equivale a automação canônica da posse.

O diretor composicional de 02/10 adiciona um critério para sistemas que
coordenam várias famílias: a variação precisa declarar sua escala temporal e
sua fonte de fronteiras. Enquanto não houver clock de andamento, ciclos
ecológicos são segmentos formais lentos, não compassos. O plano deve manter
material reconhecível, alterar dimensões nomeadas e deixar a aprovação da
relação entre segmentos para renders e escuta humana.
