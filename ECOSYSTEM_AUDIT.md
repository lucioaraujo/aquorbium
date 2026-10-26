# Auditoria do protótipo `Ecosystem.h`

**Data:** 2026-08-12  
**Escopo:** coerência conceitual, arquitetura C++/JUCE, determinismo, segurança
de áudio e aptidão para integração ao Aquorbium  
**Parecer:** preservar como memória conceitual; não promover esta implementação
ao core

## 1. Conclusão executiva

O arquivo contém uma tradução fértil do conceito Bioma para entidades de
software. A ideia central — nenhuma entidade possui visão total — aparece em
organismos com percepção situada, observadores com oclusão e filosofias de
mixagem, ambiente temporal, desejo indireto, presença, integridade e foco com
custo.

Isso não torna o arquivo uma implementação válida do instrumento. Ele não é
compilado pelo CMake atual, não possui testes e diverge do core existente. No
caminho de áudio aloca memória, usa aleatoriedade global não determinística e
faz a evolução depender do tamanho dos blocos. Há ainda erros objetivos na
geometria e na integridade. Portanto, ele deve ser tratado como **pseudocódigo
conceitual executável**, não como fundação a ser corrigida aos poucos.

A reconstrução deve ser autoral, em C++20, com JUCE nas adaptações em que ele
é apropriado. Referências externas continuarão servindo para formular
princípios e perguntas; nenhum código externo será copiado. Código dos demais
instrumentos RASGO só poderá ser reaproveitado com proveniência explícita,
adaptação ao Aquorbium e testes locais.

## 2. O que está conceitualmente correto e deve permanecer

### Perspectiva parcial e situada

- O organismo possui direção, campo perceptivo e visibilidade subaquática.
- O observador combina posição, distância, oclusão e um modo de escuta.
- A mesma cena pode produzir leituras diferentes sem existir uma mixagem
  supostamente neutra ou onisciente.

Esse é o núcleo mais original e coerente do arquivo. Deve virar um contrato
testável: uma entidade nunca reage a um estado que não conseguiu observar.

### Desejo como intenção, não automação total

`Desire` e `AccessPort` formalizam uma intervenção externa limitada. Calma,
densidade, coesão e perigo reparametrizam comportamentos; não escrevem
diretamente a sequência musical. A separação é correta. A versão real deverá
usar snapshots suavizados e independentes da GUI; APVTS será apenas um
adaptador possível.

### Tempo narrativo ecológico

Dia/noite, estação, temperatura e acasalamento oferecem forma de longo prazo
sem impor uma seção musical convencional. O princípio deve permanecer, mas os
ciclos precisam de unidades, faixas, validação e possibilidade de congelar ou
reproduzir o estado para comparação auditiva.

### Organismos individuais, cardumes e estratégias combináveis

Separar espécie, personalidade individual, cardume e estratégia evita uma
hierarquia rígida de subclasses. Um organismo pode ser territorial e predador,
por exemplo. Essa composição é adequada ao conceito, desde que a execução em
tempo real use armazenamento preparado fora do callback.

### Presença como transição

Entrar, sair, permanecer ausente ou ficar preso sem criar/destruir objetos no
callback é uma boa tradução musical e técnica. A remoção física fora da thread
de áudio também é uma decisão correta.

### Lupa como foco com custo

A Lupa não é apenas ganho: um observador obtém detalhe local e perde definição
periférica. Essa assimetria reforça o princípio de visão parcial. Deve ser
preservada, mas seu efeito futuro precisa distinguir presença, detalhe
espectral e peso de mixagem em vez de multiplicar tudo por um único número.

### Uma amostra por organismo, várias leituras

O protótipo calcula a amostra de cada organismo uma vez e a reutiliza nas
quatro filosofias. A intenção é correta: os observadores escutam a mesma cena,
não quatro execuções divergentes do gerador.

## 3. Problemas críticos que impedem a integração

### O Cérebro não controla o acaso

`FluidMotion`, a rotação, a fissura espacial e o predador usam
`juce::Random::getSystemRandom()`. O parâmetro `BrainEngine&` é ignorado em
vários pontos. Isso contradiz a seed reproduzível, cria estados impossíveis de
reconstituir e rompe o requisito de uma única origem coerente de incerteza.

**Decisão:** descartar todo acesso a RNG global. Eventos ecológicos devem usar
streams determinísticos pertencentes ao `BiomaBrain`, com ordem de consumo
estável e testes por seed.

### O resultado depende do tamanho do bloco

O ecossistema avança uma vez por callback usando
`dt = numSamples / sampleRate`; pesos ficam congelados durante todo o bloco e
as chances aleatórias são sorteadas por bloco. Mudar de 64 para 512 amostras
muda quantidade e posição dos eventos, movimento e mixagem.

**Decisão:** criar um relógio de controle baseado em contagem absoluta de
amostras, independente das fronteiras do host. Interpolações/smoothing devem
ocorrer por amostra quando afetarem áudio.

### Há alocação no callback

`Ecosystem::process` cria `weight` e `sampleCache` como `std::vector` a cada
bloco. Isso viola diretamente a política realtime do RASGO.

**Decisão:** capacidades fixas ou memória preparada em `prepare`; nenhuma
alocação, remoção, log, lock ou chamada de dispositivo em `process`.

### A fissura destrói a identidade espacial de uma consulta

`worldPosition()` sorteia um deslocamento novo toda vez que é chamada. Durante
o mesmo instante, o mesmo organismo aparece em posições diferentes para a
Lupa, para cada observador e para cada teste de oclusão. Isso não representa
instabilidade: representa ausência de estado consistente.

**Decisão:** a fissura deve produzir um estado de deformação cacheado e
evolutivo, atualizado pelo relógio ecológico. Todas as observações de um mesmo
instante consultam a mesma posição efetiva.

### A quebra não é rara: é inevitável

`dangerFactor()` nunca fica abaixo de `0.3`. O tanque recebe no mínimo
`0.15 * 0.3 = 0.045` de estresse por segundo e recupera apenas `0.02`, sem
contar extremos sazonais. Mesmo com perigo mínimo, o saldo é positivo e o
aquário termina quebrado. Isso contradiz a descrição de possibilidade rara e
drástica.

**Decisão:** retirar a fórmula. Integridade precisa de eventos causais
identificáveis, histerese, tempo de sustentação, recuperação musicalmente
legível e um portão explícito antes do estado irreversível. `shattered` não
entra no motor até sua consequência sonora e sua política de reset/preset
serem decididas.

### Os observadores “externos” começam dentro do aquário

O raio declarado é `4.0`, mas os quatro observadores são construídos a uma
distância `3.0` do centro. A geometria contradiz o próprio conceito.

**Decisão:** observadores externos precisam permanecer além do vidro ou ser
representados como direções/pontos na superfície, com invariantes testados.

### Organismos e observadores podem fugir indefinidamente da geometria

Posições recebem deriva sem contenção, reflexão, atração ou projeção de volta
à esfera. A Lupa pressupõe organismos próximos de um vidro de raio fixo, mas o
sistema não preserva esse mundo.

**Decisão:** definir física de fronteira deliberada: reflexão, contenção
suave, regiões permeáveis ou fuga via `Presence`. Ultrapassar o vidro não pode
ser um acidente numérico.

### A presa possui conhecimento total

O predador usa `observe()`, mas a presa percorre todos os predadores por
posição, ignorando direção e turbidez. Isso viola a perspectiva parcial
justamente numa relação conceitualmente central.

**Decisão:** toda reação ecológica consulta uma observação situada. Caso uma
estratégia tenha percepção diferente, isso deve ser um traço explícito, não
acesso global silencioso.

### Não existe orçamento de ganho

Pesos de observação, energia, presença, quatro submixes, blends e panorâmica
são multiplicados/somados sem normalização, teto por bus ou compensação de
densidade. O nível cresce com o número de organismos. `panLeft/panRight` não é
panorâmica de potência constante e mudanças de blend não têm smoothing.

**Decisão:** o Ecossistema deve publicar pesos limitados; um mixer separado
aplica normalização artística documentada, equal-power pan, headroom e
telemetria. O limiter técnico permanece depois do master criativo e nunca é
controlado pelo Bioma.

### A autorreferência está apenas declarada

`StructuralMutation::lastOutputEnergy` nunca é alimentado pelo processo. O
comentário promete cross-feedback, mas o código permanece estático.

**Decisão:** criar taps explícitos de observação pré-master, pré-safety e de
redução de ganho. Feedback deve ter faixa, smoothing, atraso e saturação
definidos. Um comentário não conta como integração.

## 4. Partes que devem ser redesenhadas

### Taxonomia de espécies

`DelayTap` é tratado como espécie que gera `renderSample()` sem receber áudio.
Um delay comum é relação espacial/transformador, não fonte autônoma. Ele só
deve ser organismo se houver uma hipótese musical específica — por exemplo,
tap auto-oscilante com memória própria. Caso contrário, pertence ao ambiente
ou ao módulo Espaço.

O número de espécies audíveis deve permanecer pequeno. Pulsar, granular,
oscilador, ressonador ou corda são candidatos a arquétipos; dezenas de módulos
pesquisados devem colorir relações internas, não virar dezenas de vozes
simultâneas.

### Filosofias nomeadas por artistas

Os nomes `Devine`, `Autechre`, `Barbieri` e `Cortini` associam algoritmos muito
simples a práticas artísticas complexas sem base suficiente. Referências devem
ficar nos créditos e na pesquisa. O vocabulário executável do Aquorbium deve
ser autoral e descrever o comportamento: seleção esparsa, memória de energia,
permutação situada, deriva coletiva, por exemplo.

### Polimorfismo e armazenamento

`unique_ptr` e interfaces virtuais são aceitáveis na montagem fora do áudio,
mas a arquitetura atual não define capacidade, propriedade, clonagem,
serialização nem snapshot. Para o primeiro marco, arranjos fixos e estratégias
por valor são mais auditáveis. Polimorfismo só deve voltar quando houver uma
necessidade musical que compense o custo e a complexidade.

### Monólito de 1.211 linhas

Modelo ecológico, movimento, aleatoriedade, geometria, mixagem e renderização
de áudio estão no mesmo header. Isso dificulta testes, recompilação e revisão.

**Separação recomendada:**

1. `EcosystemModel` — estados e invariantes sem JUCE GUI;
2. `EcosystemClock` — tempo absoluto e eventos determinísticos;
3. `Perception` — visão, oclusão e Lupa;
4. `Behaviour` — movimento, cardume e estratégias;
5. `EcosystemMixer` — pesos e roteamento, sem master técnico;
6. adaptador JUCE — `AudioBuffer`, APVTS e aplicativo;
7. `BiomaEngine` — integração das fontes sonoras reais.

## 5. O que deve ser descartado

- toda aleatoriedade por `getSystemRandom()`;
- atualização ecológica uma vez por bloco do host;
- alocações locais em `process()`;
- ruído novo dentro de `worldPosition()`;
- fórmula atual de fissura/quebra automática;
- posições iniciais dos observadores dentro da esfera;
- movimento sem regra de fronteira;
- acesso onisciente da presa aos predadores;
- panorâmica linear e soma sem orçamento de ganho;
- nomes de filosofias vinculados diretamente a artistas;
- alegações de integração, compilação ou cross-feedback que não possuem teste;
- identidade antiga “Rasgo Synth/GPL” no código do Aquorbium AGPLv3.

Descartar esses trechos não significa descartar o conceito que tentavam
representar. Significa reimplementar cada hipótese de forma mensurável e
autoral.

## 6. Contrato recomendado para a reconstrução

O núcleo ecológico deve oferecer, no mínimo:

```text
prepare(sampleRate, maximumBlockSize)
reset(seed/brainState)
setDesire(snapshot limitado e suavizado)
advance(sampleCount, observations)
mixSnapshot()
telemetrySnapshot()
```

Requisitos:

- mesma seed e mesmos eventos produzem estado idêntico;
- resultado invariável ao particionamento em blocos;
- nenhuma alocação ou lock depois de `prepare`;
- toda posição e todo parâmetro permanecem finitos e limitados;
- observação parcial é aplicada a predador, presa, território e mixagem;
- mudanças de Desire, Lupa, presença e blend não produzem clique;
- a física ecológica roda em taxa de controle fixa e o áudio recebe valores
  interpolados;
- áudio, modulação e observação usam rotas diferentes;
- primeiro plano e estrutura de sustentação são papéis explícitos;
- segurança técnica de saída não é modulável pelo ecossistema.

## 7. Sequência segura de desenvolvimento

1. Congelar este arquivo como referência e escrever testes de conceito antes
   do substituto.
2. Construir apenas estado, relógio, movimento limitado e percepção, ainda sem
   áudio.
3. Validar seed, blocos irregulares, limites, CPU e milhares de ciclos.
4. Produzir pesos de mixagem para os quatro organismos já audíveis no
   `BiomaEngine`, sem criar espécies novas.
5. Fazer A/B com e sem Ecossistema e verificar se cardume, foco e perspectiva
   têm consequência sonora reconhecível.
6. Integrar uma relação ecológica por vez: desejo, ambiente, Lupa, presença,
   predador/presa e somente depois integridade.
7. Só então abrir a matriz para novos motores inspirados na pesquisa de
   módulos, cada um com dossiê, protótipo isolado, teste e escuta.

## 8. Critério de excelência

O Ecossistema não estará pronto porque contém muitas metáforas em classes. Ele
estará pronto quando poucas relações forem audíveis, reproduzíveis, seguras e
musicalmente profundas. Cada mecanismo precisa responder:

1. o que muda na escuta;
2. quem consegue perceber essa mudança;
3. qual estado ou energia a causa;
4. como ela permanece controlável sem virar automação rígida;
5. como é repetida por seed e comparada em A/B;
6. quais limites impedem instabilidade técnica;
7. se pertence ao primeiro plano ou apenas sustenta o comportamento.

O `Ecosystem.h` legado da raiz foi usado somente como fonte histórica de
linguagem e hipóteses durante esta auditoria. Depois da reconstrução canônica,
foi removido do repositório e nunca integrou o motor do Aquorbium.

## 9. Encaminhamento posterior à auditoria

O substituto começou em `src/core/Ecosystem.h/.cpp`. O arquivo auditado na raiz
foi removido após a reconstrução; este relatório mantém o registro histórico de
seus problemas e das decisões tomadas. O primeiro marco novo cobre relógio determinístico, geometria limitada,
percepção, observadores externos, desejo suavizado, Lupa, presença, integridade
causal e pesos de mixagem integrados aos quatro organismos atuais. O estado e
os limites dessa reconstrução estão em `ECOSYSTEM_DESIGN.md`.
