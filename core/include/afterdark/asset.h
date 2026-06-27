// Asset loading. Sprites are shipped as ".adspr" files — a trivial raw-RGBA
// format produced from PNGs by tools/pack_assets.py. This keeps the runtime
// dependency-free (no PNG/zlib decoder needed in the engine or the native
// hosts); the conversion happens once at build/pack time in Python.
//
// .adspr layout: magic "ADSP" (4 bytes), width u16-LE, height u16-LE, then
// width*height*4 bytes of RGBA.
#pragma once

#include <string>

#include "afterdark/sprite.h"

namespace ad {

// Loads a .adspr file into a Sprite. Returns an empty Sprite (sprite.empty())
// on any error, so callers can fall back to procedural drawing.
Sprite load_adspr(const std::string& path);

// Loads a frame sequence: "<base>_1.adspr", "<base>_2.adspr", ... until one is
// missing. Also accepts a single "<base>.adspr" as a 1-frame animation. Returns
// an empty Animation if nothing loads. `fps` sets the playback rate.
Animation load_animation(const std::string& base, double fps = 8.0);

}  // namespace ad
