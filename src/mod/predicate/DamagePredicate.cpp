#include "mod/predicate/DamagePredicate.h"

#include "mod/predicate/Common.h"

namespace advancements::predicate {

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

    return DamagePredicate{true, true};
}

} // namespace advancements::predicate
