# Aquorbium — verificação dos conceitos e aprofundamento para os módulos

Criado em 2026-10-20. O autor pediu duas coisas:

- "preciso que verifique todos os conceitos que estamos usando, que constam
  no 'sobre' e se aprofunde, para o desenvolvimento dos módulos";
- "verifique nos projetos livres (github, atlas, pesquisas em
  universidades), o que ainda não exploramos que possui potencial para o
  aquorbium".

**Método.** Cada conceito passa por cinco perguntas:

1. **A fonte:** a citação está correta? O que o trabalho diz de fato?
2. **No Aquorbium:** onde e como o conceito foi usado.
3. **A fidelidade:** fiel, simplificado (com a simplificação dita) ou errado.
4. **O aprofundamento:** o que a fonte e a literatura vizinha oferecem além
   disso.
5. **Os módulos:** o que propor de concreto, com o critério de teste.

As fontes foram conferidas na web em 2026-10-20 (os links estão no fim).
Nada foi copiado: as ideias se reescrevem para os conceitos do Aquorbium.

---

## 1. Resumo da verificação

| Conceito (SOBRE / código) | Fonte conferida | No Aquorbium | Fidelidade |
|---|---|---|---|
| Nicho acústico | Krause 1993, *The Soundscape Newsletter* 6:6–10 | o corpo de apoio sai uma oitava do registro alheio | **simplificado**: só o espectro, sem a partição no tempo |
| Keynote, sinal, soundmark | Schafer 1977, *The Tuning of the World* | o pedal da âncora é a keynote; a guarda faz os sinais | **parcial**: falta o *soundmark* e o par hi-fi/lo-fi |
| Ecossistêmica audível | Di Scipio 2003, *Organised Sound* 8(3):269–277 | a realimentação da saída para o ecossistema | **parcial**: sem acoplamento com o ambiente real (microfone) |
| Eden | McCormack 2001, ECAL, LNCS 2159:133–142 | inspiração geral | **não implementado**: Eden tem recursos, reprodução e evolução |
| Música de enxame | Blackwell & Bentley 2002, CEC, 1462–1467 | o cardume em espaço 3D | **parcial**: falta a atração a alvos no espaço musical |
| Lotka–Volterra | Fontana 2012, XIX CIM, 24–29 | só a capacidade logística do nicho | **simplificado**: não há dinâmica predador-presa de populações |
| Transmissão do canto | Garland et al. 2011, *Current Biology* 21(8):687–691 | aliados copiam unidades | **fiel** no espírito (transmissão horizontal) |
| Revolução de canto | **Noad et al. 2000, *Nature* 408:537** (faltava no SOBRE; acrescentado) | um organismo inventa ou chega com um canto novo, e quem ouve o adota inteiro | **fiel** à chegada de cantores de fora; a invenção espontânea é licença nossa |
| Hierarquia do canto | **Payne & McVay 1971, *Science* 173:585–597** (não citado) | não existe: os cantos são uma frase só | **ausente**: unidade, frase, tema e canto |
| Codas | Sharma et al. 2024, *Nature Communications* 15 | ritmo, rubato, ornamento, rubato que se iguala no dueto | **fiel**, parcial: falta o "alfabeto" combinatório (ritmo × andamento × rubato × ornamento) e a troca de turnos |
| Assobio-assinatura | Janik & Sayigh 2013, *J Comp Physiol A* 199:479–489 | cada organismo tem uma assinatura | **fiel** |
| Cópia para chamar | King & Janik 2013, *PNAS* 110(32):13216–13221 | no corte, quem responde canta a assinatura de quem chamou | **inexato**: no estudo, o animal endereçado responde com a **própria** assinatura |
| Buzz terminal | Miller, Johnson & Tyack 2004, *Proc R Soc B* 271:2239–2247 | cliques que aceleram até a captura | **fiel**: cliques regulares de 0,5 a 2 s na busca, *creak* na captura |
| Intervalo entre cliques | Au 1993, *The Sonar of Dolphins* | o clique seguinte espera o eco | **fiel**: ICI maior que o tempo de ida e volta mais a latência |
| Efeito Lombard | Holt et al. 2009, *JASA* 125(1):EL27 | o ruído ouvido aumenta o acento | **simplificado**: o estudo mede +1 dB por dB de ruído; o nosso não é em dB |
| Cripse acústica | Aguilar de Soto et al. 2020, *Sci Rep* 10, doi:10.1038/s41598-019-55911-3 | a presa esperta se cala | **parcial**: o estudo mostra sincronia de grupo (98% de sobreposição vocal) e subida silenciosa em direção imprevisível |
| Rede de bolhas | literatura de jubartes do Alasca; papéis: soprar a rede, conduzir, vocalizar ("feeding call", 400–600 Hz) | o aliado faz a espiral e o caçador chama | **fiel** nos papéis; falta a "parede de som" com o centro mais quieto |
| Bolha de Minnaert | Minnaert 1933, *Phil Mag* 16:235–248 | f ≈ 3,26/R | **fiel**: √(3γP/ρ)/2π ≈ 3,28 m/s para o ar a 1 atm |
| Gotas e altura que sobe | van den Doel 2005, *ACM TAP* 2(4):534–546 | as gotas sobem a altura enquanto decaem | **fiel**, simplificado: a subida existe quando a bolha nasce perto da superfície |
| Som numa água com bolhas | Wood (equação de Wood) | a fervura deixa o som lento e opaco | **fiel** na direção: 0,3% de ar já leva c a cerca de 200 m/s |
| Absorção | — | absorção ∝ f² | **simplificado**: no mar, na faixa audível, dominam as relaxações do ácido bórico (< ~3 kHz) e do MgSO₄; elas dependem da temperatura e do pH (Ainslie & McColm 1998) |
| Velocidade do som | Mackenzie 1981 (a fórmula de nove termos) | gelo mais rápido que a água, água fria mais lenta | **fiel** na direção: +~4,5 m/s por °C; o gelo ~3700–3900 m/s |
| Consonância | Plomp & Levelt 1965, *JASA* 38:548–560; Tenney 1988 | uma tabela por classe de intervalo | **simplificado**: a dissonância sensorial depende da banda crítica (máxima perto de 1/4 dela), do **registro** e dos **parciais** do timbre |
| Vogais | Peterson & Barney 1952, *JASA* 24:175–184 | os formantes da voz e do coro | **fiel**; corrigido: as vogais "o" e "e" são [ɔ] e [ɛ] |
| Membrana | Rayleigh, *The Theory of Sound* | seis modos | **corrigido**: as razões vêm dos zeros das funções de Bessel J0–J3, não só de J0 |
| Vidro friccionado | Rossing; Jundt et al. 2006 | quatro parciais fixos | **simplificado**: as razões mudam de copo para copo (1:2,37; 1:2,52), e o nível de água no copo abaixa a altura |
| Massa estatística | Xenakis, *Pithoprakta* (1955–56) | só como inspiração | **não implementado**: Xenakis usa a distribuição de velocidades de Maxwell–Boltzmann para os glissandos |
| Catálogo de espécies | Messiaen, *Catalogue d'oiseaux* | uma assinatura por espécie | **fiel** como método |
| Fenômenos naturais com voz | J. L. Adams, *The Place Where You Go to Listen* (2006) | o dia e a estação movem a música | **fiel** como método |
| Ecossistemas de performance | Waters 2007, EMS-07; Keller & Lazzarini 2017, *Organised Sound* 22(1):61–72 | o músico toca o ambiente | **fiel** como postura |

Correções já aplicadas nesta data:
- as vogais [ɔ] e [ɛ];
- as funções de Bessel Jₙ da membrana;
- a nota sobre as razões do vidro;
- o DOI de Aguilar de Soto;
- Noad et al. (2000) no SOBRE, nas quatro línguas.

---

## 2. O que cada conceito ainda oferece: aprofundamento e módulos

### 2.1 O nicho acústico também é temporal (Krause; Aihara)

Krause descreve a partição em **frequência e em tempo**: num habitat
maduro, cada espécie ocupa sua banda e **desloca seu tempo** para não ser
mascarada. Nós só fazemos a partição espectral.

As rãs japonesas dão o modelo do tempo. Aihara et al. modelam o coro como
**osciladores de fase acoplados**:
- duas rãs se alternam em **antifase**;
- três, em **tri-fase**;
- vizinhas formam **dois aglomerados em antifase**.

**Módulo `ChorusPhase`:** cada organismo ganha um oscilador de fase (o seu
"relógio de canto"). O acoplamento depende de quanto ele ouve os outros: é
a propagação que já temos.
- Competidores repelem a fase (alternância, hoquetus emergente).
- Aliados a atraem (sincronia).
- Em vez de regras, o hoquetus e a homorritmia **emergem**.
- Teste: duas presas que se ouvem convergem para defasagem ≈ π; três, para
  2π/3.

**Módulo `HabitatHealth`:**
- mede a ocupação espectro-temporal do som do próprio aquário, com índices
  como ACI e NDSI (scikit-maad, BSD-3, como referência de método);
- um ecossistema "saudável" deixa bandas livres, um degradado se
  sobrepõe — essa saúde do habitat pode virar um parâmetro de forma.

**Feito em 2026-10-21 (`HabitatHealth`, `src/core/HabitatHealth.*`):**

- **O que se mede:** o próprio som do aquário (depois do `OutputStage`),
  só enquanto a música ecológica toca, com uma FFT de 4096 pontos a cada
  85 ms em arrays fixos (~45 µs por quadro).
- **Os índices**, reescritos dos artigos (nada copiado do scikit-maad, que
  é só a referência de método):
  - **ADI** (Villanueva-Rivera et al. 2011): a entropia de Shannon da
    ocupação de 10 bandas logarítmicas de 50 Hz a 12,8 kHz;
  - **ACI** (Pieretti et al. 2011): a variação de intensidade entre
    quadros sobre a intensidade, por banda que soa, em janelas de 5 s.
- **O limiar fica 30 dB abaixo da banda mais forte da janela.**
  - Relativo, como o soundecology faz ao normalizar o espectrograma: um
    recife quieto não é um recife vazio.
  - O limiar absoluto (−50 dBFS) chamava de "degradado" o recife quieto
    (ACI 0).
  - −50 dB relativos mediam a reverberação, que preenche todas as bandas
    (ADI saturado em ~0,9).
- **A saúde:** metade ADI, metade ACI/0,8.
- **No diretor:** multiplica a capacidade do nicho por 0,8 + 0,4 ×
  saúde. Um habitat saudável comporta mais; um degradado (bandas
  sobrepostas, um zumbido), menos.
- **Na tela:** "habitat N%" ao lado do nicho.
- **Os testes:**
  - o silêncio fica neutro;
  - um tom fixo dá ADI 0 e ACI ~0;
  - três tons em bandas distantes dão ADI = ln 3 / ln 10;
  - um tom pulsante é mais complexo que o fixo;
  - um recife variado e articulado é mais saudável que um zumbido;
  - no diretor, um nicho de 16,9 com saúde 0,9 contra 12,2 com 0,1.
- **Medido no motor real** (120 s):
  - densidade 0,25: ADI 0,80, ACI 0,55, saúde 0,75;
  - densidade 0,90: ADI 0,74, ACI 0,57, saúde 0,73;
  - a saúde varia de 0,64 a 0,86 ao longo do tempo.

**Feito em 2026-10-20 (`ChorusPhase`, `src/core/ChorusPhase.*`):**

- **O desenho:**
  - cada organismo tem um relógio de canto e só começa uma frase quando o
    relógio dispara;
  - todos compartilham um ciclo-base (um compasso), com pequenas
    desafinações por espécie, e as espécies lentas cantam a cada dois
    ciclos;
  - o acoplamento é contínuo, proporcional ao que ainda está sendo ouvido
    pela água.
- **O alvo de fase por relação:**
  - aliados e caçador: 0;
  - corte: 1/4;
  - rivais, fuga e território: 1/2;
  - vizinhos indiferentes: 1/2 com metade do peso.
- **Os testes do conceito:** dois rivais ficam em antifase, aliados entram
  em fase, a resposta do corte fica 1/4 depois, três rivais se espalham, e
  sem escuta cada um mantém o próprio relógio.
- **Medido numa estação** (|Δφ| médio): cardume 0,079 (sincronia); fuga
  0,414 e território 0,404 (antifase); indiferentes 0,336 (o nicho temporal,
  contra 0,25 do acaso); caça 0,382.
- **A caça emergiu como perseguição:** o caçador puxa para a fase da presa e
  a presa foge para a oposta, então ela fica sempre um passo à frente no
  tempo até a captura física.
- A primeira versão, com períodos de 8 a 32 passos e acoplamento só no
  instante da chegada da onda, não travava nada (|Δφ| ≈ 0,25 em tudo). O
  modelo das rãs pede relógios próximos.

### 2.2 Keynote, sinal e soundmark; hi-fi e lo-fi (Schafer)

Já temos a keynote (a âncora) e os sinais (a guarda). Faltam dois:
- **soundmark:** o som que identifica *este* lugar. Proposta: um motivo do
  aquário, nascido da primeira revolução de canto ou escolhido pelo
  músico. Ele volta nas cadências e nas mudanças de estado, como a marca
  sonora do lugar.
- **hi-fi / lo-fi:** paisagem limpa (sons distintos) ou saturada (massa).
  Ligamos isso à capacidade do nicho: no excesso, o sistema não só corta
  eventos, ele **perde definição** (sons se fundem em massa, como em
  Xenakis). Teste: a mesma estação com a capacidade alta e baixa dá
  centroides e densidades distintos.

**Feito em 2026-10-21 (no diretor):**

- **Soundmark:**
  - nasce da primeira revolução de canto ou, sem ela, do primeiro canto
    que fecha o ciclo depois de uns 24 compassos;
  - volta depois de cada cadência e de cada mudança de estado da água,
    tocado pela âncora sobre o acorde do momento, com o contorno mantido;
  - aparece como ato ("a marca sonora volta") nos quatro idiomas.
- **Hi-fi / lo-fi:**
  - a pressão é o que foi cantado mais o que o nicho recusou, sobre a
    capacidade;
  - acima de 1, o recife perde definição: notas até 1,8× mais longas,
    release até 2,5×, mais glissando, acentos puxados para o meio, alturas
    que batem entre si (±0,15 semitom), e o motor fecha os agudos de todos
    até ×0,65;
  - no gelo, a rede de alturas se mantém: o gelo lotado só alonga.
- **Medido** (300 s):
  - 7 retornos da marca sonora;
  - nas notas lo-fi, duração média de 0,76 s contra 0,37 s e dispersão
    dos acentos de 0,14 contra 0,21.
- **Medido em áudio depois (2026-10-21), no motor completo:**
  - a primeira curva (lo-fi de 1,0 a 2,2 de pressão) **nunca** se ligava.
    A pressão real fica perto de 1, qualquer que seja a DENSIDADE, porque
    a capacidade acompanha o desejo, e só chega a 1,8–3,3 nos picos de
    aglomeração; o lo-fi máximo era 0,43;
  - a curva passou a ir de 0,9 a 1,8. O lo-fi agora chega a 0,78–0,84 e
    cerca de 29% dos quadros ficam lo-fi;
  - no A/B com a mesma semente e o mesmo áudio, o fechamento dos agudos
    baixa o centroide dos quadros lo-fi só de 666 para 651 Hz (−2%): o
    corte líquido (~3 kHz) fica acima do centroide do recife (~600 Hz);
  - **o lo-fi vive na articulação, não na cor.** Os quadros lo-fi são até
    mais brilhantes que os hi-fi, porque a aglomeração soma parciais.
- **A marca sonora espaçada (2026-10-21):** no motor completo, ela voltava
  17 a 19 vezes em 10 min (a cada ~35 s, depois de cada cadência e
  mudança), o que virava um tique. Agora volta com no mínimo 32 compassos
  de intervalo: 4 a 5 vezes em 10 min.
- **Conferido no motor completo** (2 sementes × 10 min, densidade 0,5):
  - presas entre 0,38 e 1,89;
  - 7 a 9 mortes e nascimentos, geração 3 a 5;
  - canal: 48–56% ouvido ao longo, 15% através;
  - habitat entre 0,50 e 0,95;
  - 14 a 18 capturas.
  - O contador de "revolução" (9 a 13) soma as revoluções e as adoções do
    canto novo, cerca de 3 revoluções em 10 min, como previsto.
- **A marca escolhida pelo músico (2026-10-21):**
  - o botão MARCA, ao lado de SALA, toma a frase que soa agora (a da voz
    que cantou por último) como marca do lugar, e ela soa na hora;
  - as revoluções de canto não a trocam;
  - fica guardada em `music.soundmark` ("graus|passos") e volta nas
    próximas sessões, limitada ao intervalo válido;
  - o contorno aparece entre o título do painel e o botão: âmbar quando
    escolhida, cinza quando o lugar a encontrou sozinho;
  - os testes do diretor cobrem a escolha, o retorno imediato, cinco
    minutos de revoluções sem troca e a restauração com limpeza;
  - no app real, a frase marcada foi salva e, ao reabrir, voltou em âmbar
    sem nenhum clique.

### 2.3 Ecossistêmica audível de verdade (Di Scipio)

O AESI de Di Scipio acopla o sistema **ao ambiente acústico real**: o som
da sala volta como entrada e o sistema se auto-organiza com ruído e
retroalimentação.

**Módulo `RoomCoupling`** (opcional, desligado por padrão):
- a entrada do microfone vira percepção dos organismos: o barulho da sala
  é ouvido como um organismo a mais;
- o Lombard reage a ele, e o nicho se desloca para fugir dele;
- regras: sem gravar nem enviar nada (realtime-safe, só descritores).

**Feito em 2026-10-21 (`RoomCoupling`, `src/core/RoomCoupling.*`):**

- **Ligar e desligar:** o botão SALA, ao lado de MAR, é desligado por
  padrão. Só ele abre a entrada de áudio (o dispositivo é reaberto com um
  canal de entrada). O estado fica em `room.enabled`.
- **O som do próprio aquário não conta como sala.** O microfone também
  ouve as caixas.
  - O módulo aprende o acoplamento por regressão: a potência do microfone é
    k² vezes a potência da saída mais a sala, com momentos exponenciais de
    ~10 s e correção de viés.
  - A sala é só o **excedente**. Sem isso, o Lombard se empurraria até o
    teto, num laço de realimentação.
  - Nos primeiros ~3 s, enquanto aprende, nada é relatado.
  - **Corrigido em 2026-10-22:** com o aquário em silêncio (pausado ou
    mudo) não há o que aprender, e a sala não era relatada nunca, nem com
    ruído alto no microfone. O defeito foi achado pelo teste de estresse.
    Agora, com a saída em silêncio (abaixo de −80 dBFS), tudo o que o
    microfone ouve é sala, assim que a potência assenta (~0,5 s). Há teste
    em `RoomCouplingTests`, que falha sem o conserto.
- **Descritores:**
  - o nível do excedente em dBFS;
  - a força (0 a −60 dBFS, 1 a −20 dBFS);
  - o registro: centroide de três bandas, abaixo de 300 Hz, de 300 Hz a
    2 kHz e acima de 2 kHz.
- **No diretor:**
  - o ruído da sala soma-se ao ruído ouvido do Lombard, 1 dB por dB acima
    de −50 dBFS, até o teto de 9 dB que já existia;
  - as vozes que cairiam a menos de 4 semitons do registro da sala saltam
    uma oitava para longe dele.
- **Privacidade:** nenhuma amostra é guardada, gravada, enviada ou
  reproduzida, e o REC grava só a saída. O texto de ajuda diz isso nos
  quatro idiomas.
- **Os testes:**
  - microfone mudo: sem sala;
  - só o aquário pelas caixas: o acoplamento é aprendido exato (0,25) e não
    há sala;
  - um zumbido externo de 100 Hz a −29 dBFS é ouvido a −30 dBFS, grave;
  - um chiado é ouvido, agudo;
  - no diretor, uma sala a −35 dBFS sobe os acentos de 0,50 para 0,89 e
    esvazia o seu registro (de 19% das notas para 0%).
- **No app real:** o botão aparece desligado e o painel funciona.
- **Não testado:** o microfone de verdade. Ligar a SALA fica para o
  autor.

### 2.4 Vida: recursos, reprodução, evolução (McCormack; Lotka–Volterra)

Eden tem criaturas que **competem por recursos limitados, se reproduzem e
evoluem** cantos que se ouvem. Fontana resolve **Lotka–Volterra** como uma
rede de filtros estável.

**Módulo `PopulationCycles`:** populações (abundância de "presas" e
"predadores" como densidades de eventos) seguem LV discretizado. Isso dá
**ciclos de abundância** de vários minutos (fartura, caça, escassez, volta),
uma forma longa que vem da ecologia.
- Teste: as fases dos picos predador/presa defasadas cerca de 1/4 de ciclo.

**Feito em 2026-10-21 (`PopulationCycles`, `src/core/PopulationCycles.*`):**

- **O modelo:** LV com equilíbrio em N* = P* = 1, integrado em logaritmos
  pelo Euler simplético. As órbitas ficam fechadas e as populações
  positivas para qualquer passo; é a mesma preocupação de estabilidade que
  Fontana (2012) resolve com a rede de filtros, mas por outro caminho.
- **O período:** de 2 a 5 minutos (120 s + 180 s × MEMÓRIA).
- **No diretor:**
  - a abundância de presas multiplica a capacidade do nicho
    (0,55 + 0,45·N, limitada a 0,7–1,35);
  - a abundância de predadores pesa a saliência de quem caça
    (×0,5–1,8);
  - cada captura real tira presas (−6%) e alimenta os predadores (+4%).
    A ecologia do recife empurra o modelo, que não é só um relógio.
- **O painel:** duas barras, PRESAS e PREDADORES, com a marca do
  equilíbrio.
- **Os testes:**
  - positividade e limites;
  - período de 120 ± 12 s;
  - o pico dos predadores segue o das presas (entre 0,1 e 0,45 de ciclo);
  - nenhuma deriva numérica em 10 ciclos;
  - o efeito da captura;
  - no diretor, 15 min com MEMÓRIA 0, comparando dentro de cada estado da
    água, fora dos 6 compassos de assentamento depois de cada mudança.
- **Medido:** o nicho com presas fartas contra presas escassas:
  - gelo: 4,6 contra 3,5;
  - temperada: 10,9 contra 7,8;
  - turbulenta: 15,8 contra 12,2;
  - fervura: 12,9 contra 9,3.

  São 1,3 a 1,4× por estado. Numa estação de 300 s, as mudanças de estado
  da água mascaram o ciclo: a fase move o nicho de 4 a 14, mais que as
  presas.
- **Não medido em separado:** o peso dos predadores na saliência dos
  caçadores.

**Módulo `LifeCycle`:**
- energia como alimento (a captura alimenta, o jejum esvazia);
- o par em corte gera uma **cria** cuja assinatura é a recombinação das
  duas;
- a velhice escurece a matéria do corpo; a morte decompõe o canto na
  âncora (já existe).

**Feito em 2026-10-21 (`LifeCycle`, `src/core/LifeCycle.*`, no diretor):**

- **A energia é alimento:**
  - quem pasta come em proporção ao quadrado da abundância de presas do
    `PopulationCycles`;
  - quem caça só come quando captura (+0,5);
  - a presa capturada sai ferida (−0,3);
  - viver custa, e cada nota cantada custa mais.
- **A fome tem efeito musical:** abaixo de um terço da energia, a voz se
  poupa e a sua cota no nicho cai até a metade.
- **A idade escurece o corpo:** na segunda metade da vida, o motor fecha
  os agudos do organismo (corte líquido × até 0,45).
- **Morte e nascimento:**
  - a vida acaba por idade (420 a 900 s) ou por fome;
  - o canto vai para o recife (a memória da âncora);
  - depois de oito compassos de silêncio, a voz renasce cantando uma
    **recombinação**: a cabeça de cada frase do próprio canto, a cauda da
    do par (quem cortejava, senão o aliado mais próximo, senão o vínculo
    mais forte), com o contorno emendado e uma unidade mutada;
  - a geração avança.
- **Os corpos do aquário continuam nadando:** o ciclo de vida é da voz. Uma
  geração é um canto novo no mesmo corpo.
- **Os testes:**
  - um pastador alimentado vive os seus 600 s;
  - um caçador sem presa morre de fome em cerca de 320 s;
  - na escassez, cantar encurta a vida (432 s contra 600 s em silêncio);
  - a captura alimenta;
  - a escuridão fica em 0 até a metade da vida e em 0,5 aos três quartos;
  - o filho tem a cabeça de um pai e a cauda do outro, emendadas, com uma
    mutação, e 200 cruzamentos dão frases válidas;
  - no diretor, 15 min com mortes, nascimentos e gerações.
- **Medido** (3 sementes × 15 min):
  - 8 a 9 mortes e o mesmo número de nascimentos;
  - geração 4 a 5;
  - vozes com fome em 4 a 9% dos compassos e escurecidas em cerca de 25%.

### 2.5 A canção em hierarquia (Payne & McVay; Noad; Garland)

As jubartes cantam em níveis:
- **unidades** formam **frases** (20–40 s);
- frases repetidas formam **temas**;
- os temas, em ordem, formam o **canto** (5–30 min), que se repete com
  precisão.

A canção também **evolui devagar** entre estações e, raramente, sofre uma
**revolução** quando chegam poucos cantores de fora (Noad 2000). Nós temos
frases soltas e revolução.

**Módulo `SongForm`:**
- a assinatura passa a ter frase → tema (n repetições) → canto (ordem dos
  temas);
- a MEMÓRIA controla a **deriva lenta** (mudar uma unidade por ciclo) e a
  revolução (trocar tudo);
- a forma da peça passa a ser legível: a volta do tema é um acontecimento.
- Teste: autocorrelação das alturas com picos no período do tema.

**Feito em 2026-10-20 (`SongForm`, `src/core/SongForm.*`):**

- **A forma:**
  - cada organismo canta dois ou três temas; o segundo responde ao
    primeiro, com o mesmo contorno uma terça acima e o final mudado, e o
    terceiro contrasta;
  - cada tema é uma frase repetida de 2 a 4 vezes;
  - o canto recomeça (um ato novo, "o canto recomeça", no painel), e a
    MEMÓRIA vira a deriva lenta: uma unidade por volta do canto;
  - a revolução copia o canto inteiro.
- **Para o tema ser ouvido** (medido e corrigido):
  - o nicho admite a frase inteira, reservando as unidades dela, ou o
    organismo espera a próxima chamada do relógio de canto;
  - sem lugar para tudo, canta a cabeça do tema;
  - o próprio canto resiste três vezes mais à atração da harmonia.
- **Os testes do conceito:** a ordem dos temas e das repetições, o
  recomeço, a evolução de uma unidade.
- **Medido contra a versão anterior** (três sementes × 300 s; motivos de 4
  notas já ouvidos nas últimas 80): de 8,9% para 13,0% com MEMÓRIA 0,2 e de
  6,1% para 12,5% com MEMÓRIA 0,8, com o mesmo número de notas.
- O coro continua de pé: cardume 0,099; fuga 0,444; território 0,447.

### 2.6 O alfabeto das codas e a troca de turnos (Sharma et al.)

As codas combinam **ritmo, andamento, rubato e ornamento** num inventário
grande. Nas conversas, as baleias **igualam o rubato** em frações de
segundo e acrescentam ornamentos conforme o contexto.

**Módulo `CodaExchange`:**
- cada organismo escolhe codas de um alfabeto (ritmo × andamento × rubato ×
  ornamento);
- em diálogo, os turnos se sobrepõem (coro) ou se alternam;
- o rubato converge e o ornamento marca as mudanças de assunto (mudança de
  relação).
- Teste: a distância de rubato entre parceiros cai ao longo do diálogo.

**Feito em 2026-10-21 (`CodaExchange`, `src/core/CodaExchange.*`, no diretor):**

- **O alfabeto:**
  - seis ritmos (regular, 1+1+3, accelerando, ritardando, longo-curto,
    curto-longo) e três andamentos (×0,75, ×1, ×1,35);
  - cada espécie começa com o seu ritmo de identidade, o seu clã (Hersh et
    al. 2022).
- **Em troca** (corte ou aliança, com o parceiro ouvido nos últimos 48
  passos):
  - o par fala na coda de quem chama, ritmo × andamento; os graus do canto
    ficam, o ritmo vira coda;
  - quem chama às vezes muda o andamento;
  - um aliado às vezes adota o ritmo de identidade de quem chama, com
    chance MEMÓRIA × 0,15: um clã se forma.
- **O rubato converge turno a turno** (30% da diferença por coda), em vez
  do casamento instantâneo de antes (50% por passo, na corte).
- **O ornamento marca a mudança de assunto** (relação ou parceiro novo):
  55% de chance contra 5%, mais um pouco com o perigo.
- **Os testes:**
  - as formas dos ritmos e a ordem dos andamentos;
  - a convergência: uma volta fecha parte da distância, oito voltas fecham
    quase tudo;
  - o ornamento;
  - os clãs distintos;
  - no diretor (300 s): distância de rubato de 0,029 nos primeiros turnos
    para 0,016 a partir do quarto; ornamentos em 67% das mudanças contra
    14% fora delas.
- **Medido no motor completo** (2 sementes × 10 min):
  - distância de rubato de 0,056–0,060 para 0,009–0,020;
  - ornamentos em 53–60% das mudanças contra 12–15%.
- **Fica para depois:** calibrar as estatísticas de ritmo e rubato com os
  dados do Project CETI, só como estudo.

### 2.7 O endereçamento correto (King & Janik)

No estudo, quem **copia** a assinatura do outro o chama, e o chamado
**responde com a própria assinatura**.

**Correção proposta no corte:** o que chama canta a assinatura do outro,
depois o chamado responde com a sua. Hoje quem responde canta a
assinatura de quem chamou.

### 2.8 Lombard em decibéis (Holt et al.)

O efeito medido é de **+1 dB por dB de ruído**. **Ajuste:** converter o
ruído ouvido (já temos o nível recebido de cada frente) em dB acima de um
piso e elevar o acento na mesma medida, com teto. Um efeito lateral
musical: crescendos em cadeia nos momentos densos.

### 2.9 O medo que sincroniza (Aguilar de Soto et al.)

As baleias-bicudas **vocalizam em sincronia de grupo** (98% de sobreposição),
o que reduz o tempo em que podem ser detectadas, e sobem **em silêncio e
em direção imprevisível**. **Ajuste na fuga:** sob ameaça, as presas aliadas
concentram o canto numa mesma janela e depois se calam juntas. Fica um
gesto de "tutti e silêncio" vindo do medo.

### 2.10 A rede de bolhas como parede de som

As jubartes produzem chamados de 400–600 Hz que, junto com a rede, formam
uma **"parede de som" com o centro mais quieto**. **Ajuste:** durante a rede,
o caçador e o aliado empurram as outras vozes para fora de uma banda
central, o "vazio" onde a presa fica presa, e esse silêncio no meio fica
audível.

**Feito em 2026-10-20 (as correções de fidelidade 2.7, 2.8 e 2.9):**

- **Endereçamento** (King & Janik): no corte, quem chama canta a cópia da
  assinatura do outro (o "nome" dele), e o chamado responde com a própria,
  logo em seguida, igualando o rubato de quem chamou.
- **Lombard em dB** (Holt et al.):
  - o ruído ouvido em dB acima de um piso eleva a voz em tantos dB (até
    9 dB);
  - medido: acento médio 0,55, 10,5% no teto e 7,5% abaixo de 0,3. A
    primeira calibração tinha 27% no teto e foi corrigida.
- **O medo que sincroniza** (Aguilar de Soto et al.):
  - sob um caçador em ação, os não predadores que o percebem puxam as fases
    entre si para a sincronia (peso triplo);
  - quando uma presa se cala, quem a ouve e não caça se cala junto;
  - medido entre os pares ameaçados: |Δφ| 0,182, contra 0,265 sem caça. A
    sincronia é parcial, porque as caçadas duram pouco e o coro leva tempo
    para travar.

### 2.11 A água: absorção, velocidade e camadas

- **Absorção real:** trocar o f² por uma forma de duas relaxações
  (ácido bórico e MgSO₄) mais o termo viscoso, com dependência da
  temperatura (Ainslie & McColm 1998 como referência). A temperatura dos
  nossos estados da água muda a cor do som de um jeito físico. Uma opção
  ÁGUA DOCE / ÁGUA SALGADA também tem sentido (a doce quase não tem as
  relaxações).
- **Velocidade do som pela temperatura** (Mackenzie): o canon do caçador
  fica um pouco mais lento na água fria.
- **Camadas e o canal SOFAR:** o oceano real tem um mínimo de velocidade em
  profundidade que guia o som para longe, e zonas de sombra. Ferramenta de
  estudo: Bellhop (Acoustics Toolbox, Porter). **Módulo `SoundChannel`:**
  dentro da esfera, uma camada de profundidade em que os sons viajam longe
  e outras com sombra. A **altura (y) dos organismos** passa a decidir quem
  ouve quem.

**Feito em 2026-10-21 (`SoundChannel`, `src/core/SoundChannel.*`):**

- **O perfil:** o perfil canônico de Munk (1974), com ε = 0,00737 e
  B = 1300 m, a esfera lida como 5000 m de oceano e `y` como altura.
- **O eixo segue a temperatura da água:** fica logo sob a superfície na
  água polar e sob o gelo (o canal polar) e desce a um terço da
  profundidade na água quente.
- **Ao longo do canal:** espalhamento cilíndrico (Urick), +8,3 dB a 6
  unidades.
- **Através dele:** a refração deixa uma sombra (−14 dB a 4 unidades),
  recíproca. Perto da fonte, o caminho direto ainda chega.
- **Longe do canal:** o campo livre fica como antes.
- **O Bellhop (Porter)** é a referência de estudo; nada de traçado de raios
  embutido.
- **No diretor:** cada frente de onda ouvida passa pelo ganho do canal.
- **Na tela:** a faixa SOFAR atravessa a esfera do ecossistema na altura
  do eixo.
- **Medido** (3 sementes × 300 s, frentes que chegam a mais de 1,5
  unidades):
  - através do canal, a parcela ouvida cai de 40–44% para 11–13%;
  - ao longo dele, sobe de 42–52% para 46–56%.

  O ganho ao longo é modesto porque a absorção, a 150 km por unidade, já
  consumiu os médios e agudos. É o que o mar faz: o SOFAR leva longe os
  graves. O efeito musical forte é a sombra: **a altura decide quem ouve
  quem**.

**Feito em 2026-10-20 (a física da água):**

- **Absorção** (Ainslie & McColm 1998): as relaxações do ácido bórico e do
  sulfato de magnésio mais o termo viscoso, em dB/km, pela frequência, pela
  temperatura, pela salinidade e pelo pH. Isso substitui a regra f² na
  escuta entre os organismos e no timbre que chega ao ouvinte.
- **A escala do oceano em miniatura:**
  - uma unidade de distância vale ~150 km para a escuta e ~4 km para o
    timbre;
  - calibrada para que a água temperada do mar soe como antes (corte a 4
    unidades: 3.028 Hz, antes 2.911);
  - o que mudou é que a temperatura e o sal agem fisicamente.
- **Velocidade do som** (Mackenzie 1981): na água líquida segue a
  temperatura (~4,5 m/s por °C); a tabela antiga exagerava a diferença
  entre a água fria e a temperada (15%, contra ~4% no real). O gelo (rápido)
  e a água com bolhas (lenta, Wood) mantêm os seus valores.
- **A temperatura** vem do calor da água (0 a 30 °C; o gelo abaixo de zero).
  O fator de absorção por estado é 0,25 no gelo, 1 na água líquida, 1,3 na
  turbulência e 6 na fervura, pelas bolhas.
- **MAR / ÁGUA DOCE:** um seletor no painel da música, persistido nas cenas.
  A água doce, sem a química do mar, fica límpida (perto do vidro quase sem
  filtro); só a fervura escurece.
- **Os testes:** ~1.507 m/s a 15 °C; mais lento no frio; ~1 dB/km a 10 kHz;
  a água doce absorve menos de 1/10 do mar a 3 kHz e soa mais brilhante.

### 2.12 Consonância que depende do timbre (Plomp & Levelt; Sethares)

A dissonância sensorial depende da banda crítica e dos **parciais**.
Sethares (1993) parametrizou a curva de Plomp & Levelt e mostrou como
escolher a escala a partir do timbre.

**Módulo `SpectralConsonance`:** com os parciais reais de cada corpo
(harmônicos, membrana, vidro, dobra), a consonância de um par é calculada
pela soma da aspereza entre parciais, no registro real.
- Uma membrana inarmônica pede outros intervalos que uma voz.
- Isso abre **escalas que nascem dos corpos**, em vez de modos fixos.
- Teste: a curva calculada para timbre harmônico tem mínimos em 3:2, 4:3,
  5:4…

**Feito em 2026-10-20 (`SpectralConsonance`, `src/core/SpectralConsonance.*`):**

- **A conta:**
  - a aspereza de Sethares (a curva de Plomp & Levelt parametrizada, com
    os parâmetros publicados), somada sobre os parciais de dois tons;
  - a consonância é a aspereza que o intervalo **acrescenta** à dos
    próprios tons (assim o uníssono de um tom inarmônico continua liso),
    relativa ao intervalo pequeno mais áspero no mesmo registro.
- **Os testes do conceito:**
  - harmônico no médio: quinta > terça > segunda menor; quarta > segunda
    maior;
  - a terça mais áspera no grave;
  - a membrana prefere a sexta menor mais que a voz;
  - tons puros só ásperos quando próximos.
  - Resultado honesto: no temperamento igual, a terça maior (400 cents)
    bate quase como o trítono.
- **No diretor:**
  - a harmonização usa a consonância entre os espectros reais dos dois
    organismos, que o motor passa (o corpo nativo misturado ao segundo
    corpo pela MISTURA);
  - os pares indiferentes não pagam a conta;
  - quando o conjunto é inarmônico, **a escala nasce dos corpos**: as sete
    alturas mais lisas para o espectro do conjunto (Sethares: dado um
    timbre, qual escala?).
- **Medido** (3 sementes × 300 s, intervalos entre aliados e casais), com
  os corpos nativos e depois em membrana:
  - sexta menor: de 3,9% para 8,7%;
  - quinta: de 18,5% para 10,0%;
  - terças menor e maior: de 9,9% e 6,1% para 4,1% e 3,7%;
  - sexta maior, trítono e sétima maior sobem.
- **Custo:** o pior tick do diretor fica em 187 µs (cerca de 1,8% de um
  bloco de 512 amostras); fica registrado para otimizar, se preciso.

### 2.13 Massa estatística da fervura (Xenakis)

Em *Pithoprakta*, cada instrumento é uma molécula, e as velocidades dos
glissandos seguem **Maxwell–Boltzmann**. Fica uma ligação física perfeita
com os nossos estados: a **temperatura da água vira a dispersão das
velocidades**.

**Módulo `ThermalCloud`:** na turbulência e na fervura, nuvens de
glissandos (os corpos de gota, grão e dobra) cujas velocidades seguem
Maxwell–Boltzmann com T = o calor da água; no gelo, a nuvem congela.
- Teste: o histograma das velocidades acompanha a distribuição.

**Feito em 2026-10-20 (`ThermalCloud`, no diretor):**

- **Na turbulência e na fervura:** um organismo sem relação ativa, entre as
  suas frases, emite **moléculas**: glissandos cuja velocidade é sorteada
  da distribuição de Maxwell–Boltzmann, com σ ∝ √T, a temperatura da água
  (100 °C na fervura).
- **A parede do registro** reflete a molécula.
- **O orçamento:** a nuvem tem o seu próprio, por compasso (capacidade ×
  turbulência), para não tomar o lugar das frases. O glissando pode durar
  até 2 s.
- **Os testes:** velocidade média de 2√(2/π)·σ; P(v < σ) ≈ 0,199; as
  velocidades escalam com √T.
- **Medido** (3 sementes × 300 s, água morna): 46 moléculas na turbulência
  (7,0 semitons/s) e 62 na fervura (8,4 semitons/s); nenhuma no gelo, na
  água fria ou na temperada.
- **Limite honesto:** a densidade ainda é moderada, porque só os organismos
  sem relação ativa entram na nuvem. Uma nuvem mais densa pede um corpo
  dedicado (gota ou grão), uma possibilidade futura.

### 2.14 Os corpos, mais fiéis

- **Vidro:** as razões dependem do copo e o **nível de água** abaixa a
  altura (Jundt et al. 2006). A MATÉRIA pode mover a forma do copo, e o
  estado da água pode "encher o copo".
- **Gotas:** a distribuição dos raios das bolhas segue lei de potência na
  chuva e nos riachos; o impacto inicial da gota mais a bolha (Liu, Cheng
  & Tong 2019) dão chuva mais realista; o acoplamento entre bolhas
  (*coupled bubbles*) muda as frequências em nuvem.

---

## 3. O que ainda não exploramos e tem potencial (projetos livres)

| Projeto | O que é | Potencial no Aquorbium | Licença e uso |
|---|---|---|---|
| **FAUST `physmodels.lib`** (GRAME) — já está no Atlas; **usado como método em 2026-10-21 para o ARCO e a PALHETA** (ver TIMBRE_BODIES_DESIGN.md) | modelos físicos por guia de onda e modal: cordas, membranas, barras, sopros, arcos | referência de método para corpos novos (arco friccionado, palheta, flauta); validar os nossos corpos contra modelos de referência | a licença da biblioteca e dos modelos deve ser auditada antes de qualquer cópia; uso como estudo |
| **IEM Graz, guias de onda e Ambisonics** — no Atlas; **em 2026-10-21 os fones ganharam o binaural direto por fonte (Brown & Duda), mais nítido que Ambisonics de 1ª ordem para cinco fontes; Ambisonics fica para o multicanal** (ver AQUARIUM_SPACE_DESIGN.md) | espacialização e guias de onda | espacialização tridimensional da esfera (a posição y já existe) | por componente |
| **pyfar** — no Atlas | acústica de salas e sinais | validar a propagação, a absorção e os ecos | MIT; estudo |
| **Essentia** — no Atlas | análise de áudio | descritores para o `HabitatHealth` e o `RoomCoupling` | AGPL; estudo ou ferramenta externa |
| **scikit-maad** | índices de paisagem sonora: ACI, ADI, BI, NDSI e mais de 50 | método do `HabitatHealth`; medir os renders | BSD-3; estudo (é Python) |
| **Bellhop / Acoustics Toolbox** (M. Porter); bellhopcxx | traçado de raios do som no mar: canal SOFAR, sombras, tempos de chegada | o módulo `SoundChannel`; perfis de velocidade para camadas | código aberto; estudo, sem embutir |
| **Modelo de coro de rãs** (Aihara et al., *Sci Rep* 2014; *R Soc Open Sci* 2019) | osciladores de fase com antifase e aglomerados | o módulo `ChorusPhase` | artigos abertos; reescrever |
| **Harmonic Fluids** (Zheng & James 2009, Cornell); *Improved Water Sound Synthesis using Coupled Bubbles* (2023) — **usado como ideia em 2026-10-21: o `BubbleField`** (ver abaixo) | som de líquidos por bolhas acopladas a simulações | a fervura e a gota mais ricas | artigos; reescrever |
| **Chuva física** (Liu, Cheng & Tong 2019) | impacto + bolha, estatístico, barato | a chuva do corpo de gota | artigo; reescrever |
| **Project CETI**: dataset DSWP (1.501 codas, Hugging Face), Coda-detector, WhAM | dados e modelos de codas de cachalote | **estudo** das estatísticas de ritmo e rubato para calibrar o `CodaExchange` | verificar a licença; **não** embutir áudio |
| **Orcasound** | rede aberta de hidrofones, dados no AWS, código no GitHub | escuta e referência; uma ideia futura de "janela para o mar" (ouvir ao vivo, separado do instrumento) | verificar a licença dos dados; nenhum áudio de terceiros dentro do repositório |
| **Lotka–Volterra estável** (Fontana 2012) | solução por rede de filtros sem instabilidade | o módulo `PopulationCycles` | artigo; reescrever |

Regra RASGO mantida: nenhum código, áudio ou dado de terceiros entra no
repositório; usamos teoria, método e calibração, com a fonte citada.

**Feito em 2026-10-21 (`BubbleField`, `src/core/BubbleField.*`): a água
borbulhando.** Antes, a fervura só fazia os organismos cantarem alturas
de Minnaert; a água não borbulhava.
- **Cada bolha:**
  - soa na frequência de Minnaert (3,26 / R);
  - morre com o amortecimento de uma bolha milimétrica (δ ≈ 0,1,
    Leighton);
  - sobe um pouco de altura ao subir (van den Doel 2005).
- **Os raios** seguem a distribuição de Deane & Stokes (2002): R^(−3/2)
  abaixo de ~1 mm e R^(−10/3) acima.
- **Bolhas nascidas juntas** (em até 3 ms) se acoplam: o par se divide no
  modo em fase, mais grave e forte, e no modo em antifase, mais agudo e
  fraco, f0 / √(1 ± R/d) (Feuillade 1995), como no som acoplado de Xue et
  al. (2023).
- **No motor:**
  - o ritmo segue a água: ~30 bolhas/s na turbulência e ~240/s na
    fervura;
  - cada bolha tem o seu lugar no estéreo, antes do espaço;
  - sem a música ecológica o campo nem é chamado: o som fica idêntico bit
    a bit.
- **Os testes:**
  - Minnaert de 1 mm = 3,26 kHz;
  - a divisão exata dos modos acoplados (f0/√1,5 e f0/√0,5 a d = 2R);
  - as inclinações medidas da distribuição: −1,500 e −3,334;
  - ~50 bolhas/s, com 11% em pares;
  - a saída limitada, zeros exatos com a água parada e determinismo.
- **O nível,** medido na fervura (240 s, a água empurrada para quente),
  fica de 6 a 10 dB abaixo dos organismos. A medida tem ~1 dB de
  incerteza, porque a saída realimenta o ecossistema e a música diverge
  entre os dois renders.
- **A escuta** está em
  `AUDIOS_RASGO/Aquorbium/escutas/2026-10-21-fervura/`: 90 s, da água
  temperada para a turbulenta (4,3 s) e a fervura (8,7 s), −36,3 LUFS.

---

## 4. Roteiro proposto (ordem por impacto musical e risco)

| # | Módulo / ajuste | Conceito | Esforço | Por quê primeiro |
|---|---|---|---|---|
| 1 | `ChorusPhase` | nicho temporal; coro de rãs | médio | o hoquetus e a sincronia passam a **emergir**, sem regra |
| 2 | `SongForm` | hierarquia do canto; revolução | médio | dá forma legível em minutos: o tema que volta |
| 3 | correções 2.7, 2.8 e 2.9 | endereçamento; Lombard em dB; medo que sincroniza | pequeno | ficam fiéis às fontes |
| 4 | `SpectralConsonance` | Plomp & Levelt; Sethares | médio/grande | a harmonia passa a depender dos corpos |
| 5 | absorção e velocidade físicas (2.11) | relaxações; Mackenzie | pequeno | o timbre da água fica físico, com água doce ou salgada |
| 6 | `ThermalCloud` | Xenakis; temperatura | médio | a fervura vira massa estatística |
| 7 | ~~`PopulationCycles`~~ feito | Lotka–Volterra | médio | forma longa ecológica |
| 8 | ~~`LifeCycle`~~ feito | Eden | grande | nascimento, cria e envelhecimento |
| 9 | ~~`SoundChannel`~~ feito | SOFAR; camadas | médio | a profundidade decide quem ouve quem |
| 10 | ~~soundmark e hi-fi/lo-fi (2.2)~~ feito | Schafer | pequeno | identidade do lugar; clareza ou massa |
| 11 | ~~`HabitatHealth`~~ feito | Krause; índices acústicos | médio | uma medida de saúde do bioma |
| 12 | ~~`RoomCoupling` (opcional)~~ feito | Di Scipio | médio | o ambiente real entra no ecossistema |

Cada módulo segue o padrão do projeto:
- influência 0 dá o som anterior, bit a bit;
- é determinístico por seed e seguro no áudio;
- tem teste do conceito (o critério está em cada seção) e um render de
  comparação;
- fica registrado aqui e no Atlas.

---

## Fontes

- Krause 1993: scirp.org/reference/referencespapers?referenceid=1127953
- Schafer 1977: monoskop.org (The Soundscape, 1994)
- Di Scipio 2003: semanticscholar.org ('Sound is the interface')
- McCormack 2001: users.monash.edu/~jonmc/research/Papers/McCormack_ECAL01.pdf
- Blackwell & Bentley 2002: congres.cran.univ-lorraine.fr/2002/WCCI2002/CEC02/PDFFiles/Papers/7070.PDF
- Fontana 2012: researchgate.net/publication/274068940
- Garland et al. 2011: pubmed.ncbi.nlm.nih.gov/21497089
- Noad et al. 2000: nature.com/articles/35046199
- Payne & McVay 1971: researchgate.net/publication/6007737
- Sharma et al. 2024: nature.com/articles/s41467-024-47221-8
- Janik & Sayigh 2013: link.springer.com/article/10.1007/s00359-013-0817-7
- King & Janik 2013: pnas.org/doi/10.1073/pnas.1304459110
- Miller, Johnson & Tyack 2004: doi.org/10.1098/rspb.2004.2863
- Holt et al. 2009: tethys.pnnl.gov (Speaking up)
- Aguilar de Soto et al. 2020: nature.com/articles/s41598-019-55911-3
- Rede de bolhas: royalsocietypublishing.org/rspb/article/293/2063/20252409
- Au 1993 (ICI): researchgate.net/publication/329213384
- Minnaert 1933: en.wikipedia.org/wiki/Marcel_Minnaert
- van den Doel 2005: dl.acm.org/doi/10.1145/1101530.1101554
- Wood: repositories.lib.utexas.edu (An audible demonstration of the speed of sound in bubbly liquids)
- Absorção: researchgate.net/publication/224610767 (Ainslie & McColm)
- Mackenzie 1981: ui.adsabs.harvard.edu/abs/1981ASAJ...70..807M
- Gelo: asa.scitation.org/doi/10.1121/1.2996304
- Plomp & Levelt 1965: pubs.aip.org/asa/jasa/article/38/4/548
- Sethares 1993: sethares.engr.wisc.edu/consemi.html
- Peterson & Barney 1952: ling.upenn.edu/courses/Fall_2013/ling520/PetersonBarney52.pdf
- Membrana: en.wikipedia.org/wiki/Vibration_of_a_circular_membrane
- Vidro: phys.unsw.edu.au/music/people/publications/Jundtetal2006.pdf
- Xenakis: en.wikipedia.org/wiki/Pithoprakta
- Waters 2007: pure.qub.ac.uk (Performance Ecosystems)
- Keller & Lazzarini 2017: cambridge.org (Organised Sound 22 SI1)
- scikit-maad: github.com/scikit-maad/scikit-maad
- Aihara (coro de rãs): nature.com/articles/srep03891; royalsocietypublishing.org/rsos/article/6/1/181117
- Bellhop: arlpy.readthedocs.io/en/latest/uwapm.html; github.com/org-arl/AcousticsToolbox.jl
- Harmonic Fluids: cs.cornell.edu/projects/HarmonicFluids
- Chuva: dl.acm.org/doi/10.1145/3306346.3323045
- FAUST physmodels: faustlibraries.grame.fr/libs/physmodels
- Project CETI: huggingface.co/datasets/orrp/DSWP; github.com/Project-CETI/wham
- Orcasound: github.com/orcasound; registry.opendata.aws/orcasound
