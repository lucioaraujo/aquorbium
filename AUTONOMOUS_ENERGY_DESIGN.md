# Aquorbium — bioenergia autônoma

## 1. Objetivo

O Aquorbium deve produzir uma forma musical completa sem nota MIDI, arquivo ou
host externo. MIDI continua sendo uma influência performática, mas não abre a
torneira principal do som. O banco autônomo liga os cinco organismos
já existentes a um campo interno de energia, altura e duração.

Este módulo pertence somente ao Aquorbium. Integração com prompt, linguagem ou
outro instrumento está deliberadamente fora deste marco.

## 2. O que “autônomo” significa aqui

- Abrir o app é suficiente para o bioma começar a emergir sonoramente.
- Cada organismo possui energia e altura próprias; não são cinco cópias de
  uma nota central.
- As decisões nascem de `Desire`, ambiente, presença, integridade, traços do
  organismo e energia já produzida.
- A mesma seed, parâmetros e número de amostras geram o mesmo resultado,
  independentemente do tamanho dos blocos do host.
- Silenciar a bioenergia reduz seu ganho suavemente, mas o relógio interno
  continua. Reativar revela o estado presente em vez de reiniciar uma frase.
- MIDI usa o banco polifônico externo já existente e pode coexistir sem roubar
  ou reinicializar os cinco organismos autônomos.

## 3. Duas escalas de tempo

`AutonomousEnergy` separa decisão e som:

1. Um relógio de controle a 100 Hz atualiza ciclos de vida, decisões de
   presença e alvos harmônicos. O relógio independe dos blocos do dispositivo.
2. Energia, ativação e altura deslizam por amostra. Mudanças ecológicas nunca
   viram degrau de amplitude ou salto instantâneo de frequência.

Um campo Relabi de cinco taxas não comensuráveis produz encontros coletivos
sem impor compasso fixo. Ciclos individuais mais rápidos dão respiração a cada
organismo. O resultado não é um sequenciador MIDI oculto: são campos contínuos
que ocasionalmente atravessam fronteiras de decisão.

Desde 2026-09-26/27, o cruzamento coletivo positivo também avança uma única
`BiomaMarkov` compartilhada. Seus oito estados deslocam seção e contorno no
`GeometricSequencer`; decisões individuais não avançam a cadeia. O stream
aleatório da memória formal é separado, e `FORM MIX=0` reduz o deslocamento a
zero exato sem congelar o estado.

Desde 2026-09-27, as mesmas fronteiras de evento consultam uma
`AddressableStepMemory` e uma `HarmonicProgression`. Um padrão determinístico de
oito passos contribui altura e gate; a progressão recebe uma interpolação
explícita entre fase diária e sazonal. Fase orbital e identidade do organismo
deslocam o endereço consultado. `STRUCTURE MIX=0` conserva a saída anterior bit
a bit, sem congelar os dois estados nem consumir acaso adicional.

## 4. Mapeamento ecológico inicial

| Estado | Consequência musical |
|---|---|
| `density` | probabilidade de organismos despertos e velocidade das decisões |
| `calm` | ataques, liberações e glides mais longos |
| `cohesion` | preferência por graus relacionados da mesma estrutura harmônica |
| `danger` | maior mobilidade e dispersão dos graus, sem aumento bruto de volume |
| `dayPhase` | um extremo do percurso de fase da progressão harmônica |
| `seasonPhase` | outro extremo do percurso harmônico e forma de longo prazo |
| memória endereçável | deslocamento de altura e gate nos eventos existentes |
| progressão harmônica | graus de acorde escolhidos pela fase dia/estação |
| luz/temperatura/acasalamento/perigo | alvo contextual da memória formal Markov |
| estado Markov | deslocamento limitado da seção geométrica nos eventos coletivos |
| energia/força/presença | alvo de amplitude individual |
| energia de saída | homeostase: o bioma recua antes de crescer sem limite |
| integridade | redução de força; consequência autoral de ruptura continua pendente |

O campo de alturas usa uma rede intervalar de sete graus e oito regiões de
fundamental. A primeira memória de passos e progressão ecológica já desloca
essa fundação, mas ainda não é suporte definitivo a escala, edição de sequência
ou afinação. Microtonalidade, bancos editáveis e navegação geométrica ampliada
continuam módulos posteriores.

## 5. Fronteiras técnicas

- C++20 portátil, sem JUCE no core.
- Estado e memória fixos; nenhuma alocação, lock ou espera em `tickSample()`.
- Gerador pseudoaleatório próprio derivado da seed do instrumento. Decisões
  autônomas não consomem o estado aleatório do `BiomaBrain` e portanto não
  alteram a física apenas por estarem habilitadas.
- O banco autônomo possui fases próprias e não ocupa as quatro vozes externas.
- A bioenergia nasce desabilitada no core para renderes/testes explicitamente
  controlados; o app standalone a habilita por padrão porque essa é a
  identidade principal do instrumento.
- Toda telemetria atravessa a ponte atômica existente; a GUI nunca lê o módulo
  diretamente.

## 6. Critérios deste marco

- áudio finito e não silencioso sem qualquer `noteOn`;
- silêncio exato quando autonomia está desabilitada e não há entrada externa;
- determinismo bit a bit entre blocos diferentes;
- seeds distintas produzindo percursos diferentes;
- energia limitada e transições suaves;
- zero alocações no caminho de render autônomo;
- telemetria de energia, organismos ativos, raiz e eventos;
- render offline sem MIDI para escuta humana posterior;
- benchmark repetido com autonomia ativa.

Todos esses critérios possuem evidência automatizada. A formação atual inclui
Oscillator, Granular Striker, Modal Resonator, Pulsar e Physical String. O
render de referência é regenerado após mudanças de formação e registra em sua
telemetria os impactos e a energia da corda, além dos corpos granular e modal.
Na integração de cinco organismos, o render de 30 s/48 kHz terminou com cinco
organismos ativos, 101 decisões, sete nuvens granulares, quatro impactos modais
e seis excitações de corda. Após a calibração de nível, do balanço por espécie e
das caudas eventuais, RMS `0,0235317`, pico `0,145555` e delta adjacente máximo
`0,0828961`. Os valores
documentam esta execução; não são metas artísticas nem substituem escuta.

Striker, Modal e String capturam energia no próprio impacto. Durante a cauda,
um piso limitado `0,28 + 0,72 * energia` evita multiplicar esse gesto por quase
zero quando o organismo adormece. O piso é multiplicado pelo ramp global de
ativação: `setAutonomousEnabled(false)` ainda chega a silêncio exato em 120 ms,
sem congelar relógio nem estado DSP.

Este marco prova nascimento e forma musical interna. Ele não declara que a
composição, a afinação ou a paleta sonora estão finalizadas; essas decisões
dependem de escuta e dos próximos módulos de Geometria e Síntese.
