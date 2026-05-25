#pragma once

#include "mod/trigger/TriggerIndex.h"

#include <algorithm>

namespace advancements::criteria {

inline bool hasOnlyKeys(nlohmann::json const& conditions, std::initializer_list<char const*> keys) {
    for (auto const& [key, _] : conditions.items()) {
        if (std::ranges::find(keys, key) == keys.end()) {
            return false;
        }
    }
    return true;
}

template <typename T>
T const* payloadAs(TriggerContext const& context) {
    return std::get_if<T>(&context.payload);
}

} // namespace advancements::criteria
