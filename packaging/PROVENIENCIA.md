# Proveniência do empacotamento

Os scripts de `linux/` (`empacotar.sh`, `install.sh`, `testar-arranque.sh`)
foram **copiados** do Trioio (`lucioaraujo/trioio`, commit `383c009`,
`packaging/linux/`) em 2026-10-26. O Trioio, por sua vez, os copiou do
Antitotem (commit `6c88ab4`) em 2026-10-18. Eles são a versão genérica do
padrão multiplataforma do RASGO
(`RASGO_DOCUMENTATION/PADRAO_DISTRIBUICAO_MULTIPLATAFORMA.md`, no
`rasgo-instruments`). A regra dos instrumentos autônomos manda copiar, nunca
depender da pasta irmã.

- **Autor:** Lúcio Araújo (os três projetos).
- **Licença:** AGPL-3.0-or-later (a mesma nos três).
- **Adaptação ao Aquorbium:**
  - `install.sh`: sem as linhas dos dados do Trioio (o Aquorbium leva tudo
    dentro do binário);
  - `empacotar.sh`: copia também `LICENSE-AGPL-3.0.txt`, o nome da licença
    aqui;
  - `aquorbium.desktop` e `README-tarball.txt` foram escritos para o
    Aquorbium, no formato dos do Trioio.
  - `testar-arranque.sh`: o app roda na sua própria sessão (`setsid`) e o
    fim do teste encerra o grupo inteiro. No AppImage com
    `APPIMAGE_EXTRACT_AND_RUN`, o app é filho do processo lançado, e o
    `kill` de um só deixava o app tocando depois do teste (achado em
    2026-10-26; o defeito existe também nas cópias do Antitotem e do
    Trioio).
- **Ferramenta de terceiros no caminho do release:** o `linuxdeploy`, só na
  CI, fixado por versão e SHA-256 no `empacotar.sh`. Não fica no
  repositório.

Os ícones vêm de `assets/identity/` (a identidade aprovada pelo autor em
2026-10-25; `assets/identity/IDENTIDADE.md`).

Checksums dos originais copiados (Trioio @ 383c009):

```text
c2dc50b470bf1f56aa46ad304552d8b1f3c513cecf0902b21e1133f18b51d814  empacotar.sh
31acc35b092a09d791e9129b0a3297c4ed5ab894fcd15075636a501cd93a856f  install.sh
52632772917a059840022982e4e7cabc51fc13782f56bf4026841db9ca3cc3c0  testar-arranque.sh
b72a786fe74e6227315408013e3249bbbf0440e3373c07bd617d7666332c58ee  README-tarball.txt
bfb103ba5b45831c03a62c40dd255ec9defd6ae0d2fe41058052a53291eb5334  trioio.desktop
```

O fluxo da CI (`.github/workflows/package.yml`) foi copiado do Trioio no
mesmo commit e adaptado; o cabeçalho dele diz o que mudou.

Checksum do fluxo original copiado (Trioio @ 383c009):

```text
27f49afb420ef18ea084ee32a8bd788dc6d22fa54531bb55931b0eb794d85c97  .github/workflows/package.yml
```
