# Aquorbium — identidade visual

Atualização de 2026-10-25. A identidade visual final foi aprovada pelo autor:
lockup horizontal transparente, sem degradê ou caixa retangular, com o sonar
circular menor ao lado do lettering original AQUORBIUM. O estilo mantém a
paleta escura do app e a referência de instrumentos analógicos de painel naval.

## Arquivos finais

- `aquorbium-logo-sonar-texto-compacta.svg` e `.png`: lockup horizontal
  escolhido. O SVG é a fonte vetorial canônica para a composição completa.
- `aquorbium-symbol.svg`: sonar sem texto, para ícone e usos quadrados.
- `aquorbium-wordmark.svg` e `.png`: lettering vetorial original, sem fonte
  externa. São contornos desenhados para a palavra AQUORBIUM, não uma família
  tipográfica completa.
- `aquorbium-favicon.svg`, `aquorbium-favicon-64.png`, `-32.png` e `-16.png`:
  favicons simplificados para tamanhos pequenos.
- `aquorbium-1024.png`, `-512`, `-256`, `-128`, `-64`, `-48`, `-32`, `-24`,
  `-16` e `aquorbium.ico`: ícones derivados de `aquorbium-symbol.svg`.

Os SVGs são vetoriais e sem degradê. A composição final traz a fatia ciano do
sonar mais clara, quatro ecos independentes (âmbar, coral, violeta e verde),
ponteiro ciano e eixo/índice em metal claro. Não há halos em torno dos ecos nem
linhas onduladas. Os PNGs e o ICO foram exportados localmente do SVG com
Inkscape/ImageMagick; não contêm fontes ou imagens de terceiros.

## Integração no aplicativo

A identidade visual foi aprovada e os ativos foram gerados, mas ainda não foi
integrada ao cabeçalho do JUCE. `src/app/Main.cpp` continua carregando o
`aquorbium-logo.svg` anterior como símbolo e desenhando o título com a fonte da
interface. A próxima etapa de integração deve usar `aquorbium-symbol.svg` para
o cabeçalho/ícone e `aquorbium-wordmark.svg` para o nome, respeitando o espaço
do layout. Os ícones PNG usados pelo CMake agora são derivados do símbolo
final.

Não restaurar as variantes rejeitadas pelo autor. O arquivo
`aquorbium-logo.svg` anterior foi mantido separado por enquanto porque o código
do cabeçalho ainda o referencia; ele não é a identidade final aprovada.
