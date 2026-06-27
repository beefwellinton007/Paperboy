cmake_minimum_required(VERSION 3.16)
project(AfterDarkRecreation VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

if(MSVC)
  add_compile_options(/W4)
else()
  add_compile_options(-Wall -Wextra)
endif()

# --- Module SDK (header-only interface) -----------------------------------
add_library(ad_sdk INTERFACE)
target_include_directories(ad_sdk INTERFACE ${CMAKE_SOURCE_DIR}/sdk/include)

# --- Render/Platform Abstraction Layer detection --------------------------
find_package(SDL2 QUIET)
if(SDL2_FOUND)
  message(STATUS "AfterDark: SDL2 found — building windowed backend.")
else()
  message(STATUS "AfterDark: SDL2 not found — headless backend only "
                 "(fine for CI smoke tests).")
endif()

# --- Core engine ----------------------------------------------------------
set(CORE_SOURCES core/src/null_backend.cpp core/src/image_backend.cpp
                 core/src/asset.cpp)
if(SDL2_FOUND)
  list(APPEND CORE_SOURCES core/src/sdl_backend.cpp)
endif()

add_library(ad_core STATIC ${CORE_SOURCES})
target_include_directories(ad_core PUBLIC ${CMAKE_SOURCE_DIR}/core/include)
target_link_libraries(ad_core PUBLIC ad_sdk)
if(SDL2_FOUND)
  target_compile_definitions(ad_core PUBLIC AD_HAVE_SDL2)
  target_link_libraries(ad_core PUBLIC SDL2::SDL2)
endif()

# --- Modules --------------------------------------------------------------
add_library(ad_modules STATIC
  modules/modules.cpp
  modules/paperboy/paperboy.cpp
  modules/flying_toasters/flying_toasters.cpp
  modules/starry_night/starry_night.cpp
  modules/fish/fish.cpp
  modules/mowing_man/mowing_man.cpp
  modules/warp/warp.cpp
  modules/spotlight/spotlight.cpp
  modules/globe/globe.cpp
  modules/boris/boris.cpp
  modules/rat_race/rat_race.cpp
  modules/messages/messages.cpp
  modules/clocks/clocks.cpp
  modules/confetti/confetti.cpp
  modules/gravity/gravity.cpp
  modules/hard_rain/hard_rain.cpp
  modules/satori/satori.cpp
  modules/down_the_drain/down_the_drain.cpp
  modules/string_theory/string_theory.cpp
  modules/bad_dog/bad_dog.cpp
  modules/_template/template.cpp
)
target_include_directories(ad_modules PUBLIC ${CMAKE_SOURCE_DIR}/modules)
target_link_libraries(ad_modules PUBLIC ad_core)

# --- Dev harness / CI smoke runner ----------------------------------------
add_executable(ad-harness hosts/harness/main.cpp)
target_link_libraries(ad-harness PRIVATE ad_core ad_modules)

# --- Native hosts (.scr / .saver) -----------------------------------------
# Built only on their own platform; each subdir early-returns elsewhere, so the
# Linux/CI default build is unaffected.
if(WIN32)
  add_subdirectory(hosts/windows)
elseif(APPLE)
  add_subdirectory(hosts/macos)
endif()
