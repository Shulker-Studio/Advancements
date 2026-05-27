#pragma once

#include <nlohmann/json.hpp>

#include <algorithm>

namespace advancements::predicate {

inline bool hasOnlyKeys(nlohmann::json const& object, std::initializer_list<char const*> keys) {
    for (auto const& [key, _] : object.items()) {
        if (std::ranges::find(keys, key) == keys.end()) {
            return false;
        }
    }
    return true;
}

} // namespace advancements::predicate
