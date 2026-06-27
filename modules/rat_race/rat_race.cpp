// Rat Race — recreation.
//
// Mice scurry laps around an oval track, jostling for position with little
// bursts of speed. Each racer runs at its own pace along the loop; the leader
// is highlighted. A light-hearted nod to the original's frantic critters.
// Original art — no Berkeley Systems assets.
#include "rat_race.h"

#include <algorithm>
#include <cmath>
#include <vector>

namespace ad {
namespace {

struct Racer {
  double t;       // position along the loop [0,1)
  double speed;   // laps per second
  double lane;    // radial offset
  Color color;
  double burst;   // remaining speed-burst time
};

class RatRace : public Module {
 public:
  ModuleInfo info() const override {
    return {"rat-race", "Rat Race", "1.0.0", Category::Ambient};
  }

  void init(Context& ctx) override {
    w_ = ctx.screen_w;
    h_ = ctx.screen_h;
    rx_ = w_ * 0.36;
    ry_ = h_ * 0.30;
    static const Color cols[] = {{230, 120, 120}, {120, 200, 130},
                                 {120, 160, 230}, {230, 210, 110},
                                 {210, 130, 220}, {230, 230, 230}};
    racers_.clear();
    for (int i = 0; i < 6; ++i) {
      Racer r;
      r.t = ctx.rng.next_double();
      r.speed = 0.10 + ctx.rng.next_double() * 0.06;
      r.lane = (i - 2.5) * 14;
      r.color = cols[i];
      r.burst = 0;
      racers_.push_back(r);
    }
  }

  void tick(Context& ctx, double dt) override {
    time_ += dt;
    for (auto& r : racers_) {
      if (r.burst > 0) r.burst -= dt;
      else if (ctx.rng.next_double() < dt * 0.5) r.burst = 0.8;
      double sp = r.speed * (r.burst > 0 ? 2.2 : 1.0);
      r.t += sp * dt;
      if (r.t >= 1.0) r.t -= 1.0;
    }
  }

  void draw(Canvas& c) override {
    c.clear(Color{30, 90, 40});  // grass infield
    int cx = w_ / 2, cy = h_ / 2;

    // Track: a thick oval drawn as a ring of segments.
    for (int i = 0; i < 120; ++i) {
      double a = i / 120.0 * 6.2831853;
      int x = static_cast<int>(cx + std::cos(a) * rx_);
      int y = static_cast<int>(cy + std::sin(a) * ry_);
      c.fill_rect(x - 6, y - 6, 12, 12, Color{120, 95, 70});
    }

    // Find the leader (furthest along this lap) to highlight.
    std::size_t leader = 0;
    for (std::size_t i = 1; i < racers_.size(); ++i)
      if (racers_[i].t > racers_[leader].t) leader = i;

    for (std::size_t i = 0; i < racers_.size(); ++i) {
      const auto& r = racers_[i];
      double a = r.t * 6.2831853;
      double rr = 1.0 + r.lane / rx_;
      int x = static_cast<int>(cx + std::cos(a) * rx_ * rr);
      int y = static_cast<int>(cy + std::sin(a) * ry_ * rr);
      // Body + scurrying legs + tail.
      int legs = static_cast<int>(std::sin(time_ * 20 + i) * 2);
      c.fill_rect(x - 5, y - 3 + legs, 10, 6, r.color);
      c.fill_rect(x + 5, y - 1, 5, 2, Color{210, 180, 180});  // tail
      if (i == leader)
        c.fill_rect(x - 6, y - 8, 12, 3, Color{255, 215, 0});  // leader crown
    }
  }

 private:
  int w_ = 0, h_ = 0;
  double rx_ = 100, ry_ = 80, time_ = 0;
  std::vector<Racer> racers_;
};

}  // namespace

std::unique_ptr<Module> make_rat_race() {
  return std::make_unique<RatRace>();
}

}  // namespace ad
