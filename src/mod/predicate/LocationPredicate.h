#pragma once

#include <nlohmann/json.hpp>

#include <optional>
#include <string>

class Player;

namespace advancements::predicate {

struct LocationPredicate {
    std::string structureId;
};

std::optional<LocationPredicate> parseLocationPredicate(nlohmann::json const& predicate);
std::optional<std::string>       currentSupportedLocationStructure(Player& player);
bool                             matchesLocationPredicate(LocationPredicate const& predicate, std::string const& structureId);

} // namespace advancements::predicate
