#pragma once

#include <nlohmann/json.hpp>

#include <optional>

namespace advancements::predicate {

struct DamagePredicate {
    bool requireBlockedDamage;
    bool requireProjectileDamageTag;
};

std::optional<DamagePredicate> parseBlockedProjectileDamagePredicate(nlohmann::json const& conditions);

} // namespace advancements::predicate
