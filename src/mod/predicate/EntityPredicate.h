#pragma once

#include "mod/predicate/DistancePredicate.h"

#include <nlohmann/json.hpp>

#include <optional>
#include <string>

namespace advancements::predicate {

struct EntityPredicate {
    std::optional<std::string>                 typeId;
    std::optional<HorizontalDistancePredicate> horizontalDistance;
    std::optional<std::string>                 frogVariantId;
};

std::optional<nlohmann::json const*> parseSingleThisEntityPredicateRoot(nlohmann::json const& conditions, char const* rootKey);
std::optional<std::string> parseEntityTypePredicate(nlohmann::json const& predicate);
std::optional<std::string> parseFrogVariantPredicate(nlohmann::json const& predicate);
std::optional<EntityPredicate> parseSingleThisEntityDistancePredicate(nlohmann::json const& conditions, char const* rootKey);

} // namespace advancements::predicate
