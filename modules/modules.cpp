#include "modules.h"

#include "_template/template.h"
#include "fish/fish.h"
#include "flying_toasters/flying_toasters.h"
#include "mowing_man/mowing_man.h"
#include "paperboy/paperboy.h"
#include "spotlight/spotlight.h"
#include "starry_night/starry_night.h"
#include "warp/warp.h"

namespace ad {

void register_all_modules(Registry& reg) {
  // The special first-release demo.
  reg.add("paperboy", &make_paperboy);
  // Flagship recreations (see PLAN.md §3).
  reg.add("flying-toasters", &make_flying_toasters);
  reg.add("starry-night", &make_starry_night);
  reg.add("fish", &make_fish);
  reg.add("mowing-man", &make_mowing_man);
  reg.add("warp", &make_warp);
  reg.add("spotlight", &make_spotlight);
  // The copy-me template (also a working ambient starfield).
  reg.add("starfield", &make_template_starfield);

  // More classic recreations land here as they're built:
  //   reg.add("globe", &make_globe);
  //   reg.add("boris", &make_boris);
  //   ...
}

}  // namespace ad
