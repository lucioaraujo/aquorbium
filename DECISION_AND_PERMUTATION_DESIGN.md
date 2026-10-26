# Aquorbium — decisão ramificada, permutação e Relabi

Atualizado em 2026-09-24. Estado: núcleo isolado compilado e testado; sem novas
rotas no aplicativo.

## Árvore de decisão em duas camadas

`BranchingStepRouter` recebe dois sinais normalizados. O primeiro escolhe uma
categoria ampla e o segundo escolhe uma variante dentro dessa categoria. A
forma é limitada a oito categorias por quatro variantes, totalizando no máximo
32 folhas. Cada camada pode usar divisões uniformes ou limiares próprios,
estritamente crescentes.

A saída informa categoria, variante, índice linear, mudança e quantidade de
transições. O módulo não atribui significado às folhas: passos, estratégias de
sobrevivência e estados de presença continuam consumidores possíveis, não
decisões escondidas no roteador.

## Matriz de permutação

`NotePermutationMatrix` mantém um pool compartilhado de até 16 notas e oito
linhas de permutação. Uma linha contém cada índice exatamente uma vez. Trocar
a linha muda a ordem sem duplicar os dados das notas. A leitura acrescenta
offset de oitava e escala de velocidade limitados, informa volta e conserva um
contador de eventos.

Linhas com índices ausentes, repetidos ou fora da faixa são rejeitadas sem
substituir uma linha válida. O módulo trabalha em graus; converter para altura
final continua função do `IntervalQuantizer`.

## Relabi

`BiomaRelabi` já era usado por `AutonomousEnergy`, somando quatro oscilações
lentas de taxas não relacionadas. O novo teste formaliza determinismo bit a
bit entre instâncias, reprodução após reset, variação útil, limites e
recuperação de taxa inválida. Uma futura ligação a `SchoolInfo` ainda precisa
demonstrar que produz coordenação sem uníssono mecânico.

## Contrato e portões

Os três caminhos usam estado fixo, não alocam, não bloqueiam e não fazem I/O.
`DecisionPermutationTests` cobre camadas, limiares personalizados, rejeição de
configuração inválida, permutações, oitava, velocidade e Relabi.
`RealtimeSafetyTests` os executa sob contagem de heap.

Antes da integração: nomear categorias/variantes ecológicas; limitar a taxa de
troca; decidir quem seleciona linhas; renderizar trajetórias comparáveis; e
escutar se a permutação e o campo Relabi criam função composicional distinta.

Em 24/09/2026, o build Release completo e a suíte 29/29 passaram em 38,56 s.
O preflight registrou 13 GiB de memória disponível e apenas 9,2 MiB de swap
ocupada, muito abaixo do limite de segurança. Isso não substitui escuta ou
ensaio interativo do aplicativo.
