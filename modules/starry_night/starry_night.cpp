// Starry Night — flagship recreation.
//
// A night-sky gradient over a city skyline silhouette, with twinkling stars and
// the occasional shooting star streaking across. Demonstrates layered drawing
// (sky gradient -> stars -> shooting stars -> skyline) and time-based twinkle,
// the parallax/layering pattern other ambient modules reuse.
// Original art — no Berkeley Systems assets.
#include "starry_night.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include "afterdark/draw.h"

namespace ad {
namespace {

struct Star {
  int x, y;
  double phase;  // twinkle phase
  double rate;   // twinkle speed
  int size;
};

struct Shooter {
  double x, y, vx, vy, life;  // life counts down; <=0 = inactive
};

struct Building {
  int x, w, h;
  bool lit_windows;
};

class StarryNight : public Module {
 public:
  ModuleInfo info() const override {
    return {"starry-night", "Starry Night", "1.0.0", Category::Ambient};
  }

  void init(Context& ctx) override {
    w_ = ctx.screen_w;
    h_ = ctx.screen_h;
    horizon_ = static_cast<int>(h_ * 0.72);

    stars_.clear();
    int count = std::max(80, w_ * h_ / 6000);
    for (int i = 0; i < count; ++i) {
      Star s;
      s.x = ctx.rng.range(0, w_ - 1);
      s.y = ctx.rng.range(0, horizon_ - 1);
      s.phase = ctx.rng.next_double() * 6.28;
      s.rate = 1.5 + ctx.rng.next_double() * 3.0;
      s.size = ctx.rng.range(0, 9) == 0 ? 2 : 1;  // a few brighter stars
      stars_.push_back(s);
    }

    buildings_.clear();
    int x = 0;
    while (x < w_) {
      Building b;
      b.w = ctx.rng.range(40, 90);
      b.h = ctx.rng.range(h_ / 10, h_ / 4);
      b.x = x;
      b.lit_windows = ctx.rng.range(0, 2) != 0;
      buildings_.push_back(b);
      x += b.w + ctx.rng.range(-6, 10);  // slight overlap/variation
    }

    shooters_.assign(3, Shooter{0, 0, 0, 0, 0});
    shooter_timer_ = 1.5;
  }

  void tick(Context& ctx, double dt) override {
    time_ += dt;

    // Spawn shooting stars on a randomized cadence.
    shooter_timer_ -= dt;
    if (shooter_timer_ <= 0) {
      for (auto& s : shooters_) {
        if (s.life <= 0) {
          double speed = 600 + ctx.rng.next_double() * 400;
          double ang = 0.25 + ctx.rng.next_double() * 0.35;  // shallow descent
          s.x = ctx.rng.next_double() * w_ * 0.7;
          s.y = ctx.rng.next_double() * horizon_ * 0.5;
          s.vx = std::cos(ang) * speed;
          s.vy = std::sin(ang) * speed;
          s.life = 0.9;
          break;
        }
      }
      shooter_timer_ = 1.5 + ctx.rng.next_double() * 3.5;
    }
    for (auto& s : shooters_) {
      if (s.life <= 0) continue;
      s.x += s.vx * dt;
      s.y += s.vy * dt;
      s.life -= dt;
    }
  }

  void draw(Canvas& c) override {
    draw_sky(c);
    draw_stars(c);
    draw_shooters(c);
    draw_skyline(c);
  }

 private:
  void draw_sky(Canvas& c) {
    // Vertical gradient: near-black up top to deep indigo at the horizon.
    const Color top{4, 4, 20}, bottom{30, 22, 60};
    const int bands = 24;
    for (int i = 0; i < bands; ++i) {
      double t = static_cast<double>(i) / (bands - 1);
      int y0 = horizon_ * i / bands;
      int y1 = horizon_ * (i + 1) / bands;
      c.fill_rect(0, y0, w_, y1 - y0, lerp(top, bottom, t));
    }
    c.fill_rect(0, horizon_, w_, h_ - horizon_, Color{8, 8, 16});  // ground
  }

  void draw_stars(Canvas& c) {
    for (const auto& s : stars_) {
      double tw = 0.5 + 0.5 * std::sin(time_ * s.rate + s.phase);
      uint8_t b = static_cast<uint8_t>(110 + tw * 145);
      Color col{b, b, static_cast<uint8_t>(std::min(255, b + 10))};
      // Brighter stars get a soft twinkling glow halo.
      if (s.size > 1) {
        Color halo = col;
        halo.a = static_cast<uint8_t>(70 + tw * 90);
        glow(c, s.x, s.y, 3 + static_cast<int>(tw * 3), halo, 3);
      } else {
        c.fill_rect(s.x, s.y, s.size, s.size, col);
      }
    }
  }

  void draw_shooters(Canvas& c) {
    for (const auto& s : shooters_) {
      if (s.life <= 0) continue;
      // Short trailing streak behind the head.
      for (int k = 0; k < 8; ++k) {
        int px = static_cast<int>(s.x - s.vx * 0.006 * k);
        int py = static_cast<int>(s.y - s.vy * 0.006 * k);
        uint8_t b = static_cast<uint8_t>(255 - k * 26);
        c.fill_rect(px, py, 2, 2, Color{b, b, b});
      }
    }
  }

  void draw_skyline(Canvas& c) {
    for (const auto& b : buildings_) {
      int top = horizon_ - b.h;
      c.fill_rect(b.x, top, b.w, b.h, Color{6, 6, 14});  // silhouette
      if (!b.lit_windows) continue;
      // A grid of warm lit windows, some randomly dark.
      for (int wy = top + 8; wy < horizon_ - 6; wy += 12) {
        for (int wx = b.x + 6; wx < b.x + b.w - 6; wx += 12) {
          // Deterministic on/off + slow flicker from position + time.
          int seed = (wx * 73856093) ^ (wy * 19349663);
          bool on = (seed & 7) > 2;
          if (!on) continue;
          bool flick = std::sin(time_ * 0.7 + seed) > -0.9;
          c.fill_rect(wx, wy, 5, 6,
                      flick ? Color{255, 210, 120} : Color{60, 45, 20});
        }
      }
    }
  }

  int w_ = 0, h_ = 0, horizon_ = 0;
  double time_ = 0;
  double shooter_timer_ = 0;
  std::vector<Star> stars_;
  std::vector<Shooter> shooters_;
  std::vector<Building> buildings_;
};

}  // namespace

std::unique_ptr<Module> make_starry_night() {
  return std::make_unique<StarryNight>();
}

}  // namespace ad
