// Confetti Factory — recreation.
//
// Bursts of colorful confetti are flung from the bottom of the screen, arc up
// under gravity, flutter (each piece sways and "tumbles", changing width), and
// fall away. New bursts fire on a steady cadence. Original art — no Berkeley
// Systems assets.
#include "confetti.h"

#include <cmath>
#include <vector>

#include "afterdark/draw.h"

namespace ad {
namespace {

struct Piece {
  double x, y, vx, vy, spin, phase;
  Color color;
  bool alive = false;
};

class Confetti : public Module {
 public:
  ModuleInfo info() const override {
    return {"confetti", "Confetti Factory", "1.0.0", Category::Ambient};
  }

  std::vector<SettingDesc> settings_schema() const override {
    return {
        {"density", "Confetti density", SettingType::Enum, "normal", 0, 0,
         {"light", "normal", "heavy"}},
    };
  }

  void init(Context& ctx) override {
    w_ = ctx.screen_w;
    h_ = ctx.screen_h;
    std::string d = ctx.settings.get("density", "normal");
    cap_ = d == "light" ? 150 : d == "heavy" ? 600 : 320;
    pieces_.assign(cap_, Piece{});
    burst_timer_ = 0;
  }

  void tick(Context& ctx, double dt) override {
    const double g = 520.0;
    burst_timer_ -= dt;
    if (burst_timer_ <= 0) {
      launch_burst(ctx);
      burst_timer_ = 0.5 + ctx.rng.next_double() * 0.6;
    }
    for (auto& p : pieces_) {
      if (!p.alive) continue;
      p.vy += g * dt;
      p.x += p.vx * dt + std::sin(p.phase + p.y * 0.02) * 14 * dt;  // flutter
      p.y += p.vy * dt;
      p.phase += p.spin * dt;
      if (p.y > h_ + 12) p.alive = false;
    }
  }

  void draw(Canvas& c) override {
    v_gradient(c, 0, 0, w_, h_, Color{20, 18, 30}, Color{8, 8, 14});
    for (const auto& p : pieces_) {
      if (!p.alive) continue;
      // "Tumble": width oscillates so pieces look like spinning flakes.
      int wpx = 2 + static_cast<int>(std::abs(std::cos(p.phase)) * 6);
      int px = static_cast<int>(p.x), py = static_cast<int>(p.y);
      Color halo = p.color;
      halo.a = 60;
      c.fill_rect(px - 1, py - 1, wpx + 2, 7, halo);  // soft glow
      c.fill_rect(px, py, wpx, 5, p.color);
    }
  }

 private:
  void launch_burst(Context& ctx) {
    static const Color pal[] = {{240, 80, 80},   {250, 200, 70}, {90, 210, 120},
                                {90, 160, 240},  {220, 110, 230}, {250, 250, 250}};
    double ox = ctx.rng.next_double() * w_;
    int count = 30 + ctx.rng.range(0, 30);
    for (auto& p : pieces_) {
      if (count <= 0) break;
      if (p.alive) continue;
      double ang = -1.5707963 + (ctx.rng.next_double() - 0.5) * 1.4;  // upward fan
      double speed = 260 + ctx.rng.next_double() * 260;
      p.x = ox + (ctx.rng.next_double() - 0.5) * 40;
      p.y = h_ + 6;
      p.vx = std::cos(ang) * speed;
      p.vy = std::sin(ang) * speed;
      p.spin = 4 + ctx.rng.next_double() * 10;
      p.phase = ctx.rng.next_double() * 6.28;
      p.color = pal[ctx.rng.range(0, 5)];
      p.alive = true;
      --count;
    }
  }

  int w_ = 0, h_ = 0, cap_ = 320;
  double burst_timer_ = 0;
  std::vector<Piece> pieces_;
};

}  // namespace

std::unique_ptr<Module> make_confetti() { return std::make_unique<Confetti>(); }

}  // namespace ad
