#include "mod/predicate/PlayerPredicate.h"

#include "mod/predicate/Common.h"

namespace advancements::predicate {

std::optional<PlayerPredicate> parseSinglePlayerLocationPredicate(nlohmann::json const& conditions) {
    if (!hasOnlyKeys(conditions, {"player"})) {
        return std::nullopt;
    }
    if (!conditions.contains("player") || !conditions.at("player").is_array() || conditions.at("player").size() != 1) {
        return std::nullopt;
    }

    auto const& playerCondition = conditions.at("player").front();
    if (!playerCondition.is_object() || !hasOnlyKeys(playerCondition, {"condition", "entity", "predicate"})) {
        return std::nullopt;
    }
    if (!playerCondition.contains("condition") || !playerCondition.at("condition").is_string()
        || playerCondition.at("condition").get<std::string>() != "minecraft:entity_properties") {
        return std::nullopt;
    }
    if (!playerCondition.contains("entity") || !playerCondition.at("entity").is_string()
        || playerCondition.at("entity").get<std::string>() != "this") {
        return std::nullopt;
    }
    if (!playerCondition.contains("predicate") || !playerCondition.at("predicate").is_object()) {
        return std::nullopt;
    }

    auto const& entityPredicate = playerCondition.at("predicate");
    if (!hasOnlyKeys(entityPredicate, {"location"}) || !entityPredicate.contains("location")
        || !entityPredicate.at("location").is_object()) {
        return std::nullopt;
    }

    auto location = parseLocationPredicate(entityPredicate.at("location"));
    if (!location) {
        return std::nullopt;
    }

    return PlayerPredicate{*location};
}

} // namespace advancements::predicate
