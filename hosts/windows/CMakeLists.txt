# Windows screensaver host. Builds only on Windows (added conditionally from the
# top-level CMakeLists). Produces an .exe that is renamed to AfterDark.scr.
if(NOT WIN32)
  return()
endif()

find_package(SDL2 QUIET)
if(NOT SDL2_FOUND)
  message(WARNING "AfterDark: SDL2 not found; skipping Windows .scr host.")
  return()
endif()

add_executable(AfterDarkScr WIN32 scr_main.cpp)
target_link_libraries(AfterDarkScr PRIVATE ad_core ad_modules SDL2::SDL2 SDL2::SDL2main)

# A .scr is just an .exe with the .scr extension.
set_target_properties(AfterDarkScr PROPERTIES OUTPUT_NAME "AfterDark" SUFFIX ".scr")
