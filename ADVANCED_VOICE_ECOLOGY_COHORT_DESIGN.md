# Aquorbium — coorte avançada de voz e ecologia

Atualização: 2026-09-28.

## Objetivo

Esta segunda coorte codifica dez mecanismos ainda ausentes, mas já descritos
na pesquisa local. Eles entram diretamente no `IntegratedModuleBank`, sem
novos controles ou estado persistente. O objetivo é produzir matéria audível
para comparação antes de promover qualquer mecanismo ao BiomaBass definitivo,
ao `Ecosystem`, a `SchoolInfo` ou a um `Organism` específico.

## Componentes e destino experimental

| Componente | Função | Família visível |
|---|---|---|
| `BiomaBassVoice` | grave com FM thru-zero, detune orgânico, sub intervalar, wavefold pré-filtro, 2–4 polos e autorregulação por energia | `FM + HARMONICS` |
| `SchoolWaveVoice` | seis membros derivados de uma fase/frequência comum, wave morph e wave cut | `FM + HARMONICS` |
| `HarmonicRippleVoice` | pulsos e ondulações harmonicamente travados, com cluster formântico | `PERC + PULSE` |
| `RecentMixWavetable` | captura incremental do mix recente e reprodução como wavetable | `GRANULAR CAPTURE` |
| `FrequencyShifter` | sinal analítico por Hilbert FIR fixa e deslocamento linear em Hz | `FRACTAL + VECTOR` |
| `CrossFeedbackStateMachine` | energia do próprio áudio escolhe estado com histerese e publica modulação limitada | `EVOLVE + STAGES` |
| `SpotlightMixer` | foco automático que preserva e redistribui o fundo em estéreo | `BRANCH + PERMUTE` |
| `HungerProcessor` | densidade reduz alimentação disponível e introduz falhas limitadas | `INTEGRITY + MEMORY` |
| `BoundedSelfFeedback` | auto-feedback interno limitado, filtrado e saturado | `LORENZ + DRIFT` |
| `WaterInertia` | inércia assimétrica e limitada para movimento em meio aquático | `FLUID + PHASE` |

O auto-capture já existente em `BiomaGranularCapture` também foi ativado no
banco. Os disparos explícitos variam duração, velocidade, pan, ganho e reversão
por evento; derivar esses valores dos traços de cada `Organism` continua aberto.

## Contratos

- C++ puro, sem JUCE, I/O, lock ou alocação no processamento;
- memória fixa e laços de tamanho máximo conhecido;
- toda frequência respeita a taxa preparada;
- feedback, ressonância, starve e fome permanecem limitados;
- mudanças de filtro e material pesado acontecem em `prepare()` ou evento;
- entradas não finitas são recusadas ou substituídas pelo último valor válido;
- `LAB MIX=0` continua removendo toda a coorte da rota por amostra;
- implementação experimental não equivale à ligação ecológica final.

## Validação técnica

- teste dedicado dos dez componentes aprovado;
- banco integrado e teste de tempo real aprovados, incluindo as quinze famílias
  ativas e zero alocações no callback;
- suíte completa: 43/43 testes em 91,09 s;
- benchmark Release em 48 kHz/bloco 128/quatro vozes: 17,70% da janela,
  margem de 5,65× tempo real;
- pior ponto ativo observado: 35,78% em 96 kHz/bloco 64/quatro vozes, margem
  de 2,80× tempo real offline;
- `LAB MIX=0` retorna antes de processar as duas coortes;
- binário Release: 14.189.888 bytes, SHA-256
  `6f7a269d5cd68ad08889f0dc62bad1bc6b66e15e87012c4aad046e0ea00688a0`.

Essas medições não substituem deadline do dispositivo, contadores XRUN/LATE,
sessão prolongada nem escuta humana. Sub/quantizador, saída final/sequenciador,
parâmetros por `Organism`, `SchoolInfo`, `Ecosystem` e promoção por espécie
permanecem abertos.
