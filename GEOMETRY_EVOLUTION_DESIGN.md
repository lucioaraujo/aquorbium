# Aquorbium — evolução estrutural da Geometria

Atualizado em 2026-09-24. Estado: três componentes isolados compilados e
testados; sem integração ao sequenciador ativo ou ao aplicativo.

## Rotação estrutural

`PatternRotationMutator` mantém um padrão binário de até 32 posições e aplica
`std::rotate` depois de 1–1024 ciclos completos. Rotação assinada define
direção e distância. A operação muda a fase estrutural sem criar ou apagar
pulsos, informa quando ocorreu e conta ciclos e mutações.

O componente não inventa a frequência da mutação nem sabe o que é um compasso
de host. `GeometricSequencer`, relógio ecológico ou futuro `AudioPlayHead`
devem fornecer eventos de ciclo explícitos.

## Layout paralelo/serial

`StepMemoryLayout` traduz quatro trilhas lógicas sobre os 32 endereços da
memória compartilhada em três topologias:

- quatro trilhas paralelas de oito passos;
- dois grupos seriais de dezesseis passos;
- uma cadeia serial de trinta e dois passos.

O layout não copia dados nem possui cursor. Ele só resolve endereço,
comprimento lógico e grupo, permitindo trocar topologia sem migrar os 32
valores subjacentes.

## Curvas de deriva

`DriftCurveShaper` separa dois eixos que antes estavam confundidos:

- `smoothness`: quantização em oito degraus ↔ trajetória contínua;
- `family`: linear, exponencial, exponencial inversa, Bézier ease, arco ou pico.

`curvature` controla a intensidade dentro da família escolhida. O modelador é
normalizado e sem estado: random walk, Lorenz, Relabi ou gesto humano fornecem
a fase; ele apenas define como essa fase se move perceptualmente.

## Contrato e validação

Os três componentes usam armazenamento fixo, trabalho limitado e nenhuma
alocação, trava ou I/O. `GeometryEvolutionTests` cobre agenda, direção,
preservação de pulsos, mapeamento completo, modos inválidos, famílias,
suavidade e entradas não finitas. `RealtimeSafetyTests` os executa sob
contagem de heap.

Antes da integração: definir a origem do ciclo; decidir quem troca o layout;
mapear curvas a uma consequência audível; renderizar trajetórias; e provar por
escuta que mutação estrutural difere de simples rotação performática.

Em 24/09/2026, o build Release completo e a suíte 30/30 passaram em 43,00 s.
O preflight registrou 12 GiB de memória disponível e 9,2 MiB de swap ocupada,
muito abaixo do limite de segurança. Isso valida os contratos isolados, não a
musicalidade nem a estabilidade de uma sessão interativa.
