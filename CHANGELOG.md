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
