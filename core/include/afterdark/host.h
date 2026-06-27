// HostSession — the shared per-frame driver used by every host (dev harness,
// Windows .scr, macOS .saver).
//
// Native hosts own their own timer/event pump (WinMain message loop,
// ScreenSaverView animateOneFrame), so the driver is step-based rather than a
// blocking loop: feed it events, then call step() once per frame. The harness
// wraps step() in a simple while-loop. Keeping this in Core means the hosts are
// thin and the frame logic is tested once.
#pragma once

#include "afterdark/afterdark.h"
#include "afterdark/backend.h"

namespace ad {

class HostSession {
 public:
  // Does not take ownership of the module; caller keeps it alive. Settings are
  // applied before init() so the module sees its config from the first frame.
  HostSession(Module& mod, int width, int height, bool interactive,
              const Settings& settings = {})
      : mod_(mod) {
    ctx_.screen_w = width;
    ctx_.screen_h = height;
    ctx_.interactive = interactive;
    // Provided settings win; schema defaults fill any gaps.
    ctx_.settings = settings;
    for (const auto& d : mod_.settings_schema())
      if (!d.default_value.empty() && !ctx_.settings.has(d.key))
        ctx_.settings.set(d.key, d.default_value);
    mod_.init(ctx_);
  }

  // Forward an input event to the module. Returns true if the module consumed
  // it (a native host uses the negative case to dismiss the saver).
  bool handle(const Event& e) { return mod_.on_event(ctx_, e); }

  // Advance simulation by dt and render one frame to the backend.
  void step(Backend& backend, double dt) {
    ctx_.time += dt;
    mod_.tick(ctx_, dt);
    Canvas& c = backend.begin_frame();
    mod_.draw(c);
    backend.end_frame();
  }

  Context& context() { return ctx_; }

 private:
  Module& mod_;
  Context ctx_;
};

}  // namespace ad
