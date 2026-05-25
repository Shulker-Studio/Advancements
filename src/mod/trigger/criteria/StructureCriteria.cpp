#include "mod/trigger/criteria/StructureCriteria.h"

#include "mod/trigger/criteria/Common.h"

namespace advancements::criteria {

TriggerCondition compileLocationStructureCondition(nlohmann::json const& conditions) {
    if (!hasOnlyKeys(conditions, {"player"})) {
        return InvalidTriggerCondition{};
    }
    if (!conditions.contains("player") || !conditions.at("player").is_array() || conditions.at("player").size() != 1) {
        return InvalidTriggerCondition{};
    }

    auto const& playerCondition = conditions.at("player").front();
    if (!playerCondition.is_object() || !hasOnlyKeys(playerCondition, {"condition", "entity", "predicate"})) {
        return InvalidTriggerCondition{};
    }
    if (!playerCondition.contains("condition") || !playerCondition.at("condition").is_string()
        || playerCondition.at("condition").get<std::string>() != "minecraft:entity_properties") {
        return InvalidTriggerCondition{};
    }
    if (!playerCondition.contains("entity") || !playerCondition.at("entity").is_string()
        || playerCondition.at("entity").get<std::string>() != "this") {
        return InvalidTriggerCondition{};
    }
    if (!playerCondition.contains("predicate") || !playerCondition.at("predicate").is_object()) {
        return InvalidTriggerCondition{};
    }

    auto const& predicate = playerCondition.at("predicate");
    if (!hasOnlyKeys(predicate, {"location"}) || !predicate.contains("location") || !predicate.at("location").is_object()) {
        return InvalidTriggerCondition{};
    }

    auto const& location = predicate.at("location");
    if (!hasOnlyKeys(location, {"structures"}) || !location.contains("structures") || !location.at("structures").is_string()) {
        return InvalidTriggerCondition{};
    }

    return LocationStructureCondition{location.at("structures").get<std::string>()};
}

bool matchesLocationStructureCondition(TriggerCondition const& condition, TriggerContext const& context) {
    auto const* compiled = std::get_if<LocationStructureCondition>(&condition);
    auto const* payload  = payloadAs<LocationStructurePayload>(context);
    if (compiled == nullptr || payload == nullptr) {
        return false;
    }
    return payload->structureId == compiled->structureId;
}

} // namespace advancements::criteria
