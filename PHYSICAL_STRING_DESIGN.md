# Aquorbium — dossiê do corpo Physical String / Waveguide

**Estado:** versão refinada integrada como quinto organismo  
**Decisão:** implementar um corpo próprio a partir do modelo matemático; não
copiar código de módulos, bibliotecas ou instrumentos externos.

## Questão musical

O banco atual contém onda contínua, pulsos, partículas filtradas e modos
ressonantes. Falta um corpo no qual a energia realmente circule e envelheça:
um gesto curto deve voltar sobre si mesmo, perder agudos a cada percurso e
revelar uma sensação de tensão, comprimento e ponto de contato.

O objetivo não é imitar violão. A corda é matéria ecológica: pode soar como
filamento, casco, nervo ou lâmina. Ela deve nascer de um evento do Bioma sem
MIDI, aceitar uma intervenção humana sem abandonar sua memória e permanecer
distinta do banco modal e do Striker.

## Fontes e conceitos apropriados

- Karplus e Strong, [*Digital Synthesis of Plucked-String and Drum
  Timbres*](https://www.moforte.com/wp-content/uploads/2020/05/Karplus-Strong-CMJ-1983.pdf)
  (Computer Music Journal, 1983): energia inicial em uma memória curta e
  recirculação filtrada. DOI `10.2307/3680062`.
- Karjalainen, Välimäki e Tolonen, [*Plucked-string models: from the
  Karplus-Strong algorithm to digital waveguides and beyond*](https://research.aalto.fi/en/publications/plucked-string-models-from-the-karplus-strong-algorithm-to-digita/)
  (1998): leitura
  do laço como guia de onda e separação entre excitação, propagação, perdas e
  corpo.
- Välimäki e Karjalainen, [*Implementation of Fractional Delay Waveguide Models
  Using Allpass Filters*](https://research.aalto.fi/en/publications/implementation-of-fractional-delay-waveguide-models-using-allpass)
  (1995): atraso fracionário allpass para afinação entre comprimentos inteiros.
- Bank e Välimäki, [*Robust Loss Filter Design for Digital Waveguide Synthesis
  of String Tones*](https://home.mit.bme.hu/~bank/publist/spl03.pdf)
  (IEEE SPL, 2003): o filtro de perdas governa os tempos de
  decaimento dos parciais e deve permanecer estável/mínimo-fase.

As fontes orientam as decisões, não fornecem implementação. O código do
Aquorbium será escrito do zero e testado contra invariantes próprios.

## Modelo inicial

O estado principal é uma linha circular. Para uma frequência `f0` e taxa
`fs`, o percurso desejado é:

`D = fs / f0` amostras.

O laço contém três elementos:

1. atraso inteiro `N`;
2. allpass de primeira ordem com atraso de baixa frequência `d`;
3. filtro FIR de perdas `L(z) = (1-b) + b z^-1`.

O FIR tem ganho unitário em DC, atenua progressivamente altas frequências e
acrescenta aproximadamente `b` amostra de atraso na região fundamental. A
configuração usa `N + d + b ~= D`, compensando esse atraso ao calcular a
afinação. Para `0 < d < 1`, o allpass usa coeficiente
`a = (1-d)/(1+d)`, sempre dentro do círculo unitário. Valores fracionários
quase nulos ignoram o estágio, evitando estado mal condicionado junto de
`a = 1`.

O ganho de volta é derivado de um alvo de -60 dB no tempo de decaimento:

`g = exp(log(0.001) / (decaySeconds * f0))`.

Ele é limitado estritamente abaixo de um. A atenuação espectral do FIR torna
os parciais superiores mais curtos sem exigir filtros ou coeficientes de ordem
alta no callback.

## Excitação e leitura

Um impacto inicia uma rajada determinística de ruído com duração próxima de um
percurso. `hardness` governa a abertura do filtro do excitador. `pluckPosition`
cria um cancelamento de dois pontos: o ruído atual é comparado a uma cópia
causal atrasada por uma fração do comprimento. Isso introduz vales espectrais
relacionados ao ponto de contato sem uma wavetable pré-calculada.

A rajada é gerada uma amostra por vez. Não se preenche a linha inteira dentro
do callback, portanto o custo do impacto não cresce abruptamente nas notas
graves. Depois da primeira escuta cega, o ruído foi percebido antes da corda.
A segunda iteração passou o excitador por dois polos relacionados a `hardness`,
aplicou uma janela Hann sem degrau durante um percurso e reduziu o caminho de
contato direto. A saída agora privilegia a energia que já circulou pela guia.

Uma saturação suave simétrica protege somente o estado de escrita quando
impactos se acumulam. Em níveis normais ela deve permanecer praticamente
linear; não é um efeito de distorção nem substitui o master limiter.

## Faixas e orçamento

- frequência nominal: 20–6000 Hz, limitada também pelo tamanho fixo da linha e
  por `0,22 * fs`;
- decaimento: 40 ms–12 s;
- brilho, dureza e posição: snapshots finitos e limitados;
- posição física: 0,04–0,50 do comprimento, pois a outra metade é simétrica;
- memória fixa: duas linhas de `16384` floats por corpo, sem heap no render;
- trabalho por amostra: uma leitura circular, dois filtros de primeira ordem,
  excitação opcional e uma barreira de energia;
- nenhuma alocação, lock, I/O ou mudança de tamanho depois de `prepare()`.

Em taxas acima de 192 kHz, a frequência mínima efetiva pode subir para caber
na memória fixa. A telemetria publicará a frequência realmente configurada; o
módulo nunca acessará memória fora do limite.

## Autonomia, performance e híbrido

`Parameters` descreve a intenção do Bioma. `PerformanceIntent` contém
`influence` e os mesmos eixos materiais. Na fronteira de um novo impacto:

`effective = lerp(generative, performance, influence)`.

A altura continua pertencendo ao evento ecológico/nota. Decaimento, brilho,
dureza e posição podem ser deslocados pelo performer. Influência zero deve ser
idêntica bit a bit ao modo autônomo. Uma mudança durante a cauda não retuna nem
reconfigura energia já circulando; ela entra somente no próximo impacto.

## O que fica fora deste marco

- retune/glissando de uma corda já excitada;
- dispersão de corda rígida de ordem superior;
- arco, martelo não linear e acoplamento entre cordas;
- modelo de caixa acústica ou convolução;
- entrada de áudio externa no laço;
- macros performáticas dedicadas na GUI.

Essas extensões só entram se o corpo simples demonstrar identidade. Allpass
variável no meio de uma cauda exige atualização de estado específica para não
criar transientes; não será disfarçado como um slider prematuro.

## Portões do protótipo

1. silêncio exato antes da excitação e cauda que termina sem estado não finito;
2. determinismo por seed e diferença verificável entre seeds;
3. afinação medida em 48 e 96 kHz, inclusive frequências não inteiras;
4. polos/ganhos estritamente estáveis em todos os extremos;
5. impacto repetido com saída limitada e sem salto destrutivo;
6. intenção performática zero bit a bit e parâmetros congelados durante a
   cauda;
7. zero alocações no render e benchmark de pior caso;
8. renders reproduzíveis de ao menos três matérias para escuta;
9. integração ecológica somente após os oito itens anteriores e aprovação por
   escuta.

## Perguntas de escuta

- o retorno de energia soa como um corpo que lembra, ou apenas como um comb
  filter tonal?
- brilho e dureza são gestos diferentes quando a cauda já se estabeleceu?
- a posição produz identidade útil sem buracos de nível excessivos?
- os extremos cobrem filamento, casco e lâmina sem convergir ao Striker?
- em diálogo com os outros seres, a corda ocupa uma função nova ou duplica o
  ressonador modal?

## Evidência técnica do primeiro protótipo

Em Release, a suíte passou 13/13 testes. O teste dedicado cobre afinação em 48
e 96 kHz nas fundamentais 82,41, 173,37 e 441,17 Hz, seed, silêncio/cauda,
extremos, parâmetros não finitos, material e congelamento da intenção. O teste
global de tempo real inclui impactos e snapshots do módulo com zero alocações.

O benchmark deliberadamente extremo usa cinco corpos e um novo impacto a cada
128 amostras por corpo. A mediana entre três rodadas — cada uma já sendo a
mediana de cinco janelas — foi 0,65% em 48 kHz e 1,28% em 96 kHz, equivalentes
a 153,46× e 77,95× tempo real. Cada instância ocupa 131.280 bytes nesta ABI;
a memória, não a CPU, é o principal limite de multiplicação.

Os renders de 12 segundos usam a mesma seed e oito eventos:

| Caráter | RMS | Pico | DC | razão de diferença HF |
|---|---:|---:|---:|---:|
| `filament` | 0,000447 | 0,01299 | -8,74e-9 | 0,0485 |
| `shell` | 0,000788 | 0,03712 | -1,42e-10 | 0,4350 |
| `blade` | 0,002421 | 0,05970 | -6,82e-8 | 0,9057 |

As diferenças espectrais e dinâmicas são fortes e reproduzíveis. Isso prova
contraste técnico, não superioridade musical. A primeira escuta apontou excesso
de ruído no contato; a segunda reconheceu sofisticação na versão refinada e
determinou sua entrada como quinto organismo, sem substituir os outros corpos.

Depois da primeira escuta, a versão refinada preservou os três extremos e
reduziu o ataque estocástico frontal:

| Caráter refinado | RMS | Pico | DC | razão de diferença HF |
|---|---:|---:|---:|---:|
| `filament` | 0,000315 | 0,00333 | -8,60e-10 | 0,00485 |
| `shell` | 0,000806 | 0,01742 | -9,26e-11 | 0,14787 |
| `blade` | 0,002416 | 0,02882 | -4,22e-8 | 0,52053 |

Os primeiros arquivos permanecem arquivados como evidência da decisão; os
novos renders estão em `aquorbium-string-characters-refined`.

A comparação cega direta com Modal e Striker está documentada em
[`PHYSICAL_BODIES_COMPARISON.md`](PHYSICAL_BODIES_COMPARISON.md). Ela fixou
também uma restrição de integração: o buffer de 131.280 bytes não é duplicado
dentro de todos os despachantes. O banco específico contém apenas um corpo por
banco autônomo/voz externa: cinco instâncias, 656.400 bytes nesta ABI.
`OrganismVoiceEngine` publica a identidade tipada, enquanto o estado grande da
guia pertence ao `BiomaEngine`. Assim a formação tem 25 despachantes compactos,
mas não 25 linhas de atraso.
