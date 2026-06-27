# Art spec — generating sprites for the suite

You generate PNGs (AI tool of choice); I wire them in. This sheet says exactly
what to make so they drop straight into the engine.

## Hard format rules (please follow these)
- **PNG, transparent background** (real alpha — not white/checkerboard).
- **8-bit** RGB/RGBA (the default for most exports).
- **One subject, centered**, small even padding, facing **right** (I mirror for
  left).
- **Max dimension ≈ 512 px.** Author at the size it should appear on a large
  screen; bigger just wastes space. Square-ish canvas is fine.
- Consistent lighting: **light from top-left**, soft shadow bottom-right.
- Keep a **bold, clean silhouette** (the references read well because of strong
  outlines + clear shapes). Match the vibe of the refs you sent.

## Workflow
1. Drop PNGs in `assets/raw/<module>/` using the exact filenames below.
2. Run: `python3 tools/pack_assets.py assets/raw assets` (converts every PNG to
   `.adspr`). Or just hand me the PNGs and I'll pack + integrate.
3. I blit them in the module and refresh the preview montage so you can see them.

## Priority 1 — Flying Toasters (the marquee)
Match your reference: chrome toaster, **two slots with glowing red coils**, olive/
khaki lower body, big **white feathered wings**, dark outline, 3/4 view.
- `assets/raw/flying_toasters/toaster.png` — the winged toaster body (wings can
  be part of it, OR omit wings and supply them separately below for flapping).
- `assets/raw/flying_toasters/wing_up.png` and `wing_down.png` — a single white
  feathered wing in two positions (for the flap animation). Right-facing; I
  mirror for the far wing. *(Optional but makes the flap look great.)*
- `assets/raw/flying_toasters/toast.png` — a slice of toast, 3/4 view.

## Priority 2 — Paperboy (side-scroller)
- `assets/raw/paperboy/rider_1.png`, `rider_2.png` — kid on a BMX, side view,
  two pedaling frames (legs different). ~180 px tall.
- `assets/raw/paperboy/paper.png` — a rolled/folded newspaper.
- `assets/raw/paperboy/house_a.png`, `house_b.png` — suburban houses, side/front,
  ~256 px. A couple of variants for variety.
- `assets/raw/paperboy/trashcan.png`, `hydrant.png` — sidewalk obstacles.

## Priority 3 — Boris the Cat & Fish
- `assets/raw/boris/cat_walk_1.png`, `cat_walk_2.png`, `cat_sit.png`,
  `cat_sleep.png` — a cat, side view, ~120 px.
- `assets/raw/fish/fish_a.png`, `fish_b.png`, `fish_c.png` — tropical fish,
  side view, right-facing, ~96 px.

> Ambient modules (Starry Night, Warp, Satori, Gravity, Confetti, Spotlight,
> Down the Drain, Hard Rain, Clocks) stay procedural with the smooth/glow pass —
> they don't need sprite art.

## Example prompt (toaster), to adapt
> "A chrome pop-up toaster with two slots showing glowing red heating coils,
> olive-green lower body, large white feathered angel wings spread, bold dark
> outline, clean cartoon/vector style, 3/4 view, soft top-left lighting,
> centered on a fully transparent background, ~512px."

Hand me the PNGs (or a link) and I'll integrate the first batch immediately.
