// Hard Rain — recreation.
//
// Diagonal rain streaks fall against a dark sky, splashing into a puddle line at
// the bottom; every so often lightning flashes and briefly lights the whole
// scene. Demonstrates layered particles + a global flash effect. Original art —
// no Berkeley Systems assets.
#include "hard_rain.h"

#include <algorithm>
#include <cmath>
#include <vector>

namespace ad {
namespace {

struct Drop {
  double x, y, speed, len;
};

struct Splash {
  double x, y, life;
};

class HardRain : public Module {
 public:
  ModuleInfo info() const override {
    return {"hard-rain", "Hard Rain", "1.0.0", Category::Ambient};
  }

  std::vector<SettingDesc> settings_schema() const override {
    return {
        {"intensity", "Rain intensity", SettingType::Enum, "normal", 0, 0,
         {"drizzle", "normal", "downpour"}},
        {"lightning", "Lightning", SettingType::Bool, "true", 0, 0, {}},
    };
  }

  void init(Context& ctx) override {
    w_ = ctx.screen_w;
    h_ = ctx.screen_h;
    ground_ = static_cast<int>(h_ * 0.92);
    std::string in = ctx.settings.get("intensity", "normal");
    int n = in == "drizzle" ? 120 : in == "downpour" ? 600 : 300;
    lightning_ = ctx.settings.get_bool("lightning", true);
    drops_.clear();
    for (int i = 0; i < n; ++i) drops_.push_back(spawn(ctx));
    splashes_.clear();
    flash_ = 0;
    flash_timer_ = 3.0;
  }

  void tick(Context& ctx, double dt) override {
    for (auto& d : drops_) {
      d.x += d.speed * 0.25 * dt;  // slight diagonal
      d.y += d.speed * dt;
      if (d.y > ground_) {
        if (splashes_.size() < 200) splashes_.push_back({d.x, (double)ground_, 0.3});
        d = spawn(ctx);
        d.y = -ctx.rng.next_double() * 40;
      }
    }
    for (auto& s : splashes_) s.life -= dt;
    splashes_.erase(
        std::remove_if(splashes_.begin(), splashes_.end(),
                       [](const Splash& s) { return s.life <= 0; }),
        splashes_.end());

    if (flash_ > 0) flash_ -= dt * 2.5;
    if (lightning_) {
      flash_timer_ -= dt;
      if (flash_timer_ <= 0) {
        flash_ = 1.0;
        flash_timer_ = 4.0 + ctx.rng.next_double() * 8.0;
      }
    }
  }

  void draw(Canvas& c) override {
    int base = 18 + static_cast<int>((flash_ > 0 ? flash_ : 0) * 120);
    uint8_t b = static_cast<uint8_t>(base > 255 ? 255 : base);
    c.clear(Color{b, b, static_cast<uint8_t>(std::min(255, b + 12))});

    Color rain{150, 170, 210};
    for (const auto& d : drops_) {
      int x = static_cast<int>(d.x), y = static_cast<int>(d.y);
      int dx = static_cast<int>(d.len * 0.25);
      int steps = static_cast<int>(d.len);
      for (int k = 0; k < steps; k += 2) {
        int px = x + dx * k / steps;
        c.fill_rect(px, y + k, 1, 2, rain);
      }
    }
    // Splash ticks.
    for (const auto& s : splashes_) {
      int sx = static_cast<int>(s.x);
      c.fill_rect(sx - 3, ground_, 2, 2, rain);
      c.fill_rect(sx + 2, ground_, 2, 2, rain);
    }
    // Wet ground.
    c.fill_rect(0, ground_, w_, h_ - ground_, Color{30, 38, 52});
  }

 private:
  Drop spawn(Context& ctx) {
    Drop d;
    d.x = ctx.rng.next_double() * w_;
    d.y = ctx.rng.next_double() * ground_;
    d.speed = 600 + ctx.rng.next_double() * 500;
    d.len = 10 + ctx.rng.next_double() * 14;
    return d;
  }

  int w_ = 0, h_ = 0, ground_ = 0;
  bool lightning_ = true;
  double flash_ = 0, flash_timer_ = 0;
  std::vector<Drop> drops_;
  std::vector<Splash> splashes_;
};

}  // namespace

std::unique_ptr<Module> make_hard_rain() {
  return std::make_unique<HardRain>();
}

}  // namespace ad
