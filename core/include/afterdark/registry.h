// Module registry: maps stable ids -> factory functions. The host asks the
// registry to instantiate a module by id (from config or the scheduler).
#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "afterdark/afterdark.h"

namespace ad {

class Registry {
 public:
  using Factory = std::function<std::unique_ptr<Module>()>;

  void add(const std::string& id, Factory factory) {
    factories_[id] = std::move(factory);
  }

  std::unique_ptr<Module> create(const std::string& id) const {
    auto it = factories_.find(id);
    return it == factories_.end() ? nullptr : it->second();
  }

  bool has(const std::string& id) const {
    return factories_.find(id) != factories_.end();
  }

  std::vector<std::string> ids() const {
    std::vector<std::string> out;
    out.reserve(factories_.size());
    for (const auto& kv : factories_) out.push_back(kv.first);
    return out;
  }

 private:
  std::map<std::string, Factory> factories_;
};

}  // namespace ad
