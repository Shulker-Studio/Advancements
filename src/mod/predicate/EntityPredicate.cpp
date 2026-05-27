#include "mod/predicate/EntityPredicate.h"

#include "mod/predicate/Common.h"

namespace advancements::predicate {

std::optional<EntityPredicate> parseSingleThisEntityDistancePredicate(nlohmann::json const& conditions, char const* rootKey) {
    if (!hasOnlyKeys(conditions, {rootKey, "signal_strength"})) {
        return std::nullopt;
    }
    if (!conditions.contains(rootKey) || !conditions.at(rootKey).is_array()) {
        return std::nullopt;
    }

    auto const& entities = conditions.at(rootKey);
    if (entities.size() != 1 || !entities.at(0).is_object()) {
        return std::nullopt;
    }

    auto const& entityEntry = entities.at(0);
    if (!hasOnlyKeys(entityEntry, {"condition", "entity", "predicate"})) {
        return std::nullopt;
    }
    if (!entityEntry.contains("condition") || !entityEntry.at("condition").is_string()
        || entityEntry.at("condition").get<std::string>() != "minecraft:entity_properties") {
        return std::nullopt;
    }
    if (!entityEntry.contains("entity") || !entityEntry.at("entity").is_string()
        || entityEntry.at("entity").get<std::string>() != "this") {
        return std::nullopt;
    }
    if (!entityEntry.contains("predicate") || !entityEntry.at("predicate").is_object()) {
        return std::nullopt;
    }

    auto const& predicate = entityEntry.at("predicate");
    if (!hasOnlyKeys(predicate, {"distance"}) || !predicate.contains("distance") || !predicate.at("distance").is_object()) {
        return std::nullopt;
    }

    auto horizontalDistance = parseHorizontalDistancePredicate(predicate.at("distance"));
    if (!horizontalDistance) {
        return std::nullopt;
    }

    return EntityPredicate{horizontalDistance};
}

} // namespace advancements::predicate
