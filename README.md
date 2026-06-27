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

## Repository layout

```
sdk/         Module SDK (afterdark.h interface) + module.json schema
core/        AfterDark Core: module registry + Render/Platform Abstraction Layer
               (SDL2 backend when available, headless backend always)
modules/
  paperboy/         special first-release game-screensaver demo
  flying_toasters/  flagship recreation (winged toasters)
  starry_night/     flagship recreation (skyline + stars)
  fish/             flagship recreation (flocking aquarium)
  mowing_man/       flagship recreation (mows the lawn)
  warp/             recreation (hyperspace starfield)
  spotlight/        recreation (roving beam reveal)
  globe/            recreation (rotating wireframe Earth)
  boris/            recreation (a wandering cat)
  rat_race/         recreation (mice racing a track)
  messages/         recreation (bouncing text)
  clocks/           recreation (grid of analog clocks)
  confetti/         recreation (confetti bursts)
  gravity/          recreation (bouncing balls)
  _template/        copy-me template (also a working starfield)
hosts/
  harness/   standalone dev runner + headless CI smoke tester
  windows/   .scr host (SDL-based; builds on a Windows runner)
  macos/     .saver host (Core Graphics; builds on a macOS runner)
```

## Build & run

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j

build/ad-harness --list                       # list modules
build/ad-harness --module paperboy            # run it (SDL2 window if installed)
build/ad-harness --module paperboy --play     # interactive demo mode
build/ad-harness --module paperboy --set scroll_speed=fast --set day_length=10  # config
build/ad-harness --module paperboy --headless --frames 300   # CI smoke test
```

Without SDL2 installed the harness uses the headless backend (no window) — used
by CI to verify every module renders without crashing. Install SDL2
(`libsdl2-dev` / `brew install sdl2`) to get a real window.

### Add a new module
1. `cp -r modules/_template modules/<your-id>`
2. Edit `ModuleInfo` + `module.json` (id/name/category) and your `init/tick/draw`.
3. Register it in `modules/modules.cpp` and add its source to `CMakeLists.txt`.

All artwork and code are original recreations — no Berkeley Systems assets are
used (see PLAN.md §7).
