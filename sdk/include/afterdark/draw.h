// Drawing helpers built on Canvas::fill_rect (which blends when Color.a < 255).
// These give modules smooth/modern primitives — anti-aliased-ish circles,
// gradients, glows, and lines — without each backend implementing them.
#pragma once

#include <cmath>

#include "afterdark/afterdark.h"

namespace ad {

// Vertical gradient fill from top color to bottom color over [y, y+h).
inline void v_gradient(Canvas& c, int x, int y, int w, int h, Color top,
                       Color bottom, int bands = 32) {
  if (h <= 0) return;
  if (bands > h) bands = h;
  for (int i = 0; i < bands; ++i) {
    double t = bands > 1 ? static_cast<double>(i) / (bands - 1) : 0;
    int y0 = y + h * i / bands, y1 = y + h * (i + 1) / bands;
    c.fill_rect(x, y0, w, y1 - y0, lerp(top, bottom, t));
  }
}

// Filled circle with a 1px alpha-feathered edge (cheap anti-aliasing) using
// horizontal spans. Honors col.a.
inline void fill_circle(Canvas& c, int cx, int cy, int r, Color col) {
  if (r <= 0) return;
  for (int dy = -r; dy <= r; ++dy) {
    double dx = static_cast<double>(r) * r - static_cast<double>(dy) * dy;
    if (dx < 0) continue;
    int half = static_cast<int>(std::sqrt(dx));
    c.fill_rect(cx - half, cy + dy, half * 2 + 1, 1, col);
    // Feather the left/right edge pixel at half opacity.
    Color edge = col;
    edge.a = static_cast<uint8_t>(col.a / 2);
    c.fill_rect(cx - half - 1, cy + dy, 1, 1, edge);
    c.fill_rect(cx + half + 1, cy + dy, 1, 1, edge);
  }
}

// Soft glow: concentric translucent rings fading outward. Great for stars,
// lights, particles. `layers` controls smoothness.
inline void glow(Canvas& c, int cx, int cy, int radius, Color col,
                 int layers = 5) {
  for (int i = layers; i >= 1; --i) {
    int r = radius * i / layers;
    Color g = col;
    g.a = static_cast<uint8_t>(col.a * (layers - i + 1) / (layers * 3 + 1));
    fill_circle(c, cx, cy, r, g);
  }
  fill_circle(c, cx, cy, radius / layers + 1, col);  // bright core
}

// Thick line as a run of small rects (honors col.a).
inline void line(Canvas& c, int x0, int y0, int x1, int y1, int thick,
                 Color col) {
  int dx = x1 - x0, dy = y1 - y0;
  int adx = dx < 0 ? -dx : dx, ady = dy < 0 ? -dy : dy;
  int steps = (adx > ady ? adx : ady);
  if (steps < 1) steps = 1;
  int t2 = thick / 2;
  for (int i = 0; i <= steps; ++i) {
    int x = x0 + dx * i / steps, y = y0 + dy * i / steps;
    c.fill_rect(x - t2, y - t2, thick, thick, col);
  }
}

}  // namespace ad
