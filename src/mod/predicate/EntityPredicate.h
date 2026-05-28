#pragma once

#include "mod/predicate/DistancePredicate.h"

#include <nlohmann/json.hpp>

#include <optional>

namespace advancements::predicate {

struct EntityPredicate {
    std::optional<HorizontalDistancePredicate> horizontalDistance;
};

std::optional<nlohmann::json const*> parseSingleThisEntityPredicateRoot(nlohmann::json const& conditions, char const* rootKey);
std::optional<EntityPredicate> parseSingleThisEntityDistancePredicate(nlohmann::json const& conditions, char const* rootKey);

} // namespace advancements::predicate
