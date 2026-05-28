#include "mod/predicate/EntityPredicate.h"

#include "mod/predicate/Common.h"

namespace advancements::predicate {

std::optional<nlohmann::json const*> parseSingleThisEntityPredicateRoot(nlohmann::json const& conditions, char const* rootKey) {
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

    return &entityEntry.at("predicate");
}

std::optional<std::string> parseEntityTypePredicate(nlohmann::json const& predicate) {
    if (!predicate.contains("type") || !predicate.at("type").is_string()) {
        return std::nullopt;
    }
    return predicate.at("type").get<std::string>();
}

std::optional<std::string> parseFrogVariantPredicate(nlohmann::json const& predicate) {
    auto const typeId = parseEntityTypePredicate(predicate);
    if (!typeId || *typeId != "minecraft:frog") {
        return std::nullopt;
    }
    if (!predicate.contains("type_specific") || !predicate.at("type_specific").is_object()) {
        return std::nullopt;
    }

    auto const& typeSpecific = predicate.at("type_specific");
    if (!hasOnlyKeys(typeSpecific, {"type", "variant"})) {
        return std::nullopt;
    }
    if (!typeSpecific.contains("type") || !typeSpecific.at("type").is_string()
        || typeSpecific.at("type").get<std::string>() != "minecraft:frog") {
        return std::nullopt;
    }
    if (!typeSpecific.contains("variant") || !typeSpecific.at("variant").is_string()) {
        return std::nullopt;
    }

    auto variantId = typeSpecific.at("variant").get<std::string>();
    if (variantId != "minecraft:temperate" && variantId != "minecraft:cold" && variantId != "minecraft:warm") {
        return std::nullopt;
    }
    return variantId;
}

std::optional<EntityPredicate> parseSingleThisEntityDistancePredicate(nlohmann::json const& conditions, char const* rootKey) {
    if (!hasOnlyKeys(conditions, {rootKey, "signal_strength"})) {
        return std::nullopt;
    }

    auto entityPredicate = parseSingleThisEntityPredicateRoot(conditions, rootKey);
    if (!entityPredicate) {
        return std::nullopt;
    }

    auto const& predicate = **entityPredicate;
    if (!hasOnlyKeys(predicate, {"distance"}) || !predicate.contains("distance") || !predicate.at("distance").is_object()) {
        return std::nullopt;
    }

    auto horizontalDistance = parseHorizontalDistancePredicate(predicate.at("distance"));
    if (!horizontalDistance) {
        return std::nullopt;
    }

    return EntityPredicate{std::nullopt, horizontalDistance, std::nullopt};
}

} // namespace advancements::predicate
