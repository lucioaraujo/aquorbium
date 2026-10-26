# Aquorbium — coorte do Master criativo

Data: 2026-09-30

## Objetivo

Implementar o Master criativo sem misturá-lo ao `OutputStage`, que continua
sendo a proteção técnica final. A superfície recebe somente um macro grande,
`CREATIVE MASTER`; densidade, calma, luz e integridade distribuem internamente
o gesto entre os corpos do Master.

## Corpos da coorte

1. crossover Linkwitz–Riley de quarta ordem em grave, médio e agudo;
2. compressão estéreo vinculada independente por banda;
3. EQ de três bandas derivado da mesma separação, sem filtro duplicado;
4. largura mid/side somente em médio/agudo, preservando a fundação grave;
5. compressão óptica lenta de cola antes do limiter técnico.

## Rota

```text
organismos → Ambiente → Espaço → Cauda → calibração
            → MasterChain criativo → OutputStage técnico → saída
```

`influence=0` retorna os dois samples de entrada bit a bit e não executa os
corpos internos. Ao abrir o macro, uma rampa de 45 ms introduz a cadeia. O
`crackStress` estreita apenas a informação lateral acima do grave. A redução
de ganho é publicada por telemetria, mas ainda não volta ao Cérebro.

## Limites

- sem heap, lock, I/O ou tamanho variável no callback;
- estados fixos e coeficientes calculados fora de `process()`;
- entradas não finitas viram zero;
- saída criativa não faz hard clipping; a proteção continua exclusivamente no
  `OutputStage`;
- EQ antes/depois ainda não é exposto: nesta coorte ele permanece antes da
  dinâmica;
- loudness BS.1770, true peak conforme norma e oversampling continuam fora;
- nenhuma alteração em `RASGO_MODULAR`.

## Validação de 2026-09-30

- build Release completo aprovado;
- teste dedicado aprovado: bypass exato, três bandas, redução independente,
  cola, largura sob trinca, entradas não finitas e zero alocação;
- testes críticos de motor, controle, tempo real, persistência e layout: 6/6;
- suíte integral: 45/45 em 104,45 s;
- auditoria gráfica: 72 PNGs, quatro resoluções e dezoito vistas por resolução;
- inspeção visual das vistas Master em 1920×1006 e 960×600 sem sobreposição;
- benchmark com banco e Master ativos: 21,57% em 48 kHz/bloco 128/quatro
  vozes; pior caso 42,95% em 96 kHz/bloco 32/quatro vozes.

As medições são offline e não substituem a próxima escuta no dispositivo nem
garantem ausência de XRUN.

## Critério de pronto

Bypass exato, separação espectral mensurável, redução independente por banda,
grave estável sob width, automação suave, zero alocação, build Release, suíte
completa e benchmark serial. A escuta musical permanece obrigatória depois.
