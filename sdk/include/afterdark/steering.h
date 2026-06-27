// Reusable 2D vector math + boid-style flocking steering.
//
// Shared movement helper for modules with autonomous agents (Fish!, and later
// anything that wanders or flocks). Header-only, no dependencies beyond the
// standard library.
#pragma once

#include <cmath>
#include <cstddef>
#include <vector>

namespace ad {

struct Vec2 {
  double x = 0, y = 0;
  Vec2() = default;
  Vec2(double x_, double y_) : x(x_), y(y_) {}
  Vec2 operator+(Vec2 o) const { return {x + o.x, y + o.y}; }
  Vec2 operator-(Vec2 o) const { return {x - o.x, y - o.y}; }
  Vec2 operator*(double s) const { return {x * s, y * s}; }
  Vec2& operator+=(Vec2 o) { x += o.x; y += o.y; return *this; }
};

inline double length(Vec2 v) { return std::sqrt(v.x * v.x + v.y * v.y); }

inline Vec2 normalize(Vec2 v) {
  double l = length(v);
  return l > 1e-9 ? Vec2{v.x / l, v.y / l} : Vec2{0, 0};
}

inline Vec2 limit(Vec2 v, double max_len) {
  double l = length(v);
  return (l > max_len && l > 1e-9) ? v * (max_len / l) : v;
}

struct FlockParams {
  double perception = 60;   // neighbor radius
  double separation = 22;   // "too close" radius
  double sep_weight = 1.6;
  double ali_weight = 1.0;
  double coh_weight = 0.9;
  double max_speed = 90;
  double max_force = 200;
};

// Classic Reynolds boids: separation + alignment + cohesion. Returns a steering
// acceleration for agent i (O(n^2); fine for the modest counts modules use).
inline Vec2 flock_steer(std::size_t i, const std::vector<Vec2>& pos,
                        const std::vector<Vec2>& vel, const FlockParams& p) {
  Vec2 sep{0, 0}, ali{0, 0}, coh{0, 0};
  int sep_n = 0, ali_n = 0, coh_n = 0;
  for (std::size_t j = 0; j < pos.size(); ++j) {
    if (j == i) continue;
    Vec2 d = pos[i] - pos[j];
    double dist = length(d);
    if (dist < 1e-6) continue;
    if (dist < p.separation) {
      sep += normalize(d) * (1.0 / dist);
      ++sep_n;
    }
    if (dist < p.perception) {
      ali += vel[j];
      ++ali_n;
      coh += pos[j];
      ++coh_n;
    }
  }

  Vec2 steer{0, 0};
  auto seek = [&](Vec2 desired) {
    desired = normalize(desired) * p.max_speed;
    return limit(desired - vel[i], p.max_force);
  };
  if (sep_n) steer += seek(sep * (1.0 / sep_n)) * p.sep_weight;
  if (ali_n) steer += seek(ali * (1.0 / ali_n)) * p.ali_weight;
  if (coh_n) steer += seek((coh * (1.0 / coh_n)) - pos[i]) * p.coh_weight;
  return steer;
}

}  // namespace ad
