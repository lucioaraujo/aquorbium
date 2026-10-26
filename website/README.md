# Site do Aquorbium

**Estado: em preparação (2026-10-26).** A página vai para
<https://lucioaraujo.github.io/aquorbium/> (GitHub Pages do repositório
próprio; `.github/workflows/deploy-site.yml`, que só roda quando o site
muda). Enquanto a v0.1.0 não sai, as páginas têm `noindex`, o selo "em
preparação" e os botões de baixar levam à página de releases
(`PUBLICADO = False` no `gerar_paginas.py`). No lançamento: `PUBLICADO =
True`, gerar de novo, e preparar o card do portal RASGO em `RASGO_WEBSITE/`
(o portal só lista instrumentos lançados; o envio do portal é do autor).

## Como gerar

```sh
python3 gerar_paginas.py && python3 seo.py
```

- `gerar_paginas.py` é a fonte única: o modelo e os textos dos quatro
  idiomas. Gera `index.html` (português, canônica), `en.html`, `fr.html` e
  `es.html`. **Os .html são derivados.** Editar os textos no script.
- `seo.py` injeta os metadados (descrição, Open Graph, hreflang, JSON-LD) e
  escreve o `sitemap.xml`. É o código comum da família; só a configuração do
  topo é do Aquorbium.

## Conteúdo e regras

- A faixa fixa `rasgo-strip` aponta para o portal; o cabeçalho próprio vem
  logo abaixo, com o símbolo e o lettering aprovados. Regra da família RASGO.
- As fontes são livres e embutidas (DejaVu), nunca fontes de sistema
  nomeadas; a licença delas viaja em `assets/fonts/LICENSE-DejaVu.txt`.
- O contato usa o diálogo de copiar o endereço, não `mailto:`.
- A paleta vem da interface (`src/app/Main.cpp`, `Palette`): o ciano do sonar
  e as cores dos cinco organismos.
- **O que é próprio desta página** (o autor: "na página web própria pode
  inovar"), em `assets/sonar.js`:
  - o **sonar vivo** no topo: o símbolo da identidade animado, os cinco
    organismos nadando, o feixe acendendo cada um, as linhas de relação;
    com "reduzir movimento", um quadro parado;
  - **a semente desta visita**: um mundo e um bioma sorteados no navegador,
    na ideia da semente de abertura do app; declarado como ilustração, sem
    som e sem rodar o código do app.

## Proveniência

Cópias fixadas do site do Trioio (`lucioaraujo/trioio`, commit `383c009`),
o mesmo autor e a mesma licença:

| Arquivo | sha256 do original | Adaptação |
|---|---|---|
| `assets/fonts/dejavu-sans-400.woff2` | `78a6fff8c1c80c36308959c615fc8be4c1fc0382ebbffbfa3090d22dd7071e8a` | nenhuma; DejaVu (Bitstream Vera, livre) |
| `assets/fonts/dejavu-sans-700.woff2` | `1a1984b5e3fa20c8b2b292d65ead462134d2d970377438888d485e108294a248` | idem |
| `assets/fonts/dejavu-sans-mono-400.woff2` | `07bed8326332033d814124dc5ab4cbb1171432caffb5fb150d4ac674676ba05a` | idem |
| `assets/fonts/dejavu-sans-mono-700.woff2` | `8a1e8269d844898145b1f8dbbe6e2221f0e061cdc1b10be8eccfb184fdca2782` | idem |
| `assets/contact.js` | `55f650ea3bd79a2ccf0af306489bf1fdc90131d6005626416e001f6f92783d58` | nenhuma |
| `seo.py` | `3772e6175490779a40939add16454c9aebb9b9aea4c91c06faaf7af8899a0990` | a configuração do topo |
| `../.github/workflows/deploy-site.yml` | `5ee91e1db3d44b2e9de484c5ad32333f6ef9a5b4afb29dba79798b486629413c` | o cabeçalho |

`assets/fonts/LICENSE-DejaVu.txt` vem do pacote Debian `fonts-dejavu-core`
(`/usr/share/doc/fonts-dejavu-core/copyright`). A licença da Bitstream Vera
pede que o aviso acompanhe as fontes; os sites irmãos ainda não o trazem.

## Imagens (derivadas, próprias)

| Arquivo | Origem |
|---|---|
| `assets/interface.jpg` | o app tocando, em inglês (semente 228228182), 2026-10-26, JPEG q84; o mesmo de `docs/assets/aquorbium-interface.jpg` |
| `assets/og-aquorbium.jpg` | 1200×630, recorte central de `assets/interface.jpg` |
| `assets/identity/*.svg` | cópias de `../assets/identity/` (a identidade aprovada em 2026-10-25) |
