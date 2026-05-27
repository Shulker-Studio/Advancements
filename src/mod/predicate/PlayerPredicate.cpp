#include "mod/predicate/PlayerPredicate.h"

#include "mod/predicate/Common.h"

namespace advancements::predicate {

std::optional<nlohmann::json const*> parseSinglePlayerThisEntityPredicateRoot(nlohmann::json const& conditions) {
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

    return &playerCondition.at("predicate");
}

std::optional<PlayerPredicate> parseSinglePlayerLocationPredicate(nlohmann::json const& conditions) {
    auto entityPredicate = parseSinglePlayerThisEntityPredicateRoot(conditions);
    if (!entityPredicate) {
        return std::nullopt;
    }

    auto const& predicate = **entityPredicate;
    if (!hasOnlyKeys(predicate, {"location"}) || !predicate.contains("location") || !predicate.at("location").is_object()) {
        return std::nullopt;
    }

    auto location = parseLocationPredicate(predicate.at("location"));
    if (!location) {
        return std::nullopt;
    }

    return PlayerPredicate{*location};
}

} // namespace advancements::predicate
