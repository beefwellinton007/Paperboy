// Bad Dog! — recreation.
//
// A dog romps around the screen, changing direction at the edges and leaving a
// trail of fading paw prints; now and then it stops to bark (a little "!"). A
// light-hearted critter module. Original art — no Berkeley Systems assets.
#include "bad_dog.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include "afterdark/text.h"

namespace ad {
namespace {

struct Paw {
  int x, y;
  double life;
};

class BadDog : public Module {
 public:
  ModuleInfo info() const override {
    return {"bad-dog", "Bad Dog!", "1.0.0", Category::Ambient};
  }

  void init(Context& ctx) override {
    w_ = ctx.screen_w;
    h_ = ctx.screen_h;
    x_ = w_ * 0.5;
    y_ = h_ * 0.5;
    double a = ctx.rng.next_double() * 6.2831853;
    vx_ = std::cos(a) * 180;
    vy_ = std::sin(a) * 180;
    paws_.clear();
    bark_timer_ = 3.0;
    barking_ = 0;
    paw_timer_ = 0;
  }

  void tick(Context& ctx, double dt) override {
    time_ += dt;
    if (barking_ > 0) {
      barking_ -= dt;
    } else {
      x_ += vx_ * dt;
      y_ += vy_ * dt;
      if (x_ < 20 || x_ > w_ - 20) { vx_ = -vx_; nudge(ctx); }
      if (y_ < 20 || y_ > h_ - 30) { vy_ = -vy_; nudge(ctx); }
      x_ = std::clamp(x_, 20.0, (double)w_ - 20);
      y_ = std::clamp(y_, 20.0, (double)h_ - 30);

      paw_timer_ -= dt;
      if (paw_timer_ <= 0) {
        if (paws_.size() < 120)
          paws_.push_back({(int)x_, (int)y_ + 14, 2.5});
        paw_timer_ = 0.12;
      }
    }

    bark_timer_ -= dt;
    if (bark_timer_ <= 0) {
      barking_ = 0.8;
      bark_timer_ = 4.0 + ctx.rng.next_double() * 6.0;
    }

    for (auto& p : paws_) p.life -= dt;
    paws_.erase(std::remove_if(paws_.begin(), paws_.end(),
                               [](const Paw& p) { return p.life <= 0; }),
                paws_.end());
  }

  void draw(Canvas& c) override {
    c.clear(Color{40, 50, 35});  // grassy
    // Fading paw prints.
    for (const auto& p : paws_) {
      uint8_t a = static_cast<uint8_t>(std::min(1.0, p.life / 2.5) * 120);
      c.fill_rect(p.x - 2, p.y, 4, 3, Color{a, a, a});
    }
    draw_dog(c);
  }

 private:
  void nudge(Context& ctx) {
    vx_ += (ctx.rng.next_double() - 0.5) * 60;
    vy_ += (ctx.rng.next_double() - 0.5) * 60;
  }

  void draw_dog(Canvas& c) {
    int x = (int)x_, y = (int)y_;
    int f = vx_ >= 0 ? 1 : -1;
    Color body{150, 110, 70}, dark{110, 80, 50};
    // Body, head, ears, legs, tail.
    c.fill_rect(x - 16, y - 6, 30, 14, body);          // body
    int hx = x + f * 16;
    c.fill_rect(hx - 6, y - 12, 14, 14, body);          // head
    c.fill_rect(hx - 8, y - 12, 5, 8, dark);            // ear
    c.fill_rect(hx + f * 6, y - 6, 3, 3, Color{20, 20, 20});  // nose
    int gait = static_cast<int>(std::sin(time_ * 14) * 3);
    c.fill_rect(x - 12, y + 8, 4, 8 + gait, body);
    c.fill_rect(x + 8, y + 8, 4, 8 - gait, body);
    int tail = static_cast<int>(std::sin(time_ * 12) * 6);
    c.fill_rect(x - 20, y - 8 + tail, 6, 4, dark);      // wagging tail
    if (barking_ > 0)
      draw_text(c, hx + f * 8, y - 28, "!", 4, Color{255, 240, 120});
  }

  int w_ = 0, h_ = 0;
  double x_ = 0, y_ = 0, vx_ = 0, vy_ = 0, time_ = 0;
  double bark_timer_ = 0, barking_ = 0, paw_timer_ = 0;
  std::vector<Paw> paws_;
};

}  // namespace

std::unique_ptr<Module> make_bad_dog() { return std::make_unique<BadDog>(); }

}  // namespace ad
