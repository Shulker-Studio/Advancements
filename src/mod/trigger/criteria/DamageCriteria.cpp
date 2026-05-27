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
    if (!hasOnlyKeys(damage, {"type", "dealt"})) {
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
    bool requireMainhandMace        = false;
    std::optional<float> damageDealtMin;

    if (type.contains("direct_entity")) {
        if (!type.at("direct_entity").is_object()) {
            return InvalidTriggerCondition{};
        }
        auto const& directEntity = type.at("direct_entity");
        if (!hasOnlyKeys(directEntity, {"type", "equipment"})) {
            return InvalidTriggerCondition{};
        }
        if (directEntity.contains("type")) {
            if (!directEntity.at("type").is_string()) {
                return InvalidTriggerCondition{};
            }
            if (directEntity.at("type").get<std::string>() != "#minecraft:arrows") {
                return InvalidTriggerCondition{};
            }
            requireArrowDirectEntity = true;
        }
        if (directEntity.contains("equipment")) {
            if (!directEntity.at("equipment").is_object()) {
                return InvalidTriggerCondition{};
            }
            auto const& equipment = directEntity.at("equipment");
            if (!hasOnlyKeys(equipment, {"mainhand"})) {
                return InvalidTriggerCondition{};
            }
            if (!equipment.contains("mainhand") || !equipment.at("mainhand").is_object()) {
                return InvalidTriggerCondition{};
            }
            auto const& mainhand = equipment.at("mainhand");
            if (!hasOnlyKeys(mainhand, {"items"})) {
                return InvalidTriggerCondition{};
            }
            if (!mainhand.contains("items") || !mainhand.at("items").is_string()) {
                return InvalidTriggerCondition{};
            }
            if (mainhand.at("items").get<std::string>() != "minecraft:mace") {
                return InvalidTriggerCondition{};
            }
            requireMainhandMace = true;
        }
        if (!requireArrowDirectEntity && !requireMainhandMace) {
            return InvalidTriggerCondition{};
        }
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
        auto const tagId = tagEntry.at("id").get<std::string>();
        if (!tagEntry.at("expected").get<bool>()) {
            return InvalidTriggerCondition{};
        }
        if (tagId == "minecraft:is_projectile") {
            requireProjectileDamageTag = true;
        } else if (tagId == "minecraft:mace_smash") {
            requireMainhandMace = true;
        } else {
            return InvalidTriggerCondition{};
        }
    }

    if (damage.contains("dealt")) {
        if (!damage.at("dealt").is_object()) {
            return InvalidTriggerCondition{};
        }
        auto const& dealt = damage.at("dealt");
        if (!hasOnlyKeys(dealt, {"min"})) {
            return InvalidTriggerCondition{};
        }
        if (!dealt.contains("min") || !dealt.at("min").is_number()) {
            return InvalidTriggerCondition{};
        }
        damageDealtMin = dealt.at("min").get<float>();
    }

    if (requireArrowDirectEntity || requireProjectileDamageTag) {
        if (!requireArrowDirectEntity || !requireProjectileDamageTag || requireMainhandMace || damageDealtMin.has_value()) {
            return InvalidTriggerCondition{};
        }
        return PlayerHurtEntityCondition{true, true, false, std::nullopt};
    }

    if (!requireMainhandMace || !damageDealtMin.has_value()) {
        return InvalidTriggerCondition{};
    }

    return PlayerHurtEntityCondition{false, false, true, damageDealtMin};
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
    if (compiled->requireMainhandMace && !payload->mainhandItemIsMace) {
        return false;
    }
    if (compiled->damageDealtMin.has_value() && payload->damageDealt < *compiled->damageDealtMin) {
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
