# macOS validation (10 minutes)

Goal: confirm the suite **runs on real hardware** — CI compiles it but never
actually runs the `.saver` as a screensaver. Two levels; do Level 1 first.

Prereqs:
```bash
brew install cmake sdl2          # sdl2 only needed for the windowed harness
git clone <repo> && cd Paperboy
git checkout claude/after-dark-screensaver-i42rfm
```

## Level 1 — windowed harness (fastest, see it live)
Builds the engine + a desktop window you can watch immediately.
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/ad-harness --module flying-toasters     # a window opens
./build/ad-harness --list                       # all 20 modules
./build/ad-harness --module paperboy --play     # interactive demo
```
✅ Pass = a window opens and the module animates smoothly.

## Level 2 — the actual screensaver bundle
The macOS build also produces `AfterDark.saver`.
```bash
# after the build above:
find build -name 'AfterDark.saver'              # locate the bundle
cp -R "$(find build -name 'AfterDark.saver' | head -1)" ~/Library/Screen\ Savers/
```
Then: System Settings → Screen Saver → pick **After Dark** → it should preview
and run (it defaults to the Paperboy module for now).

✅ Pass = it appears in the list, previews, and runs full-screen; moving the
mouse / pressing a key exits.

## What to report back
- Which level you reached, and any build errors (copy the first error).
- Does it animate smoothly? Any flicker, wrong colors, or crashes?
- Retina/multi-monitor behavior if you can check.

> Notes: code signing/notarization isn't set up yet, so Gatekeeper may warn on
> the `.saver` — right-click → Open, or allow it in System Settings → Privacy &
> Security. Module selection + a config UI are still in progress; the bundle runs
> Paperboy by default.
