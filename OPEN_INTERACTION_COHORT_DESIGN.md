# Aquorbium — coorte de interação aberta

Atualização: 2026-10-02.

## Objetivo

Esta coorte transforma quatro hipóteses da pesquisa em mecanismos de core
portáteis e audíveis, sem aumentar a densidade do painel. Os componentes
entram nas quinze famílias do `IntegratedModuleBank`, herdam `LAB MIX`, o
bypass antecipado e os controles grandes já existentes. A entrega é uma rota
experimental para escuta; não promove ainda `AccessPort`, toque multissensor
ou edição textual ao vivo a funções completas do instrumento.

## Componentes e destino experimental

| Componente | Função | Família visível |
|---|---|---|
| `DoubleScrollAttractor` | atrator caótico tridimensional determinístico, com duas regiões, três eixos e pares complementares | `LORENZ + DRIFT` |
| `SemanticAccessRouter` | 24 rotas fixas entre oito portas semânticas de áudio, controle/modulação e evento | `BRANCH + PERMUTE` |
| `OpenRuleProgram` | até 16 regras simples compiladas fora do callback e avaliadas na fronteira de evento | `EVOLVE + STAGES` |
| `ContactCrackleExciter` | excitação estéreo por contato, posição, pressão, velocidade e entropia | `GESTURE WAVES` |

## Proveniência e diferença autoral

O papel musical do double-scroll foi comparado à descrição oficial do
[Joranalogue Orbit 3](https://joranalogue.com/products/orbit-3): caos
determinístico tridimensional, regiões biestáveis e saídas complementares que
podem operar em taxas de controle e áudio. O Aquorbium não copia circuito,
esquema, firmware ou código do produto. `DoubleScrollAttractor` usa um sistema
digital por trechos, integração de ponto médio, parâmetros e normalização
próprios, subordinados ao Bioma e aos limites do banco.

`SemanticAccessRouter`, `OpenRuleProgram` e `ContactCrackleExciter` também são
implementações próprias. As referências de catálogo registradas em
`aquorbium-arquitetura.md` fornecem problemas e princípios de interação, não
receitas de implementação.

## Contratos individuais

### DoubleScrollAttractor

- estado tridimensional e seed determinística;
- integração RK2 por amostra, reset seguro diante de estado não finito ou
  divergente e normalização por `tanh`;
- três saídas bipolares, três complementares exatas e observação da região;
- taxa limitada de 0,002 a 80 Hz e parâmetros unitários de separação,
  distribuição e região tame/wild;
- a comparação A/B musical com `LorenzCV` continua obrigatória.

### SemanticAccessRouter

- oito portas e 24 rotas pré-alocadas;
- domínios de origem: áudio, controle e evento;
- domínios de destino: áudio, modulação e evento;
- áudio destinado a modulação passa por seguidor de envoltória; os demais
  cruzamentos mantêm semântica explícita;
- ganhos, profundidade contextual e somas são limitados;
- a configuração atual é fixa e preparada fora do callback. Edição por
  `AccessPort`, patch e organismo permanece fora desta entrega.

### OpenRuleProgram

- gramática: `fonte ('<'|'>') número ':' destino '=' número`, com regras
  separadas por ponto e vírgula;
- fontes: densidade, perigo, dia, estação, trinca e caos;
- destinos: ramificação, evolução, gesto e integridade;
- compilação transacional: uma expressão inválida não substitui o programa
  anterior;
- avaliação limitada, sem alocação, lock ou parsing no callback;
- a gramática é deliberadamente pequena. Editor GUI/APVTS, mensagens de erro
  e publicação atômica de programas editados continuam pendentes.

### ContactCrackleExciter

- contato recebe posição XY, pressão, velocidade e entropia;
- burst combina ruído, impulso e corpo ressonante, com panorama de potência
  constante e saída limitada;
- seed torna os testes reproduzíveis; um futuro adaptador de GUI poderá somar
  jitter físico real;
- `amount=0` produz zero exato;
- a rota atual é disparada por eventos ecológicos. O gesto simultâneo de dois
  pontos no vidro ainda não existe.

## Integração e observação

O atrator é calculado antes da fronteira de evento. As regras leem o contexto
ecológico e a magnitude caótica nesse limite; seus resultados modulam as
famílias sem trocar topologia em plena amostra. O contato recebe o evento, mas
decai e produz áudio por amostra. O roteador lê áudio, controle e evento já
saneados e devolve contribuição limitada à ramificação.

A telemetria do banco informa `doubleScrollX`, `accessModulation`,
`openRuleInfluence` e `contactEnergy`. Esses campos existem para teste e
diagnóstico; não criam gráficos ou textos novos no painel. Nenhum valor foi
adicionado ao formato persistente.

## Segurança em tempo real

- C++20 puro, sem dependência de JUCE;
- arrays e estados de tamanho fixo;
- nenhuma alocação, I/O, lock ou espera no processamento;
- parsing e configuração somente fora do callback;
- saneamento de parâmetros e entradas não finitas;
- saídas limitadas e reset de divergência do atrator;
- `LAB MIX=0` retorna antes de processar toda a coorte.

## Validação de 2026-10-02

- sintaxe estrita com `-Wall -Wextra -Wpedantic -Werror`: aprovada;
- `aquorbium_open_interaction_tests`: aprovado;
- `aquorbium_integrated_module_bank_tests`: bypass exato, quinze famílias,
  determinismo, integração ecológica e observabilidade da coorte aprovados;
- `aquorbium_realtime_tests`: aprovado com zero alocação no callback;
- compilação direcionada com um job: aprovada;
- suíte integral, benchmark, app/dispositivo real, render e escuta musical:
  não executados neste marco.

O preflight encontrou cerca de 10 GiB disponíveis, disco em leitura/escrita e
espaço suficiente, mas 1,7 de 2 GiB de swap ocupados e outra sessão de
desenvolvimento ativa. Por governança de recursos, a validação parou nos alvos
direcionados. O próximo portão técnico é repetir suíte e benchmark quando a
swap estiver abaixo de 50%; o próximo portão artístico é a escuta A/B das
quatro famílias, começando por Lorenz versus double-scroll.
