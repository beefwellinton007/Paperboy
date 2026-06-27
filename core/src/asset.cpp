#include "afterdark/asset.h"

#include <cstdint>
#include <fstream>
#include <vector>

namespace ad {

Sprite load_adspr(const std::string& path) {
  std::ifstream f(path, std::ios::binary);
  if (!f) return Sprite{};
  char magic[4];
  f.read(magic, 4);
  if (!f || magic[0] != 'A' || magic[1] != 'D' || magic[2] != 'S' ||
      magic[3] != 'P')
    return Sprite{};
  unsigned char dim[4];
  f.read(reinterpret_cast<char*>(dim), 4);
  if (!f) return Sprite{};
  int w = dim[0] | (dim[1] << 8);
  int h = dim[2] | (dim[3] << 8);
  if (w <= 0 || h <= 0 || w > 8192 || h > 8192) return Sprite{};
  std::vector<unsigned char> rgba(static_cast<size_t>(w) * h * 4);
  f.read(reinterpret_cast<char*>(rgba.data()),
         static_cast<std::streamsize>(rgba.size()));
  if (!f) return Sprite{};
  return Sprite::from_rgba(w, h, rgba.data());
}

}  // namespace ad
