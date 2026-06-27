// Image backend — rasterizes frames into an RGB buffer and saves them as PPM
// files. Dependency-free; used to render visual previews of modules (which are
// then converted to PNG and assembled into a gallery by tools/make_gallery.py).
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

#include "afterdark/backend.h"

namespace ad {
namespace {

class ImageCanvas : public Canvas {
 public:
  void resize(int w, int h) {
    w_ = w;
    h_ = h;
    buf_.assign(static_cast<size_t>(w) * h * 3, 0);
  }
  int width() const override { return w_; }
  int height() const override { return h_; }

  void clear(Color c) override {
    for (int i = 0; i < w_ * h_; ++i) {
      buf_[i * 3 + 0] = c.r;
      buf_[i * 3 + 1] = c.g;
      buf_[i * 3 + 2] = c.b;
    }
  }

  void fill_rect(int x, int y, int w, int h, Color c) override {
    int x0 = x < 0 ? 0 : x, y0 = y < 0 ? 0 : y;
    int x1 = x + w > w_ ? w_ : x + w, y1 = y + h > h_ ? h_ : y + h;
    for (int yy = y0; yy < y1; ++yy) {
      uint8_t* row = &buf_[(static_cast<size_t>(yy) * w_ + x0) * 3];
      for (int xx = x0; xx < x1; ++xx) {
        *row++ = c.r;
        *row++ = c.g;
        *row++ = c.b;
      }
    }
  }

  const std::vector<uint8_t>& pixels() const { return buf_; }

 private:
  int w_ = 0, h_ = 0;
  std::vector<uint8_t> buf_;
};

class ImageBackend : public Backend {
 public:
  ImageBackend(std::string prefix, int stride)
      : prefix_(std::move(prefix)), stride_(stride < 1 ? 1 : stride) {}

  bool init(int w, int h, const std::string&) override {
    canvas_.resize(w, h);
    return true;
  }
  bool poll_event(Event&) override { return false; }
  Canvas& begin_frame() override { return canvas_; }

  void end_frame() override {
    if (frame_ % stride_ == 0) save(frame_ / stride_);
    ++frame_;
  }
  bool should_close() const override { return false; }

 private:
  void save(int index) {
    char name[512];
    std::snprintf(name, sizeof(name), "%s_%04d.ppm", prefix_.c_str(), index);
    std::ofstream f(name, std::ios::binary);
    if (!f) return;
    f << "P6\n" << canvas_.width() << " " << canvas_.height() << "\n255\n";
    const auto& px = canvas_.pixels();
    f.write(reinterpret_cast<const char*>(px.data()),
            static_cast<std::streamsize>(px.size()));
  }

  std::string prefix_;
  int stride_;
  int frame_ = 0;
  ImageCanvas canvas_;
};

}  // namespace

std::unique_ptr<Backend> make_image_backend(const std::string& prefix,
                                            int stride) {
  return std::make_unique<ImageBackend>(prefix, stride);
}

}  // namespace ad
