#include "modules.h"

#include "_template/template.h"
#include "boris/boris.h"
#include "clocks/clocks.h"
#include "confetti/confetti.h"
#include "fish/fish.h"
#include "gravity/gravity.h"
#include "flying_toasters/flying_toasters.h"
#include "globe/globe.h"
#include "messages/messages.h"
#include "mowing_man/mowing_man.h"
#include "paperboy/paperboy.h"
#include "rat_race/rat_race.h"
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
  reg.add("globe", &make_globe);
  reg.add("boris", &make_boris);
  reg.add("rat-race", &make_rat_race);
  reg.add("messages", &make_messages);
  reg.add("clocks", &make_clocks);
  reg.add("confetti", &make_confetti);
  reg.add("gravity", &make_gravity);
  // The copy-me template (also a working ambient starfield).
  reg.add("starfield", &make_template_starfield);

  // More classic recreations land here as they're built:
  //   reg.add("hard-rain", &make_hard_rain);
  //   reg.add("satori",    &make_satori);
  //   ...
}

}  // namespace ad
