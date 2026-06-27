// Boris the Cat — recreation.
//
// A cat strolls across the screen, occasionally stopping to sit and wash, then
// curls up for a nap before wandering off again at a new height. A small state
// machine (Walk -> Sit -> Sleep -> Walk) drives behavior; legs and tail animate
// with simple sine wobbles. Original art — no Berkeley Systems assets.
#include "boris.h"

#include <algorithm>
#include <cmath>

#include "afterdark/draw.h"

namespace ad {
namespace {

enum class State { Walk, Sit, Sleep };

class Boris : public Module {
 public:
  ModuleInfo info() const override {
    return {"boris", "Boris the Cat", "1.0.0", Category::Ambient};
  }

  void init(Context& ctx) override {
    w_ = ctx.screen_w;
    h_ = ctx.screen_h;
    new_walk(ctx, true);
  }

  void tick(Context& ctx, double dt) override {
    time_ += dt;
    state_timer_ -= dt;
    if (state_ == State::Walk) {
      x_ += dir_ * speed_ * dt;
      if (x_ < -60 || x_ > w_ + 60) {  // walked off — reappear elsewhere
        new_walk(ctx, false);
      } else if (state_timer_ <= 0) {
        state_ = State::Sit;
        state_timer_ = 3.0 + ctx.rng.next_double() * 3.0;
      }
    } else if (state_ == State::Sit) {
      if (state_timer_ <= 0) {
        state_ = State::Sleep;
        state_timer_ = 4.0 + ctx.rng.next_double() * 5.0;
      }
    } else {  // Sleep
      if (state_timer_ <= 0) new_walk(ctx, false);
    }
  }

  void draw(Canvas& c) override {
    // Cozy room: wall gradient over a floor band, with a soft shadow under Boris.
    int floor = y_ + 18;
    v_gradient(c, 0, 0, w_, h_, Color{40, 34, 50}, Color{26, 22, 34});
    c.fill_rect(0, floor, w_, h_ - floor, Color{46, 38, 44});
    c.fill_rect(0, floor, w_, 2, Color{70, 60, 72});
    int sx = static_cast<int>(x_);
    c.fill_rect(sx - 20, floor - 2, 40, 4, Color{0, 0, 0, 80});  // contact shadow
    if (state_ == State::Sleep) draw_sleeping(c);
    else draw_upright(c);
  }

 private:
  void new_walk(Context& ctx, bool first) {
    state_ = State::Walk;
    y_ = ctx.rng.range(h_ / 3, h_ - 60);
    dir_ = ctx.rng.range(0, 1) ? 1 : -1;
    speed_ = 60 + ctx.rng.next_double() * 60;
    x_ = first ? ctx.rng.range(0, w_) : (dir_ > 0 ? -50 : w_ + 50);
    state_timer_ = 3.0 + ctx.rng.next_double() * 5.0;
  }

  Color fur() const { return Color{120, 110, 130}; }
  Color fur_dark() const { return Color{90, 82, 100}; }

  void draw_upright(Canvas& c) {
    int x = static_cast<int>(x_), y = y_;
    int f = dir_;  // facing
    // Body + head.
    c.fill_rect(x - 16, y - 4, 32, 16, fur());            // body
    int hx = x + f * 16;
    c.fill_rect(hx - 6, y - 14, 14, 14, fur());           // head
    // Ears.
    c.fill_rect(hx - 6, y - 20, 4, 6, fur_dark());
    c.fill_rect(hx + 4, y - 20, 4, 6, fur_dark());
    // Eyes.
    c.fill_rect(hx + f * 2 - 2, y - 9, 2, 2, Color{30, 230, 120});
    c.fill_rect(hx + f * 2 + 2, y - 9, 2, 2, Color{30, 230, 120});
    // Tail, curling and swishing.
    double sw = std::sin(time_ * 4) * 6;
    int tx = x - f * 16;
    c.fill_rect(tx - f * 8, y - 10 + static_cast<int>(sw), 8, 4, fur_dark());
    // Legs: walk animation when moving, planted when sitting.
    if (state_ == State::Walk) {
      int swing = static_cast<int>(std::sin(time_ * 10) * 4);
      c.fill_rect(x - 12, y + 12, 4, 8 + swing, fur());
      c.fill_rect(x + 8, y + 12, 4, 8 - swing, fur());
    } else {  // Sit
      c.fill_rect(x - 12, y + 12, 24, 6, fur());
    }
  }

  void draw_sleeping(Canvas& c) {
    int x = static_cast<int>(x_), y = y_;
    // Curled-up oval-ish blob.
    c.fill_rect(x - 20, y + 2, 40, 14, fur());
    c.fill_rect(x - 16, y - 2, 32, 8, fur());
    // Tail wrapped around the front.
    c.fill_rect(x - 20, y + 8, 14, 5, fur_dark());
    // "Z Z Z" rising sleep marks (drawn as little blocks).
    int zb = (static_cast<int>(time_) % 3) + 1;
    for (int i = 0; i < zb; ++i)
      c.fill_rect(x + 18 + i * 8, y - 8 - i * 8, 5, 5, Color{200, 200, 220});
  }

  int w_ = 0, h_ = 0, y_ = 0, dir_ = 1;
  double x_ = 0, speed_ = 80, time_ = 0, state_timer_ = 0;
  State state_ = State::Walk;
};

}  // namespace

std::unique_ptr<Module> make_boris() { return std::make_unique<Boris>(); }

}  // namespace ad
