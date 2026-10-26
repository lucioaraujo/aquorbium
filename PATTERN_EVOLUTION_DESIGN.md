# Aquorbium — banco de padrões e Random to Loop

Atualizado em 2026-09-24. Estado: protótipos isolados compilados e testados;
sem integração ao sequenciador ativo, ao `AquariumIntegrity`, à GUI ou ao
aplicativo.

## Problema musical

A Geometria precisa conservar acontecimentos sem congelar o Bioma. Dois
comportamentos complementares atendem a isso:

- recuperação de configurações reconhecíveis e encadeamento de formas;
- passagem contínua entre acaso renovado e repetição com memória.

O primeiro permite retorno estrutural. O segundo permite estabilizar um fluxo
sem trocar abruptamente de aleatório para loop.

## `StepPatternBank`

O banco guarda oito snapshots fixos de `AddressableStepMemory`. Cada snapshot
preserva comprimento e os quatro valores dos seus até 32 passos. Uma cadeia
de até 16 referências determina a ordem de recuperação e informa volta ao
início. Slots ausentes e cadeias que apontem para slots vazios são rejeitados
sem alterar a cadeia válida anterior.

Salvar/restaurar em memória não é persistência em disco. Serialização, nomes,
versionamento e confirmação de substituição pertencem à camada de aplicação e
continuam pendentes.

## `RandomToLoop`

O módulo recebe um valor vivo e um controle `loopAmount` entre 0 e 1. Durante
o primeiro ciclo, preenche uma memória fixa de 1 a 64 posições. Depois:

- em `0`, publica e grava o valor vivo: acaso continuamente renovado;
- em `1`, publica o valor gravado sem alterá-lo: ciclo fixo;
- entre os extremos, publica e grava a interpolação: o loop envelhece
  gradualmente em direção ao fluxo novo.

Os extremos têm caminhos explícitos para conservar identidade numérica. A
classe não gera acaso; Lorenz, Markov, random walk ou outra fonte podem
alimentá-la sem acoplamento.

## Contrato e validação

Ambos usam apenas armazenamento fixo, têm trabalho limitado e não alocam,
bloqueiam ou fazem I/O. `PatternEvolutionTests` cobre snapshot, restauração,
cadeia, wrap, slots inválidos, captura inicial, extremos exatos, interpolação e
entradas não finitas. `RealtimeSafetyTests` inclui salvamento/restauração,
avanço da cadeia e processamento do loop sob contagem de heap.

Antes da integração: decidir quem solicita recuperação; mapear a integridade
do aquário sem apagar consequências; limitar mudanças de padrão; renderizar e
escutar se a memória produz forma reconhecível sem repetição mecânica.

Em 24/09/2026, o build Release completo e a suíte 28/28 passaram em 40,65 s.
A swap permaneceu em 0 B no preflight do build. Isso valida o núcleo isolado,
não a integração musical ou uma sessão interativa do aplicativo.
