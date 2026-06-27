# After Dark — Pro Art Brief (every image, every frame)

This is the production brief. Generate exactly these images, at these sizes, with
these filenames. Follow the **Universal Rules** for every single image — that's
what makes the set look like one cohesive, pro product instead of 40 random
pictures.

---

## 0. How to use this
- Generate each listed image as its own PNG (one clean subject per image).
- For animations, generate each numbered frame **with identical framing, scale,
  camera, and lighting** — only the moving part changes. (Jitter between frames =
  the #1 thing that looks amateur.)
- Drop finished PNGs in `assets/raw/<module>/` with the exact filenames, or send
  them to me. I pack + wire them in and send back previews.

## 1. Universal rules (apply to EVERY image)
- **Format:** PNG, **true transparent background (alpha)**. No white box, no
  checkerboard, **no drop shadow on the ground**, no border, no text/watermark.
- **One subject, centered**, filling ~80% of the canvas with even padding.
- **View:** as specified per asset (3/4 for appliances, side-on for characters &
  props). Characters/props **face RIGHT** (engine mirrors for left).
- **Lighting:** single soft light from **top-left**; subtle ambient occlusion
  bottom-right. Consistent across the whole set.
- **Outline:** bold, clean, near-black (#1A1A1A) outline around the silhouette,
  slightly thinner interior lines. Strong readable silhouette.
- **Style:** glossy cartoon / vector-illustration with smooth cel shading (think
  the classic After Dark toaster + modern polish). Pick this and never deviate.
- **Palette:** keep saturation/contrast consistent set-wide. Reuse the same
  metals, skin tone, greens, etc. across modules.
- **Resolution:** author at the sizes below (sized for a 1440p screen). Square-ish
  canvas is fine; the subject just needs the listed long-edge size.
- **Consistency trick:** for multi-frame sets, reuse the same seed / "same
  subject, only X changes" / a reference image, so the body stays identical.

**Style suffix — paste at the end of every prompt:**
> "glossy cartoon vector illustration, smooth cel shading, bold clean near-black
> outline, soft top-left lighting, centered, fully transparent background, no
> ground shadow, no text, high detail, crisp edges"

## 2. Naming & delivery
- Single image: `name.png`
- Animation frames: `name_1.png`, `name_2.png`, … (1-based, **identical canvas**)
- Folders: `assets/raw/flying_toasters/`, `assets/raw/paperboy/`,
  `assets/raw/boris/`, `assets/raw/fish/`

---

## 3. BATCH 1 — Flying Toasters  (6 images — start here)
The marquee. Chrome 2-slot pop-up toaster, **3/4 front-left view**, glowing red
heating coils visible in the slots, olive/khaki lower body, chrome top, small
lever knob on the right. Wings are a **separate** asset so they flap (I mirror
one wing for the far side and animate both).

| # | filename | what it is | view | size (long edge) |
|---|----------|-----------|------|------|
| 1 | `toaster_body.png` | the toaster **with NO wings** | 3/4 front-left | 360 px |
| 2 | `wing_1.png` | one right wing — **fully raised/up** | side, feathers fanned up | 200 px |
| 3 | `wing_2.png` | same wing — **mid up-stroke** | between up and level | 200 px |
| 4 | `wing_3.png` | same wing — **level / spread out** | horizontal | 200 px |
| 5 | `wing_4.png` | same wing — **down-swept** | angled down | 200 px |
| 6 | `toast.png` | a single slice of golden toast | slight 3/4 | 150 px |

Animation: I play wing 1→2→3→4→3→2 looping (~10 fps) for a smooth flap. The 4
wing frames **must be the same wing at the same position**, only the feather
angle changing.

**Prompts:**
- toaster_body: "A chrome pop-up 2-slot toaster, 3/4 front-left view, two slots
  with glowing red-orange heating coils inside, polished silver chrome top,
  olive-green lower body, small red lever knob on the right side, NO wings,
  [style suffix]"
- wing_1..4: "A single large white feathered angel wing, right side, [POSITION:
  raised straight up / mid up-stroke / spread level / swept downward], soft
  cream-white feathers with light gray shadow between feathers, [style suffix]"
  — generate all four with the **same wing**, only the position word changing.
- toast: "A single slice of toasted golden-brown bread, slight 3/4 angle, soft
  rounded corners, [style suffix]"

---

## 4. BATCH 2 — Paperboy  (~14 images)
Side-scroller, everything **side-on, facing right**.

**Rider (kid on a BMX bike):**
| # | filename | what | size |
|---|----------|------|------|
| 1 | `rider_1.png` | kid pedaling — **pedals vertical** (one leg up, one down) | 190 px |
| 2 | `rider_2.png` | kid pedaling — **pedals horizontal** (legs swapped) | 190 px |
| 3 | `rider_throw.png` | kid **throwing a paper** — arm extended forward | 190 px |

Same kid, same bike, same scale across all three — only legs/arm change. Outfit:
red cap, blue shirt, satchel of papers. I play rider_1↔rider_2 while riding (~8
fps) and swap to rider_throw briefly on a toss.

**Papers & props (1 frame each):**
| filename | what | size |
|----------|------|------|
| `paper.png` | a rolled/folded newspaper | 48 px |
| `house_a.png` | suburban house, warm tan walls, red roof | 280 px |
| `house_b.png` | suburban house, blue walls, gray roof | 280 px |
| `house_c.png` | suburban house, green walls, brown roof | 280 px |
| `tree.png` | leafy round tree | 200 px |
| `hedge.png` | low rectangular hedge/bush | 140 px |
| `mailbox.png` | curbside mailbox on a post | 120 px |
| `trashcan.png` | metal trash can with lid (obstacle) | 120 px |
| `hydrant.png` | red fire hydrant (obstacle) | 100 px |

**Parallax background layers (must tile horizontally — left & right edges match):**
| filename | what | size |
|----------|------|------|
| `bg_far.png` | distant tree line / soft hills, seamless tile | 1024×300 |
| `bg_mid.png` | picket fence + bushes strip, seamless tile | 1024×200 |

(The sky is a procedural gradient; you don't need to draw it.)

**Prompts (examples):**
- rider_1/2: "A cartoon kid on a yellow BMX bike, side view facing right, red
  baseball cap, blue t-shirt, canvas satchel full of newspapers, [pedals vertical
  / pedals horizontal, legs mid-stride], [style suffix]"
- rider_throw: "...same kid on the BMX, side view facing right, right arm
  extended forward having just thrown a rolled newspaper, [style suffix]"
- house_a: "A cute suburban single-story house, side/slightly-3/4 view, tan
  stucco walls, red tile roof, door and two windows, small lawn strip at the
  base, [style suffix]"
- bg_far: "A seamless horizontally-tileable strip of distant green hills and a
  soft tree line under open sky, side-scroller background, muted depth, left and
  right edges match perfectly for tiling, [style suffix]"

---

## 5. BATCH 3 — Boris the Cat & Fish

**Boris (side view, facing right, ~150 px):**
| filename | what |
|----------|------|
| `cat_walk_1.png` | walk cycle — contact (front leg forward) |
| `cat_walk_2.png` | walk cycle — passing (legs under body) |
| `cat_walk_3.png` | walk cycle — contact (other leg forward) |
| `cat_walk_4.png` | walk cycle — passing (legs under body, other) |
| `cat_sit.png` | sitting upright, tail curled |
| `cat_groom.png` | sitting, licking a raised front paw |
| `cat_sleep.png` | curled up asleep, eyes closed |

Same cat (color/markings) in every frame. Walk plays 1→2→3→4 (~8 fps).

**Fish (side view, facing right, ~120 px) — 3 species, 2 tail frames each:**
| filename | what |
|----------|------|
| `fish_a_1.png` / `fish_a_2.png` | orange clownfish-style, tail neutral / flicked |
| `fish_b_1.png` / `fish_b_2.png` | blue tang-style, tail neutral / flicked |
| `fish_c_1.png` / `fish_c_2.png` | yellow tropical, tail neutral / flicked |

Each pair = same fish, only the tail/fins move. Plays 1↔2 (~4 fps).

**Prompts (examples):**
- cat_walk_x: "A cartoon house cat, side view facing right, gray tabby with white
  paws, [walk pose: front leg reaching forward / legs gathered under body],
  [style suffix]" — keep the same cat across all 7.
- fish_a: "A small cartoon clownfish, side view facing right, orange with white
  bands, [tail straight / tail flicked left], [style suffix]"

---

## 6. Animation timing (for reference — I set these in code)
| sprite | frames | playback |
|--------|--------|----------|
| toaster wings | 4 (ping-pong 1-2-3-4-3-2) | ~10 fps |
| paperboy pedaling | 2 | ~8 fps |
| cat walk | 4 | ~8 fps |
| fish tail | 2 | ~4 fps |

## 7. QA checklist before you send them
- [ ] Background fully transparent (open in Preview — you see through it).
- [ ] No baked-in ground shadow, border, or text.
- [ ] Animation frames share identical canvas size + subject position/scale.
- [ ] Subject faces right; consistent top-left lighting + outline across the set.
- [ ] Filenames match exactly (lowercase, underscores, the right folder).

> Start with **Batch 1 (the 6 toaster images)**. Send them over and I'll have
> Flying Toasters at reference quality the same session.
