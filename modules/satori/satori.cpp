// Satori — recreation.
//
// Evolving abstract art: a symmetric rose/spirograph curve is traced point by
// point with a slowly cycling color, mirrored across several axes for a
// kaleidoscopic look. Every so often the curve parameters re-seed, so the
// pattern keeps reinventing itself. Original art — no Berkeley Systems assets.
#include "satori.h"

#include <cmath>

namespace ad {
namespace {

class Satori : public Module {
 public:
  ModuleInfo info() const override {
    return {"satori", "Satori", "1.0.0", Category::Ambient};
  }

  std::vector<SettingDesc> settings_schema() const override {
    return {
        {"symmetry", "Symmetry", SettingType::Int, "6", 2, 12, {}},
        {"fade", "Trails fade", SettingType::Bool, "true", 0, 0, {}},
    };
  }

  void init(Context& ctx) override {
    w_ = ctx.screen_w;
    h_ = ctx.screen_h;
    sym_ = ctx.settings.get_int("symmetry", 6);
    fade_ = ctx.settings.get_bool("fade", true);
    reseed(ctx);
    first_ = true;
  }

  void tick(Context& ctx, double dt) override {
    t_ += dt;
    hue_ += dt * 40;
    reseed_timer_ -= dt;
    if (reseed_timer_ <= 0) reseed(ctx);
  }

  void draw(Canvas& c) override {
    if (first_) { c.clear(Color{0, 0, 0}); first_ = false; }
    if (fade_) {
      // Soft fade: overlay sparse dark specks so old trails dim over time.
      // (No alpha in the canvas, so we darken by overdrawing thin bands.)
      for (int y = 0; y < h_; y += 3) c.fill_rect(0, y, w_, 1, Color{0, 0, 0});
    } else {
      c.clear(Color{0, 0, 0});
    }

    int cx = w_ / 2, cy = h_ / 2;
    double scale = std::min(w_, h_) * 0.42;
    Color col = hsv(hue_, 0.7, 1.0);

    // Trace a short arc of the rose curve this frame; mirror across `sym_` axes.
    for (int i = 0; i < 140; ++i) {
      double a = t_ * 0.6 + i * 0.045;
      double r = std::cos(k_ * a) * scale;
      double bx = std::cos(a) * r, by = std::sin(a) * r;
      for (int s = 0; s < sym_; ++s) {
        double ang = s * 6.2831853 / sym_;
        double ca = std::cos(ang), sa = std::sin(ang);
        int px = cx + static_cast<int>(bx * ca - by * sa);
        int py = cy + static_cast<int>(bx * sa + by * ca);
        c.fill_rect(px, py, 2, 2, col);
      }
    }
  }

 private:
  void reseed(Context& ctx) {
    k_ = ctx.rng.range(2, 9);
    reseed_timer_ = 8.0 + ctx.rng.next_double() * 8.0;
  }

  // Minimal HSV->RGB (h in degrees, s/v in 0..1).
  Color hsv(double h, double s, double v) {
    h = std::fmod(h, 360.0);
    double cc = v * s, x = cc * (1 - std::fabs(std::fmod(h / 60.0, 2) - 1));
    double m = v - cc;
    double r = 0, g = 0, b = 0;
    if (h < 60) { r = cc; g = x; }
    else if (h < 120) { r = x; g = cc; }
    else if (h < 180) { g = cc; b = x; }
    else if (h < 240) { g = x; b = cc; }
    else if (h < 300) { r = x; b = cc; }
    else { r = cc; b = x; }
    return {static_cast<uint8_t>((r + m) * 255),
            static_cast<uint8_t>((g + m) * 255),
            static_cast<uint8_t>((b + m) * 255)};
  }

  int w_ = 0, h_ = 0, sym_ = 6, k_ = 5;
  bool fade_ = true, first_ = true;
  double t_ = 0, hue_ = 0, reseed_timer_ = 0;
};

}  // namespace

std::unique_ptr<Module> make_satori() { return std::make_unique<Satori>(); }

}  // namespace ad
