#pragma once

#include <nlohmann/json.hpp>

#include <optional>
#include <string>

namespace advancements::predicate {

struct DamagePredicate {
    bool requireBlockedDamage;
    bool requireProjectileDamageTag;
    bool requireMaceSmashDamageTag;
    std::optional<std::string> directEntityTypeId;
    std::optional<std::string> directEntityMainhandItemId;
    std::optional<float>       dealtMin;
};

std::optional<DamagePredicate> parseBlockedProjectileDamagePredicate(nlohmann::json const& conditions);
std::optional<DamagePredicate> parsePlayerHurtEntityDamagePredicate(nlohmann::json const& conditions);

} // namespace advancements::predicate
