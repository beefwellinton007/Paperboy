// Mowing Man — flagship recreation.
//
// A little guy pushes a mower back and forth across a lawn of tall grass,
// progressively clearing it into striped mowed rows. When the lawn is done it
// pauses, then the grass regrows and he starts over.
//
// Demonstrates the *persistent field* pattern: the module owns a logical grid
// of mowed/unmowed cells that survives frame-to-frame and is redrawn from state
// (no offscreen render target needed, so it's identical on every backend).
// Drawing coalesces mowed cells into horizontal runs to keep the rect count low.
// Original art — no Berkeley Systems assets.
#include "mowing_man.h"

#include <algorithm>
#include <cmath>
#include <vector>

namespace ad {
namespace {

class MowingMan : public Module {
 public:
  ModuleInfo info() const override {
    return {"mowing-man", "Mowing Man", "1.0.0", Category::Ambient};
  }

  void init(Context& ctx) override {
    w_ = ctx.screen_w;
    h_ = ctx.screen_h;
    cell_ = 12;
    cols_ = (w_ + cell_ - 1) / cell_;
    rows_ = (h_ + cell_ - 1) / cell_;
    band_cells_ = 3;  // mower swath height in cells
    reset_lawn();
    (void)ctx;
  }

  void tick(Context&, double dt) override {
    time_ += dt;

    if (done_) {
      regrow_timer_ -= dt;
      if (regrow_timer_ <= 0) reset_lawn();
      return;
    }

    // Drive the mower along the current band; reverse + drop a band at edges.
    mower_x_ += speed_ * dir_ * dt;
    const double half_w = mower_w_ * 0.5;
    if (dir_ > 0 && mower_x_ > w_ - half_w) {
      mower_x_ = w_ - half_w;
      next_band();
    } else if (dir_ < 0 && mower_x_ < half_w) {
      mower_x_ = half_w;
      next_band();
    }
    mow_under_mower();
  }

  void draw(Canvas& c) override {
    c.clear(Color{40, 110, 45});  // tall-grass base

    // Texture the unmowed lawn lightly (checker) — drawn first, behind stripes.
    for (int r = 0; r < rows_; ++r)
      for (int col = 0; col < cols_; ++col)
        if (!mowed_[r * cols_ + col] && ((r + col) & 1))
          c.fill_rect(col * cell_, r * cell_, cell_, cell_, Color{36, 100, 40});

    draw_mowed_runs(c);
    draw_mower(c);
  }

 private:
  void reset_lawn() {
    mowed_.assign(static_cast<std::size_t>(cols_) * rows_, 0);
    band_top_ = 0;
    mower_x_ = mower_w_ * 0.5;
    dir_ = 1;
    done_ = false;
    speed_ = 220;
  }

  void next_band() {
    band_top_ += band_cells_;
    dir_ = -dir_;
    if (band_top_ >= rows_) {
      done_ = true;
      regrow_timer_ = 4.0;
    }
  }

  void mow_under_mower() {
    int c0 = std::max(0, static_cast<int>((mower_x_ - mower_w_ * 0.5) / cell_));
    int c1 = std::min(cols_ - 1, static_cast<int>((mower_x_ + mower_w_ * 0.5) / cell_));
    int r0 = std::max(0, band_top_);
    int r1 = std::min(rows_ - 1, band_top_ + band_cells_ - 1);
    for (int r = r0; r <= r1; ++r)
      for (int col = c0; col <= c1; ++col) mowed_[r * cols_ + col] = 1;
  }

  void draw_mowed_runs(Canvas& c) {
    for (int r = 0; r < rows_; ++r) {
      // Alternate stripe shade by band so it reads as a freshly mowed lawn.
      bool light = ((r / band_cells_) & 1) == 0;
      Color stripe = light ? Color{120, 190, 95} : Color{95, 170, 80};
      int col = 0;
      while (col < cols_) {
        if (!mowed_[r * cols_ + col]) { ++col; continue; }
        int start = col;
        while (col < cols_ && mowed_[r * cols_ + col]) ++col;
        c.fill_rect(start * cell_, r * cell_, (col - start) * cell_, cell_, stripe);
      }
    }
  }

  void draw_mower(Canvas& c) {
    int mx = static_cast<int>(mower_x_);
    int my = (band_top_ + band_cells_ / 2) * cell_ + cell_ / 2;
    int facing = dir_;
    // Mower deck + wheels.
    c.fill_rect(mx - 16, my - 6, 32, 14, Color{200, 60, 40});
    c.fill_rect(mx - 14, my + 6, 8, 8, Color{20, 20, 20});
    c.fill_rect(mx + 6, my + 6, 8, 8, Color{20, 20, 20});
    // Handle angled back from the mower toward the man.
    int hx = mx - facing * 18;
    c.fill_rect(std::min(mx, hx), my - 22, std::abs(mx - hx) + 3, 3,
                Color{60, 60, 60});
    // The man: legs, body, head, behind the handle.
    int px = mx - facing * 24;
    c.fill_rect(px - 3, my - 4, 3, 12, Color{40, 40, 120});   // back leg
    c.fill_rect(px + 1, my - 4, 3, 12, Color{40, 40, 120});   // front leg
    c.fill_rect(px - 4, my - 20, 9, 16, Color{220, 200, 60}); // shirt
    c.fill_rect(px - 3, my - 30, 7, 8, Color{235, 200, 160}); // head
    c.fill_rect(px - 4, my - 32, 9, 3, Color{120, 70, 40});   // hat brim
  }

  int w_ = 0, h_ = 0, cell_ = 12, cols_ = 0, rows_ = 0;
  int band_cells_ = 3, band_top_ = 0, dir_ = 1;
  double mower_x_ = 0, mower_w_ = 34, speed_ = 220, time_ = 0;
  double regrow_timer_ = 0;
  bool done_ = false;
  std::vector<uint8_t> mowed_;
};

}  // namespace

std::unique_ptr<Module> make_mowing_man() {
  return std::make_unique<MowingMan>();
}

}  // namespace ad
