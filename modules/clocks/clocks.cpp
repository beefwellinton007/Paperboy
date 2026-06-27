// Clocks — recreation.
//
// A grid of analog clocks, each started at a different time and ticking forward
// (fast enough that the hands visibly move). Hands are drawn as runs of small
// rects along a line — a tiny reusable line-draw built on fill_rect. Original
// art — no Berkeley Systems assets.
#include "clocks.h"

#include <cmath>
#include <vector>

namespace ad {
namespace {

void draw_line(Canvas& c, int x0, int y0, int x1, int y1, int thick, Color col) {
  int dx = x1 - x0, dy = y1 - y0;
  int steps = std::max(std::abs(dx), std::abs(dy));
  if (steps == 0) steps = 1;
  for (int i = 0; i <= steps; ++i) {
    int x = x0 + dx * i / steps;
    int y = y0 + dy * i / steps;
    c.fill_rect(x - thick / 2, y - thick / 2, thick, thick, col);
  }
}

class Clocks : public Module {
 public:
  ModuleInfo info() const override {
    return {"clocks", "Clocks", "1.0.0", Category::Ambient};
  }

  void init(Context& ctx) override {
    w_ = ctx.screen_w;
    h_ = ctx.screen_h;
    cell_ = std::min(w_, h_) / 4;
    if (cell_ < 80) cell_ = 80;
    cols_ = std::max(1, w_ / cell_);
    rows_ = std::max(1, h_ / cell_);
    offsets_.clear();
    for (int i = 0; i < cols_ * rows_; ++i)
      offsets_.push_back(ctx.rng.next_double() * 43200.0);  // seconds into 12h
  }

  void tick(Context&, double dt) override {
    time_ += dt * 60.0;  // 1 real second = 1 clock minute, so hands move
  }

  void draw(Canvas& c) override {
    c.clear(Color{18, 20, 30});
    int ox = (w_ - cols_ * cell_) / 2;
    int oy = (h_ - rows_ * cell_) / 2;
    for (int r = 0; r < rows_; ++r)
      for (int col = 0; col < cols_; ++col) {
        int idx = r * cols_ + col;
        int cx = ox + col * cell_ + cell_ / 2;
        int cy = oy + r * cell_ + cell_ / 2;
        draw_clock(c, cx, cy, cell_ / 2 - 8, time_ + offsets_[idx]);
      }
  }

 private:
  void draw_clock(Canvas& c, int cx, int cy, int rad, double t) {
    // Face ring (12 tick marks) + center hub.
    for (int i = 0; i < 12; ++i) {
      double a = i / 12.0 * 6.2831853 - 1.5707963;
      int x = cx + static_cast<int>(std::cos(a) * rad);
      int y = cy + static_cast<int>(std::sin(a) * rad);
      c.fill_rect(x - 2, y - 2, 4, 4, Color{180, 185, 200});
    }
    c.fill_rect(cx - 3, cy - 3, 6, 6, Color{230, 230, 240});

    double secs = std::fmod(t, 43200.0);
    double hour = secs / 3600.0;          // 0..12
    double minute = std::fmod(secs / 60.0, 60.0);
    double second = std::fmod(secs, 60.0);

    auto hand = [&](double frac, int len, int thick, Color col) {
      double a = frac * 6.2831853 - 1.5707963;
      draw_line(c, cx, cy, cx + static_cast<int>(std::cos(a) * len),
                cy + static_cast<int>(std::sin(a) * len), thick, col);
    };
    hand(hour / 12.0, static_cast<int>(rad * 0.5), 4, Color{230, 230, 240});   // hour
    hand(minute / 60.0, static_cast<int>(rad * 0.8), 3, Color{200, 210, 230});  // minute
    hand(second / 60.0, static_cast<int>(rad * 0.9), 1, Color{230, 90, 90});    // second
  }

  int w_ = 0, h_ = 0, cell_ = 100, cols_ = 1, rows_ = 1;
  double time_ = 0;
  std::vector<double> offsets_;
};

}  // namespace

std::unique_ptr<Module> make_clocks() { return std::make_unique<Clocks>(); }

}  // namespace ad
