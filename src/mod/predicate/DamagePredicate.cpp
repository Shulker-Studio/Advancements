#include "mod/predicate/DamagePredicate.h"

#include "mod/predicate/Common.h"

namespace advancements::predicate {
namespace {

std::optional<std::string> parseDamageTagPredicate(nlohmann::json const& tagEntry) {
    if (!hasOnlyKeys(tagEntry, {"id", "expected"})) {
        return std::nullopt;
    }
    if (!tagEntry.contains("id") || !tagEntry.at("id").is_string()) {
        return std::nullopt;
    }
    if (!tagEntry.contains("expected") || !tagEntry.at("expected").is_boolean()) {
        return std::nullopt;
    }
    if (!tagEntry.at("expected").get<bool>()) {
        return std::nullopt;
    }
    return tagEntry.at("id").get<std::string>();
}

std::optional<std::string> parseDirectEntityType(nlohmann::json const& directEntity) {
    if (!directEntity.contains("type")) {
        return std::nullopt;
    }
    if (!directEntity.at("type").is_string()) {
        return std::nullopt;
    }
    return directEntity.at("type").get<std::string>();
}

std::optional<std::string> parseDirectEntityMainhandItem(nlohmann::json const& directEntity) {
    if (!directEntity.contains("equipment")) {
        return std::nullopt;
    }
    if (!directEntity.at("equipment").is_object()) {
        return std::nullopt;
    }
    auto const& equipment = directEntity.at("equipment");
    if (!hasOnlyKeys(equipment, {"mainhand"})) {
        return std::nullopt;
    }
    if (!equipment.contains("mainhand") || !equipment.at("mainhand").is_object()) {
        return std::nullopt;
    }
    auto const& mainhand = equipment.at("mainhand");
    if (!hasOnlyKeys(mainhand, {"items"})) {
        return std::nullopt;
    }
    if (!mainhand.contains("items") || !mainhand.at("items").is_string()) {
        return std::nullopt;
    }
    return mainhand.at("items").get<std::string>();
}

std::optional<float> parseDamageDealtMin(nlohmann::json const& damage) {
    if (!damage.contains("dealt")) {
        return std::nullopt;
    }
    if (!damage.at("dealt").is_object()) {
        return std::nullopt;
    }
    auto const& dealt = damage.at("dealt");
    if (!hasOnlyKeys(dealt, {"min"})) {
        return std::nullopt;
    }
    if (!dealt.contains("min") || !dealt.at("min").is_number()) {
        return std::nullopt;
    }
    return dealt.at("min").get<float>();
}

} // namespace

std::optional<DamagePredicate> parseBlockedProjectileDamagePredicate(nlohmann::json const& conditions) {
    if (!hasOnlyKeys(conditions, {"damage"})) {
        return std::nullopt;
    }
    if (!conditions.contains("damage") || !conditions.at("damage").is_object()) {
        return std::nullopt;
    }

    auto const& damage = conditions.at("damage");
    if (!hasOnlyKeys(damage, {"blocked", "type"})) {
        return std::nullopt;
    }
    if (!damage.contains("blocked") || !damage.at("blocked").is_boolean() || !damage.at("blocked").get<bool>()) {
        return std::nullopt;
    }
    if (!damage.contains("type") || !damage.at("type").is_object()) {
        return std::nullopt;
    }

    auto const& type = damage.at("type");
    if (!hasOnlyKeys(type, {"tags"}) || !type.contains("tags") || !type.at("tags").is_array()) {
        return std::nullopt;
    }

    auto const& tags = type.at("tags");
    if (tags.size() != 1 || !tags.at(0).is_object()) {
        return std::nullopt;
    }

    auto const& tagEntry = tags.at(0);
    if (!hasOnlyKeys(tagEntry, {"id", "expected"})) {
        return std::nullopt;
    }
    if (!tagEntry.contains("id") || !tagEntry.at("id").is_string() || !tagEntry.contains("expected")
        || !tagEntry.at("expected").is_boolean()) {
        return std::nullopt;
    }
    if (tagEntry.at("id").get<std::string>() != "minecraft:is_projectile" || !tagEntry.at("expected").get<bool>()) {
        return std::nullopt;
    }

    return DamagePredicate{true, true, false, std::nullopt, std::nullopt, std::nullopt};
}

std::optional<DamagePredicate> parsePlayerHurtEntityDamagePredicate(nlohmann::json const& conditions) {
    if (!hasOnlyKeys(conditions, {"damage"})) {
        return std::nullopt;
    }
    if (!conditions.contains("damage") || !conditions.at("damage").is_object()) {
        return std::nullopt;
    }

    auto const& damage = conditions.at("damage");
    if (!hasOnlyKeys(damage, {"type", "dealt"})) {
        return std::nullopt;
    }
    if (!damage.contains("type") || !damage.at("type").is_object()) {
        return std::nullopt;
    }

    auto const& type = damage.at("type");
    if (!hasOnlyKeys(type, {"direct_entity", "tags"})) {
        return std::nullopt;
    }

    DamagePredicate predicate{false, false, false, std::nullopt, std::nullopt, parseDamageDealtMin(damage)};
    if (damage.contains("dealt") && !predicate.dealtMin) {
        return std::nullopt;
    }

    if (type.contains("direct_entity")) {
        if (!type.at("direct_entity").is_object()) {
            return std::nullopt;
        }
        auto const& directEntity = type.at("direct_entity");
        if (!hasOnlyKeys(directEntity, {"type", "equipment"})) {
            return std::nullopt;
        }
        predicate.directEntityTypeId = parseDirectEntityType(directEntity);
        if (directEntity.contains("type") && !predicate.directEntityTypeId) {
            return std::nullopt;
        }
        predicate.directEntityMainhandItemId = parseDirectEntityMainhandItem(directEntity);
        if (directEntity.contains("equipment") && !predicate.directEntityMainhandItemId) {
            return std::nullopt;
        }
        if (!predicate.directEntityTypeId && !predicate.directEntityMainhandItemId) {
            return std::nullopt;
        }
    }

    if (type.contains("tags")) {
        if (!type.at("tags").is_array()) {
            return std::nullopt;
        }
        auto const& tags = type.at("tags");
        if (tags.size() != 1 || !tags.at(0).is_object()) {
            return std::nullopt;
        }
        auto const tagId = parseDamageTagPredicate(tags.at(0));
        if (!tagId) {
            return std::nullopt;
        }
        if (*tagId == "minecraft:is_projectile") {
            predicate.requireProjectileDamageTag = true;
        } else if (*tagId == "minecraft:mace_smash") {
            predicate.requireMaceSmashDamageTag = true;
        } else {
            return std::nullopt;
        }
    }

    return predicate;
}

} // namespace advancements::predicate
