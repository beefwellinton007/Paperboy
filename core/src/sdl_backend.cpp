// SDL2 backend — the real windowed renderer used on developer machines and as
// the rendering core inside the native hosts. Compiled only when SDL2 is found
// (guarded by AD_HAVE_SDL2 from CMake).
#ifdef AD_HAVE_SDL2

#include <SDL.h>

#include <unordered_map>

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

  // GPU-textured image blit: one RenderCopyEx per sprite. Textures are cached
  // by the source pixel pointer (sprites are stable for the program lifetime).
  void draw_rgba(const void* key, const unsigned char* rgba, int sw, int sh,
                 int dx, int dy, int dw, int dh, bool flip_x,
                 unsigned char alpha) override {
    SDL_Texture* tex = nullptr;
    auto it = cache_.find(key);
    if (it != cache_.end()) {
      tex = it->second;
    } else {
      tex = SDL_CreateTexture(r_, SDL_PIXELFORMAT_ABGR8888,
                              SDL_TEXTUREACCESS_STATIC, sw, sh);
      if (tex) {
        SDL_UpdateTexture(tex, nullptr, rgba, sw * 4);
        SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
      }
      cache_[key] = tex;
    }
    if (!tex) return;
    SDL_SetTextureAlphaMod(tex, alpha);
    SDL_Rect dst{dx, dy, dw, dh};
    SDL_RenderCopyEx(r_, tex, nullptr, &dst, 0, nullptr,
                     flip_x ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
  }

  void resize(int w, int h) { w_ = w; h_ = h; }
  void destroy_textures() {
    for (auto& kv : cache_)
      if (kv.second) SDL_DestroyTexture(kv.second);
    cache_.clear();
  }

 private:
  SDL_Renderer* r_;
  int w_, h_;
  std::unordered_map<const void*, SDL_Texture*> cache_;
};

enum class Mode { Windowed, Fullscreen, Embedded };

class SdlBackend : public Backend {
 public:
  explicit SdlBackend(Mode mode = Mode::Windowed, void* embed = nullptr)
      : mode_(mode), embed_(embed) {}

  bool init(int w, int h, const std::string& title) override {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) return false;
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");  // smooth texture scaling
    if (mode_ == Mode::Embedded && embed_) {
      win_ = SDL_CreateWindowFrom(embed_);  // render into a host-owned window
    } else {
      Uint32 flags = SDL_WINDOW_ALLOW_HIGHDPI;
      flags |= (mode_ == Mode::Fullscreen) ? SDL_WINDOW_FULLSCREEN_DESKTOP
                                           : SDL_WINDOW_RESIZABLE;
      win_ = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED, w, h, flags);
    }
    if (!win_) return false;
    ren_ = SDL_CreateRenderer(win_, -1, SDL_RENDERER_ACCELERATED |
                                            SDL_RENDERER_PRESENTVSYNC);
    if (!ren_) return false;
    SDL_SetRenderDrawBlendMode(ren_, SDL_BLENDMODE_BLEND);  // honor Color.a
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
    if (canvas_) canvas_->destroy_textures();
    if (ren_) SDL_DestroyRenderer(ren_);
    if (win_) SDL_DestroyWindow(win_);
    SDL_Quit();
  }

 private:
  Mode mode_;
  void* embed_;
  SDL_Window* win_ = nullptr;
  SDL_Renderer* ren_ = nullptr;
  std::unique_ptr<SdlCanvas> canvas_;
};

}  // namespace

std::unique_ptr<Backend> make_sdl_backend() {
  return std::make_unique<SdlBackend>(Mode::Windowed);
}

std::unique_ptr<Backend> make_sdl_backend_fullscreen() {
  return std::make_unique<SdlBackend>(Mode::Fullscreen);
}

std::unique_ptr<Backend> make_sdl_backend_embedded(void* native_window) {
  return std::make_unique<SdlBackend>(Mode::Embedded, native_window);
}

}  // namespace ad

#endif  // AD_HAVE_SDL2
