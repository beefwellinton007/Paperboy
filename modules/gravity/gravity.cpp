// Gravity — recreation.
//
// Balls bounce around the screen under gravity, losing a little energy on each
// floor/wall hit and getting re-kicked when they settle, so the motion never
// dies. Each ball leaves a short fading trail. Demonstrates simple physics
// integration with restitution. Original art — no Berkeley Systems assets.
#include "gravity.h"

#include <cmath>
#include <vector>

#include "afterdark/draw.h"

namespace ad {
namespace {

struct Ball {
  double x, y, vx, vy;
  int radius;
  Color color;
};

class Gravity : public Module {
 public:
  ModuleInfo info() const override {
    return {"gravity", "Gravity", "1.0.0", Category::Ambient};
  }

  std::vector<SettingDesc> settings_schema() const override {
    return {
        {"ball_count", "Number of balls", SettingType::Int, "14", 1, 60, {}},
    };
  }

  void init(Context& ctx) override {
    w_ = ctx.screen_w;
    h_ = ctx.screen_h;
    int n = ctx.settings.get_int("ball_count", 14);
    static const Color pal[] = {{240, 90, 90},  {250, 200, 80}, {100, 210, 130},
                                {100, 170, 240}, {220, 120, 230}};
    balls_.clear();
    for (int i = 0; i < n; ++i) {
      Ball b;
      b.radius = ctx.rng.range(8, 22);
      b.x = ctx.rng.range(b.radius, w_ - b.radius);
      b.y = ctx.rng.range(b.radius, h_ / 2);
      b.vx = (ctx.rng.next_double() - 0.5) * 300;
      b.vy = ctx.rng.next_double() * 100;
      b.color = pal[ctx.rng.range(0, 4)];
      balls_.push_back(b);
    }
  }

  void tick(Context& ctx, double dt) override {
    const double g = 900.0, restitution = 0.82;
    for (auto& b : balls_) {
      b.vy += g * dt;
      b.x += b.vx * dt;
      b.y += b.vy * dt;
      if (b.x < b.radius) { b.x = b.radius; b.vx = -b.vx * restitution; }
      if (b.x > w_ - b.radius) { b.x = w_ - b.radius; b.vx = -b.vx * restitution; }
      if (b.y > h_ - b.radius) {
        b.y = h_ - b.radius;
        b.vy = -b.vy * restitution;
        // Re-kick balls that have nearly settled so motion never dies.
        if (std::abs(b.vy) < 120) {
          b.vy = -(300 + ctx.rng.next_double() * 300);
          b.vx += (ctx.rng.next_double() - 0.5) * 200;
        }
      }
      if (b.y < b.radius) { b.y = b.radius; b.vy = -b.vy * restitution; }
    }
  }

  void draw(Canvas& c) override {
    v_gradient(c, 0, 0, w_, h_, Color{16, 18, 28}, Color{6, 7, 12});
    for (const auto& b : balls_) {
      int cx = static_cast<int>(b.x), cy = static_cast<int>(b.y);
      // Soft trail of fading circles opposite the velocity.
      for (int k = 4; k >= 1; --k) {
        int tx = static_cast<int>(b.x - b.vx * 0.012 * k);
        int ty = static_cast<int>(b.y - b.vy * 0.012 * k);
        Color t = b.color;
        t.a = static_cast<uint8_t>(40 / k);
        fill_circle(c, tx, ty, b.radius - k, t);
      }
      // Glow halo + crisp ball with a highlight.
      Color halo = b.color;
      halo.a = 70;
      glow(c, cx, cy, b.radius * 2, halo, 4);
      fill_circle(c, cx, cy, b.radius, b.color);
      Color hi{255, 255, 255, 150};
      fill_circle(c, cx - b.radius / 3, cy - b.radius / 3, b.radius / 3, hi);
    }
  }

 private:

  int w_ = 0, h_ = 0;
  std::vector<Ball> balls_;
};

}  // namespace

std::unique_ptr<Module> make_gravity() { return std::make_unique<Gravity>(); }

}  // namespace ad
