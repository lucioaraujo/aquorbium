# Aquorbium — coorte de refinamento do núcleo

Atualização: 2026-10-02.

## Objetivo

Esta coorte fecha quatro lacunas já registradas: reduzir energia de alias no
wavefolder senoidal, devolver ao Cérebro a compressão realmente observada no
Master, distinguir ar e água na rota experimental e transformar o gesto real
da Lupa em memória de movimento. O trabalho não acrescenta knobs, campos de
cena, páginas ou módulos visuais; ele aprofunda rotas existentes.

## Componentes e integração

| Componente | Função | Integração |
|---|---|---|
| `OversampledSineFolder` | avalia o sine-shaping em quatro subpassos interpolados e decima por média curta | `SynthFMVoice` e wavefolder pré-filtro do `BiomaBassVoice` |
| `BoundedMasterFeedback` | converte redução de ganho multibanda/cola em influência ecológica com ataque, release e teto | `MasterChain` → `BiomaBrain`, na fronteira de controle de 200 Hz |
| `AirWaterMedium` | transição suave entre ar e água com amortecimento, perda, turbidez e temperatura | família `FLUID + PHASE` do `IntegratedModuleBank` |
| `MotionGestureRecorder` da Lupa | grava XY real durante a posse e reproduz o percurso depois da soltura | família `GESTURE WAVES` do `IntegratedModuleBank` |

## Contratos e limites

### Wavefolder

O caminho `amount=0` permanece exato. A avaliação 4× é deliberadamente leve:
interpola a entrada entre amostras, calcula quatro estados não lineares e usa
uma média curta na volta à taxa original. O teste espectral dirigido confirma
redução de pelo menos 15% da energia medida na banda alta no caso agressivo de
7,3 kHz/drive 10. Isso não demonstra comportamento universalmente livre de
alias nem substitui uma futura comparação com meia-banda, ADAA ou polyBLAMP.

### Feedback do Master

O adaptador lê a redução de ganho real das três bandas e da cola, não a saída
já limitada. A atualização ocorre a 200 Hz; ataque e release impedem saltos, o
valor é limitado a 0,32 e `CREATIVE MASTER=0` leva o alvo a zero. O Cérebro usa
essa influência apenas para deslocar levemente quantidade e velocidade da
deriva. `reset()` e troca de seed limpam o estado, evitando transportar a
compressão de uma sessão para outra. O limiter técnico continua fora do loop.

### Meio ar/água

A altura normalizada do primeiro organismo autônomo define uma transição suave
em torno da linha d'água. Turbidez aumenta a densidade; temperatura desloca
discretamente o brilho; amortecimento e perda atuam apenas conforme cresce a
mistura de água. A rota no ar retorna o áudio direto. Nesta etapa, o meio atua
somente na família experimental `FLUID + PHASE`: regiões por organismo e a
promoção para a acústica canônica do Ecossistema continuam abertas.

### Memória da Lupa

Enquanto a Lupa está sob posse humana e `LAB MIX` está ativo, a posição XY
real é amostrada a 200 Hz em memória fixa de até 2.048 pontos, aproximadamente
10,24 segundos. Ao soltar, o trajeto entra em loop e contribui para `GESTURE
WAVES`; uma mutação máxima de 0,1% por volta evita repetição completamente
estática sem apagar o gesto. Com o laboratório desligado, o banco retorna
antes e não grava. A reprodução não automatiza `heldByObserver` nem substitui
o gesto humano da Lupa.

## Tempo real e superfície

- C++20 puro e estados de tamanho fixo;
- nenhuma alocação, lock, I/O ou parsing no callback;
- contexto da água e da Lupa atravessa a estrutura fixa já usada pelo banco;
- telemetria acrescenta `masterFeedback`, `mediumWaterMix` e
  `lensGestureActive`, sem criar texto ou componente no painel;
- os quinze controles de família e os 63 valores persistentes permanecem
  inalterados.

## Validação de 2026-10-02

- sintaxe estrita com `-Wall -Wextra -Wpedantic -Werror`: aprovada;
- `aquorbium_core_refinement_tests`: bypass/limites, comparação espectral,
  feedback e diferença ar/água aprovados;
- `aquorbium_integrated_module_bank_tests`: meio submerso e captura/reprodução
  real da Lupa observados;
- `aquorbium_realtime_tests`: aprovado com zero alocação no callback;
- `aquorbium_core_tests`: cenário completo comprovou feedback do Master maior
  que zero e menor ou igual a 0,32;
- regressões de módulos pendentes e ecologia avançada: aprovadas;
- compilação e testes dirigidos foram executados com um job.
- suíte Release integral: 45/45 testes aprovados em 93,05 s;
- benchmark Release com banco e Master ativos: 48 kHz/bloco 128/quatro vozes
  mediu 29,69% (3,37× offline); 96 kHz/bloco 128/quatro vozes mediu 56,41%
  (1,77× offline); o pior caso foi 62,08% em 96 kHz/bloco 128/zero vozes
  externas (1,61× offline).

Ainda faltam escuta musical, render espectral de referência, deadline/XRUN no
dispositivo, execução do app JUCE e promoção do meio ar/água para regiões
acústicas por organismo. O build de core/ferramentas foi validado sem JUCE.
Integração técnica não equivale a aprovação composicional.
