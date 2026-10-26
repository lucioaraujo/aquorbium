# Coorte de integração especializada

Data: 2026-09-29

## Objetivo

Fechar pendências que já possuíam fundamentos isolados, mas ainda não
participavam do encadeamento musical real do Aquorbium. Esta coorte não cria
novos controles de painel: usa estado ecológico, traços dos organismos e
áudio já disponíveis no core.

## Integrações

1. O `Species::Pulsar` recebe pulsaret, agrupamento interno e mascaramento por
   pulso. Densidade, integridade e traços do organismo definem continuidade,
   espaçamento e complexidade, com decisões determinísticas por seed.
2. Cada disparo de `BiomaGranularCapture` deriva duração, velocidade, pan,
   ganho e reversão de `strength`, `agility` e `intelligence` do organismo
   granular, em vez de usar uma nuvem uniforme.
3. O subgrave de `BiomaBassVoice` passa a receber um intervalo negativo do
   `IntervalQuantizer` usado pela decisão melódica do banco.
4. A amostra final pós-Space, pós-cauda e pós-proteção retorna no sample
   seguinte ao `CrossFeedbackStateMachine`; sua modulação altera decisões do
   sequenciador com limites estreitos e histerese.
5. `MultiTapSpace` recebe uma deriva lenta, sem alocação, por
   `SlewRateRandomWalk`. A profundidade é pequena, a velocidade de variação é
   limitada e o valor padrão da classe continua neutro.
6. As grandezas contínuas de `Desire`/Brain que chegam ao
   `IntegratedModuleBank` passam por soft take-over de um polo antes de
   comandar módulos internos. Eventos e áudio de feedback não são apagados
   por essa suavização.

## Contrato de segurança

- nenhum heap, mutex, arquivo ou log no callback;
- toda história tem capacidade fixa;
- seeds reproduzíveis;
- entradas não finitas são retidas ou substituídas por valores seguros;
- o bypass `LAB MIX = 0` continua produzindo zero exato;
- a deriva do Space tem profundidade e slew limitados e não altera a matriz
  de feedback;
- nenhuma alteração em `RASGO_MODULAR`.

## Critério de pronto

Build Release, testes específicos dos corpos alterados, suíte CTest completa,
teste de zero alocação existente e benchmark serial do core. A aprovação
musical permanece uma etapa humana posterior.

## Validação executada

- build Release completo concluído;
- testes diretamente afetados: 6/6;
- suíte completa: 44/44 em 94,24 s;
- benchmark ativo: 19,21% em 48 kHz/bloco 128/quatro vozes;
- pior caso observado: 38,92% em 96 kHz/bloco 64/quatro vozes, 2,57× tempo
  real offline;
- binário: 14.209.824 bytes, SHA-256
  `cdb25a33ea9dab4db3b5383dd711ed17815442835563f822fd64d7e8379180cc`;
- inspeção auditiva e estabilidade prolongada no dispositivo: ainda não
  executadas nesta rodada.
