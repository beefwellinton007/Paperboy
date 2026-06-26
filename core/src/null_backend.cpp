// Headless backend implementation. Used by CI smoke tests and anywhere there
// is no display (like the build sandbox). Counts draw calls instead of drawing.
#include "afterdark/backend.h"

namespace ad {
namespace {

class NullCanvas : public Canvas {
 public:
  NullCanvas(int w, int h) : w_(w), h_(h) {}
  int width() const override { return w_; }
  int height() const override { return h_; }
  void clear(Color) override { ++clears; }
  void fill_rect(int, int, int, int, Color) override { ++rects; }

  long clears = 0;
  long rects = 0;

 private:
  int w_, h_;
};

class NullBackend : public Backend {
 public:
  bool init(int w, int h, const std::string&) override {
    canvas_ = std::make_unique<NullCanvas>(w, h);
    return true;
  }
  bool poll_event(Event&) override { return false; }  // no input headless
  Canvas& begin_frame() override { return *canvas_; }
  void end_frame() override {}
  bool should_close() const override { return false; }

  long rect_count() const { return canvas_ ? canvas_->rects : 0; }

 private:
  std::unique_ptr<NullCanvas> canvas_;
};

}  // namespace

std::unique_ptr<Backend> make_null_backend() {
  return std::make_unique<NullBackend>();
}

long null_backend_rect_count(const Backend& b) {
  const auto* nb = dynamic_cast<const NullBackend*>(&b);
  return nb ? nb->rect_count() : 0;
}

std::unique_ptr<Backend> make_default_backend(bool force_headless) {
#ifdef AD_HAVE_SDL2
  if (!force_headless) return make_sdl_backend();
#else
  (void)force_headless;
#endif
  return make_null_backend();
}

}  // namespace ad
