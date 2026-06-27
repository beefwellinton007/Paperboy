# Changelog

All notable changes to the After Dark recreation suite are documented here.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).
This project uses suite-level versions (`2.0`, `2.01`, …) as defined in
[`PLAN.md`](./PLAN.md). **Entries stay under `[Unreleased]` until a version is
confirmed production-ready** (see PLAN.md §9), at which point the section is
renamed to the version, dated, tagged, and published.

> Nothing below has shipped yet — `2.0` is in development.

## [Unreleased]

### Added
- Project plan (`PLAN.md`): architecture, module catalog, versioning scheme,
  roadmap, and the production-ready gate.
- This changelog.

### Decided
- Engine stack: **portable backend** (one shared C++/Rust engine over
  SDL2/sokol with native host shims per OS).

### Added (Phase 0 scaffold — in progress)
- **Module SDK** (`sdk/include/afterdark/afterdark.h`): Canvas, Event, Rng,
  Context, and the `Module` interface every screensaver implements.
- `module.json` manifest schema (`sdk/schema/module.schema.json`) — drives the
  catalog and per-platform config UI.
- **AfterDark Core**: module `Registry` + Render/Platform Abstraction Layer with
  a headless backend (CI/no-display) and an SDL2 backend (windowed).
- **Paperboy** module — the special first-release demo: side-scrolling delivery,
  paper-toss physics, obstacle dodging, day→dusk→night palette cycle, score HUD;
  autonomous ambient mode + interactive `--play` mode.
- `_template` module (a working Warp-style starfield) as the copy-me starting
  point for new modules.
- **Dev harness** (`ad-harness`): load any module instantly; `--headless`
  doubles as the per-module CI smoke test.
- Native host stubs for Windows `.scr` and macOS `.saver`.
- CMake build and GitHub Actions CI (build + headless smoke on Linux/macOS/Windows).
- **Text rendering** (`sdk/include/afterdark/text.h`): built-in 3x5 bitmap font
  drawn via `fill_rect`, works on every backend with no asset files. Paperboy's
  score HUD now shows real text (and on-screen controls in `--play` mode).
- **Flying Toasters** module — first flagship recreation: winged toasters and
  toast drift diagonally across a night sky, each flyer with its own wing-flap
  phase. Establishes the per-entity sprite-animation pattern for the catalog
  (original art — no Berkeley Systems assets).
- **Starry Night** module — flagship recreation: gradient night sky, twinkling
  stars, shooting stars, and a city skyline with flickering lit windows.
  Establishes the layered/parallax drawing pattern (original art).
- **Boid steering helper** (`sdk/include/afterdark/steering.h`): reusable Vec2
  math + Reynolds flocking (separation/alignment/cohesion) for autonomous-agent
  modules.
- **Fish! (Aquatic Realm)** module — flagship recreation: a school of fish
  flocking via the steering helper, rising bubbles, swaying seaweed, sandy
  bottom. First consumer of the shared steering helper (original art).
- **Mowing Man** module — flagship recreation: a man pushes a mower back and
  forth, clearing tall grass into striped mowed rows, then the lawn regrows and
  he restarts. Establishes the persistent-field pattern (module-owned mowed grid
  redrawn from state, coalesced into runs; no offscreen buffer needed).
- **Warp (Hyperspace)** module — recreation: a 3D starfield with motion-blur
  streaks that lengthen with speed and surge into hyperspace bursts.
- **Spotlight** module — recreation: a roving spotlight on a Lissajous path
  reveals a hidden scene (a tiled floor + an "AFTER DARK" sign) only where the
  beam falls, via a per-scanline shadow mask.
- **Globe** module — recreation: a rotating wireframe Earth with continents,
  drawn via 3D rotation + orthographic projection (first 3D-math module).
- **Boris the Cat** module — recreation: a cat strolls, sits to wash, and naps,
  driven by a small Walk/Sit/Sleep state machine.
- **Rat Race** module — recreation: mice scurry laps around an oval track with
  random speed bursts; the leader wears a crown.
- **Messages** module — recreation: text drifts and bounces around the screen,
  changing color on each bounce and cycling through a message list (user text
  arrives once the config UI is wired).
- **Clocks** module — recreation: a grid of analog clocks ticking from
  different start times; adds a small reusable line-draw helper.
- **Confetti Factory** module — recreation: bursts of confetti arc up, flutter,
  and fall under gravity (density setting via the config schema).
- **Gravity** module — recreation: trailed balls bounce with restitution and
  re-kick so motion never settles (ball-count setting via the config schema).
- **Hard Rain** module — recreation: diagonal rain with splashes and occasional
  lightning flashes (intensity + lightning settings via the config schema).
- **Satori** module — recreation: evolving kaleidoscopic rose-curve art with
  cycling color (symmetry + fade settings via the config schema).
- **Down the Drain** module — recreation: particles spiral into a central drain
  endlessly (swirl-speed setting via the config schema).
- **String Theory** module — recreation: animated string-art envelopes between
  Lissajous-moving anchors with cycling color (strings-count setting).
- **Bad Dog!** module — recreation: a dog romps around leaving fading paw prints
  and barking now and then.

### Platform hosts (in progress)
- **Shared HostSession** (`core/include/afterdark/host.h`): step-based per-frame
  driver (handle events, tick + draw one frame) so the harness and native hosts
  share one tested code path. Harness refactored onto it.
- **SDL backend modes**: added fullscreen and embedded (wrap an existing native
  window) variants alongside the windowed one, for the screensaver hosts.
- **Windows `.scr` host** implemented: parses `/s` `/p <HWND>` `/c`, runs
  fullscreen or in the preview pane via the embedded SDL backend, dismisses on
  input. (Settings dialog + config persistence still TODO.)
- **macOS `.saver` host** implemented: `ScreenSaverView` subclass with a Core
  Graphics-backed Canvas driven by HostSession. (Configure sheet + persistence
  still TODO.)
- CMake builds each native host only on its platform; CI installs SDL2 on
  Linux/macOS so the windowed backend and `.saver` compile there.
  (Native hosts build/verify on Windows/macOS runners, not in this Linux env.)

### Graphics polish (in progress)
- **Alpha blending** in the renderer (image + SDL backends; Core Graphics already
  honored it) — `Color.a` now composites src-over.
- **Draw helpers** (`sdk/afterdark/draw.h`): vertical gradients, feathered
  `fill_circle`, soft `glow`, thick `line` — the "smooth/modern" toolkit.
- **Sprite system** (`sdk/afterdark/sprite.h`): pixel-art authored inline as
  text grids + palette, blitted with per-pixel alpha, scale, and flip — no image
  files/decoders needed.
- Showcase upgrades: **Flying Toasters** now uses hand-drawn toaster/toast/wing
  sprites on a gradient sky; **Starry Night** bright stars now twinkle with a
  soft glow. (Rollout to the rest of the catalog continues.)

### Previews / tooling
- **Image backend** (`make_image_backend`): rasterizes frames to RGB and writes
  PPM — dependency-free preview rendering without a display. Harness gains
  `--render <prefix>` / `--stride`.
- **Gallery generator** (`tools/make_gallery.py`, `tools/preview.sh`): renders
  every module and builds a captioned `montage.png` + an animated HTML flipbook
  (pure stdlib — PPM→PNG via zlib). Headline image committed at
  `docs/preview/montage.png`.

### Configuration
- **Settings store** (`ad::Settings` in `sdk/afterdark.h`, on `Context`): typed
  key/value config (bool/int/float/enum/string) read by modules with fallback
  defaults; keys match each `module.json` `settings` schema.
- HostSession applies settings before `init()`; harness gains `--set key=value`
  (repeatable) to drive config in dev/CI.
- **Paperboy** now reads `scroll_speed`, `day_length`, and `show_score` from
  settings — the first end-to-end manifest→config→behavior path.
- Native hosts have a `load_settings()` hook wired in; reading persisted user
  choices (Windows registry / macOS ScreenSaverDefaults) is the remaining TODO.
- **Config schema in code** (`SettingDesc` + `Module::settings_schema()`): modules
  declare their settings (type, default, range, enum options) in C++ — one
  runtime source of truth for both default-seeding and the config UI. HostSession
  seeds schema defaults (provided settings still win); harness `--config <module>`
  dumps the schema. Paperboy declares its schema; other modules follow as their
  settings are wired.

### Planned for 2.0 (first release)
- Shared **AfterDark Core** engine + Render/Platform Abstraction Layer.
- **Module SDK** and `module.json` manifest format (room to add more modules).
- Platform hosts: Windows `.scr` and macOS `.saver`, plus a dev harness.
- Flagship recreations: Flying Toasters, Starry Night, Fish!/Aquatic Realm,
  Mowing Man, Warp/Hyperspace.
- **Special debut module — Paperboy:** an interactive/idle game-style
  screensaver demo (new "playable module" category).
- Per-module config UI driven by the manifest settings schema.

---

### Versioning quick reference
- `2.0` — first production-ready release.
- `2.01`, `2.02`, … — incremental releases (fixes, added modules).
- `2.1`, `2.2` — larger feature drops.
- `3.0` — major architectural change.
- Each module also carries its own version in its `module.json`.
