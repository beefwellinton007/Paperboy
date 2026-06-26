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
