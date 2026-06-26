# After Dark — Recreation Suite

A modern, cross-platform (Windows + macOS) recreation of the classic **After
Dark** screensaver collection, built on a shared engine with a Module SDK so the
catalog can keep growing.

The debut release (**2.0**) ships flagship recreations (Flying Toasters, Starry
Night, Fish!, Mowing Man, Warp) plus a special original module: the **Paperboy**
game screensaver — a demo of playable/idle "game" modules.

- 📋 Project plan & architecture: [`PLAN.md`](./PLAN.md)
- 📝 Changelog (published per production-ready release): [`CHANGELOG.md`](./CHANGELOG.md)

> Status: **pre-2.0, in development.** Nothing has shipped yet.

## Repository layout (planned)

```
core/        AfterDark Core engine + Render/Platform Abstraction Layer
hosts/
  windows/   .scr host shim
  macos/     .saver host shim
  harness/   standalone dev runner (fast module iteration)
modules/
  paperboy/  special first-release game-screensaver demo
  ...        flagship + classic recreations
sdk/         Module SDK headers, manifest schema, docs
```

All artwork and code are original recreations — no Berkeley Systems assets are
used (see PLAN.md §7).
