// Render/Platform Abstraction Layer (RAL) backend interface.
//
// A backend owns the window/surface, pumps OS events, and hands the module a
// Canvas each frame. Swapping backends (SDL2 <-> headless <-> native host)
// requires zero module changes.
#pragma once

#include <memory>
#include <string>

#include "afterdark/afterdark.h"

namespace ad {

class Backend {
 public:
  virtual ~Backend() = default;

  virtual bool init(int width, int height, const std::string& title) = 0;
  // Pop one pending event; returns false when the queue is empty this frame.
  virtual bool poll_event(Event& out) = 0;
  virtual Canvas& begin_frame() = 0;
  virtual void end_frame() = 0;
  virtual bool should_close() const = 0;
  virtual void shutdown() {}
};

// Headless backend: no window, no deps. Records draw-call counts so CI can
// smoke-test every module ("did it clear + draw without crashing?").
std::unique_ptr<Backend> make_null_backend();

#ifdef AD_HAVE_SDL2
std::unique_ptr<Backend> make_sdl_backend();
#endif

// Picks SDL2 when compiled in and not forced headless; otherwise the null
// backend. The harness uses this so the same binary works with or without a
// display.
std::unique_ptr<Backend> make_default_backend(bool force_headless);

// Headless-only stats accessor (returns 0 for non-null backends). Used by the
// harness smoke test to assert a module actually rendered.
long null_backend_rect_count(const Backend& b);

}  // namespace ad
