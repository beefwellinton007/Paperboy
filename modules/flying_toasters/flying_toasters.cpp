// Flying Toasters — the first flagship recreation, and the reference for the
// sprite-animation pattern the rest of the catalog reuses.
//
// Winged toasters (and the occasional slice of toast) drift diagonally down and
// to the left across a dark background, exactly like the icon of the original
// suite. Each flyer carries its own animation phase so wings flap out of sync.
//
// Art is procedural (drawn from fill_rects) rather than a sprite sheet — that
// keeps the module asset-free for now and demonstrates per-entity animation
// timing. A real sprite-atlas path can replace draw_flyer() later without
// touching the simulation. (Original-art recreation — no Berkeley assets.)
#include "flying_toasters.h"

#include <algorithm>
#include <cmath>
#include <unordered_map>
#include <vector>

#include "afterdark/draw.h"
#include "afterdark/sprite.h"

namespace ad {
namespace {

struct Flyer {
  double x, y;     // top-left in screen space
  double speed;    // px/s along the diagonal
  double phase;    // wing-flap phase offset
  int scale;       // pixel size multiplier
  bool toast;      // true = slice of toast, false = toaster
};

class FlyingToasters : public Module {
 public:
  ModuleInfo info() const override {
    return {"flying-toasters", "Flying Toasters", "1.0.0", Category::Ambient};
  }

  void init(Context& ctx) override {
    w_ = ctx.screen_w;
    h_ = ctx.screen_h;
    flyers_.clear();
    // Stagger initial flyers diagonally across and above the screen.
    for (int i = 0; i < 16; ++i) {
      Flyer f = spawn(ctx);
      f.x = ctx.rng.next_double() * (w_ + h_) - h_;
      f.y = ctx.rng.next_double() * h_;
      flyers_.push_back(f);
    }
  }

  void tick(Context& ctx, double dt) override {
    time_ += dt;
    for (auto& f : flyers_) {
      // Classic diagonal: down and to the left.
      f.x -= f.speed * dt;
      f.y += f.speed * 0.5 * dt;
      int sz = 24 * f.scale;
      if (f.x < -sz || f.y > h_ + sz) f = respawn_top_right(ctx, f);
    }
  }

  void draw(Canvas& c) override {
    // Modern touch: a soft vertical night-sky gradient instead of flat fill.
    v_gradient(c, 0, 0, w_, h_, Color{18, 18, 42}, Color{6, 6, 16});
    // Back-to-front so nearer (bigger) flyers overlap farther ones.
    std::sort(flyers_.begin(), flyers_.end(),
              [](const Flyer& a, const Flyer& b) { return a.scale < b.scale; });
    for (const auto& f : flyers_) draw_flyer(c, f);
  }

 private:
  Flyer spawn(Context& ctx) {
    Flyer f;
    // Scale toasters to the screen so they aren't tiny on big/Retina displays.
    int unit = std::max(1, h_ / 300);
    f.scale = ctx.rng.range(1, 3) * unit;
    f.speed = (60 + ctx.rng.range(1, 3) * 25 + ctx.rng.next_double() * 30) *
              std::max(1.0, h_ / 600.0);
    f.phase = ctx.rng.next_double() * 6.28;
    f.toast = ctx.rng.range(0, 4) == 0;  // ~1 in 5 is toast
    f.x = w_;
    f.y = 0;
    return f;
  }

  // Recycle off the top-right edge so the stream never empties.
  Flyer respawn_top_right(Context& ctx, const Flyer& old) {
    Flyer f = spawn(ctx);
    int sz = 24 * f.scale;
    f.x = w_ + ctx.rng.next_double() * w_ * 0.5;
    f.y = -sz - ctx.rng.next_double() * h_ * 0.5;
    (void)old;
    return f;
  }

  // Hand-drawn pixel-art sprites, built once.
  static const Sprite& toaster_sprite() {
    static const Sprite s(
        {
            ".....CCCCCCCC.....",
            "...CCDDDDDDDDCC...",
            "..CDSSSSSSSSSSDC..",
            "..CDSSSSSSSSSSDCL.",
            ".CDDDDDDDDDDDDDDL.",
            ".CDhhhhhhhhhhhhDL.",
            ".CDhCCCCCCCCCChDC.",
            ".CDhCmmmmmmmmChDC.",
            ".CDhCmmmmmmmmChDC.",
            ".CDhhhhhhhhhhhhDC.",
            ".CDDDDDDDDDDDDDDC.",
            "..CC..ffff..CC....",
        },
        {{'C', {225, 230, 240}},  // chrome highlight
         {'D', {150, 160, 180}},  // chrome mid
         {'h', {110, 120, 145}},  // chrome shadow
         {'m', {90, 100, 125}},   // inset face
         {'S', {35, 38, 50}},     // toast slot
         {'L', {215, 75, 60}},    // lever knob
         {'f', {70, 78, 100}}});  // feet
    return s;
  }
  static const Sprite& toast_sprite() {
    static const Sprite s(
        {
            "..bbbbbb..",
            ".bBBBBBBb.",
            "bBBBBWBBBb",
            "bBBBBBBBBb",
            "bBWBBBBBBb",
            "bBBBBBBWBb",
            "bBBBBBBBBb",
            ".bBBBBBBb.",
            "..bbbbbb..",
        },
        {{'b', {120, 70, 38}}, {'B', {228, 192, 120}}, {'W', {245, 220, 160}}});
    return s;
  }
  static const Sprite& wing_sprite() {
    static const Sprite s(
        {
            ".....ww",
            "...wwWW",
            ".wwWWWW",
            "wWWWWWg",
            ".wwWWg.",
            "...wg..",
        },
        {{'W', {248, 248, 252}}, {'w', {205, 210, 225}}, {'g', {150, 158, 178}}});
    return s;
  }

  void draw_flyer(Canvas& c, const Flyer& f) const {
    int x = static_cast<int>(f.x), y = static_cast<int>(f.y), s = f.scale;
    if (f.toast) {
      toast_sprite().blit(c, x, y, s);
      return;
    }
    const Sprite& body = toaster_sprite();
    const Sprite& wing = wing_sprite();
    // Wings flap together; a sine raises/lowers them around the body.
    double flap = std::sin(time_ * 9.0 + f.phase);
    int wy = y + static_cast<int>((2 - flap * 3) * s);
    int ww = wing.width() * s;
    wing.blit(c, x - ww + 2 * s, wy, s, /*flip_x=*/true);            // back wing
    body.blit(c, x, y, s);
    wing.blit(c, x + body.width() * s - 2 * s, wy, s, /*flip_x=*/false);  // front
  }

  int w_ = 0, h_ = 0;
  double time_ = 0;
  std::vector<Flyer> flyers_;
};

}  // namespace

std::unique_ptr<Module> make_flying_toasters() {
  return std::make_unique<FlyingToasters>();
}

}  // namespace ad
