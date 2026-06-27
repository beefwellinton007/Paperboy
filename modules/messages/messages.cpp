// Messages — recreation.
//
// A line of text drifts around the screen, bouncing off the edges and changing
// color on each bounce, cycling through a list of messages. The classic "type
// your own message" module; the message list is a placeholder until the config
// UI can feed user text in via the manifest settings. Original art.
#include "messages.h"

#include <string>
#include <vector>

#include "afterdark/draw.h"
#include "afterdark/text.h"

namespace ad {
namespace {

class Messages : public Module {
 public:
  ModuleInfo info() const override {
    return {"messages", "Messages", "1.0.0", Category::Ambient};
  }

  void init(Context& ctx) override {
    w_ = ctx.screen_w;
    h_ = ctx.screen_h;
    scale_ = std::max(4, w_ / 90);
    msgs_ = {"AFTER DARK", "FLYING TOASTERS", "GOOD NIGHT",
             "PAPERBOY DELIVERS", "SCREENSAVER 2.0"};
    idx_ = 0;
    x_ = w_ * 0.2;
    y_ = h_ * 0.4;
    vx_ = 110;
    vy_ = 80;
    color_ = pick(ctx);
    swap_timer_ = 5.0;
  }

  void tick(Context& ctx, double dt) override {
    x_ += vx_ * dt;
    y_ += vy_ * dt;
    int tw = text_width(msgs_[idx_], scale_);
    int th = 5 * scale_;
    bool bounced = false;
    if (x_ < 0) { x_ = 0; vx_ = -vx_; bounced = true; }
    if (x_ + tw > w_) { x_ = w_ - tw; vx_ = -vx_; bounced = true; }
    if (y_ < 0) { y_ = 0; vy_ = -vy_; bounced = true; }
    if (y_ + th > h_) { y_ = h_ - th; vy_ = -vy_; bounced = true; }
    if (bounced) color_ = pick(ctx);

    swap_timer_ -= dt;
    if (swap_timer_ <= 0) {
      idx_ = (idx_ + 1) % msgs_.size();
      swap_timer_ = 5.0;
      color_ = pick(ctx);
    }
  }

  void draw(Canvas& c) override {
    v_gradient(c, 0, 0, w_, h_, Color{14, 12, 26}, Color{4, 4, 10});
    int tx = static_cast<int>(x_), ty = static_cast<int>(y_);
    // Neon glow: a translucent offset halo behind the crisp text.
    Color halo = color_;
    halo.a = 70;
    for (int dx = -1; dx <= 1; ++dx)
      for (int dy = -1; dy <= 1; ++dy)
        if (dx || dy)
          draw_text(c, tx + dx * scale_, ty + dy * scale_, msgs_[idx_], scale_,
                    halo);
    draw_text(c, tx, ty, msgs_[idx_], scale_, color_);
  }

 private:
  Color pick(Context& ctx) {
    static const Color p[] = {{255, 80, 80},  {80, 255, 120}, {110, 160, 255},
                              {255, 220, 90}, {235, 120, 235}, {120, 235, 235}};
    return p[ctx.rng.range(0, 5)];
  }

  int w_ = 0, h_ = 0, scale_ = 4;
  std::size_t idx_ = 0;
  double x_ = 0, y_ = 0, vx_ = 100, vy_ = 80, swap_timer_ = 5;
  Color color_{255, 255, 255};
  std::vector<std::string> msgs_;
};

}  // namespace

std::unique_ptr<Module> make_messages() {
  return std::make_unique<Messages>();
}

}  // namespace ad
