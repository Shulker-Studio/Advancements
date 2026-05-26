#include "mod/trigger/criteria/NetherTravelCriteria.h"

#include "mod/trigger/criteria/Common.h"

namespace advancements::criteria {

TriggerCondition compileNetherTravelCondition(nlohmann::json const& conditions) {
    if (!hasOnlyKeys(conditions, {"distance"})) {
        return InvalidTriggerCondition{};
    }
    if (!conditions.contains("distance") || !conditions.at("distance").is_object()) {
        return InvalidTriggerCondition{};
    }

    auto const& distance = conditions.at("distance");
    if (!hasOnlyKeys(distance, {"horizontal"}) || !distance.contains("horizontal")
        || !distance.at("horizontal").is_object()) {
        return InvalidTriggerCondition{};
    }

    auto const& horizontal = distance.at("horizontal");
    if (!hasOnlyKeys(horizontal, {"min"}) || !horizontal.contains("min") || !horizontal.at("min").is_number()) {
        return InvalidTriggerCondition{};
    }

    return NetherTravelTriggerCondition{horizontal.at("min").get<float>()};
}

bool matchesNetherTravelCondition(TriggerCondition const& condition, TriggerContext const& context) {
    auto const* compiled = std::get_if<NetherTravelTriggerCondition>(&condition);
    auto const* payload  = payloadAs<NetherTravelTriggerPayload>(context);
    if (compiled == nullptr || payload == nullptr) {
        return false;
    }
    return payload->horizontalDistance >= compiled->horizontalDistanceMin;
}

} // namespace advancements::criteria
