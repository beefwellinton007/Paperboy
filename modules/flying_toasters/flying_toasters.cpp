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
#include <vector>

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
    c.clear(Color{12, 12, 28});  // deep night-blue, like the original
    // Back-to-front so nearer (bigger) flyers overlap farther ones.
    std::sort(flyers_.begin(), flyers_.end(),
              [](const Flyer& a, const Flyer& b) { return a.scale < b.scale; });
    for (const auto& f : flyers_) draw_flyer(c, f);
  }

 private:
  Flyer spawn(Context& ctx) {
    Flyer f;
    f.scale = ctx.rng.range(1, 3);
    f.speed = 60 + f.scale * 25 + ctx.rng.next_double() * 30;
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

  void draw_flyer(Canvas& c, const Flyer& f) const {
    int x = static_cast<int>(f.x), y = static_cast<int>(f.y);
    int s = f.scale;
    if (f.toast) {
      // A slice of toast: bread body + a darker crust border.
      c.fill_rect(x, y, 18 * s, 16 * s, Color{120, 70, 40});
      c.fill_rect(x + 2 * s, y + 2 * s, 14 * s, 12 * s, Color{225, 190, 120});
      return;
    }
    // Toaster body, slot, and the delivery lever.
    c.fill_rect(x, y, 22 * s, 16 * s, Color{200, 205, 215});      // chrome body
    c.fill_rect(x + 2 * s, y + 2 * s, 18 * s, 3 * s, Color{40, 40, 50});  // slot
    c.fill_rect(x + 20 * s, y + 6 * s, 2 * s, 5 * s, Color{90, 90, 100});  // lever
    c.fill_rect(x, y + 14 * s, 22 * s, 2 * s, Color{150, 155, 165});       // base trim

    // Wings flap with a per-flyer phase: a sine drives the vertical span.
    double flap = std::sin(time_ * 9.0 + f.phase);
    int wing_h = static_cast<int>((4 + flap * 3) * s);
    int wy = y + 4 * s - wing_h;
    c.fill_rect(x - 10 * s, wy, 10 * s, wing_h, Color{245, 245, 245});       // back wing
    c.fill_rect(x + 22 * s, wy, 10 * s, wing_h, Color{220, 220, 230});       // front wing
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
