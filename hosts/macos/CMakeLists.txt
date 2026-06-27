# macOS screensaver host. Builds only on macOS (added conditionally from the
# top-level CMakeLists). Produces an AfterDark.saver bundle.
if(NOT APPLE)
  return()
endif()

add_library(AfterDarkSaver MODULE ADSaverView.mm)
set_target_properties(AfterDarkSaver PROPERTIES
  BUNDLE TRUE
  BUNDLE_EXTENSION "saver"
  OUTPUT_NAME "AfterDark"
  MACOSX_BUNDLE_GUI_IDENTIFIER "com.afterdark.recreation"
  XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC YES
)
target_link_libraries(AfterDarkSaver PRIVATE ad_core ad_modules
  "-framework Cocoa" "-framework ScreenSaver" "-framework CoreGraphics")

# NOTE: the .saver Info.plist must set NSPrincipalClass = ADSaverView. Add a
# configured Info.plist on the macOS runner before packaging.
