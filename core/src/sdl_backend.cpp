// SDL2 backend — the real windowed renderer used on developer machines and as
// the rendering core inside the native hosts. Compiled only when SDL2 is found
// (guarded by AD_HAVE_SDL2 from CMake).
#ifdef AD_HAVE_SDL2

#include <SDL.h>

#include "afterdark/backend.h"

namespace ad {
namespace {

Key map_key(SDL_Keycode k) {
  switch (k) {
    case SDLK_LEFT: return Key::Left;
    case SDLK_RIGHT: return Key::Right;
    case SDLK_UP: return Key::Up;
    case SDLK_DOWN: return Key::Down;
    case SDLK_SPACE: return Key::Space;
    case SDLK_ESCAPE: return Key::Escape;
    default: return Key::Other;
  }
}

class SdlCanvas : public Canvas {
 public:
  SdlCanvas(SDL_Renderer* r, int w, int h) : r_(r), w_(w), h_(h) {}
  int width() const override { return w_; }
  int height() const override { return h_; }
  void clear(Color c) override {
    SDL_SetRenderDrawColor(r_, c.r, c.g, c.b, c.a);
    SDL_RenderClear(r_);
  }
  void fill_rect(int x, int y, int w, int h, Color c) override {
    SDL_SetRenderDrawColor(r_, c.r, c.g, c.b, c.a);
    SDL_Rect rect{x, y, w, h};
    SDL_RenderFillRect(r_, &rect);
  }
  void resize(int w, int h) { w_ = w; h_ = h; }

 private:
  SDL_Renderer* r_;
  int w_, h_;
};

class SdlBackend : public Backend {
 public:
  bool init(int w, int h, const std::string& title) override {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) return false;
    win_ = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED, w, h,
                            SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    if (!win_) return false;
    ren_ = SDL_CreateRenderer(win_, -1, SDL_RENDERER_ACCELERATED |
                                            SDL_RENDERER_PRESENTVSYNC);
    if (!ren_) return false;
    canvas_ = std::make_unique<SdlCanvas>(ren_, w, h);
    return true;
  }

  bool poll_event(Event& out) override {
    SDL_Event e;
    if (!SDL_PollEvent(&e)) return false;
    switch (e.type) {
      case SDL_QUIT:
        out.type = EventType::Quit;
        return true;
      case SDL_KEYDOWN:
        out.type = EventType::KeyDown;
        out.key = map_key(e.key.keysym.sym);
        return true;
      case SDL_KEYUP:
        out.type = EventType::KeyUp;
        out.key = map_key(e.key.keysym.sym);
        return true;
      case SDL_MOUSEMOTION:
        out.type = EventType::PointerMove;
        out.x = e.motion.x;
        out.y = e.motion.y;
        return true;
      case SDL_MOUSEBUTTONDOWN:
        out.type = EventType::PointerDown;
        out.x = e.button.x;
        out.y = e.button.y;
        return true;
      default:
        out.type = EventType::None;
        return true;  // consumed an event we don't care about; keep draining
    }
  }

  Canvas& begin_frame() override {
    int w, h;
    SDL_GetRendererOutputSize(ren_, &w, &h);
    canvas_->resize(w, h);
    return *canvas_;
  }
  void end_frame() override { SDL_RenderPresent(ren_); }
  bool should_close() const override { return false; }

  void shutdown() override {
    if (ren_) SDL_DestroyRenderer(ren_);
    if (win_) SDL_DestroyWindow(win_);
    SDL_Quit();
  }

 private:
  SDL_Window* win_ = nullptr;
  SDL_Renderer* ren_ = nullptr;
  std::unique_ptr<SdlCanvas> canvas_;
};

}  // namespace

std::unique_ptr<Backend> make_sdl_backend() {
  return std::make_unique<SdlBackend>();
}

}  // namespace ad

#endif  // AD_HAVE_SDL2
