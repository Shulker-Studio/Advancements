#include "mod/trigger/criteria/EntityCriteria.h"

#include "mod/trigger/criteria/Common.h"

namespace advancements::criteria {
namespace {

bool isSupportedFrogVariantId(std::string const& variantId) {
    return variantId == "minecraft:temperate" || variantId == "minecraft:cold" || variantId == "minecraft:warm";
}

TriggerCondition compilePlayerInteractedWithFrogCondition(nlohmann::json const& conditions) {
    if (!hasOnlyKeys(conditions, {"item", "entity"})) {
        return InvalidTriggerCondition{};
    }
    if (!conditions.contains("item") || !conditions.at("item").is_object()) {
        return InvalidTriggerCondition{};
    }
    if (!conditions.contains("entity") || !conditions.at("entity").is_array()) {
        return InvalidTriggerCondition{};
    }

    auto const& item = conditions.at("item");
    if (!hasOnlyKeys(item, {"items"}) || !item.contains("items") || !item.at("items").is_string()) {
        return InvalidTriggerCondition{};
    }
    auto const itemId = item.at("items").get<std::string>();
    if (itemId != "minecraft:lead") {
        return InvalidTriggerCondition{};
    }

    auto const& entity = conditions.at("entity");
    if (entity.size() != 1 || !entity.at(0).is_object()) {
        return InvalidTriggerCondition{};
    }

    auto const& entityEntry = entity.at(0);
    if (!hasOnlyKeys(entityEntry, {"condition", "entity", "predicate"})) {
        return InvalidTriggerCondition{};
    }
    if (!entityEntry.contains("condition") || !entityEntry.at("condition").is_string()
        || entityEntry.at("condition").get<std::string>() != "minecraft:entity_properties") {
        return InvalidTriggerCondition{};
    }
    if (!entityEntry.contains("entity") || !entityEntry.at("entity").is_string()
        || entityEntry.at("entity").get<std::string>() != "this") {
        return InvalidTriggerCondition{};
    }
    if (!entityEntry.contains("predicate") || !entityEntry.at("predicate").is_object()) {
        return InvalidTriggerCondition{};
    }

    auto const& predicate = entityEntry.at("predicate");
    if (!hasOnlyKeys(predicate, {"type", "type_specific"})) {
        return InvalidTriggerCondition{};
    }
    if (!predicate.contains("type") || !predicate.at("type").is_string()
        || predicate.at("type").get<std::string>() != "minecraft:frog") {
        return InvalidTriggerCondition{};
    }
    if (!predicate.contains("type_specific") || !predicate.at("type_specific").is_object()) {
        return InvalidTriggerCondition{};
    }

    auto const& typeSpecific = predicate.at("type_specific");
    if (!hasOnlyKeys(typeSpecific, {"type", "variant"})) {
        return InvalidTriggerCondition{};
    }
    if (!typeSpecific.contains("type") || !typeSpecific.at("type").is_string()
        || typeSpecific.at("type").get<std::string>() != "minecraft:frog") {
        return InvalidTriggerCondition{};
    }
    if (!typeSpecific.contains("variant") || !typeSpecific.at("variant").is_string()) {
        return InvalidTriggerCondition{};
    }

    auto const variantId = typeSpecific.at("variant").get<std::string>();
    if (!isSupportedFrogVariantId(variantId)) {
        return InvalidTriggerCondition{};
    }

    return PlayerInteractedWithEntityCondition{itemId, "minecraft:frog", variantId};
}

TriggerCondition compilePlayerKilledEntityProjectileCondition(nlohmann::json const& conditions) {
    if (!hasOnlyKeys(conditions, {"entity", "killing_blow"})) {
        return InvalidTriggerCondition{};
    }
    if (!conditions.contains("entity") || !conditions.at("entity").is_array()) {
        return InvalidTriggerCondition{};
    }

    auto const& entity = conditions.at("entity");
    if (entity.size() != 1 || !entity.at(0).is_object()) {
        return InvalidTriggerCondition{};
    }

    auto const& entityEntry = entity.at(0);
    if (!hasOnlyKeys(entityEntry, {"condition", "entity", "predicate"})) {
        return InvalidTriggerCondition{};
    }
    if (!entityEntry.contains("condition") || !entityEntry.at("condition").is_string()
        || entityEntry.at("condition").get<std::string>() != "minecraft:entity_properties") {
        return InvalidTriggerCondition{};
    }
    if (!entityEntry.contains("entity") || !entityEntry.at("entity").is_string()
        || entityEntry.at("entity").get<std::string>() != "this") {
        return InvalidTriggerCondition{};
    }
    if (!entityEntry.contains("predicate") || !entityEntry.at("predicate").is_object()) {
        return InvalidTriggerCondition{};
    }

    auto const& predicate = entityEntry.at("predicate");
    if (!hasOnlyKeys(predicate, {"type", "distance"})) {
        return InvalidTriggerCondition{};
    }
    if (!predicate.contains("type") || !predicate.at("type").is_string()) {
        return InvalidTriggerCondition{};
    }

    auto const targetEntityTypeId = predicate.at("type").get<std::string>();
    std::optional<float> horizontalMin;
    if (predicate.contains("distance")) {
        if (!predicate.at("distance").is_object()) {
            return InvalidTriggerCondition{};
        }
        auto const& distance = predicate.at("distance");
        if (!hasOnlyKeys(distance, {"horizontal"}) || !distance.contains("horizontal")
            || !distance.at("horizontal").is_object()) {
            return InvalidTriggerCondition{};
        }

        auto const& horizontal = distance.at("horizontal");
        if (!hasOnlyKeys(horizontal, {"min"}) || !horizontal.contains("min") || !horizontal.at("min").is_number()) {
            return InvalidTriggerCondition{};
        }
        horizontalMin = horizontal.at("min").get<float>();
    }

    if (!conditions.contains("killing_blow") || !conditions.at("killing_blow").is_object()) {
        return InvalidTriggerCondition{};
    }

    auto const& killingBlow = conditions.at("killing_blow");
    if (!hasOnlyKeys(killingBlow, {"direct_entity", "tags"}) || !killingBlow.contains("tags")
        || !killingBlow.at("tags").is_array()) {
        return InvalidTriggerCondition{};
    }

    std::optional<std::string> directEntityTypeId;
    if (killingBlow.contains("direct_entity")) {
        if (!killingBlow.at("direct_entity").is_object()) {
            return InvalidTriggerCondition{};
        }
        auto const& directEntity = killingBlow.at("direct_entity");
        if (!hasOnlyKeys(directEntity, {"type"}) || !directEntity.contains("type")
            || !directEntity.at("type").is_string()) {
            return InvalidTriggerCondition{};
        }
        directEntityTypeId = directEntity.at("type").get<std::string>();
    }

    auto const& tags = killingBlow.at("tags");
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

    if (targetEntityTypeId == "minecraft:skeleton") {
        if (!horizontalMin || *horizontalMin != 50.0F || directEntityTypeId) {
            return InvalidTriggerCondition{};
        }
    } else if (targetEntityTypeId == "minecraft:ghast") {
        if (horizontalMin || directEntityTypeId != "minecraft:fireball") {
            return InvalidTriggerCondition{};
        }
    } else {
        return InvalidTriggerCondition{};
    }

    return PlayerKilledEntitySniperDuelCondition{targetEntityTypeId, horizontalMin, true, directEntityTypeId};
}

bool matchesPlayerKilledEntityProjectileCondition(TriggerCondition const& condition, TriggerContext const& context) {
    auto const* compiled = std::get_if<PlayerKilledEntitySniperDuelCondition>(&condition);
    auto const* payload  = payloadAs<PlayerKilledEntitySniperDuelPayload>(context);
    if (compiled == nullptr || payload == nullptr) {
        return false;
    }
    if (payload->killedEntityTypeId != compiled->targetEntityTypeId) {
        return false;
    }
    if (compiled->horizontalDistanceMin && payload->horizontalDistance < *compiled->horizontalDistanceMin) {
        return false;
    }
    if (compiled->requireProjectileKillingBlow && !payload->killingBlowIsProjectile) {
        return false;
    }
    if (compiled->directEntityTypeId && payload->directEntityTypeId != compiled->directEntityTypeId) {
        return false;
    }
    return true;
}

} // namespace

TriggerCondition compileEntityCondition(nlohmann::json const& conditions) {
    if (!hasOnlyKeys(conditions, {"entity"})) {
        return InvalidTriggerCondition{};
    }
    if (!conditions.contains("entity") || !conditions.at("entity").is_string()) {
        return InvalidTriggerCondition{};
    }
    return EntityTriggerCondition{conditions.at("entity").get<std::string>()};
}

TriggerCondition compilePlayerInteractedWithEntityCondition(nlohmann::json const& conditions) {
    return compilePlayerInteractedWithFrogCondition(conditions);
}

TriggerCondition compilePlayerKilledEntityCondition(nlohmann::json const& conditions) {
    auto compiled = compilePlayerKilledEntityProjectileCondition(conditions);
    if (!std::holds_alternative<InvalidTriggerCondition>(compiled)) {
        return compiled;
    }
    return compileEntityCondition(conditions);
}

TriggerCondition compileSummonedEntityCondition(nlohmann::json const& conditions) {
    if (!hasOnlyKeys(conditions, {"entity"})) {
        return InvalidTriggerCondition{};
    }
    if (!conditions.contains("entity") || !conditions.at("entity").is_array()) {
        return InvalidTriggerCondition{};
    }

    auto const& entity = conditions.at("entity");
    if (entity.size() != 1 || !entity.at(0).is_object()) {
        return InvalidTriggerCondition{};
    }

    auto const& entityEntry = entity.at(0);
    if (!hasOnlyKeys(entityEntry, {"condition", "entity", "predicate"})) {
        return InvalidTriggerCondition{};
    }
    if (!entityEntry.contains("condition") || !entityEntry.at("condition").is_string()
        || entityEntry.at("condition").get<std::string>() != "minecraft:entity_properties") {
        return InvalidTriggerCondition{};
    }
    if (!entityEntry.contains("entity") || !entityEntry.at("entity").is_string()
        || entityEntry.at("entity").get<std::string>() != "this") {
        return InvalidTriggerCondition{};
    }
    if (!entityEntry.contains("predicate") || !entityEntry.at("predicate").is_object()) {
        return InvalidTriggerCondition{};
    }

    auto const& predicate = entityEntry.at("predicate");
    if (!hasOnlyKeys(predicate, {"type"}) || !predicate.contains("type") || !predicate.at("type").is_string()) {
        return InvalidTriggerCondition{};
    }

    auto const entityTypeId = predicate.at("type").get<std::string>();
    if (entityTypeId != "minecraft:wither" && entityTypeId != "minecraft:ender_dragon") {
        return InvalidTriggerCondition{};
    }
    return EntityTriggerCondition{entityTypeId};
}

bool matchesEntityCondition(TriggerCondition const& condition, TriggerContext const& context) {
    auto const* compiled = std::get_if<EntityTriggerCondition>(&condition);
    auto const* payload  = payloadAs<EntityTriggerPayload>(context);
    if (compiled == nullptr || payload == nullptr) {
        return false;
    }
    return payload->entityTypeId == compiled->entityTypeId;
}

bool matchesPlayerInteractedWithEntityCondition(TriggerCondition const& condition, TriggerContext const& context) {
    auto const* compiled = std::get_if<PlayerInteractedWithEntityCondition>(&condition);
    auto const* payload  = payloadAs<PlayerInteractedWithEntityPayload>(context);
    if (compiled == nullptr || payload == nullptr) {
        return false;
    }
    return payload->itemId == compiled->itemId && payload->entityTypeId == compiled->entityTypeId
        && payload->entityVariantId == compiled->entityVariantId;
}

bool matchesPlayerKilledEntityCondition(TriggerCondition const& condition, TriggerContext const& context) {
    if (std::holds_alternative<PlayerKilledEntitySniperDuelCondition>(condition)) {
        return matchesPlayerKilledEntityProjectileCondition(condition, context);
    }

    auto const* compiled = std::get_if<EntityTriggerCondition>(&condition);
    if (compiled == nullptr) {
        return false;
    }

    if (auto const* payload = payloadAs<EntityTriggerPayload>(context); payload != nullptr) {
        return payload->entityTypeId == compiled->entityTypeId;
    }

    if (auto const* payload = payloadAs<PlayerKilledEntitySniperDuelPayload>(context); payload != nullptr) {
        return payload->killedEntityTypeId == compiled->entityTypeId;
    }

    return false;
}

} // namespace advancements::criteria
