# Art-driven plan — road to reference-quality 2.0

The engine is ready (sprites, GPU texture blit, glow/gradient toolkit, asset
pipeline, animation). From here, **art is the lever.** This plan sequences the
art production so what you generate drops straight in and stays cohesive.

## 1. Module tiers
- **Hero (full sprite art):** Flying Toasters, Paperboy, Boris the Cat, Fish.
  These define the suite's quality bar.
- **Ambient (procedural + glow, mostly done):** Starry Night, Warp, Satori,
  Gravity, Confetti, Down the Drain, Hard Rain, Spotlight, Globe, Clocks,
  Messages, Rat Race, Bad Dog!, Mowing Man, Starfield. May get light textures
  later, but they don't need hero art.

## 2. Style guide (keep everything cohesive)
- **Transparent background** (real alpha), one subject centered, **facing right**
  (engine mirrors for left).
- **Lighting from top-left**, soft shadow bottom-right.
- **Bold, clean outline** + readable silhouette (the references read because of
  this). Cartoon/vector or rendered-3D look — pick one and stay consistent.
- **Perspective:** Flying Toasters = 3/4 view; Paperboy & characters = side view.
- **Resolution:** author at on-screen size for ~1440p. Heroes ≈ 256–512 px on the
  long edge; props ≈ 128–256 px. Square-ish canvas; even padding.
- **Palette:** keep a shared palette feel across modules (similar saturation,
  outline darkness) so the suite looks like one product.

## 3. Animation convention
Multi-frame sprites are just numbered files; the engine plays them:
- `name_1.png`, `name_2.png`, … (1-based). The engine loads the sequence into an
  `Animation` and cycles at a set FPS.
- Single-frame art: just `name.png` (no number).
- Typical frame counts: wings 2–4, pedaling 2, cat walk 2–4, fish idle 2.

## 4. Asset manifest (prioritized batches)
Drop PNGs in `assets/raw/<module>/`; filenames matter.

**Batch 1 — Flying Toasters** (smallest, highest impact; proves the look)
| file | what | size | frames |
|------|------|------|--------|
| `toaster.png` | chrome toaster, red coils, olive base, 3/4 | ~360px | 1 (body) |
| `wing_1..3.png` | one white feathered wing, flap frames | ~160px | 2–3 |
| `toast.png` | toast slice, 3/4 | ~140px | 1 |

**Batch 2 — Paperboy** (side-scroller, with parallax layers)
| file | what | size | frames |
|------|------|------|--------|
| `rider_1..2.png` | kid on BMX, pedaling | ~180px | 2 |
| `paper.png` | rolled newspaper | ~48px | 1 |
| `house_a/b/c.png` | suburban houses | ~256px | 1 each |
| `tree.png`,`hedge.png`,`mailbox.png`,`trashcan.png`,`hydrant.png` | props | ~96–160px | 1 |
| `bg_sky.png` (optional) | far parallax band | wide | 1 |

**Batch 3 — Boris & Fish**
| file | what | size | frames |
|------|------|------|--------|
| `boris/cat_walk_1..4.png` | cat walk cycle, side | ~140px | 2–4 |
| `boris/cat_sit.png`,`cat_sleep.png` | poses | ~140px | 1 |
| `fish/fish_a/b/c.png` | tropical fish, side, right | ~110px | 1–2 |

## 5. Production loop (per batch)
1. You generate PNGs (AI tool) to spec → drop in `assets/raw/<module>/`.
2. `python3 tools/pack_assets.py assets/raw assets` (or hand me the PNGs).
3. I wire them into the module (`load_adspr` / `Animation`, `blit_scaled`),
   keeping a procedural fallback if an asset is missing.
4. I refresh `docs/preview/montage.png` + per-module previews and send them.
5. We iterate on anything that looks off (size, framing, palette).

Turnaround per batch is short once art exists — Batch 1 is ~3 sprites.

## 6. Engine to-dos that support the art (I own these)
- [x] Sprite + GPU texture blit + smooth scaling
- [x] `.adspr` asset pipeline (PNG→pack→load)
- [x] **Animation** (numbered-frame sequences) — *added with this plan*
- [ ] Per-module asset loading with graceful procedural fallback (as each hero
      module is wired)
- [ ] **Asset bundling** into the `.scr` / `.saver` so the installed app finds
      its art (Resources dir for macOS; embed/next-to-exe for Windows)
- [ ] Config UI for picking modules/settings (separate track)

## 7. Roadmap to 2.0
1. **Art Batch 1 (Toasters)** → integrate → confirm the quality bar on your Mac.
2. **Art Batch 2 (Paperboy)** + parallax background layers.
3. **Art Batch 3 (Boris, Fish).**
4. **Asset bundling** + module-select config UI.
5. **Signing/notarization + installers** (Apple Developer acct + Windows cert)
   and on-device QA → tag **2.0**, publish the changelog.

## 8. What I need from you to start Batch 1
1. Pick the AI image tool you'll use.
2. Generate the **toaster** first (prompt in `docs/ART_SPEC.md`) — transparent PNG.
3. Drop it in `assets/raw/flying_toasters/` or paste it here.

I integrate the moment it arrives, and you'll see the jump on Flying Toasters.
