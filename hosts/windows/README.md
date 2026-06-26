# Windows host (`.scr`)

A Windows screensaver is just an `.exe` renamed to `.scr` that honors three
command-line modes:

| Flag | Meaning |
|------|---------|
| `/s` | Run full-screen (the actual screensaver) |
| `/p <HWND>` | Render into the little preview pane in the Screen Saver dialog |
| `/c` | Show the configuration dialog |

## Plan

`scr_main.cpp` (stub: [scr_main.cpp](./scr_main.cpp)) will:

1. Parse the `/s` `/p` `/c` flags.
2. Create the render target (the full-screen window, or the supplied preview
   `HWND`) and hand it to **AfterDark Core** via the SDL2/native backend.
3. Pick the module(s) from saved config (registry/INI), then run the same
   `init/tick/draw` loop the harness uses.
4. On `/c`, render the manifest-driven settings UI (see `sdk/schema/module.schema.json`).
5. Dismiss on real input/movement in `/s` mode (OS requirement).

Packaging: build the exe, rename to `AfterDark.scr`, wrap in a signed installer.

> Not built by the default CMake target — requires the Windows SDK. Build on a
> Windows runner in CI.
