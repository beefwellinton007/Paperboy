// Warp / Hyperspace — flagship recreation.
//
// A 3D starfield flying toward the viewer. Unlike the plain template starfield,
// stars stretch into motion-blur streaks that lengthen with speed, and the warp
// speed pulses — easing up, then surging into a hyperspace burst. Streaks are
// drawn as a short run of rects (no line primitive needed). Original art.
#include "warp.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include "afterdark/draw.h"

namespace ad {
namespace {

struct Star3 {
  double x, y, z;  // z in (0,1]; smaller = closer
};

class Warp : public Module {
 public:
  ModuleInfo info() const override {
    return {"warp", "Warp (Hyperspace)", "1.0.0", Category::Ambient};
  }

  void init(Context& ctx) override {
    w_ = ctx.screen_w;
    h_ = ctx.screen_h;
    stars_.assign(std::max(150, w_ * h_ / 3500), Star3{});
    for (auto& s : stars_) s = spawn(ctx);
  }

  void tick(Context& ctx, double dt) override {
    time_ += dt;
    // Speed eases in a slow sine, then occasional hyperspace surges.
    double base = 0.35 + 0.25 * (0.5 + 0.5 * std::sin(time_ * 0.5));
    surge_ = std::max(0.0, surge_ - dt);
    if (surge_ <= 0 && ctx.rng.next_double() < dt * 0.15) surge_ = 1.2;
    speed_ = base + (surge_ > 0 ? 1.6 : 0.0);

    for (auto& s : stars_) {
      s.z -= speed_ * dt;
      if (s.z <= 0.02) s = spawn(ctx);
    }
  }

  void draw(Canvas& c) override {
    c.clear(Color{0, 0, 8});
    int cx = w_ / 2, cy = h_ / 2;
    for (const auto& s : stars_) {
      double px = cx + s.x / s.z;
      double py = cy + s.y / s.z;
      // Previous (deeper) position gives the streak direction/length.
      double pz = std::min(1.0, s.z + speed_ * 0.03);
      double qx = cx + s.x / pz;
      double qy = cy + s.y / pz;
      draw_streak(c, qx, qy, px, py, s.z);
    }
  }

 private:
  Star3 spawn(Context& ctx) {
    return {(ctx.rng.next_double() - 0.5) * w_ * 1.4,
            (ctx.rng.next_double() - 0.5) * h_ * 1.4,
            ctx.rng.next_double() * 0.98 + 0.02};
  }

  void draw_streak(Canvas& c, double x0, double y0, double x1, double y1,
                   double z) {
    int closeness = static_cast<int>((1.0 - z) * 3) + 1;  // 1..4 px thick/bright
    uint8_t b = static_cast<uint8_t>(120 + (1.0 - z) * 135);
    Color col{b, b, static_cast<uint8_t>(std::min(255, b + 20))};
    int steps = std::max(1, static_cast<int>(std::hypot(x1 - x0, y1 - y0) / 2));
    steps = std::min(steps, 40);
    for (int i = 0; i <= steps; ++i) {
      double t = static_cast<double>(i) / steps;
      int px = static_cast<int>(x0 + (x1 - x0) * t);
      int py = static_cast<int>(y0 + (y1 - y0) * t);
      if (px < 0 || px >= w_ || py < 0 || py >= h_) continue;
      // Fade the streak from tail (faint) to head (bright).
      Color seg = col;
      seg.a = static_cast<uint8_t>(60 + t * 195);
      c.fill_rect(px, py, closeness, closeness, seg);
    }
    // Bright glowing head for near stars.
    if (z < 0.5 && x1 >= 0 && x1 < w_ && y1 >= 0 && y1 < h_) {
      Color h = col;
      h.a = 120;
      glow(c, static_cast<int>(x1), static_cast<int>(y1), closeness + 2, h, 3);
    }
  }

  int w_ = 0, h_ = 0;
  double time_ = 0, speed_ = 0.4, surge_ = 0;
  std::vector<Star3> stars_;
};

}  // namespace

std::unique_ptr<Module> make_warp() { return std::make_unique<Warp>(); }

}  // namespace ad
