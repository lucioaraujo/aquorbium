# Changelog

## Unreleased

- **Streaming version at −14 LUFS** (saida-ref 1.1, the family's common
  technical output): next to each recording, `_streaming.wav` (−14 LUFS, true
  peak ≤ −1 dBTP, 24-bit PCM with TPDF) and a `.txt` with the measurements,
  made offline on a thread of its own. The live sound and the raw recording
  do not change. Dynamics guard: the limiter never takes more than 6 dB; a
  take too dynamic for that stays under −14 and the `.txt` says why.
  The recording report keeps saying what each delivery profile would do to
  the raw take.

## 0.2.0 — 2026-11-16

- **Maestruc receiver** (the RASGO family's conductor, protocol
  `maestruc.link/1`, local OSC only): with `--maestruc` or
  `AQUORBIUM_MAESTRUC=1`, Maestruc can pause and resume the biome (the same
  PAUSE button) and read its state and session seed. Off by default:
  without the option nothing changes. Seed and tempo while open answer
  "capability absent": the session's world is born from the seed before the
  audio starts, and the tempo belongs to the ecosystem. Copy of
  `MAESTRUC/receptor/MaestrucLink.h`; `juce_osc` module in the app.
- **Sound bridge to Maestruc** (Windows, macOS and Linux, no virtual audio
  driver): with `--maestruc`, the mix goes to Maestruc through a shared-memory
  file only the user can read; while Maestruc is reading, the Aquorbium
  silences its own output (no double sound) and, if Maestruc closes or stops,
  plays on its own again within about half a second. Takes, stems,
  Ambisonics and the diagnostic capture are unchanged. Bridges left by an
  instrument that was killed are removed the next time one opens. Copy of
  `MAESTRUC/receptor/PonteDeSom.h`.
- **Master from Maestruc:** the output volume (−18 to +6 dB, the same as the
  header volume and OUTPUT TRIM) and SILENCE; the Aquorbium has no per-channel
  mixer, so only the master is offered. Changes made by hand are sent back.
- **Session password:** Maestruc's commands are only accepted with the
  password it writes to a file only the user can read.
- **`--seed N`** (1 to 2147483646) opens the session with that seed without
  saving it as fixed (`session.fixedSeed` keeps its meaning).
- **JUCE 9.0.3** for local builds too (the CI already used it; the local
  build was on 9.0.1).
- `./run` opens `build/` (the folder in INSTALL.md); the older
  `build-reactivation-20260915` no longer configures after the project
  moved and stays only as a fallback.

## 0.1.0 — first public version, 2026-10-27

The first public version of the Aquorbium, an audible biome: five sound
organisms (MORPH, GRAIN, MODAL, PULSAR, STRING) living in a sphere of water.

- **Every opening is another piece.** The session's seed draws a world - a
  key and a mode, a harmonic language (functional, modal, Brazilian,
  impressionist, chromatic) with modulations and polytonal episodes, the
  registers, the orbits, the pace, an arc of tension - and a biome: the
  water, the light, the population, which modules wake, each organism's
  body, the ecology and the synthesis voices.
- **VARY:** the controls move slowly by themselves over the music's time,
  around where the seed or the musician left them; the climax of the form
  warms the water, rest lets it settle.
- **The synthesis device:** one semi-modular voice per organism, played by
  the ecosystem's notes, with an ecological modulation matrix and PROPOSE.
- **Space and output:** the aquarium space, the tail, the environment, the
  creative MASTER, the output stage with loudness and true-peak metering,
  binaural headphones and an AmbiX recording.
- **Interface** in English, Portuguese, French and Spanish, with TUTORIAL and
  LEARN.
- **Packages** for Windows (installer and portable .zip), macOS (Universal 2,
  sealed ad hoc) and Linux (.deb, AppImage, .tar.gz). See `INSTALL.md`.

The development history up to 2026-10-26 is kept in the author's private
monorepo (`rasgo-instruments`, last Aquorbium commit `02292a1`).
