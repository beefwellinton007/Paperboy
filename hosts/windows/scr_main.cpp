// Windows screensaver host (.scr) — STUB.
//
// A .scr is an .exe that honors /s (run), /p <HWND> (preview), /c (config).
// This stub sketches the entry point; the real implementation embeds AfterDark
// Core and renders into the screensaver/preview HWND via the backend. Built on
// a Windows runner (needs the Windows SDK), not by the default CMake target.
//
// #include <windows.h>
// #include "afterdark/backend.h"
// #include "afterdark/registry.h"
// #include "modules.h"

enum class Mode { Run, Preview, Config };

// int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR cmdline, int) {
//   Mode mode = parse_mode(cmdline);          // /s, /p <hwnd>, /c
//   switch (mode) {
//     case Mode::Config:  show_config_dialog(); return 0;
//     case Mode::Preview: run_in_hwnd(parse_hwnd(cmdline), /*preview=*/true); return 0;
//     case Mode::Run:     run_fullscreen(); return 0;
//   }
// }

int main() { return 0; }  // placeholder so the file is syntactically valid
