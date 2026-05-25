#include "mod/trigger/criteria/DamageCriteria.h"

#include "mod/trigger/criteria/Common.h"

namespace advancements::criteria {

TriggerCondition compilePlayerHurtEntityCondition(nlohmann::json const& conditions) {
    if (!hasOnlyKeys(conditions, {"damage"})) {
        return InvalidTriggerCondition{};
    }
    if (!conditions.contains("damage") || !conditions.at("damage").is_object()) {
        return InvalidTriggerCondition{};
    }

    auto const& damage = conditions.at("damage");
    if (!hasOnlyKeys(damage, {"type"})) {
        return InvalidTriggerCondition{};
    }
    if (!damage.contains("type") || !damage.at("type").is_object()) {
        return InvalidTriggerCondition{};
    }

    auto const& type = damage.at("type");
    if (!hasOnlyKeys(type, {"direct_entity", "tags"})) {
        return InvalidTriggerCondition{};
    }

    bool requireArrowDirectEntity   = false;
    bool requireProjectileDamageTag = false;

    if (type.contains("direct_entity")) {
        if (!type.at("direct_entity").is_object()) {
            return InvalidTriggerCondition{};
        }
        auto const& directEntity = type.at("direct_entity");
        if (!hasOnlyKeys(directEntity, {"type"})) {
            return InvalidTriggerCondition{};
        }
        if (!directEntity.contains("type") || !directEntity.at("type").is_string()) {
            return InvalidTriggerCondition{};
        }
        if (directEntity.at("type").get<std::string>() != "#minecraft:arrows") {
            return InvalidTriggerCondition{};
        }
        requireArrowDirectEntity = true;
    }

    if (type.contains("tags")) {
        if (!type.at("tags").is_array()) {
            return InvalidTriggerCondition{};
        }
        auto const& tags = type.at("tags");
        if (tags.size() != 1 || !tags.at(0).is_object()) {
            return InvalidTriggerCondition{};
        }
        auto const& tagEntry = tags.at(0);
        if (!hasOnlyKeys(tagEntry, {"id", "expected"})) {
            return InvalidTriggerCondition{};
        }
        if (!tagEntry.contains("id") || !tagEntry.at("id").is_string()) {
            return InvalidTriggerCondition{};
        }
        if (!tagEntry.contains("expected") || !tagEntry.at("expected").is_boolean()) {
            return InvalidTriggerCondition{};
        }
        if (tagEntry.at("id").get<std::string>() != "minecraft:is_projectile" || !tagEntry.at("expected").get<bool>()) {
            return InvalidTriggerCondition{};
        }
        requireProjectileDamageTag = true;
    }

    if (!requireArrowDirectEntity || !requireProjectileDamageTag) {
        return InvalidTriggerCondition{};
    }

    return PlayerHurtEntityCondition{requireArrowDirectEntity, requireProjectileDamageTag};
}

TriggerCondition compileEntityHurtPlayerCondition(nlohmann::json const& conditions) {
    if (!hasOnlyKeys(conditions, {"damage"})) {
        return InvalidTriggerCondition{};
    }
    if (!conditions.contains("damage") || !conditions.at("damage").is_object()) {
        return InvalidTriggerCondition{};
    }

    auto const& damage = conditions.at("damage");
    if (!hasOnlyKeys(damage, {"blocked", "type"})) {
        return InvalidTriggerCondition{};
    }
    if (!damage.contains("blocked") || !damage.at("blocked").is_boolean() || !damage.at("blocked").get<bool>()) {
        return InvalidTriggerCondition{};
    }
    if (!damage.contains("type") || !damage.at("type").is_object()) {
        return InvalidTriggerCondition{};
    }

    auto const& type = damage.at("type");
    if (!hasOnlyKeys(type, {"tags"})) {
        return InvalidTriggerCondition{};
    }
    if (!type.contains("tags") || !type.at("tags").is_array()) {
        return InvalidTriggerCondition{};
    }

    auto const& tags = type.at("tags");
    if (tags.size() != 1 || !tags.at(0).is_object()) {
        return InvalidTriggerCondition{};
    }
    auto const& tagEntry = tags.at(0);
    if (!hasOnlyKeys(tagEntry, {"id", "expected"})) {
        return InvalidTriggerCondition{};
    }
    if (!tagEntry.contains("id") || !tagEntry.at("id").is_string() || !tagEntry.contains("expected")
        || !tagEntry.at("expected").is_boolean()) {
        return InvalidTriggerCondition{};
    }
    if (tagEntry.at("id").get<std::string>() != "minecraft:is_projectile" || !tagEntry.at("expected").get<bool>()) {
        return InvalidTriggerCondition{};
    }

    return EntityHurtPlayerCondition{true, true};
}

bool matchesPlayerHurtEntityCondition(TriggerCondition const& condition, TriggerContext const& context) {
    auto const* compiled = std::get_if<PlayerHurtEntityCondition>(&condition);
    auto const* payload  = payloadAs<PlayerHurtEntityPayload>(context);
    if (compiled == nullptr || payload == nullptr) {
        return false;
    }
    if (compiled->requireArrowDirectEntity && !payload->directEntityIsArrow) {
        return false;
    }
    if (compiled->requireProjectileDamageTag && !payload->isProjectileDamage) {
        return false;
    }
    return true;
}

bool matchesEntityHurtPlayerCondition(TriggerCondition const& condition, TriggerContext const& context) {
    auto const* compiled = std::get_if<EntityHurtPlayerCondition>(&condition);
    auto const* payload  = payloadAs<EntityHurtPlayerPayload>(context);
    if (compiled == nullptr || payload == nullptr) {
        return false;
    }
    if (compiled->requireBlockedDamage && !payload->blockedDamage) {
        return false;
    }
    if (compiled->requireProjectileDamageTag && !payload->isProjectileDamage) {
        return false;
    }
    return true;
}

} // namespace advancements::criteria
