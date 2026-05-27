#pragma once

#include <nlohmann/json.hpp>

#include <optional>

namespace advancements::predicate {

struct HorizontalDistancePredicate {
    float min;
};

std::optional<HorizontalDistancePredicate> parseHorizontalDistancePredicate(nlohmann::json const& predicate);
bool                                       matchesHorizontalDistancePredicate(HorizontalDistancePredicate const& predicate, float horizontalDistance);

} // namespace advancements::predicate
