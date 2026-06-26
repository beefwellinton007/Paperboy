# macOS host (`.saver`)

A macOS screensaver is a bundle (`AfterDark.saver`) containing a principal
class that subclasses `ScreenSaverView`. macOS instantiates it for full-screen,
for the System Settings preview, and for the config sheet.

## Plan

`ADSaverView` (sketch: [ADSaverView.mm](./ADSaverView.mm)) will:

1. Subclass `ScreenSaverView`; set `animationTimeInterval` (~1/60).
2. In `initWithFrame:isPreview:`, embed **AfterDark Core**, create the
   Metal/SDL2 backend bound to the view's layer, load module(s) from
   `ScreenSaverDefaults`.
3. `animateOneFrame` → Core `tick` + `draw`.
4. `hasConfigureSheet` / `configureSheet` → manifest-driven settings UI
   (see `sdk/schema/module.schema.json`).
5. Handle the `isPreview` flag (smaller, lower-cost rendering).

Packaging: build the `.saver` bundle, codesign + notarize, ship in a `.dmg`.

> Not built by the default CMake target — requires Xcode/macOS SDK. Build on a
> macOS runner in CI.
