#include "mod/trigger/criteria/EntityCriteria.h"

#include "mod/predicate/DistancePredicate.h"
#include "mod/predicate/EntityPredicate.h"
#include "mod/trigger/criteria/Common.h"

#include <algorithm>

namespace advancements::criteria {
namespace {

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

    auto const entityPredicate = predicate::parseSingleThisEntityPredicateRoot(conditions, "entity");
    if (!entityPredicate) {
        return InvalidTriggerCondition{};
    }

    auto const& entityPredicateJson = **entityPredicate;
    if (!hasOnlyKeys(entityPredicateJson, {"type", "type_specific"})) {
        return InvalidTriggerCondition{};
    }
    auto const variantId = predicate::parseFrogVariantPredicate(entityPredicateJson);
    if (!variantId) {
        return InvalidTriggerCondition{};
    }

    return PlayerInteractedWithEntityCondition{itemId, "minecraft:frog", *variantId};
}

TriggerCondition compilePlayerKilledEntityProjectileCondition(nlohmann::json const& conditions) {
    if (!hasOnlyKeys(conditions, {"entity", "killing_blow"})) {
        return InvalidTriggerCondition{};
    }
    if (!conditions.contains("entity") || !conditions.at("entity").is_array()) {
        return InvalidTriggerCondition{};
    }

    auto const entityPredicate = predicate::parseSingleThisEntityPredicateRoot(conditions, "entity");
    if (!entityPredicate) {
        return InvalidTriggerCondition{};
    }

    auto const& entityPredicateJson = **entityPredicate;
    if (!hasOnlyKeys(entityPredicateJson, {"type", "distance"})) {
        return InvalidTriggerCondition{};
    }
    auto const targetEntityTypeId = predicate::parseEntityTypePredicate(entityPredicateJson);
    if (!targetEntityTypeId) {
        return InvalidTriggerCondition{};
    }
    std::optional<float> horizontalMin;
    if (entityPredicateJson.contains("distance")) {
        auto const distance = predicate::parseHorizontalDistancePredicate(entityPredicateJson.at("distance"));
        if (!distance) {
            return InvalidTriggerCondition{};
        }
        horizontalMin = distance->min;
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
        if (!hasOnlyKeys(directEntity, {"type"})) {
            return InvalidTriggerCondition{};
        }
        directEntityTypeId = predicate::parseEntityTypePredicate(directEntity);
        if (!directEntityTypeId) {
            return InvalidTriggerCondition{};
        }
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

    if (*targetEntityTypeId == "minecraft:skeleton") {
        if (!horizontalMin || *horizontalMin != 50.0F || directEntityTypeId) {
            return InvalidTriggerCondition{};
        }
    } else if (*targetEntityTypeId == "minecraft:ghast") {
        if (horizontalMin || directEntityTypeId != "minecraft:fireball") {
            return InvalidTriggerCondition{};
        }
    } else {
        return InvalidTriggerCondition{};
    }

    return PlayerKilledEntitySniperDuelCondition{*targetEntityTypeId, horizontalMin, true, directEntityTypeId};
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

    auto const entityPredicate = predicate::parseSingleThisEntityPredicateRoot(conditions, "entity");
    if (!entityPredicate) {
        return InvalidTriggerCondition{};
    }

    auto const& entityPredicateJson = **entityPredicate;
    if (!hasOnlyKeys(entityPredicateJson, {"type"})) {
        return InvalidTriggerCondition{};
    }

    auto const entityTypeId = predicate::parseEntityTypePredicate(entityPredicateJson);
    if (!entityTypeId || (*entityTypeId != "minecraft:wither" && *entityTypeId != "minecraft:ender_dragon")) {
        return InvalidTriggerCondition{};
    }
    return EntityTriggerCondition{*entityTypeId};
}

TriggerCondition compileBredAnimalsCondition(nlohmann::json const& conditions) {
    if (!hasOnlyKeys(conditions, {"child"})) {
        return InvalidTriggerCondition{};
    }
    if (!conditions.contains("child") || !conditions.at("child").is_array()) {
        return InvalidTriggerCondition{};
    }

    auto const entityPredicate = predicate::parseSingleThisEntityPredicateRoot(conditions, "child");
    if (!entityPredicate) {
        return InvalidTriggerCondition{};
    }

    auto const& entityPredicateJson = **entityPredicate;
    if (!hasOnlyKeys(entityPredicateJson, {"type"})) {
        return InvalidTriggerCondition{};
    }

    auto const entityTypeId = predicate::parseEntityTypePredicate(entityPredicateJson);
    if (!entityTypeId) {
        return InvalidTriggerCondition{};
    }
    return BredAnimalsCondition{*entityTypeId};
}

TriggerCondition compileTameAnimalCondition(nlohmann::json const& conditions) {
    if (conditions.empty()) {
        return NoTriggerCondition{};
    }

    if (!hasOnlyKeys(conditions, {"entity"})) {
        return InvalidTriggerCondition{};
    }

    auto const entityPredicate = predicate::parseSingleThisEntityPredicateRoot(conditions, "entity");
    if (!entityPredicate) {
        return InvalidTriggerCondition{};
    }

    auto const& entityPredicateJson = **entityPredicate;
    if (!hasOnlyKeys(entityPredicateJson, {"type_specific"}) || !entityPredicateJson.at("type_specific").is_object()) {
        return InvalidTriggerCondition{};
    }

    auto const& typeSpecific = entityPredicateJson.at("type_specific");
    if (!hasOnlyKeys(typeSpecific, {"type", "variant"}) || !typeSpecific.contains("type")
        || !typeSpecific.at("type").is_string() || !typeSpecific.contains("variant")
        || !typeSpecific.at("variant").is_string()) {
        return InvalidTriggerCondition{};
    }

    auto const entityTypeId    = typeSpecific.at("type").get<std::string>();
    auto const entityVariantId = typeSpecific.at("variant").get<std::string>();
    if (entityTypeId != "minecraft:cat" && entityTypeId != "minecraft:wolf") {
        return InvalidTriggerCondition{};
    }

    return TameAnimalCondition{entityTypeId, entityVariantId};
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

bool matchesBredAnimalsCondition(TriggerCondition const& condition, TriggerContext const& context) {
    auto const* compiled = std::get_if<BredAnimalsCondition>(&condition);
    auto const* payload  = payloadAs<BredAnimalsPayload>(context);
    if (compiled == nullptr || payload == nullptr) {
        return false;
    }
    return std::ranges::find(payload->childTypeIds, compiled->childTypeId) != payload->childTypeIds.end();
}

bool matchesTameAnimalCondition(TriggerCondition const& condition, TriggerContext const& context) {
    if (std::holds_alternative<NoTriggerCondition>(condition)) {
        return payloadAs<TameAnimalPayload>(context) != nullptr;
    }

    auto const* compiled = std::get_if<TameAnimalCondition>(&condition);
    auto const* payload  = payloadAs<TameAnimalPayload>(context);
    if (compiled == nullptr || payload == nullptr) {
        return false;
    }
    return payload->entityTypeId == compiled->entityTypeId && payload->entityVariantId == compiled->entityVariantId;
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
