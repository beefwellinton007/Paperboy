#include "modules.h"

#include "_template/template.h"
#include "paperboy/paperboy.h"

namespace ad {

void register_all_modules(Registry& reg) {
  // The special first-release demo.
  reg.add("paperboy", &make_paperboy);
  // The copy-me template (also a working ambient starfield).
  reg.add("starfield", &make_template_starfield);

  // Flagship + classic recreations land here as they're built (see PLAN.md §3):
  //   reg.add("flying-toasters", &make_flying_toasters);
  //   reg.add("starry-night",    &make_starry_night);
  //   ...
}

}  // namespace ad
