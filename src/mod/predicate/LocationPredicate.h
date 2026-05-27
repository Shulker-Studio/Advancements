#pragma once

#include <nlohmann/json.hpp>

#include <optional>
#include <string>

class Player;

namespace advancements::predicate {

struct PositionRangePredicate {
    std::optional<float> yMin;
};

struct LocationPredicate {
    std::optional<std::string> structureId;
    std::optional<PositionRangePredicate> position;
};

std::optional<LocationPredicate> parseLocationPredicate(nlohmann::json const& predicate);
std::optional<std::string>       currentSupportedLocationStructure(Player& player);
bool                             matchesLocationPredicate(LocationPredicate const& predicate, std::string const& structureId);

} // namespace advancements::predicate
