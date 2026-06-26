// macOS screensaver host (.saver) — STUB (Objective-C++).
//
// Subclasses ScreenSaverView and drives AfterDark Core each frame. The real
// implementation binds the backend to the view's layer (Metal or SDL2) and
// loads modules from ScreenSaverDefaults. Built with Xcode on a macOS runner,
// not by the default CMake target.
//
// #import <ScreenSaver/ScreenSaver.h>
// #include "afterdark/backend.h"
// #include "afterdark/registry.h"
// #include "modules.h"
//
// @interface ADSaverView : ScreenSaverView @end
//
// @implementation ADSaverView
// - (instancetype)initWithFrame:(NSRect)frame isPreview:(BOOL)isPreview {
//   self = [super initWithFrame:frame isPreview:isPreview];
//   // create backend bound to self.layer; register_all_modules; module->init
//   [self setAnimationTimeInterval:1.0/60.0];
//   return self;
// }
// - (void)animateOneFrame { /* ctx.time += dt; module->tick; module->draw */ }
// - (BOOL)hasConfigureSheet { return YES; }
// - (NSWindow*)configureSheet { /* manifest-driven settings UI */ return nil; }
// @end
