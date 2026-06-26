#include "modules.h"

#include "_template/template.h"
#include "flying_toasters/flying_toasters.h"
#include "paperboy/paperboy.h"
#include "starry_night/starry_night.h"

namespace ad {

void register_all_modules(Registry& reg) {
  // The special first-release demo.
  reg.add("paperboy", &make_paperboy);
  // Flagship recreations (see PLAN.md §3).
  reg.add("flying-toasters", &make_flying_toasters);
  reg.add("starry-night", &make_starry_night);
  // The copy-me template (also a working ambient starfield).
  reg.add("starfield", &make_template_starfield);

  // More flagship + classic recreations land here as they're built:
  //   reg.add("fish",        &make_fish);
  //   reg.add("mowing-man",  &make_mowing_man);
  //   ...
}

}  // namespace ad
