// ad-harness — standalone dev runner for modules.
//
// This is where module development happens: load any module instantly, no
// screensaver install required. It also doubles as the CI smoke tester via
// --headless, which runs a module for N frames with the null backend and
// asserts it rendered without crashing.
//
// Usage:
//   ad-harness --list
//   ad-harness --module paperboy [--width 1280 --height 800] [--play]
//   ad-harness --module paperboy --headless [--frames 120]
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include "afterdark/asset.h"
#include "afterdark/backend.h"
#include "afterdark/host.h"
#include "afterdark/registry.h"
#include "modules.h"

namespace {

struct Args {
  std::string module = "paperboy";
  int width = 1280;
  int height = 800;
  bool headless = false;
  bool play = false;
  bool list = false;
  bool config = false;
  bool fullscreen = false;
  int frames = 120;
  std::string render;  // PPM output prefix; empty = no rendering
  int stride = 10;     // save every Nth frame when rendering
  ad::Settings settings;
};

Args parse(int argc, char** argv) {
  Args a;
  for (int i = 1; i < argc; ++i) {
    std::string s = argv[i];
    auto next = [&]() { return (i + 1 < argc) ? argv[++i] : ""; };
    if (s == "--module" || s == "-m") a.module = next();
    else if (s == "--width") a.width = std::atoi(next());
    else if (s == "--height") a.height = std::atoi(next());
    else if (s == "--frames") a.frames = std::atoi(next());
    else if (s == "--headless") a.headless = true;
    else if (s == "--play") a.play = true;
    else if (s == "--list" || s == "-l") a.list = true;
    else if (s == "--config") a.config = true;
    else if (s == "--fullscreen" || s == "-f") a.fullscreen = true;
    else if (s == "--render") a.render = next();
    else if (s == "--stride") a.stride = std::atoi(next());
    else if (s == "--set") {  // --set key=value (repeatable)
      std::string kv = next();
      auto eq = kv.find('=');
      if (eq != std::string::npos)
        a.settings.set(kv.substr(0, eq), kv.substr(eq + 1));
    }
  }
  return a;
}

}  // namespace

int main(int argc, char** argv) {
  Args args = parse(argc, argv);

  // Asset-pipeline probe: --probe-sprite <path> loads an .adspr and reports it.
  for (int i = 1; i < argc; ++i) {
    if (std::string(argv[i]) == "--probe-sprite" && i + 1 < argc) {
      ad::Sprite s = ad::load_adspr(argv[i + 1]);
      if (s.empty()) { std::fprintf(stderr, "probe: load failed\n"); return 1; }
      ad::Color p = s.at(s.width() / 2, s.height() / 2);
      std::printf("probe ok: %dx%d  center rgba=%d,%d,%d,%d\n", s.width(),
                  s.height(), p.r, p.g, p.b, p.a);
      return 0;
    }
  }

  ad::Registry reg;
  ad::register_all_modules(reg);

  if (args.list) {
    std::printf("Available modules:\n");
    for (const auto& id : reg.ids()) std::printf("  %s\n", id.c_str());
    return 0;
  }

  auto mod = reg.create(args.module);
  if (!mod) {
    std::fprintf(stderr, "error: unknown module '%s' (try --list)\n",
                 args.module.c_str());
    return 2;
  }

  if (args.config) {  // dump the module's declared config schema
    static const char* kType[] = {"bool", "int", "float", "enum", "color", "string"};
    auto schema = mod->settings_schema();
    std::printf("Config schema for '%s' (%zu settings):\n", args.module.c_str(),
                schema.size());
    for (const auto& d : schema) {
      std::printf("  %-14s %-7s default=%-8s", d.key.c_str(),
                  kType[static_cast<int>(d.type)], d.default_value.c_str());
      if (d.type == ad::SettingType::Enum) {
        std::printf(" options=[");
        for (size_t i = 0; i < d.options.size(); ++i)
          std::printf("%s%s", i ? "," : "", d.options[i].c_str());
        std::printf("]");
      } else if (d.type == ad::SettingType::Int ||
                 d.type == ad::SettingType::Float) {
        std::printf(" range=[%g,%g]", d.min, d.max);
      }
      std::printf("  (%s)\n", d.label.c_str());
    }
    return 0;
  }

  std::unique_ptr<ad::Backend> backend;
  if (!args.render.empty())
    backend = ad::make_image_backend(args.render, args.stride);
#ifdef AD_HAVE_SDL2
  else if (args.fullscreen && !args.headless)
    backend = ad::make_sdl_backend_fullscreen();
#endif
  else
    backend = ad::make_default_backend(args.headless);
  const bool finite = args.headless || !args.render.empty();
  if (!backend->init(args.width, args.height, "AfterDark — " + args.module)) {
    std::fprintf(stderr, "error: backend init failed\n");
    return 3;
  }

  ad::HostSession session(*mod, args.width, args.height, args.play,
                          args.settings);

  auto info = mod->info();
  std::printf("Running '%s' v%s (%s)%s\n", info.name.c_str(),
              info.version.c_str(),
              info.category == ad::Category::Interactive ? "interactive" : "ambient",
              args.headless ? " [headless]" : "");

  const double dt = 1.0 / 60.0;  // fixed timestep
  int frame = 0;
  bool running = true;
  while (running) {
    ad::Event e;
    while (backend->poll_event(e)) {
      if (e.type == ad::EventType::Quit ||
          (e.type == ad::EventType::KeyDown && e.key == ad::Key::Escape)) {
        running = false;
        break;
      }
      session.handle(e);
    }
    session.step(*backend, dt);

    if (finite && ++frame >= args.frames) break;
    if (backend->should_close()) break;
  }

  if (!args.render.empty())
    std::printf("Rendered '%s': %d frames -> %s_*.ppm (stride %d)\n",
                args.module.c_str(), frame, args.render.c_str(), args.stride);

  long rects = ad::null_backend_rect_count(*backend);
  if (args.headless) {
    // Smoke assertion: a module that ran for N frames must have drawn.
    if (rects <= 0) {
      std::fprintf(stderr, "SMOKE FAIL: '%s' drew nothing in %d frames\n",
                   args.module.c_str(), args.frames);
      backend->shutdown();
      return 1;
    }
    std::printf("SMOKE OK: '%s' ran %d frames, %ld rect draws\n",
                args.module.c_str(), args.frames, rects);
  }

  backend->shutdown();
  return 0;
}
