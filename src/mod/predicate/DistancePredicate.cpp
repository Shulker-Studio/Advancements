#include "mod/predicate/DistancePredicate.h"

#include "mod/predicate/Common.h"

namespace advancements::predicate {

std::optional<HorizontalDistancePredicate> parseHorizontalDistancePredicate(nlohmann::json const& predicate) {
    if (!predicate.is_object() || !hasOnlyKeys(predicate, {"horizontal"})) {
        return std::nullopt;
    }
    if (!predicate.contains("horizontal") || !predicate.at("horizontal").is_object()) {
        return std::nullopt;
    }

    auto const& horizontal = predicate.at("horizontal");
    if (!hasOnlyKeys(horizontal, {"min"}) || !horizontal.contains("min") || !horizontal.at("min").is_number()) {
        return std::nullopt;
    }

    return HorizontalDistancePredicate{horizontal.at("min").get<float>()};
}

bool matchesHorizontalDistancePredicate(HorizontalDistancePredicate const& predicate, float horizontalDistance) {
    return horizontalDistance >= predicate.min;
}

} // namespace advancements::predicate
