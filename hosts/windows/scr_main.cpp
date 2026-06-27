// Windows screensaver host (.scr).
//
// A .scr is an .exe that honors /s (run full-screen), /p <HWND> (render into the
// Screen Saver dialog's preview pane), and /c (configuration). This host embeds
// AfterDark Core: it builds an SDL backend (fullscreen, or wrapping the preview
// HWND) and drives the chosen module with the shared HostSession.
//
// Builds on a Windows runner with SDL2 (see CMakeLists.txt in this folder); not
// part of the default Linux build. Config persistence + a real settings dialog
// are the remaining TODOs (marked below).
#ifdef _WIN32

#include <windows.h>

#include <cstdlib>
#include <string>

#include "afterdark/backend.h"
#include "afterdark/host.h"
#include "afterdark/registry.h"
#include "modules.h"

namespace {

enum class Mode { Run, Preview, Config };

struct Parsed {
  Mode mode = Mode::Run;
  HWND preview = nullptr;
};

// Command lines look like: "/s", "/p 12345", "/c", "/c:12345".
Parsed parse(LPSTR cmdline) {
  Parsed p;
  std::string s = cmdline ? cmdline : "";
  for (auto& ch : s) ch = static_cast<char>(::tolower(ch));
  auto num_after = [&](size_t pos) -> HWND {
    while (pos < s.size() && (s[pos] == ' ' || s[pos] == ':')) ++pos;
    if (pos >= s.size()) return nullptr;
    return reinterpret_cast<HWND>(static_cast<uintptr_t>(
        strtoull(s.c_str() + pos, nullptr, 10)));
  };
  if (s.find("/p") != std::string::npos) {
    p.mode = Mode::Preview;
    p.preview = num_after(s.find("/p") + 2);
  } else if (s.find("/c") != std::string::npos) {
    p.mode = Mode::Config;
  } else {
    p.mode = Mode::Run;  // "/s" or no args
  }
  return p;
}

// Which module to run. TODO: read from persisted config (registry/INI). For now
// default to the debut module so the installer demonstrates Paperboy.
std::string selected_module() { return "paperboy"; }

int run_loop(std::unique_ptr<ad::Backend> backend, bool preview) {
  ad::Registry reg;
  ad::register_all_modules(reg);
  auto mod = reg.create(selected_module());
  if (!mod) return 1;

  // Sized later by the backend; start with sensible defaults.
  if (!backend->init(800, 600, "AfterDark")) return 2;
  ad::HostSession session(*mod, 800, 600, /*interactive=*/false);

  const double dt = 1.0 / 60.0;
  POINT origin{};
  GetCursorPos(&origin);
  for (;;) {
    ad::Event e;
    while (backend->poll_event(e)) {
      // Real input dismisses the saver (except in preview).
      if (!preview && (e.type == ad::EventType::KeyDown ||
                       e.type == ad::EventType::PointerDown)) {
        return 0;
      }
      session.handle(e);
    }
    if (!preview) {
      POINT now{};
      GetCursorPos(&now);
      if (abs(now.x - origin.x) > 8 || abs(now.y - origin.y) > 8) return 0;
    }
    session.step(*backend, dt);
    Sleep(15);
  }
}

}  // namespace

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR cmdline, int) {
  Parsed p = parse(cmdline);
  switch (p.mode) {
    case Mode::Config:
      // TODO: manifest-driven settings dialog (sdk/schema/module.schema.json).
      MessageBoxA(nullptr, "AfterDark settings coming soon.", "AfterDark",
                  MB_OK);
      return 0;
    case Mode::Preview:
      if (!p.preview) return 0;
      return run_loop(ad::make_sdl_backend_embedded(p.preview), true);
    case Mode::Run:
    default:
      return run_loop(ad::make_sdl_backend_fullscreen(), false);
  }
}

#endif  // _WIN32
