#pragma once
#include <memory>
#include "afterdark/afterdark.h"

namespace ad {
// Factory for the Paperboy module (registered in modules.cpp).
std::unique_ptr<Module> make_paperboy();
}  // namespace ad
