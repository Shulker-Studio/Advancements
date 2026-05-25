#include "mod/trigger/criteria/ProjectileCriteria.h"

#include "mod/trigger/criteria/Common.h"

namespace advancements::criteria {

TriggerCondition compileTargetHitCondition(nlohmann::json const& conditions) {
    if (!hasOnlyKeys(conditions, {"signal_strength", "projectile"})) {
        return InvalidTriggerCondition{};
    }

    if (!conditions.contains("signal_strength") || !conditions.at("signal_strength").is_number_integer()) {
        return InvalidTriggerCondition{};
    }
    auto const signalStrength = conditions.at("signal_strength").get<int>();
    if (signalStrength != 15) {
        return InvalidTriggerCondition{};
    }

    if (!conditions.contains("projectile") || !conditions.at("projectile").is_array()) {
        return InvalidTriggerCondition{};
    }
    auto const& projectile = conditions.at("projectile");
    if (projectile.size() != 1) {
        return InvalidTriggerCondition{};
    }

    auto const& projectileEntry = projectile.at(0);
    if (!projectileEntry.is_object() || !hasOnlyKeys(projectileEntry, {"condition", "entity", "predicate"})) {
        return InvalidTriggerCondition{};
    }
    if (!projectileEntry.contains("condition") || !projectileEntry.at("condition").is_string()
        || projectileEntry.at("condition").get<std::string>() != "minecraft:entity_properties") {
        return InvalidTriggerCondition{};
    }
    if (!projectileEntry.contains("entity") || !projectileEntry.at("entity").is_string()
        || projectileEntry.at("entity").get<std::string>() != "this") {
        return InvalidTriggerCondition{};
    }
    if (!projectileEntry.contains("predicate") || !projectileEntry.at("predicate").is_object()) {
        return InvalidTriggerCondition{};
    }

    auto const& predicate = projectileEntry.at("predicate");
    if (!hasOnlyKeys(predicate, {"distance"}) || !predicate.contains("distance") || !predicate.at("distance").is_object()) {
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

    auto const horizontalMin = horizontal.at("min").get<float>();
    if (horizontalMin != 30.0F) {
        return InvalidTriggerCondition{};
    }

    return TargetHitCondition{signalStrength, horizontalMin};
}

bool matchesTargetHitCondition(TriggerCondition const& condition, TriggerContext const& context) {
    auto const* compiled = std::get_if<TargetHitCondition>(&condition);
    auto const* payload  = payloadAs<TargetHitPayload>(context);
    if (compiled == nullptr || payload == nullptr) {
        return false;
    }
    if (payload->signalStrength != compiled->requiredSignalStrength) {
        return false;
    }
    return payload->projectileHorizontalDistance >= compiled->projectileHorizontalDistanceMin;
}

} // namespace advancements::criteria
