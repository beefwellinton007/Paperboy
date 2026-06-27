# After Dark — Recreation Project Plan

> A faithful, modern recreation of Berkeley Systems' **After Dark** screensaver
> suite for **Windows** and **macOS**, built on a shared engine with a module
> SDK so the catalog can grow indefinitely. First release ships a small set of
> flagship modules plus a **special debut module: the Paperboy game screensaver**
> (a playable/idle demo).

---

## 1. Goals

1. **Recreate the complete After Dark module set** across the historic releases
   (original After Dark, More After Dark, MultiModule Mania, and themed packs),
   in the spirit of the originals — not pixel-perfect rips, but recognizable,
   loving recreations.
2. **Two first-class platforms:** Windows (`.scr`) and macOS (`.saver`), from a
   single shared codebase.
3. **Room to add more:** a stable Module SDK + manifest so new modules drop in
   without touching the host. Third parties (and us) can ship module packs.
4. **A special first release** featuring an original module not in the classic
   set — the **Paperboy game screensaver** — as a demo of "playable" screensaver
   modules (a new module *category*, not just eye candy).
5. **Disciplined versioning & changelog** so every release is traceable and we
   can publish release notes the moment a build is confirmed production-ready.

---

## 2. Architecture

### 2.1 Layered design

```
+-----------------------------------------------------------+
|  Platform Hosts (thin shims)                              |
|   - Windows:  AfterDark.scr   (Win32 /S /P /C entry)      |
|   - macOS:    AfterDark.saver (ScreenSaverView subclass)  |
|   - Dev:      ad-harness      (standalone window/CLI)     |
+-----------------------------------------------------------+
|  AfterDark Core (shared, portable)                        |
|   - Module loader & lifecycle (load/init/tick/draw/free)  |
|   - Scheduler / "random module" + "MultiModule" mixer     |
|   - Config store (per-module settings, persisted)         |
|   - Input + idle/wake + multi-monitor + preview handling  |
|   - Asset pipeline (sprites, audio, fonts)                |
+-----------------------------------------------------------+
|  Render & Platform Abstraction Layer (RAL)                |
|   - 2D drawing, sprites, text, audio, timing, RNG         |
|   - One portable backend; native windowing per OS         |
+-----------------------------------------------------------+
|  Modules (the screensavers)  -- where the catalog lives   |
|   flying-toasters/  starry-night/  fish/  mowing-man/ ... |
|   paperboy/   <-- special first-release demo module       |
+-----------------------------------------------------------+
```

### 2.2 Recommended tech stack

**Single shared engine in portable C++17** (alt: Rust) over a thin graphics
layer, with **native host shims** per OS:

- **Rendering:** portable 2D over GPU — recommend `sokol_gfx`/SDL2 (or raw
  Direct3D11 on Win + Metal on Mac if we want fully native). Most After Dark
  modules are 2D sprite/vector animations, so a 2D-first renderer keeps modules
  simple.
- **Windows host:** a `.scr` is just an `.exe` honoring `/s` (run), `/p HWND`
  (preview), `/c` (config). Host embeds Core, renders into the screensaver/preview
  HWND.
- **macOS host:** a `.saver` bundle subclassing `ScreenSaverView`; host embeds
  Core, renders into the view, exposes `configureSheet`.
- **Dev harness:** a plain desktop window + hotkeys to load any module instantly
  — this is where 95% of module development happens (no need to install a
  screensaver to iterate).

> **Decision (locked):** **portable backend** — one shared engine over
> SDL2/sokol with native host shims per OS — chosen for speed of building the
> whole catalog. We revisit only if a specific module needs OS-specific tricks.
> Alternatives considered and set aside: fully native per-OS renderer (D3D11 +
> Metal — ~2x rendering work) and a web-tech engine (Canvas/WebGL in an embedded
> webview — easiest authoring but heaviest host).

### 2.3 Module SDK & manifest

Every module is a folder with a manifest and an implementation of a small,
stable interface:

```
modules/paperboy/
  module.json        # id, name, version, category, settings schema, assets
  src/               # module code against the Core API
  assets/            # sprites, audio, fonts
```

Core ABI/interface (conceptual):

```c
typedef struct ADModule {
  void  (*init)(ADContext*);          // alloc state, load assets
  void  (*tick)(ADContext*, double dt);// advance simulation
  void  (*draw)(ADContext*, ADCanvas*);// render one frame
  bool  (*input)(ADContext*, ADEvent*);// optional (interactive modules)
  void  (*config)(ADContext*, ADUI*);  // optional settings UI
  void  (*free)(ADContext*);
} ADModule;
```

`category` distinguishes classic **ambient** modules from new **interactive**
ones (like Paperboy). The settings schema drives the config UI on both OSes
from one declaration.

---

## 3. Module catalog (recreation targets)

Grouped roughly by original release. Phase 1 ships the bolded "flagship" set;
the rest are scheduled across later phases. This list is the working backlog —
we add/trim as we go.

**Flagship (Phase 1):**
- **Flying Toasters** (winged toasters + toast)
- **Starry Night** (skyline + twinkling stars/shooting stars)
- **Fish! / Aquatic Realm** (aquarium)
- **Mowing Man** (mows the screen clear)
- **Warp / Hyperspace** (starfield)

**Original After Dark / More After Dark (Phase 2+):**
- Spotlight, Globe, Rat Race, Boris (the cat), Nirvana, Messages,
  Confetti Factory, Hard Rain, Puzzle, Bad Dog!, Satori, Down the Drain,
  Gravity, Lunatic Fringe, String Theory, Can of Worms, Meadow, Clocks.

**MultiModule Mania / later packs (Phase 3+):**
- MultiModule mixer (run several at once — supported by Core scheduler).

**Themed packs (Phase 4+, behind their own pack manifests):**
- Star Trek / TNG style, Disney style, Looney Tunes style, Simpsons style,
  Marvel style, Totally Twisted. (Recreations/parodies; we use original art to
  avoid IP issues — see §7.)

**New original modules (our additions):**
- **Paperboy** — the special first-release demo (see §4).
- Open slot for community/our future modules via the SDK.

> We will not reuse Berkeley Systems / licensed artwork. Each module is an
> original recreation of the *behavior and vibe*. (See §7 Legal.)

---

## 4. Special first release: Paperboy game screensaver

A debut **interactive/idle** module that doubles as a playable demo — proving the
"games as screensavers" category.

- **Idle (screensaver) mode:** an AI paperboy auto-rides the street, tossing
  papers onto porches, dodging obstacles — fully autonomous eye candy when no one
  touches the keyboard.
- **Demo/playable mode:** in the dev harness and (optionally) when launched from
  config preview, arrow keys / mouse let you steer and throw. Any real input in
  true screensaver mode still dismisses the saver (OS requirement), so "playable"
  lives in the harness + a windowed demo build.
- **Scope for v2.0:** one street, paper-tossing, a few obstacles/houses, score
  overlay, day→dusk palette cycle. Deliberately a *demo* — not the full arcade
  game.
- Establishes reusable Core systems (sprite animation, tilemap/scroller, input,
  HUD/score) that later interactive modules reuse.

---

## 5. Versioning

- **Suite version starts at `2.0`** (honoring that classic After Dark reached
  v4.x — we're a fresh line, debuting at 2.0).
- Scheme: `2.0`, then `2.01`, `2.02`, … for incremental releases; `2.1`, `2.2`
  for larger feature drops; `3.0` for a major architectural leap.
- **Per-module versions** are tracked independently in each `module.json`
  (e.g., `paperboy 1.0.0`) so module updates don't force a suite bump — the
  suite version reflects the packaged release.
- Git tags: `v2.0`, `v2.01`, … on the confirmed production-ready commit.

---

## 6. Roadmap / phases

| Phase | Version | Deliverable |
|------|---------|-------------|
| 0 | (pre-2.0) | Core engine, RAL, Module SDK, dev harness, CI building `.scr` + `.saver`. |
| 1 | **2.0** | Host shims + flagship modules (Toasters, Starry Night, Fish, Mowing Man, Warp) + **Paperboy** demo. First production-ready release. |
| 2 | 2.01–2.0x | Original/More After Dark module backlog; bug fixes; config polish. |
| 3 | 2.1 | MultiModule mixer; module-pack installer; more modules. |
| 4 | 2.2+ | Themed packs (original-art recreations); community SDK docs. |

Each phase ends with: green CI, signed/notarized installers (Win + Mac),
QA pass, and a published changelog entry.

---

## 7. Legal / IP notes

- **No original Berkeley Systems assets or code.** All art, audio, and animation
  are original recreations.
- "After Dark," "Flying Toasters," and themed franchises (Star Trek, Disney,
  etc.) are trademarks of their owners. Themed packs are **parody/homage with
  original art**; we'll confirm naming/branding and likely ship under our own
  suite name with modules described as "in the style of." Final naming decision
  before public release.

---

## 8. Build, packaging, QA

- **CI:** matrix build (Windows + macOS), runs the harness in headless smoke mode
  per module.
- **Packaging:** Windows `.scr` + installer (signed); macOS `.saver` (signed +
  notarized), distributed in a `.dmg`.
- **QA checklist per release:** multi-monitor, high-DPI/Retina, preview pane,
  config persistence, wake-on-input, CPU/GPU/idle behavior, no memory leaks over
  long runs.

---

## 9. Definition of "production-ready" (gate for publishing the changelog)

A version is production-ready when: all targeted modules pass QA on both OSes;
installers are signed/notarized; no known crashers or leaks; config + preview
work on both platforms; and CI is green. Only then do we tag, ship, and post the
corresponding `CHANGELOG.md` entry.
