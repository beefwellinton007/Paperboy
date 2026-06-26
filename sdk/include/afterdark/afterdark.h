// AfterDark Module SDK — public types every module compiles against.
//
// This is the stable surface modules depend on. Keep it small and additive:
// breaking changes here ripple to every module in the catalog.
#pragma once

#include <cstdint>
#include <string>

namespace ad {

// ---------------------------------------------------------------------------
// Drawing primitives
// ---------------------------------------------------------------------------
struct Color {
  uint8_t r = 0, g = 0, b = 0, a = 255;
};

inline Color lerp(const Color& x, const Color& y, double t) {
  if (t < 0) t = 0;
  if (t > 1) t = 1;
  auto mix = [&](uint8_t a, uint8_t b) {
    return static_cast<uint8_t>(a + (b - a) * t);
  };
  return Color{mix(x.r, y.r), mix(x.g, y.g), mix(x.b, y.b), mix(x.a, y.a)};
}

// Minimal immediate-mode canvas. The render backend implements this; modules
// only ever see the abstract interface, so the same module code runs on the
// SDL2 backend, the native hosts, and the headless/CI backend unchanged.
class Canvas {
 public:
  virtual ~Canvas() = default;
  virtual int width() const = 0;
  virtual int height() const = 0;
  virtual void clear(Color c) = 0;
  virtual void fill_rect(int x, int y, int w, int h, Color c) = 0;
};

// ---------------------------------------------------------------------------
// Input / events
// ---------------------------------------------------------------------------
enum class EventType { None, KeyDown, KeyUp, PointerMove, PointerDown, Quit };
enum class Key { None, Left, Right, Up, Down, Space, Escape, Other };

struct Event {
  EventType type = EventType::None;
  Key key = Key::None;
  int x = 0, y = 0;
};

// ---------------------------------------------------------------------------
// Deterministic RNG (xorshift32). Modules should draw randomness from here
// rather than rand() so behavior is reproducible in tests and across hosts.
// ---------------------------------------------------------------------------
struct Rng {
  uint32_t state = 0x1234567u;
  explicit Rng(uint32_t seed = 0x1234567u) : state(seed ? seed : 1u) {}
  uint32_t next_u32() {
    uint32_t x = state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    return state = x;
  }
  double next_double() { return next_u32() / 4294967296.0; }  // [0,1)
  int range(int lo, int hi) {                                 // inclusive
    if (hi <= lo) return lo;
    return lo + static_cast<int>(next_u32() % static_cast<uint32_t>(hi - lo + 1));
  }
};

// ---------------------------------------------------------------------------
// Per-run context handed to every module callback.
// ---------------------------------------------------------------------------
struct Context {
  int screen_w = 0;
  int screen_h = 0;
  double time = 0;       // seconds since module init (host-advanced)
  bool interactive = false;  // true in playable/demo mode (harness --play)
  Rng rng;
};

// ---------------------------------------------------------------------------
// Module classification + identity
// ---------------------------------------------------------------------------
enum class Category {
  Ambient,      // classic eye-candy screensaver
  Interactive,  // playable / game-style module (e.g. Paperboy)
};

struct ModuleInfo {
  std::string id;       // stable slug, matches module.json "id"
  std::string name;     // display name
  std::string version;  // module's own semver, matches module.json "version"
  Category category = Category::Ambient;
};

// ---------------------------------------------------------------------------
// The module interface. A screensaver is anything implementing this.
// (A C ABI variant for dynamically-loaded third-party modules can wrap this
//  later; statically-linked first-party modules use the C++ class directly.)
// ---------------------------------------------------------------------------
class Module {
 public:
  virtual ~Module() = default;
  virtual ModuleInfo info() const = 0;
  virtual void init(Context&) {}
  virtual void tick(Context&, double /*dt*/) {}
  virtual void draw(Canvas&) {}
  // Return true if the event was consumed (interactive modules). Returning
  // false lets the host apply default behavior (e.g. dismiss the saver).
  virtual bool on_event(Context&, const Event&) { return false; }
};

}  // namespace ad
