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

static_assert(sizeof(Color) == 4, "Color must be 4 tightly-packed bytes");

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

  // Build from a tightly-packed RGBA buffer (w*h*4 bytes) — the form produced
  // by the asset pipeline from PNGs.
  static Sprite from_rgba(int w, int h, const unsigned char* rgba) {
    Sprite s;
    s.w_ = w;
    s.h_ = h;
    s.px_.resize(static_cast<size_t>(w) * h);
    for (int i = 0; i < w * h; ++i)
      s.px_[i] = Color{rgba[i * 4], rgba[i * 4 + 1], rgba[i * 4 + 2],
                       rgba[i * 4 + 3]};
    return s;
  }

  bool empty() const { return px_.empty(); }
  int width() const { return w_; }
  int height() const { return h_; }
  const Color& at(int x, int y) const { return px_[y * w_ + x]; }
  // Contiguous RGBA bytes (Color is 4 tightly-packed bytes).
  const unsigned char* rgba() const {
    return reinterpret_cast<const unsigned char*>(px_.data());
  }

  // Draw at (x,y) top-left, integer-scaled. `flip_x` mirrors; `tint_a` scales
  // opacity. Routes through Canvas::draw_rgba (one GPU call on real backends).
  void blit(Canvas& c, int x, int y, int scale = 1, bool flip_x = false,
            uint8_t tint_a = 255) const {
    if (px_.empty()) return;
    c.draw_rgba(this, rgba(), w_, h_, x, y, w_ * scale, h_ * scale, flip_x,
                tint_a);
  }

  // Draw scaled to an explicit destination size (smooth scaling on real
  // backends) — use for high-res art sized to the screen.
  void blit_scaled(Canvas& c, int x, int y, int dw, int dh, bool flip_x = false,
                   uint8_t tint_a = 255) const {
    if (px_.empty()) return;
    c.draw_rgba(this, rgba(), w_, h_, x, y, dw, dh, flip_x, tint_a);
  }

 private:
  int w_ = 0, h_ = 0;
  std::vector<Color> px_;
};

}  // namespace ad
