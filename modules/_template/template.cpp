// Module template — copy this folder to add a new screensaver to the catalog.
//
// It's also a real, working ambient module (a tiny Warp-style starfield) so the
// registry always has a second entry and CI has something minimal to smoke.
// Steps to add your own module:
//   1. cp -r modules/_template modules/<your-id>
//   2. rename the factory + edit ModuleInfo + module.json (id/name/category)
//   3. register it in modules/modules.cpp
#include <vector>

#include "afterdark/afterdark.h"
#include "template.h"

namespace ad {
namespace {

class Starfield : public Module {
 public:
  ModuleInfo info() const override {
    return {"starfield", "Starfield (template)", "1.0.0", Category::Ambient};
  }

  void init(Context& ctx) override {
    w_ = ctx.screen_w;
    h_ = ctx.screen_h;
    stars_.clear();
    for (int i = 0; i < 200; ++i) stars_.push_back(spawn(ctx));
  }

  void tick(Context& ctx, double dt) override {
    for (auto& s : stars_) {
      s.z -= dt * 0.6;
      if (s.z <= 0.02) s = spawn(ctx);
    }
  }

  void draw(Canvas& c) override {
    c.clear(Color{0, 0, 10});
    int cx = w_ / 2, cy = h_ / 2;
    for (const auto& s : stars_) {
      int px = static_cast<int>(cx + s.x / s.z);
      int py = static_cast<int>(cy + s.y / s.z);
      if (px < 0 || px >= w_ || py < 0 || py >= h_) continue;
      int size = static_cast<int>(1 + (1.0 - s.z) * 3);
      uint8_t bright = static_cast<uint8_t>(120 + (1.0 - s.z) * 135);
      c.fill_rect(px, py, size, size, Color{bright, bright, bright});
    }
  }

 private:
  struct Star { double x, y, z; };
  Star spawn(Context& ctx) {
    return {(ctx.rng.next_double() - 0.5) * w_,
            (ctx.rng.next_double() - 0.5) * h_,
            ctx.rng.next_double() * 0.98 + 0.02};
  }
  int w_ = 0, h_ = 0;
  std::vector<Star> stars_;
};

}  // namespace

std::unique_ptr<Module> make_template_starfield() {
  return std::make_unique<Starfield>();
}

}  // namespace ad
