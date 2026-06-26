// Fish! / Aquatic Realm — flagship recreation.
//
// A virtual aquarium: a school of fish swims with boid-style flocking (via the
// shared steering helper), bubbles rise, seaweed sways, and a sandy bottom
// anchors the tank. First module to use ad::flock_steer — the reusable movement
// pattern for autonomous-agent modules. Original art — no Berkeley assets.
#include "fish.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include "afterdark/steering.h"

namespace ad {
namespace {

struct Bubble {
  double x, y, speed, wobble;
};

struct Weed {
  int x, base_h;
  double sway;
};

class Fish : public Module {
 public:
  ModuleInfo info() const override {
    return {"fish", "Fish! (Aquatic Realm)", "1.0.0", Category::Ambient};
  }

  void init(Context& ctx) override {
    w_ = ctx.screen_w;
    h_ = ctx.screen_h;
    sand_ = static_cast<int>(h_ * 0.9);

    int n = std::max(18, w_ / 28);
    pos_.clear();
    vel_.clear();
    color_.clear();
    size_.clear();
    for (int i = 0; i < n; ++i) {
      pos_.push_back({ctx.rng.next_double() * w_,
                      ctx.rng.next_double() * (sand_ - 20)});
      double ang = ctx.rng.next_double() * 6.28;
      vel_.push_back({std::cos(ang) * 40, std::sin(ang) * 40});
      color_.push_back(fish_color(ctx));
      size_.push_back(ctx.rng.range(6, 11));
    }

    bubbles_.clear();
    for (int i = 0; i < 40; ++i) bubbles_.push_back(spawn_bubble(ctx));

    weeds_.clear();
    for (int x = 20; x < w_; x += ctx.rng.range(40, 90)) {
      weeds_.push_back({x, ctx.rng.range(h_ / 8, h_ / 4),
                        ctx.rng.next_double() * 6.28});
    }
  }

  void tick(Context& ctx, double dt) override {
    time_ += dt;
    FlockParams fp;
    fp.max_speed = 95;
    for (std::size_t i = 0; i < pos_.size(); ++i) {
      Vec2 steer = flock_steer(i, pos_, vel_, fp);
      // Gentle wander so the school keeps moving even when cohesive.
      steer += Vec2{std::cos(time_ * 0.7 + i), std::sin(time_ * 0.5 + i * 2)} * 12;
      vel_[i] = limit(vel_[i] + steer * dt, fp.max_speed);
      pos_[i] += vel_[i] * dt;
      keep_in_tank(i);
    }

    for (auto& b : bubbles_) {
      b.y -= b.speed * dt;
      b.x += std::sin(time_ * 2 + b.wobble) * 8 * dt;
      if (b.y < 0) b = spawn_bubble(ctx);
    }
  }

  void draw(Canvas& c) override {
    draw_water(c);
    draw_weeds(c);
    for (std::size_t i = 0; i < pos_.size(); ++i) draw_fish(c, i);
    draw_bubbles(c);
    c.fill_rect(0, sand_, w_, h_ - sand_, Color{200, 180, 130});  // sand
  }

 private:
  Color fish_color(Context& ctx) {
    static const Color palette[] = {
        {240, 140, 40}, {230, 210, 70}, {90, 170, 230},
        {220, 90, 110}, {150, 220, 160}, {235, 235, 235}};
    return palette[ctx.rng.range(0, 5)];
  }

  Bubble spawn_bubble(Context& ctx) {
    return {ctx.rng.next_double() * w_, static_cast<double>(sand_),
            20 + ctx.rng.next_double() * 40, ctx.rng.next_double() * 6.28};
  }

  void keep_in_tank(std::size_t i) {
    // Steer away from the edges instead of hard-wrapping (more natural).
    const double margin = 40, push = 60;
    if (pos_[i].x < margin) vel_[i].x += push * 0.016;
    if (pos_[i].x > w_ - margin) vel_[i].x -= push * 0.016;
    if (pos_[i].y < margin) vel_[i].y += push * 0.016;
    if (pos_[i].y > sand_ - margin) vel_[i].y -= push * 0.016;
    pos_[i].x = std::clamp(pos_[i].x, 0.0, static_cast<double>(w_));
    pos_[i].y = std::clamp(pos_[i].y, 0.0, static_cast<double>(sand_));
  }

  void draw_water(Canvas& c) {
    const Color top{20, 90, 140}, bot{8, 40, 80};
    const int bands = 20;
    for (int i = 0; i < bands; ++i) {
      double t = static_cast<double>(i) / (bands - 1);
      int y0 = sand_ * i / bands, y1 = sand_ * (i + 1) / bands;
      c.fill_rect(0, y0, w_, y1 - y0, lerp(top, bot, t));
    }
  }

  void draw_weeds(Canvas& c) {
    for (const auto& wd : weeds_) {
      for (int seg = 0; seg < wd.base_h; seg += 6) {
        int sx = wd.x + static_cast<int>(std::sin(time_ * 1.5 + wd.sway +
                                                   seg * 0.15) *
                                         (seg * 0.12));
        c.fill_rect(sx, sand_ - seg - 6, 5, 6, Color{30, 120, 60});
      }
    }
  }

  void draw_fish(Canvas& c, std::size_t i) {
    int x = static_cast<int>(pos_[i].x), y = static_cast<int>(pos_[i].y);
    int s = size_[i];
    bool right = vel_[i].x >= 0;
    Color col = color_[i];
    c.fill_rect(x - s, y - s / 2, 2 * s, s, col);  // body
    // Tail on the trailing side; wags with time.
    int wag = static_cast<int>(std::sin(time_ * 8 + i) * (s / 3));
    int tx = right ? x - s - s / 2 : x + s;
    c.fill_rect(tx, y - s / 2 + wag, s / 2, s, col);
    // Eye on the leading side.
    int ex = right ? x + s - 3 : x - s + 1;
    c.fill_rect(ex, y - s / 4, 2, 2, Color{10, 10, 10});
  }

  void draw_bubbles(Canvas& c) {
    for (const auto& b : bubbles_)
      c.fill_rect(static_cast<int>(b.x), static_cast<int>(b.y), 3, 3,
                  Color{200, 230, 245});
  }

  int w_ = 0, h_ = 0, sand_ = 0;
  double time_ = 0;
  std::vector<Vec2> pos_, vel_;
  std::vector<Color> color_;
  std::vector<int> size_;
  std::vector<Bubble> bubbles_;
  std::vector<Weed> weeds_;
};

}  // namespace

std::unique_ptr<Module> make_fish() {
  return std::make_unique<Fish>();
}

}  // namespace ad
