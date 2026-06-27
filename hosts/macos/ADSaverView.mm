// macOS screensaver host (.saver) — Objective-C++.
//
// A .saver is a bundle whose principal class subclasses ScreenSaverView. macOS
// gives us a CGContext in drawRect:, so the proper native path is a Core
// Graphics-backed Canvas (no SDL needed on Mac). We embed AfterDark Core: a
// CGBackend implements ad::Backend over the current CGContext, and the shared
// HostSession ticks + draws each frame.
//
// Builds with Xcode/clang on a macOS runner (see CMakeLists.txt here); not part
// of the default Linux build. Config sheet + persistence are the remaining TODOs.
#ifdef __APPLE__

#import <Cocoa/Cocoa.h>
#import <ScreenSaver/ScreenSaver.h>

#include <memory>

#include "afterdark/backend.h"
#include "afterdark/host.h"
#include "afterdark/registry.h"
#include "modules.h"

namespace {

// Canvas drawing into whatever CGContext is current for this frame.
class CGCanvas : public ad::Canvas {
 public:
  CGCanvas(int w, int h) : w_(w), h_(h) {}
  void set_context(CGContextRef ctx) { ctx_ = ctx; }
  int width() const override { return w_; }
  int height() const override { return h_; }
  void clear(ad::Color c) override { fill_rect(0, 0, w_, h_, c); }
  void fill_rect(int x, int y, int w, int h, ad::Color c) override {
    if (!ctx_) return;
    CGContextSetRGBFillColor(ctx_, c.r / 255.0, c.g / 255.0, c.b / 255.0,
                             c.a / 255.0);
    // Flip Y so module coordinates have the origin at the top-left.
    CGContextFillRect(ctx_, CGRectMake(x, h_ - y - h, w, h));
  }

 private:
  int w_, h_;
  CGContextRef ctx_ = nullptr;
};

class CGBackend : public ad::Backend {
 public:
  CGBackend(int w, int h) : canvas_(w, h) {}
  bool init(int, int, const std::string&) override { return true; }
  bool poll_event(ad::Event&) override { return false; }  // Cocoa handles input
  ad::Canvas& begin_frame() override { return canvas_; }
  void end_frame() override {}
  bool should_close() const override { return false; }
  void set_context(CGContextRef ctx) { canvas_.set_context(ctx); }

 private:
  CGCanvas canvas_;
};

}  // namespace

@interface ADSaverView : ScreenSaverView {
  std::unique_ptr<ad::Module> _module;
  std::unique_ptr<CGBackend> _backend;
  std::unique_ptr<ad::HostSession> _session;
}
@end

@implementation ADSaverView

- (instancetype)initWithFrame:(NSRect)frame isPreview:(BOOL)isPreview {
  self = [super initWithFrame:frame isPreview:isPreview];
  if (self) {
    int w = (int)frame.size.width, h = (int)frame.size.height;
    ad::Registry reg;
    ad::register_all_modules(reg);
    // TODO: read selected module + per-module settings from ScreenSaverDefaults
    // (written by the configure sheet); default to paperboy with built-ins.
    _module = reg.create("paperboy");
    ad::Settings settings;  // TODO: populate from ScreenSaverDefaults
    _backend = std::make_unique<CGBackend>(w, h);
    _backend->init(w, h, "AfterDark");
    _session = std::make_unique<ad::HostSession>(*_module, w, h,
                                                 /*interactive=*/false, settings);
    [self setAnimationTimeInterval:1.0 / 60.0];
  }
  return self;
}

- (void)drawRect:(NSRect)rect {
  [super drawRect:rect];
  if (!_session) return;
  CGContextRef ctx = [[NSGraphicsContext currentContext] CGContext];
  _backend->set_context(ctx);
  _session->step(*_backend, [self animationTimeInterval]);
}

- (void)animateOneFrame {
  [self setNeedsDisplay:YES];  // step happens in drawRect with a valid context
}

- (BOOL)hasConfigureSheet {
  return NO;  // TODO: manifest-driven settings sheet
}

- (NSWindow *)configureSheet {
  return nil;
}

@end

#endif  // __APPLE__
