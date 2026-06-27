// Gravity — recreation.
//
// Balls bounce around the screen under gravity, losing a little energy on each
// floor/wall hit and getting re-kicked when they settle, so the motion never
// dies. Each ball leaves a short fading trail. Demonstrates simple physics
// integration with restitution. Original art — no Berkeley Systems assets.
#include "gravity.h"

#include <cmath>
#include <vector>

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
    c.clear(Color{12, 14, 20});
    for (const auto& b : balls_) {
      // Trail opposite the velocity.
      for (int k = 1; k <= 4; ++k) {
        int tx = static_cast<int>(b.x - b.vx * 0.01 * k);
        int ty = static_cast<int>(b.y - b.vy * 0.01 * k);
        int r = b.radius - k * 2;
        if (r <= 0) continue;
        Color t{static_cast<uint8_t>(b.color.r / (k + 1)),
                static_cast<uint8_t>(b.color.g / (k + 1)),
                static_cast<uint8_t>(b.color.b / (k + 1))};
        c.fill_rect(tx - r, ty - r, r * 2, r * 2, t);
      }
      draw_ball(c, b);
    }
  }

 private:
  // A filled circle approximated by horizontal spans (uses only fill_rect).
  void draw_ball(Canvas& c, const Ball& b) {
    int cx = static_cast<int>(b.x), cy = static_cast<int>(b.y), r = b.radius;
    for (int dy = -r; dy <= r; ++dy) {
      int half = static_cast<int>(std::sqrt(static_cast<double>(r * r - dy * dy)));
      c.fill_rect(cx - half, cy + dy, half * 2, 1, b.color);
    }
  }

  int w_ = 0, h_ = 0;
  std::vector<Ball> balls_;
};

}  // namespace

std::unique_ptr<Module> make_gravity() { return std::make_unique<Gravity>(); }

}  // namespace ad
