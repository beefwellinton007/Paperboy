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
  int frames = 120;
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

  auto backend = ad::make_default_backend(args.headless);
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

    if (args.headless && ++frame >= args.frames) break;
    if (backend->should_close()) break;
  }

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
