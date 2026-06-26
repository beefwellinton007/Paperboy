#pragma once
#include "afterdark/registry.h"

namespace ad {
// Registers every first-party module into the registry. The host calls this
// once at startup. Add new modules here.
void register_all_modules(Registry& reg);
}  // namespace ad
