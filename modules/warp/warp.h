#pragma once
#include <memory>
#include "afterdark/afterdark.h"

namespace ad {
std::unique_ptr<Module> make_warp();
}  // namespace ad
