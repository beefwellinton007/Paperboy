// Down the Drain — recreation.
//
// Colorful particles spiral inward toward a drain at the center, accelerating
// and shrinking as they approach, then vanish and respawn out at the edge — an
// endless whirlpool. Demonstrates polar-coordinate motion. Original art — no
// Berkeley Systems assets.
#include "down_the_drain.h"

#include <algorithm>
#include <cmath>
#include <vector>

namespace ad {
namespace {

struct Particle {
  double angle, radius, spin;
  Color color;
};

class DownTheDrain : public Module {
 public:
  ModuleInfo info() const override {
    return {"down-the-drain", "Down the Drain", "1.0.0", Category::Ambient};
  }

  std::vector<SettingDesc> settings_schema() const override {
    return {{"swirl", "Swirl speed", SettingType::Enum, "normal", 0, 0,
             {"slow", "normal", "fast"}}};
  }

  void init(Context& ctx) override {
    w_ = ctx.screen_w;
    h_ = ctx.screen_h;
    max_r_ = std::hypot(w_, h_) * 0.5;
    std::string s = ctx.settings.get("swirl", "normal");
    swirl_ = s == "slow" ? 0.6 : s == "fast" ? 1.8 : 1.1;
    parts_.assign(std::max(200, w_ * h_ / 2500), Particle{});
    for (auto& p : parts_) p = spawn(ctx);
  }

  void tick(Context& ctx, double dt) override {
    for (auto& p : parts_) {
      double pull = 1.0 + (max_r_ - p.radius) / max_r_ * 3.0;  // faster near center
      p.radius -= 60.0 * pull * dt;
      p.angle += p.spin * swirl_ * pull * dt;
      if (p.radius < 4) p = spawn(ctx);
    }
  }

  void draw(Canvas& c) override {
    c.clear(Color{6, 8, 14});
    int cx = w_ / 2, cy = h_ / 2;
    // The drain.
    c.fill_rect(cx - 6, cy - 6, 12, 12, Color{0, 0, 0});
    for (const auto& p : parts_) {
      int x = cx + static_cast<int>(std::cos(p.angle) * p.radius);
      int y = cy + static_cast<int>(std::sin(p.angle) * p.radius * 0.7);  // ellipse
      int sz = 1 + static_cast<int>((max_r_ - p.radius) / max_r_ * 3);
      c.fill_rect(x, y, sz, sz, p.color);
    }
  }

 private:
  Particle spawn(Context& ctx) {
    static const Color pal[] = {{90, 170, 240},  {120, 220, 200}, {200, 120, 240},
                                {240, 200, 120}, {240, 120, 140}};
    Particle p;
    p.angle = ctx.rng.next_double() * 6.2831853;
    p.radius = max_r_ * (0.6 + ctx.rng.next_double() * 0.4);
    p.spin = 0.8 + ctx.rng.next_double() * 0.8;
    p.color = pal[ctx.rng.range(0, 4)];
    return p;
  }

  int w_ = 0, h_ = 0;
  double max_r_ = 100, swirl_ = 1.1;
  std::vector<Particle> parts_;
};

}  // namespace

std::unique_ptr<Module> make_down_the_drain() {
  return std::make_unique<DownTheDrain>();
}

}  // namespace ad
