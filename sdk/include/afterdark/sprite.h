// Sprite system: small pixel-art images authored inline as text grids, blitted
// to a Canvas with per-pixel alpha. No external image files or decoders needed —
// the "hand-drawn" marquee art (toasters, paperboy, cat, ...) is authored as
// character grids mapped through a palette. '.' or ' ' = transparent.
#pragma once

#include <algorithm>
#include <string>
#include <unordered_map>
#include <vector>

#include "afterdark/afterdark.h"

namespace ad {

class Sprite {
 public:
  Sprite() = default;

  // Build from rows of single-char pixels. `palette` maps a char to a Color;
  // chars '.' and ' ' are always transparent. All rows should be equal length;
  // shorter rows are padded with transparency.
  Sprite(const std::vector<std::string>& rows,
         const std::unordered_map<char, Color>& palette) {
    h_ = static_cast<int>(rows.size());
    for (const auto& r : rows) w_ = std::max<int>(w_, static_cast<int>(r.size()));
    px_.assign(static_cast<size_t>(w_) * h_, Color{0, 0, 0, 0});
    for (int y = 0; y < h_; ++y) {
      const std::string& r = rows[y];
      for (int x = 0; x < static_cast<int>(r.size()); ++x) {
        char ch = r[x];
        if (ch == '.' || ch == ' ') continue;
        auto it = palette.find(ch);
        if (it != palette.end()) px_[y * w_ + x] = it->second;
      }
    }
  }

  int width() const { return w_; }
  int height() const { return h_; }
  const Color& at(int x, int y) const { return px_[y * w_ + x]; }

  // Draw at (x,y) top-left, each source pixel scaled to scale×scale. `flip_x`
  // mirrors horizontally. `tint_a` scales overall opacity (255 = opaque).
  void blit(Canvas& c, int x, int y, int scale = 1, bool flip_x = false,
            uint8_t tint_a = 255) const {
    for (int sy = 0; sy < h_; ++sy) {
      for (int sx = 0; sx < w_; ++sx) {
        Color p = px_[sy * w_ + (flip_x ? (w_ - 1 - sx) : sx)];
        if (p.a == 0) continue;
        if (tint_a != 255) p.a = static_cast<uint8_t>(p.a * tint_a / 255);
        c.fill_rect(x + sx * scale, y + sy * scale, scale, scale, p);
      }
    }
  }

 private:
  int w_ = 0, h_ = 0;
  std::vector<Color> px_;
};

}  // namespace ad
