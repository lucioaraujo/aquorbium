# Aquorbium — cauda sintética integrada

Atualizado em 2026-09-26. Estado: integrada, operável e validada tecnicamente;
escuta musical e estabilidade prolongada no dispositivo real pendentes.

## Função musical

`AquariumTail` é uma cauda estéreo somente molhada. Não reconstrói uma sala:
prolonga acontecimentos como atmosfera do aquário. Uma instância compartilhada
recebe a soma direta já acrescida das primeiras reflexões de `MultiTapSpace` e
devolve a reverberação tardia antes da calibração de saída.

## Modelo e limites

- Rede de quatro linhas de atraso com matriz Hadamard ortonormal.
- Uma segunda matriz assinada deforma a circulação pelo parâmetro `nightmare`;
  a interpolação entre matrizes permanece contrativa.
- `decaySeconds` calcula ganho para aproximadamente −60 dB; `damping` escurece
  o retorno; `diffusion` controla injeção e largura; `level` limita a saída wet.
- Memória fixa de 4 × 32.768 floats, cerca de 512 KiB. O tamanho máximo da
  linha é limitado; em taxas extremas o tempo físico máximo encolhe.
- Feedback é limitado a 0,985. Entradas inválidas viram silêncio; não há
  arquivos, JUCE, lock, heap ou trigonometria no `tick()`.
- Mudança de `size` recalcula o comprimento imediatamente. A integração o fixa
  em 0,55 e mantém difusão em 0,75 até existir mudança sem descontinuidade.

## Superfície de performance

O cartão `06 CAUDA` oferece quatro macros grandes:

- `TAIL MIX`: influência total e bypass transparente;
- `DECAY`: mapeamento exponencial de 0,30 a 12 segundos;
- `DAMPING`: absorção progressiva do agudo;
- `NIGHTMARE`: torção da circulação interna.

Todos os valores recebem rampa de 15 ms; os coeficientes são atualizados a
200 Hz. O FDN continua avançando com `TAIL MIX=0`, mas sua saída não é somada.
Assim o bypass é bit a bit transparente e a ativação não reinicializa memória.
Os três controles dependentes ficam visualmente inativos no bypass.

Os quatro valores entram no mesmo quadro lock-free dos demais controles e nas
cenas. O snapshot atual tem 35 valores totais. Cenas V1 de 25 valores recebem
os padrões de Ambiente e Cauda; cenas de 31 valores recebem apenas os padrões
da Cauda.

## Validação técnica de 2026-09-25/26

- Núcleo: silêncio, impulso, caimento, finitude em 48/96 kHz e diferença da
  matriz `nightmare`.
- Integração: cada macro alterou a saída; com `TAIL MIX=0`, os três controles
  dependentes produziram diferença exatamente zero.
- Estresse: quatro vozes, macros altas, pico 0,462652, sem amostra inválida.
- Render: `aquorbium_synthetic_tail_render DIRETORIO_NOVO` produz quatro WAVs
  alinhados de 12 s e relatório; o diretório é recusado se já existir.
- Evidência em `validation-output/2026-09-25-synthetic-tail/`: bypass repetido
  bit a bit, picos 0,220–0,237, diferenças RMS de 0,00161, 0,00778 e 0,00480
  para curto/escuro, longo/claro e nightmare.
- Custo offline local: cerca de 0,057 × tempo real para o instrumento completo;
  é medição desta máquina, não orçamento universal.
- Layout: 20 capturas em quatro dimensões, knobs ≥52 px, áreas ≥80×92 px e
  inspeção visual de 1280×800/960×600 sem sobreposição.

## Portões restantes

1. Escutar os quatro renders e a faixa útil dos controles no instrumento.
2. Verificar cliques, estalos e travamento em uso manual prolongado.
3. Decidir se dia/noite, estação ou `crackStress` devem modular a Cauda; hoje
   ela é manual e não cria feedback ecológico oculto.
4. Implementar mudança de `size` sem descontinuidade antes de expô-la.
5. Só após escuta decidir calibração de nível e eventual vínculo ambiental.

O código é implementação original local. A ideia de reverb como gerador de
cauda, e não simulador de sala, já estava documentada na pesquisa do projeto.
