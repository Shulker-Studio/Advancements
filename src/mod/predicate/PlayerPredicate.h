#pragma once

#include "mod/predicate/LocationPredicate.h"

#include <nlohmann/json.hpp>

#include <optional>

namespace advancements::predicate {

struct PlayerPredicate {
    LocationPredicate location;
};

std::optional<nlohmann::json const*> parseSinglePlayerThisEntityPredicateRoot(nlohmann::json const& conditions);
std::optional<PlayerPredicate> parseSinglePlayerLocationPredicate(nlohmann::json const& conditions);

} // namespace advancements::predicate
