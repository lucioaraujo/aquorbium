# Aquorbium — a música que vem do ecossistema

Criado em 2026-10-20, depois da escuta do autor: "musicalmente está muito podre
ainda, não há acontecimentos composicionais generativos interessantes e nem
variação"; "o aquorbium é monolítico"; "parece um passa tempo"; "falta
tempero". O autor também deu as diretrizes:

- "a base dos acontecimentos são os conceitos do instrumento, pautado no
  ecossistema, a música vem desses conceitos";
- "estabelecermos mais relações com conceitos musicais e composicionais";
- "se aprofundando nos códigos, nas literaturas, nas ações artísticas e
  musicais que trabalham o tema, nas pesquisas também";
- "os instrumentos rasgo são essencialmente potentes e ofertam muitas
  possibilidades composicionais".

Este documento é **hipótese de concepção**, para o autor revisar e ampliar ("se
for o caso precisaremos desenvolver mais os conceitos"). A escuta decide.

## 1. O diagnóstico (medido, não suposto)

Num render autônomo de 180 s (a mesma configuração do
`aquorbium_autonomous_render`), divididos em seis janelas de 30 s, o
Aquorbium tocou:

| Medida | Valor em cada janela de 30 s |
|---|---|
| ataques | 38 a 47, sem arco |
| registro | ~32–70 MIDI, o mesmo |
| classes de altura | 10 a 12 (entropia 3,1–3,5 bits, quase uniforme) |
| organismos soando | ~2 de 5 |
| fundamentais | 4 a 5 trocas por janela |

Cada organismo tocava uma nota a cada 3–4 s, num ritmo quase regular,
passeando por 16 a 21 alturas e sem repetir nada. O retrato é o de um sino de
vento:

- sem motivo e sem centro;
- sem arco e sem papéis;
- os cinco organismos fazendo a mesma coisa.

A causa está no código: cada organismo decidia "toco ou não" por uma
probabilidade genérica (`AutonomousEnergy::makeDecision`), a mesma para
todos. A riqueza ecológica do ecossistema (predador, presa, território,
migração, cardume, acasalamento, integridade, percepção) não chegava às notas.

## 2. O princípio

Os acontecimentos musicais nascem das **interações que os organismos
percebem**, segundo o contrato do Ecossistema (`ECOSYSTEM_DESIGN.md` §2):

- **invariante 1:** nenhuma entidade possui visão total;
- **invariante 2:** uma reação só pode usar o que a entidade observou;
- **invariante 3:** o desejo altera tendências, mas não escreve acontecimentos;
- **invariante 6:** primeiro plano e sustentação são papéis explícitos.

Por isso, o diretor consulta `Ecosystem::perceivedBy(observador, alvo)`, a
mesma percepção subaquática que move as estratégias: campo de visão,
distância, turbidez e presença. Não há outra física. O desejo (densidade,
calma, perigo) só adensa ou rarefaz o que já acontece.

## 3. Conceito ecológico → conceito musical → referência

| Ecossistema | Música e composição | Referências |
|---|---|---|
| **Nicho acústico**: cada espécie ocupa a sua faixa de frequência e de tempo para não ser mascarada | **orquestração por nichos**: cada organismo tem o seu registro e o seu lugar no tempo; quando dois coincidem, o de sustentação se desloca | Krause, *niche hypothesis* (1993) e *The Great Animal Orchestra* (2012) |
| **Predador vê a presa e caça** | **imitação em cânone que se fecha em stretto**: o predador repete as células da presa, com um atraso igual à distância; na captura, uníssono e golpe | o stretto da fuga; Blackwell, *Swarm Music* (partículas que se atraem em espaço musical) |
| **Presa vê o predador e foge** | **diminuição e síncope**: a célula se adensa, desloca os acentos e salta para longe do registro do caçador | Lotka–Volterra como dinâmica de perseguição; Fontana, síntese interativa por Lotka–Volterra |
| **Migratório percorre a esfera** | **modulação**: o migrante carrega o centro harmônico; cruzar uma região é mudar de tonalidade (por quintas) | Schafer: a *keynote* que pode modular |
| **Séssil + territorial** (âncora, memória) | **keynote e pedal**: o som de fundo que dá o centro, "ouvido sem ser escutado"; **memória**: o motivo do predador volta em aumentação, mais grave | Schafer, *The Tuning of the World* (1977): *keynote*, *signal* e *soundmark* |
| **Territorial vê um intruso** | **sinal** (*signal*): golpes em contratempo, um alarme | Schafer: o sinal de aviso |
| **Cardume coeso / disperso** | **homorritmia / heterofonia**: coeso, segue os ataques do líder; disperso, cada um no seu tempo | a heterofonia como textura; a micropolifonia como massa |
| **Acasalamento** (dois organismos se percebem) | **dueto em terças**, antifonia | o canto de corte como dueto; Rothenberg, a música entre espécies |
| **Rachadura do vidro** (causa identificável) | **ruptura**: silêncio, depois estilhaços (pontilhismo) | Di Scipio: o ruído e a perturbação no sistema acoplado |
| **Entrada e saída de organismos** (presença) | **entradas e saídas de vozes**: o arranjo vem da presença, não de um arco imposto | Waters, *performance ecosystems*: agência entre corpos e ambiente |
| **Cada espécie com o seu canto** | **soundmark** e **motivo**: a identidade que se repete e se transforma (transposição, inversão, fragmento) | Messiaen, *Catalogue d'oiseaux* (1956–58): o canto de cada espécie como material composto |
| **Dia, noite, estação** | **cor modal e andamento**: a luz escolhe o modo (do frígio ao lídio) e puxa o andamento | John Luther Adams, *The Place Where You Go to Listen* (2006): fenômenos naturais com vozes próprias |
| **O ecossistema que se auto-organiza com o ambiente** | **sistema acoplado**: o som realimenta o processo (no Aquorbium, a saída já volta ao sequenciador) | Di Scipio, *Audible Ecosystemics*; McCormack, *Eden* (2001) |
| **A massa de muitos indivíduos** | **massa estatística**, nuvem granular | Xenakis, *Pithoprakta* (1955–56) e *Formalized Music*; Miranda, autômatos celulares (CAMUS, Chaosynth) |
| **Cantores e críticos que coevoluem** | **avaliação interna**: o que soa e o que se escolhe guardar (futuro) | Todd e Werner, *Frankensteinian methods* (1999) |
| **Ouvir o ecossistema inteiro debaixo d'água** | o aquário como **escuta subaquática**: camadas de profundidade, velocidades diferentes do som | Jana Winderen, as gravações com hidrofones; Annea Lockwood |

## 4. O que está implementado (2026-10-20)

`src/core/EcologicalMusicDirector.h/.cpp` faz três coisas:

- **atribui os papéis pela estratégia principal** de cada organismo, e não
  pelo índice;
- **mantém um relógio de semicolcheias**, com o andamento como tendência da
  luz e da densidade;
- **mantém o centro harmônico do migrante, o modo da luz e uma progressão por
  compasso.**

| Papel | No código |
|---|---|
| predador | motivo de 4 a 7 notas; repete ou transforma conforme MEMÓRIA; caçando, faz cânone nas células da presa com atraso = distância; captura em uníssono |
| presa | célula euclidiana; fugindo, de 7 a 11 pulsos, rotação a cada compasso e saltos de oitava e de quinta |
| migrante | notas do acorde nos tempos (ou nos ataques do líder, se o cardume estiver coeso); anuncia a nova região |
| séssil | pedal na fundamental (keynote); a cada 6–12 compassos, a memória do motivo do predador, duas vezes mais lenta e mais grave |
| territorial | sinais em contratempo quando vê um intruso; estilhaços na captura e na ruptura |
| par do acasalamento | o segundo canta uma terça abaixo |

**Os controles** (`Performance`):

- **influência**: zero é a rota anterior, intocada;
- **reação**: o quanto o organismo reage ao que percebe;
- **memória**: repetir ou transformar.

Determinístico por seed, sem alocação, trava nem I/O.

## 5. Próximos portões

1. **A escuta do autor:** os renders antes e depois e o app.
2. **O cartão no app:** INFLUÊNCIA, REAÇÃO e MEMÓRIA, e botões para provocar
   os atos (soltar o predador, chamar o acasalamento, rachar o vidro). É a
   potência para quem toca.
3. **Ampliar os conceitos com o autor:** por exemplo, a coevolução (o que os
   organismos "aprendem" a cantar), as camadas de profundidade de Winderen e
   a saúde do habitat como densidade de nichos, segundo Krause.
4. **Escolher o timbre de cada papel:** junto da escuta.

## Referências

- Krause, B. (1993). The niche hypothesis: a virtual symphony of animal sounds,
  the origins of musical expression and the health of habitats. *The
  Soundscape Newsletter* (World Forum for Acoustic Ecology), 6. Também:
  *The Great Animal Orchestra*, Little, Brown, 2012.
- Schafer, R. M. (1977). *The Tuning of the World*. Knopf (reed. *The
  Soundscape*, Destiny Books, 1994).
- Di Scipio, A. (2003). 'Sound is the interface': from interactive to
  ecosystemic signal processing. *Organised Sound* 8(3), 269–277.
- McCormack, J. (2001). Eden: an evolutionary sonic ecosystem. *ECAL 2001*,
  LNCS 2159, Springer, 133–142.
- Blackwell, T., & Bentley, P. (2002). Improvised music with swarms. *Proc.
  IEEE CEC 2002*. Também: Blackwell, T. (2007), Swarming and music, in
  Miranda & Biles (orgs.), *Evolutionary Computer Music*, Springer.
- Fontana, F. (2012). Interactive sound synthesis by the Lotka–Volterra
  population model. *Proc. Colloquium on Music Informatics (XIX CIM)*.
- Waters, S. (2007). Performance ecosystems: ecological approaches to
  musical interaction. *EMS-07 Proceedings*, Leicester.
- Keller, D., & Lazzarini, V. (2017). Ecologically grounded creative practices
  in ubiquitous music. *Organised Sound* 22(1), 61–72.
- Todd, P. M., & Werner, G. M. (1999). Frankensteinian methods for
  evolutionary music composition. In Griffith & Todd (orgs.), *Musical
  Networks*, MIT Press.
- Miranda, E. R. (2007). Cellular automata music: from sound synthesis to
  musical forms. In Miranda & Biles (orgs.), *Evolutionary Computer Music*,
  Springer.
- Messiaen, O. *Catalogue d'oiseaux* (1956–58).
- Xenakis, I. *Pithoprakta* (1955–56); *Formalized Music* (1971; ed. rev.
  Pendragon, 1992).
- Adams, J. L. *The Place Where You Go to Listen* (2006), instalação, Museum
  of the North, Fairbanks; livro homônimo, Wesleyan, 2009.
- Winderen, J. Gravações com hidrofones desde 2005 (por exemplo, *Energy
  Field*, Touch, 2010; *bára*, TBA21, 2017).
- Rothenberg, D. *Thousand Mile Song* (2008).

Nenhum código dessas obras foi copiado. As ideias são reescritas para os
conceitos do Aquorbium, como manda a regra do RASGO.

## 6. Segunda versão: a teia de relações, as incompatibilidades, a densidade e os estados líquidos (2026-10-20)

Pedidos do autor: "teia de relações, de ataques, fugas"; "harmonia,
incompatibilidades"; "densidade"; "estados líquidos"; "os sons que se propagam
pelos líquidos"; "frequências líquidas"; "algo intenso, bem pensado e codado,
com detalhes refinados".

O diretor `src/core/EcologicalMusicDirector.*` foi reescrito em cinco camadas:

1. **O meio, ou os estados líquidos.** A água passa por gelo, fria,
   temperada, turbulenta e fervura. O calor é a temperatura da estação mais a
   agitação dos organismos, e o estado só muda depois de pago o calor latente
   (histerese). Cada estado define:
   - a viscosidade (portamento);
   - a turbulência (rubato, heterofonia, fragmentação);
   - a cristalinidade (rede pentatônica e grade par);
   - a velocidade do som, a absorção, as bolhas e a capacidade do nicho.
2. **A propagação.** Cada nota é uma frente de onda. O organismo a ouve
   quando ela chega, atenuada por espalhamento e por uma absorção que cresce
   com f², e com o desvio Doppler. A imitação do caçador é o que ele OUVIU.
   Os cliques de ecolocalização esperam o eco: o buzz terminal emerge do
   tempo de ida e volta.
3. **A teia de relações.** Uma matriz 5×5 com caça, fuga, rivalidade
   (hoquetus), aliança (homorritmia/heterofonia e cópia de canto), corte
   (cópia para endereçar) e guarda (sinais). A intensidade cresce rápido e
   decai devagar.
4. **Harmonia e incompatibilidade.** Os pares compatíveis puxam a
   consonância e os incompatíveis, a fricção. A tensão acumulada leva à
   dominante e abre o cromático; dissolvido o conflito, vem a cadência
   (resolução). O migrante só modula em água calma (16 compassos de
   intervalo, no mínimo).
5. **Densidade.** A capacidade do nicho cresce logisticamente até o K do meio
   e do desejo e é repartida por saliência. O efeito Lombard eleva o acento.
   Há ainda as bolhas de Minnaert (fervura), a cripse acústica (a presa
   congela) e a decomposição (quem parte ou é capturado vira memória da
   âncora).

**Medição** (diagnóstico de 300 s, influência 1):
- a forma foi temperada → turbulenta → fervura → turbulenta → temperada →
  fria → gelo (~60 s) → fria;
- houve ~90 eventos/30 s nos estados quentes e ~36 no gelo;
- no gelo e na água fria, 7–8 classes de altura (entropia 2,5–2,7);
- os atos: caça 10, buzz 3, captura 6, fuga 10, dueto 5, defesa 9,
  modulação 4, resolução 3, mudança de estado 8, bolhas 16.

Também mudou nesta etapa:
- `directedNote` recebe o glide do meio;
- nova ferramenta `tools/EcologicalMusicRender.cpp`
  (`aquorbium_ecological_music_render`): uma estação de 300 s antes/depois,
  com o roteiro de escuta.

**Validação (2026-10-20):**
- `tests/EcologicalMusicDirectorTests.cpp` cobre:
  - a consonância, a compatibilidade, Minnaert (bolha de 3 mm ≈ 1,09 kHz),
    a absorção e a tabela dos meios;
  - a influência 0 sem nenhuma nota e o determinismo por seed;
  - uma estação de 300 s, com 5 estados alcançados, as notas no gelo
    cristalizadas, nenhum compasso acima da capacidade e todas as notas na
    faixa.
- A suíte completa passou: 50/50.
- O render está em `validation-output/ecological-music-2026-10-20/`
  (fora do git): pico 0,22 depois e 0,19 antes, sem clipping, com um roteiro
  de escuta por segundo.
- A aprovação musical não foi avaliada: depende da escuta do autor.

**Ponto de retomada:**
1. A escuta do autor, com o `depois_musica_ecologica_ROTEIRO_DE_ESCUTA.txt`
   ao lado do WAV.
2. ~~O cartão no app~~ feito em 2026-10-20. No cartão 02 GEOMETRIA, a
   última linha tem quatro controles:
   - **MÚSICA** (influência; zero = rota anterior);
   - **REAÇÃO**;
   - **MEMÓRIA**;
   - **ÁGUA** (viés de calor: o centro segue a estação, a esquerda leva ao
     gelo, a direita à fervura; a mudança respeita o calor latente).

   Com a MÚSICA acima de zero, o status do cartão mostra "ÁGUA <estado> ·
   <ato> <tensão>%".
3. ~~Ligar a absorção ao timbre~~ feito em 2026-10-20. No `BiomaEngine`,
   cada organismo da rota autônoma passa por um passa-baixa cujo corte é o
   −3 dB da absorção do meio ao longo do caminho na água até o ouvinte
   (`EcologicalMusicDirector::liquidCutoffHz`, `Ecosystem::waterPathToListener`).
   - A fervura soa opaca e o gelo, límpido.
   - Quem nada longe do vidro soa mais escuro.
   - Medição e render em `aquorbium-tarefas.md`.
4. ~~A rede de bolhas e a revolução de canto~~ feitas em 2026-10-20.
   - **Rede de bolhas** (a caça cooperativa das jubartes, com papéis
     divididos): quando o organismo que um aliado segue caça, e o aliado
     também percebe a presa, o aliado faz uma espiral ascendente em terças
     sobre duas oitavas, repetida. O caçador troca os cliques por um chamado
     de alimentação, forte, a cada tempo.
   - **Revolução de canto** (Garland et al. 2011): de vez em quando (mais
     com MEMÓRIA), ou quando um organismo entra, ele inventa um canto novo.
     Quem o ouve adota o canto inteiro em poucas frases, não unidade por
     unidade.
   - Numa estação com MEMÓRIA 1, houve 11 redes de bolhas e 9 eventos de
     revolução (testado). No app, aparecem no registro do painel.

### Referências da segunda versão

- Au, W. W. L. (1993). *The Sonar of Dolphins*. Springer. (O intervalo entre
  cliques excede o tempo de ida e volta do eco.)
- Miller, P. J. O., Johnson, M. P., & Tyack, P. L. (2004). Sperm whale
  behaviour indicates the use of echolocation click buzzes 'creaks' in prey
  capture. *Proc. R. Soc. B* 271, 2239–2247.
- Sharma, P. et al. (2024). Contextual and combinatorial structure in sperm
  whale vocalisations. *Nature Communications* 15, 3617. (Ritmo, andamento,
  rubato e ornamento nas codas.)
- Janik, V. M., & Sayigh, L. S. (2013). Communication in bottlenose dolphins:
  50 years of signature whistle research. *J. Comp. Physiol. A* 199, 479–489.
- King, S. L., & Janik, V. M. (2013). Bottlenose dolphins can use learned
  vocal labels to address each other. *PNAS* 110(32), 13216–13221.
- Garland, E. C. et al. (2011). Dynamic horizontal cultural transmission of
  humpback whale song at the ocean basin scale. *Current Biology* 21(8),
  687–691.
- Holt, M. M. et al. (2009). Speaking up: killer whales (*Orcinus orca*)
  increase their call amplitude in response to vessel noise. *JASA* 125(1),
  EL27–EL32.
- Aguilar de Soto, N. et al. (2020). Fear of killer whales drives extreme
  synchrony in deep diving beaked whales. *Scientific Reports* 10.
  doi:10.1038/s41598-019-55911-3.
- Minnaert, M. (1933). On musical air-bubbles and the sounds of running
  water. *Philosophical Magazine* 16, 235–248.
- Wood, A. B. (1930). *A Textbook of Sound*. Bell. (A velocidade do som num
  líquido com bolhas.)
- Plomp, R., & Levelt, W. J. M. (1965). Tonal consonance and critical
  bandwidth. *JASA* 38, 548–560.
- Tenney, J. (1988). *A History of 'Consonance' and 'Dissonance'*. Excelsior.
- Kolmogorov, A. N. (1941). The local structure of turbulence in
  incompressible viscous fluid for very large Reynolds numbers. *Dokl. Akad.
  Nauk SSSR* 30. (A cascata, usada aqui como figura de fragmentação.)

Nenhum código dessas obras foi copiado; as relações físicas usadas (Minnaert,
absorção ∝ f², Doppler, tempo de ida e volta) são reescritas a partir da
teoria e exageradas na escala musical onde indicado no código.
