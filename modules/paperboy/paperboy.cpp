// Paperboy — the special first-release demo module.
//
// A side-scrolling street: the paperboy rides along (camera scrolls the world
// past a fixed rider), tossing papers onto undelivered houses and dodging the
// occasional obstacle. Runs fully autonomously as ambient eye candy; in
// interactive mode (harness --play) the player steers and throws.
//
// Deliberately a *demo* of the new "playable module" category — one street,
// paper tossing, dodging, score, and a day->dusk->night palette cycle. It
// exercises the Core systems (scrolling, sprites-as-rects, input, HUD) that
// future interactive modules will reuse.
#include "paperboy.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <unordered_map>
#include <vector>

#include "afterdark/draw.h"
#include "afterdark/sprite.h"
#include "afterdark/text.h"

namespace ad {
namespace {

struct House {
  double world_x;   // position along the street
  int width;
  int height;
  bool delivered = false;
};

struct Paper {
  double x, y, vx, vy;
  bool active = false;
};

struct Obstacle {
  double world_x;
  int width = 26;
  int height = 34;
};

Color sky_color(double phase) {
  const Color day{135, 206, 235}, dusk{250, 140, 80}, night{18, 18, 56};
  if (phase < 0.34) return lerp(day, dusk, phase / 0.34);
  if (phase < 0.67) return lerp(dusk, night, (phase - 0.34) / 0.33);
  return lerp(night, day, (phase - 0.67) / 0.33);
}

class Paperboy : public Module {
 public:
  ModuleInfo info() const override {
    return {"paperboy", "Paperboy", "1.0.0", Category::Interactive};
  }

  std::vector<SettingDesc> settings_schema() const override {
    return {
        {"scroll_speed", "Ride speed", SettingType::Enum, "normal", 0, 0,
         {"slow", "normal", "fast"}},
        {"day_length", "Day cycle (seconds)", SettingType::Int, "40", 10, 300, {}},
        {"show_score", "Show delivery score", SettingType::Bool, "true", 0, 0, {}},
    };
  }

  void init(Context& ctx) override {
    w_ = ctx.screen_w;
    h_ = ctx.screen_h;
    ctx_interactive_ = ctx.interactive;

    // Config (see modules/paperboy/module.json).
    std::string sp = ctx.settings.get("scroll_speed", "normal");
    scroll_speed_ = sp == "slow" ? 90.0 : sp == "fast" ? 240.0 : 150.0;
    day_len_ = ctx.settings.get_int("day_length", 40);
    if (day_len_ < 1) day_len_ = 1;
    show_score_ = ctx.settings.get_bool("show_score", true);
    street_top_ = h_ * 0.62;
    street_bot_ = h_ * 0.92;
    rider_x_ = w_ * 0.18;
    rider_y_ = (street_top_ + street_bot_) * 0.5;
    target_y_ = rider_y_;
    camera_x_ = 0;
    score_ = 0;
    throw_timer_ = 0;
    houses_.clear();
    papers_.clear();
    obstacles_.clear();
    // Seed the street ahead of the rider.
    double x = w_ * 0.5;
    for (int i = 0; i < 12; ++i) {
      spawn_house(ctx, x);
      x += ctx.rng.range(220, 360);
    }
    next_obstacle_x_ = camera_x_ + ctx.rng.range(400, 900);
  }

  void tick(Context& ctx, double dt) override {
    c_time_ += dt;  // drives the day/dusk/night palette cycle in draw()
    camera_x_ += scroll_speed_ * dt;  // px/s the world moves past the rider

    // Keep the street populated ahead and recycle what scrolls off-screen.
    double rightmost = 0;
    for (const auto& hsh : houses_)
      rightmost = std::max(rightmost, hsh.world_x + hsh.width);
    while (rightmost - camera_x_ < w_ + 200) {
      double nx = rightmost + ctx.rng.range(220, 360);
      spawn_house(ctx, nx);
      rightmost = nx + houses_.back().width;
    }
    houses_.erase(std::remove_if(houses_.begin(), houses_.end(),
                                 [&](const House& hsh) {
                                   return hsh.world_x + hsh.width - camera_x_ < -50;
                                 }),
                  houses_.end());

    // Obstacles on the street.
    if (camera_x_ > next_obstacle_x_) {
      obstacles_.push_back({camera_x_ + w_ + 40});
      next_obstacle_x_ = camera_x_ + ctx.rng.range(500, 1100);
    }
    obstacles_.erase(std::remove_if(obstacles_.begin(), obstacles_.end(),
                                    [&](const Obstacle& o) {
                                      return o.world_x - camera_x_ < -60;
                                    }),
                     obstacles_.end());

    if (!ctx.interactive) ai_drive(ctx, dt);

    // Smoothly move toward the target lane (player- or AI-chosen).
    rider_y_ += (target_y_ - rider_y_) * std::min(1.0, dt * 8.0);
    rider_y_ = std::clamp(rider_y_, street_top_, street_bot_);

    // Advance papers (projectile arc) and resolve deliveries.
    const double gravity = 900.0;
    for (auto& p : papers_) {
      if (!p.active) continue;
      p.x += p.vx * dt;
      p.y += p.vy * dt;
      p.vy += gravity * dt;
      if (p.y > street_top_ + 4 || p.x - camera_x_ > w_ + 40 || p.x < camera_x_ - 40)
        p.active = false;
      else
        try_deliver(p);
    }
    papers_.erase(std::remove_if(papers_.begin(), papers_.end(),
                                 [](const Paper& p) { return !p.active; }),
                  papers_.end());
  }

  bool on_event(Context& ctx, const Event& e) override {
    if (!ctx.interactive) return false;  // ambient mode: let host dismiss
    if (e.type == EventType::KeyDown) {
      if (e.key == Key::Up) target_y_ = street_top_;
      else if (e.key == Key::Down) target_y_ = street_bot_;
      else if (e.key == Key::Space) throw_paper();
      else return false;
      return true;
    }
    return false;
  }

  void draw(Canvas& c) override {
    const double day = std::fmod(c_time_, day_len_) / day_len_;
    c.clear(sky_color(day));

    // Sun/moon arcs across the sky with the day cycle.
    int sun = static_cast<int>(w_ * (0.1 + 0.8 * day));
    int sun_y = static_cast<int>(h_ * 0.12 + std::sin(day * 3.14159) * -h_ * 0.05);
    Color orb = day < 0.6 ? Color{255, 245, 200} : Color{230, 230, 245};
    c.fill_rect(sun - 18, sun_y - 18, 36, 36, orb);

    // Ground / street.
    c.fill_rect(0, static_cast<int>(street_top_), w_,
                h_ - static_cast<int>(street_top_), Color{70, 70, 80});
    c.fill_rect(0, static_cast<int>(street_top_), w_, 4, Color{120, 120, 130});

    // Houses (lawn + body + roof + door). Delivered ones light a window.
    for (const auto& hsh : houses_) {
      int hx = static_cast<int>(hsh.world_x - camera_x_);
      if (hx > w_ || hx + hsh.width < 0) continue;
      int base = static_cast<int>(street_top_);
      int top = base - hsh.height;
      c.fill_rect(hx, base - 6, hsh.width, 6, Color{60, 130, 60});      // lawn
      c.fill_rect(hx, top, hsh.width, hsh.height, Color{180, 150, 120});  // body
      c.fill_rect(hx - 6, top, hsh.width + 12, 12, Color{140, 60, 50});   // roof
      c.fill_rect(hx + hsh.width / 2 - 7, base - 26, 14, 26, Color{90, 60, 40});  // door
      Color win = hsh.delivered ? Color{255, 220, 120} : Color{120, 150, 170};
      c.fill_rect(hx + 8, top + 16, 14, 14, win);
    }

    // Obstacles (trash cans).
    for (const auto& o : obstacles_) {
      int ox = static_cast<int>(o.world_x - camera_x_);
      c.fill_rect(ox, static_cast<int>(street_top_) - o.height, o.width,
                  o.height, Color{40, 90, 110});
    }

    // Papers in flight.
    for (const auto& p : papers_) {
      if (!p.active) continue;
      c.fill_rect(static_cast<int>(p.x - camera_x_), static_cast<int>(p.y), 8, 6,
                  Color{245, 245, 235});
    }

    draw_rider(c);

    // Score HUD (real text via the built-in font).
    if (show_score_)
      draw_text(c, 14, 14, "SCORE " + std::to_string(score_), 4,
                Color{255, 215, 0});
    if (ctx_interactive_)
      draw_text(c, 14, 44, "UP/DOWN MOVE  SPACE THROW", 2, Color{230, 230, 230});
  }

 private:
  static const Sprite& rider_sprite() {
    static const Sprite s(
        {
            "...HHHHH..",
            "..HHHHHHH.",
            "...SSSS...",
            "...SSSS...",
            "..BBBBBB.S",  // arm reaches forward (skin tip)
            ".BBBBBBBSS",
            ".BBBBBBBB.",
            ".bBBBBBb..",
            ".bBBBBb...",
            "..LLLLL...",
            "..LL.LL...",
            ".LL...LL..",
        },
        {{'H', {215, 65, 55}},   // cap
         {'S', {235, 200, 160}}, // skin
         {'B', {55, 105, 215}},  // shirt
         {'b', {38, 78, 175}},   // shirt shadow
         {'L', {60, 60, 92}}});  // pants
    return s;
  }

  void draw_rider(Canvas& c) {
    int rx = static_cast<int>(rider_x_);
    int ry = static_cast<int>(rider_y_);
    const Color tire{25, 25, 30}, spoke{150, 155, 165}, frame{200, 50, 45};
    int wy = ry + 14, wr = 9;
    double spin = camera_x_ * 0.12;  // wheels spin with travel
    for (int wx : {rx - 12, rx + 12}) {
      fill_circle(c, wx, wy, wr, tire);
      for (int k = 0; k < 4; ++k) {
        double a = spin + k * 1.5707963;
        line(c, wx, wy, wx + static_cast<int>(std::cos(a) * (wr - 2)),
             wy + static_cast<int>(std::sin(a) * (wr - 2)), 1, spoke);
      }
      c.fill_rect(wx - 1, wy - 1, 3, 3, spoke);  // hub
    }
    line(c, rx - 12, wy, rx + 12, wy, 3, frame);   // down tube
    line(c, rx + 12, wy, rx + 4, ry, 3, frame);    // seat/fork
    const Sprite& r = rider_sprite();
    r.blit(c, rx - r.width() * 3 / 2 + 2, ry - 24, 3);
  }

  void spawn_house(Context& ctx, double world_x) {
    House hsh;
    hsh.world_x = world_x;
    hsh.width = ctx.rng.range(70, 110);
    hsh.height = ctx.rng.range(70, 120);
    houses_.push_back(hsh);
  }

  void throw_paper() {
    Paper p;
    p.x = rider_x_ + camera_x_;
    p.y = rider_y_ - 16;
    p.vx = 260;
    p.vy = -260;
    p.active = true;
    papers_.push_back(p);
  }

  void try_deliver(Paper& p) {
    for (auto& hsh : houses_) {
      if (hsh.delivered) continue;
      double left = hsh.world_x, right = hsh.world_x + hsh.width;
      if (p.x >= left && p.x <= right && p.y >= street_top_ - hsh.height &&
          p.y <= street_top_) {
        hsh.delivered = true;
        ++score_;
        p.active = false;
        return;
      }
    }
  }

  void ai_drive(Context& ctx, double dt) {
    // Dodge: if an obstacle is close ahead, hop to the far lane.
    for (const auto& o : obstacles_) {
      double sx = o.world_x - camera_x_;
      if (sx > rider_x_ - 20 && sx < rider_x_ + 120) {
        target_y_ = street_top_;  // hop up over the trash can
      }
    }
    if (obstacles_.empty() || target_y_ == street_top_) {
      // Drift back toward delivering lane when clear.
      bool clear = true;
      for (const auto& o : obstacles_) {
        double sx = o.world_x - camera_x_;
        if (sx > rider_x_ - 20 && sx < rider_x_ + 120) clear = false;
      }
      if (clear) target_y_ = (street_top_ + street_bot_) * 0.55;
    }

    // Aim: throw at the nearest undelivered house just ahead of the rider.
    throw_timer_ -= dt;
    if (throw_timer_ <= 0) {
      for (const auto& hsh : houses_) {
        double sx = hsh.world_x - camera_x_;
        if (!hsh.delivered && sx > rider_x_ + 30 && sx < rider_x_ + 220) {
          throw_paper();
          throw_timer_ = ctx.rng.next_double() * 0.4 + 0.5;
          break;
        }
      }
    }
  }

  // Local clock for the palette cycle (draw() has no Context; tick advances it).
  double c_time_ = 0;

  // Config, resolved from Settings in init().
  double scroll_speed_ = 150.0;  // px/s
  double day_len_ = 40.0;        // seconds per full day cycle
  bool show_score_ = true;

  int w_ = 0, h_ = 0;
  bool ctx_interactive_ = false;
  double street_top_ = 0, street_bot_ = 0;
  double rider_x_ = 0, rider_y_ = 0, target_y_ = 0;
  double camera_x_ = 0;
  double throw_timer_ = 0;
  double next_obstacle_x_ = 0;
  int score_ = 0;
  std::vector<House> houses_;
  std::vector<Paper> papers_;
  std::vector<Obstacle> obstacles_;
};

}  // namespace

std::unique_ptr<Module> make_paperboy() {
  return std::make_unique<Paperboy>();
}

}  // namespace ad
