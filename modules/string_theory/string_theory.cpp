// String Theory — recreation.
//
// Animated string-art: two endpoints trace Lissajous paths around the screen,
// and a fan of straight "strings" is drawn between interpolated points along
// each, producing the classic envelope curves. Color cycles slowly and the
// whole figure breathes as the endpoints move. Original art — no Berkeley assets.
#include "string_theory.h"

#include <algorithm>
#include <cmath>

namespace ad {
namespace {

void line(Canvas& c, int x0, int y0, int x1, int y1, Color col) {
  int dx = x1 - x0, dy = y1 - y0;
  int steps = std::max(std::abs(dx), std::abs(dy)) / 2;
  if (steps < 1) steps = 1;
  for (int i = 0; i <= steps; ++i)
    c.fill_rect(x0 + dx * i / steps, y0 + dy * i / steps, 1, 1, col);
}

class StringTheory : public Module {
 public:
  ModuleInfo info() const override {
    return {"string-theory", "String Theory", "1.0.0", Category::Ambient};
  }

  std::vector<SettingDesc> settings_schema() const override {
    return {{"strings", "Strings per figure", SettingType::Int, "14", 4, 40, {}}};
  }

  void init(Context& ctx) override {
    w_ = ctx.screen_w;
    h_ = ctx.screen_h;
    n_ = ctx.settings.get_int("strings", 14);
  }

  void tick(Context&, double dt) override {
    t_ += dt;
    hue_ += dt * 30;
  }

  void draw(Canvas& c) override {
    c.clear(Color{0, 0, 0});
    // Four moving anchor points (two segments) on Lissajous paths.
    auto px = [&](double freq, double phase) {
      return static_cast<int>(w_ * (0.5 + 0.42 * std::sin(t_ * freq + phase)));
    };
    auto py = [&](double freq, double phase) {
      return static_cast<int>(h_ * (0.5 + 0.42 * std::sin(t_ * freq + phase)));
    };
    int ax = px(0.7, 0), ay = py(0.9, 0);
    int bx = px(1.1, 2.0), by = py(0.6, 1.0);
    int cx = px(0.5, 4.0), cy = py(1.3, 3.0);
    int dx = px(0.9, 5.5), dy = py(0.8, 2.0);

    for (int i = 0; i <= n_; ++i) {
      double f = static_cast<double>(i) / n_;
      int sx = ax + static_cast<int>((bx - ax) * f);
      int sy = ay + static_cast<int>((by - ay) * f);
      int ex = cx + static_cast<int>((dx - cx) * f);
      int ey = cy + static_cast<int>((dy - cy) * f);
      Color col = hsv(hue_ + f * 120.0, 0.7, 1.0);
      line(c, sx, sy, ex, ey, col);
    }
  }

 private:
  Color hsv(double h, double s, double v) {
    h = std::fmod(h, 360.0);
    if (h < 0) h += 360.0;
    double cc = v * s, x = cc * (1 - std::fabs(std::fmod(h / 60.0, 2) - 1));
    double m = v - cc, r = 0, g = 0, b = 0;
    if (h < 60) { r = cc; g = x; }
    else if (h < 120) { r = x; g = cc; }
    else if (h < 180) { g = cc; b = x; }
    else if (h < 240) { g = x; b = cc; }
    else if (h < 300) { r = x; b = cc; }
    else { r = cc; b = x; }
    return {static_cast<uint8_t>((r + m) * 255),
            static_cast<uint8_t>((g + m) * 255),
            static_cast<uint8_t>((b + m) * 255)};
  }

  int w_ = 0, h_ = 0, n_ = 14;
  double t_ = 0, hue_ = 0;
};

}  // namespace

std::unique_ptr<Module> make_string_theory() {
  return std::make_unique<StringTheory>();
}

}  // namespace ad
