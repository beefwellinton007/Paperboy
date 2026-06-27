// Spotlight — flagship recreation.
//
// A roving spotlight wanders a dark screen on a Lissajous path, revealing a
// hidden scene only where the light falls. The scene underneath is a tiled
// grid plus a big "AFTER DARK" sign (drawn with the built-in font) — invisible
// until the beam sweeps over it.
//
// The reveal is done with a shadow mask: the full scene is drawn, then black is
// painted everywhere except a circular hole around the beam (computed per
// scanline). Original art — no Berkeley Systems assets.
#include "spotlight.h"

#include <algorithm>
#include <cmath>
#include <string>

#include "afterdark/text.h"

namespace ad {
namespace {

class Spotlight : public Module {
 public:
  ModuleInfo info() const override {
    return {"spotlight", "Spotlight", "1.0.0", Category::Ambient};
  }

  void init(Context& ctx) override {
    w_ = ctx.screen_w;
    h_ = ctx.screen_h;
    radius_ = std::max(70, std::min(w_, h_) / 5);
    (void)ctx;
  }

  void tick(Context&, double dt) override { time_ += dt; }

  void draw(Canvas& c) override {
    draw_scene(c);  // the hidden content, fully drawn

    // Beam center on a Lissajous path that stays on-screen.
    double bx = w_ * (0.5 + 0.42 * std::sin(time_ * 0.43));
    double by = h_ * (0.5 + 0.40 * std::sin(time_ * 0.67 + 1.0));
    apply_shadow_mask(c, bx, by);
  }

 private:
  void draw_scene(Canvas& c) {
    c.clear(Color{30, 30, 38});
    // Checkerboard floor.
    const int t = 48;
    for (int y = 0; y < h_; y += t)
      for (int x = 0; x < w_; x += t)
        if (((x / t) + (y / t)) & 1)
          c.fill_rect(x, y, t, t, Color{52, 52, 66});
    // The hidden sign.
    const std::string msg = "AFTER DARK";
    int scale = std::max(4, w_ / 120);
    int tw = text_width(msg, scale);
    draw_text(c, (w_ - tw) / 2, h_ / 2 - 5 * scale / 2, msg, scale,
              Color{255, 220, 130});
  }

  void apply_shadow_mask(Canvas& c, double bx, double by) {
    const Color dark{0, 0, 0};
    const int r = radius_;
    int cx = static_cast<int>(bx), cy = static_cast<int>(by);
    for (int y = 0; y < h_; ++y) {
      int dy = y - cy;
      if (std::abs(dy) > r) {
        c.fill_rect(0, y, w_, 1, dark);  // whole row outside the beam
        continue;
      }
      int half = static_cast<int>(std::sqrt(static_cast<double>(r * r - dy * dy)));
      int left = cx - half, right = cx + half;
      if (left > 0) c.fill_rect(0, y, left, 1, dark);
      if (right < w_) c.fill_rect(right, y, w_ - right, 1, dark);
      // Feathered penumbra: graduated translucent dark bands just inside the
      // edge so the beam fades in rather than cutting off hard.
      const int FE = 16;
      for (int k = 0; k < FE; ++k) {
        unsigned char a = static_cast<unsigned char>(200 - k * 200 / FE);
        Color band{0, 0, 0, a};
        c.fill_rect(left + k, y, 1, 1, band);
        c.fill_rect(right - 1 - k, y, 1, 1, band);
      }
    }
  }

  int w_ = 0, h_ = 0, radius_ = 80;
  double time_ = 0;
};

}  // namespace

std::unique_ptr<Module> make_spotlight() {
  return std::make_unique<Spotlight>();
}

}  // namespace ad
