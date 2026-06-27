// Globe — recreation.
//
// A slowly rotating wireframe Earth: latitude/longitude grid points spun about
// a tilted axis and projected orthographically, with front-facing points drawn
// brighter than those wrapping around the back. A scattering of "land" points
// adds continents. First module with real 3D projection math (reused later by
// any rotating-solid module). Original art — no Berkeley assets.
#include "globe.h"

#include <algorithm>
#include <cmath>
#include <vector>

namespace ad {
namespace {

struct P3 {
  double x, y, z;
  bool land;
};

class Globe : public Module {
 public:
  ModuleInfo info() const override {
    return {"globe", "Globe", "1.0.0", Category::Ambient};
  }

  void init(Context& ctx) override {
    w_ = ctx.screen_w;
    h_ = ctx.screen_h;
    radius_ = std::min(w_, h_) * 0.36;
    pts_.clear();
    // Grid of meridian/parallel sample points.
    for (int lat = -80; lat <= 80; lat += 10) {
      for (int lon = 0; lon < 360; lon += 6) {
        pts_.push_back(make_point(lat, lon, false));
      }
    }
    // Sparse "continents": random clusters of land points.
    for (int i = 0; i < 6; ++i) {
      int clat = ctx.rng.range(-50, 60);
      int clon = ctx.rng.range(0, 359);
      int n = ctx.rng.range(40, 90);
      for (int k = 0; k < n; ++k) {
        int la = std::clamp(clat + ctx.rng.range(-18, 18), -89, 89);
        int lo = (clon + ctx.rng.range(-22, 22) + 360) % 360;
        pts_.push_back(make_point(la, lo, true));
      }
    }
  }

  void tick(Context&, double dt) override { angle_ += dt * 0.4; }

  void draw(Canvas& c) override {
    c.clear(Color{2, 4, 16});
    int cx = w_ / 2, cy = h_ / 2;
    const double tilt = 0.41;  // ~23.5 deg axial tilt
    double ca = std::cos(angle_), sa = std::sin(angle_);
    double ct = std::cos(tilt), st = std::sin(tilt);

    for (const auto& p : pts_) {
      // Rotate about the vertical axis, then apply axial tilt.
      double x = p.x * ca + p.z * sa;
      double z = -p.x * sa + p.z * ca;
      double y = p.y * ct - z * st;
      double zt = p.y * st + z * ct;

      int px = static_cast<int>(cx + x * radius_);
      int py = static_cast<int>(cy + y * radius_);
      bool front = zt >= 0;  // toward viewer
      double depth = (zt + 1) * 0.5;  // 0 back .. 1 front

      Color col;
      if (p.land)
        col = front ? Color{90, 200, 110} : Color{30, 80, 45};
      else {
        uint8_t b = static_cast<uint8_t>(40 + depth * 150);
        col = {static_cast<uint8_t>(b / 2), static_cast<uint8_t>(b / 2), b};
      }
      int s = front ? 2 : 1;
      c.fill_rect(px, py, s, s, col);
    }
  }

 private:
  P3 make_point(int lat_deg, int lon_deg, bool land) {
    double lat = lat_deg * 3.14159265 / 180.0;
    double lon = lon_deg * 3.14159265 / 180.0;
    return {std::cos(lat) * std::cos(lon), std::sin(lat),
            std::cos(lat) * std::sin(lon), land};
  }

  int w_ = 0, h_ = 0;
  double radius_ = 100, angle_ = 0;
  std::vector<P3> pts_;
};

}  // namespace

std::unique_ptr<Module> make_globe() { return std::make_unique<Globe>(); }

}  // namespace ad
