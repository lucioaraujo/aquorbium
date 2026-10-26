// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Lúcio de Araújo
//
// The Aquorbium page's own two pieces (2026-10-26; the author: "na página web
// própria pode inovar"):
//   - the SONAR, drawn live: the identity's sonar comes alive - the beam
//     turns, the five organisms swim in the sphere in the app's colours, the
//     beam lights each one as it passes, and lines of relation appear when
//     two come close (coral a hunt, amber a flight, cyan a school, pink a
//     courtship). With "reduce motion", one still frame.
//   - the SEED OF THIS VISIT: a seed drawn in the browser, and from it a world
//     and a biome in the spirit of the app's opening seed
//     (src/core/SessionGenesis.h, src/core/OpeningSeed.h). An illustration:
//     it does not run the app's code and plays no sound.
(() => {
  const css = getComputedStyle(document.documentElement);
  const colour = (name) => css.getPropertyValue(name).trim() || '#50ded5';

  // a small seeded generator (splitmix32), so a seed always shows the same world
  const generator = (seed) => {
    let s = seed >>> 0;
    return () => {
      s = (s + 0x9e3779b9) >>> 0;
      let z = s;
      z = Math.imul(z ^ (z >>> 16), 0x85ebca6b) >>> 0;
      z = Math.imul(z ^ (z >>> 13), 0xc2b2ae35) >>> 0;
      return ((z ^ (z >>> 16)) >>> 0) / 4294967296;
    };
  };

  // ---- the sonar ----------------------------------------------------------
  const canvas = document.querySelector('[data-sonar]');
  if (canvas && canvas.getContext) {
    const g = canvas.getContext('2d');
    const still = window.matchMedia('(prefers-reduced-motion: reduce)').matches;
    const names = ['MORPH', 'GRAIN', 'MODAL', 'PULSAR', 'STRING'];
    const tints = ['--cyan', '--amber', '--coral', '--violet', '--green'].map(colour);
    // who relates to whom, and how (the app's colours of relation)
    const relations = [
      [0, 3, colour('--coral')],   // the predator hunts the prey
      [3, 0, colour('--amber')],   // the prey flees
      [1, 4, colour('--cyan')],    // a school
      [2, 4, colour('--pink')],    // a courtship
    ];
    const random = generator((Math.random() * 4294967296) >>> 0);
    const fish = names.map((n, i) => {
      const a = random() * Math.PI * 2, r = 0.25 + 0.5 * random();
      return { x: Math.cos(a) * r, y: Math.sin(a) * r, vx: 0, vy: 0, glow: 0, i };
    });
    let size = 0, beam = random() * Math.PI * 2, last = performance.now();

    const fit = () => {
      const ratio = window.devicePixelRatio || 1;
      const box = canvas.getBoundingClientRect();
      size = Math.max(160, Math.min(box.width, box.height || box.width));
      canvas.width = Math.round(size * ratio);
      canvas.height = Math.round(size * ratio);
      g.setTransform(ratio, 0, 0, ratio, 0, 0);
    };

    const step = (dt) => {
      beam = (beam + dt * Math.PI * 2 / 6) % (Math.PI * 2);   // a turn in 6 s
      for (const f of fish) {
        // a slow random swim, held inside the sphere by a soft wall
        f.vx += (random() - 0.5) * 0.35 * dt;
        f.vy += (random() - 0.5) * 0.35 * dt;
        const r = Math.hypot(f.x, f.y);
        if (r > 0.78) { f.vx -= f.x * 0.9 * dt; f.vy -= f.y * 0.9 * dt; }
        f.vx *= 1 - 0.6 * dt; f.vy *= 1 - 0.6 * dt;
        f.x += f.vx * dt; f.y += f.vy * dt;
        // the beam lights what it crosses
        let d = Math.abs(Math.atan2(f.y, f.x) - beam);
        d = Math.min(d, Math.PI * 2 - d);
        f.glow = d < 0.12 ? 1 : f.glow * Math.exp(-dt * 1.4);
      }
      // the hunter drifts towards its prey, the prey away
      const hunter = fish[0], prey = fish[3];
      hunter.vx += (prey.x - hunter.x) * 0.08 * dt; hunter.vy += (prey.y - hunter.y) * 0.08 * dt;
      prey.vx -= (hunter.x - prey.x) * 0.05 * dt; prey.vy -= (hunter.y - prey.y) * 0.05 * dt;
    };

    const draw = () => {
      const c = size / 2, R = size * 0.46;
      g.clearRect(0, 0, size, size);
      g.fillStyle = colour('--panel');
      g.beginPath(); g.arc(c, c, R, 0, Math.PI * 2); g.fill();
      g.strokeStyle = colour('--line'); g.lineWidth = 1;
      for (const k of [0.33, 0.66, 1]) { g.beginPath(); g.arc(c, c, R * k, 0, Math.PI * 2); g.stroke(); }
      g.beginPath(); g.moveTo(c - R, c); g.lineTo(c + R, c); g.moveTo(c, c - R); g.lineTo(c, c + R); g.stroke();
      // the beam and its fading wake
      const wake = g.createConicGradient ? g.createConicGradient(beam - 0.9, c, c) : null;
      if (wake) {
        wake.addColorStop(0, 'rgba(80, 222, 213, 0)');
        wake.addColorStop(0.9 / (Math.PI * 2), 'rgba(80, 222, 213, 0.22)');
        wake.addColorStop(0.9 / (Math.PI * 2) + 0.001, 'rgba(80, 222, 213, 0)');
        g.fillStyle = wake;
        g.beginPath(); g.moveTo(c, c); g.arc(c, c, R, beam - 0.9, beam); g.closePath(); g.fill();
      }
      g.strokeStyle = colour('--cyan'); g.lineWidth = 1.5;
      g.beginPath(); g.moveTo(c, c); g.lineTo(c + Math.cos(beam) * R, c + Math.sin(beam) * R); g.stroke();
      // relations, when two are close
      for (const [a, b, tint] of relations) {
        const p = fish[a], q = fish[b];
        const d = Math.hypot(p.x - q.x, p.y - q.y);
        if (d > 0.7) continue;
        g.globalAlpha = Math.min(1, (0.7 - d) * 2.2);
        g.strokeStyle = tint; g.lineWidth = 1.2;
        g.beginPath(); g.moveTo(c + p.x * R, c + p.y * R); g.lineTo(c + q.x * R, c + q.y * R); g.stroke();
      }
      g.globalAlpha = 1;
      g.font = '600 ' + Math.max(9, size * 0.028) + 'px "DejaVu Sans Mono", monospace';
      for (const f of fish) {
        const x = c + f.x * R, y = c + f.y * R, tint = tints[f.i];
        g.globalAlpha = 0.25 + 0.35 * f.glow;
        g.fillStyle = tint;
        g.beginPath(); g.arc(x, y, size * (0.035 + 0.02 * f.glow), 0, Math.PI * 2); g.fill();
        g.globalAlpha = 1;
        g.beginPath(); g.arc(x, y, size * 0.014, 0, Math.PI * 2); g.fill();
        g.fillStyle = colour('--muted');
        g.fillText(names[f.i], x + size * 0.03, y - size * 0.025);
      }
      g.strokeStyle = colour('--steel'); g.lineWidth = 2;
      g.beginPath(); g.arc(c, c, R, 0, Math.PI * 2); g.stroke();
    };

    const frame = (now) => {
      const dt = Math.min(0.05, (now - last) / 1000);
      last = now;
      step(dt);
      draw();
      if (!document.hidden) requestAnimationFrame(frame);
    };
    fit();
    window.addEventListener('resize', () => { fit(); draw(); });
    if (still) { for (let k = 0; k < 120; ++k) step(1 / 30); draw(); }
    else {
      document.addEventListener('visibilitychange', () => {
        if (!document.hidden) { last = performance.now(); requestAnimationFrame(frame); }
      });
      requestAnimationFrame(frame);
    }
  }

  // ---- the seed of this visit ---------------------------------------------
  const box = document.querySelector('[data-seed]');
  const textNode = document.querySelector('[data-seed-text]');
  if (box && textNode) {
    const t = JSON.parse(textNode.textContent);
    const keys = ['C', 'C#', 'D', 'D#', 'E', 'F', 'F#', 'G', 'G#', 'A', 'A#', 'B'];
    const el = (tag, cls, text) => {
      const n = document.createElement(tag);
      if (cls) n.className = cls;
      if (text !== undefined) n.textContent = text;
      return n;
    };
    const show = (seed) => {
      const r = generator(seed);
      const pick = (list) => list[Math.floor(r() * list.length) % list.length];
      const gene = () => 0.5 * (r() + r());
      box.replaceChildren();
      box.append(el('p', 'seed-number', t.seed + ' ' + seed));
      // the world
      const world = el('div', 'seed-col');
      world.append(el('h3', '', t.world));
      const polytonal = r() < 0.33;
      world.append(el('p', 'seed-world', pick(keys) + ' ' + pick(t.modes) + ' · ' + pick(t.languages)));
      if (polytonal) world.append(el('p', '', t.polytonal));
      world.append(el('p', '', t.arc + ': ' + pick(t.arcs) + ', ' + Math.round(4 + 4 * r()) + ' ' + t.minutes));
      // the biome
      const biome = el('div', 'seed-col');
      biome.append(el('h3', '', t.biome));
      const sea = r() < 0.65;
      const list = el('ul', 'genes');
      t.genes.forEach((name) => {
        const v = name === t.genes[t.genes.length - 1] ? Math.min(1, Math.max(0, gene() + (sea ? 0.15 : -0.15))) : gene();
        const li = el('li');
        li.append(el('span', 'gene-name', name));
        const bar = el('span', 'gene-bar');
        const fill = el('span', 'gene-fill');
        fill.style.width = Math.round(v * 100) + '%';
        bar.append(fill);
        li.append(bar);
        list.append(li);
      });
      biome.append(list);
      biome.append(el('p', '', sea ? t.sea : t.fresh));
      // which modules wake
      const wake = el('div', 'seed-col');
      wake.append(el('h3', '', t.wake));
      const order = t.modules.map((m, i) => [r(), m]).sort((a, b) => a[0] - b[0]).map((x) => x[1]);
      const n = 1 + Math.min(3, Math.floor(r() * 2.2 + r() * 1.4));
      const ul = el('ul', 'awake');
      order.slice(0, n).forEach((m) => ul.append(el('li', '', m)));
      wake.append(ul);
      box.append(world, biome, wake);
    };
    const fresh = () => (Math.floor(Math.random() * 0x7ffffffe) + 1);
    show(fresh());
    document.querySelector('[data-seed-again]')?.addEventListener('click', () => show(fresh()));
  }
})();
